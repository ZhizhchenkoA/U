#include "error.h"

Exception::Exception() {};


Exception::Exception(const Exception&) {}

Exception& Exception::operator=(const Exception&) {
    return *this;
}

const char* Exception::what() const noexcept{
    return "Unknown Exception";
}

const char *FileException::what() const noexcept
{
    return "Unknown file exception ";
}

const char *FileNotFound::what() const noexcept
{
    return "File not found ";
}

const char *JSONWrongFormat::what() const noexcept
{
    return "Wrong json format ";
}

const char *GeoJSONWrongFormat::what() const noexcept
{
    return "Wrong geojson format ";
}

const char *NoSuchSubject::what() const noexcept
{
    return "No such subject ";
}

const char *ListError::what() const noexcept
{
    return "Unknow list error ";
}

const char *EmptyListError::what() const noexcept
{
    return "The list is empty ";
}
 
const char *EndOfIterator::what() const noexcept
{
    return "Iterator reached the lst element of a list ";
}

const char *WrongIndexError::what() const noexcept
{
    return "Wrong Index Error";
}
