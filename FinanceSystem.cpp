#include "FinanceSystem.h"
#include "DSA/Sorting.h"
#include "DSA/Searching.h"
#include <iostream>
#include <iomanip>
#include <cctype>
#include <fstream>
#include <sstream>
#include <ctime>

using namespace std;

namespace
{
    string toLower(string text)
    {
        for (size_t i = 0; i < text.size(); ++i)
            text[i] = static_cast<char>(tolower(static_cast<unsigned char>(text[i])));
        return text;
    }

    // Converts user text to the exact spelling the modules expect.
    // Returns "" if the text is not recognised.
    string normalizeType(const string& type)
    {
        string t = toLower(type);
        if (t == "income")  return "Income";
        if (t == "expense") return "Expense";
        return "";
    }

    string normalizeAccount(const string& accountName)
    {
        string a = toLower(accountName);
        if (a == "cash") return "Cash";
        if (a == "bank") return "Bank";
        return "";
    }
}

FinanceSystem::FinanceSystem()
{
    budgetCount = 0;
    goalCount = 0;
    investmentCount = 0;
    nextTransactionId = 1;

    for (int i = 0; i < MAX_INVESTMENTS; i++)
        investments[i] = nullptr;
}

FinanceSystem::~FinanceSystem()
{
    // delete through a base-class pointer is safe because
    // Investment has a virtual destructor.
    for (int i = 0; i < investmentCount; i++)
        delete investments[i];
}

// ======================= Account =======================

bool FinanceSystem::setOpeningBalances(double cash, double bank)
{
    if (cash < 0 || bank < 0)
        return false;

    account.setBalances(cash, bank);
    return true;
}

const Account& FinanceSystem::getAccount() const
{
    return account;
}

// ===================== Transactions =====================

// Checks the user's input and converts type/account to the exact spelling
// the modules expect ("Income"/"Expense", "Cash"/"Bank").
// Used by both addTransaction() and addPendingTransaction().
bool FinanceSystem::cleanTransactionInput(string& type, double amount, const string& category,
                                          const string& date, string& accountName) const
{
    type = normalizeType(type);
    accountName = normalizeAccount(accountName);

    if (type.empty() || accountName.empty())
        return false;
    if (amount <= 0 || category.empty())
        return false;
    if (!Transaction::isValidDate(date))
        return false;

    return true;
}

// PRIVATE helper: puts a transaction into the system (balance + list + hash table).
// Does NOT touch the undo stack, so undo can call it without recording itself.
bool FinanceSystem::insertTransaction(const Transaction& transaction)
{
    int id = transaction.getTransactionId();

    // The hash table also protects against duplicate IDs in O(1).
    if (idIndex.contains(id))
        return false;

    // Update the balance first: if an expense cannot be paid, nothing is recorded.
    bool balanceUpdated;
    if (transaction.getType() == "Income")
        balanceUpdated = account.deposit(transaction.getAccountName(), transaction.getAmount());
    else
        balanceUpdated = account.withdraw(transaction.getAccountName(), transaction.getAmount());

    if (!balanceUpdated)
        return false;

    // insert() returns a pointer to the copy that now lives inside the linked
    // list; the hash table stores it. No search needed, so adding is O(1).
    idIndex.insert(id, transactions.insert(transaction));
    return true;
}

// PRIVATE helper: takes a transaction out of the system (balance + list + hash table).
// Does NOT touch the undo stack.
bool FinanceSystem::removeTransaction(int transactionId)
{
    Transaction* transaction = idIndex.find(transactionId);   // O(1) average
    if (transaction == nullptr)
        return false;

    // Reverse the original effect on the balance.
    bool reversed;
    if (transaction->getType() == "Income")
        reversed = account.withdraw(transaction->getAccountName(), transaction->getAmount());
    else
        reversed = account.deposit(transaction->getAccountName(), transaction->getAmount());

    if (!reversed)
        return false;

    // Remove from the hash table FIRST: after transactions.remove() the
    // pointer stored in the table would point to deleted memory.
    idIndex.remove(transactionId);
    return transactions.remove(transactionId);
}

int FinanceSystem::addTransaction(const string& type, double amount, const string& category,
                                  const string& date, const string& accountName,
                                  const string& description)
{
    string cleanType = type;
    string cleanAccount = accountName;
    if (!cleanTransactionInput(cleanType, amount, category, date, cleanAccount))
        return -1;

    Transaction transaction(nextTransactionId, cleanType, amount, category,
                            date, cleanAccount, description);
    if (!insertTransaction(transaction))
        return -1;

    nextTransactionId++;

    // Remember the action so it can be undone.
    UndoAction action;
    action.type = UndoAction::ADD_TRANSACTION;
    action.transaction = transaction;
    undoStack.push(action);

    return transaction.getTransactionId();
}

