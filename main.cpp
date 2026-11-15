#include <iostream>
#include <sstream>
#include <string>
#include "FinanceSystem.h"

using namespace std;


const string DATA_FOLDER = "Data";



string readLine(const string& prompt)
{
    cout << prompt;
    string text;
    if (!getline(cin, text))
        return "";   // input was closed (Ctrl+Z / end of file)

    // Remove spaces (and a Windows '\r') at both ends, so "Food " matches "Food".
    size_t start = text.find_first_not_of(" \t\r");
    if (start == string::npos)
        return "";
    size_t end = text.find_last_not_of(" \t\r");
    return text.substr(start, end - start + 1);
}

// After an expense is recorded, show the budget it counts towards
// (or explain why it does not count towards one).
void showBudgetAfterExpense(FinanceSystem& fs, int transactionId)
{
    Transaction* transaction = fs.findTransaction(transactionId);
    if (transaction != nullptr && transaction->getType() == "Expense")
        fs.displayBudgetFor(transaction->getCategory(), transaction->getDate());
}

int readInt(const string& prompt)
{
    while (true)
    {
        string text = readLine(prompt);
        if (!cin)
            return 0;   // input closed: 0 means "back / exit" in every menu

        istringstream input(text);
        int value;
        char extra;
        if ((input >> value) && !(input >> extra))
            return value;

        cout << "Please enter a whole number.\n";
    }
}

double readDouble(const string& prompt)
{
    while (true)
    {
        string text = readLine(prompt);
        if (!cin)
            return 0;

        istringstream input(text);
        double value;
        char extra;
        if ((input >> value) && !(input >> extra))
            return value;

        cout << "Please enter a number.\n";
    }
}

// ========================= Transactions =========================

void readTransactionDetails(string& type, double& amount, string& category,
                            string& date, string& accountName, string& description)
{
    type = readLine("Type (Income/Expense): ");
    amount = readDouble("Amount: Rs. ");
    category = readLine("Category (e.g. Food, Rent, Salary): ");
    date = readLine("Date (DD-MM-YYYY): ");
    accountName = readLine("Account (Cash/Bank): ");
    description = readLine("Description (optional): ");
}

void addTransaction(FinanceSystem& fs)
{
    string type, category, date, accountName, description;
    double amount;
    readTransactionDetails(type, amount, category, date, accountName, description);

    int id = fs.addTransaction(type, amount, category, date, accountName, description);
    if (id != -1)
    {
        cout << "Transaction added with ID " << id << ".\n";
        showBudgetAfterExpense(fs, id);
    }
    else
        cout << "Not added. Check that: type is Income or Expense, amount is more than 0,\n"
             << "category is not empty, the date is a real DD-MM-YYYY date, account is\n"
             << "Cash or Bank, and the account has enough money for an expense.\n";
}

void findTransaction(FinanceSystem& fs)
{
    int id = readInt("Transaction ID: ");
    Transaction* transaction = fs.findTransaction(id);
    if (transaction != nullptr)
        transaction->displayTransaction();
    else
        cout << "No transaction with ID " << id << ".\n";
}

void deleteTransaction(FinanceSystem& fs)
{
    int id = readInt("Transaction ID to delete: ");
    if (fs.deleteTransaction(id))
        cout << "Transaction " << id << " deleted. (You can undo this.)\n";
    else
        cout << "Not deleted: there is no such ID, or removing this income would make\n"
             << "the account balance negative.\n";
}

void undoLastAction(FinanceSystem& fs)
{
    if (!fs.canUndo())
        cout << "Nothing to undo.\n";
    else if (fs.undoLastAction())
        cout << "Last action undone.\n";
    else
        cout << "Cannot undo: it would make the account balance negative.\n";
}

