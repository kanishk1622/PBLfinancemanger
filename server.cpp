// server.cpp - the WEB version of the Finance Manager.
//
// It is a very small web server written with Windows sockets (Winsock).
// It does two jobs:
//   1. Sends the web page files (web/index.html, web/style.css, web/app.js).
//   2. Answers the page's requests ("/api/...") by calling FinanceSystem,
//      and replies in JSON. All rules and data structures stay in C++.
//
// Build (from the project folder):  see the "Build web server" task, or
//   g++ -std=c++14 -Wall server.cpp FinanceSystem.cpp Account/Account.cpp Budgets/Budget.cpp
//       Transactions/transactions.cpp Goals/goals.cpp Investment/investment.cpp
//       DSA/Linked_list.cpp DSA/Sorting.cpp DSA/Searching.cpp DSA/Stack.cpp DSA/Queue.cpp
//       DSA/HashTable.cpp DSA/Heap.cpp -o server.exe -lws2_32
// Run from the project folder, then open http://localhost:8080

#include <winsock2.h>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "FinanceSystem.h"

using namespace std;

const int PORT = 8080;
const string DATA_FOLDER = "Data";
const size_t MAX_REQUEST_SIZE = 1000000;   // 1 MB: protects against huge requests
const int RECEIVE_TIMEOUT_MS = 3000;       // a silent browser connection cannot block us forever

// =========================== Small helpers ===========================

// Turns a number into text without extra zeros (350.75, not 350.750000).
string num(double value)
{
    ostringstream out;
    out.precision(15);
    out << value;
    return out.str();
}

// Makes text safe to put inside a JSON string: "  \  and control characters.
string jsonText(const string& text)
{
    string out = "\"";
    for (size_t i = 0; i < text.size(); i++)
    {
        char ch = text[i];
        if (ch == '"')       out += "\\\"";
        else if (ch == '\\') out += "\\\\";
        else if (ch == '\n') out += "\\n";
        else if (ch == '\r') out += "\\r";
        else if (ch == '\t') out += "\\t";
        else if (static_cast<unsigned char>(ch) < 0x20) out += ' ';
        else out += ch;
    }
    return out + "\"";
}

string lowerCase(string text)
{
    for (size_t i = 0; i < text.size(); i++)
        text[i] = static_cast<char>(tolower(static_cast<unsigned char>(text[i])));
    return text;
}

// Form data arrives URL-encoded: "Rent%2C+March" means "Rent, March".
string urlDecode(const string& text)
{
    string out;
    for (size_t i = 0; i < text.size(); i++)
    {
        if (text[i] == '+')
        {
            out += ' ';
        }
        else if (text[i] == '%' && i + 2 < text.size() &&
                 isxdigit(static_cast<unsigned char>(text[i + 1])) &&
                 isxdigit(static_cast<unsigned char>(text[i + 2])))
        {
            out += static_cast<char>(strtol(text.substr(i + 1, 2).c_str(), nullptr, 16));
            i += 2;
        }
        else
        {
            out += text[i];
        }
    }
    return out;
}

// The parameters of one request, e.g. amount=500&category=Food
struct Params
{
    static const int MAX = 20;
    string names[MAX];
    string values[MAX];
    int count = 0;

    void parse(const string& text)   // "a=1&b=2"
    {
        size_t start = 0;
        while (start <= text.size() && count < MAX)
        {
            size_t end = text.find('&', start);
            if (end == string::npos)
                end = text.size();
            string pair = text.substr(start, end - start);
            size_t equals = pair.find('=');
            if (!pair.empty())
            {
                names[count] = urlDecode(pair.substr(0, equals));
                values[count] = equals == string::npos ? "" : urlDecode(pair.substr(equals + 1));
                count++;
            }
            start = end + 1;
        }
    }

    string get(const string& name) const
    {
        for (int i = 0; i < count; i++)
            if (names[i] == name)
                return values[i];
        return "";
    }
};

// Text -> number. Fails on "", "abc" and "12abc".
bool toNumber(const string& text, double& value)
{
    istringstream in(text);
    char extra;
    return (in >> value) && !(in >> extra);
}

