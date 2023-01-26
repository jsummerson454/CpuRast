#include <iostream>
#include "examples.h"

int main(int argc, char** argv) {
	if (argc < 2) {
		std::cout << "Please input which example you wish to run" << std::endl;
		std::cout << "Available examples: basic_example" << std::endl;
		return -1;
	}
	if (strcmp("basic_example", argv[1]) == 0) {
		std::cout << "Exectuing basic_example" << std::endl;
		return BasicExample::run(argv[2]);
	}
	else {
		std::cout << "Example name " << argv[1] << " not recognised" << std::endl;
		return -2;
	}
}