bool FinanceSystem::deleteTransaction(int transactionId)
{
    Transaction* transaction = idIndex.find(transactionId);
    if (transaction == nullptr)
        return false;

    // Copy it BEFORE removing, so undo can restore it later.
    UndoAction action;
    action.type = UndoAction::DELETE_TRANSACTION;
    action.transaction = *transaction;

    if (!removeTransaction(transactionId))
        return false;

    undoStack.push(action);
    return true;
}

Transaction* FinanceSystem::findTransaction(int transactionId)
{
    return idIndex.find(transactionId);   // hash table: O(1) average instead of O(n)
}

// ========================= Undo =========================

bool FinanceSystem::undoLastAction()
{
    UndoAction action;
    if (!undoStack.peek(action))
        return false;   // nothing to undo

    bool undone;
    if (action.type == UndoAction::ADD_TRANSACTION)
        undone = removeTransaction(action.transaction.getTransactionId());   // undo add = remove it
    else
        undone = insertTransaction(action.transaction);                      // undo delete = put it back

    // Only pop if the undo worked, so a failed undo is not lost.
    if (undone)
        undoStack.pop(action);

    return undone;
}

bool FinanceSystem::canUndo() const
{
    return !undoStack.isEmpty();
}

// =================== Pending (Queue) ===================

bool FinanceSystem::addPendingTransaction(const string& type, double amount, const string& category,
                                          const string& date, const string& accountName,
                                          const string& description)
{
    string cleanType = type;
    string cleanAccount = accountName;
    if (!cleanTransactionInput(cleanType, amount, category, date, cleanAccount))
        return false;

    // ID 0 = "not assigned yet"; the real ID is given when it is processed.
    return pending.enqueue(Transaction(0, cleanType, amount, category,
                                       date, cleanAccount, description));
}

int FinanceSystem::processNextPending()
{
    Transaction next;
    if (!pending.peek(next))
        return -1;   // queue is empty

    int id = addTransaction(next.getType(), next.getAmount(), next.getCategory(),
                            next.getDate(), next.getAccountName(), next.getDescription());

    // Only remove it from the queue if it was recorded successfully
    // (e.g. it stays in the queue if the balance was too low).
    if (id != -1)
        pending.dequeue(next);

    return id;
}

void FinanceSystem::displayPending() const
{
    pending.display();
}

int FinanceSystem::getPendingCount() const
{
    return pending.size();
}

// ============ Scheduled bills (heap) + ready queue ============

bool FinanceSystem::addScheduledTransaction(const string& type, double amount, const string& category,
                                            const string& date, const string& accountName,
                                            const string& description)
{
    string cleanType = type;
    string cleanAccount = accountName;
    if (!cleanTransactionInput(cleanType, amount, category, date, cleanAccount))
        return false;

    // ID 0 = "not assigned yet"; the real ID is given when it is finally processed.
    return scheduled.insert(Transaction(0, cleanType, amount, category,
                                        date, cleanAccount, description));
}

int FinanceSystem::releaseDue(const string& date)
{
    if (!Transaction::isValidDate(date))
        return -1;

    Transaction dateHolder;
    dateHolder.setDate(date);
    int limit = dateHolder.getDateKey();   // YYYYMMDD

    // The heap always has the EARLIEST bill on top, so we can stop at the first
    // bill that is not due yet: every bill below it is due even later.
    int released = 0;
    while (!scheduled.isEmpty() && scheduled.peekMinKey() <= limit && !pending.isFull())
    {
        Transaction bill;
        scheduled.removeMin(bill);   // O(log n)
        pending.enqueue(bill);       // O(1)
        released++;
    }
    return released;
}

FinanceSystem::ReleaseResult FinanceSystem::releaseScheduled(int sequence, const string& date)
{
    if (!Transaction::isValidDate(date))
        return INVALID_DATE;

    Transaction bill;
    if (!scheduled.findBySequence(sequence, bill))
        return NO_SUCH_BILL;

    Transaction dateHolder;
    dateHolder.setDate(date);
    if (bill.getDateKey() > dateHolder.getDateKey())
        return NOT_DUE_YET;   // only due bills may be released

    if (pending.isFull())
        return QUEUE_FULL;

    scheduled.removeBySequence(sequence, bill);   // O(n) find + O(log n) repair
    pending.enqueue(bill);                         // joins the BACK of the ready queue
    return RELEASED;
}

