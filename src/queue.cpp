#include "queue.h"

Queue::Queue() : front(0), rear(0), size(0) {}

Queue::~Queue() {
    clear();
}

void Queue::add(AbstractSubject* data) {
    QueueNode* newNode = new QueueNode(data);
    if (rear == 0) {
        front = rear = newNode;
    } else {
        rear->next = newNode;
        rear = newNode;
    }
    
    size++;
}

AbstractSubject* Queue::remove() {
    if (front == 0) {
        return 0;
    }
    
    QueueNode* temp = front;
    AbstractSubject* data = temp->data;
    
    front = front->next;
    
    if (front == 0) {
        rear = 0;
    }
    
    delete temp;
    size--;
    return data;
}

AbstractSubject* Queue::getFront() const {
    if (front == 0) {
        return 0;
    }
    return front->data;
}

bool Queue::isEmpty() const {
    return front == 0;
}

int Queue::getSize() const {
    return size;
}

void Queue::clear() {
    while (!isEmpty()) {
        remove();
    }
}