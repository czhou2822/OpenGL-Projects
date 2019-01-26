// Draws colored cube  
#include "Angel.h"
#include <string>
#include <Vector>
#include <fstream>
#include <iostream>     

//----------------------------------------------------------------------------
int width = 0;
int height = 0;

using namespace std;
// remember to prototype
void generateGeometry( void );
void display( void );
void keyboard( unsigned char key, int x, int y );
void quad( int a, int b, int c, int d );
void colorcube(void);
void drawCube(void);
void moveCube(void);
void select_current_drawing(int input);


typedef Angel::vec4  color4;
typedef Angel::vec4  point4;
typedef vec3 point3;
point3 translate_matrix = point3( 0,0,0 );
point3 scale_matrix = point3(0,0,0);
float shearing_counter = 1;
// handle to program
GLuint program;
point3 get_normal_vector(point3 input_a, point3 input_b, point3 input_c);
point3 get_middle_vector(point3 input_a, point3 input_b, point3 input_c);
using namespace std;

const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)
char state = 'W';
int move_counter = 0;
int ply_maximum = 43;
int ply_counter = 0;
float translate_delta = 0.0;
float current_scale = 0;
float breath_counter = 0;
float coefficient_input = 0;
point4 normal_input = {0,0,0,0};
bool breath_up = true;
bool normal_toggle = false;
int init_number = 0;
Angel::mat4 modelMat = Angel::identity();
float theta = 0;
float margin = 100;

point4 points[NumVertices];
color4 colors[NumVertices];

struct one_drawing{
	int number_of_vertices;
	int number_of_face;
	//float *extents;

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
	vector<point3> normal_array;
	mat4 model_matrix;
	vector<point3> points_giant_array;
	vector<point3> middle_point_array;
	float width;
	float height;
	float depth;
};


//one_drawing *the_mighty_array = (one_drawing *)malloc(sizeof(one_drawing) *43);
//char the_mighty_array_char = [ 'ply_files/airplane.ply', 'ply_files/ant.ply', 'ply_files/apple.ply' ];


one_drawing current_drawing;
//one_drawing the_mighty_array[43];

vector<one_drawing> the_mighty_array;
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

void ShaderSetup() {
	program = InitShader("vshader1.glsl", "fshader1.glsl");
	glUseProgram(program);
	glClearColor(1.0, 1.0, 1.0, 1.0);

	GLuint normal = glGetUniformLocationARB(program, "normal");
	glUniform4fv(normal, 1, normal_input);
	GLuint coe = glGetUniformLocationARB(program, "coefficient");
	glUniform1f(coe, coefficient_input);
}

void generateGeometry( void )
{	
    colorcube();

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
    glClearColor( 0.0, 0.0, 0.0, 1.0 ); // white background
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
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );
	glDisable( GL_DEPTH_TEST ); 
}

void my_draw(int input)
{
	// change to GL_FILL
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	// draw functions should enable then disable the features 
	// that are specifit the themselves
	// the depth is disabled after the draw 
	// in case you need to draw overlays
	glEnable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLES, 0, input);
	//glutSwapBuffers();
	glDisable(GL_DEPTH_TEST);
}