bool toWholeNumber(const string& text, int& value)
{
    istringstream in(text);
    char extra;
    return (in >> value) && !(in >> extra);
}

// ============================ HTTP replies ============================

struct Reply
{
    int status = 200;
    string contentType = "application/json";
    string body;
};

Reply jsonReply(const string& json, int status = 200)
{
    Reply reply;
    reply.status = status;
    reply.body = json;
    return reply;
}

Reply errorReply(const string& message, int status = 400)
{
    return jsonReply("{\"ok\":false,\"error\":" + jsonText(message) + "}", status);
}

// After every change the data is saved, so nothing is lost if the window is closed.
Reply savedReply(FinanceSystem& fs, const string& extraJson = "")
{
    string json = "{\"ok\":true";
    if (!extraJson.empty())
        json += "," + extraJson;
    if (!fs.saveData(DATA_FOLDER))
        json += ",\"warning\":\"Could not save: make sure the Data folder exists.\"";
    return jsonReply(json + "}");
}

// The type of a file, from its extension. Unknown types are not served.
string contentTypeFor(const string& path)
{
    size_t dot = path.rfind('.');
    string ext = dot == string::npos ? "" : lowerCase(path.substr(dot));
    if (ext == ".html") return "text/html; charset=utf-8";
    if (ext == ".js")   return "application/javascript; charset=utf-8";
    if (ext == ".css")  return "text/css; charset=utf-8";
    if (ext == ".svg")  return "image/svg+xml";
    if (ext == ".png")  return "image/png";
    if (ext == ".ico")  return "image/x-icon";
    if (ext == ".json") return "application/json";
    return "";
}

// Serves a file from the web/ folder (the built React app). Safety rules, so that
// nobody can read other files on the disk:
//   - only letters, digits and . _ - / are allowed in the path
//   - ".." is refused (it would climb out of web/)
//   - only the file types listed in contentTypeFor() are served
Reply fileReply(string path)
{
    if (path == "/")
        path = "/index.html";

    for (size_t i = 0; i < path.size(); i++)
    {
        char ch = path[i];
        if (!isalnum(static_cast<unsigned char>(ch)) && ch != '.' && ch != '_' && ch != '-' && ch != '/')
            return errorReply("Not found", 404);
    }
    string type = contentTypeFor(path);
    if (path.find("..") != string::npos || type.empty())
        return errorReply("Not found", 404);

    string file = "web" + path;
    ifstream in(file.c_str(), ios::binary);
    if (!in)
    {
        // A missing index.html usually means server.exe was started from the wrong folder.
        if (path == "/index.html")
            return errorReply("Missing web/index.html. Run server.exe from the project folder.", 500);
        return errorReply("Not found", 404);
    }

    stringstream content;
    content << in.rdbuf();
    Reply reply;
    reply.contentType = type;
    reply.body = content.str();
    return reply;
}

// ============================ JSON builders ============================

string transactionJson(const Transaction& t)
{
    return "{\"id\":" + to_string(t.getTransactionId()) +
           ",\"type\":" + jsonText(t.getType()) +
           ",\"amount\":" + num(t.getAmount()) +
           ",\"category\":" + jsonText(t.getCategory()) +
           ",\"date\":" + jsonText(t.getDate()) +
           ",\"account\":" + jsonText(t.getAccountName()) +
           ",\"description\":" + jsonText(t.getDescription()) + "}";
}

string transactionListJson(const vector<const Transaction*>& list)
{
    string json = "[";
    for (size_t i = 0; i < list.size(); i++)
        json += (i ? "," : "") + transactionJson(*list[i]);
    return json + "]";
}

string budgetJson(FinanceSystem& fs, int index)
{
    const Budget& b = fs.getBudget(index);
    TransactionList& list = fs.getTransactions();
    return "{\"category\":" + jsonText(b.getCategory()) +
           ",\"month\":" + to_string(b.getMonth()) + ",\"year\":" + to_string(b.getYear()) +
           ",\"limit\":" + num(b.getLimit()) +
           ",\"spent\":" + num(b.calculateSpent(list)) +
           ",\"remaining\":" + num(b.calculateRemaining(list)) +
           ",\"percent\":" + num(b.calculateUsagePercent(list)) + "}";
}

