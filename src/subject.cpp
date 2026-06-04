#include "subject.h"
#include "error.h"
#include <algorithm>

using json = nlohmann::json;

static AbstractSubject* find_in_list_by_name(const std::list<AbstractSubject*>& lst, const std::string& name)
{
    for (AbstractSubject* subject : lst) {
        const std::list<std::string>& names = subject->get_names();
        if (std::find(names.begin(), names.end(), name) != names.end()) {
            return subject;
        }
    }
    return nullptr;
}

AbstractSubject* Map::find_subject_by_name(
    std::list<AbstractSubject*>& lst,
    const std::string& name)
{
    return find_in_list_by_name(lst, name);
}

void AbstractSubject::add_polygon()
{
    border.emplace_back(new Polygon());
}

void AbstractSubject::add_coord(Coordinates c)
{
    if (border.empty()) {
        return;
    }

    border.back()->push_back(c);
}

void Map::get_from_JSON(const std::string& subject_borders_file,
                        const std::string& subject_neighbours_file)
{
    std::ifstream borders_in(subject_borders_file);
    if (!borders_in.is_open()) {
        throw FileNotFound();
    }
    
    json borders_json;
    borders_in >> borders_json;
    
    for (const auto& feature : borders_json["features"]) {
        AbstractSubject* subj = new SubjectRussia();
        std::string name = feature["properties"]["name"].get<std::string>();
        subj->add_name(name);
        
        const auto& geom = feature["geometry"];
        std::string type = geom["type"].get<std::string>();
        
        if (type == "Polygon") {
            subj->add_polygon();
            const auto& ring = geom["coordinates"][0];
            for (const auto& pt : ring) {
                Coordinates c{pt[0].get<double>(), pt[1].get<double>()};
                subj->add_coord(c);
            }
        }
        else if (type == "MultiPolygon") {
            for (const auto& poly : geom["coordinates"]) {
                subj->add_polygon();      
                const auto& ring = poly[0];  
                for (const auto& pt : ring) {
                    Coordinates c{pt[0].get<double>(), pt[1].get<double>()};
                    subj->add_coord(c); 
                }
            }
        }
        subject_list.push_back(subj);
    }
    std::ifstream neigh_in(subject_neighbours_file);
    if (!neigh_in.is_open()) {
        throw FileNotFound();
    }
    
    json neigh_json;
    neigh_in >> neigh_json;
    
    for (const auto& [subj_name, neigh_array] : neigh_json.items()) {
        AbstractSubject* subj = find_subject_by_name(subject_list, subj_name);
        if (!subj) continue;
        
        for (const auto& neigh_name_json : neigh_array) {
            std::string neigh_name = neigh_name_json.get<std::string>();
            AbstractSubject* neigh_subj = find_subject_by_name(subject_list, neigh_name);
            if (neigh_subj) {
                subj->add_neighbour(neigh_subj);
            }
        }
    }
}

bool Map::is_neighbours(const std::string& subject_name_1,
                        const std::string& subject_name_2)
{
    AbstractSubject* subj1 = find_subject_by_name(subject_list, subject_name_1);
    if (!subj1) 
        return false;
    return find_in_list_by_name(subj1->get_neighbours(), subject_name_2) != nullptr;
}