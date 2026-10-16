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

    // Getters
    int getTransactionId() const;
    string getType() const;
    double getAmount() const;
    string getCategory() const;
    string getDate() const;
    string getAccountName() const;
    string getDescription() const;

    // Setters
    void setAmount(double amount);
    void setCategory(string category);
    void setDate(string date);
    void setAccountName(string AccountName);
    void setDescription(string description);

    // Display 
    void displayTransaction() const;
};

#endif