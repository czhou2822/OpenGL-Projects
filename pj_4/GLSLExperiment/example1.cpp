// Draws colored cube  

#include "Angel.h"
#include <string>
#include <Vector>
#include <fstream>
#include <iostream>     
#include "turtle_string.h"
#include "single_line.h"
#include <stack>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//----------------------------------------------------------------------------
int width = 0;
int height = 0;

// remember to prototype

void display( void );
void keyboard( unsigned char key, int x, int y );


point3 get_normal_vector(point3 input_a, point3 input_b, point3 input_c);

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;
typedef vec3 point3;
// handle to program
GLuint program;
GLuint vao_plain;
GLuint texture_grass;
GLuint texture_stone;
GLuint texture_cubemap;
GLuint modelMatrix;
vec4 model_color;


using namespace std;
float theta = 0;
bool texture_toggle = false;
bool shadow_toggle = false;
bool extends_toggle = false;
bool reflection_toggle = false;
bool refraction_toggle = false;
bool show_fog = false;
float fog_max = 10;

float z_pos = 0;

point3 up = point3(0, 1, 0);
point3 at = point3(0, 0, 0);
point3 eye = point3(5, 5, 5);
mat4   globle_modelMat = Angel::identity();
mat4   current_matrix = Angel::identity();
mat4   floor_plain_matrix =Angel::RotateX(90)*Angel::identity();
mat4   right_plain_matrix = Angel::RotateY(-90)*Angel::identity();
mat4   back_plain_matrix = Angel::identity();
mat4   shadow_matrix = Angel::identity();
mat4   point_of_start_matrix = Angel::Translate(3,3,3)*Angel::identity();
mat4   rotation_matrix = Angel::identity();



//buddha coefficient
vec4 ambiant = vec4(0.33, 0.22, 0.03, 1);
vec4 diffuse = vec4(0.78, 0.57, 0.11, 1);
vec4 specular = vec4(0.99, 0.94, 0.81, 1);
float shininess = 10;
float refraction_index = 1.5;

vector<std::string> faces = {
	"ply_files/nvposx.bmp",
	"ply_files/nvnegx.bmp",
	"ply_files/nvposy.bmp",
	"ply_files/nvnegy.bmp",
	"ply_files/nvposz.bmp",
	"ply_files/nvnegz.bmp"
};


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
	vector<point3> normal_array;
	mat4 model_matrix;
	vector<point3> points_giant_array;


};

struct Spot_Light_Struct {
	vec4 position  = point4(3,10,3, 1);
	vec4 direction = point4(0,-1,0, 0);
	vec4 light_color = point4(1,1,1, 1);
	float cut_off = 23.4;
	float light_intensity = 3;
};

Spot_Light_Struct spot_light;
one_drawing buddha;




void shader_setup() {
	program = InitShader("vshader1.glsl", "fshader1.glsl");
	glUseProgram(program);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	Angel::mat4 view = Angel::LookAt(eye, at, up);
	glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_TRUE, view);
	modelMatrix = glGetUniformLocation(program, "model_matrix");
	Angel::mat4 perspectiveMat = Angel::Perspective((GLfloat)90.0, (GLfloat)width / (GLfloat)height, (GLfloat)0.1, (GLfloat)50);
	GLuint viewMatrix = glGetUniformLocation(program, "projection_matrix");
	glUniformMatrix4fv(viewMatrix, 1, GL_TRUE, perspectiveMat);

	glUniformMatrix3fv(glGetUniformLocation(program, "camaraPos"), 1, GL_TRUE, eye);





}

void setup_cutoff(float input) {
	glUniform1f(glGetUniformLocation(program, "cut_off_angle"), input);
	cout << "cut_off_angle: " << spot_light.cut_off << endl;
}

void setup_fog(float input) {
	glUniform1f(glGetUniformLocation(program, "fog_max"), input);
	cout << "fog_max: " << fog_max << endl;
}

void setup_shininess(float input) {
	glUniform1f(glGetUniformLocation(program, "shininess"), input);
	cout << "shininess: " << shininess << endl;
}

void setup_refraction_index(float input) {
	glUniform1f(glGetUniformLocation(program, "refraction_index"), input);
	cout << "refraction_index: " << refraction_index << endl;
}

