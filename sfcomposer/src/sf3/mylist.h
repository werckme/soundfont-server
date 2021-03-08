#ifndef MYLIST_H
#define MYLIST_H

#include <vector>
#include <stdexcept>
#include <string.h>

template<typename T>
class MyList : public std::vector<T>
{
public:
    typedef std::vector<T> Base;
    MyList();
    MyList(T*, int size);
    template <class InputIterator>
    MyList(InputIterator begin, InputIterator end) : Base(begin, end) {}
    void append(const T&);
    T takeLast();
    virtual ~MyList() = default;
};

template<typename T>
MyList<T>::MyList() 
{

}

template<typename T>
MyList<T>::MyList(T* inBff, int size)
{
    Base::resize(size);
    ::memcpy(this->data(), inBff, size);
}

template<typename T>
void MyList<T>::append(const T& val)
{
    Base::push_back(val);
}

template<typename T>
T MyList<T>::takeLast()
{
    T result = Base::back();
    Base::pop_back();
    return result;
}

#endif