// After an expense: the budget it counts towards, or a note if the category's
// budget is for another month (same idea as the console's displayBudgetFor()).
string budgetAfterExpenseJson(FinanceSystem& fs, int transactionId)
{
    Transaction* t = fs.findTransaction(transactionId);
    if (t == nullptr || t->getType() != "Expense")
        return "";

    int index = fs.findBudgetIndexFor(t->getCategory(), t->getDate());
    if (index != -1)
        return "\"budget\":" + budgetJson(fs, index);

    string months;
    for (int i = 0; i < fs.getBudgetCount(); i++)
    {
        const Budget& b = fs.getBudget(i);
        if (lowerCase(b.getCategory()) == lowerCase(t->getCategory()))
            months += string(months.empty() ? "" : ", ") + (b.getMonth() < 10 ? "0" : "") +
                      to_string(b.getMonth()) + "-" + to_string(b.getYear());
    }
    if (months.empty())
        return "";

    return "\"budgetNote\":" + jsonText("There is no '" + t->getCategory() + "' budget for " +
           t->getDate().substr(3) + ", so this expense is not counted in a budget. Your '" +
           t->getCategory() + "' budget is for: " + months +
           " (a budget only counts expenses dated in its own month).");
}

// ============================== API ==============================
// GET requests only read. POST requests change data (and then save).

