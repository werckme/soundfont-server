#include "sf3/sfont.h"
#include <iostream>
#include <stdexcept>
#include <string>

void process(const std::string& sfPath)
{
	SfTools::SoundFont sf(sfPath);
	sf.read();
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