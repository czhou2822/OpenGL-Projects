#include "single_line.h"




single_line::single_line()
{
	buffer_set_up();

}


single_line::~single_line()
{
}

void single_line::render()
{
	glBindVertexArray(VAO);
	glDrawArrays(GL_LINES, 0, 2);
	glBindVertexArray(0);
}

void single_line::buffer_set_up()
{
	GLuint buffer;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	vec4 points_temp[] = { o_1, o_2 };

	glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * 2,
		points_temp, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

}
