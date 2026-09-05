#ifndef GOALS_H
#define GOALS_H

#include <string>
using namespace std;

class Goal
{
private:
    int goalId;
    string goalName;
    double targetAmount;
    double currentSavings;
    double monthlySaving;
    string targetDate;

public:
    Goal();

    void createGoal();
    void updateGoal();

    double calculateProgress() const;
    double calculateRemaining() const;
    int calculateRequiredMonths() const;

    void displayGoal() const;
};

#endif