//void read_txt(char *fn) {
one_drawing read_txt(char *fn) {
	vector<point3> vertics_index;
	vector<point3> points;
	vector<point3> normal_output;
	vector<point3> raw_giant_array;
	vector<point3> middle_point_array_output;
	//vector<point3> vao_giant_array;

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

	//input_file >> context;
	//cout << "context: "<<context << '\n';
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
		points.push_back(point3(x,y,z));
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
	point3 *vao_giant_array = (point3 *)malloc(sizeof(point3) *5*num_faces);
	point3 *normal_giant_array = (point3 *)malloc(sizeof(point3) * 3 * num_faces);
	point3 *middle_giant_array = (point3 *)malloc(sizeof(point3) * 2 * num_faces);
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
		point3 normal_temp = get_normal_vector(points[vertix1], points[vertix2], points[vertix3]);
		point3 middle_temp = get_middle_vector(points[vertix1], points[vertix2], points[vertix3]);
		//cout << middle_temp << endl;
		

		
		normal_temp = normalize(normal_temp);

		middle_giant_array[middle_giant_array_index] = point3(middle_temp);
		middle_giant_array_index++;
		//cout << "temp: "<<middle_temp << endl;
		//cout << "array: "<<middle_giant_array[middle_giant_array_index] << endl;
		//cout << endl;
		
		middle_giant_array[middle_giant_array_index] = point3(middle_temp + normal_temp);
		middle_giant_array_index++;
		//cout << "temp + normal: "<<middle_temp + normal_temp << endl;
		//cout << "array: "<<middle_giant_array[middle_giant_array_index] << endl;
		//cout << endl;
		

		normal_giant_array[giant_array_index - 3] = normal_temp;
		normal_giant_array[giant_array_index - 2] = normal_temp;
		normal_giant_array[giant_array_index - 1] = normal_temp;

		

		vertics_index.push_back(temp);
		normal_output.push_back(normal_temp);
		middle_point_array_output.push_back(middle_temp);
		input_file >> context;
		//cout << temp << '\n';
	}
		
	for (int i = 0; i < num_faces; i ++) {
		vao_giant_array[giant_array_index] = middle_giant_array[2*i];
		giant_array_index++; 
		//cout << vao_giant_array[giant_array_index] << endl;
		//i++;
		vao_giant_array[giant_array_index] = middle_giant_array[2*i+1];
		//cout << vao_giant_array[giant_array_index] << endl;
		//cout << middle_giant_array[2 * i + 1] << endl;
	}

	// Create a vertex array object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(point3)* 5 * num_faces, vao_giant_array, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	GLuint buffer_2;
	glGenBuffers(1, &buffer_2);
	glBindBuffer(GL_ARRAY_BUFFER, buffer_2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(point3) * 3 * num_faces, normal_giant_array, GL_STATIC_DRAW);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(1);

	//cout << "num_face: " << num_faces << ", num_vertices: " << num_vertices<<'\n';

	//for (point3 n : vertics_index) {
	//	std::cout << n << '\n';
	//}

	//cout << x_min << " " << x_max << " \n";
	//cout << y_min << " " << y_max << " \n";
	//cout << z_min << " " << z_max << " \n";

	//translate_matrix = point3( -(x_min+x_max)/2, -(y_min + y_max) / 2,-(z_min + z_max) / 2 );
	//scale_matrix = point3( 1.0/ scale_factor,1.0/ scale_factor,1.0/ scale_factor);
	//current_scale = scale_factor;
	
	
	float extends_output[] = { x_min, x_max, y_min, y_max, z_min, z_max, scale_factor };

	

	one_drawing output{
		   num_vertices,
		   num_faces,
		   //extends_output,
		   x_min, x_max, y_min, y_max, z_min, z_max, scale_factor,
		   points,
		   vertics_index,
		   vao,
		   normal_output,
		   Angel::identity() * Angel::Scale(1 / scale_factor, 1 / scale_factor, 1 / scale_factor)* Angel::Translate(-(x_min + x_max) / 2,-(y_min + y_max) / 2, -(z_min + z_max) / 2),
		   raw_giant_array,
		   middle_point_array_output,
		   width_output,
		   height_output,
		   depth_output
 };
	//cout << "scale_factor: "<< output.extents[6] << endl;
	//cout << "scale_factor output: "<< output.extents[6] << endl;
	//globle_dummy = output;

	//modelMat = modelMat * Angel::Scale(1.0f / (10*scale_factor), 1.0f / (10*scale_factor), 1.0f / (10*scale_factor))* Angel::Translate(-(x_min + x_max) / 2, -(y_min + y_max) / 2, -(z_min + z_max) / 2);
	input_file.close();
	return output;

}

void toggle_normal(bool do_function) {
	//cout << do_function << endl;
	if (do_function == false) return;
	cout << "toggle_in" << endl;
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	// draw functions should enable then disable the features 
	// that are specifit the themselves
	// the depth is disabled after the draw 
	// in case you need to draw overlays
	//glEnable(GL_DEPTH_TEST);
	glDrawArrays(GL_LINES, 3 * current_drawing.number_of_face, 2 * current_drawing.number_of_face);
	//glutSwapBuffers();
	//glDisable(GL_DEPTH_TEST);
}

void define_current_draw(one_drawing input) {
	
	//cout << "num_face: " << input.number_of_face << ", num_vertices: " << input.number_of_vertices << '\n';
	//cout << input.model_matrix << endl;
	current_drawing = input;
	current_scale = input.scale_factor;
	//cout << current_scale << endl;
	modelMat = Angel::identity() * input.model_matrix;
	//current_scale = input.extents[6];
	glBindVertexArray(input.vao);
}

