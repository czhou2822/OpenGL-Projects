// Draws colored cube  

#include "Angel.h"
#include <string>
#include <Vector>
#include <fstream>
#include <iostream>     
#include "turtle_string.h"
#include "single_line.h"
#include <stack>

//----------------------------------------------------------------------------
int width = 0;
int height = 0;

// remember to prototype
void generateGeometry( void );
void display( void );
void keyboard( unsigned char key, int x, int y );
void quad( int a, int b, int c, int d );
void colorcube(void);
void drawCube(void);

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;
typedef vec3 point3;
// handle to program
GLuint program;
vec4 model_color;


point4 point4_dummy = point4(0,0,1,1);
single_line *line_dummy;
char state = 'a';
using namespace std;

const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

point4 points[NumVertices];
color4 colors[NumVertices];
point3 globle_up = point3(0, 0, 1);
point3 globle_at = point3(0, 0, 0);
point3 globle_eye = point3(2, 0, 0);
mat4   globle_modelMat = Angel::identity();

struct one_drawing {
	int number_of_vertices;
	int number_of_face;
	float x_min;
	float x_max;
	float y_min;
	float y_max;
	float z_min;
	float z_max;
	float scale_factor;
	vector<point3> points_array;
	vector<point3> face_array;
	GLuint vao;
	mat4 model_matrix;
	vector<point3> points_giant_array;
	float width;
	float height;
	float depth;
};

one_drawing beethoven;
one_drawing car;

turtle_string dummy_turtle{
	1,       //len
	3,	     //iter
	45,      //rot
	"F",     //start
	"F-F",   //function
	"F-F"    //final_string
};


turtle_string turtle_array[5];
int turtle_index = 0;

// Vertices of a unit cube centered at origin, sides aligned with axes
point4 vertices[8] = {
    point4( -0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5, -0.5, -0.5, 1.0 ),
    point4( -0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5, -0.5, -0.5, 1.0 )


};
// RGBA olors
color4 vertex_colors[8] = {
    color4( 0.0, 0.0, 0.0, 1.0 ),  // black
    color4( 1.0, 0.0, 0.0, 1.0 ),  // red
    color4( 1.0, 1.0, 0.0, 1.0 ),  // yellow
    color4( 0.0, 1.0, 0.0, 1.0 ),  // green
    color4( 0.0, 0.0, 1.0, 1.0 ),  // blue
    color4( 1.0, 0.0, 1.0, 1.0 ),  // magenta
    color4( 1.0, 1.0, 1.0, 1.0 ),  // white
    color4( 0.0, 1.0, 1.0, 1.0 )   // cyan
};
// quad generates two triangles for each face and assigns colors
//    to the vertices
int Index = 0;
void quad( int a, int b, int c, int d )
{
    colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[b]; points[Index] = vertices[b]; Index++;
    colors[Index] = vertex_colors[c]; points[Index] = vertices[c]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[c]; points[Index] = vertices[c]; Index++;
    colors[Index] = vertex_colors[d]; points[Index] = vertices[d]; Index++;
}

// generate 12 triangles: 36 vertices and 36 colors
void colorcube()
{
    quad( 1, 0, 3, 2 );
    quad( 2, 3, 7, 6 );
    quad( 3, 0, 4, 7 );
    quad( 6, 5, 1, 2 );
    quad( 4, 5, 6, 7 );
    quad( 5, 4, 0, 1 );
}

void generateGeometry( void )
{	
    //colorcube();

    // Create a vertex array object
    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors),
		  NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );


	// Load shaders and use the resulting shader program
    program = InitShader( "vshader1.glsl", "fshader1.glsl" );
    glUseProgram( program );
     // set up vertex arrays
    GLuint vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(0) );

    GLuint vColor = glGetAttribLocation( program, "vColor" ); 
    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(sizeof(points)) );

	// sets the default color to clear screen
    glClearColor( 1.0, 1.0, 1.0, 1.0 ); // white background
}

