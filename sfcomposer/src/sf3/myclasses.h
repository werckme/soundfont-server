#ifndef MYCLASSES_H
#define MYCLASSES_H

#include "myfile.h"
#include "mysysinfo.h"
#include "mylist.h"
#include "mystring.h"

typedef MyFile QFile;
typedef MyFile QIODevice;
typedef MySysinfo QSysInfo;

template <typename T>
using QList = MyList<T>;
typedef MyList<char> QByteArray;

typedef MyString QString;

#endif