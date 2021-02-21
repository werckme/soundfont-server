#ifndef MYSTRING_H
#define MYSTRING_H

#include <string>
#include <vector>
#include "mydef.h"

class MyString : public std::string
{
public:
    typedef std::string Base; 
    using Base::Base;
    bool contains(const std::string &) const;
    std::vector<MyString> split(char) const;
    double toDouble(bool) const;
};

#endif