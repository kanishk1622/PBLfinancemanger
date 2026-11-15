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

   
    bool setGoal(int id, const string& name, double target, double savings,
                 double monthly, const string& date);

   
    int getGoalId() const;
    string getGoalName() const;
    double getTargetAmount() const;
    double getCurrentSavings() const;
    double getMonthlySaving() const;
    string getTargetDate() const;

    double calculateProgress() const;
    double calculateRemaining() const;
    int calculateRequiredMonths() const;   

    void displayGoal() const;
};

#endif