void transactionMenu(FinanceSystem& fs)
{
    while (true)
    {
        cout << "\n----------- TRANSACTIONS -----------\n"
             << "  1. Add transaction\n"
             << "  2. View all transactions\n"
             << "  3. Find by ID\n"
             << "  4. Search by category\n"
             << "  5. Search by type (Income/Expense)\n"
             << "  6. Sort by amount\n"
             << "  7. Sort by category\n"
             << "  8. Sort by date\n"
             << "  9. Delete transaction\n"
             << " 10. Undo last add/delete\n"
             << "  0. Back\n";

        int choice = readInt("Choose: ");
        switch (choice)
        {
        case 1: addTransaction(fs); break;
        case 2: fs.displayTransactions(); break;
        case 3: findTransaction(fs); break;
        case 4: fs.searchByCategory(readLine("Category: ")); break;
        case 5: fs.searchByType(readLine("Type (Income/Expense): ")); break;
        case 6: fs.sortTransactionsByAmount(); break;
        case 7: fs.sortTransactionsByCategory(); break;
        case 8: fs.sortTransactionsByDate(); break;
        case 9: deleteTransaction(fs); break;
        case 10: undoLastAction(fs); break;
        case 0: return;
        default: cout << "Invalid choice.\n";
        }
    }
}

// ===================== Pending transactions =====================

// Bills go through two data structures (like an operating-system scheduler):
//   1. SCHEDULED: a min-heap ordered by due date (the earliest bill is on top)
//   2. READY:     a FIFO queue of bills that are due, paid in order

// Lists the scheduled bills (earliest first), lets the user pick ONE, and releases
// it into the ready queue if it is due on or before the chosen date.
void releaseOneBill(FinanceSystem& fs)
{
    if (fs.getScheduledCount() == 0)
    {
        cout << "No scheduled bills.\n";
        return;
    }

    string today = FinanceSystem::todayDate();
    string date = readLine("Bills due on or before (DD-MM-YYYY, Enter = today " + today + "): ");
    if (date.empty())
        date = today;
    if (!Transaction::isValidDate(date))
    {
        cout << "Invalid date. Use DD-MM-YYYY.\n";
        return;
    }
    Transaction limit;
    limit.setDate(date);

    // Empty a COPY of the heap to list the bills in date order (heap sort), and
    // remember each bill's arrival number so the chosen one can be released.
    TransactionHeap copy = fs.getScheduledCopy();
    int sequences[TransactionHeap::CAPACITY];
    int count = 0;
    Transaction bill;
    while (!copy.isEmpty())
    {
        sequences[count] = copy.peekMinSequence();
        copy.removeMin(bill);
        count++;
        cout << "  " << count << ". " << bill.getDate() << "  " << bill.getType() << " Rs. "
             << bill.getAmount() << "  " << bill.getCategory()
             << (bill.getDateKey() <= limit.getDateKey() ? "   [due]" : "   [upcoming]") << '\n';
    }

    int number = readInt("Bill number to release (0 = cancel): ");
    if (number == 0)
        return;
    if (number < 1 || number > count)
    {
        cout << "No bill with that number.\n";
        return;
    }

    switch (fs.releaseScheduled(sequences[number - 1], date))
    {
    case FinanceSystem::RELEASED:
        cout << "Bill " << number << " moved to the back of the ready queue.\n";
        break;
    case FinanceSystem::NOT_DUE_YET:
        cout << "That bill is not due on or before " << date << ", so it cannot be released yet.\n";
        break;
    case FinanceSystem::QUEUE_FULL:
        cout << "The ready queue is full (" << TransactionQueue::CAPACITY << " bills).\n";
        break;
    default:
        cout << "That bill could not be released.\n";
    }
}

