#pragma once

#include "Angel.h"
#include <Vector>

class single_line
{
public:
	single_line();
	~single_line();
	GLuint VAO;
	float len = 0.1;
	vec4 o_1 = vec4(0, 0, 0, 1);
	vec4 o_2 = vec4(0, 0, len, 1);


	void render();
	void buffer_set_up();
};