void pulsing_function(float input, one_drawing input_drawing) {
	//cout << input_drawing.points_giant_array[0] << endl;
	//cout << input_drawing.points_giant_array[1] << endl;
	//cout << input_drawing.points_giant_array[2] << endl;
	//cout << input_drawing.points_giant_array[3] << endl;
	//cout << input_drawing.points_giant_array[4] << endl;
	//cout << input_drawing.points_giant_array[5] << endl;
	
	//point3 p0 = point3(6, 1, 4);
	//point3 p1 = point3(7, 0, 9);
	//point3 p2 = point3(1, 1, 2);
	//point3 temp = p0 + 2*p1;
	//cout << temp << endl;

	int index = 0;
	point3 *normal_giant_array = (point3 *)malloc(sizeof(point3) * 3 * input_drawing.number_of_face);
	for (int i = 0; i < input_drawing.number_of_face; i++) {
		//int first_point = input_drawing.face_array[i].x;
		//int second_point = input_drawing.face_array[i].y;
		//int third_point = input_drawing.face_array[i].z;
		//point3 next_normal = input_drawing.normal_array[i];
		//float x = input_drawing.points_array[first_point].x + input * next_normal.x;
		//float y = input_drawing.points_array[first_point].y + input * next_normal.y;
		//float z = input_drawing.points_array[first_point].z + input * next_normal.z;
		//point3 first_normal_point = point3(x, y, z);
		//cout << index << endl;
		normal_giant_array[index] = input_drawing.points_giant_array[index] + input * input_drawing.normal_array[index];
		index++;
		//cout << index << endl;
		normal_giant_array[index] = input_drawing.points_giant_array[index] + input * input_drawing.normal_array[index];
		index++;
		//cout << index << endl;
		normal_giant_array[index] = input_drawing.points_giant_array[index] + input * input_drawing.normal_array[index];
		index++;
		//cout << index << endl;
	}

	//glBindVertexArray(input_drawing.vao);





	return;

}

void my_init() {

	the_mighty_array.push_back(read_txt("ply_files/airplane.ply"));
	the_mighty_array.push_back(read_txt("ply_files/ant.ply"));
	the_mighty_array.push_back(read_txt("ply_files/apple.ply"));
	the_mighty_array.push_back(read_txt("ply_files/balance.ply"));
	the_mighty_array.push_back(read_txt("ply_files/beethoven.ply"));
	the_mighty_array.push_back(read_txt("ply_files/big_atc.ply"));
	the_mighty_array.push_back(read_txt("ply_files/big_dodge.ply"));
	the_mighty_array.push_back(read_txt("ply_files/big_porsche.ply"));
	the_mighty_array.push_back(read_txt("ply_files/big_spider.ply"));
	the_mighty_array.push_back(read_txt("ply_files/canstick.ply"));
	the_mighty_array.push_back(read_txt("ply_files/chopper.ply"));
	the_mighty_array.push_back(read_txt("ply_files/cow.ply"));
	the_mighty_array.push_back(read_txt("ply_files/dolphins.ply"));
	the_mighty_array.push_back(read_txt("ply_files/egret.ply"));
	the_mighty_array.push_back(read_txt("ply_files/f16.ply"));
	the_mighty_array.push_back(read_txt("ply_files/footbones.ply"));
	the_mighty_array.push_back(read_txt("ply_files/fracttree.ply"));
	the_mighty_array.push_back(read_txt("ply_files/galleon.ply"));
	the_mighty_array.push_back(read_txt("ply_files/hammerhead.ply"));
	the_mighty_array.push_back(read_txt("ply_files/helix.ply"));
	the_mighty_array.push_back(read_txt("ply_files/hind.ply"));
	the_mighty_array.push_back(read_txt("ply_files/kerolamp.ply"));
	the_mighty_array.push_back(read_txt("ply_files/ketchup.ply"));
	the_mighty_array.push_back(read_txt("ply_files/mug.ply"));
	the_mighty_array.push_back(read_txt("ply_files/part.ply"));
	the_mighty_array.push_back(read_txt("ply_files/pickup_big.ply"));
	the_mighty_array.push_back(read_txt("ply_files/pump.ply"));
	the_mighty_array.push_back(read_txt("ply_files/pumpa_tb.ply"));
	the_mighty_array.push_back(read_txt("ply_files/sandal.ply"));
	the_mighty_array.push_back(read_txt("ply_files/saratoga.ply"));
	the_mighty_array.push_back(read_txt("ply_files/scissors.ply"));
	the_mighty_array.push_back(read_txt("ply_files/shark.ply"));
	the_mighty_array.push_back(read_txt("ply_files/steeringweel.ply"));
	the_mighty_array.push_back(read_txt("ply_files/stratocaster.ply"));
	the_mighty_array.push_back(read_txt("ply_files/street_lamp.ply"));
	the_mighty_array.push_back(read_txt("ply_files/teapot.ply"));
	the_mighty_array.push_back(read_txt("ply_files/tennis_shoe.ply"));
	the_mighty_array.push_back(read_txt("ply_files/tommygun.ply"));
	the_mighty_array.push_back(read_txt("ply_files/trashcan.ply"));
	the_mighty_array.push_back(read_txt("ply_files/turbine.ply"));
	the_mighty_array.push_back(read_txt("ply_files/urn2.ply"));
	the_mighty_array.push_back(read_txt("ply_files/walkman.ply"));
	the_mighty_array.push_back(read_txt("ply_files/weathervane.ply"));

}

