#include "error.h"

Exception::Exception() {};


Exception::Exception(const Exception&) {}

Exception& Exception::operator=(const Exception&) {
    return *this;
}

const char* Exception::what() const{
    return "Unknown Exception";
}

const char *FileException::what() const
{
    return "Unknown file exception ";
}

const char *FileNotFound::what() const
{
    return "File not found ";
}

const char *JSONWrongFormat::what() const
{
    return "Wrong json format ";
}

const char *GeoJSONWrongFormat::what() const
{
    return "Wrong geojson format ";
}

const char *NoSuchSubject::what() const
{
    return "No such subject ";
}

const char *ListError::what() const
{
    return "Unknow list error ";
}

const char *EmptyListError::what() const
{
    return "The list is empty ";
}

const char *EndOfIterator::what() const
{
    return "Iterator reached the lst element of a list ";
}

const char *WrongIndexError::what() const
{
    return "Wrong Index Error";
}
