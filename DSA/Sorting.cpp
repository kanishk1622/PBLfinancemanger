
#include <iostream>
#include <string>
#include "Sorting.h"

using namespace std;



namespace
{
    bool amountAfter(const Transaction& a, const Transaction& b)
    {
        return a.getAmount() > b.getAmount();
    }

    bool categoryAfter(const Transaction& a, const Transaction& b)
    {
        return a.getCategory() > b.getCategory();
    }

    // Compare YYYYMMDD numbers, not "DD-MM-YYYY" strings:
    // as strings, "05-01-2027" < "20-12-2025" because '0' < '2'.
    bool dateAfter(const Transaction& a, const Transaction& b)
    {
        return a.getDateKey() > b.getDateKey();
    }
}



void Sorting::mergeSort(const Transaction* items[], const Transaction* temp[],
                        int left, int right, ComesAfter comesAfter)
{
    if (left >= right)
        return;   // 0 or 1 item: already sorted

    int middle = left + (right - left) / 2;
    mergeSort(items, temp, left, middle, comesAfter);        // sort the left half
    mergeSort(items, temp, middle + 1, right, comesAfter);   // sort the right half

    // Merge: repeatedly take the smaller front item of the two halves.
    int a = left;          // front of the left half
    int b = middle + 1;    // front of the right half
    int k = left;          // next free place in temp
    while (a <= middle && b <= right)
    {
        // Take from the RIGHT only if it is strictly smaller. On a tie the
        // LEFT item goes first, so equal items keep their order (stable).
        if (comesAfter(*items[a], *items[b]))
            temp[k++] = items[b++];
        else
            temp[k++] = items[a++];
    }
    while (a <= middle)          // copy what is left of the left half
        temp[k++] = items[a++];
    while (b <= right)           // copy what is left of the right half
        temp[k++] = items[b++];

    for (k = left; k <= right; k++)
        items[k] = temp[k];
}

void Sorting::mergeSortList(TransactionList& transactions, ComesAfter comesAfter)
{
    int n = 0;
    for (TransactionNode* node = transactions.getHead(); node != nullptr; node = node->next)
        n++;
    if (n < 2)
        return;

    // a) pointers to the transactions, in list order
    const Transaction** items = new const Transaction*[n];
    const Transaction** temp = new const Transaction*[n];
    int i = 0;
    for (TransactionNode* node = transactions.getHead(); node != nullptr; node = node->next)
        items[i++] = &node->data;

    // b) sort the pointers
    mergeSort(items, temp, 0, n - 1, comesAfter);

    // c) copy the transactions in sorted order FIRST, then write them back.
    //    (Writing straight from the pointers would overwrite nodes that later
    //    pointers still point to.)
    Transaction* sorted = new Transaction[n];
    for (i = 0; i < n; i++)
        sorted[i] = *items[i];

    i = 0;
    for (TransactionNode* node = transactions.getHead(); node != nullptr; node = node->next)
        node->data = sorted[i++];

    delete[] items;
    delete[] temp;
    delete[] sorted;
}

void Sorting::sortByAmount(TransactionList& transactions)
{
    if (transactions.getHead() == nullptr)
        return;

    mergeSortList(transactions, amountAfter);

    cout << "\nSorted by amount (ascending):\n";
    transactions.display();
}

void Sorting::sortByCategory(TransactionList& transactions)
{
    if (transactions.getHead() == nullptr)
        return;

    mergeSortList(transactions, categoryAfter);

    cout << "\nSorted by category (A-Z):\n";
    transactions.display();
}

void Sorting::sortByDate(TransactionList& transactions)
{
    if (transactions.getHead() == nullptr)
        return;

    mergeSortList(transactions, dateAfter);

    cout << "\nSorted by date (oldest first):\n";
    transactions.display();
}
