#include "myfile.h"
#include <stdexcept>

MyFile::MyFile(const std::string &)
{
}

bool MyFile::open(Mode)
{
    throw std::runtime_error("not yet impl.");
}

qint64 MyFile::pos() const
{
    throw std::runtime_error("not yet impl.");
}

bool MyFile::seek(qint64)
{
    throw std::runtime_error("not yet impl.");
}

int MyFile::read(char *, int)
{
    throw std::runtime_error("not yet impl.");
}

int MyFile::write(const char *, int)
{
    throw std::runtime_error("not yet impl.");
}

std::string MyFile::fileName() const
{
    throw std::runtime_error("not yet impl.");
}

void MyFile::close()
{
    throw std::runtime_error("not yet impl.");
}