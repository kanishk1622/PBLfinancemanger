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
    TransactionNode* tail;   // last node: lets insert() add at the end in O(1)
                             // (nullptr when the list is empty)

public:
    TransactionList();
    ~TransactionList();   // frees every node

    // Copying is disabled: a default copy would make two lists share the same
    // nodes, and both destructors would then delete them (double free).
    TransactionList(const TransactionList&) = delete;
    TransactionList& operator=(const TransactionList&) = delete;

    Transaction* insert(const Transaction& transaction);   // returns the stored copy

    bool remove(int transactionId);

    Transaction* searchById(int transactionId);

    void display() const;

    TransactionNode* getHead() const;
};

#endif