#include <iostream>
#include "transactions.h"

using namespace std;

Transaction::Transaction()
{
    transactionId = 0;
    type = "";
    amount = 0.0;
    category = "";
    date = "";
    AccountName = "";
    description = "";
}

Transaction::Transaction(int transactionId,
                         string type,
                         double amount,
                         string category,
                         string date,
                         string AccountName,
                         string description)
{
    this->transactionId = transactionId;
    this->type = type;
    this->amount = amount;
    this->category = category;
    this->date = date;
    this->AccountName = AccountName;
    this->description = description;
}

// Getter functions
int Transaction::getTransactionId() const
{
    return transactionId;
}

string Transaction::getType() const
{
    return type;
}

double Transaction::getAmount() const
{
    return amount;
}

string Transaction::getCategory() const
{
    return category;
}

string Transaction::getDate() const
{
    return date;
}

string Transaction::getAccountName() const
{
    return AccountName;
}

string Transaction::getDescription() const
{
    return description;
}

//setter functions
void Transaction::setAmount(double amount)
{
    if (amount > 0)
    {
        this->amount = amount;
    }
}

void Transaction::setCategory(string category)
{
    this->category = category;
}

void Transaction::setDate(string date)
{
    this->date = date;
}

void Transaction::setAccountName(string accountName)
{
    this->AccountName = AccountName;
}

void Transaction::setDescription(string description)
{
    this->description = description;
}

void Transaction::displayTransaction() const
{
    cout << "Transaction ID : " << transactionId << endl;
    cout << "Type           : " << type << endl;
    cout << "Amount         : " << amount << endl;
    cout << "Category       : " << category << endl;
    cout << "Date           : " << date << endl;
    cout << "Account        : " << AccountName << endl;
    cout << "Description    : " << description << endl;
}