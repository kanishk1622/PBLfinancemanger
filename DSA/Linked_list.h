#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include "Transaction.h"

// Node for storing a Transaction
struct TransactionNode
{
    Transaction data;
    TransactionNode* next;

    // Constructor for the node
    TransactionNode(const Transaction& transaction)
    {
        data = transaction;
        next = nullptr;
    }
};

// Linked List for storing transactions
class TransactionList
{
private:
    TransactionNode* head;

public:
    // Constructor
    TransactionList();

    // Insert a transaction into the list
    void insert(const Transaction& transaction);

    // Remove a transaction using its ID
    bool remove(int transactionId);

    // Search for a transaction using its ID
    Transaction* searchById(int transactionId);

    // Display all transactions
    void display() const;

    // Get the first node
    TransactionNode* getHead() const;
};

#endif