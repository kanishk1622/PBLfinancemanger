#ifndef BUDGET_H
#define BUDGET_H

#include <string>
using namespace std;

class Budget
{
private:
    string category;
    double limit;

public:
    Budget();

    void createBudget(string cat);
    void displayBudget();

    string getCategory();
    double getLimit();
};

#endif