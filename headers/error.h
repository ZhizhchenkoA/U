#pragma once
#include <exception>



class Exception : public std::exception {
public:
    Exception();
    Exception(const Exception&);
    Exception& operator=(const Exception&);

    virtual const char* what() const noexcept override;
    virtual ~Exception() {};
}; 

class FileException : public Exception{
public:
    virtual const char* what() const noexcept override;
};

class FileNotFound : public FileException {
public:
    virtual const char* what() const noexcept override;
};

class JSONWrongFormat : public FileException {
public:
    virtual const char* what() const noexcept override;
};

class GeoJSONWrongFormat : public FileException {
public:
    virtual const char* what() const noexcept override;
};

class NoSuchSubject : public Exception{
public:
    virtual const char* what() const noexcept override;  
};

class ListError : public Exception {
public:
    virtual const char* what() const noexcept override; 
};

class EmptyListError: public ListError
{
public:
    virtual const char* what() const noexcept override;
}; 

class WrongIndexError: public ListError
{
public:
    virtual const char* what() const noexcept override;
}; 

class EndOfIterator: public ListError{class Exception : public std::exception {
public:
    Exception();
    Exception(const Exception&);
    Exception& operator=(const Exception&);

    virtual const char* what() const noexcept override;
    virtual ~Exception() {};
}; 
public:
    virtual const char* what() const noexcept override;
};