void drawCube(void)
{
	// change to GL_FILL
	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	// draw functions should enable then disable the features 
	// that are specifit the themselves
	// the depth is disabled after the draw 
	// in case you need to draw overlays
	glEnable( GL_DEPTH_TEST );
    glDrawArrays( GL_LINES, 0, NumVertices );
	glDisable( GL_DEPTH_TEST ); 
}

void draw_one_line() {
	line_dummy->render();
}

void update_model_matrix(mat4 input) {
	GLuint modelMatrix = glGetUniformLocation(program, "model_matrix");
	glUniformMatrix4fv(modelMatrix, 1, GL_TRUE, input);
}

void draw_string(mat4 starting_point, string input, float rot, float len) {

	//float len = 0.1;
	

	mat4 modelMatrix = Angel::identity();
	stack<mat4> the_stack;
	modelMatrix = modelMatrix * starting_point;

	for (int i = 0; i < input.length(); i++) {
		switch (input.at(i)) {
		case 'F':
			update_model_matrix(modelMatrix);
			draw_one_line();
			modelMatrix = modelMatrix * Angel::Translate(0, 0, len);
			break;
		case '+':
			modelMatrix = modelMatrix * Angel::RotateX(rot);
			update_model_matrix(modelMatrix);
			break;
		case '-':
			modelMatrix = modelMatrix * Angel::RotateX(-rot);
			update_model_matrix(modelMatrix);
			break;

		case '&':
			modelMatrix = modelMatrix * Angel::RotateY(rot);
			update_model_matrix(modelMatrix);
			break;
		case '^':
			modelMatrix = modelMatrix * Angel::RotateY(-rot);
			update_model_matrix(modelMatrix);
			break;

		case 134:
			modelMatrix = modelMatrix * Angel::RotateZ(rot);
			update_model_matrix(modelMatrix);
			break;
		case '/':
			modelMatrix = modelMatrix * Angel::RotateZ(-rot);
			update_model_matrix(modelMatrix);
			break;
		case '[':  //push
			the_stack.push(mat4(modelMatrix));
			break;
		case ']':  //pop
			modelMatrix = the_stack.top();
			the_stack.pop();
			break;

		}
	}
	std::cout << endl;
}

void draw_plain() {
	point3 t1 = point3(5,-5,-3);
	point3 t2 = point3(-5,-5,-3);
	point3 t3 = point3(5,5,-3);
	point3 t4 = point3(-5,5,-3);

	point3 plain_array[] = { t2,t3,t4,t2,t3,t1 };

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(point3) * 6 , plain_array, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisable(GL_DEPTH_TEST);

}

void draw_ply(one_drawing input)
{
	glBindVertexArray(input.vao);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLES, 0, input.number_of_face*3);
	glDisable(GL_DEPTH_TEST);
}

