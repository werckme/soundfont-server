#include "mystring.h"
#include <stdexcept>

MyString::MyString() 
{

}

MyString::MyString(const std::string &str) : Base(str)
{

}

bool MyString::contains(const std::string &) const
{
    throw std::runtime_error("not yet impl.");
}

std::vector<MyString> MyString::split(char) const
{
    throw std::runtime_error("not yet impl.");
}

double MyString::toDouble(bool) const
{
    throw std::runtime_error("not yet impl.");
}