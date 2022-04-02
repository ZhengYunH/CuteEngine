#pragma comment(linker, "/subsystem:console")
#include <iostream>
#include "Core/Engine.h"


int main()
{
	try {
		zyh::GEngine->Run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
