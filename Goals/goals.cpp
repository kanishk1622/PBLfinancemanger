#include "goals.h"
#include <iostream>

using namespace std;

static bool validDateFormat(string date)
{
    if (date.length() != 10 || date[2] != '-' || date[5] != '-')
        return false;

    for (int i = 0; i < 10; i++)
    {
        if (i == 2 || i == 5)
            continue;

        if (date[i] < '0' || date[i] > '9')
            return false;
    }

    int day = (date[0] - '0') * 10 + (date[1] - '0');
    int month = (date[3] - '0') * 10 + (date[4] - '0');
    int year = (date[6] - '0') * 1000 + (date[7] - '0') * 100 +
               (date[8] - '0') * 10 + (date[9] - '0');

    if (day < 1 || day > 31 || month < 1 || month > 12 || year < 1)
        return false;

    return true;
}

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
    cout << "\n--- Create Financial Goal ---\n";

    cout << "Enter Goal ID (positive number): ";
    while (!(cin >> goalId) || goalId <= 0)
    {
        cout << "Invalid ID. Enter a positive whole number: ";
        cin.clear();
        cin.ignore(1000, '\n');
    }
    cin.ignore(1000, '\n');

    cout << "Enter Goal Name: ";
    getline(cin, goalName);
    while (goalName.empty())
    {
        cout << "Goal name cannot be empty. Enter Goal Name: ";
        getline(cin, goalName);
    }

    cout << "Enter Target Amount (greater than 0): Rs. ";
    while (!(cin >> targetAmount) || targetAmount <= 0)
    {
        cout << "Invalid amount. Enter a number greater than 0: Rs. ";
        cin.clear();
        cin.ignore(1000, '\n');
    }

    cout << "Enter Current Savings (0 or more): Rs. ";
    while (!(cin >> currentSavings) || currentSavings < 0)
    {
        cout << "Invalid savings. Enter 0 or a positive amount: Rs. ";
        cin.clear();
        cin.ignore(1000, '\n');
    }

    cout << "Enter Monthly Saving Amount (0 or more): Rs. ";
    while (!(cin >> monthlySaving) || monthlySaving < 0)
    {
        cout << "Invalid monthly saving. Enter 0 or a positive amount: Rs. ";
        cin.clear();
        cin.ignore(1000, '\n');
    }
    cin.ignore(1000, '\n');

    cout << "Enter Target Date (DD-MM-YYYY): ";
    getline(cin, targetDate);
    while (!validDateFormat(targetDate))
    {
        cout << "Invalid date format. Example: 25-12-2026. Enter again: ";
        getline(cin, targetDate);
    }

    cout << "Goal created successfully.\n";
}

void Goal::updateGoal()
{
    cout << "\n--- Update Goal Savings ---\n";

    cout << "Enter new Current Savings (0 or more): Rs. ";
    while (!(cin >> currentSavings) || currentSavings < 0)
    {
        cout << "Invalid savings. Enter 0 or a positive amount: Rs. ";
        cin.clear();
        cin.ignore(1000, '\n');
    }

    cout << "Enter new Monthly Saving Amount (0 or more): Rs. ";
    while (!(cin >> monthlySaving) || monthlySaving < 0)
    {
        cout << "Invalid monthly saving. Enter 0 or a positive amount: Rs. ";
        cin.clear();
        cin.ignore(1000, '\n');
    }
    cin.ignore(1000, '\n');

    cout << "Goal savings updated successfully.\n";
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
    double remaining = calculateRemaining();

    if (remaining <= 0)
        return 0;

    if (monthlySaving <= 0)
        return -1;

    int months = static_cast<int>(remaining / monthlySaving);
    if (months * monthlySaving < remaining)
        months++;

    return months;
}

void Goal::displayGoal() const
{
    cout << "\n====================================\n";
    cout << "          FINANCIAL GOAL\n";
    cout << "====================================\n";

    cout << "Goal ID           : " << goalId << '\n';
    cout << "Goal Name         : " << goalName << '\n';
    cout << "Target Amount     : Rs. " << targetAmount << '\n';
    cout << "Current Savings   : Rs. " << currentSavings << '\n';
    cout << "Remaining Amount  : Rs. " << calculateRemaining() << '\n';
    cout << "Monthly Saving    : Rs. " << monthlySaving << '\n';
    cout << "Target Date       : " << targetDate << '\n';
    cout << "Progress          : " << calculateProgress() << "%\n";

    int months = calculateRequiredMonths();
    if (months == -1)
        cout << "Required Months   : Cannot calculate because monthly saving is zero.\n";
    else
        cout << "Required Months   : " << months << '\n';

    if (calculateRemaining() == 0)
        cout << "Status            : Goal achieved!\n";
    else if (calculateProgress() >= 80)
        cout << "Status            : Almost there!\n";
    else
        cout << "Status            : In progress\n";

    cout << "====================================\n";
}
