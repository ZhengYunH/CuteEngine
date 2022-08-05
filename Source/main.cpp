#pragma comment(linker, "/subsystem:console")
#include <iostream>
#include "Core/Engine.h"
#include "Core/EventHelper.h"


int main()
{
	zyh::GEngine->Run();

	try {
		// zyh::GEngine->Run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
