#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <string>
using namespace std;

class Transaction
{
private:
    int transactionId;
    string type;
    double amount;
    string category;
    string date;
    string AccountName;
    string description;

public:
    Transaction();
    Transaction(int transactionId,
                string type,
                double amount,
                string category,
                string date,
                string AccountName,
                string description);

    int getTransactionId() const;
    string getType() const;
    double getAmount() const;
    string getCategory() const;
    string getDate() const;
    string getAccountName() const;
    string getDescription() const;

    void setAmount(double amount);
    void setCategory(string category);
    void setDate(string date);
    void setAccountName(string AccountName);
    void setDescription(string description);

    // isValidDate checks the format AND the calendar (e.g. rejects 31-02-2026).
    static bool isValidDate(const string& date);
    // Returns 0 if the stored date is invalid.
    int getDateKey() const;
    void displayTransaction() const;
};

#endif