void pendingMenu(FinanceSystem& fs)
{
    while (true)
    {
        cout << "\n------- BILLS (scheduled: " << fs.getScheduledCount()
             << ", ready to pay: " << fs.getPendingCount() << ") -------\n"
             << "  1. Add bill (scheduled by due date)\n"
             << "  2. View scheduled bills (earliest due first)\n"
             << "  3. Release due bills to the ready queue\n"
             << "  4. View ready queue\n"
             << "  5. Pay next ready bill\n"
             << "  6. Move front bill to the back of the queue\n"
             << "  7. Release one due bill\n"
             << "  0. Back\n";

        int choice = readInt("Choose: ");
        if (choice == 1)
        {
            string type, category, date, accountName, description;
            double amount;
            readTransactionDetails(type, amount, category, date, accountName, description);

            if (fs.addScheduledTransaction(type, amount, category, date, accountName, description))
                cout << "Bill scheduled for " << date << ". It changes your balance only when it is paid.\n";
            else
                cout << "Not added: check the values (or " << TransactionHeap::CAPACITY << " bills are already scheduled).\n";
        }
        else if (choice == 2)
        {
            fs.displayScheduled();
        }
        else if (choice == 3)
        {
            string today = FinanceSystem::todayDate();
            string date = readLine("Release bills due on or before (DD-MM-YYYY, Enter = today " + today + "): ");
            if (date.empty())
                date = today;

            int released = fs.releaseDue(date);
            if (released == -1)
                cout << "Invalid date. Use DD-MM-YYYY.\n";
            else if (released == 0)
                cout << "No bills are due on or before " << date << ".\n";
            else
                cout << released << " bill(s) moved to the ready queue (earliest first).\n";

            if (released >= 0 && fs.getScheduledCount() > 0 && fs.getPendingCount() == TransactionQueue::CAPACITY)
                cout << "The ready queue is full; the remaining bills stay scheduled.\n";
        }
        else if (choice == 4)
        {
            fs.displayPending();
        }
        else if (choice == 6)
        {
            if (fs.moveFrontToBack())
                cout << "The front bill moved to the back of the queue.\n";
            else
                cout << "Nothing to move: the queue needs at least 2 bills.\n";
        }
        else if (choice == 7)
        {
            releaseOneBill(fs);
        }
        else if (choice == 5)
        {
            if (fs.getPendingCount() == 0)
            {
                cout << "No bills are ready to pay. Release due bills first (option 3).\n";
                continue;
            }
            int id = fs.processNextPending();
            if (id != -1)
            {
                cout << "Processed: recorded as transaction ID " << id << ".\n";
                showBudgetAfterExpense(fs, id);
            }
            else
                cout << "Could not process: not enough money in the account.\n"
                     << "It stays at the front of the queue.\n";
        }
        else if (choice == 0)
        {
            return;
        }
        else
        {
            cout << "Invalid choice.\n";
        }
    }
}

// =========================== Budgets ===========================

void budgetMenu(FinanceSystem& fs)
{
    while (true)
    {
        cout << "\n------------- BUDGETS -------------\n"
             << "  1. Add monthly budget\n"
             << "  2. Change a budget limit\n"
             << "  3. View budgets and spending\n"
             << "  0. Back\n";

        int choice = readInt("Choose: ");
        if (choice == 1)
        {
            string category = readLine("Category: ");
            double limit = readDouble("Monthly limit: Rs. ");
            int month = readInt("Month (1-12): ");
            int year = readInt("Year (e.g. 2026): ");

            if (fs.addBudget(category, limit, month, year))
                cout << "Budget added.\n";
            else
                cout << "Not added. The limit must be more than 0, month 1-12, and there can be\n"
                     << "only one budget per category per month (maximum " << FinanceSystem::MAX_BUDGETS << " budgets).\n";
        }
        else if (choice == 2)
        {
            string category = readLine("Category: ");
            int month = readInt("Month (1-12): ");
            int year = readInt("Year: ");
            double limit = readDouble("New limit: Rs. ");

            if (fs.updateBudget(category, month, year, limit))
                cout << "Budget updated.\n";
            else
                cout << "Not updated: no such budget, or the limit is not more than 0.\n";
        }
        else if (choice == 3)
        {
            fs.displayBudgets();
        }
        else if (choice == 0)
        {
            return;
        }
        else
        {
            cout << "Invalid choice.\n";
        }
    }
}

// ============================ Goals ============================

void goalMenu(FinanceSystem& fs)
{
    while (true)
    {
        cout << "\n------------- GOALS -------------\n"
             << "  1. Add savings goal\n"
             << "  2. Update savings of a goal\n"
             << "  3. View goals\n"
             << "  0. Back\n";

        int choice = readInt("Choose: ");
        if (choice == 1)
        {
            if (!fs.addGoal())   // Goal::createGoal() asks the questions
                cout << "Goal not added (the input ended, or the maximum of "
                     << FinanceSystem::MAX_GOALS << " goals is reached).\n";
        }
        else if (choice == 2)
        {
            fs.displayGoals();
            int number = readInt("Goal number to update: ");
            if (!fs.updateGoal(number))   // Goal::updateGoal() asks the questions
                cout << "Goal not updated (no goal with that number, or the input ended).\n";
        }
        else if (choice == 3)
        {
            fs.displayGoals();
        }
        else if (choice == 0)
        {
            return;
        }
        else
        {
            cout << "Invalid choice.\n";
        }
    }
}

// ========================= Investments =========================