void setup_light() {

	glUniform4fv(glGetUniformLocation(program, "light_position"), 1, spot_light.position);
	glUniform1f(glGetUniformLocation(program, "light_intensity"), spot_light.light_intensity);
	glUniform4fv(glGetUniformLocation(program, "light_direction"), 1, spot_light.direction);
	glUniform4fv(glGetUniformLocation(program, "Ka"), 1, ambiant);
	glUniform4fv(glGetUniformLocation(program, "Kd"), 1, diffuse);
	glUniform4fv(glGetUniformLocation(program, "Ks"), 1, specular);
	
	setup_cutoff(45);


}

void define_plain() {

	point3 t1 = point3(5, 0, 0);
	point3 t2 = point3(0, 0, 0);
	point3 t3 = point3(5, 5, 0);
	point3 t4 = point3(0, 5, 0);
	point3 plain_array[] = { t2,t3,t4,t2,t3,t1 };

	glGenVertexArrays(1, &vao_plain);
	glBindVertexArray(vao_plain);

	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(point3) * 6 , plain_array, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

}

void draw_plain() {

	glBindVertexArray(vao_plain);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisable(GL_DEPTH_TEST);
}

void draw_ply(one_drawing input)
{
	glBindVertexArray(input.vao);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	current_matrix = input.model_matrix;

	glEnable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLES, 0, input.number_of_face*3);
	glDisable(GL_DEPTH_TEST);
}

void draw_extends() {
	//back face
	point3 p1 = point3(buddha.x_min, buddha.y_min, buddha.z_min);
	point3 p2 = point3(buddha.x_max, buddha.y_min, buddha.z_min);
	point3 p3 = point3(buddha.x_max, buddha.y_max, buddha.z_min);
	point3 p4 = point3(buddha.x_min, buddha.y_max, buddha.z_min);
	//front face
	point3 p5 = point3(buddha.x_min, buddha.y_min, buddha.z_max);
	point3 p6 = point3(buddha.x_max, buddha.y_min, buddha.z_max);
	point3 p7 = point3(buddha.x_max, buddha.y_max, buddha.z_max);
	point3 p8 = point3(buddha.x_min, buddha.y_max, buddha.z_max);


	point3 extends_array[] = { p1, p2, p2, p3, p3, p4, p4, p1,
							   p5, p6, p6, p7, p7, p8, p8, p5,
							   p4, p8, p7, p3, p1, p5, p2, p6 };

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(point3) * 24, extends_array, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glEnable(GL_DEPTH_TEST);
	glDrawArrays(GL_LINES, 0, 24);
	glDisable(GL_DEPTH_TEST);

}

one_drawing read_txt(char *fn) {
	vector<point3> vertics_index;
	vector<point3> points;
	vector<point3> normal_output;
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
		
	}

	float scale_factor = x_max;
	if (y_max > x_max) {
		if (z_max > x_max)
			scale_factor = z_max;
	}
	else if (z_max > x_max) {
		scale_factor = z_max;
	}



	scale_factor = abs(scale_factor);
	point3 *vao_giant_array = (point3 *)malloc(sizeof(point3) * 3 * num_faces);
	point3 *normal_giant_array = (point3 *)malloc(sizeof(point3) * 3 * num_faces);

	int giant_array_index = 0;

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
		
		
						
		normal_giant_array[giant_array_index - 3] += normal_temp;
		normal_giant_array[giant_array_index - 2] += normal_temp;
		normal_giant_array[giant_array_index - 1] += normal_temp;
		//normal_temp = normalize(normal_temp);

		//vertics_index.push_back(temp);
		//normal_output.push_back(normal_temp);

		input_file >> context;
		//cout << temp << '\n';
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

	GLuint buffer_2;
	glGenBuffers(1, &buffer_2);
	glBindBuffer(GL_ARRAY_BUFFER, buffer_2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(point3) * 3 * num_faces, normal_giant_array, GL_STATIC_DRAW);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(1);


	one_drawing output{
		   num_vertices,
		   num_faces,
		   x_min, x_max, y_min, y_max, z_min, z_max, scale_factor,
		   points,
		   vertics_index,
		   vao,
		   normal_output,
		   Angel::identity() * Angel::Scale(1 / scale_factor, 1 / scale_factor, 1 / scale_factor)* Angel::Translate(-(x_min + x_max) / 2,-(y_min + y_max) / 2, -(z_min + z_max) / 2),
		   raw_giant_array
	};
	input_file.close();
	return output;

}

