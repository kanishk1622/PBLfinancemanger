#include "Investment.h"

Investment::Investment(string n, double invested, double current, double qty) {
    name = n;
    amountInvested = invested;
    currentValue = current;
    quantity = qty;
}

Investment::~Investment() {
}

double Investment::getAmountInvested() const {
    return amountInvested;
}

double Investment::getCurrentValue() const {
    return currentValue;
}

double Investment::getQuantity() const {
    return quantity;
}

string Investment::getName() const {
    return name;
}

void Investment::updateCurrentValue(double newValue) {
    currentValue = newValue;
}

double Investment::profitLoss() const {
    return currentValue - amountInvested;
}

Stock::Stock(string n, double invested, double current, double qty)
    : Investment(n, invested, current, qty) {
}

void Stock::display() const {
    cout << "\nInvestment Type : Stock";
    cout << "\nName            : " << name;
    cout << "\nAmount Invested : " << amountInvested;
    cout << "\nCurrent Value   : " << currentValue;
    cout << "\nQuantity        : " << quantity;

    double pl = profitLoss();

    if (pl >= 0)
        cout << "\nProfit          : " << pl;
    else
        cout << "\nLoss            : " << -pl;

    cout << "\n";
}

string Stock::getType() const {
    return "Stock";
}


MutualFund::MutualFund(string n, double invested, double current, double qty)
    : Investment(n, invested, current, qty) {
}

void MutualFund::display() const {
    cout << "\nInvestment Type : Mutual Fund";
    cout << "\nName            : " << name;
    cout << "\nAmount Invested : " << amountInvested;
    cout << "\nCurrent Value   : " << currentValue;
    cout << "\nQuantity/Units  : " << quantity;

    double pl = profitLoss();

    if (pl >= 0)
        cout << "\nProfit          : " << pl;
    else
        cout << "\nLoss            : " << -pl;

    cout << "\n";
}

string MutualFund::getType() const {
    return "Mutual Fund";
}

FixedDeposit::FixedDeposit(string n, double invested, double current, double qty)
    : Investment(n, invested, current, qty) {
}

void FixedDeposit::display() const {
    cout << "\nInvestment Type : Fixed Deposit";
    cout << "\nName            : " << name;
    cout << "\nAmount Invested : " << amountInvested;
    cout << "\nCurrent Value   : " << currentValue;
    cout << "\nQuantity        : " << quantity;

    double pl = profitLoss();

    if (pl >= 0)
        cout << "\nProfit          : " << pl;
    else
        cout << "\nLoss            : " << -pl;

    cout << "\n";
}

string FixedDeposit::getType() const {
    return "Fixed Deposit";
}


Gold::Gold(string n, double invested, double current, double qty)
    : Investment(n, invested, current, qty) {
}

void Gold::display() const {
    cout << "\nInvestment Type : Gold";
    cout << "\nName            : " << name;
    cout << "\nAmount Invested : " << amountInvested;
    cout << "\nCurrent Value   : " << currentValue;
    cout << "\nQuantity        : " << quantity;

    double pl = profitLoss();

    if (pl >= 0)
        cout << "\nProfit          : " << pl;
    else
        cout << "\nLoss            : " << -pl;

    cout << "\n";
}

string Gold::getType() const {
    return "Gold";
}


OtherInvestment::OtherInvestment(string n, double invested, double current, double qty)
    : Investment(n, invested, current, qty) {
}

void OtherInvestment::display() const {
    cout << "\nInvestment Type : Other";
    cout << "\nName            : " << name;
    cout << "\nAmount Invested : " << amountInvested;
    cout << "\nCurrent Value   : " << currentValue;
    cout << "\nQuantity        : " << quantity;

    double pl = profitLoss();

    if (pl >= 0)
        cout << "\nProfit          : " << pl;
    else
        cout << "\nLoss            : " << -pl;

    cout << "\n";
}

string OtherInvestment::getType() const {
    return "Other";
}