Reply handleApi(FinanceSystem& fs, const string& method, const string& path, const Params& p)
{
    bool post = (method == "POST");

    // ---------------- Dashboard ----------------
    if (path == "/api/dashboard" && !post)
    {
        const Account& a = fs.getAccount();
        double income = fs.getTotalIncome(), expense = fs.getTotalExpense();
        double invested = fs.getTotalInvested(), portfolio = fs.getPortfolioValue();

        // Spending per category for the chart (small arrays, case ignored).
        const int MAX_CATEGORIES = 500;
        string names[MAX_CATEGORIES];
        double totals[MAX_CATEGORIES];
        int categories = 0;
        for (TransactionNode* n = fs.getTransactions().getHead(); n != nullptr; n = n->next)
        {
            if (n->data.getType() != "Expense")
                continue;
            int k = 0;
            while (k < categories && lowerCase(names[k]) != lowerCase(n->data.getCategory()))
                k++;
            if (k == categories)
            {
                if (categories == MAX_CATEGORIES)
                    continue;
                names[k] = n->data.getCategory();
                totals[k] = 0;
                categories++;
            }
            totals[k] += n->data.getAmount();
        }

        string chart = "[";
        for (int k = 0; k < categories; k++)
            chart += (k ? "," : "") + string("{\"category\":") + jsonText(names[k]) +
                     ",\"amount\":" + num(totals[k]) + "}";
        chart += "]";

        string exceeded = "[";
        int found = 0;
        for (int i = 0; i < fs.getBudgetCount(); i++)
        {
            const Budget& b = fs.getBudget(i);
            if (b.calculateRemaining(fs.getTransactions()) < 0)
                exceeded += (found++ ? "," : "") + jsonText(b.getCategory() + " (" +
                            (b.getMonth() < 10 ? "0" : "") + to_string(b.getMonth()) + "-" +
                            to_string(b.getYear()) + ")");
        }
        exceeded += "]";

        return jsonReply("{\"ok\":true,\"cash\":" + num(a.getCashBalance()) +
                         ",\"bank\":" + num(a.getBankBalance()) +
                         ",\"total\":" + num(a.getTotalBalance()) +
                         ",\"income\":" + num(income) + ",\"expense\":" + num(expense) +
                         ",\"net\":" + num(income - expense) +
                         ",\"invested\":" + num(invested) + ",\"portfolio\":" + num(portfolio) +
                         ",\"profitLoss\":" + num(portfolio - invested) +
                         ",\"budgetCount\":" + to_string(fs.getBudgetCount()) +
                         ",\"goalCount\":" + to_string(fs.getGoalCount()) +
                         ",\"investmentCount\":" + to_string(fs.getInvestmentCount()) +
                         ",\"pendingCount\":" + to_string(fs.getPendingCount()) +
                         ",\"scheduledCount\":" + to_string(fs.getScheduledCount()) +
                         ",\"exceededBudgets\":" + exceeded +
                         ",\"spendingByCategory\":" + chart + "}");
    }

    if (path == "/api/balances" && post)
    {
        double cash, bank;
        if (!toNumber(p.get("cash"), cash) || !toNumber(p.get("bank"), bank))
            return errorReply("Cash and bank must be numbers.");
        if (!fs.setOpeningBalances(cash, bank))
            return errorReply("Balances cannot be negative.");
        return savedReply(fs);
    }

    // ---------------- Transactions ----------------
    if (path == "/api/transactions" && !post)
    {
        vector<const Transaction*> all;
        for (TransactionNode* n = fs.getTransactions().getHead(); n != nullptr; n = n->next)
            all.push_back(&n->data);
        return jsonReply("{\"ok\":true,\"canUndo\":" + string(fs.canUndo() ? "true" : "false") +
                         ",\"transactions\":" + transactionListJson(all) + "}");
    }

    if (path == "/api/transactions/search" && !post)
    {
        string by = p.get("by"), value = p.get("value");
        vector<const Transaction*> result;
        if (by == "category")
            result = fs.findByCategory(value);
        else if (by == "type")
            result = fs.findByType(value);
        else if (by == "id")
        {
            int id;
            if (!toWholeNumber(value, id))
                return errorReply("The ID must be a whole number.");
            Transaction* t = fs.findTransaction(id);   // hash table lookup
            if (t != nullptr)
                result.push_back(t);
        }
        else
            return errorReply("Search by category, type or id.");
        return jsonReply("{\"ok\":true,\"transactions\":" + transactionListJson(result) + "}");
    }

    if (path == "/api/transactions/add" && post)
    {
        double amount;
        if (!toNumber(p.get("amount"), amount))
            return errorReply("The amount must be a number.");
        int id = fs.addTransaction(p.get("type"), amount, p.get("category"), p.get("date"),
                                   p.get("account"), p.get("description"));
        if (id == -1)
            return errorReply("Not added. Check that: type is Income or Expense, amount is more than 0, "
                              "category is not empty, the date is a real date, account is Cash or Bank, "
                              "and the account has enough money for an expense.");
        string extra = "\"id\":" + to_string(id);
        string budget = budgetAfterExpenseJson(fs, id);
        return savedReply(fs, budget.empty() ? extra : extra + "," + budget);
    }

    if (path == "/api/transactions/delete" && post)
    {
        int id;
        if (!toWholeNumber(p.get("id"), id) || !fs.deleteTransaction(id))
            return errorReply("Not deleted: there is no such ID, or removing this income would "
                              "make the account balance negative.");
        return savedReply(fs);
    }

    if (path == "/api/transactions/undo" && post)
    {
        if (!fs.canUndo())
            return errorReply("Nothing to undo.");
        if (!fs.undoLastAction())
            return errorReply("Cannot undo: it would make the account balance negative.");
        return savedReply(fs);
    }

    if (path == "/api/transactions/sort" && post)
    {
        string by = p.get("by");
        if (by == "amount")        fs.sortTransactionsByAmount();
        else if (by == "category") fs.sortTransactionsByCategory();
        else if (by == "date")     fs.sortTransactionsByDate();
        else return errorReply("Sort by amount, category or date.");
        return savedReply(fs);
    }

    // ---------------- Budgets ----------------
    if (path == "/api/budgets" && !post)
    {
        string json = "[";
        for (int i = 0; i < fs.getBudgetCount(); i++)
            json += (i ? "," : "") + budgetJson(fs, i);
        return jsonReply("{\"ok\":true,\"budgets\":" + json + "]}");
    }

    if ((path == "/api/budgets/add" || path == "/api/budgets/update") && post)
    {
        double limit;
        int month, year;
        if (!toNumber(p.get("limit"), limit) || !toWholeNumber(p.get("month"), month) ||
            !toWholeNumber(p.get("year"), year))
            return errorReply("Limit, month and year must be numbers.");

        if (path == "/api/budgets/add")
        {
            if (!fs.addBudget(p.get("category"), limit, month, year))
                return errorReply("Not added. The limit must be more than 0, month 1-12, and there can be "
                                  "only one budget per category per month (maximum " +
                                  to_string(FinanceSystem::MAX_BUDGETS) + " budgets).");
        }
        else if (!fs.updateBudget(p.get("category"), month, year, limit))
            return errorReply("Not updated: no such budget, or the limit is not more than 0.");
        return savedReply(fs);
    }

    // ---------------- Goals ----------------
    if (path == "/api/goals" && !post)
    {
        string json = "[";
        for (int i = 0; i < fs.getGoalCount(); i++)
        {
            const Goal& g = fs.getGoal(i);
            json += (i ? "," : "") + string("{\"number\":") + to_string(i + 1) +
                    ",\"id\":" + to_string(g.getGoalId()) + ",\"name\":" + jsonText(g.getGoalName()) +
                    ",\"target\":" + num(g.getTargetAmount()) + ",\"savings\":" + num(g.getCurrentSavings()) +
                    ",\"monthly\":" + num(g.getMonthlySaving()) + ",\"date\":" + jsonText(g.getTargetDate()) +
                    ",\"progress\":" + num(g.calculateProgress()) + ",\"remaining\":" + num(g.calculateRemaining()) +
                    ",\"months\":" + to_string(g.calculateRequiredMonths()) + "}";
        }
        return jsonReply("{\"ok\":true,\"goals\":" + json + "]}");
    }

    if (path == "/api/goals/add" && post)
    {
        int id;
        double target, savings, monthly;
        if (!toWholeNumber(p.get("id"), id) || !toNumber(p.get("target"), target) ||
            !toNumber(p.get("savings"), savings) || !toNumber(p.get("monthly"), monthly))
            return errorReply("ID, target, savings and monthly saving must be numbers.");
        if (!fs.addGoalValues(id, p.get("name"), target, savings, monthly, p.get("date")))
            return errorReply("Not added. The ID must be positive and not already used, the name not empty, "
                              "target more than 0, savings and monthly saving 0 or more, and the date a "
                              "real date (maximum " + to_string(FinanceSystem::MAX_GOALS) + " goals).");
        return savedReply(fs);
    }

    if (path == "/api/goals/update" && post)
    {
        int number;
        double savings, monthly;
        if (!toWholeNumber(p.get("number"), number) || !toNumber(p.get("savings"), savings) ||
            !toNumber(p.get("monthly"), monthly))
            return errorReply("Goal number, savings and monthly saving must be numbers.");
        if (!fs.updateGoalValues(number, savings, monthly))
            return errorReply("Not updated: no goal with that number, or a value is negative.");
        return savedReply(fs);
    }

    // ---------------- Investments ----------------
    if (path == "/api/investments" && !post)
    {
        string json = "[";
        for (int i = 0; i < fs.getInvestmentCount(); i++)
        {
            const Investment* inv = fs.getInvestment(i);
            json += (i ? "," : "") + string("{\"number\":") + to_string(i + 1) +
                    ",\"type\":" + jsonText(inv->getType()) +   // virtual call: polymorphism
                    ",\"name\":" + jsonText(inv->getName()) +
                    ",\"invested\":" + num(inv->getAmountInvested()) +
                    ",\"current\":" + num(inv->getCurrentValue()) +
                    ",\"quantity\":" + num(inv->getQuantity()) +
                    ",\"profitLoss\":" + num(inv->profitLoss()) +
                    ",\"percent\":" + num(inv->profitLossPercent()) + "}";
        }
        return jsonReply("{\"ok\":true,\"investments\":" + json + "]}");
    }

    if (path == "/api/investments/add" && post)
    {
        double invested, current, quantity;
        if (!toNumber(p.get("invested"), invested) || !toNumber(p.get("current"), current) ||
            !toNumber(p.get("quantity"), quantity))
            return errorReply("Amounts and quantity must be numbers.");
        if (!fs.addInvestment(p.get("type"), p.get("name"), invested, current, quantity))
            return errorReply("Not added. Choose a valid type; the name must not be empty, amount invested "
                              "and quantity more than 0, and current value not negative (maximum " +
                              to_string(FinanceSystem::MAX_INVESTMENTS) + ").");
        return savedReply(fs);
    }

    if (path == "/api/investments/update" && post)
    {
        int number;
        double value;
        if (!toWholeNumber(p.get("number"), number) || !toNumber(p.get("value"), value))
            return errorReply("Investment number and value must be numbers.");
        if (!fs.updateInvestmentValue(number, value))
            return errorReply("Not updated: no investment with that number, or the value is negative.");
        return savedReply(fs);
    }

    // ---------------- Pending queue ----------------
    if (path == "/api/pending" && !post)
    {
        TransactionQueue copy = fs.getPendingCopy();   // emptying the COPY keeps the real queue
        Transaction t;
        string json = "[";
        for (int position = 1; copy.dequeue(t); position++)
            json += (position > 1 ? "," : "") + string("{\"position\":") + to_string(position) +
                    ",\"type\":" + jsonText(t.getType()) + ",\"amount\":" + num(t.getAmount()) +
                    ",\"category\":" + jsonText(t.getCategory()) + ",\"date\":" + jsonText(t.getDate()) +
                    ",\"account\":" + jsonText(t.getAccountName()) +
                    ",\"description\":" + jsonText(t.getDescription()) + "}";
        return jsonReply("{\"ok\":true,\"pending\":" + json + "]}");
    }

    if (path == "/api/pending/add" && post)
    {
        double amount;
        if (!toNumber(p.get("amount"), amount))
            return errorReply("The amount must be a number.");
        if (!fs.addPendingTransaction(p.get("type"), amount, p.get("category"), p.get("date"),
                                      p.get("account"), p.get("description")))
            return errorReply("Not added: check the values (or the queue is full: " +
                              to_string(TransactionQueue::CAPACITY) + " items).");
        return savedReply(fs);
    }

    if (path == "/api/pending/process" && post)
    {
        if (fs.getPendingCount() == 0)
            return errorReply("No pending transactions.");
        int id = fs.processNextPending();
        if (id == -1)
            return errorReply("Could not process: not enough money in the account. "
                              "It stays at the front of the queue.");
        string extra = "\"id\":" + to_string(id);
        string budget = budgetAfterExpenseJson(fs, id);
        return savedReply(fs, budget.empty() ? extra : extra + "," + budget);
    }

    // ---------------- Scheduled bills (min-heap) ----------------
    if (path == "/api/scheduled" && !post)
    {
        string today = FinanceSystem::todayDate();
        Transaction todayHolder;
        todayHolder.setDate(today);
        int todayKey = todayHolder.getDateKey();

        // Emptying a COPY of the heap gives the bills in due-date order (heap sort).
        TransactionHeap copy = fs.getScheduledCopy();
        Transaction t;
        string json = "[";
        for (int position = 1; !copy.isEmpty(); position++)
        {
            int sequence = copy.peekMinSequence();   // the bill's number, used to release just this one
            copy.removeMin(t);
            json += (position > 1 ? "," : "") + string("{\"position\":") + to_string(position) +
                    ",\"seq\":" + to_string(sequence) +
                    ",\"type\":" + jsonText(t.getType()) + ",\"amount\":" + num(t.getAmount()) +
                    ",\"category\":" + jsonText(t.getCategory()) + ",\"date\":" + jsonText(t.getDate()) +
                    ",\"account\":" + jsonText(t.getAccountName()) +
                    ",\"description\":" + jsonText(t.getDescription()) +
                    ",\"due\":" + (t.getDateKey() <= todayKey ? "true" : "false") + "}";
        }
        return jsonReply("{\"ok\":true,\"today\":" + jsonText(today) + ",\"scheduled\":" + json + "]}");
    }

    if (path == "/api/scheduled/release" && post)
    {
        int sequence;
        if (!toWholeNumber(p.get("seq"), sequence))
            return errorReply("Choose a bill to release.");
        string date = p.get("date").empty() ? FinanceSystem::todayDate() : p.get("date");

        switch (fs.releaseScheduled(sequence, date))
        {
        case FinanceSystem::RELEASED:
            return savedReply(fs);
        case FinanceSystem::NO_SUCH_BILL:
            return errorReply("That bill is no longer scheduled. The list has been refreshed.");
        case FinanceSystem::NOT_DUE_YET:
            return errorReply("That bill is not due on or before " + date + ", so it cannot be released yet.");
        case FinanceSystem::QUEUE_FULL:
            return errorReply("The ready queue is full (" + to_string(TransactionQueue::CAPACITY) + " bills).");
        default:
            return errorReply("The release date must be a real DD-MM-YYYY date.");
        }
    }

    if (path == "/api/scheduled/add" && post)
    {
        double amount;
        if (!toNumber(p.get("amount"), amount))
            return errorReply("The amount must be a number.");
        if (!fs.addScheduledTransaction(p.get("type"), amount, p.get("category"), p.get("date"),
                                        p.get("account"), p.get("description")))
            return errorReply("Not added: check the values (or " + to_string(TransactionHeap::CAPACITY) +
                              " bills are already scheduled).");
        return savedReply(fs);
    }

    if (path == "/api/pending/release" && post)
    {
        string date = p.get("date").empty() ? FinanceSystem::todayDate() : p.get("date");
        int released = fs.releaseDue(date);
        if (released == -1)
            return errorReply("The release date must be a real DD-MM-YYYY date.");
        bool queueFull = fs.getPendingCount() == TransactionQueue::CAPACITY && fs.getScheduledCount() > 0;
        return savedReply(fs, "\"released\":" + to_string(released) + ",\"date\":" + jsonText(date) +
                              ",\"queueFull\":" + (queueFull ? "true" : "false"));
    }

    if (path == "/api/pending/move-to-back" && post)
    {
        if (!fs.moveFrontToBack())
            return errorReply("Nothing to move: the ready queue needs at least 2 bills.");
        return savedReply(fs);
    }

    return errorReply("Unknown request: " + method + " " + path, 404);
}

