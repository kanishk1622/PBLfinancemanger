#include <iostream>
#include "transactions.h"

using namespace std;

// Default constructor
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

// Parameterized constructor
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

// Returns transaction ID
int Transaction::getTransactionId() const
{
    return transactionId;
}

// Returns transaction type
string Transaction::getType() const
{
    return type;
}

// Returns transaction amount
double Transaction::getAmount() const
{
    return amount;
}

// Returns transaction category
string Transaction::getCategory() const
{
    return category;
}

// Returns transaction date
string Transaction::getDate() const
{
    return date;
}

// Returns account name
string Transaction::getAccountName() const
{
    return AccountName;
}

// Returns transaction description
string Transaction::getDescription() const
{
    return description;
}

// Changes transaction amount
void Transaction::setAmount(double amount)
{
    if (amount > 0)
    {
        this->amount = amount;
    }
}

// Changes transaction category
void Transaction::setCategory(string category)
{
    this->category = category;
}

// Changes transaction date
void Transaction::setDate(string date)
{
    this->date = date;
}

// Changes account name
void Transaction::setAccountName(string accountName)
{
    this->AccountName = AccountName;
}

// Changes transaction description
void Transaction::setDescription(string description)
{
    this->description = description;
}

// Displays transaction information
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