#include "Stack.h"

UndoStack::UndoStack()
{
    top = nullptr;
    count = 0;
}

UndoStack::~UndoStack()
{
    clear();
}

void UndoStack::push(const UndoAction& action)
{
    StackNode* newNode = new StackNode;
    newNode->action = action;
    newNode->next = top;    // new node sits on top of the old top
    top = newNode;
    count++;
}

bool UndoStack::pop(UndoAction& action)
{
    if (top == nullptr)
        return false;

    StackNode* temp = top;
    action = temp->action;
    top = top->next;
    delete temp;
    count--;
    return true;
}

bool UndoStack::peek(UndoAction& action) const
{
    if (top == nullptr)
        return false;

    action = top->action;
    return true;
}

bool UndoStack::isEmpty() const
{
    return top == nullptr;
}

int UndoStack::size() const
{
    return count;
}

void UndoStack::clear()
{
    UndoAction discarded;
    while (pop(discarded))
    {
    }
}