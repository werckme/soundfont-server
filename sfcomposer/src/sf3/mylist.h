#ifndef MYLIST_H
#define MYLIST_H

#include <list>

template<typename T>
class MyList : public std::list<T>
{
public:
    void append(const T&);
    void takeLast();
};

#endif