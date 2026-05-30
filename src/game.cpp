#include "game.h"
#include <queue>
#include <algorithm>    // std::find
#include <cstdlib>
#include <ctime>
#include <iostream>

// Вспомогательная: поиск индекса элемента в std::list
static int index_of_in_list(const std::list<AbstractSubject*>& lst, AbstractSubject* ptr) {
    int idx = 0;
    for (auto* item : lst) {
        if (item == ptr) return idx;
        ++idx;
    }
    return -1;
}

// Вспомогательная: проверка, посещён ли регион
static bool is_visited_in_game(const std::vector<AbstractSubject*>& visited, 
                               AbstractSubject* subject) {
    return std::find(visited.begin(), visited.end(), subject) != visited.end();
}

// BFS для расчёта дистанций
std::vector<int> Game::calculateDistances() {
    std::vector<int> distances(NumberOfSubjects, -1);
    std::vector<bool> visitedInBFS(NumberOfSubjects, false);
    
    int finalIndex = index_of_in_list(Subjects, FinalPosition);
    if (finalIndex == -1) return distances;
    
    // STL queue: front() + pop() вместо remove()
    std::queue<AbstractSubject*> q;
    distances[finalIndex] = 0;
    visitedInBFS[finalIndex] = true;
    q.push(FinalPosition);
    
    while (!q.empty()) {
        AbstractSubject* current = q.front();  // ⚠️ сначала получаем
        q.pop();                                // ⚠️ потом удаляем
        
        int currentIndex = index_of_in_list(Subjects, current);
        if (currentIndex == -1) continue;
        
        for (AbstractSubject* neighbor : current->get_neighbours()) {
            int neighborIndex = index_of_in_list(Subjects, neighbor);
            if (neighborIndex == -1) continue;
            
            bool visitedInGame = is_visited_in_game(Visited, neighbor) || neighbor->is_visited();
            
            if (!visitedInGame && !visitedInBFS[neighborIndex]) {
                distances[neighborIndex] = distances[currentIndex] + 1;
                visitedInBFS[neighborIndex] = true;
                q.push(neighbor);
            }
        }
    }
    
    return distances;  // std::vector управляет памятью автоматически
}

Game::Game(int numberOfSubjects, std::list<AbstractSubject*>& subjects)
    : NumberOfSubjects(numberOfSubjects), 
      Subjects(subjects), 
      StartPosition(nullptr), 
      Position(nullptr), 
      FinalPosition(nullptr), 
      Turn(0), 
      Mistakes(0), 
      GameFinished(false)
{
    srand(static_cast<unsigned>(time(nullptr)));
    
    if (NumberOfSubjects <= 0 || Subjects.empty()) return;
    
    // Для доступа по индексу копируем list в vector (единоразово)
    std::vector<AbstractSubject*> subjects_vec(Subjects.begin(), Subjects.end());
    
    int startIndex = rand() % NumberOfSubjects;
    StartPosition = subjects_vec[startIndex];
    Position = StartPosition;
    Visited.push_back(Position);  // push_back для std::vector
    
    int finalIndex;
    do {
        finalIndex = rand() % NumberOfSubjects;
    } while (finalIndex == startIndex);
    FinalPosition = subjects_vec[finalIndex];
    
    StartPosition->visit();
    Turn = 0;
}

Game::~Game() = default;

int Game::makePlayerMove(const std::string& destination) {
    if (GameFinished || Turn != 0) return -1;
    
    // Поиск региона по имени
    AbstractSubject* destSubject = nullptr;
    for (AbstractSubject* subject : Subjects) {
        const auto& names = subject->get_names();
        if (std::find(names.begin(), names.end(), destination) != names.end()) {
            destSubject = subject;
            break;
        }
    }
    
    if (!destSubject) {
        if (++Mistakes >= 3) {
            GameFinished = true;
            return -2;
        }
        return -1;
    }
    
    // Проверка: сосед и не посещён
    const auto& neighbors = Position->get_neighbours();
    bool isNeighbor = (std::find(neighbors.begin(), neighbors.end(), destSubject) != neighbors.end());
    bool wasVisited = is_visited_in_game(Visited, destSubject) || destSubject->is_visited();
    
    if (!isNeighbor || wasVisited) {
        if (++Mistakes >= 3) {
            GameFinished = true;
            return -2;
        }
        return -1;
    }
    
    // Выполнение хода
    Position = destSubject;
    Visited.push_back(Position);
    Position->visit();
    Turn = 1;
    
    if (Position == FinalPosition) {
        GameFinished = true;
        return 1;
    }
    return 0;
}