one_drawing read_txt(char *fn) {
	vector<point3> vertics_index;
	vector<point3> points;
	vector<point3> raw_giant_array;

	std::ifstream input_file;
	input_file.open(fn);

	std::string context;
	input_file >> context;
	if (context != "ply") {
		//exit
	}

	while (context != "vertex") {
		input_file >> context;
	}

	int num_vertices;
	input_file >> num_vertices;

	while (context != "face") {
		input_file >> context;
	}

	int num_faces;
	input_file >> num_faces;

	while (context != "end_header") {
		input_file >> context;
	}

	float x_min = FLT_MAX;
	float y_min = FLT_MAX;
	float z_min = FLT_MAX;
	float x_max = FLT_MIN;
	float y_max = FLT_MIN;
	float z_max = FLT_MIN;
	float x = 0;
	float y = 0;
	float z = 0;

	for (int i = 0; i < num_vertices; i++) {
		input_file >> x;
		if (x > x_max) x_max = x;
		else if (x < x_min) x_min = x;

		input_file >> y;
		if (y > y_max) y_max = y;
		else if (y < y_min) y_min = y;

		input_file >> z;
		if (z > z_max) z_max = z;
		else if (z < z_min) z_min = z;

		point3 temp = point3(x, y, z);
		points.push_back(point3(x, y, z));
		//cout << temp << '\n';
	}

	float scale_factor = x_max;
	if (y_max > x_max) {
		if (z_max > x_max)
			scale_factor = z_max;
	}
	else if (z_max > x_max) {
		scale_factor = z_max;
	}

	float width_output = 0.5*(x_max + abs(x_min));
	float height_output = 0.5*(y_max + abs(y_min));
	float depth_output = 0.5*(z_max + abs(z_min));

	scale_factor = abs(scale_factor);
	point3 *vao_giant_array = (point3 *)malloc(sizeof(point3) * 3 * num_faces);

	int giant_array_index = 0;
	int middle_giant_array_index = 0;
	int vertix1 = 0;
	int vertix2 = 0;
	int vertix3 = 0;
	input_file >> context;
	for (int j = 0; j < num_faces; j++) {
		input_file >> vertix1;
		vao_giant_array[giant_array_index] = points[vertix1];
		raw_giant_array.push_back(points[vertix1]);
		giant_array_index++;

		input_file >> vertix2;
		vao_giant_array[giant_array_index] = points[vertix2];
		raw_giant_array.push_back(points[vertix2]);
		giant_array_index++;

		input_file >> vertix3;
		vao_giant_array[giant_array_index] = points[vertix3];
		raw_giant_array.push_back(points[vertix3]);
		giant_array_index++;

		point3 temp = point3(vertix1, vertix2, vertix3);
		vertics_index.push_back(temp);
		input_file >> context;

	}

	// Create a vertex array object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(point3) * 3 * num_faces, vao_giant_array, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);


	float extends_output[] = { x_min, x_max, y_min, y_max, z_min, z_max, scale_factor };

	cout << extends_output << endl;


	one_drawing output{
		   num_vertices,
		   num_faces,
		   //extends_output,
		   x_min, x_max, y_min, y_max, z_min, z_max, scale_factor,
		   points,
		   vertics_index,
		   vao,
		   Angel::Scale(1 / scale_factor, 1 / scale_factor, 1 / scale_factor)* Angel::Translate(-(x_min + x_max) / 2,-(y_min + y_max) / 2, -(z_min + z_max) / 2),
		   raw_giant_array,
		   width_output,
		   height_output,
		   depth_output
	};

	input_file.close();
	return output;

}

void my_init() {
	dummy_turtle.read_file("ply_files/lsys1.txt");
	turtle_array[0].read_file("ply_files/lsys1.txt");
	turtle_array[1].read_file("ply_files/lsys2.txt");
	turtle_array[2].read_file("ply_files/lsys3.txt");
	turtle_array[3].read_file("ply_files/lsys4.txt");
	turtle_array[4].read_file("ply_files/lsys5.txt");

	beethoven = read_txt("ply_files/beethoven.ply");
	car = read_txt("ply_files/big_porsche.ply");
	line_dummy = new single_line();
	std::cout << dummy_turtle.final_string << endl;
}

