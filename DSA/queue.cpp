#include <iostream>
#include "Queue.h"

using namespace std;

TransactionQueue::TransactionQueue()
{
    clear();
}

bool TransactionQueue::enqueue(const Transaction& transaction)
{
    if (isFull())
        return false;

    rear = (rear + 1) % CAPACITY;   // wrap around to index 0 after the last slot
    items[rear] = transaction;
    count++;
    return true;
}

bool TransactionQueue::dequeue(Transaction& transaction)
{
    if (isEmpty())
        return false;

    transaction = items[front];
    front = (front + 1) % CAPACITY;
    count--;
    return true;
}

bool TransactionQueue::peek(Transaction& transaction) const
{
    if (isEmpty())
        return false;

    transaction = items[front];
    return true;
}

bool TransactionQueue::isEmpty() const
{
    return count == 0;
}

bool TransactionQueue::isFull() const
{
    return count == CAPACITY;
}

int TransactionQueue::size() const
{
    return count;
}

void TransactionQueue::clear()
{
    front = 0;
    rear = CAPACITY - 1;  
}

void TransactionQueue::display() const
{
    if (isEmpty())
    {
        cout << "No pending transactions." << endl;
        return;
    }

    for (int i = 0; i < count; i++)
    {
        int index = (front + i) % CAPACITY;
        cout << "[Pending " << (i + 1) << "]" << endl;
        items[index].displayTransaction();
        cout << "-----------------------------" << endl;
    }
}