void addInvestment(FinanceSystem& fs)
{
    const string types[] = {"Stock", "Mutual Fund", "Fixed Deposit", "Gold", "Other"};

    cout << "Investment type:\n";
    for (int i = 0; i < 5; i++)
        cout << "  " << (i + 1) << ". " << types[i] << '\n';

    int typeNumber = readInt("Choose type (1-5): ");
    if (typeNumber < 1 || typeNumber > 5)
    {
        cout << "Invalid type.\n";
        return;
    }

    string name = readLine("Name (e.g. TCS, SBI FD): ");
    double invested = readDouble("Amount invested: Rs. ");
    double current = readDouble("Current value: Rs. ");
    double quantity = readDouble("Quantity (shares / units / grams; 1 for an FD): ");

    if (fs.addInvestment(types[typeNumber - 1], name, invested, current, quantity))
        cout << "Investment added.\n";
    else
        cout << "Not added. Name must not be empty, amount invested and quantity must be\n"
             << "more than 0, and current value cannot be negative (maximum " << FinanceSystem::MAX_INVESTMENTS << " investments).\n";
}

void investmentMenu(FinanceSystem& fs)
{
    while (true)
    {
        cout << "\n----------- INVESTMENTS -----------\n"
             << "  1. Add investment\n"
             << "  2. Update current value\n"
             << "  3. View investments (profit/loss)\n"
             << "  0. Back\n";

        int choice = readInt("Choose: ");
        if (choice == 1)
        {
            addInvestment(fs);
        }
        else if (choice == 2)
        {
            fs.displayInvestments();
            int number = readInt("Investment number to update: ");
            double value = readDouble("New current value: Rs. ");
            if (fs.updateInvestmentValue(number, value))
                cout << "Value updated.\n";
            else
                cout << "Not updated: no investment with that number, or the value is negative.\n";
        }
        else if (choice == 3)
        {
            fs.displayInvestments();
        }
        else if (choice == 0)
        {
            return;
        }
        else
        {
            cout << "Invalid choice.\n";
        }
    }
}

// =========================== Account ===========================

void setBalances(FinanceSystem& fs)
{
    cout << "Current balances: Cash Rs. " << fs.getAccount().getCashBalance()
         << ", Bank Rs. " << fs.getAccount().getBankBalance() << '\n'
         << "Use this to set your starting balances. It REPLACES the current balances.\n";

    double cash = readDouble("Cash balance: Rs. ");
    double bank = readDouble("Bank balance: Rs. ");

    if (fs.setOpeningBalances(cash, bank))
        cout << "Balances set.\n";
    else
        cout << "Not set: balances cannot be negative.\n";
}

// ============================= Main =============================

int main()
{
    // static: stored in the program's data area, not on the (small) stack,
    // because FinanceSystem holds large fixed-size arrays (about 180 KB).
    static FinanceSystem fs;

    if (!fs.loadData(DATA_FOLDER))
        cout << "Warning: some saved records were damaged and were skipped.\n";

    cout << "\n=============================================\n"
         << "   SMART PERSONAL FINANCE MANAGEMENT SYSTEM\n"
         << "=============================================\n";

    while (true)
    {
        cout << "\n============== MAIN MENU ==============\n"
             << "  1. Dashboard\n"
             << "  2. Transactions\n"
             << "  3. Bills\n"
             << "  4. Budgets\n"
             << "  5. Goals\n"
             << "  6. Investments\n"
             << "  7. Set account balances\n"
             << "  8. Save now\n"
             << "  0. Save and exit\n";

        int choice = readInt("Choose: ");
        if (choice == 0)
            break;

        switch (choice)
        {
        case 1: fs.displayDashboard(); break;
        case 2: transactionMenu(fs); break;
        case 3: pendingMenu(fs); break;
        case 4: budgetMenu(fs); break;
        case 5: goalMenu(fs); break;
        case 6: investmentMenu(fs); break;
        case 7: setBalances(fs); break;
        case 8:
            if (fs.saveData(DATA_FOLDER))
                cout << "Saved.\n";
            else
                cout << "ERROR: could not save. Make sure the '" << DATA_FOLDER << "' folder exists.\n";
            break;
        default: cout << "Invalid choice.\n";
        }
    }

    if (fs.saveData(DATA_FOLDER))
        cout << "Data saved. Goodbye!\n";
    else
        cout << "ERROR: could not save. Make sure the '" << DATA_FOLDER << "' folder exists.\n";

    return 0;
}
