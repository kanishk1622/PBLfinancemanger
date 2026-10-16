#ifndef INVESTMENT_H
#define INVESTMENT_H

#include <iostream>
#include <string>
using namespace std;

class Investment {
protected:
    string name;
    double amountInvested;
    double currentValue;
    double quantity;

public:
    Investment(string n, double invested, double current, double qty);
    virtual ~Investment();

    // const means it will not modif current object's data members
    // only reading is allowed no changing
    virtual void display() const = 0;
    virtual string getType() const = 0;

    double getAmountInvested() const;
    double getCurrentValue() const;
    double getQuantity() const;
    string getName() const;

    void updateCurrentValue(double newValue);

    double profitLoss() const;
};

class Stock : public Investment {
public:
    Stock(string n, double invested, double current, double qty);

    // const override written cause it ovverides a constant function which is virtual in parent class
    // cannot remove const in a function as non const member function cannot call const member function 
    void display() const override;
    string getType() const override;
};


class MutualFund : public Investment {
public:
    MutualFund(string n, double invested, double current, double qty);

    void display() const override;
    string getType() const override;
};


class FixedDeposit : public Investment {
public:
    FixedDeposit(string n, double invested, double current, double qty);

    void display() const override;
    string getType() const override;
};

class Gold : public Investment {
public:
    Gold(string n, double invested, double current, double qty);

    void display() const override;
    string getType() const override;
};

class OtherInvestment : public Investment {
public:
    OtherInvestment(string n, double invested, double current, double qty);

    void display() const override;
    string getType() const override;
};

#endif