bool FinanceSystem::moveFrontToBack()
{
    if (pending.size() < 2)
        return false;   // with 0 or 1 bills, nothing would change

    Transaction front;
    pending.dequeue(front);   // take it off the front...
    pending.enqueue(front);   // ...and join the back of the line
    return true;
}

TransactionHeap FinanceSystem::getScheduledCopy() const
{
    return scheduled;
}

int FinanceSystem::getScheduledCount() const
{
    return scheduled.size();
}

void FinanceSystem::displayScheduled() const
{
    if (scheduled.isEmpty())
    {
        cout << "No scheduled bills.\n";
        return;
    }

    // Removing the minimum again and again from a COPY gives the bills in date
    // order (this is heap sort, O(n log n)). The real heap is not changed.
    TransactionHeap copy = scheduled;
    Transaction bill;
    for (int position = 1; copy.removeMin(bill); position++)
    {
        cout << "[Scheduled " << position << "]\n";
        bill.displayTransaction();
        cout << "-----------------------------\n";
    }
}

string FinanceSystem::todayDate()
{
    time_t now = time(nullptr);
    tm* local = localtime(&now);
    ostringstream out;
    out << setfill('0') << setw(2) << local->tm_mday << '-'
        << setw(2) << (local->tm_mon + 1) << '-' << (local->tm_year + 1900);
    return out.str();
}

void FinanceSystem::displayTransactions() const
{
    transactions.display();
}

TransactionList& FinanceSystem::getTransactions()
{
    return transactions;
}

int FinanceSystem::searchByCategory(const string& category)
{
    return Searching::searchByCategory(transactions, category);
}

int FinanceSystem::searchByType(const string& type)
{
    return Searching::searchByType(transactions, type);
}

// Sorting swaps the data between list nodes, so the hash table's pointers
// would point at the wrong transactions afterwards. rebuild() fixes that.
void FinanceSystem::sortTransactionsByAmount()
{
    Sorting::sortByAmount(transactions);
    idIndex.rebuild(transactions);
}

void FinanceSystem::sortTransactionsByCategory()
{
    Sorting::sortByCategory(transactions);
    idIndex.rebuild(transactions);
}

void FinanceSystem::sortTransactionsByDate()
{
    Sorting::sortByDate(transactions);
    idIndex.rebuild(transactions);
}

// ======================== Budgets ========================

int FinanceSystem::findBudgetIndex(const string& category, int month, int year) const
{
    for (int i = 0; i < budgetCount; i++)
    {
        if (toLower(budgets[i].getCategory()) == toLower(category) &&
            budgets[i].getMonth() == month && budgets[i].getYear() == year)
            return i;
    }
    return -1;
}

bool FinanceSystem::addBudget(const string& category, double limit, int month, int year)
{
    if (budgetCount >= MAX_BUDGETS)
        return false;

    // Only one budget per category per month.
    if (findBudgetIndex(category, month, year) != -1)
        return false;

    // setBudget validates the values; only count the slot if it succeeded.
    if (!budgets[budgetCount].setBudget(category, limit, month, year))
        return false;

    budgetCount++;
    return true;
}

bool FinanceSystem::updateBudget(const string& category, int month, int year, double newLimit)
{
    int index = findBudgetIndex(category, month, year);
    if (index == -1)
        return false;

    return budgets[index].updateBudget(newLimit);
}

void FinanceSystem::displayBudgets() const
{
    if (budgetCount == 0)
    {
        cout << "No budgets created.\n";
        return;
    }

    for (int i = 0; i < budgetCount; i++)
        budgets[i].displayBudget(transactions);
}

