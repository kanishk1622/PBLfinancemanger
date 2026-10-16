
#ifndef SEARCHING_H
#define SEARCHING_H

#include "Linked_list.h"
#include <string>

class Searching
{
public:
    static Transaction* searchByID(
        TransactionList& transactions, int id);

    static void searchByCategory(
        TransactionList& transactions, const std::string& category);

    static void searchByType(
        TransactionList& transactions, const std::string& type);
};

#endif
