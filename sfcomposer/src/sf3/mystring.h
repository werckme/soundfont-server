#ifndef MYSTRING_H
#define MYSTRING_H

#include <string>
#include <vector>
#include "mydef.h"

class MyString : public std::string
{
public:
    typedef std::string Base; 
    MyString();
    MyString(const std::string &);
    bool contains(const std::string &) const;
    std::vector<MyString> split(char) const;
    double toDouble(bool) const;
    virtual ~MyString() = default;
};

#endif