bool FinanceSystem::displayBudgetFor(const string& category, const string& date) const
{
    // Budgets are monthly, so find the month and year of this date.
    Transaction temp;
    temp.setDate(date);
    int dateKey = temp.getDateKey();     // YYYYMMDD, 0 if the date is invalid
    if (dateKey == 0)
        return false;

    int month = (dateKey / 100) % 100;
    int year = dateKey / 10000;

    int index = findBudgetIndex(category, month, year);
    if (index != -1)
    {
        budgets[index].displayBudget(transactions);   // shows spent / remaining / warning
        return true;
    }

    // No budget for this month. If the category has a budget in ANOTHER month,
    // explain why this expense is not counted, so the user is not confused.
    // (If the category has no budget at all, nothing is printed.)
    bool otherMonths = false;
    for (int i = 0; i < budgetCount; i++)
    {
        if (toLower(budgets[i].getCategory()) == toLower(category))
        {
            if (!otherMonths)
                cout << "Note: there is no '" << category << "' budget for "
                     << setfill('0') << setw(2) << month << setfill(' ') << "-" << year
                     << ", so this expense is not counted in a budget.\n"
                     << "Your '" << budgets[i].getCategory() << "' budget is for: ";
            else
                cout << ", ";
            cout << setfill('0') << setw(2) << budgets[i].getMonth() << setfill(' ')
                 << "-" << budgets[i].getYear();
            otherMonths = true;
        }
    }
    if (otherMonths)
        cout << " (a budget only counts expenses dated in its own month).\n";

    return false;
}

// ========================= Goals =========================

bool FinanceSystem::addGoal()
{
    if (goalCount >= MAX_GOALS)
        return false;

    goals[goalCount].createGoal();

    // If the input closed half-way (e.g. Ctrl+Z), createGoal() stops early and
    // the goal is only half filled. Count it only if it is complete and valid.
    if (!isCompleteGoal(goals[goalCount]))
        return false;   // not counted: the slot is simply reused next time

    goalCount++;
    return true;
}

bool FinanceSystem::updateGoal(int goalNumber)
{
    if (goalNumber < 1 || goalNumber > goalCount)
        return false;

    // Keep a copy: if the input closes half-way, a failed read leaves 0 in the
    // goal, so we put the old values back instead of saving wrong data.
    Goal before = goals[goalNumber - 1];
    goals[goalNumber - 1].updateGoal();

    if (!isCompleteGoal(goals[goalNumber - 1]))
    {
        goals[goalNumber - 1] = before;
        return false;
    }
    return true;
}

// True if the last keyboard read worked AND every value of the goal is valid.
// cin.fail() is true when a read failed (e.g. the input closed half-way).
// setGoal() re-checks all values; given the goal's own values it changes nothing.
bool FinanceSystem::isCompleteGoal(Goal& goal) const
{
    if (cin.fail())
        return false;

    return goal.setGoal(goal.getGoalId(), goal.getGoalName(), goal.getTargetAmount(),
                        goal.getCurrentSavings(), goal.getMonthlySaving(), goal.getTargetDate());
}

void FinanceSystem::displayGoals() const
{
    if (goalCount == 0)
    {
        cout << "No goals created.\n";
        return;
    }

    for (int i = 0; i < goalCount; i++)
    {
        cout << "\n[Goal " << (i + 1) << "]\n";
        goals[i].displayGoal();
    }
}

// ====================== Investments ======================

bool FinanceSystem::addInvestment(const string& type, const string& name,
                                  double amountInvested, double currentValue, double quantity)
{
    if (investmentCount >= MAX_INVESTMENTS)
        return false;
    if (!Investment::isValidInput(name, amountInvested, currentValue, quantity))
        return false;

    string t = toLower(type);
    Investment* investment = nullptr;

    if (t == "stock")
        investment = new Stock(name, amountInvested, currentValue, quantity);
    else if (t == "mutual fund" || t == "mutualfund")
        investment = new MutualFund(name, amountInvested, currentValue, quantity);
    else if (t == "fixed deposit" || t == "fd")
        investment = new FixedDeposit(name, amountInvested, currentValue, quantity);
    else if (t == "gold")
        investment = new Gold(name, amountInvested, currentValue, quantity);
    else if (t == "other")
        investment = new OtherInvestment(name, amountInvested, currentValue, quantity);
    else
        return false;

    investments[investmentCount++] = investment;
    return true;
}

bool FinanceSystem::updateInvestmentValue(int investmentNumber, double newValue)
{
    if (investmentNumber < 1 || investmentNumber > investmentCount)
        return false;

    return investments[investmentNumber - 1]->updateCurrentValue(newValue);
}

void FinanceSystem::displayInvestments() const
{
    if (investmentCount == 0)
    {
        cout << "No investments added.\n";
        return;
    }

    for (int i = 0; i < investmentCount; i++)
    {
        cout << "\n[Investment " << (i + 1) << "]";
        investments[i]->display();   // runtime polymorphism: calls Stock/Gold/... version
    }
}

// ================= Read-only access (web server) =================

int FinanceSystem::getBudgetCount() const
{
    return budgetCount;
}

