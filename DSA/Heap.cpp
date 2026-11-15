#include "Heap.h"

TransactionHeap::TransactionHeap()
{
    clear();
}

bool TransactionHeap::comesBefore(const HeapItem& a, const HeapItem& b)
{
    if (a.key != b.key)
        return a.key < b.key;          // earlier due date first
    return a.sequence < b.sequence;    // same date: the one added first
}

void TransactionHeap::swapItems(int i, int j)
{
    HeapItem temp = items[i];
    items[i] = items[j];
    items[j] = temp;
}

// Move the item at 'index' UP while it should come before its parent.
void TransactionHeap::siftUp(int index)
{
    while (index > 0)
    {
        int parent = (index - 1) / 2;
        if (!comesBefore(items[index], items[parent]))
            break;
        swapItems(index, parent);
        index = parent;
    }
}

// Move the item at 'index' DOWN while one of its children should come before it.
void TransactionHeap::siftDown(int index)
{
    while (true)
    {
        int left = 2 * index + 1;
        int right = 2 * index + 2;
        int smallest = index;

        if (left < count && comesBefore(items[left], items[smallest]))
            smallest = left;
        if (right < count && comesBefore(items[right], items[smallest]))
            smallest = right;

        if (smallest == index)
            break;   // both children come after it: the heap order is correct
        swapItems(index, smallest);
        index = smallest;
    }
}

bool TransactionHeap::insert(const Transaction& transaction)
{
    int key = transaction.getDateKey();   // 0 means the date is invalid
    if (isFull() || key == 0)
        return false;

    items[count].transaction = transaction;
    items[count].key = key;
    items[count].sequence = nextSequence++;
    count++;
    siftUp(count - 1);
    return true;
}

bool TransactionHeap::removeMin(Transaction& transaction)
{
    if (isEmpty())
        return false;

    transaction = items[0].transaction;
    count--;
    if (count > 0)
    {
        items[0] = items[count];   // the last item takes the top place...
        siftDown(0);               // ...and sinks to its correct position
    }
    return true;
}

bool TransactionHeap::peekMin(Transaction& transaction) const
{
    if (isEmpty())
        return false;

    transaction = items[0].transaction;
    return true;
}

int TransactionHeap::peekMinKey() const
{
    return isEmpty() ? 0 : items[0].key;
}

int TransactionHeap::peekMinSequence() const
{
    return isEmpty() ? -1 : items[0].sequence;
}

// A heap is NOT sorted, so finding one particular bill means checking every item.
int TransactionHeap::indexOfSequence(int sequence) const
{
    for (int i = 0; i < count; i++)
    {
        if (items[i].sequence == sequence)
            return i;
    }
    return -1;
}

bool TransactionHeap::findBySequence(int sequence, Transaction& transaction) const
{
    int index = indexOfSequence(sequence);
    if (index == -1)
        return false;

    transaction = items[index].transaction;
    return true;
}

bool TransactionHeap::removeBySequence(int sequence, Transaction& transaction)
{
    int index = indexOfSequence(sequence);   // O(n)
    if (index == -1)
        return false;

    transaction = items[index].transaction;
    count--;
    if (index == count)
        return true;   // it was the last item: nothing to repair

    // Fill the gap with the last item. That item may now be EARLIER than its new
    // parent (move it up) or LATER than one of its children (move it down).
    items[index] = items[count];
    int parent = (index - 1) / 2;
    if (index > 0 && comesBefore(items[index], items[parent]))
        siftUp(index);     // O(log n)
    else
        siftDown(index);   // O(log n)
    return true;
}

bool TransactionHeap::isEmpty() const
{
    return count == 0;
}

bool TransactionHeap::isFull() const
{
    return count == CAPACITY;
}

int TransactionHeap::size() const
{
    return count;
}

void TransactionHeap::clear()
{
    count = 0;
    nextSequence = 0;
}
