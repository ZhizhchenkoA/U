#pragma once

#include "subject.h"
#include <string>
#include <list>
#include <vector>

class Game {
private:
    int NumberOfSubjects;
    std::list<AbstractSubject*>& Subjects;  // Ссылка на список из Map
    AbstractSubject* StartPosition;
    AbstractSubject* Position;
    AbstractSubject* FinalPosition;
    int Turn;   // 0 - player, 1 - computer
    std::vector<AbstractSubject*> Visited;  // Вектор для быстрого доступа и кэш-локальности
    int Mistakes;
    bool GameFinished;
    int playerTotalTimeSec_;
    int computerTotalTimeSec_;
    

    std::vector<int> calculateDistances();
public:
    Game(int NumberOfSubjects, std::list<AbstractSubject*>& Subjects);
    ~Game();

    int makeNetworkMove(const std::string& destination, int expectedTurn);
    
    int makePlayerMove(const std::string& destination);
    int makeComputerMove();
    
    std::string getCurrentRegionName() const;
    std::string getStartRegionName() const;
    std::string getFinalRegionName() const;
    
    // Возвращаем вектор строк — удобнее для Qt и JSON
    std::vector<std::string> getNeighborRegionNames() const;
    std::vector<std::string> getVisitedRegionNames() const;
    
    int getMistakesCount() const;
    int getTurn() const;
    bool isGameFinished() const;
    int getWinner() const;
    bool isRegionReachable(const std::string& regionName) const;

    std::list<AbstractSubject*> getAllRegions() const;
    void reset();
    void addPlayerTime(int seconds);
    void addComputerTime(int seconds);
    int getPlayerTotalTime() const;
    int getComputerTotalTime() const;    

};