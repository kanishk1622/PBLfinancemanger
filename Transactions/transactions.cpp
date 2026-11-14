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
    this->AccountName = accountName;
}

void Transaction::setDescription(string description)
{
    this->description = description;
}

// Splits "DD-MM-YYYY" into numbers. Returns false if the text is not in that exact format.
static bool parseDate(const string& date, int& day, int& month, int& year)
{
    if (date.length() != 10 || date[2] != '-' || date[5] != '-')
        return false;

    for (int i = 0; i < 10; i++)
    {
        if (i == 2 || i == 5)
            continue;
        if (date[i] < '0' || date[i] > '9')
            return false;
    }

    day = (date[0] - '0') * 10 + (date[1] - '0');
    month = (date[3] - '0') * 10 + (date[4] - '0');
    year = (date[6] - '0') * 1000 + (date[7] - '0') * 100 +
           (date[8] - '0') * 10 + (date[9] - '0');
    return true;
}

bool Transaction::isValidDate(const string& date)
{
    int day, month, year;
    if (!parseDate(date, day, month, year))
        return false;

    if (year < 1 || month < 1 || month > 12 || day < 1)
        return false;

    int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    bool leapYear = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    if (leapYear)
        daysInMonth[1] = 29;

    return day <= daysInMonth[month - 1];
}

int Transaction::getDateKey() const
{
    if (!isValidDate(date))
        return 0;

    int day, month, year;
    parseDate(date, day, month, year);
    return year * 10000 + month * 100 + day;
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