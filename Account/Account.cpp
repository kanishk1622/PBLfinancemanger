#include "Account.h"

Account::Account()
{
    cashBalance = 0;
    bankBalance = 0;
}

bool Account::deposit(string accountType, double amount)
{
    if (amount <= 0)
        return false;

    if (accountType == "Cash")
        cashBalance += amount;
    else if (accountType == "Bank")
        bankBalance += amount;
    else
        return false;

    return true;
}

bool Account::withdraw(string accountType, double amount)
{
    if (amount <= 0)
        return false;

    if (accountType == "Cash")
    {
        if (cashBalance < amount)
            return false;

        cashBalance -= amount;
    }
    else if (accountType == "Bank")
    {
        if (bankBalance < amount)
            return false;

        bankBalance -= amount;
    }
    else
        return false;

    return true;
}

double Account::getCashBalance() const
{
    return cashBalance;
}

double Account::getBankBalance() const
{
    return bankBalance;
}

double Account::getTotalBalance() const
{
    return cashBalance + bankBalance;
}

void Account::setBalances(double cash, double bank)
{
    if (cash >= 0 && bank >= 0)
    {
        cashBalance = cash;
        bankBalance = bank;
    }
}