#ifndef MYFILE_H
#define MYFILE_H

#include <string>
#include "mydef.h"
#include <stdio.h>

class MyFile {
private:
    FILE* pFile = nullptr;
    std::string path;
public:
    enum Mode {ReadOnly, WriteOnly};
    MyFile(const std::string &);
    bool open(Mode);
    qint64 pos() const;
    bool seek(qint64);
    int read(char*, int);
    int write(const char *, int);
    std::string fileName() const;
    void close();
};

#endif