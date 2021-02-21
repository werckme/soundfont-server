#ifndef MYLIST_H
#define MYLIST_H

#include <vector>

template<typename T>
class MyList : public std::vector<T>
{
public:
    typedef std::vector<T> Base;
    MyList();
    MyList(T*, int size);
    void append(const T&);
    void takeLast();

};

#endif