const Budget& FinanceSystem::getBudget(int index) const
{
    return budgets[index];
}

int FinanceSystem::getGoalCount() const
{
    return goalCount;
}

const Goal& FinanceSystem::getGoal(int index) const
{
    return goals[index];
}

int FinanceSystem::getInvestmentCount() const
{
    return investmentCount;
}

const Investment* FinanceSystem::getInvestment(int index) const
{
    return investments[index];
}

TransactionQueue FinanceSystem::getPendingCopy() const
{
    return pending;
}

vector<const Transaction*> FinanceSystem::findByCategory(const string& category) const
{
    vector<const Transaction*> matches;
    for (TransactionNode* node = transactions.getHead(); node != nullptr; node = node->next)
    {
        if (toLower(node->data.getCategory()) == toLower(category))
            matches.push_back(&node->data);
    }
    return matches;
}

vector<const Transaction*> FinanceSystem::findByType(const string& type) const
{
    vector<const Transaction*> matches;
    for (TransactionNode* node = transactions.getHead(); node != nullptr; node = node->next)
    {
        if (toLower(node->data.getType()) == toLower(type))
            matches.push_back(&node->data);
    }
    return matches;
}

int FinanceSystem::findBudgetIndexFor(const string& category, const string& date) const
{
    Transaction temp;
    temp.setDate(date);
    int dateKey = temp.getDateKey();   // YYYYMMDD, 0 if the date is invalid
    if (dateKey == 0)
        return -1;

    return findBudgetIndex(category, (dateKey / 100) % 100, dateKey / 10000);
}

bool FinanceSystem::addGoalValues(int id, const string& name, double target, double savings,
                                  double monthly, const string& date)
{
    if (goalCount >= MAX_GOALS || !Transaction::isValidDate(date))
        return false;

    for (int i = 0; i < goalCount; i++)
    {
        if (goals[i].getGoalId() == id)
            return false;   // goal ID already used
    }

    // setGoal() checks the other values and changes nothing if one is invalid.
    if (!goals[goalCount].setGoal(id, name, target, savings, monthly, date))
        return false;

    goalCount++;
    return true;
}

bool FinanceSystem::updateGoalValues(int goalNumber, double savings, double monthly)
{
    if (goalNumber < 1 || goalNumber > goalCount)
        return false;

    Goal& goal = goals[goalNumber - 1];
    return goal.setGoal(goal.getGoalId(), goal.getGoalName(), goal.getTargetAmount(),
                        savings, monthly, goal.getTargetDate());
}

// =================== Saving and loading (CSV) ===================
//
// Data is saved as CSV files (Comma-Separated Values), which open directly in
// Excel or Google Sheets. Each file starts with a header row naming the columns:
//
//   account.csv       cash,bank
//   transactions.csv  id,type,amount,category,date,account,description
//   budgets.csv       category,limit,month,year
//   goals.csv         id,name,target,savings,monthlySaving,targetDate
//   investments.csv   type,name,invested,currentValue,quantity
//   pending.csv       type,amount,category,date,account,description   (ready queue, FIFO order)
//   scheduled.csv     type,amount,category,date,account,description   (heap, due-date order)
//
// Standard CSV quoting: a text value containing a comma or a double quote is
// written inside double quotes, and any " inside it is written twice:
//   Rent, March        ->  "Rent, March"
//   Mom's "gift"       ->  "Mom's ""gift"""

namespace
{
    const char COMMA = ',';
    const char QUOTE = '"';
    const int MAX_FIELDS = 8;

    const string ACCOUNT_HEADER = "cash,bank";
    const string TRANSACTION_HEADER = "id,type,amount,category,date,account,description";
    const string BUDGET_HEADER = "category,limit,month,year";
    const string GOAL_HEADER = "id,name,target,savings,monthlySaving,targetDate";
    const string INVESTMENT_HEADER = "type,name,invested,currentValue,quantity";
    const string PENDING_HEADER = "type,amount,category,date,account,description";

    // Prepares one text value for a CSV file (see the quoting rule above).
    string csvField(const string& text)
    {
        bool needsQuotes = false;
        string inner = "";

        for (size_t i = 0; i < text.size(); i++)
        {
            char ch = text[i];
            if (ch == '\n' || ch == '\r')
                ch = ' ';                       // one record must stay on one line
            if (ch == COMMA || ch == QUOTE)
                needsQuotes = true;
            if (ch == QUOTE)
                inner += QUOTE;                 // " is written as ""
            inner += ch;
        }

        if (needsQuotes)
            return QUOTE + inner + QUOTE;
        return inner;
    }