int Game::makeComputerMove() {
    if (GameFinished || Turn != 1) return -1;
    
    auto distances = calculateDistances();  // std::vector, автоматическая очистка
    
    int currentIndex = index_of_in_list(Subjects, Position);
    if (currentIndex == -1) {
        GameFinished = true;
        return -2;
    }
    
    const auto& neighbors = Position->get_neighbours();
    
    // Проверка немедленной победы
    for (AbstractSubject* neighbor : neighbors) {
        if (neighbor == FinalPosition) {
            bool wasVisited = is_visited_in_game(Visited, FinalPosition) || FinalPosition->is_visited();
            if (!wasVisited) {
                Position = FinalPosition;
                Visited.push_back(Position);
                Position->visit();
                GameFinished = true;
                return 1;
            }
        }
    }
    
    // Стратегия: ход с нечётной дистанцией
    AbstractSubject* bestMove = nullptr;
    int bestDistance = -1;
    bool foundOdd = false;
    
    for (AbstractSubject* neighbor : neighbors) {
        bool wasVisited = is_visited_in_game(Visited, neighbor) || neighbor->is_visited();
        if (wasVisited) continue;
        
        int neighborIndex = index_of_in_list(Subjects, neighbor);
        if (neighborIndex == -1) continue;
        
        int distance = distances[neighborIndex];
        
        if (distance != -1 && distance % 2 == 1) {
            if (!foundOdd || distance < bestDistance) {
                foundOdd = true;
                bestMove = neighbor;
                bestDistance = distance;
            }
        } else if (!foundOdd) {
            if (!bestMove || (distance != -1 && (bestDistance == -1 || distance < bestDistance))) {
                bestMove = neighbor;
                bestDistance = distance;
            }
        }
    }
    
    if (!bestMove) {
        GameFinished = true;
        return -2;
    }
    
    Position = bestMove;
    Visited.push_back(Position);
    Position->visit();
    Turn = 0;
    
    if (Position == FinalPosition) {
        GameFinished = true;
        return 1;
    }
    return 0;
}

// === Геттеры для UI ===

std::string Game::getCurrentRegionName() const {
    if (!Position || Position->get_names().empty()) return "";
    return Position->get_names().front();  // front() вместо Get(0)
}

std::string Game::getStartRegionName() const {
    if (!StartPosition || StartPosition->get_names().empty()) return "";
    return StartPosition->get_names().front();
}

std::string Game::getFinalRegionName() const {
    if (!FinalPosition || FinalPosition->get_names().empty()) return "";
    return FinalPosition->get_names().front();
}

std::vector<std::string> Game::getNeighborRegionNames() const {
    std::vector<std::string> result;
    if (!Position) return result;
    
    for (AbstractSubject* neighbor : Position->get_neighbours()) {
        const auto& names = neighbor->get_names();
        if (!names.empty()) {
            result.push_back(names.front());
        }
    }
    return result;
}

std::vector<std::string> Game::getVisitedRegionNames() const {
    std::vector<std::string> result;
    for (AbstractSubject* visited : Visited) {  // Visited — std::vector
        const auto& names = visited->get_names();
        if (!names.empty()) {
            result.push_back(names.front());
        }
    }
    return result;
}

int Game::getMistakesCount() const { return Mistakes; }
int Game::getTurn() const { return Turn; }
bool Game::isGameFinished() const { return GameFinished; }

int Game::getWinner() const {
    if (!GameFinished) return -1;
    if (Position == FinalPosition) return (Turn == 1) ? 1 : 0;
    if (Mistakes >= 3) return 1;
    return (Turn == 0) ? 1 : 0;
}

void Game::reset() {
    std::vector<AbstractSubject*> subjects_vec(Subjects.begin(), Subjects.end());
    
    int startIndex = rand() % NumberOfSubjects;
    StartPosition = subjects_vec[startIndex];
    Position = StartPosition;
    
    int finalIndex;
    do {
        finalIndex = rand() % NumberOfSubjects;
    } while (finalIndex == startIndex);
    FinalPosition = subjects_vec[finalIndex];
    
    Visited.clear();
    Visited.push_back(Position);
    
    // Сброс флагов посещения
    for (AbstractSubject* subject : Subjects) {
        subject->unvisit();
    }
    Position->visit();
    
    Turn = 0;
    Mistakes = 0;
    GameFinished = false;
}

bool Game::isRegionReachable(const std::string& regionName) const {
    if (!Position || GameFinished || Turn != 0) return false;
    
    for (AbstractSubject* neighbor : Position->get_neighbours()) {
        const auto& names = neighbor->get_names();
        if (std::find(names.begin(), names.end(), regionName) != names.end()) {
            bool wasVisited = is_visited_in_game(Visited, neighbor) || neighbor->is_visited();
            return !wasVisited;
        }
    }
    return false;
}