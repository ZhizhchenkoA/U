#pragma once


class Error{};

class EmptyListError: public Error{};

class EndOfIterator: public Error{};

template <class T>
class List{
    template <class X>
    struct list_node{
        X value;
        list_node *next;
    };

    list_node<T> *head;
    int size_;
public:

    List(): head(nullptr), size_(0){};
    
    void push(T);
    T pop();
    
    void clear(){
        while(size_)
            pop();
    }
    int size() const{
        return size_;
    }
    ~List(){
        clear();
    }
    template <class X>
    class Iterator{
        list_node<X>* cur_node;
        
        public:
            Iterator(const List<X> *lst){
                cur_node = lst->head;
            }
            X next(){
                if (!cur_node)
                    throw EndOfIterator();
                X val = cur_node->value;
                cur_node = cur_node->next;
                return val;
            }
            bool isEnd(){
                return !cur_node;
            }
    };
    Iterator<T> iter() const{
        return Iterator<T>(this);
    }
};


template <class T>
inline void List<T>::push(T value)
{
    list_node<T>* new_elem = new list_node<T>;
    new_elem->value = value;
    new_elem->next = this->head;
    this->head = new_elem;
    size_++;
}


template <class T>
inline T List<T>::pop()
{
    if (!size_)
        throw EmptyListError();
    T return_value = this->head->value;
    list_node<T>* tmp = this->head;
    this->head = this->head->next;
    this->size_--;
    delete tmp;    
    return return_value;
}

