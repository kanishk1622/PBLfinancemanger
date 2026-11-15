#ifndef HEAP_H
#define HEAP_H

#include "../Transactions/transactions.h"

struct HeapItem
{
    Transaction transaction;
    int key;        
    int sequence;   
};

// MIN-HEAP of scheduled (future) transactions, ordered by due date.
// The bill with the EARLIEST date is always at the top (index 0).
//

class TransactionHeap
{
public:
    static const int CAPACITY = 500;

private:
    HeapItem items[CAPACITY];
    int count;
    int nextSequence;

    // True if a should be nearer the top than b: earlier date, or same date but added first.
    static bool comesBefore(const HeapItem& a, const HeapItem& b);
    void swapItems(int i, int j);
    void siftUp(int index);
    void siftDown(int index);
    int indexOfSequence(int sequence) const;   // linear search: array index, or -1

public:
    TransactionHeap();

    bool insert(const Transaction& transaction);   // false if full or the date is invalid
    bool removeMin(Transaction& transaction);       // false if empty
    bool peekMin(Transaction& transaction) const;   // false if empty; does not remove
    int peekMinKey() const;                          // YYYYMMDD of the top item, 0 if empty
    int peekMinSequence() const;                     // arrival number of the top item, -1 if empty

    // Each bill keeps its unique arrival number (sequence) while it is in the heap,
    // so one particular bill can be found or removed, not only the top one.
    bool findBySequence(int sequence, Transaction& transaction) const;   // O(n); false if not found
    // Remove ANY bill: find it (O(n)), move the last item into its place, then
    // sift that item up OR down to repair the heap order (O(log n)).
    bool removeBySequence(int sequence, Transaction& transaction);       // false if not found

    bool isEmpty() const;
    bool isFull() const;
    int size() const;
    void clear();
};

#endif
