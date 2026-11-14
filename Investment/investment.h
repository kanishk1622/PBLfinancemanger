#ifndef INVESTMENT_H
#define INVESTMENT_H

#include <iostream>
#include <string>
using namespace std;

// Abstract base class: cannot be instantiated because getType() is pure virtual.
class Investment {
protected:
    string name;
    double amountInvested;
    double currentValue;
    double quantity;

public:
    // Invalid (negative) values are stored as 0. Callers should check
    // isValidInput() first so the user can be told what was wrong.
    Investment(string n, double invested, double current, double qty);
    virtual ~Investment();   // virtual so 'delete basePointer' runs the right destructor

    static bool isValidInput(const string& n, double invested, double current, double qty);

    // display() is written once here. It calls getType(), which is virtual, so the
    // correct type name ("Stock", "Gold", ...) is printed at runtime: polymorphism.
    // It is still virtual so a derived class can extend it if needed.
    virtual void display() const;
    virtual string getType() const = 0;   // pure virtual: every derived class must define it

    // const: these functions only read the object, never modify it
    double getAmountInvested() const;
    double getCurrentValue() const;
    double getQuantity() const;
    string getName() const;

    bool updateCurrentValue(double newValue);   // false if newValue < 0

    double profitLoss() const;
    double profitLossPercent() const;            // 0 if nothing invested
};

class Stock : public Investment {
public:
    Stock(string n, double invested, double current, double qty);
    string getType() const override;
};

class MutualFund : public Investment {
public:
    MutualFund(string n, double invested, double current, double qty);
    string getType() const override;
};

class FixedDeposit : public Investment {
public:
    FixedDeposit(string n, double invested, double current, double qty);
    string getType() const override;
};

class Gold : public Investment {
public:
    Gold(string n, double invested, double current, double qty);
    string getType() const override;
};

class OtherInvestment : public Investment {
public:
    OtherInvestment(string n, double invested, double current, double qty);
    string getType() const override;
};

#endif