    // Splits one CSV line into fields[] and returns how many fields it has.
    //   a,"b, c",,"say ""hi"""   ->   {"a", "b, c", "", "say \"hi\""}   (4 fields)
    // Returns -1 if a quote is never closed (a damaged line).
    int splitCsvLine(const string& line, string fields[])
    {
        int count = 0;
        string current = "";
        bool insideQuotes = false;

        for (size_t i = 0; i < line.size(); i++)
        {
            char ch = line[i];

            if (insideQuotes)
            {
                if (ch == QUOTE && i + 1 < line.size() && line[i + 1] == QUOTE)
                {
                    current += QUOTE;           // "" inside quotes means one "
                    i++;
                }
                else if (ch == QUOTE)
                {
                    insideQuotes = false;       // closing quote
                }
                else
                {
                    current += ch;              // commas inside quotes are just text
                }
            }
            else if (ch == QUOTE)
            {
                insideQuotes = true;            // opening quote
            }
            else if (ch == COMMA)
            {
                if (count < MAX_FIELDS)
                    fields[count] = current;
                count++;
                current = "";
            }
            else
            {
                current += ch;
            }
        }

        if (insideQuotes)
            return -1;

        if (count < MAX_FIELDS)
            fields[count] = current;
        return count + 1;
    }

    // Text -> number. Fails on "abc", "" and "12abc".
    bool toDouble(const string& text, double& value)
    {
        istringstream input(text);
        char extra;
        return (input >> value) && !(input >> extra);
    }

    bool toInt(const string& text, int& value)
    {
        istringstream input(text);
        char extra;
        return (input >> value) && !(input >> extra);
    }

    // Reads the next non-empty line. Removes a Windows '\r' at the end if present.
    bool readRecord(ifstream& file, string& line)
    {
        while (getline(file, line))
        {
            if (!line.empty() && line[line.size() - 1] == '\r')
                line.erase(line.size() - 1);
            if (!line.empty())
                return true;
        }
        return false;
    }

    // Reads the next data record, skipping the header row if this is the first line.
    bool readDataRecord(ifstream& file, string& line, const string& header, bool& firstLine)
    {
        while (readRecord(file, line))
        {
            bool isHeader = firstLine && line == header;
            firstLine = false;
            if (!isHeader)
                return true;
        }
        return false;
    }
}

