#include "game.h"
#include <cstdlib>

Game::Game(String file, int NumberOfSubjects){
  srand(time(0)); // ???
  sub_number = rand() % NumberOfSubjects;
  // should i parse a json and create a list here or should it be created earlier? 
}

int Game::PlayerMove(String destination){
  if(Position.neighbours.names.in_list(destination) && !Visited.names.in_list(destination)){
    Turn = 1;
    Position = Position.neighbours.find_in_list(destination);
    Visited.push(Position);
    return 0;
  }
  else return -1;
}

int Game::play(){

}