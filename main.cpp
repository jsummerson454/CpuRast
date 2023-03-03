#include <iostream>
#include "examples.h"

int main(int argc, char** argv) {
	if (argc < 2) {
		std::cout << "Please input which example you wish to run" << std::endl;
		std::cout << "Available examples: basic_example, cherkerboard_example, model_example" << std::endl;
		return -1;
	}
	if (strcmp("basic_example", argv[1]) == 0) {
		std::cout << "Executing basic_example" << std::endl;
		if (argc < 3) {
			return BasicExample::run();
		}
		else {
			return BasicExample::run(argv[2]);
		}
	}
	if (strcmp("texture_tests", argv[1]) == 0) {
		std::cout << "Executing texture_tests" << std::endl;
		return TextureTests::runTests();
	}
	if (strcmp("checkerboard_example", argv[1]) == 0) {
		std::cout << "Executing checkerboard_example" << std::endl;
		if (argc < 3) {
			return CheckerboardExample::run();
		}
		else {
			return CheckerboardExample::run(argv[2]);
		}
	}
	if (strcmp("model_example", argv[1]) == 0) {
		std::cout << "Executing model_example" << std::endl;
		return ModelExample::run();
	}
	else {
		std::cout << "Example name " << argv[1] << " not recognised" << std::endl;
		return -2;
	}
}