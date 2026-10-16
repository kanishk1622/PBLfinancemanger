
#ifndef SORTING_H
#define SORTING_H

#include "Linked_list.h"

class Sorting
{
public:
    static void sortByAmount(TransactionList& transactions);
    static void sortByCategory(TransactionList& transactions);
    static void sortByDate(TransactionList& transactions);
};

#endif
