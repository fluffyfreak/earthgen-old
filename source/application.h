// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef __application_h__
#define __application_h__

#include <cstdio>
#include "utils.h"

struct GLFWwindow;

class Application {
public:
	Application(int argv, char** args);
	~Application();
	void Init();
	int Run();

private:
	bool InitWindow();
	void Shutdown();

	GLFWwindow* _primaryWindow;
};


#endif // __application_h__

