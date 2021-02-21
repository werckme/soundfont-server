#include "sf3/sfont.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include "sf3/mysysinfo.h"

void process(const std::string& sfPath)
{
	auto x = MySysinfo::ByteOrder;
	SfTools::SoundFont sf(sfPath);
	sf.read();
	//sf.file = new QFile("copy.sf2");
	//sf.file->open(QFile::WriteOnly);
	QFile out("copy.sf2");
	out.open(QFile::WriteOnly);
	try {
		//sf.write();
		sf.compress(&out, 1, 1);
	}
	catch (...) {
		//delete sf.file;
		throw;
	}
	//delete sf.file;
}


int main(int argc, const char** argv)
{
	try {
		if (argc < 2) {
			throw std::runtime_error(
				"usage: sfcomposer <path to sf>"
			);
		}
		process(argv[1]);
	} catch (const std::exception& ex) {
		std::cout << ex.what() << std::endl;
		return -1;
	}
	return 0;
}