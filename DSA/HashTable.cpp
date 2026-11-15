#include "HashTable.h"

HashTable::HashTable()
{
    for (int i = 0; i < TABLE_SIZE; i++)
        buckets[i] = nullptr;
    count = 0;
}

HashTable::~HashTable()
{
    clear();
}

int HashTable::hash(int key) const
{
    // The extra "+ TABLE_SIZE) % TABLE_SIZE" keeps the index positive for negative keys.
    return ((key % TABLE_SIZE) + TABLE_SIZE) % TABLE_SIZE;
}

bool HashTable::insert(int key, Transaction* value)
{
    if (value == nullptr || contains(key))
        return false;

    int index = hash(key);

    // Insert at the head of the chain: O(1).
    HashEntry* entry = new HashEntry;
    entry->key = key;
    entry->value = value;
    entry->next = buckets[index];
    buckets[index] = entry;

    count++;
    return true;
}

Transaction* HashTable::find(int key) const
{
    HashEntry* current = buckets[hash(key)];

    // Only this one bucket is searched, not the whole data set.
    while (current != nullptr)
    {
        if (current->key == key)
            return current->value;
        current = current->next;
    }

    return nullptr;
}

bool HashTable::remove(int key)
{
    int index = hash(key);
    HashEntry* current = buckets[index];
    HashEntry* previous = nullptr;

    while (current != nullptr)
    {
        if (current->key == key)
        {
            if (previous == nullptr)
                buckets[index] = current->next;   // removing the first entry in the chain
            else
                previous->next = current->next;

            delete current;
            count--;
            return true;
        }

        previous = current;
        current = current->next;
    }

    return false;
}

bool HashTable::contains(int key) const
{
    return find(key) != nullptr;
}

void HashTable::rebuild(TransactionList& transactions)
{
    clear();

    TransactionNode* node = transactions.getHead();
    while (node != nullptr)
    {
        insert(node->data.getTransactionId(), &(node->data));
        node = node->next;
    }
}

void HashTable::clear()
{
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        HashEntry* current = buckets[i];
        while (current != nullptr)
        {
            HashEntry* temp = current;
            current = current->next;
            delete temp;
        }
        buckets[i] = nullptr;
    }
    count = 0;
}

int HashTable::size() const
{
    return count;
}

double HashTable::loadFactor() const
{
    return static_cast<double>(count) / TABLE_SIZE;
}
