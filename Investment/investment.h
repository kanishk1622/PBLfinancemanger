#ifndef INVESTMENT_H
#define INVESTMENT_H

#include<iostream>

using namespace std;

// BASE CLASS 
class investment{
    // we used protected because it allows us to access variables in child class (used for inheritance) 
    // which is not allowed in private
    protected:
    string name;
    double amountInvested;
    double curr_value;
    double quan;

    public:

    //code end here for now



};


#endif