bool FinanceSystem::saveData(const string& folder) const
{
    ofstream accountFile((folder + "/account.csv").c_str());
    ofstream transactionFile((folder + "/transactions.csv").c_str());
    ofstream budgetFile((folder + "/budgets.csv").c_str());
    ofstream goalFile((folder + "/goals.csv").c_str());
    ofstream investmentFile((folder + "/investments.csv").c_str());
    ofstream pendingFile((folder + "/pending.csv").c_str());
    ofstream scheduledFile((folder + "/scheduled.csv").c_str());

    if (!accountFile || !transactionFile || !budgetFile ||
        !goalFile || !investmentFile || !pendingFile || !scheduledFile)
        return false;   // usually means the folder does not exist

    // Up to 15 significant digits: 350.75 is written as 350.75 (no extra zeros)
    // and fractional quantities such as 2.5 grams are kept exactly.
    accountFile << setprecision(15);
    transactionFile << setprecision(15);
    budgetFile << setprecision(15);
    goalFile << setprecision(15);
    investmentFile << setprecision(15);
    pendingFile << setprecision(15);
    scheduledFile << setprecision(15);

    accountFile << ACCOUNT_HEADER << '\n'
                << account.getCashBalance() << COMMA << account.getBankBalance() << '\n';

    transactionFile << TRANSACTION_HEADER << '\n';
    for (TransactionNode* node = transactions.getHead(); node != nullptr; node = node->next)
    {
        const Transaction& t = node->data;
        transactionFile << t.getTransactionId() << COMMA << t.getType() << COMMA
                        << t.getAmount() << COMMA << csvField(t.getCategory()) << COMMA
                        << t.getDate() << COMMA << t.getAccountName() << COMMA
                        << csvField(t.getDescription()) << '\n';
    }

    budgetFile << BUDGET_HEADER << '\n';
    for (int i = 0; i < budgetCount; i++)
    {
        budgetFile << csvField(budgets[i].getCategory()) << COMMA << budgets[i].getLimit()
                   << COMMA << budgets[i].getMonth() << COMMA << budgets[i].getYear() << '\n';
    }

    goalFile << GOAL_HEADER << '\n';
    for (int i = 0; i < goalCount; i++)
    {
        goalFile << goals[i].getGoalId() << COMMA << csvField(goals[i].getGoalName())
                 << COMMA << goals[i].getTargetAmount() << COMMA << goals[i].getCurrentSavings()
                 << COMMA << goals[i].getMonthlySaving() << COMMA
                 << csvField(goals[i].getTargetDate()) << '\n';
    }

    investmentFile << INVESTMENT_HEADER << '\n';
    for (int i = 0; i < investmentCount; i++)
    {
        investmentFile << investments[i]->getType() << COMMA   // virtual call
                       << csvField(investments[i]->getName()) << COMMA
                       << investments[i]->getAmountInvested() << COMMA
                       << investments[i]->getCurrentValue() << COMMA
                       << investments[i]->getQuantity() << '\n';
    }

    // The queue has no way to look at every item, so we copy it and empty the
    // COPY in FIFO order. The real queue is not changed.
    pendingFile << PENDING_HEADER << '\n';
    TransactionQueue copy = pending;
    Transaction t;
    while (copy.dequeue(t))
    {
        pendingFile << t.getType() << COMMA << t.getAmount() << COMMA
                    << csvField(t.getCategory()) << COMMA << t.getDate() << COMMA
                    << t.getAccountName() << COMMA << csvField(t.getDescription()) << '\n';
    }

    // Scheduled bills are written in due-date order by emptying a COPY of the heap
    // (heap sort). Loading inserts them again in that order, so bills with the same
    // date keep their order.
    scheduledFile << PENDING_HEADER << '\n';
    TransactionHeap heapCopy = scheduled;
    while (heapCopy.removeMin(t))
    {
        scheduledFile << t.getType() << COMMA << t.getAmount() << COMMA
                      << csvField(t.getCategory()) << COMMA << t.getDate() << COMMA
                      << t.getAccountName() << COMMA << csvField(t.getDescription()) << '\n';
    }

    return accountFile.good() && transactionFile.good() && budgetFile.good() &&
           goalFile.good() && investmentFile.good() && pendingFile.good() &&
           scheduledFile.good();
}

// Adds a saved transaction WITHOUT changing the balance, because account.csv
// already holds the final balances. The record is still validated.
bool FinanceSystem::loadTransaction(const Transaction& transaction)
{
    int id = transaction.getTransactionId();
    if (id <= 0 || idIndex.contains(id))
        return false;

    string type = transaction.getType();
    string accountName = transaction.getAccountName();
    if (!cleanTransactionInput(type, transaction.getAmount(), transaction.getCategory(),
                               transaction.getDate(), accountName))
        return false;

    Transaction* stored = transactions.insert(Transaction(id, type, transaction.getAmount(),
                                              transaction.getCategory(), transaction.getDate(),
                                              accountName, transaction.getDescription()));
    idIndex.insert(id, stored);

    // New transactions must get an ID higher than any loaded one.
    if (id >= nextTransactionId)
        nextTransactionId = id + 1;

    return true;
}

