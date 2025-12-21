#pragma once

#include "subject.h"

struct QueueNode{
    AbstractSubject* data;
    QueueNode* next;
    
    QueueNode(AbstractSubject* data = 0) : data(data), next(0) {}
};


class Queue {
private:
    QueueNode* front;
    QueueNode* rear;
    int size;
    
public:
    Queue();
    ~Queue();
    
    void add(AbstractSubject* data);
    AbstractSubject* remove();
    AbstractSubject* getFront() const;
    
    bool isEmpty() const;
    int getSize() const;
    void clear();
};