#ifndef SEARCHING_H
#define SEARCHING_H

#include "Linked_list.h"
#include <string>

// Linear search over the transaction linked list: O(n) for every function.
// (For O(1) average lookup by ID, see HashTable.)
class Searching
{
public:
    // Returns a pointer to the matching transaction, or nullptr if not found.
    static Transaction* searchByID(
        TransactionList& transactions, int id);

    // Print every match (case-insensitive) and return how many were found.
    static int searchByCategory(
        TransactionList& transactions, const std::string& category);

    static int searchByType(
        TransactionList& transactions, const std::string& type);
};

#endif