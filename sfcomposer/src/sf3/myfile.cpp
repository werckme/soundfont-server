#include "myfile.h"
#include <stdexcept>
#include <iostream>

MyFile::MyFile(const std::string & path) : path(path)
{
}

bool MyFile::open(Mode mode)
{
    if (pFile != nullptr) {
        throw std::runtime_error("file '" + path + "' already open");
    }
    std::string strmode;
    switch (mode)
    {
    case ReadOnly: strmode = "rb"; break;
    case WriteOnly: strmode = "wb"; break;
    }
    pFile = fopen(path.c_str(), strmode.c_str());
    return pFile != nullptr;
}

qint64 MyFile::pos() const
{
    if (pFile == nullptr) {
        throw std::runtime_error("file '" + path + "' is not open");
    }
    return ftell(pFile);
}

bool MyFile::seek(qint64 offset)
{
    if (pFile == nullptr) {
        throw std::runtime_error("file '" + path + "' is not open");
    }
    return fseek(pFile, offset, SEEK_SET) == 0;
}

int MyFile::read(char *outBff, int numBytes)
{
    if (pFile == nullptr) {
        throw std::runtime_error("file '" + path + "' is not open");
    }
    auto result = fread(outBff, 1, numBytes, pFile);
    return result;
}

int MyFile::write(const char *bff, int numBytes)
{
    if (pFile == nullptr) {
        throw std::runtime_error("file '" + path + "' is not open");
    }
    return fwrite(bff, 1, numBytes, pFile);

}

std::string MyFile::fileName() const
{
    return path;
}

void MyFile::close()
{
    if (pFile == nullptr) {
        throw std::runtime_error("file '" + path + "' already closed");
    }
    fclose(pFile);
    pFile = nullptr;
}