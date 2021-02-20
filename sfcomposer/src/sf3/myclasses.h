#ifndef MYCLASSES_H
#define MYCLASSES_H

#include "myfile.h"
#include "mysysinfo.h"
#include "mylist.h"

typedef MyFile QFile;
typedef MyFile QIODevice;
typedef MySysinfo QSysInfo;

template <typename T>
using QList = MyList<T>;


#endif