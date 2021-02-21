#ifndef MYLIST_H
#define MYLIST_H

#include <vector>
#include <stdexcept>

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

template<typename T>
MyList<T>::MyList() 
{

}

template<typename T>
MyList<T>::MyList(T*, int size)
{

}

template<typename T>
void MyList<T>::append(const T&)
{
    throw std::runtime_error("not yet impl.");
}

template<typename T>
void MyList<T>::takeLast()
{
    throw std::runtime_error("not yet impl.");
}

#endif