//----------------------------------------------------------------------------
// this is where the drawing should happen
void display( void )
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );     // clear the window
	GLuint modelMatrix = glGetUniformLocation(program, "model_matrix");
	GLuint modelColor = glGetUniformLocation(program, "color_input");
	

	switch (state) {
	case 'a':
		draw_string(Angel::Translate(0, 0, -2), turtle_array[turtle_index].final_string, turtle_array[turtle_index].rot, 0.02);
		globle_modelMat = Angel::identity();
		glUniformMatrix4fv(modelMatrix, 1, GL_TRUE, globle_modelMat);
		break;
	case 'b':
	case 'c':
	case 'd':
	case 'e':
		draw_string(Angel::Translate(0, 0, -2), turtle_array[turtle_index].final_string, turtle_array[turtle_index].rot, 0.1);
		globle_modelMat = Angel::identity();
		
		glUniformMatrix4fv(modelMatrix, 1, GL_TRUE, globle_modelMat);
		
		
		break;
	case 'g':
		draw_ply(beethoven);
		globle_modelMat = beethoven.model_matrix;
		glUniformMatrix4fv(modelMatrix, 1, GL_TRUE, globle_modelMat);

		break;
	
	case 't':
		globle_modelMat = Angel::Translate(-1, 0, 0)*Angel::identity();
		glUniformMatrix4fv(modelMatrix, 1, GL_TRUE, globle_modelMat);
		draw_plain();

		globle_up = point3(0, 0, 1);
		globle_at = point3(0, 0, 0);
		globle_eye = point3(10, 0, 0);
		break;

	case 'f':
		globle_modelMat = Angel::identity();
		glUniformMatrix4fv(modelMatrix, 1, GL_TRUE, globle_modelMat);
		model_color = vec4((float(rand() % 100 + 1) / 100), (float(rand() % 100 + 1) / 100), (float(rand() % 100 + 1) / 100), 1);
		glUniform4f(modelColor, model_color[0], model_color[1], model_color[2], model_color[3]);
		draw_string(Angel::Translate(1, 0, -2.5), turtle_array[0].final_string, turtle_array[0].rot,0.02);

		model_color = vec4((float(rand() % 100 + 1) / 100), (float(rand() % 100 + 1) / 100), (float(rand() % 100 + 1) / 100), 1);
		glUniform4f(modelColor, model_color[0], model_color[1], model_color[2], model_color[3]);
		draw_string(Angel::Translate(0,-7, -6), turtle_array[1].final_string, turtle_array[1].rot,0.1);

		model_color = vec4((float(rand() % 100 + 1) / 100), (float(rand() % 100 + 1) / 100), (float(rand() % 100 + 1) / 100), 1);
		glUniform4f(modelColor, model_color[0], model_color[1], model_color[2], model_color[3]);
		draw_string(Angel::Translate(0, -5, 4), turtle_array[2].final_string, turtle_array[2].rot,0.1);

		model_color = vec4((float(rand() % 100 + 1) / 100), (float(rand() % 100 + 1) / 100), (float(rand() % 100 + 1) / 100), 1);
		glUniform4f(modelColor, model_color[0], model_color[1], model_color[2], model_color[3]);
		draw_string(Angel::Translate(0, -7, 6), turtle_array[3].final_string, turtle_array[3].rot,0.1);
		
		model_color = vec4((float(rand() % 100 + 1) / 100), (float(rand() % 100 + 1) / 100), (float(rand() % 100 + 1) / 100), 1);
		glUniform4f(modelColor, model_color[0], model_color[1], model_color[2], model_color[3]);
		draw_string(Angel::Translate(4, 5, 5), turtle_array[4].final_string, turtle_array[4].rot, 0.1);

		model_color = vec4(0,1,0,1);
		glUniform4f(modelColor, model_color[0], model_color[1], model_color[2], model_color[3]);
		globle_modelMat = Angel::Translate(0, 0, -5)*Angel::Scale(1.3, 1.3, 0)*Angel::Translate(-1, 0, 0)*Angel::identity();
		glUniformMatrix4fv(modelMatrix, 1, GL_TRUE, globle_modelMat);
		draw_plain();
		
		model_color = vec4((float(rand() % 100 + 1) / 100), (float(rand() % 100 + 1) / 100), (float(rand() % 100 + 1) / 100), 1);
		glUniform4f(modelColor, model_color[0], model_color[1], model_color[2], model_color[3]);
		globle_modelMat = Angel::Translate(3,-4,-2)*Angel::RotateX(90)*Angel::RotateY(90)*Angel::Translate(7,2,2)*beethoven.model_matrix;
		glUniformMatrix4fv(modelMatrix, 1, GL_TRUE, globle_modelMat);
		draw_ply(beethoven);

		model_color = vec4((float(rand() % 100 + 1) / 100), (float(rand() % 100 + 1) / 100), (float(rand() % 100 + 1) / 100), 1);
		glUniform4f(modelColor, model_color[0], model_color[1], model_color[2], model_color[3]);
		globle_modelMat = Angel::Translate(7.5,-2,-2)*Angel::RotateZ(-90)*Angel::RotateX(90)*car.model_matrix;
		glUniformMatrix4fv(modelMatrix, 1, GL_TRUE, globle_modelMat);
		draw_ply(car);
		

		globle_up = point3(0, 0, 1);
		globle_at = point3(0, 0, 0);
		globle_eye = point3(10, 0, 0);
		
		break;
	}

	Angel::mat4 view = Angel::LookAt(globle_eye,globle_at,globle_up);
	glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_TRUE, view);
	

	Angel::mat4 perspectiveMat = Angel::Perspective((GLfloat)90.0, (GLfloat)width / (GLfloat)height, (GLfloat)0.1, (GLfloat)50);

	GLuint viewMatrix = glGetUniformLocation(program, "projection_matrix");
	glUniformMatrix4fv(viewMatrix, 1, GL_TRUE, perspectiveMat);


    glFlush(); // force output to graphics hardware

	glutSwapBuffers();

}

