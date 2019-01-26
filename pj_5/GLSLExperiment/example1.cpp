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

#include <complex>
typedef std::complex<float>  Complex;

inline float Random() { return float(rand()) / RAND_MAX; }


//----------------------------------------------------------------------------
float width = 0.5;
float height = 0.5;

// remember to prototype

void display( void );
void keyboard( unsigned char key, int x, int y );


typedef Angel::vec4  color4;
typedef Angel::vec4  point4;
typedef vec3 point3;
// handle to program
GLuint program;
GLuint vao_plain;
GLuint texture_og;
GLuint texture_mandlebrot;

GLuint modelMatrix;
vec4 model_color;


using namespace std;

float tolerance = 0.5;
float quantize = 4;

bool og_toggle = true;
bool luminance_toggle = false;
bool negative_toggle = false;
bool toon_toggle = false;
bool twirl_toggle = false;
bool edge_toggle = false;
bool embossing_toggle = false;
bool ripple_toggle = false;
bool spherical_toggle = false;
bool mandlebrot_toggle = false;
char state;








void shader_setup() {
	program = InitShader("vshader1.glsl", "fshader1.glsl");
	glUseProgram(program);
	glClearColor(1.0, 1.0, 1.0, 1.0);


	Angel::mat4 perspectiveMat = Angel::Ortho2D(0,1,0,1);
	GLuint viewMatrix = glGetUniformLocation(program, "projection_matrix");
	glUniformMatrix4fv(viewMatrix, 1, GL_TRUE, perspectiveMat);

	glUniform1f(glGetUniformLocation(program, "uT"), tolerance);
	glUniform1f(glGetUniformLocation(program, "uQuantize"), quantize);



}

void define_plain() {

	point3 t1 = point3(1, 0, 0);
	point3 t2 = point3(0, 0, 0);
	point3 t3 = point3(1, 1, 0);
	point3 t4 = point3(0, 1, 0);
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

	glDrawArrays(GL_TRIANGLES, 0, 6);

}

void texture_init() {
	
	GLuint texture;
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &texture_og);
	glBindTexture(GL_TEXTURE_2D, texture_og);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	string file_path = "usain_bolt.bmp";
	cout << "Loading Texture : " << file_path << endl;
	unsigned char *data = stbi_load(file_path.c_str(), &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		cout << "Failed to load texture data : " << file_path << endl;
	}
	stbi_image_free(data);

}
GLfloat image[512][512][3];

void mendlebrot_init() {
	const int N = 512;
	const int M = 512;
	int n = 512;
	int m = 512;
	int max = 100;
	float cx = -0.5;
	float cy = 0.5;

	//s = 0;
	//c = 0.2+0.5i;

	float width = 0.5;
	float height = 0.5;

	for (int i = 0; i < N; i++)
		for (int j = 0; j < M; j++) {

			float x = i * (width / (n - 1)) + cx - width / 2;
			float y = j * (height / (m - 1)) + cy - height / 2;

			// cout << x << " " << y << endl;

			Complex c(0.0, 0.0);
			Complex p(x, y);

			float  v;
			for (int k = 0; k < max; k++) {
				// cout << k << endl;
				// compute c = c^2 + p
				c *= c;
				c += p;
				v = norm(c);
				if (v > 4.0)
					break;      /* assume not in set if mag > 4 */
			}


			// assign gray level to point based on its magnitude */

			if (v > 1.0)
				v = 1.0;        /* clamp if > 1 */
			image[i][j][0] = v;
			image[i][j][1] = Random();
			image[i][j][1] = 2.0 * sin(v) - 1.0;
			image[i][j][2] = 1.0 - v;
		}


	glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &texture_mandlebrot);
	glBindTexture(GL_TEXTURE_2D, texture_mandlebrot);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, n, m, 0, GL_RGB, GL_FLOAT, image);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);



}

void my_init() {



	

	texture_init();
	define_plain();
	mendlebrot_init();


}

//----------------------------------------------------------------------------
// this is where the drawing should happen
void display( void )
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );     // clear the window
	
	glViewport(0, 0, 512, 512);


	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_og);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture_mandlebrot);
	glUniform1i(glGetUniformLocation(program, "mandlebrot"), 1);


	draw_plain();
	



    glFlush(); // force output to graphics hardware

	glutSwapBuffers();

}



// keyboard handler
void keyboard( unsigned char key, int x, int y )
{
	state = key;
	cout << key << endl;
	glUniform1i(glGetUniformLocation(program, "texture_state"), 0);
	switch (key) {
	case 033:
		exit(EXIT_SUCCESS);
		break;

	case 'o':
		og_toggle == !og_toggle;
		if (og_toggle == true) glUniform1i(glGetUniformLocation(program, "texture_state"), 0);
		break;

	case 'l':
		luminance_toggle = !luminance_toggle;
		if (luminance_toggle == true) glUniform1i(glGetUniformLocation(program, "texture_state"), 1);
		break;

	case 'n':
		negative_toggle = !negative_toggle;
		if (negative_toggle == true) glUniform1i(glGetUniformLocation(program, "texture_state"), 2);
		break;

	case 'd':
		edge_toggle = !edge_toggle;
		if (edge_toggle == true) glUniform1i(glGetUniformLocation(program, "texture_state"), 3);
		break;

	case 'e':
		embossing_toggle = !embossing_toggle;
		if (embossing_toggle == true) glUniform1i(glGetUniformLocation(program, "texture_state"), 4);
		break;

	case 't':
		toon_toggle = !toon_toggle;
		if (toon_toggle == true) glUniform1i(glGetUniformLocation(program, "texture_state"), 5);
		break;

	case 'w':
		twirl_toggle = !twirl_toggle;
		if (twirl_toggle == true) glUniform1i(glGetUniformLocation(program, "texture_state"), 6);
		break;

	case 'p':
		ripple_toggle = !ripple_toggle;
		if (ripple_toggle == true) glUniform1i(glGetUniformLocation(program, "texture_state"), 7);
		break;

	case 's':
		spherical_toggle = !spherical_toggle;
		if (spherical_toggle == true) glUniform1i(glGetUniformLocation(program, "texture_state"), 8);
		break;

	case 'r':
		mandlebrot_toggle = !mandlebrot_toggle;
		if (mandlebrot_toggle == true) glUniform1i(glGetUniformLocation(program, "texture_state"), 9);
		
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
    glutInitWindowSize( 512, 512 );
	width = 512;
	height = 512;

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