// ========================= Reading one request =========================

// Finds a header value, e.g. headerValue(head, "content-length"). Case is ignored.
string headerValue(const string& head, const string& name)
{
    string lowerHead = lowerCase(head);
    size_t pos = lowerHead.find("\r\n" + name + ":");
    if (pos == string::npos)
        return "";
    pos += name.size() + 3;
    size_t end = head.find("\r\n", pos);
    string value = head.substr(pos, end - pos);
    size_t first = value.find_first_not_of(' ');
    return first == string::npos ? "" : value.substr(first);
}

// Reads the whole request: the header part, then the body (Content-Length bytes).
bool receiveRequest(SOCKET client, string& head, string& body)
{
    string data;
    char buffer[4096];
    size_t headerEnd;

    while ((headerEnd = data.find("\r\n\r\n")) == string::npos)
    {
        int got = recv(client, buffer, sizeof(buffer), 0);
        if (got <= 0 || data.size() > MAX_REQUEST_SIZE)
            return false;
        data.append(buffer, got);
    }

    head = data.substr(0, headerEnd);
    body = data.substr(headerEnd + 4);

    size_t length = strtoul(headerValue(head, "content-length").c_str(), nullptr, 10);
    if (length > MAX_REQUEST_SIZE)
        return false;
    while (body.size() < length)
    {
        int got = recv(client, buffer, sizeof(buffer), 0);
        if (got <= 0)
            return false;
        body.append(buffer, got);
    }
    body = body.substr(0, length);
    return true;
}

