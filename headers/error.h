#pragma once

class Error{}; 

class EmptyListError: public Error{}; 

class EndOfIterator: public Error{};