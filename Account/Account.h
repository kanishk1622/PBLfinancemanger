#ifndef ACCOUNT_H
#define ACCOUNT_H
#include <string>
using namespace std;

class Account
{
private:
    double cashBalance;
    double bankBalance;

public:
    Account();

    bool deposit(string accountType, double amount);
    bool withdraw(string accountType, double amount);

    double getCashBalance() const;
    double getBankBalance() const;
    double getTotalBalance() const;

    void setBalances(double cash, double bank);
};

#endif