#pragma once
#include <QString>
#include <nlohmann/json.hpp>
#include <vector>
#include <string>

using json = nlohmann::json;

namespace NetworkProtocol {
    inline QString makeWelcomeMsg(int playerNumber) {
        json j;
        j["type"] = "welcome";
        j["playerNumber"] = playerNumber; // 1 or 2
        return QString::fromStdString(j.dump()) + "\n";
    }

    inline QString makeMoveMsg(const std::string& regionName) {
        json j;
        j["type"] = "move";
        j["region"] = regionName;
        return QString::fromStdString(j.dump()) + "\n";
    }

    inline QString makeStateMsg(int turn, const std::string& current, const std::string& final, 
                                int mistakes, const std::vector<std::string>& visited, 
                                bool finished, int winner, const std::string& errorMsg = "") {
        json j;
        j["type"] = "state";
        j["turn"] = turn;
        j["current"] = current;
        j["final"] = final;
        j["mistakes"] = mistakes;
        j["visited"] = visited;
        j["finished"] = finished;
        j["winner"] = winner;
        j["error"] = errorMsg;
        return QString::fromStdString(j.dump()) + "\n";
    }
}