point3 get_normal_vector(point3 input_a, point3 input_b, point3 input_c) {
	//cout << globle_dummy.face_array[0] << endl;
	//point3 p0 = globle_dummy.points_array[globle_dummy.face_array[0].x];
	//point3 p1 = globle_dummy.points_array[globle_dummy.face_array[0].y];
	//point3 p2 = globle_dummy.points_array[globle_dummy.face_array[0].z];

	//p0 = point3(6, 1, 4);
	//p1 = point3(7, 0, 9);
	//p2 = point3(1, 1, 2);


	//cout << input_a << endl;
	//cout << input_b << endl;
	//cout << input_c << endl;
	//x
	float x = (input_a.y - input_b.y)*(input_a.z + input_b.z) + (input_b.y - input_c.y)*(input_b.z + input_c.z) + (input_c.y - input_a.y)*(input_c.z + input_a.z);
	//y
	float y = (input_a.z - input_b.z)*(input_a.x + input_b.x) + (input_b.z - input_c.z)*(input_b.x + input_c.x) + (input_c.z - input_a.z)*(input_c.x + input_a.x);
	//z
	float z = (input_a.x - input_b.x)*(input_a.y + input_b.y) + (input_b.x - input_c.x)*(input_b.y + input_c.y) + (input_c.x - input_a.x)*(input_c.y + input_a.y);

	point3 p_output = point3(x, y, z);
	//cout << p_output << endl;
	return p_output;

}

point3 get_middle_vector(point3 input_a, point3 input_b, point3 input_c) {
	float x = (input_a.x + input_b.x + input_c.x) / 3;
	float y = (input_a.y + input_b.y + input_c.y) / 3;
	float z = (input_a.z + input_b.z + input_c.z) / 3;
	return point3(x, y, z);
}

