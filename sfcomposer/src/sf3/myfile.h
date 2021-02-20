#ifndef MYFILE_H
#define MYFILE_H

#include <string>
#include "mydef.h"

class MyFile {
public:
    enum Mode {ReadOnly};
    MyFile(const std::string &);
    bool open(Mode);
    qint64 pos() const;
    bool seek(qint64);
    int read(char*, int);
    int write(const char *, int);
};

#endif