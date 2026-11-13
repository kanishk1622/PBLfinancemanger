#ifndef STACK_H
#define STACK_H
#include "../Transactions/transactions.h"
struct UndoAction
{
    enum ActionType { ADD_TRANSACTION, DELETE_TRANSACTION };

    ActionType type;
    Transaction transaction;
};

class UndoStack
{
private:
    struct StackNode
    {
        UndoAction action;
        StackNode* next;
    };

    StackNode* top;
    int count;

public:
    UndoStack();
    ~UndoStack();

    UndoStack(const UndoStack&) = delete;
    UndoStack& operator=(const UndoStack&) = delete;

    void push(const UndoAction& action);
    bool pop(UndoAction& action);  
    bool peek(UndoAction& action) const;   

    bool isEmpty() const;
    int size() const;
    void clear();
};

#endif