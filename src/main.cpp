#include <iostream>
#include <stdlib.h>

#include "App.h"

int main(int argc, char** argv)
{
	try {
		srand(21);
		App app;
		app.run();
	} catch (Ogre::Exception& e) {
		std::cerr << "Ogre exception: " << e.what() << std::endl;
		return 1;
	} catch (std::exception& e) {
		std::cerr << "std::exception: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}