//----------------------------------------------------------------------------
// this is where the drawing should happen
void display( void )
{
	// SOME RANDOM TIPS
	//========================================================================
	// remember to enable depth buffering when drawing in 3d

	// avoid using glTranslatex, glRotatex, push and pop
	// pass your own view matrix to the shader directly
	// refer to the latest OpenGL documentation for implementation details

    // Do not set the near and far plane too far appart!
	// depth buffers do not have unlimited resolution
	// surfaces will start to fight as they come nearer to each other
	// if the planes are too far appart (quantization errors :(   )

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );     // clear the window

	Angel::mat4 perspectiveMat = Angel::Perspective((GLfloat)90, (GLfloat)width/(GLfloat)height, (GLfloat)0.01, (GLfloat) 100.0);
	point3 up = point3(0, 1, 0);
	point3 at = point3(0,0,0);
	point3 eye = point3(0, 0, 1.3*current_drawing.z_max/current_scale);
	Angel::mat4 view = Angel::LookAt(up, at, vec3(0, 0, 5));
	glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_TRUE, view);

	float viewMatrixf[16];
	viewMatrixf[0] = perspectiveMat[0][0];viewMatrixf[4] = perspectiveMat[0][1];
	viewMatrixf[1] = perspectiveMat[1][0];viewMatrixf[5] = perspectiveMat[1][1];
	viewMatrixf[2] = perspectiveMat[2][0];viewMatrixf[6] = perspectiveMat[2][1];
	viewMatrixf[3] = perspectiveMat[3][0];viewMatrixf[7] = perspectiveMat[3][1];

	viewMatrixf[8] = perspectiveMat[0][2];viewMatrixf[12] = perspectiveMat[0][3];
	viewMatrixf[9] = perspectiveMat[1][2];viewMatrixf[13] = perspectiveMat[1][3];
	viewMatrixf[10] = perspectiveMat[2][2];viewMatrixf[14] = perspectiveMat[2][3];
	viewMatrixf[11] = perspectiveMat[3][2];viewMatrixf[15] = perspectiveMat[3][3];
	
	//Angel::mat4 modelMat = Angel::identity();
	//modelMat = modelMat * Angel::Translate(0.0, 0.0, -2.0f) * Angel::RotateY(45.0f) * Angel::RotateX(35.0f);
	
	//std::cout << modelMat << std::endl;
	


	//drawCube();

	switch (state) {
	case '0':
		//-ve direction
		//call idle function
		point3 p0 = point3(6, 1, 4);
		point3 p1 = point3(7, 0, 9);
		point3 p2 = point3(1, 1, 2);
		get_normal_vector(p0,p1,p2);
		break;
		
	}



	float modelMatrixf[16];
	modelMatrixf[0] = modelMat[0][0];modelMatrixf[4] = modelMat[0][1];
	modelMatrixf[1] = modelMat[1][0];modelMatrixf[5] = modelMat[1][1];
	modelMatrixf[2] = modelMat[2][0];modelMatrixf[6] = modelMat[2][1];
	modelMatrixf[3] = modelMat[3][0];modelMatrixf[7] = modelMat[3][1];

	modelMatrixf[8] = modelMat[0][2];modelMatrixf[12] = modelMat[0][3];
	modelMatrixf[9] = modelMat[1][2];modelMatrixf[13] = modelMat[1][3];
	modelMatrixf[10] = modelMat[2][2];modelMatrixf[14] = modelMat[2][3];
	modelMatrixf[11] = modelMat[3][2];modelMatrixf[15] = modelMat[3][3];
	
	// set up projection matricies
	GLuint modelMatrix = glGetUniformLocationARB(program, "model_matrix");
	glUniformMatrix4fv( modelMatrix, 1, GL_FALSE, modelMatrixf );
	GLuint viewMatrix = glGetUniformLocationARB(program, "projection_matrix");
	glUniformMatrix4fv( viewMatrix, 1, GL_FALSE, viewMatrixf);

	//cout << current_drawing.number_of_face * 3 << endl;
	my_draw(current_drawing.number_of_face*3);
	//glEnable(GL_DEPTH_TEST);
	//glPolygonMode();
	//glDrawArrays(GL_TRIANGLES, 0, 7356);




    glFlush(); // force output to graphics hardware

	// use this call to double buffer
	glutSwapBuffers();
	// you can implement your own buffers with textures
}

