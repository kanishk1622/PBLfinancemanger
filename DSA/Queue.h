#ifndef QUEUE_H
#define QUEUE_H

#include "../Transactions/transactions.h"

class TransactionQueue
{
public:
    static const int CAPACITY = 500;

private:
    Transaction items[CAPACITY];
    int front;   
    int rear;    
    int count;

public:
    TransactionQueue();

    bool enqueue(const Transaction& transaction);   
    bool dequeue(Transaction& transaction);        
    bool peek(Transaction& transaction) const;      

    bool isEmpty() const;
    bool isFull() const;
    int size() const;
    void clear();

    void display() const;   
};

#endif