Reply handleRequest(FinanceSystem& fs, const string& head, const string& body)
{
    // First line, e.g.  "POST /api/transactions/add HTTP/1.1"
    istringstream firstLine(head.substr(0, head.find("\r\n")));
    string method, target;
    firstLine >> method >> target;

    string path = target, query;
    size_t questionMark = target.find('?');
    if (questionMark != string::npos)
    {
        path = target.substr(0, questionMark);
        query = target.substr(questionMark + 1);
    }

    // Security 1: only answer requests addressed to THIS computer (stops "DNS rebinding").
    string host = headerValue(head, "host");
    if (host != "localhost:" + to_string(PORT) && host != "127.0.0.1:" + to_string(PORT))
        return errorReply("Wrong host", 403);

    if (path.compare(0, 5, "/api/") != 0)
    {
        if (method != "GET")
            return errorReply("Method not allowed", 405);
        return fileReply(path);
    }

    // Security 2: our page adds this header to every request. A different website open in
    // the browser cannot add it without our permission, so it cannot change the data (CSRF).
    if (headerValue(head, "x-finance-app") != "1")
        return errorReply("Requests must come from the Finance Manager page.", 403);

    if (method != "GET" && method != "POST")
        return errorReply("Method not allowed", 405);

    Params params;
    params.parse(method == "POST" ? body : query);
    return handleApi(fs, method, path, params);
}

