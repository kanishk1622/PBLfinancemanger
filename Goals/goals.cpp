#include "goals.h"
#include <iostream>
#include <cmath>

using namespace std;

Goal::Goal()
{
    goalId = 0;
    goalName = "";
    targetAmount = 0;
    currentSavings = 0;
    monthlySaving = 0;
    targetDate = "";
}

void Goal::createGoal()
{
    cout << "\nEnter Goal ID: ";
    cin >> goalId;

    cin.ignore();

    cout << "Enter Goal Name: ";
    getline(cin, goalName);

    cout << "Enter Target Amount: ";
    cin >> targetAmount;

    cout << "Enter Current Savings: ";
    cin >> currentSavings;

    cout << "Enter Monthly Saving Amount: ";
    cin >> monthlySaving;

    cin.ignore();

    cout << "Enter Target Date (DD-MM-YYYY): ";
    getline(cin, targetDate);
}

double Goal::calculateProgress() const
{
    if (targetAmount <= 0)
        return 0;

    return (currentSavings / targetAmount) * 100;
}

double Goal::calculateRemaining() const
{
    double remaining = targetAmount - currentSavings;

    if (remaining < 0)
        return 0;

    return remaining;
}

int Goal::calculateRequiredMonths() const
{
    if (monthlySaving <= 0)
        return 0;

    return static_cast<int>(
        ceil(calculateRemaining() / monthlySaving)
    );
}

void Goal::updateGoal()
{
    cout << "\nEnter new current savings: ";
    cin >> currentSavings;

    cout << "Enter new monthly saving amount: ";
    cin >> monthlySaving;
}

void Goal::displayGoal() const
{
    cout << "\n==================================\n";
    cout << "           FINANCIAL GOAL\n";
    cout << "====================================\n";

    cout << "Goal ID           : " << goalId << endl;
    cout << "Goal Name         : " << goalName << endl;
    cout << "Target Amount     : Rs. " << targetAmount << endl;
    cout << "Current Savings   : Rs. " << currentSavings << endl;
    cout << "Remaining Amount  : Rs. "
         << calculateRemaining() << endl;
    cout << "Monthly Saving    : Rs. "
         << monthlySaving << endl;
    cout << "Target Date       : " << targetDate << endl;
    cout << "Progress          : "
         << calculateProgress() << "%" << endl;
    cout << "Required Months   : "
         << calculateRequiredMonths() << endl;

    cout << "====================================\n";
}