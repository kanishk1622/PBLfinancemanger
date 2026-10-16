#include "Budget.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <limits>
using namespace std;
namespace
{
    string lowerText(string text)
    {
        for (size_t i = 0; i < text.size(); ++i)
            text[i] = static_cast<char>(tolower(static_cast<unsigned char>(text[i])));
        return text;
    }
    bool getMonthYear(const string& date, int& dateMonth, int& dateYear)
    {
        int day;
        char firstDash, secondDash;
        istringstream input(date);
        if (!(input >> day >> firstDash >> dateMonth >> secondDash >> dateYear))
            return false;
        return firstDash == '-' && secondDash == '-' &&
               day >= 1 && day <= 31 && dateMonth >= 1 && dateMonth <= 12 && dateYear > 0;
    }
}
Budget::Budget() : category(""), limit(0), month(1), year(2000) {}
Budget::Budget(string cat, double amount, int budgetMonth, int budgetYear)
    : category(""), limit(0), month(1), year(2000)
{
    setBudget(cat, amount, budgetMonth, budgetYear);
}
void Budget::createBudget(string cat)
{
    double amount;
    int budgetMonth, budgetYear;
    cout << "Enter monthly budget amount for " << cat << ": Rs. ";
    if (!(cin >> amount))
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid amount.\n";
        return;
    }
    cout << "Enter month (1-12): ";
    if (!(cin >> budgetMonth))
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid month.\n";
        return;
    }
    cout << "Enter year (e.g. 2026): ";
    if (!(cin >> budgetYear))
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid year.\n";
        return;
    }
    if (!setBudget(cat, amount, budgetMonth, budgetYear))
        cout << "Budget not created. Use a category, positive amount, valid month and year.\n";
}
bool Budget::setBudget(string cat, double amount, int budgetMonth, int budgetYear)
{
    if (cat.empty() || amount <= 0 || budgetMonth < 1 || budgetMonth > 12 || budgetYear <= 0)
        return false;
    category = cat;
    limit = amount;
    month = budgetMonth;
    year = budgetYear;
    return true;
}
bool Budget::updateBudget(double newLimit)
{
    if (newLimit <= 0)
        return false;
    limit = newLimit;
    return true;
}
void Budget::displayBudget() const
{
    cout << category << " (" << setfill('0') << setw(2) << month << "-" << year
         << "): Rs. " << fixed << setprecision(2) << limit << setfill(' ') << '\n';
}
void Budget::displayBudget(const TransactionList& transactions) const
{
    const double spent = calculateSpent(transactions);
    const double remaining = limit - spent;
    cout << fixed << setprecision(2);
    cout << "\n----- MONTHLY BUDGET -----\n";
    cout << "Category: " << category << '\n';
    cout << "Month/Year: " << setfill('0') << setw(2) << month << "-" << year << setfill(' ') << '\n';
    cout << "Budget limit: Rs. " << limit << '\n';
    cout << "Total spent:  Rs. " << spent << '\n';
    cout << "Remaining:    Rs. " << remaining << '\n';
    cout << "Usage:        " << calculateUsagePercent(transactions) << "%\n";
    if (remaining < 0)
        cout << "WARNING: Budget exceeded by Rs. " << -remaining << "!\n";
    else if (remaining == 0)
        cout << "WARNING: You have used the full budget.\n";
}
string Budget::getCategory() const { return category; }
double Budget::getLimit() const { return limit; }
int Budget::getMonth() const { return month; }
int Budget::getYear() const { return year; }
double Budget::calculateSpent(const TransactionList& transactions) const
{
    double total = 0;
    TransactionNode* current = transactions.getHead();
    while (current != nullptr)
    {
        const Transaction& transaction = current->data;
        int transactionMonth = 0, transactionYear = 0;
        const bool isExpense = lowerText(transaction.getType()) == "expense";
        const bool sameCategory = lowerText(transaction.getCategory()) == lowerText(category);
        const bool validDate = getMonthYear(transaction.getDate(), transactionMonth, transactionYear);
        const bool sameMonthYear = validDate && transactionMonth == month && transactionYear == year;
        if (isExpense && sameCategory && sameMonthYear && transaction.getAmount() > 0)
            total += transaction.getAmount();
        current = current->next;
    }
    return total;
}
double Budget::calculateRemaining(const TransactionList& transactions) const
{
    return limit - calculateSpent(transactions);
}
double Budget::calculateUsagePercent(const TransactionList& transactions) const
{
    if (limit <= 0)
        return 0;
    return (calculateSpent(transactions) / limit) * 100.0;
}