void sendReply(SOCKET client, const Reply& reply)
{
    string statusText = reply.status == 200 ? "OK" : reply.status == 400 ? "Bad Request" :
                        reply.status == 403 ? "Forbidden" : reply.status == 404 ? "Not Found" :
                        reply.status == 405 ? "Method Not Allowed" : "Error";
    string message = "HTTP/1.1 " + to_string(reply.status) + " " + statusText + "\r\n" +
                     "Content-Type: " + reply.contentType + "\r\n" +
                     "Content-Length: " + to_string(reply.body.size()) + "\r\n" +
                     "Cache-Control: no-store\r\n" +
                     "Connection: close\r\n\r\n" + reply.body;

    size_t sent = 0;
    while (sent < message.size())
    {
        int n = send(client, message.c_str() + sent, static_cast<int>(message.size() - sent), 0);
        if (n <= 0)
            break;
        sent += n;
    }
}

// ================================ Main ================================

int main(int argc, char* argv[])
{
    // "server.exe --no-browser" starts without opening the browser (used for testing).
    bool openBrowser = !(argc > 1 && string(argv[1]) == "--no-browser");

    // static: stored in the program's data area, not on the (small) stack,
    // because FinanceSystem holds large fixed-size arrays (about 180 KB).
    static FinanceSystem fs;
    if (!fs.loadData(DATA_FOLDER))
        cout << "Warning: some saved records were damaged and were skipped.\n";

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cout << "ERROR: could not start Windows networking.\n";
        return 1;
    }

    SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = inet_addr("127.0.0.1");   // this computer only

    if (listener == INVALID_SOCKET ||
        bind(listener, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR ||
        listen(listener, SOMAXCONN) == SOCKET_ERROR)
    {
        cout << "ERROR: could not use port " << PORT << ". Is server.exe already running?\n";
        WSACleanup();
        return 1;
    }

    cout << "=============================================\n"
         << "  FINANCE MANAGER - WEB VERSION\n"
         << "  Open  http://localhost:" << PORT << "  in your browser.\n"
         << "  Data is saved after every change.\n"
         << "  To stop: close this window (or press Ctrl+C).\n"
         << "=============================================\n";

    if (openBrowser)
        system(("start \"\" http://localhost:" + to_string(PORT)).c_str());   // open the browser

    // One request at a time: accept, read, answer, close.
    while (true)
    {
        SOCKET client = accept(listener, nullptr, nullptr);
        if (client == INVALID_SOCKET)
            continue;

        DWORD timeout = RECEIVE_TIMEOUT_MS;
        setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));

        string head, body;
        if (receiveRequest(client, head, body))
            sendReply(client, handleRequest(fs, head, body));

        closesocket(client);
    }
}
