
#ifndef SORTING_H
#define SORTING_H

#include "Linked_list.h"

class Sorting
{
public:
 
    static void sortByAmount(TransactionList& transactions);
    static void sortByCategory(TransactionList& transactions);
    static void sortByDate(TransactionList& transactions);

private:
   
    typedef bool (*ComesAfter)(const Transaction& a, const Transaction& b);

   
    static void mergeSortList(TransactionList& transactions, ComesAfter comesAfter);


    static void mergeSort(const Transaction* items[], const Transaction* temp[],
                          int left, int right, ComesAfter comesAfter);
};

#endif
