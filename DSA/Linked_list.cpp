#include <iostream>
#include "TransactionList.h"

using namespace std;

// Constructor
TransactionList::TransactionList()
{
    head = nullptr;
}

// Insert a transaction at the end of the list
void TransactionList::insert(const Transaction& transaction)
{
    TransactionNode* newNode = new TransactionNode(transaction);

    // If the list is empty
    if (head == nullptr)
    {
        head = newNode;
        return;
    }

    // Traverse to the last node
    TransactionNode* temp = head;

    while (temp->next != nullptr)
    {
        temp = temp->next;
    }

    // Add the new node at the end
    temp->next = newNode;
}

// Remove a transaction using its ID
bool TransactionList::remove(int transactionId)
{
    // If the list is empty
    if (head == nullptr)
    {
        return false;
    }

    // If the first node contains the transaction
    if (head->data.getTransactionId() == transactionId)
    {
        TransactionNode* temp = head;
        head = head->next;

        delete temp;

        return true;
    }

    // Search for the transaction
    TransactionNode* current = head;

    while (current->next != nullptr)
    {
        if (current->next->data.getTransactionId() == transactionId)
        {
            TransactionNode* temp = current->next;

            current->next = temp->next;

            delete temp;

            return true;
        }

        current = current->next;
    }

    // Transaction not found
    return false;
}

// Search for a transaction using its ID
Transaction* TransactionList::searchById(int transactionId)
{
    TransactionNode* temp = head;

    while (temp != nullptr)
    {
        if (temp->data.getTransactionId() == transactionId)
        {
            return &(temp->data);
        }

        temp = temp->next;
    }

    return nullptr;
}

// Display all transactions
void TransactionList::display() const
{
    TransactionNode* temp = head;

    if (temp == nullptr)
    {
        cout << "No transactions found." << endl;
        return;
    }

    while (temp != nullptr)
    {
        temp->data.displayTransaction();
        cout << "-----------------------------" << endl;

        temp = temp->next;
    }
}

// Return the head of the list
TransactionNode* TransactionList::getHead() const
{
    return head;
}