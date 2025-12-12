#pragma once 

#include "list.h"

struct Coordinates{
    double x;
    double y;
};

class String{
    // тут будет реализация класса для строк -- потом перенесу
};


class AbstractSubject {
protected:
    List<String> names;
    List<AbstractSubject*> neighbours;
    List<Coordinates> border;
public:
    AbstractSubject():neighbours(), border(){};
    void add_neighbour(AbstractSubject**);
    void add_name(String);
    void add_coord(Coordinates);
};

class SubjectRussia: public AbstractSubject{
    int number_of_subject;
};

class Map{
    AbstractSubject **subject_array;
public:
    void get_from_JSON(String);
    bool is_neighbours(String, String);
};