//// this is where the drawing should happen
//void display(void)
//{
//	// SOME RANDOM TIPS
//	//========================================================================
//	// remember to enable depth buffering when drawing in 3d
//
//	// avoid using glTranslatex, glRotatex, push and pop
//	// pass your own view matrix to the shader directly
//	// refer to the latest OpenGL documentation for implementation details
//
//	// Do not set the near and far plane too far appart!
//	// depth buffers do not have unlimited resolution
//	// surfaces will start to fight as they come nearer to each other
//	// if the planes are too far appart (quantization errors :(   )
//
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     // clear the window
//
//	Angel::mat4 perspectiveMat = Angel::Perspective((GLfloat)45.0, (GLfloat)width / (GLfloat)height, (GLfloat)0.1, (GLfloat) 100.0);
//
//	float viewMatrixf[16];
//	viewMatrixf[0] = perspectiveMat[0][0]; viewMatrixf[4] = perspectiveMat[0][1];
//	viewMatrixf[1] = perspectiveMat[1][0]; viewMatrixf[5] = perspectiveMat[1][1];
//	viewMatrixf[2] = perspectiveMat[2][0]; viewMatrixf[6] = perspectiveMat[2][1];
//	viewMatrixf[3] = perspectiveMat[3][0]; viewMatrixf[7] = perspectiveMat[3][1];
//
//	viewMatrixf[8] = perspectiveMat[0][2]; viewMatrixf[12] = perspectiveMat[0][3];
//	viewMatrixf[9] = perspectiveMat[1][2]; viewMatrixf[13] = perspectiveMat[1][3];
//	viewMatrixf[10] = perspectiveMat[2][2]; viewMatrixf[14] = perspectiveMat[2][3];
//	viewMatrixf[11] = perspectiveMat[3][2]; viewMatrixf[15] = perspectiveMat[3][3];
//
//	Angel::mat4 modelMat = Angel::identity();
//	modelMat = modelMat * Angel::Translate(0.0, 0.0, -2.0f) * Angel::RotateY(45.0f) * Angel::RotateX(35.0f);
//	float modelMatrixf[16];
//	modelMatrixf[0] = modelMat[0][0]; modelMatrixf[4] = modelMat[0][1];
//	modelMatrixf[1] = modelMat[1][0]; modelMatrixf[5] = modelMat[1][1];
//	modelMatrixf[2] = modelMat[2][0]; modelMatrixf[6] = modelMat[2][1];
//	modelMatrixf[3] = modelMat[3][0]; modelMatrixf[7] = modelMat[3][1];
//
//	modelMatrixf[8] = modelMat[0][2]; modelMatrixf[12] = modelMat[0][3];
//	modelMatrixf[9] = modelMat[1][2]; modelMatrixf[13] = modelMat[1][3];
//	modelMatrixf[10] = modelMat[2][2]; modelMatrixf[14] = modelMat[2][3];
//	modelMatrixf[11] = modelMat[3][2]; modelMatrixf[15] = modelMat[3][3];
//
//	// set up projection matricies
//	GLuint modelMatrix = glGetUniformLocationARB(program, "model_matrix");
//	glUniformMatrix4fv(modelMatrix, 1, GL_FALSE, modelMatrixf);
//	GLuint viewMatrix = glGetUniformLocationARB(program, "projection_matrix");
//	glUniformMatrix4fv(viewMatrix, 1, GL_FALSE, viewMatrixf);
//
//	drawCube();
//	//my_draw(7356);
//	glFlush(); // force output to graphics hardware
//
//	// use this call to double buffer
//	glutSwapBuffers();
//	// you can implement your own buffers with textures
//}




mat4 shear(float input) {
	Angel::mat4 output = Angel::identity();
	output[0][1] = output[2][1]= input;
	return output;
}
//----------------------------------------------------------------------------
//void select_current_drawing(int input) {
//	switch (input) {
//	case 0:
//		current_drawing = read_txt("ply_files/airplane.ply");
//		return;
//	case 1:
//		current_drawing = read_txt("ply_files/ant.ply");
//		return;
//	case 2:
//		current_drawing = read_txt("ply_files/apple.ply");
//		return;
//	}
//}
// keyboard handler
void keyboard( unsigned char key, int x, int y )
{	
	cout << "state: " << key << endl;
    switch ( key ) {
    case 033:
        exit( EXIT_SUCCESS );
        break;
	case'W':
		state = key;
		move_counter = 0;
		translate_delta = 0;
		break;
	case 'N':
		//next wireframe
		state = key;
		move_counter = 0;
		translate_delta = 0;
		ply_counter++;
		if (ply_counter == ply_maximum) ply_counter = 0;
		//select_current_drawing(ply_counter);
		//display();
		cout << ply_counter << endl;
		break;
	case 'P':
		//previous wireframe
		state = key;
		move_counter = 0;
		translate_delta = 0;
		ply_counter--;
		if (ply_counter < 0) ply_counter = ply_maximum-1;
		//select_current_drawing(ply_counter);
		//display();
		cout << ply_counter << endl;
		break;
	case 'x':
	case 'X':
	case 'y':
	case 'Y':
	case 'z':
	case 'Z':
	case 'R':
		//rotation
		state = key;
		move_counter = 0;
		translate_delta = 0;
		//cout << "state: " << key << endl;
		break;
	case 'B':
		state = key;
		move_counter = 0;
		translate_delta = 0;
		break;
	case 'h':
		//increase shearing
		state = 'h';
		move_counter = 0;
		translate_delta = 0;
		if (shearing_counter < 0) shearing_counter = 0;
		shearing_counter++;
		modelMat = modelMat * shear(shearing_counter / current_scale);
		cout << modelMat << endl;
		break;
	case 'H':
		//decrease shearing
		state = 'H';
		move_counter = 0;
		translate_delta = 0;
		if (shearing_counter > 0) shearing_counter = 0;
		shearing_counter--;
		modelMat = modelMat * shear(shearing_counter / current_scale);
		//cout << modelMat << endl;
		break;

	case 'm':
		state = 'm';
		normal_toggle = !normal_toggle;
		break;
	}
}

