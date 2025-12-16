#pragma once

#include "subject.h"
#include "str.h"

class Game{
  String FileName;
  int NumberOfSubjects;
  AbstractSubject* Position;
  int Turn;   // 0 - player, 1 - computer
  List<AbstractSubject> Visited;
public:
  Game(String file, int NumberOfSubjects);
  int PlayerMove(String destination);
  int ComputerMove(); // some magic should happen here
  int play()
}