#ifndef BUDGET_H
#define BUDGET_H
#include <string>
#include "../DSA/Linked_list.h"
using namespace std;
class Budget
{
private:
    string category;
    double limit;
    int month;
    int year;
public:
    Budget();
    Budget(string cat, double amount, int budgetMonth, int budgetYear);
    void createBudget(string cat);
    bool setBudget(string cat, double amount, int budgetMonth, int budgetYear);
    bool updateBudget(double newLimit);
    void displayBudget() const;
    void displayBudget(const TransactionList& transactions) const;
    string getCategory() const;
    double getLimit() const;
    int getMonth() const;
    int getYear() const;
    double calculateSpent(const TransactionList& transactions) const;
    double calculateRemaining(const TransactionList& transactions) const;
    double calculateUsagePercent(const TransactionList& transactions) const;
};
#endif