point3 get_normal_vector(point3 input_a, point3 input_b, point3 input_c) {

	float x = (input_a.y - input_b.y)*(input_a.z + input_b.z) + (input_b.y - input_c.y)*(input_b.z + input_c.z) + (input_c.y - input_a.y)*(input_c.z + input_a.z);
	//y
	float y = (input_a.z - input_b.z)*(input_a.x + input_b.x) + (input_b.z - input_c.z)*(input_b.x + input_c.x) + (input_c.z - input_a.z)*(input_c.x + input_a.x);
	//z
	float z = (input_a.x - input_b.x)*(input_a.y + input_b.y) + (input_b.x - input_c.x)*(input_b.y + input_c.y) + (input_c.x - input_a.x)*(input_c.y + input_a.y);

	point3 p_output = point3(x, y, z);
	return p_output;

}

void texture_init() {
	
	glGenTextures(1, &texture_grass);
	glBindTexture(GL_TEXTURE_2D, texture_grass);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	string file_path = "ply_files/grass.bmp";
	cout << "Loading Texture : " << file_path << endl;
	unsigned char *data_0 = stbi_load(file_path.c_str(), &width, &height, &nrChannels, 0);
	if (data_0)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_0);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		cout << "Failed to load texture data : " << file_path << endl;
	}
	stbi_image_free(data_0);


	//GLuint texture_stone;
	glGenTextures(1, &texture_stone);
	glBindTexture(GL_TEXTURE_2D, texture_stone);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	stbi_set_flip_vertically_on_load(true);
	file_path = "ply_files/stones.bmp";
	cout << "Loading Texture : " << file_path << endl;
	unsigned char *data_1 = stbi_load(file_path.c_str(), &width, &height, &nrChannels, 0);
	if (data_1)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_1);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		cout << "Failed to load texture data : " << file_path << endl;
	}
	stbi_image_free(data_1);


}

void cube_map_init() {
	glGenTextures(1, &texture_cubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture_cubemap);

	int width, height, nrChannels;
	unsigned char *data;
	for (GLuint i = 0; i < faces.size(); i++)
	{
		data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
		);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);



}

void shadow_matrix_init() {
	shadow_matrix[3][1] = -1 / (spot_light.position.y);
	shadow_matrix[3][3] = 0;
	cout << shadow_matrix << endl;

	shadow_matrix = Angel::Translate(spot_light.position.x, spot_light.position.y, spot_light.position.z) *
		shadow_matrix *
		Angel::Translate(-spot_light.position.x, -spot_light.position.y, -spot_light.position.z);

}




void my_init() {


	buddha = read_txt("ply_files/cow.ply");
	buddha = read_txt("ply_files/happy.ply");
	
	setup_light();
	texture_init();
	define_plain();
	shadow_matrix_init();
	cube_map_init();
	
	setup_shininess(shininess);
	setup_fog(10);

}

