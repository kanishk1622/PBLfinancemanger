#include "Budget.h"
#include <iostream>

using namespace std;

Budget::Budget()
{
    category = "";
    limit = 0;
}

void Budget::createBudget(string cat)
{
    category = cat;

    cout << "Enter budget for " << category << ": ";
    cin >> limit;
}

void Budget::displayBudget()
{
    cout << category << " : Rs. " << limit << endl;
}

string Budget::getCategory()
{
    return category;
}

double Budget::getLimit()
{
    return limit;
}