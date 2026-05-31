#pragma once 

#include <list>
#include <string>
#include <vector>
#include <algorithm>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <QDebug>

struct Coordinates {
    double x;
    double y;
};


typedef std::list<Coordinates> Polygon;

class AbstractSubject {
protected:
    std::list<std::string> names;
    std::list<AbstractSubject*> neighbours;
    std::list<Polygon*> border;
    bool is_visited_;
public:
    AbstractSubject() : is_visited_(false) {};
    
    void add_neighbour(AbstractSubject* n) { neighbours.push_back(n); }
    void add_name(const std::string& name) { names.push_back(name); }
    void add_coord(Coordinates c);
    void add_polygon();
    
    void visit() { is_visited_ = true; }
    void unvisit() { is_visited_ = false; }
    
    std::list<std::string>& get_names() { return names; }
    std::list<AbstractSubject*>& get_neighbours() { return neighbours; }
    std::list<Polygon*>& get_border() { return border; }
    bool is_visited() const { return is_visited_; }
};

class SubjectRussia : public AbstractSubject {
    int number_of_subject = 0;
};

class Map {
    std::list<AbstractSubject*> subject_list;
    
    static AbstractSubject* find_subject_by_name(std::list<AbstractSubject*>& lst, const std::string& name);
public:
    void get_from_JSON(const std::string& json_path, const std::string& geojson_path);
    bool is_neighbours(const std::string& name1, const std::string& name2);
    std::list<AbstractSubject*>& get_subjects() { return subject_list; }
};