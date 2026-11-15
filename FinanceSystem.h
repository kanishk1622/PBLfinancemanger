#ifndef FINANCESYSTEM_H
#define FINANCESYSTEM_H

#include <string>
#include <vector>
#include "Account/Account.h"
#include "DSA/Linked_list.h"
#include "Budgets/Budget.h"
#include "Goals/goals.h"
#include "Investment/investment.h"
#include "DSA/HashTable.h"
#include "DSA/Stack.h"
#include "DSA/Queue.h"
#include "DSA/Heap.h"

using namespace std;

class FinanceSystem
{
public:
    static const int MAX_BUDGETS = 500;
    static const int MAX_GOALS = 200;
    static const int MAX_INVESTMENTS = 500;

private:
    Account account;
    TransactionList transactions;

    Budget budgets[MAX_BUDGETS];
    int budgetCount;

    Goal goals[MAX_GOALS];
    int goalCount;

    // Base-class pointers: each element may be a Stock, Gold, etc.
    // FinanceSystem owns these objects and deletes them in the destructor.
    Investment* investments[MAX_INVESTMENTS];
    int investmentCount;

    int nextTransactionId;   // IDs are generated, so they can never be duplicated

    HashTable idIndex;          // transaction ID -> transaction, for O(1) lookup
    UndoStack undoStack;        // history of add/delete actions, most recent on top
    TransactionQueue pending;   // READY bills, paid first-in first-out
    TransactionHeap scheduled;  // FUTURE bills, min-heap ordered by due date

    int findBudgetIndex(const string& category, int month, int year) const;

    // Helpers shared by the public functions and by undo (see FinanceSystem.cpp).
    bool cleanTransactionInput(string& type, double amount, const string& category,
                               const string& date, string& accountName) const;
    bool insertTransaction(const Transaction& transaction);
    bool removeTransaction(int transactionId);
    bool loadTransaction(const Transaction& transaction);   // used only by loadData()
    bool isCompleteGoal(Goal& goal) const;                   // used by addGoal()/updateGoal()

public:
    FinanceSystem();
    ~FinanceSystem();

    // Owns raw pointers, so copying must be disabled (would double-delete).
    FinanceSystem(const FinanceSystem&) = delete;
    FinanceSystem& operator=(const FinanceSystem&) = delete;

    // ---------- Account ----------
    bool setOpeningBalances(double cash, double bank);
    const Account& getAccount() const;

    // ---------- Transactions ----------
    // type: "Income" or "Expense"; accountName: "Cash" or "Bank" (case-insensitive).
    // Returns the new transaction ID, or -1 if the input is invalid or the
    // account does not have enough money for an expense.
    int addTransaction(const string& type, double amount, const string& category,
                       const string& date, const string& accountName,
                       const string& description);

    // Deletes a transaction and reverses its effect on the account balance.
    // Fails if the ID does not exist, or if removing an income would make
    // the account balance negative (that money was already spent).
    bool deleteTransaction(int transactionId);

    Transaction* findTransaction(int transactionId);   // uses the hash table
    void displayTransactions() const;
    TransactionList& getTransactions();   // for Searching/Sorting modules

    // Linear search (Searching module). Print the matches and return how many.
    int searchByCategory(const string& category);
    int searchByType(const string& type);

    void sortTransactionsByAmount();
    void sortTransactionsByCategory();
    void sortTransactionsByDate();

    // ---------- Undo (stack) ----------
    // Reverses the most recent add or delete of a transaction.
    // Returns false if there is nothing to undo, or if the undo is not possible
    // (e.g. undoing an income whose money has already been spent).
    bool undoLastAction();
    bool canUndo() const;

    // ---------- Pending transactions (queue) ----------
    // Validates and adds a planned transaction to the back of the queue.
    bool addPendingTransaction(const string& type, double amount, const string& category,
                               const string& date, const string& accountName,
                               const string& description);
    // Records the transaction at the front of the queue.
    // Returns its new ID, or -1 if the queue is empty or it could not be recorded
    // (in that case it stays in the queue).
    int processNextPending();
    void displayPending() const;
    int getPendingCount() const;

