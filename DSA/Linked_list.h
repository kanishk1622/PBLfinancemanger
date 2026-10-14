#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include "../Transactions/transactions.h"


struct TransactionNode
{
    Transaction data;
    TransactionNode* next;

    TransactionNode(const Transaction& transaction)
    {
        data = transaction;
        next = nullptr;
    }
};

class TransactionList
{
private:
    TransactionNode* head;

public:
    TransactionList();

    void insert(const Transaction& transaction);

    bool remove(int transactionId);

    Transaction* searchById(int transactionId);

    void display() const;

    TransactionNode* getHead() const;
};

#endif