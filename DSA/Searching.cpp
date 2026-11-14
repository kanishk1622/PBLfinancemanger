#include <iostream>
#include <cctype>
#include "Searching.h"

using namespace std;

namespace
{
    // Case-insensitive comparison so "food" matches "Food".
    bool equalsIgnoreCase(const string& a, const string& b)
    {
        if (a.size() != b.size())
            return false;

        for (size_t i = 0; i < a.size(); i++)
        {
            if (tolower(static_cast<unsigned char>(a[i])) !=
                tolower(static_cast<unsigned char>(b[i])))
                return false;
        }
        return true;
    }
}

Transaction* Searching::searchByID(TransactionList& transactions, int id)
{
    TransactionNode* current = transactions.getHead();

    while (current != nullptr)
    {
        if (current->data.getTransactionId() == id)
            return &(current->data);

        current = current->next;
    }

    return nullptr;
}

int Searching::searchByCategory(TransactionList& transactions, const string& category)
{
    int found = 0;
    TransactionNode* current = transactions.getHead();

    while (current != nullptr)
    {
        if (equalsIgnoreCase(current->data.getCategory(), category))
        {
            current->data.displayTransaction();
            cout << "-----------------------------" << endl;
            found++;
        }
        current = current->next;
    }

    if (found == 0)
        cout << "No transactions found in category \"" << category << "\"." << endl;

    return found;
}

int Searching::searchByType(TransactionList& transactions, const string& type)
{
    int found = 0;
    TransactionNode* current = transactions.getHead();

    while (current != nullptr)
    {
        if (equalsIgnoreCase(current->data.getType(), type))
        {
            current->data.displayTransaction();
            cout << "-----------------------------" << endl;
            found++;
        }
        current = current->next;
    }

    if (found == 0)
        cout << "No " << type << " transactions found." << endl;

    return found;
}