    // ---------- Scheduled bills (min-heap) + ready queue ----------
    // Like an operating-system scheduler:
    //   the HEAP decides WHEN a bill is due (earliest due date always on top),
    //   the QUEUE decides the ORDER in which due bills are paid (FIFO).
    bool addScheduledTransaction(const string& type, double amount, const string& category,
                                 const string& date, const string& accountName,
                                 const string& description);
    // Moves every scheduled bill dated on or before 'date' from the heap into the
    // ready queue, earliest first. Stops early if the queue is full.
    // Returns how many were moved, or -1 if 'date' is not a valid DD-MM-YYYY date.
    int releaseDue(const string& date);

    // Releases ONE scheduled bill (chosen by its arrival number, see
    // TransactionHeap::peekMinSequence) into the ready queue, if it is due on or
    // before 'date'. Uses the heap's remove-any-item operation.
    enum ReleaseResult { RELEASED, NO_SUCH_BILL, NOT_DUE_YET, QUEUE_FULL, INVALID_DATE };
    ReleaseResult releaseScheduled(int sequence, const string& date);
    // A stuck bill at the front of the ready queue is dequeued and enqueued again,
    // so the next bill can be paid. Needs at least 2 bills in the queue.
    bool moveFrontToBack();
    TransactionHeap getScheduledCopy() const;   // a copy, so the real heap is not changed
    int getScheduledCount() const;
    void displayScheduled() const;              // console: scheduled bills in date order

    // Today's date from the computer's clock, as DD-MM-YYYY.
    static string todayDate();

    // ---------- Budgets ----------
    bool addBudget(const string& category, double limit, int month, int year);
    bool updateBudget(const string& category, int month, int year, double newLimit);
    void displayBudgets() const;
    // Shows the budget that an expense in this category on this date counts
    // towards. If there is none, says so and lists the months that DO have a
    // budget for this category. Returns true if a matching budget exists.
    bool displayBudgetFor(const string& category, const string& date) const;

    // ---------- Goals ----------
    // Interactive: the Goal asks the questions. Both return false (and change
    // nothing) if the input closed half-way, e.g. Ctrl+Z.
    bool addGoal();
    bool updateGoal(int goalNumber);      // goalNumber starts at 1, as shown on screen
    void displayGoals() const;

    // ---------- Investments ----------
    // type: "Stock", "Mutual Fund", "Fixed Deposit", "Gold" or "Other" (case-insensitive).
    bool addInvestment(const string& type, const string& name,
                       double amountInvested, double currentValue, double quantity);
    bool updateInvestmentValue(int investmentNumber, double newValue);   // starts at 1
    void displayInvestments() const;

    // ---------- Read-only access (used by the web server) ----------
    // The display...() functions above PRINT data; the web page needs the data
    // itself. These functions only read: they never change anything.
    // Indexes start at 0 and must be less than the matching count.
    int getBudgetCount() const;
    const Budget& getBudget(int index) const;
    int getGoalCount() const;
    const Goal& getGoal(int index) const;
    int getInvestmentCount() const;
    const Investment* getInvestment(int index) const;
    TransactionQueue getPendingCopy() const;   // a copy, so the real queue is not changed

    // Linear search that RETURNS the matches instead of printing them
    // (the console still uses the Searching module). Case is ignored.
    vector<const Transaction*> findByCategory(const string& category) const;
    vector<const Transaction*> findByType(const string& type) const;

    // Index of the budget an expense in this category on this date counts
    // towards (for getBudget), or -1 if there is none.
    int findBudgetIndexFor(const string& category, const string& date) const;

    // ---------- Goals from values (web version: no keyboard questions) ----------
    // addGoalValues also rejects a goal ID that is already used, and checks the
    // date against the calendar (so 31-02-2026 is refused).
    bool addGoalValues(int id, const string& name, double target, double savings,
                       double monthly, const string& date);
    bool updateGoalValues(int goalNumber, double savings, double monthly);   // number starts at 1

    // ---------- Saving and loading (CSV files) ----------
    // Writes everything to 6 CSV files in 'folder' (they open in Excel):
    // a header row, then one record per line. The undo history is not saved.
    // Returns false if any file could not be written (e.g. folder missing).
    bool saveData(const string& folder) const;

    // Reads the files written by saveData(). Call it once, at program start.
    // Missing files are fine (first run). Every record is validated again;
    // damaged lines are skipped and the function returns false.
    bool loadData(const string& folder);

    // ---------- Summary ----------
    double getTotalIncome() const;
    double getTotalExpense() const;
    double getTotalInvested() const;
    double getPortfolioValue() const;
    void displayDashboard() const;
};

#endif