void moveCube() {
	move_counter++;
	
	//cout << "translate_delta: "<< (translate_delta*current_drawing.x_max / current_scale) << endl;
	//if (move_counter == 5) {
	//	//cout <<"translate_delta: "<<translate_delta<< '\n';
	//	//cout <<"current_scale: "<<current_scale << '\n';
	//	//cout << "input.extents[6] " << current_drawing.extents[1] << endl;
	//	move_counter = 0;
	//	translate_delta++;
	//
	//}

	if (move_counter == 100) {
		//do stuff
		//cout << "idle function. " << endl;
		translate_delta = translate_delta+5;
		if (translate_delta == 1000) translate_delta = 0;
		switch (state) {
		case'W':
		case 'N':
		case 'P':
			define_current_draw(the_mighty_array[ply_counter]);

			break;
		case 'R':
			//rotation, to be done
			theta += 1.5;
			if (theta > 360.0) theta -= 360.0;

			//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			modelMat = current_drawing.model_matrix*Angel::RotateX(theta);

			break;
		case 'x':
			//cout << translate_delta / 100 << '\n';
			modelMat = current_drawing.model_matrix* Angel::Translate(-(current_drawing.width*translate_delta/100) / current_scale, 0, 0);
			break;
		case 'X':
			modelMat = current_drawing.model_matrix * Angel::Translate(((current_drawing.width*translate_delta / 100) / current_scale), 0, 0);
			break;
		case 'y':
			modelMat = current_drawing.model_matrix * Angel::Translate(0, -((current_drawing.height*translate_delta / 100) / current_scale), 0);
			break;
		case 'Y':
			modelMat = current_drawing.model_matrix * Angel::Translate(0, ((current_drawing.height*translate_delta / 100) / current_scale), 0);
			break;
		case 'z':
			modelMat = current_drawing.model_matrix * Angel::Translate(0, 0, -((current_drawing.depth*translate_delta / 100) / current_scale));
			break;
		case 'Z':
			modelMat = current_drawing.model_matrix * Angel::Translate(0, 0, (current_drawing.depth*translate_delta / 100) / current_scale);
			break;
		case 'B':

			if (breath_up == true) coefficient_input +=0.1;
			else coefficient_input-=0.1;

			if (coefficient_input > 1) breath_up = false;
			else if (coefficient_input <= 0) breath_up = true;
			//cout << "breath counter: "<< coefficient_input << endl;
			//coefficient_input += 0.1;
			//if (coefficient_input > 5) coefficient_input -= 0.1;
			//else if (coefficient_input <-2) coefficient_input += 0.1;
			glUniform1f(glGetUniformLocation(program, "coefficient"), coefficient_input);
			//pulsing_function(breath_counter / 100, current_drawing);
			break;
		case 'h':
		case 'H':
			//modelMat = modelMat * shear(shearing_counter / current_scale);
			//cout << modelMat << endl;
			break;
		case 'm':
			
			toggle_normal(normal_toggle);
			break;
		default:
			//cout << "default case" << endl;
			break;
		}



		move_counter = 0;
	}
	

	glutPostRedisplay();
	//display();

}

//----------------------------------------------------------------------------
// entry point
int main( int argc, char **argv )
{
	// init glut
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitWindowSize( 512, 512 );
	width = 512;
	height = 512;

	// create window
	// opengl can be incorperated into other packages like wxwidgets, fltoolkit, etc.
    glutCreateWindow( "second project" );

	// init glew
    glewInit();
	//ShaderSetup();
	my_init();

    //generateGeometry();
	 ShaderSetup();
	// assign handlers
    glutDisplayFunc( display );
    glutKeyboardFunc( keyboard );
	glutIdleFunc(moveCube);
	// should add menus
	// add mouse handler
	// add resize window functionality (should probably try to preserve aspect ratio)

	// enter the drawing loop
	// frame rate can be controlled with 
    glutMainLoop();
    return 0;
}
