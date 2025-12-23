#pragma once

class Exception{
public:
    Exception();
    Exception(const Exception&);
    Exception& operator=(const Exception&);

    virtual const char* what() const;
    virtual ~Exception() {};
}; 

class FileException : public Exception{
public:
    virtual const char* what() const override;
};

class FileNotFound : public FileException {
public:
    virtual const char* what() const override;
};

class JSONWrongFormat : public FileException {
public:
    virtual const char* what() const override;
};

class GeoJSONWrongFormat : public FileException {
public:
    virtual const char* what() const override;
};

class NoSuchSubject : public Exception{
public:
    virtual const char* what() const override;  
};

class ListError : public Exception {
public:
    virtual const char* what() const override; 
};

class EmptyListError: public ListError
{
public:
    virtual const char* what() const override;
}; 

class WrongIndexError: public ListError
{
public:
    virtual const char* what() const override;
}; 

class EndOfIterator: public ListError{
public:
    virtual const char* what() const override;
};