//----------------------------------------------------------------------------

// keyboard handler
void keyboard( unsigned char key, int x, int y )
{
	state = key;
	//vec4 model_color((rand() % 100 + 1)/100, (rand() % 100 + 1) / 100, (rand() % 100 + 1) / 100, 1);
	model_color = vec4((float(rand() % 100 + 1) / 100), (float(rand() % 100 + 1) / 100), (float(rand() % 100 + 1) / 100),1);
	GLuint modelColor = glGetUniformLocation(program, "color_input");
	Angel::mat4 view;
	switch (key) {
	case 033:
		exit(EXIT_SUCCESS);
		break;
	case 'a':
		turtle_index = 0;
		glUniform4f(modelColor, model_color[0], model_color[1], model_color[2], model_color[3]);
		break;
	case 'b':
		turtle_index = 1;
		glUniform4f(modelColor, model_color[0], model_color[1], model_color[2], model_color[3]);
		break;
	case 'c':
		turtle_index = 2;
		glUniform4f(modelColor, model_color[0], model_color[1], model_color[2], model_color[3]);
		break;
	case 'd':
		turtle_index = 3;
		glUniform4f(modelColor, model_color[0], model_color[1], model_color[2], model_color[3]);
		break;
	case 'e':
		turtle_index = 4;
		glUniform4f(modelColor, model_color[0], model_color[1], model_color[2], model_color[3]);
		break;

	case 'f':
		globle_up = point3(0, 0, 1);
		globle_at = point3(0, 0, 0);
		globle_eye = point3(10, 0, 0);
		view = Angel::LookAt(globle_eye, globle_at, globle_up);
		glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_TRUE, view);

		break;
	case 'g':

		break;
	case 't':
		
		break;
	case 'v':
		cout <<(float(rand() % 100 + 1) / 100) << endl;
		break;
	}
	
	glutPostRedisplay();
	
}



//----------------------------------------------------------------------------
// entry point
int main( int argc, char **argv )
{
	// init glut
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitWindowSize( 1024, 1024 );
	width = 1024;
	height = 1024;

	// create window
	// opengl can be incorperated into other packages like wxwidgets, fltoolkit, etc.
    glutCreateWindow( "PJ_3" );

	// init glew
    glewInit();

    generateGeometry();
	my_init();
	// assign handlers
    glutDisplayFunc( display );
    glutKeyboardFunc( keyboard );

	

	// should add menus
	// add mouse handler
	// add resize window functionality (should probably try to preserve aspect ratio)

	// enter the drawing loop
	// frame rate can be controlled with 
    glutMainLoop();
    return 0;
}



//floor
//own design of turtle string
//beeethoven