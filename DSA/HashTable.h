#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "Linked_list.h"

// Hash function: id % TABLE_SIZE, with TABLE_SIZE prime so IDs spread evenly.
// Average cost of insert / find / remove: O(1); worst case O(n) if all IDs collide.

class HashTable
{
public:
    static const int TABLE_SIZE = 1009;   // a prime number

private:
    struct HashEntry
    {
        int key;                    // transaction ID
        Transaction* value;         // where that transaction lives in the list
        HashEntry* next;            // next entry in the same bucket (chain)
    };

    HashEntry* buckets[TABLE_SIZE];
    int count;

    int hash(int key) const;

public:
    HashTable();
    ~HashTable();

    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;

    bool insert(int key, Transaction* value);   // false if the key already exists (duplicate ID)
    Transaction* find(int key) const;           // nullptr if not found
    bool remove(int key);                       // false if not found
    bool contains(int key) const;

    // Empties the table, then re-adds every transaction currently in the list.
    void rebuild(TransactionList& transactions);
    void clear();

    int size() const;
    double loadFactor() const;   // entries / buckets; keep below ~1 for O(1) lookups
};

#endif