bool FinanceSystem::loadData(const string& folder)
{
    // Loading into a system that already has data would create duplicates.
    if (transactions.getHead() != nullptr || budgetCount > 0 || goalCount > 0 ||
        investmentCount > 0 || !pending.isEmpty() || !scheduled.isEmpty())
        return false;

    bool allGood = true;
    bool first;
    string line;
    string f[MAX_FIELDS];
    int id, month, year;
    double a, b, c;

    // A file that does not exist simply loads nothing (first run).
    ifstream accountFile((folder + "/account.csv").c_str());
    first = true;
    if (readDataRecord(accountFile, line, ACCOUNT_HEADER, first))
    {
        if (!(splitCsvLine(line, f) == 2 && toDouble(f[0], a) && toDouble(f[1], b) &&
              setOpeningBalances(a, b)))
            allGood = false;
    }

    ifstream transactionFile((folder + "/transactions.csv").c_str());
    first = true;
    while (readDataRecord(transactionFile, line, TRANSACTION_HEADER, first))
    {
        if (!(splitCsvLine(line, f) == 7 && toInt(f[0], id) && toDouble(f[2], a) &&
              loadTransaction(Transaction(id, f[1], a, f[3], f[4], f[5], f[6]))))
            allGood = false;
    }

    ifstream budgetFile((folder + "/budgets.csv").c_str());
    first = true;
    while (readDataRecord(budgetFile, line, BUDGET_HEADER, first))
    {
        if (!(splitCsvLine(line, f) == 4 && toDouble(f[1], a) && toInt(f[2], month) &&
              toInt(f[3], year) && addBudget(f[0], a, month, year)))
            allGood = false;
    }

    ifstream goalFile((folder + "/goals.csv").c_str());
    first = true;
    while (readDataRecord(goalFile, line, GOAL_HEADER, first))
    {
        if (splitCsvLine(line, f) == 6 && toInt(f[0], id) && toDouble(f[2], a) &&
            toDouble(f[3], b) && toDouble(f[4], c) && goalCount < MAX_GOALS &&
            goals[goalCount].setGoal(id, f[1], a, b, c, f[5]))
            goalCount++;
        else
            allGood = false;
    }

    ifstream investmentFile((folder + "/investments.csv").c_str());
    first = true;
    while (readDataRecord(investmentFile, line, INVESTMENT_HEADER, first))
    {
        if (!(splitCsvLine(line, f) == 5 && toDouble(f[2], a) && toDouble(f[3], b) &&
              toDouble(f[4], c) && addInvestment(f[0], f[1], a, b, c)))
            allGood = false;
    }

    ifstream pendingFile((folder + "/pending.csv").c_str());
    first = true;
    while (readDataRecord(pendingFile, line, PENDING_HEADER, first))
    {
        if (!(splitCsvLine(line, f) == 6 && toDouble(f[1], a) &&
              addPendingTransaction(f[0], a, f[2], f[3], f[4], f[5])))
            allGood = false;
    }

    ifstream scheduledFile((folder + "/scheduled.csv").c_str());
    first = true;
    while (readDataRecord(scheduledFile, line, PENDING_HEADER, first))
    {
        if (!(splitCsvLine(line, f) == 6 && toDouble(f[1], a) &&
              addScheduledTransaction(f[0], a, f[2], f[3], f[4], f[5])))
            allGood = false;
    }

    return allGood;
}

// ======================== Summary ========================

double FinanceSystem::getTotalIncome() const
{
    double total = 0;
    for (TransactionNode* node = transactions.getHead(); node != nullptr; node = node->next)
    {
        if (node->data.getType() == "Income")
            total += node->data.getAmount();
    }
    return total;
}

double FinanceSystem::getTotalExpense() const
{
    double total = 0;
    for (TransactionNode* node = transactions.getHead(); node != nullptr; node = node->next)
    {
        if (node->data.getType() == "Expense")
            total += node->data.getAmount();
    }
    return total;
}

double FinanceSystem::getTotalInvested() const
{
    double total = 0;
    for (int i = 0; i < investmentCount; i++)
        total += investments[i]->getAmountInvested();
    return total;
}

double FinanceSystem::getPortfolioValue() const
{
    double total = 0;
    for (int i = 0; i < investmentCount; i++)
        total += investments[i]->getCurrentValue();
    return total;
}

void FinanceSystem::displayDashboard() const
{
    double income = getTotalIncome();
    double expense = getTotalExpense();
    double invested = getTotalInvested();
    double portfolio = getPortfolioValue();

    cout << fixed << setprecision(2);
    cout << "\n=========== DASHBOARD ===========\n";
    cout << "Cash Balance      : Rs. " << account.getCashBalance() << '\n';
    cout << "Bank Balance      : Rs. " << account.getBankBalance() << '\n';
    cout << "Total Balance     : Rs. " << account.getTotalBalance() << '\n';
    cout << "---------------------------------\n";
    cout << "Total Income      : Rs. " << income << '\n';
    cout << "Total Expense     : Rs. " << expense << '\n';
    cout << "Net Savings       : Rs. " << (income - expense) << '\n';
    cout << "---------------------------------\n";
    cout << "Amount Invested   : Rs. " << invested << '\n';
    cout << "Portfolio Value   : Rs. " << portfolio << '\n';
    cout << "Investment P/L    : Rs. " << (portfolio - invested) << '\n';
    cout << "---------------------------------\n";
    cout << "Budgets           : " << budgetCount << '\n';

    for (int i = 0; i < budgetCount; i++)
    {
        if (budgets[i].calculateRemaining(transactions) < 0)
            cout << "  WARNING: " << budgets[i].getCategory() << " budget exceeded!\n";
    }

    cout << "Goals             : " << goalCount << '\n';
    cout << "Investments       : " << investmentCount << '\n';
    cout << "=================================\n";
}
