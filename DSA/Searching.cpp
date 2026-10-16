#include <iostream>
#include "Searching.h"

using namespace std;

Transaction* Searching::searchByID(
    TransactionList& transactions, int id)
{
    TransactionNode* current = transactions.getHead();

    while (current != nullptr)
    {
        if (current->data.getTransactionId() == id)
            return &(current->data);

        current = current->next;
    }

    cout << "Transaction not found.\n";
    return nullptr;
}

void Searching::searchByCategory(
    TransactionList& transactions, const string& category)
{
    TransactionNode* current = transactions.getHead();
    bool found = false;

    while (current != nullptr)
    {
        if (current->data.getCategory() == category)
        {
            current->data.displayTransaction();
            cout << "----------------------\n";
            found = true;
        }

        current = current->next;
    }

    if (!found)
        cout << "No transactions found for this category.\n";
}

void Searching::searchByType(
    TransactionList& transactions, const string& type)
{
    TransactionNode* current = transactions.getHead();
    bool found = false;

    while (current != nullptr)
    {
        if (current->data.getType() == type)
        {
            current->data.displayTransaction();
            cout << "----------------------\n";
            found = true;
        }

        current = current->next;
    }

    if (!found)
        cout << "No transactions found for this type.\n";
}
