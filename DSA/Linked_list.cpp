#include <iostream>
#include "Linked_list.h"

using namespace std;

TransactionList::TransactionList()
{
    head = nullptr;
}

TransactionList::~TransactionList()
{
    while (head != nullptr)
    {
        TransactionNode* temp = head;
        head = head->next;
        delete temp;
    }
}

void TransactionList::insert(const Transaction& transaction)
{
    TransactionNode* newNode = new TransactionNode(transaction);
    if (head == nullptr)
    {
        head = newNode;
        return;
    }

    TransactionNode* temp = head;

    while (temp->next != nullptr)
    {
        temp = temp->next;
    }

    temp->next = newNode;
}

bool TransactionList::remove(int transactionId)
{
    
    if (head == nullptr)
    {
        return false;
    }

    if (head->data.getTransactionId() == transactionId)
    {
        TransactionNode* temp = head;
        head = head->next;

        delete temp;

        return true;
    }

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

    return false;
}

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

TransactionNode* TransactionList::getHead() const
{
    return head;
}