//----------------------------------------------------------------------------
// this is where the drawing should happen
void display( void )
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );     // clear the window
	

	glUniform1i(glGetUniformLocation(program, "texture_state"), 2);

	if (texture_toggle == true) {
		//floor
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_grass);
		glUniform1i(glGetUniformLocation(program, "texture_grass"), 0);
		glUniform1i(glGetUniformLocation(program, "texture_state"), 0);
		glUniformMatrix4fv(modelMatrix, 1, GL_TRUE, floor_plain_matrix);
		draw_plain();

		//right
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture_stone);
		glUniform1i(glGetUniformLocation(program, "texture_stone"), 1);
		glUniform1i(glGetUniformLocation(program, "texture_state"), 1);
		glUniformMatrix4fv(modelMatrix, 1, GL_TRUE, right_plain_matrix);
		draw_plain();

		//back
		glUniformMatrix4fv(modelMatrix, 1, GL_TRUE, back_plain_matrix);
		draw_plain();
	}
	else {
		//floor
		glUniform1i(glGetUniformLocation(program, "texture_state"), 4);
		glUniformMatrix4fv(modelMatrix, 1, GL_TRUE, floor_plain_matrix);
		draw_plain();

		//right
		glUniform1i(glGetUniformLocation(program, "texture_state"), 3);
		glUniformMatrix4fv(modelMatrix, 1, GL_TRUE, right_plain_matrix);
		draw_plain();

		//back
		glUniformMatrix4fv(modelMatrix, 1, GL_TRUE, back_plain_matrix);
		draw_plain();
	}



	if (reflection_toggle == true) {
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture_cubemap);
		glUniform1i(glGetUniformLocation(program, "texture_state"), 7);
	}
	else if (refraction_toggle == true) {
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture_cubemap);
		glUniform1i(glGetUniformLocation(program, "texture_state"), 8);
	}
	else glUniform1i(glGetUniformLocation(program, "texture_state"), 2);

	//glUniform1i(glGetUniformLocation(program, "texture_state"), 2);
	glUniformMatrix4fv(modelMatrix, 1, GL_TRUE, rotation_matrix);
	draw_ply(buddha);
	


	if (extends_toggle == true) {
		glUniform1i(glGetUniformLocation(program, "texture_state"), 6);
		draw_extends();
	}

	if (shadow_toggle == true) {
		glUniformMatrix4fv(modelMatrix, 1, GL_TRUE, shadow_matrix * rotation_matrix);
		glUniform1i(glGetUniformLocation(program, "texture_state"), 5);
		draw_ply(buddha);
	}

	if (show_fog == true) {
		mat4 mat_temp = Angel::Translate(0, 0, 1)*rotation_matrix;;
		glUniformMatrix4fv(modelMatrix, 1, GL_TRUE, mat_temp);
		draw_ply(buddha);

		mat_temp = Angel::Translate(0, 0, 2)*rotation_matrix;;
		glUniformMatrix4fv(modelMatrix, 1, GL_TRUE, mat_temp);
		draw_ply(buddha);

		mat_temp = Angel::Translate(0, 0, 3)*rotation_matrix;;
		glUniformMatrix4fv(modelMatrix, 1, GL_TRUE, mat_temp);
		draw_ply(buddha);

	}





    glFlush(); // force output to graphics hardware

	glutSwapBuffers();

}

void reset_fog() {
	fog_max = 10;
	setup_fog(fog_max);
}

void movdBuddha() {
	theta += 0.5;
	if (theta > 360.0) theta -= 360.0;
	rotation_matrix = point_of_start_matrix * Angel::RotateY(theta) * buddha.model_matrix;
	glutPostRedisplay();

}
//----------------------------------------------------------------------------

// keyboard handler
void keyboard( unsigned char key, int x, int y )
{
	show_fog = false;

	switch (key) {
	case 033:
		exit(EXIT_SUCCESS);
		break;

	case 'A':
		shadow_toggle = !shadow_toggle;
		reset_fog();
		break;

	case 'B':
		texture_toggle = !texture_toggle;
		reset_fog();
		break;

	case 'C':
		reflection_toggle = !reflection_toggle;
		refraction_toggle = false;
		reset_fog();
		break;

	case 'D':
		refraction_toggle = !refraction_toggle;
		reflection_toggle = false;
		reset_fog();
		break;

	case 'E':
		extends_toggle = !extends_toggle;
		reset_fog();
		break;

	case 'P':
		spot_light.cut_off = spot_light.cut_off + 0.3;
		setup_cutoff(spot_light.cut_off);
		reset_fog();
		break;
	case 'p':
		spot_light.cut_off = spot_light.cut_off - 0.3;
		setup_cutoff(spot_light.cut_off);
		reset_fog();
		break;
	
	case 'S':
		shininess = shininess + 1;
		setup_shininess(shininess);
		reset_fog();
		break;
	case 's':
		shininess = shininess - 1;
		setup_shininess(shininess);
		reset_fog();
		break;

	case 'I':
		refraction_index = refraction_index + 0.2;
		setup_refraction_index(refraction_index);
		reset_fog();
		break;
	case 'i':
		refraction_index = refraction_index - 0.2;
		setup_refraction_index(refraction_index);
		reset_fog();
		break;

	case 'F':
		fog_max++;
		show_fog = true;
		setup_fog(fog_max);
		break;
	case 'f':
		fog_max--;
		show_fog = true;
		setup_fog(fog_max);
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
    glutCreateWindow( "PJ_4" );

	// init glew
    glewInit();

    //generateGeometry();

	shader_setup();

	my_init();
	// assign handlers
    glutDisplayFunc( display );
    glutKeyboardFunc( keyboard );
	glutIdleFunc(movdBuddha);
	

	// should add menus
	// add mouse handler
	// add resize window functionality (should probably try to preserve aspect ratio)

	// enter the drawing loop
	// frame rate can be controlled with 
    glutMainLoop();
    return 0;
}




//shadow
//