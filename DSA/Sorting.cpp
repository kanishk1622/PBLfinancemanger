
#include <iostream>
#include <string>
#include "Sorting.h"

using namespace std;

void Sorting::sortByAmount(TransactionList& transactions)
{
    bool swapped;
    TransactionNode* end = nullptr;

    if (transactions.getHead() == nullptr)
        return;

    do
    {
        swapped = false;
        TransactionNode* current = transactions.getHead();

        while (current->next != end)
        {
            if (current->data.getAmount() >
                current->next->data.getAmount())
            {
                Transaction temp = current->data;
                current->data = current->next->data;
                current->next->data = temp;
                swapped = true;
            }

            current = current->next;
        }

        end = current;

    } while (swapped);

    cout << "\nSorted by amount (ascending):\n";
    transactions.display();
}

void Sorting::sortByCategory(TransactionList& transactions)
{
    bool swapped;
    TransactionNode* end = nullptr;

    if (transactions.getHead() == nullptr)
        return;

    do
    {
        swapped = false;
        TransactionNode* current = transactions.getHead();

        while (current->next != end)
        {
            if (current->data.getCategory() >
                current->next->data.getCategory())
            {
                Transaction temp = current->data;
                current->data = current->next->data;
                current->next->data = temp;
                swapped = true;
            }

            current = current->next;
        }

        end = current;

    } while (swapped);

    cout << "\nSorted by category (A-Z):\n";
    transactions.display();
}

void Sorting::sortByDate(TransactionList& transactions)
{
    bool swapped;
    TransactionNode* end = nullptr;

    if (transactions.getHead() == nullptr)
        return;

    do
    {
        swapped = false;
        TransactionNode* current = transactions.getHead();

        while (current->next != end)
        {
            if (current->data.getDate() >
                current->next->data.getDate())
            {
                Transaction temp = current->data;
                current->data = current->next->data;
                current->next->data = temp;
                swapped = true;
            }

            current = current->next;
        }

        end = current;

    } while (swapped);

    cout << "\nSorted by date (oldest first):\n";
    transactions.display();
}
