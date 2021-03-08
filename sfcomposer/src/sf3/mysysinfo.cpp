#include "mysysinfo.h"

namespace {
	union {
		unsigned short shortVar;
		unsigned char  charVar[2];
	} test_endianness;
}

// https://de.wikipedia.org/wiki/Byte-Reihenfolge
bool isLittleEndian() {
    test_endianness.shortVar = 0x8000; 
    if (test_endianness.charVar[0] != 0) {
        return false;
    }
    else {
        return true;
    }
}

const int MySysinfo::ByteOrder = isLittleEndian() ? 0 : 1;
const int MySysinfo::BigEndian = 1;