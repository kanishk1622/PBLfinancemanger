#include "investment.h"

Investment::Investment(string n, double invested, double current, double qty) {
    name = n;
    amountInvested = invested < 0 ? 0 : invested;
    currentValue = current < 0 ? 0 : current;
    quantity = qty < 0 ? 0 : qty;
}

Investment::~Investment() {
}

bool Investment::isValidInput(const string& n, double invested, double current, double qty) {
    return !n.empty() && invested > 0 && current >= 0 && qty > 0;
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

bool Investment::updateCurrentValue(double newValue) {
    if (newValue < 0)
        return false;

    currentValue = newValue;
    return true;
}

double Investment::profitLoss() const {
    return currentValue - amountInvested;
}

double Investment::profitLossPercent() const {
    if (amountInvested <= 0)
        return 0;
    return (profitLoss() / amountInvested) * 100.0;
}

void Investment::display() const {
    cout << "\nInvestment Type : " << getType();   // virtual call
    cout << "\nName            : " << name;
    cout << "\nAmount Invested : " << amountInvested;
    cout << "\nCurrent Value   : " << currentValue;
    cout << "\nQuantity        : " << quantity;

    double pl = profitLoss();

    if (pl >= 0)
        cout << "\nProfit          : " << pl;
    else
        cout << "\nLoss            : " << -pl;

    cout << " (" << profitLossPercent() << "%)";
    cout << "\n";
}

// ---------- Derived classes: each only defines its type ----------

Stock::Stock(string n, double invested, double current, double qty)
    : Investment(n, invested, current, qty) {
}

string Stock::getType() const {
    return "Stock";
}

MutualFund::MutualFund(string n, double invested, double current, double qty)
    : Investment(n, invested, current, qty) {
}

string MutualFund::getType() const {
    return "Mutual Fund";
}

FixedDeposit::FixedDeposit(string n, double invested, double current, double qty)
    : Investment(n, invested, current, qty) {
}

string FixedDeposit::getType() const {
    return "Fixed Deposit";
}

Gold::Gold(string n, double invested, double current, double qty)
    : Investment(n, invested, current, qty) {
}

string Gold::getType() const {
    return "Gold";
}

OtherInvestment::OtherInvestment(string n, double invested, double current, double qty)
    : Investment(n, invested, current, qty) {
}

string OtherInvestment::getType() const {
    return "Other";
}