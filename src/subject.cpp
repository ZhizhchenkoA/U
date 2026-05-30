#include "subject.h"
#include "error.h"
#include <algorithm>  // для std::find

using json = nlohmann::json;

// Вспомогательная функция для поиска по имени в std::list
static AbstractSubject* find_in_list_by_name(
    const std::list<AbstractSubject*>& lst,
    const std::string& name)
{
    for (auto* subject : lst) {
        const auto& names = subject->get_names(); // std::list<std::string>
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

void AbstractSubject::add_neighbour(AbstractSubject* s)
{
    // std::find вместо in_list()
    if (std::find(neighbours.begin(), neighbours.end(), s) == neighbours.end()) {
        neighbours.push_back(s);  // push_back вместо push()
    }
}

void AbstractSubject::add_name(const std::string& s)
{
    if (std::find(names.begin(), names.end(), s) == names.end()) {
        names.push_back(s);
    }
}

void AbstractSubject::add_polygon()
{
    border.emplace_back(new Polygon());  // emplace_back для эффективности
}

void AbstractSubject::add_coord(Coordinates c)
{
    if (border.empty()) {
        // throw Error("add_coord(): no polygon"); // раскомментируйте при необходимости
        return;
    }
    // front() вместо iter().next()
    border.front()->push_back(c);  // push_back для std::list<Coordinates>
}

void Map::get_from_JSON(const std::string& subject_borders_file,
                        const std::string& subject_neighbours_file)
{
    // === Загрузка границ ===
    std::ifstream borders_in(subject_borders_file);
    json borders_json;
    borders_in >> borders_json;

    for (const auto& feature : borders_json["features"]) {
        auto* subj = new SubjectRussia();

        // nlohmann::json работает нативно с std::string
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

        subject_list.push_back(subj);  // push_back вместо push()
    }

    // === Загрузка соседей ===
    std::ifstream neigh_in(subject_neighbours_file);
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
    if (!subj1) return false;
    
    return find_in_list_by_name(subj1->get_neighbours(), subject_name_2) != nullptr;
}