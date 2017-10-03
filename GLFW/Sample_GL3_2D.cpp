#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include<unistd.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <FTGL/ftgl.h>

using namespace std;

struct VAO {
	GLuint VertexArrayID;
	GLuint VertexBuffer;
	GLuint ColorBuffer;

	GLenum PrimitiveMode;
	GLenum FillMode;
	int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

struct FTGLFont {
  FTFont* font;
  GLuint fontMatrixID;
  GLuint fontColorID;
} GL3Font;

GLuint programID, fontProgramID, textureProgramID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
	glfwDestroyWindow(window);
	glfwTerminate();
	//    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;

	// Create Vertex Array Object
	// Should be done after CreateWindow and before any other GL calls
	glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
	glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
	glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

	glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
	glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
	glVertexAttribPointer(
			0,                  // attribute 0. Vertices
			3,                  // size (x,y,z)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
	glVertexAttribPointer(
			1,                  // attribute 1. Color
			3,                  // size (r,g,b)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
	GLfloat* color_buffer_data = new GLfloat [3*numVertices];
	for (int i=0; i<numVertices; i++) {
		color_buffer_data [3*i] = red;
		color_buffer_data [3*i + 1] = green;
		color_buffer_data [3*i + 2] = blue;
	}

	return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
	// Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

	// Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

	// Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

	// Enable Vertex Attribute 1 - Color
	glEnableVertexAttribArray(1);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

	// Draw the geometry !
	glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/
int l6=0,r6=0,l7=0,r7=0,r8=0,r9=0;

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
int posx1=0,posy1=0,posx2=0,posy2=6,flag=1,posz1=0,posz2=0;
int triangle_rotation;
int disable=0;
int l8f=0;
int moves=0;
int stmove=0;
int ent=0;
int enter=0;
char ab[2];
int blo=0;
int lmouse=0;
int pass=0;
int view=0;
int menu=0;
int soff=0;
int l3=0,r3=0;
int r4=0;
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Function is called first on GLFW_PRESS.

	if (action == GLFW_RELEASE) {
		switch (key) {
			case GLFW_KEY_C:
				rectangle_rot_status = !rectangle_rot_status;
				break;
			case GLFW_KEY_P:
				triangle_rot_status = !triangle_rot_status;
				break;
			case GLFW_KEY_X:
				// do something ..
				break;
			default:
				break;
		}
	}
	else if (action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_ESCAPE:
				quit(window);
				break;
			case GLFW_KEY_ENTER:
				ent=1;
				break;
			default:
				break;
		}

	}
	if(pass==1 && blo==0){
		if(key==GLFW_KEY_1)
	sprintf(ab,"1");

		if(key==GLFW_KEY_2)
	sprintf(ab,"2");

		if(key==GLFW_KEY_3)
	sprintf(ab,"3");

		if(key==GLFW_KEY_4)
	sprintf(ab,"4");

		if(key==GLFW_KEY_5)
	sprintf(ab,"5");
		if(key==GLFW_KEY_6)
	sprintf(ab,"6");
		if(key==GLFW_KEY_7)
	sprintf(ab,"7");
		if(key==GLFW_KEY_8)
	sprintf(ab,"8");
		if(key==GLFW_KEY_9)
	sprintf(ab,"9");

	}
	if(key==GLFW_KEY_O)
		view=0;
	if(key==GLFW_KEY_B)
		view=1;
	if(key==GLFW_KEY_T)
		view=2;
	if(key==GLFW_KEY_F)
		view=3;
	if(key==GLFW_KEY_H)
		view=4;

	if(key==GLFW_KEY_RIGHT && action==GLFW_PRESS && !disable){
		if(soff==0)

		system("mpg123  -vC sound1.mp3 &");
		moves++;
		stmove++;
		if(l8f==0){

		if(posx1==posx2 && posy2>posy1){
			posx1+=6;
			posx2+=12;
			posy2-=6;

		}

		else if(posx1==posx2 && posy1>posy2){
			posx1+=12;
			posx2+=6;
			posy1-=6;
		}
		else if(posx1>posx2 && posy1==posy2){
			posx1+=6;
			posx2+=12;
			posy2+=6;
		}
		else if(posx2>posx1 && posy1==posy2){
			posx2+=6;
			posx1+=12;
			posy1+=6;
		}
		else if(posx2==posx1 && posy1==posy2){
			posx1+=6;
			posx2+=6;
			//posz2+=6;
		}
	}
	else if(l8f==1){
		posx2+=6;
	}
	else if(l8f==2){
		posx1+=6;
	}

	}

	else if(key==GLFW_KEY_LEFT && action==GLFW_PRESS && !disable){
		moves++;
		stmove++;
		if(soff==0)

		system("mpg123  -vC sound1.mp3 &");

		if(l8f==0){

		if(posx1==posx2 && posy2>posy1){
			posx1-=6;
			posx2-=12;
			posy2-=6;

		}

		else if(posx1==posx2 && posy1>posy2){
			posx1-=12;
			posx2-=6;
			posy1-=6;
		}
		else if(posx1>posx2 && posy1==posy2){
			posx1-=12;
			posx2-=6;
			posy1+=6;
		}
		else if(posx2>posx1 && posy1==posy2){
			posx1-=6;
			posx2-=12;
			posy2+=6;
		}
		else if(posx2==posx1 && posy1==posy2){
			posx1-=6;
			posx2-=6;
			//posz2+=6;
		}
	}
	else if(l8f==1){
		posx2-=6;
	}
	else if(l8f==2)
		posx1-=6;
	}

	else if(key==GLFW_KEY_UP && action==GLFW_PRESS && !disable){
		moves++;
		stmove++;
		if(soff==0)

		system("mpg123  -vC sound1.mp3 &");

		if(l8f==0)
		{
		if(posz1==posz2 && posy2>posy1){
			posz1-=6;
			posz2-=12;
			posy2-=6;

		}

		else if(posz1==posz2 && posy1>posy2){
			posz1-=12;
			posz2-=6;
			posy1-=6;
		}
		else if(posz1>posz2 && posy1==posy2){
			posz1-=12;
			posz2-=6;
			posy1+=6;
		}
		else if(posz2>posz1 && posy1==posy2){
			posz1-=6;
			posz2-=12;
			posy2+=6;
		}
		else if(posz2==posz1 && posy1==posy2){
			posz1-=6;
			posz2-=6;
			//posz2+=6;
		}
			}
			else if(l8f==1){
				posz2-=6;
			}
			else if(l8f==2)
				posz1-=6;

	}
	if(key==GLFW_KEY_DOWN && action==GLFW_PRESS && !disable){
		moves++;
		stmove++;
		if(soff==0)

		system("mpg123  -vC sound1.mp3 &");

		if(l8f==0){
		if(posz1==posz2 && posy2>posy1){
			posz1+=6;
			posz2+=12;
			posy2-=6;

		}

		else if(posz1==posz2 && posy1>posy2){
			posz1+=12;
			posz2+=6;
			posy1-=6;
		}
		else if(posz1>posz2 && posy1==posy2){
			posz1+=6;
			posz2+=12;
			posy2+=6;
		}
		else if(posz2>posz1 && posy1==posy2){
			posz2+=6;
			posz1+=12;
			posy1+=6;
		}
		else if(posz2==posz1 && posy1==posy2){
			posz1+=6;
			posz2+=6;
			//posz2+=6;
		}
	}
	else if(l8f==1)
		posz2+=6;
	else if(l8f==2)
		posz1+=6;

	}
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
			quit(window);
			break;
		default:
			break;
	}
}
int lmouse1=0;
int mouse=0;
int togtext=0;
double lxg;
		double lyg;
/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
	switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			if (action == GLFW_RELEASE)
				triangle_rot_dir *= -1;
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			if (action == GLFW_RELEASE) {
				rectangle_rot_dir *= -1;
			}
			break;
		default:
			break;
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && blo==1) {
		if(GLFW_PRESS == action){
			lmouse1 = 1;
			mouse=1;
		}
		if(action==GLFW_RELEASE){
			lmouse1=0;
			mouse=0;
		}
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && blo==0) {
		if(GLFW_PRESS == action){
			lmouse = 1;
		}
		if(action==GLFW_RELEASE)
			lmouse=0;
	}
	if(lmouse1==1){
		glfwGetCursorPos(window, &lxg, &lyg);

	}
	if(lmouse==1){
		double lx;
		double ly;
		glfwGetCursorPos(window, &lx, &ly);
		if(lx>593 && lx<860 && ly>434 && ly<490){
			enter=1;

		}
		if(lx>593 && lx<860 && ly>512 && ly<575){
			pass=1;
		}
		if(lx>593 && lx<860 && ly>596 && ly<651){
			togtext=1;
		}
		if(lx>264 && lx<441 && ly>593 && ly<652){
			togtext=0;
		}
		
	}
	if(mouse==1){
		double lx;
		double ly;
		glfwGetCursorPos(window, &lx, &ly);
		if(lx>84 && lx<171 && ly>21 && ly<65){
			if(menu==0)
			menu=1;
			else if(menu==1)
				menu=0;
		}
		if(lx>133 && lx<287 && ly>268 && ly<294 && menu==1){
			if(soff==0)
			soff=1;
		else if(soff==1)
			soff=0;
		}
		if(lx>160 && lx<220 && ly>316 && ly<340 && menu==1){
			flag=9;
		}
		//-18.0f+posx1+l3+l6+l7, 3.0f+posy1+spo, -6.0f+posz1+r3+r4+r6+r7+r8+r9)); // glTra
		if(lx>1371 && lx<1404 && ly>625 && ly<656){
			if(soff==0)

		system("mpg123  -vC sound1.mp3 &");
		moves++;
		stmove++;
		if(l8f==0){

		if(posx1==posx2 && posy2>posy1){
			posx1+=6;
			posx2+=12;
			posy2-=6;

		}

		else if(posx1==posx2 && posy1>posy2){
			posx1+=12;
			posx2+=6;
			posy1-=6;
		}
		else if(posx1>posx2 && posy1==posy2){
			posx1+=6;
			posx2+=12;
			posy2+=6;
		}
		else if(posx2>posx1 && posy1==posy2){
			posx2+=6;
			posx1+=12;
			posy1+=6;
		}
		else if(posx2==posx1 && posy1==posy2){
			posx1+=6;
			posx2+=6;
			//posz2+=6;
		}
	}
	else if(l8f==1){
		posx2+=6;
	}
	else if(l8f==2){
		posx1+=6;
	}
		}

		else if(lx>1225 && lx<1254 && ly>625 && ly<656){
			moves++;
		stmove++;
		if(soff==0)

		system("mpg123  -vC sound1.mp3 &");

		if(l8f==0){

		if(posx1==posx2 && posy2>posy1){
			posx1-=6;
			posx2-=12;
			posy2-=6;

		}

		else if(posx1==posx2 && posy1>posy2){
			posx1-=12;
			posx2-=6;
			posy1-=6;
		}
		else if(posx1>posx2 && posy1==posy2){
			posx1-=12;
			posx2-=6;
			posy1+=6;
		}
		else if(posx2>posx1 && posy1==posy2){
			posx1-=6;
			posx2-=12;
			posy2+=6;
		}
		else if(posx2==posx1 && posy1==posy2){
			posx1-=6;
			posx2-=6;
			//posz2+=6;
		}
	}
	else if(l8f==1){
		posx2-=6;
	}
	else if(l8f==2)
		posx1-=6;
	}


else if(lx>1300 && lx<1327 && ly>544 && ly<573 ){
	moves++;
		stmove++;
		if(soff==0)

		system("mpg123  -vC sound1.mp3 &");

		if(l8f==0)
		{
		if(posz1==posz2 && posy2>posy1){
			posz1-=6;
			posz2-=12;
			posy2-=6;

		}

		else if(posz1==posz2 && posy1>posy2){
			posz1-=12;
			posz2-=6;
			posy1-=6;
		}
		else if(posz1>posz2 && posy1==posy2){
			posz1-=12;
			posz2-=6;
			posy1+=6;
		}
		else if(posz2>posz1 && posy1==posy2){
			posz1-=6;
			posz2-=12;
			posy2+=6;
		}
		else if(posz2==posz1 && posy1==posy2){
			posz1-=6;
			posz2-=6;
			//posz2+=6;
		}
			}
			else if(l8f==1){
				posz2-=6;
			}
			else if(l8f==2)
				posz1-=6;
}


else if(lx>1300 && lx<1327 && ly>700 && ly<734){
	moves++;
		stmove++;
		if(soff==0)

		system("mpg123  -vC sound1.mp3 &");

		if(l8f==0){
		if(posz1==posz2 && posy2>posy1){
			posz1+=6;
			posz2+=12;
			posy2-=6;

		}

		else if(posz1==posz2 && posy1>posy2){
			posz1+=12;
			posz2+=6;
			posy1-=6;
		}
		else if(posz1>posz2 && posy1==posy2){
			posz1+=6;
			posz2+=12;
			posy2+=6;
		}
		else if(posz2>posz1 && posy1==posy2){
			posz2+=6;
			posz1+=12;
			posy1+=6;
		}
		else if(posz2==posz1 && posy1==posy2){
			posz1+=6;
			posz2+=6;
			//posz2+=6;
		}
	}
	else if(l8f==1)
		posz2+=6;
	else if(l8f==2)
		posz1+=6;

}


		}

	}

int dis=0;
float zoom=1;

/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
	int fbwidth=width, fbheight=height;
	/* With Retina display on Mac OS X, GLFW's FramebufferSize
	   is different from WindowSize */
	glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 0.9f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
	// Perspective projection for 3D views
	// Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

	// Ortho projection for 2D views
	if(dis==0)
	Matrices.projection = glm::perspective(fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);
	else
	Matrices.projection = glm::ortho(-100.0f,100.0f,-50.0f,50.0f, 0.1f, 500.0f);


}

VAO *triangle,*triangle1,*triangle2,*triangle3, *rectangle,*cuboid[10][15],*cub1,*cub2,*circle,*rectangle1,*rectangle2,*rect[10][15],*level[7],*cuboid1[10][15],*dcub,*dcu,*circle1,*dcub1,*dcub2,*dcub3,*dcub4,*dcub5;

// Creates the triangle object used in this sample code
void createTriangle ()
{
	/* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

	/* Define vertex array as used in glBegin (GL_TRIANGLES) */
	static const GLfloat vertex_buffer_data [] = {
		0, 1,0, // vertex 0
		-1,-1,0, // vertex 1
		1,-1,0, // vertex 2
	};

	static const GLfloat color_buffer_data [] = {
		1,0,0, // color 0
		0,1,0, // color 1
		0,0,1, // color 2
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
	triangle1 = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);

	triangle2 = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);

	triangle3 = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);

}

// Creates the rectangle object used in this sample code
void createRectangle ()
{
	// GL3 accepts only Triangles. Quads are not supported
	static const GLfloat vertex_buffer_data [] = {
		-1.2,-1,0, // vertex 1
		1.2,-1,0, // vertex 2
		1.2, 1,0, // vertex 3

		1.2, 1,0, // vertex 3
		-1.2, 1,0, // vertex 4
		-1.2,-1,0  // vertex 1
	};
	GLfloat display [] = {
		0,0,0, // vertex 1
		12,0,0, // vertex 2
		12, 2,0, // vertex 3

		12, 2,0, // vertex 3
		0, 2,0, // vertex 4
		0,0,0 
	};

	 GLfloat color_buffer_data [] = {
		0,0,0, // color 1
		0,0,0, // color 2
		0,0,0, // color 3

		0,0,0, // color 3
		0,0,0, // color 4
		0,0,0  // color 1
	};
	GLfloat colordisplay [] = {
		1,1,1,
		1,1,1,
		1,1,1,
		1,1,1,
		1,1,1,
		1,1,1
			// color 1
	};
	GLfloat colordisplay1 [] = {
		0,0,1,
		0,0,1,
		0,0,1,
		0,0,1,
		0,0,1,
		0,0,1
			// color 1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
	rectangle1 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, colordisplay, GL_FILL);
	rectangle2 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, colordisplay1, GL_FILL);

	for(int i=0;i<10;i++){
		for(int j=0;j<15;j++)
	rect[i][j] = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
	}
	for(int i=0;i<7;i++)
		level[i]=create3DObject(GL_TRIANGLES, 6, display, colordisplay, GL_FILL);


}
float posy[10][10];
void init();
void level1();
void level2();
void level4();
void level6();
void level7();
void level8();
void level9();




void createCircle()
{
	GLfloat vertex_buffer_data [360*9]={0};
	for(int i=0;i<360;i++)
	{
		vertex_buffer_data[9*i]=0;
		vertex_buffer_data[9*i+1]=0;
		vertex_buffer_data[9*i+2]=0;
		vertex_buffer_data[9*i+3]=2*cos(i*M_PI/180);
		vertex_buffer_data[9*i+4]=2*sin(i*M_PI/180);
		vertex_buffer_data[9*i+5]=0;
		vertex_buffer_data[9*i+6]=2*cos((i+1)*M_PI/180);
		vertex_buffer_data[9*i+7]=2*sin((i+1)*M_PI/180);
		vertex_buffer_data[9*i+8]=0;
	}
	GLfloat color_buffer_data [360*9];
	GLfloat color_buffer_data1 [360*9];

	for (int i = 0; i<360*9 ; i+=3)
	{
		color_buffer_data[i]=1;
		color_buffer_data[i+1]=0;
		color_buffer_data[i+2]=0;
	}
	for (int i = 0; i<360*9 ; i+=3)
	{
		color_buffer_data1[i]=0;
		color_buffer_data1[i+1]=1;
		color_buffer_data1[i+2]=0;
	}
	circle = create3DObject(GL_TRIANGLES,360*3,vertex_buffer_data,color_buffer_data,GL_FILL);
	circle1 = create3DObject(GL_TRIANGLES,360*3,vertex_buffer_data,color_buffer_data1,GL_FILL);

}
void level3();
void createCuboid(){
	GLfloat vertex_buffer_data [] = {
		-2.0f,-2.0f,-2.0f, // triangle 1 : begin
		-2.0f,-2.0f, 2.0f,
		-2.0f, 2.0f, 2.0f, // triangle 1 : end
		2.0f, 2.0f,-2.0f, // triangle 2 : begin
		-2.0f,-2.0f,-2.0f,
		-2.0f, 2.0f,-2.0f, // triangle 2 : end
		2.0f,-2.0f, 2.0f,
		-2.0f,-2.0f,-2.0f,
		2.0f,-2.0f,-2.0f,
		2.0f, 2.0f,-2.0f,
		2.0f,-2.0f,-2.0f,
		-2.0f,-2.0f,-2.0f,
		-2.0f,-2.0f,-2.0f,
		-2.0f, 2.0f, 2.0f,
		-2.0f, 2.0f,-2.0f,
		2.0f,-2.0f, 2.0f,
		-2.0f,-2.0f, 2.0f,
		-2.0f,-2.0f,-2.0f,
		-2.0f, 2.0f, 2.0f,
		-2.0f,-2.0f, 2.0f,
		2.0f,-2.0f, 2.0f,
		2.0f, 2.0f, 2.0f,
		2.0f,-2.0f,-2.0f,
		2.0f, 2.0f,-2.0f,
		2.0f,-2.0f,-2.0f,
		2.0f, 2.0f, 2.0f,
		2.0f,-2.0f, 2.0f,
		2.0f, 2.0f, 2.0f,
		2.0f, 2.0f,-2.0f,
		-2.0f, 2.0f,-2.0f,
		2.0f, 2.0f, 2.0f,
		-2.0f, 2.0f,-2.0f,
		-2.0f, 2.0f, 2.0f,
		2.0f, 2.0f, 2.0f,
		-2.0f, 2.0f, 2.0f,
		2.0f,-2.0f, 2.0f
	};
	GLfloat color_buffer_data1[12*3*3];
	GLfloat color_buffer_data2[12*3*3];
	GLfloat color_buffer_data3[12*3*3];
	GLfloat color_buffer_data4[12*3*3];
	GLfloat color_buffer_data5[12*3*3];
	GLfloat color_buffer_data6[12*3*3];
	GLfloat color_buffer_data7[12*3*3];
	GLfloat color_buffer_data8[12*3*3];



	for (int v = 0; v < 12*3 ; v++){
		if(vertex_buffer_data[3*v+0]==-2 && vertex_buffer_data[3*v+1]==-2 &&vertex_buffer_data[3*v+2]==-2){
		color_buffer_data1[3*v+0] = 0.9;
		color_buffer_data1[3*v+1] = 0.9;
		color_buffer_data1[3*v+2] = 0.9;
		}
		if(vertex_buffer_data[3*v+0]==2 && vertex_buffer_data[3*v+1]==2 && vertex_buffer_data[3*v+2]==2 ){
		color_buffer_data1[3*v+0] = 0;
		color_buffer_data1[3*v+1] = 0;
		color_buffer_data1[3*v+2] = 0;
		}
		else{
		color_buffer_data1[3*v+0] = 0.9;
		color_buffer_data1[3*v+1] = 0.9;
		color_buffer_data1[3*v+2] = 0.9;
		}
	}
	for (int v = 0; v < 12*3 ; v++){
		if(vertex_buffer_data[3*v+0]==-2 && vertex_buffer_data[3*v+1]==-2 &&vertex_buffer_data[3*v+2]==-2){
		color_buffer_data2[3*v+0] = 0.9;
		color_buffer_data2[3*v+1] = 0.9;
		color_buffer_data2[3*v+2] = 0.9;
		}
		if(vertex_buffer_data[3*v+0]==2 && vertex_buffer_data[3*v+1]==2 && vertex_buffer_data[3*v+2]==2){
		color_buffer_data2[3*v+0] = 0;
		color_buffer_data2[3*v+1] = 0;
		color_buffer_data2[3*v+2] = 0;
		}
		else{
		color_buffer_data2[3*v+0] = 0.9;
		color_buffer_data2[3*v+1] = 0.9;
		color_buffer_data2[3*v+2] = 0.9;
		}
	}
	for (int v = 0; v < 12*3 ; v++){

		if(vertex_buffer_data[3*v+0]==-2 && vertex_buffer_data[3*v+1]==-2 &&vertex_buffer_data[3*v+2]==-2){
		color_buffer_data2[3*v+0] = 0.7;
		color_buffer_data2[3*v+1] = 0.3;
		color_buffer_data2[3*v+2] = 0.3;
		}
		if(vertex_buffer_data[3*v+0]==2 && vertex_buffer_data[3*v+1]==2 && vertex_buffer_data[3*v+2]==2){
		color_buffer_data3[3*v+0] = 0.7;
		color_buffer_data3[3*v+1] = 0.3;
		color_buffer_data3[3*v+2] = 0.3;
		}
		else{
		color_buffer_data3[3*v+0] = 1;
		color_buffer_data3[3*v+1] = 0.7;
		color_buffer_data3[3*v+2] = 0;
		}
	}
	for (int v = 0; v < 12*3 ; v++){
		if(vertex_buffer_data[3*v+0]==-2 && vertex_buffer_data[3*v+1]==-2 &&vertex_buffer_data[3*v+2]==-2){
		color_buffer_data4[3*v+0] = 0;
		color_buffer_data4[3*v+1] = 1;
		color_buffer_data4[3*v+2] = 0;
		}
		if(vertex_buffer_data[3*v+0]==2 && vertex_buffer_data[3*v+1]==2 && vertex_buffer_data[3*v+2]==2){
		color_buffer_data4[3*v+0] = 0;
		color_buffer_data4[3*v+1] = 1;
		color_buffer_data4[3*v+2] = 0;
		}
		else{
		color_buffer_data4[3*v+0] = 1;
		color_buffer_data4[3*v+1] = 1;
		color_buffer_data4[3*v+2] = 1;
		}
	}
	for (int v = 0; v < 12*3 ; v++){
		if(vertex_buffer_data[3*v+0]==-2 && vertex_buffer_data[3*v+1]==-2 &&vertex_buffer_data[3*v+2]==-2){
		color_buffer_data5[3*v+0] = 1;
		color_buffer_data5[3*v+1] = 0;
		color_buffer_data5[3*v+2] = 0;
		}
		if(vertex_buffer_data[3*v+0]==2 && vertex_buffer_data[3*v+1]==2 && vertex_buffer_data[3*v+2]==2){
		color_buffer_data5[3*v+0] = 1;
		color_buffer_data5[3*v+1] = 0;
		color_buffer_data5[3*v+2] = 0;
		}
		else{
		color_buffer_data5[3*v+0] = 1;
		color_buffer_data5[3*v+1] = 1;
		color_buffer_data5[3*v+2] = 1;
		}
	}
	for (int v = 0; v < 12*3 ; v++){
		color_buffer_data6[3*v+0] = 0;
		color_buffer_data6[3*v+1] = 0;
		color_buffer_data6[3*v+2] = 0;
	}
	for (int v = 0; v < 12*3 ; v++){

		if(vertex_buffer_data[3*v+0]==-2 && vertex_buffer_data[3*v+1]==-2 &&vertex_buffer_data[3*v+2]==-2){
		color_buffer_data7[3*v+0] = 1;
		color_buffer_data7[3*v+1] = 0.9;
		color_buffer_data7[3*v+2] = 0;
		}
		if(vertex_buffer_data[3*v+0]==2 && vertex_buffer_data[3*v+1]==2 && vertex_buffer_data[3*v+2]==2){
		color_buffer_data7[3*v+0] = 1;
		color_buffer_data7[3*v+1] = 0.6;
		color_buffer_data7[3*v+2] = 0;
		}
		else{
		color_buffer_data7[3*v+0] = 1;
		color_buffer_data7[3*v+1] = 1;
		color_buffer_data7[3*v+2] = 1;
		}
	}
	for (int v = 0; v < 12*3 ; v++){
		if(vertex_buffer_data[3*v+0]==-2 && vertex_buffer_data[3*v+1]==-2 &&vertex_buffer_data[3*v+2]==-2){
		color_buffer_data8[3*v+0] = 1;
		color_buffer_data8[3*v+1] = 0.9;
		color_buffer_data8[3*v+2] = 0;
		}
		if(vertex_buffer_data[3*v+0]==2 && vertex_buffer_data[3*v+1]==2 && vertex_buffer_data[3*v+2]==2){
		color_buffer_data8[3*v+0] = 1;
		color_buffer_data8[3*v+1] = 0.6;
		color_buffer_data8[3*v+2] = 0;
		}
		else{
		color_buffer_data8[3*v+0] = 1;
		color_buffer_data8[3*v+1] = 1;
		color_buffer_data8[3*v+2] = 1;
		}
	}
	for(int i=0;i<10;i++)
		for(int j=0;j<15;j++){
			if((i+j)%2==0)
				cuboid[i][j] = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data1, GL_FILL);
			else
				cuboid[i][j] = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data2, GL_FILL);


			cub1= create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data3, GL_FILL);
			cub2= create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data3, GL_FILL);
			dcu= create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data1, GL_FILL);

			dcub= create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data5, GL_FILL);
			dcub1= create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data4, GL_FILL);
			dcub2= create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data5, GL_FILL);
			dcub3= create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data6, GL_FILL);
			dcub4= create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data7, GL_FILL);
			dcub5= create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data8, GL_FILL);





		}

		for(int i=0;i<10;i++)
		for(int j=0;j<15;j++){
			if((i+j)%2==0)
				cuboid1[i][j] = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data3,GL_LINE);
			else
				cuboid1[i][j] = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data3,GL_LINE);


		}

	init();
	level1();
}

float camera_rotation_angle = 45;
float rectangle_rotation = 0;

void mousezoom(GLFWwindow* window, double xoffset, double yoffset)
{
	if (yoffset==-1) { 
		zoom/=1.1; 
	}
	else if(yoffset==1){
		zoom*=1.1; 
	}
	if (zoom<=1) {
		zoom = 1;
	}
	if (zoom>=2) {
		zoom=2;
	}

	//reshapeWindow(window,1500,800);
	/*if(xpos-100.0f/zoom<-100)
		xpos=-8+8.0f/zoom;
	else if(xpos+100.0f/zoom>100)
		xpos=100-100.0f/zoom;
	if(ypos-50.0f/zoom<-50)
		ypos=-50+50.0f/zoom;
	else if(ypos+50.0f/zoom>50)
		ypos=50-50.0f/zoom;
*/
	//Matrices.projection = glm::perspective(fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

	Matrices.projection = glm::ortho((float)(-100.0f/zoom), (float)(100.0f/zoom), (float)(-50.0f/zoom), (float)(50.0f/zoom), 0.1f, 500.0f);
}
/* Render the scene with openGL */
/* Edit this function according to your assignment */
int a[10][15];
void level1(){
	for(int i=0;i<10;i++)
		for(int j=0;j<10;j++)
			a[i][j]=1;
	for(int i=0;i<2;i++)
		for(int j=0;j<10;j++)
			a[i][j]=0;
	for(int i=9;i>=8;i--)
		for(int  j=0;j<10;j++)
			a[i][j]=0;
	a[6][7]=4;
	a[6][0]=0;
	a[6][2]=0;
	a[6][1]=0;
	a[6][3]=0;
	a[6][4]=0;
	int i=7;
	for(int j=0;j<6;j++)
		a[i][j]=0;
	a[i][9]=0;
	a[5][0]=0;
	i=2;
	for(int j=3;j<10;j++)
		a[i][j]=0;
	i=3;
	for(int j=6;j<10;j++)
		a[i][j]=0;
	a[4][9]=0;
	for(int i=0;i<10;i++)
		for(int j=10;j<15;j++)
			a[i][j]=0;
}
int l2tog=0,l2f=0;
int l2togl=0,l2r=0;


void level2(){
	for(int i=0;i<10;i++)
		for(int j=0;j<15;j++)
			a[i][j]=1;
	for(int i=0;i<3;i++)
		for(int j=0;j<15;j++)
			a[i][j]=0;
	for(int i=8;i<10;i++)
		for(int j=0;j<15;j++)
			a[i][j]=0;
	for(int i=3;i<8;i++){
		a[i][4]=0;
		a[i][5]=0;
	}
	int i=2;
	for(int j=6;j<15;j++)
		a[i][j]=1;
	a[2][10]=0;
	a[2][11]=0;
	for(int i=3;i<8;i++)
		for(int j=10;j<12;j++)
			a[i][j]=0;
	a[7][12]=0;
	a[7][13]=0;
	a[7][14]=0;
	a[3][13]=4;
	a[4][2]=2;
	l2tog=0;
	l2f=0;
	a[3][8]=3;
	l2r=0;
	l2togl=0;

}
void level3(){
	for(int i=0;i<10;i++){
		for(int j=0;j<15;j++)
			a[i][j]=1;
	}
	for(int i=0;i<4;i++)
		for(int j=0;j<15;j++)
			a[i][j]=0;
	for(int i=8;i<10;i++)
		for(int j=0;j<15;j++)
			a[i][j]=0;
		for(int i=6;i<8;i++)
		for(int j=4;j<11;j++)
			a[i][j]=0;
		a[7][11]=0;
		int i=3;
		for(int j=6;j<15;j++)
			a[i][j]=1;
		a[4][4]=0;
		a[4][5]=0;
		a[4][9]=0;
		a[4][10]=0;
		a[5][9]=0;
		a[5][10]=0;
		a[3][13]=0;
		a[3][14]=0;
		a[4][13]=0;
		a[4][14]=0;
		a[6][13]=4;


}
void level4(){
	for(int i=0;i<10;i++){
		for(int j=0;j<15;j++)
			a[i][j]=1;
	}
	int i=0;
	for(int j=0;j<15;j++)
		a[i][j]=0;
	for(int i=1;i<3;i++){
		for(int j=0;j<3;j++)
			a[i][j]=0;
		for(int j=13;j<15;j++)
			a[i][j]=0;
	}
	for(int i=3;i<6;i++)
		for(int j=4;j<9;j++)
			a[i][j]=0;
		a[4][3]=0;
		a[4][9]=0;
		a[5][3]=0;
		a[5][9]=0;
		a[6][3]=0;
		a[6][4]=0;
		a[7][3]=0;
		a[7][4]=0;

		for(int i=8;i<10;i++)
		for(int j=0;j<5;j++)
			a[i][j]=0;
		for(int i=8;i<10;i++)
		for(int j=8;j<10;j++)
			a[i][j]=0;
		
		a[8][6]=4;
		a[8][13]=5;
		a[6][8]=1;
		a[6][9]=6;
		a[7][8]=1;
		a[7][9]=6;
		for(int i=1;i<3;i++)
			for(int j=3;j<10;j++)
				a[i][j]=6;
			for(int i=6;i<10;i++)
			for(int j=9;j<15;j++)
				a[i][j]=6;
		a[8][13]=5;


}
void level6(){
	for(int i=0;i<10;i++){
		for(int j=0;j<15;j++)
			a[i][j]=1;
	}
	int i;
	//int i=0;
	//for(int j=0;j<15;j++)
	//	a[i][j]=0;
	for(int i=0;i<3;i++){
		for(int j=0;j<5;j++)
			a[i][j]=0;
	}
	a[1][6]=0;
	a[1][7]=0;
	a[2][6]=0;
	a[2][7]=0;
	a[3][6]=0;
	for(int i=3;i<6;i++){
		for(int j=7;j<11;j++)
			a[i][j]=0;
	}
	a[6][8]=0;
	a[6][7]=0;
	a[5][11]=0;
	for(i=6;i<9;i++)
	for(int j=11;j<15;j++)
		a[i][j]=0;
	for(i=0;i<2;i++)
	for(int j=11;j<15;j++)
		a[i][j]=0;
	a[2][13]=0,a[2][14]=0;
	for(i=4;i<9;i++)
	for(int j=0;j<4;j++)
		a[i][j]=0;
	for(i=6;i<9;i++)
	for(int j=4;j<6;j++)
		a[i][j]=0;
	a[9][6]=0;
	a[9][10]=0;
	a[4][13]=4;
	for(int j=0;j<6;j++)
		a[9][j]=0;
	for(int j=10;j<15;j++)
		a[9][j]=0;
}
void level7(){
	for(int i=0;i<10;i++){
		for(int j=0;j<15;j++)
			a[i][j]=1;
	}
	int i=0;
	for(int j=0;j<15;j++)
		a[i][j]=0;
	i=9;
	for(int j=0;j<15;j++)
		a[i][j]=0;
	for(int i=1;i<3;i++)
		for(int j=0;j<8;j++)
		a[i][j]=0;
	for(int i=1;i<3;i++)
		for(int j=12;j<15;j++)
		a[i][j]=0;
	i=3;
		for(int j=3;j<8;j++)
		a[i][j]=0;
	a[3][9]=0;
	a[3][10]=0;
	a[4][9]=0;
	for(int i=4;i<7;i++)
		for(int j=10;j<12;j++)
		a[i][j]=0;
	for(int i=5;i<8;i++)
		for(int j=3;j<7;j++)
		a[i][j]=0;
	for(int i=7;i<9;i++)
		for(int j=9;j<15;j++)
		a[i][j]=0;
	a[4][13]=4;
	a[5][9]=2;

}
void level8(){
	for(int i=0;i<10;i++){
		for(int j=0;j<15;j++)
			a[i][j]=0;
	}
	for(int i=4;i<7;i++){
		for(int j=0;j<6;j++)
			a[i][j]=1;
	}
	for(int i=1;i<10;i++){
		for(int j=9;j<12;j++)
			a[i][j]=1;
	}
	for(int i=4;i<7;i++){
		for(int j=12;j<15;j++)
			a[i][j]=1;
	}
	a[5][13]=4;
	a[5][4]=7;
l8f=0;
}
int sound=0;
void level9(){
	for(int i=0;i<10;i++){
		for(int j=0;j<15;j++)
			a[i][j]=0;
	}
	for(int i=3;i<6;i++){
		for(int j=0;j<4;j++)
			a[i][j]=1;
	}
	for(int i=3;i<6;i++){
		for(int j=11;j<15;j++)
			a[i][j]=1;
	}
	for(int i=5;i<6;i++){
		for(int j=4;j<15;j++)
			a[i][j]=1;
	}
	a[3][7]=1;
	a[4][7]=1;
	a[6][6]=1,a[7][6]=1,a[6][8]=1,a[7][8]=1,a[6][7]=4;
	a[7][7]=1;
	a[4][13]=7;

}
float spo;
int attempts=1;
void init(){
	for(int i=0;i<15;i++)
		for(int j=0;j<10;j++)
			posy[i][j]=-60;

sound=0;
	//posy[0][0]=0;
	spo=60;
	posx1=0;
	posx2=0;
	posz1=0;
	posz2=0;
	posy1=0;
	posy2=6;
	disable=0;

dis=1;
stmove=0;
if(flag==1){
			level1();
		}
		if(flag==2)
			level2();
		if(flag==3)
			level3();
		if(flag==4)
			level4();
		if(flag==5)
			level6();
		if(flag==6)
			level7();
		if(flag==7)
			level8();
		if(flag==8)
			level9();


}

double current_time,utime=glfwGetTime();
int flagdown=0;

glm::vec3 getRGBfromHue (int hue)
{
  float intp;
  float fracp = modff(hue/60.0, &intp);
  float x = 1.0 - abs((float)((int)intp%2)+fracp-1.0);
  float y=1;
  if(hue==100){
    return glm::vec3(1,1,1);

  }
else{
  if (hue < 60)
    return glm::vec3(1,x,0);
  else if (hue < 120)
    return glm::vec3(x,1,0);
  else if (hue < 180)
    return glm::vec3(0,1,x);
  else if (hue < 240)
    return glm::vec3(0,x,1);
  else if (hue < 300)
    return glm::vec3(x,0,1);
  else
    return glm::vec3(1,0,x);

}
}


void display_string(GLFWwindow *window){
	double lx;
	double ly;
	glfwGetCursorPos(window, &lx, &ly);
	cout<<lx<<" "<<ly<<'\n';

}
float camera_rotation_angle1=0;
double utime1;
double utime4=glfwGetTime();
int score=0;
int heli=0;
void drag (GLFWwindow* window){
	double lx1;
	double ly1;
	glfwGetCursorPos(window, &lx1, &ly1);
	if(heli==1 && lmouse1==1){
	camera_rotation_angle-=(lx1-lxg)/800;
	camera_rotation_angle1-=(lx1-lxg)/800;
}
	}
void draw ()
{

	// clear the color and depth in the frame buffer
	
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// use the loaded shader program
	// Don't change unless you know what you are doing
	glUseProgram (programID);

	// Eye - Location of camera. Don't change unless you are sure!!
	glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
	// Target - Where is the camera looking at.  Don't change unless you are sure!!
	glm::vec3 target (0, 0, 0);
	// Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
	glm::vec3 up (0, 1, 0);

	//if(zoom<0)
	//Matrices.view = glm::lookAt(glm::vec3(-30,70,60), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

	//Matrices.projection = glm::perspective(0.9f+zoom, (GLfloat) 1500 / (GLfloat) 800, 0.1f, 500.0f);
	if(attempts==4){
		flag=9;
		utime1=glfwGetTime();

	}

	if(dis==0)
	Matrices.view = glm::lookAt(glm::vec3(-30,70,60), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane
	if(dis==1 || blo==0)
Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); 
	
	if(view==0){
	Matrices.view = glm::lookAt(glm::vec3(-30,70,60), glm::vec3(0,0,0), glm::vec3(0,1,0));
	Matrices.projection = glm::ortho((float)(-100.0f/zoom), (float)(100.0f/zoom), (float)(-50.0f/zoom), (float)(50.0f/zoom), 0.1f, 500.0f);
	heli=0;

}
	if(view==1){
		heli=1;
	//Matrices.projection = glm::ortho(-100.0f,100.0f,-50.0f,50.0f,0.1f, 500.0f);
	//glm::mat4 translateTriangle1 = glm::translate (glm::vec3(-18.0f+posx1+l3+l6+l7, 3.0f+posy1+spo, -6.0f+posz1+r3+r4+r6+r7+r8+r9)); // glTranslatef

	Matrices.projection = glm::perspective(0.9f+0.6f, (GLfloat) 1500 / (GLfloat) 800, 0.1f, 500.0f);

	Matrices.view = glm::lookAt(glm::vec3(-8+posx1+l3+l6+l7,15,-4+posz1+r3+r4+r6+r7+r8+r9), glm::vec3(30,0,10), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane
	}
	if(view==2){
		heli=1;
	//Matrices.projection = glm::perspective(0.9f, (GLfloat) 1500 / (GLfloat) 800, 0.1f, 500.0f);
	Matrices.projection = glm::ortho(-100.0f,100.0f,-50.0f,50.0f,0.1f, 500.0f);
	
	Matrices.view = glm::lookAt(glm::vec3(0,90,0), glm::vec3(0,0,0), glm::vec3(0,0,-1)); 
	}
	if(view==3){
		heli=1;
	Matrices.projection = glm::perspective(0.9f+0.3f, (GLfloat) 1500 / (GLfloat) 800, 0.1f, 500.0f);
	Matrices.view = glm::lookAt(glm::vec3(-33+posx1+l3+l6+l7,24,-8+posz1+r3+r4+r6+r7+r8+r9), glm::vec3(30,0,10), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

	} 
    if(view==4){
    	heli=1;
	Matrices.projection = glm::ortho(-100.0f/zoom,100.0f/zoom,-50.0f/zoom,50.0f/zoom,0.1f, 500.0f);

	Matrices.view = glm::lookAt(glm::vec3(-30*cos(camera_rotation_angle*M_PI/180),70,60*sin(camera_rotation_angle*M_PI/180)), glm::vec3(0,0,0), glm::vec3(0,1,0));
	} 
	// Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
	//  Don't change unless you are sure!!
	glm::mat4 VP = Matrices.projection * Matrices.view;

	// Send our transformation to the currently bound shader, in the "MVP" uniform
	// For each model you render, since the MVP will be different (at least the M part)
	//  Don't change unless you are sure!!
	glm::mat4 MVP;	

	// Compute Camera matrix (view)
	// Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
	//  Don't change unless you are sure!!
	if(flag==9){
		
		for(int i=0;i<10;i++)
			for(int j=0;j<15;j++)
				a[i][j]=0;
moves=0;
score=0;
float fontScaleValue = 36;
int fontScale=150;
	glm::vec3 fontColor1= getRGBfromHue(fontScale);
	glUseProgram(fontProgramID);


	char level_strl[30],level_strl1[30];
	char level_strl2[30],level_strl3[30];
	if(attempts<=3)
	sprintf(level_strl,"YOU WIN");
	if(attempts>3){
Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); 
	//attempts=5;
	sprintf(level_strl,"YOU LOOSE");
	attempts=5;
}


glUseProgram(fontProgramID);

	// Transform the text
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText = glm::translate(glm::vec3(-40,4,0));
	glm::mat4 scaleText = glm::scale(glm::vec3(fontScaleValue,fontScaleValue,fontScaleValue));
	Matrices.model *= (translateText * scaleText);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor1[0]);
	GL3Font.font->Render(level_strl);
	dis=1;
	ent=0;
	enter=0;
	pass=0;
	l3=0,r3=0,l6=0,r6=0,l7=0,r7=0,r8=0,r9=0;
	double ctime=glfwGetTime();
	if(ctime-utime1>3){
	//	re=1;
		flag=1;
		blo=0;
		init();

	}
		

	}

	if(blo==0){
		double ctime4=glfwGetTime();
		float fontScaleValue = 36;
int fontScale=150;
	glm::vec3 fontColor1= getRGBfromHue(fontScale);
	glUseProgram(programID);


	char level_strl[30],level_strl1[30];
	char level_strl2[30],level_strl3[30],level_strl4[100],level_strl5[30];
	sprintf(level_strl,"BLOXORZ");
	sprintf(level_strl1,"START NEW GAME");
	sprintf(level_strl2,"LOAD STAGE");
	sprintf(level_strl3,"CREDITS");
	sprintf(level_strl4,"ALL GRAPHICS,AUDIO,ACTIONSCRIPT,PUZZLES IN BLOXORZ CREATED BY VISHAL REDDY,IIIT-H ON 14th FEBRUARY");
	//sprintf(level_strl4,"ALL  FEBRUARY");
	sprintf(level_strl5,"BACK");



	glUseProgram(fontProgramID);


Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); 
	glUseProgram(programID);
	Matrices.model = glm::mat4(1.0f);
					
glm::mat4 translateTriangle2 = glm::translate (glm::vec3(-5,-8,0)); // glTranslatef
				glm::mat4 rotateTriangle2 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1));
				glm::mat4 scaleTriangle2 = glm::scale (glm::vec3(100.0f, 100.0f, 1.0f)); // glTranslatef
				// rotate about vector (1,0,0)
				glm::mat4 triangleTransform2 = translateTriangle2 * rotateTriangle2*scaleTriangle2;
				Matrices.model *= triangleTransform2; 
				MVP = Matrices.projection * Matrices.view * Matrices.model; // MVP = p * V * M

				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

				draw3DObject(rectangle);
// Fixed camera for 2D (ortho) in XY plane
Matrices.model = glm::mat4(1.0f);
					
glm::mat4 translateTriangle1 = glm::translate (glm::vec3(-3,-8,0)); // glTranslatef
				glm::mat4 rotateTriangle1 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1));
				glm::mat4 scaleTriangle1 = glm::scale (glm::vec3(15.0f, 4.0f, 1.0f)); // glTranslatef
				// rotate about vector (1,0,0)
				glm::mat4 triangleTransform1 = translateTriangle1 * rotateTriangle1*scaleTriangle1;
				Matrices.model *= triangleTransform1; 
				MVP = Matrices.projection * Matrices.view * Matrices.model; // MVP = p * V * M

				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
				if(!togtext)

				draw3DObject(rectangle1);


				Matrices.model = glm::mat4(1.0f);
					
glm::mat4 translateTriangle3 = glm::translate (glm::vec3(-3,-18,0)); // glTranslatef
				glm::mat4 rotateTriangle3 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1));
				glm::mat4 scaleTriangle3 = glm::scale (glm::vec3(15.0f, 4.0f, 1.0f)); // glTranslatef
				// rotate about vector (1,0,0)
				glm::mat4 triangleTransform3 = translateTriangle3 * rotateTriangle3*scaleTriangle3;
				Matrices.model *= triangleTransform3; 
				MVP = Matrices.projection * Matrices.view * Matrices.model; // MVP = p * V * M

				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
				if(!togtext)
				draw3DObject(rectangle1);

				Matrices.model = glm::mat4(1.0f);
					
glm::mat4 translateTriangle4 = glm::translate (glm::vec3(-3,-28,0)); // glTranslatef
				glm::mat4 rotateTriangle4 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1));
				glm::mat4 scaleTriangle4 = glm::scale (glm::vec3(15.0f, 4.0f, 1.0f)); // glTranslatef
				// rotate about vector (1,0,0)
				glm::mat4 triangleTransform4 = translateTriangle4 * rotateTriangle4*scaleTriangle4;
				Matrices.model *= triangleTransform4; 
				MVP = Matrices.projection * Matrices.view * Matrices.model; // MVP = p * V * M

				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

				if(!togtext)
				draw3DObject(rectangle1);

			Matrices.model = glm::mat4(1.0f);
					
glm::mat4 translateTriangle5 = glm::translate (glm::vec3(-53,-28,0)); // glTranslatef
				glm::mat4 rotateTriangle5 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1));
				glm::mat4 scaleTriangle5 = glm::scale (glm::vec3(10.0f, 4.0f, 1.0f)); // glTranslatef
				// rotate about vector (1,0,0)
				glm::mat4 triangleTransform5 = translateTriangle5 * rotateTriangle5*scaleTriangle5;
				Matrices.model *= triangleTransform5; 
				MVP = Matrices.projection * Matrices.view * Matrices.model; // MVP = p * V * M

				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

				if(togtext)
				draw3DObject(rectangle1);
	glUseProgram(fontProgramID);

	// Transform the text
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText = glm::translate(glm::vec3(-40,4,0));
	glm::mat4 scaleText = glm::scale(glm::vec3(fontScaleValue,fontScaleValue,fontScaleValue));
	Matrices.model *= (translateText * scaleText);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor1[0]);
	GL3Font.font->Render(level_strl);
	int fontScale1=5;
	fontScaleValue=8;
	glm::vec3 fontColor= getRGBfromHue(fontScale1);

	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText1 = glm::translate(glm::vec3(-20,-10,0));
	glm::mat4 scaleText1 = glm::scale(glm::vec3(fontScaleValue,fontScaleValue,fontScaleValue));
	Matrices.model *= (translateText1 * scaleText1);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);
	if(!togtext)
	GL3Font.font->Render(level_strl1);

	glm::vec3 fontColor2= getRGBfromHue(fontScale1);

	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText2 = glm::translate(glm::vec3(-15,-20,0));
	glm::mat4 scaleText2 = glm::scale(glm::vec3(fontScaleValue,fontScaleValue,fontScaleValue));
	Matrices.model *= (translateText2 * scaleText2);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor2[0]);
	if(!togtext)
	GL3Font.font->Render(level_strl2);


	

	glm::vec3 fontColor3= getRGBfromHue(fontScale1);

	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText3 = glm::translate(glm::vec3(-15,-30,0));
	glm::mat4 scaleText3 = glm::scale(glm::vec3(fontScaleValue,fontScaleValue,fontScaleValue));
	Matrices.model *= (translateText3 * scaleText3);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor3[0]);
	if(!togtext)
	GL3Font.font->Render(level_strl3);
fontScale1=100;
	fontScaleValue=6;
glm::vec3 fontColor4= getRGBfromHue(fontScale1);

	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText4 = glm::translate(glm::vec3(-85,-15,0));
	glm::mat4 scaleText4 = glm::scale(glm::vec3(fontScaleValue,fontScaleValue,fontScaleValue));
	Matrices.model *= (translateText4 * scaleText4);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor4[0]);
	if(togtext)
	GL3Font.font->Render(level_strl4);


fontScale1=0;
	fontScaleValue=6;
glm::vec3 fontColor5= getRGBfromHue(fontScale1);

	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText5 = glm::translate(glm::vec3(-58,-29.5,0));
	glm::mat4 scaleText5 = glm::scale(glm::vec3(fontScaleValue,fontScaleValue,fontScaleValue));
	Matrices.model *= (translateText5 * scaleText5);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor5[0]);
	if(togtext)
	GL3Font.font->Render(level_strl5);

	//double ctime=glfwGetTime();
	if(enter==1){
		blo=1;
		
		utime=glfwGetTime();
		utime1=glfwGetTime();

	}
	if(ent==1){
			//cout<<"yes";
		if(ab[0]=='1'){
				flag=1;
				level1();
				blo=1;
		utime1=glfwGetTime();
		utime=glfwGetTime();

			}
			if(ab[0]=='2'){
				flag=2;
				level2();
				blo=1;
		utime1=glfwGetTime();
		utime=glfwGetTime();

			}
			if(ab[0]=='3'){
				flag=3;
				level3();
				blo=1;
		utime1=glfwGetTime();
		utime=glfwGetTime();
		l3=0;
			r3=18;

			}
			if(ab[0]=='4'){
				flag=4;
				level4();
				blo=1;
		utime1=glfwGetTime();
		utime=glfwGetTime();
		l3=0;
			r3=18;


			}
			if(ab[0]=='5'){
				flag=5;
				level6();
				blo=1;
		utime1=glfwGetTime();
		utime=glfwGetTime();
		l3=-6;
			r3=0;


			}
			if(ab[0]=='6'){
				flag=6;
				level7();
				blo=1;
		utime1=glfwGetTime();
		utime=glfwGetTime();
		l3=0;
			r3=18;
			l6=-6;
			r6=-18;

			}
			if(ab[0]=='7'){
				flag=7;
				level8();
				blo=1;
		utime1=glfwGetTime();
		utime=glfwGetTime();
		l3=0;
			r3=18;
			l6=-6;
			r6=-18;
			l7=6;
			r7=6;

			}
			if(ab[0]=='8'){
				flag=8;
				level9();
				blo=1;
		utime1=glfwGetTime();
		utime=glfwGetTime();
		l3=0;
			r3=18;
l6=0;
			r6=-12;

			}

		}


	}

		
	if(pass==1 && blo==0){
	glUseProgram(programID);

		Matrices.model = glm::mat4(1.0f);
					
glm::mat4 translateTriangle1 = glm::translate (glm::vec3(33,-22,0)); // glTranslatef
				glm::mat4 rotateTriangle1 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1));
				glm::mat4 scaleTriangle1 = glm::scale (glm::vec3(10.0f, 0.5f, 1.0f)); // glTranslatef
				// rotate about vector (1,0,0)
				glm::mat4 triangleTransform1 = translateTriangle1 * rotateTriangle1*scaleTriangle1;
				Matrices.model *= triangleTransform1; 
				MVP = Matrices.projection * Matrices.view * Matrices.model; // MVP = p * V * M

				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

				draw3DObject(rectangle1);

				float fontScaleValue = 12 ;
int fontScale=280;
	glm::vec3 fontColor= getRGBfromHue(fontScale);
	glUseProgram(fontProgramID);

Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

	// Transform the text
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText = glm::translate(glm::vec3(30,-20,0));
	glm::mat4 scaleText = glm::scale(glm::vec3(fontScaleValue,fontScaleValue,fontScaleValue));
	Matrices.model *= (translateText * scaleText);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);
	GL3Font.font->Render(ab);
				
				

	}
	if(dis==0 && blo==1){
		int ti=glfwGetTime();
		int ti1,ti2,ti3;
		ti-=utime1;
		ti1=ti/3600;
		ti2=ti/60;
		ti3=(ti-(ti2*60));
float fontScaleValue = 12 ;
int fontScale=280;
	glm::vec3 fontColor= getRGBfromHue(fontScale);
	glUseProgram(fontProgramID);


	char level_strl[30];
	sprintf(level_strl,"TIME: %d:%d:%d",ti1,ti2,ti3);




Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

	// Transform the text
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText = glm::translate(glm::vec3(48,42,0));
	glm::mat4 scaleText = glm::scale(glm::vec3(fontScaleValue,fontScaleValue,fontScaleValue));
	Matrices.model *= (translateText * scaleText);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);
	GL3Font.font->Render(level_strl);




	 fontScaleValue = 12 ;
 fontScale=280;
	glm::vec3 fontColor3= getRGBfromHue(fontScale);
	glUseProgram(fontProgramID);


	char level_strl3[30];
	sprintf(level_strl3,"SCORE: %d",score);




Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

	// Transform the text
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText4 = glm::translate(glm::vec3(-68,42,0));
	glm::mat4 scaleText4 = glm::scale(glm::vec3(fontScaleValue,fontScaleValue,fontScaleValue));
	Matrices.model *= (translateText4 * scaleText4);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor3[0]);
	GL3Font.font->Render(level_strl3);

glUseProgram(programID);
	Matrices.model = glm::mat4(1.0f);
					
glm::mat4 translateTriangle1 = glm::translate (glm::vec3(-83,44.5,0)); // glTranslatef
				glm::mat4 rotateTriangle1 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1));
				glm::mat4 scaleTriangle1 = glm::scale (glm::vec3(5.0f, 2.8f, 1.0f)); // glTranslatef
				// rotate about vector (1,0,0)
				glm::mat4 triangleTransform1 = translateTriangle1 * rotateTriangle1*scaleTriangle1;
				Matrices.model *= triangleTransform1; 
				MVP = Matrices.projection * Matrices.view * Matrices.model; // MVP = p * V * M

				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

				draw3DObject(rectangle2);


	fontScaleValue = 8 ;
 fontScale=100;
	glm::vec3 fontColor4= getRGBfromHue(fontScale);
	glUseProgram(fontProgramID);


	char level_strl4[30];
	sprintf(level_strl4,"MENU");




Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

	// Transform the text
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText5 = glm::translate(glm::vec3(-88,43,0));
	glm::mat4 scaleText5 = glm::scale(glm::vec3(fontScaleValue,fontScaleValue,fontScaleValue));
	Matrices.model *= (translateText5 * scaleText5);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor4[0]);
	GL3Font.font->Render(level_strl4);
	if(menu==1){
		//char level_strl5[30];
		glUseProgram(programID);
	Matrices.model = glm::mat4(1.0f);
					
glm::mat4 translateTriangle1 = glm::translate (glm::vec3(-73,6,0)); // glTranslatef
				glm::mat4 rotateTriangle1 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1));
				glm::mat4 scaleTriangle1 = glm::scale (glm::vec3(15.0f, 11.8f, 1.0f)); // glTranslatef
				// rotate about vector (1,0,0)
				glm::mat4 triangleTransform1 = translateTriangle1 * rotateTriangle1*scaleTriangle1;
				Matrices.model *= triangleTransform1; 
				MVP = Matrices.projection * Matrices.view * Matrices.model; // MVP = p * V * M

				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

				draw3DObject(rectangle);
fontScaleValue = 6 ;
 fontScale=100;
	glm::vec3 fontColor4= getRGBfromHue(fontScale);
	glUseProgram(fontProgramID);


	char level_strl4[30];

	sprintf(level_strl4,"TOGGLE SOUND");




Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

	// Transform the text
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText5 = glm::translate(glm::vec3(-83,14,0));
	glm::mat4 scaleText5 = glm::scale(glm::vec3(fontScaleValue,fontScaleValue,fontScaleValue));
	Matrices.model *= (translateText5 * scaleText5);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor4[0]);
	GL3Font.font->Render(level_strl4);


fontScaleValue = 6 ;
 fontScale=100;
	glm::vec3 fontColor5= getRGBfromHue(fontScale);
	glUseProgram(fontProgramID);


	char level_strl5[30];
	sprintf(level_strl5,"QUIT");




Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

	// Transform the text
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText6 = glm::translate(glm::vec3(-78,8,0));
	glm::mat4 scaleText6 = glm::scale(glm::vec3(fontScaleValue,fontScaleValue,fontScaleValue));
	Matrices.model *= (translateText6 * scaleText6);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor5[0]);
	GL3Font.font->Render(level_strl5);


	fontScaleValue = 6 ;
 fontScale=100;
	glm::vec3 fontColor6= getRGBfromHue(fontScale);
	glUseProgram(fontProgramID);


	char level_strl6[30];
	sprintf(level_strl6,"LEVEL:%d",flag);




Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

	// Transform the text
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText7 = glm::translate(glm::vec3(-81,2,0));
	glm::mat4 scaleText7 = glm::scale(glm::vec3(fontScaleValue,fontScaleValue,fontScaleValue));
	Matrices.model *= (translateText7 * scaleText7);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor6[0]);
	GL3Font.font->Render(level_strl6);


	fontScaleValue = 6 ;
 fontScale=100;
	glm::vec3 fontColor7= getRGBfromHue(fontScale);
	glUseProgram(fontProgramID);


	char level_strl7[30];
	sprintf(level_strl7,"ATTEMPTS:%d",attempts);




Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

	// Transform the text
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText8 = glm::translate(glm::vec3(-81,-4,0));
	glm::mat4 scaleText8 = glm::scale(glm::vec3(fontScaleValue,fontScaleValue,fontScaleValue));
	Matrices.model *= (translateText8 * scaleText8);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor7[0]);
	GL3Font.font->Render(level_strl7);


	}
	

	}
	


	if(dis==1 && blo==1){
double ctime=glfwGetTime();
		if(ctime - utime > 2){
			utime=glfwGetTime();
			dis=0;

		}

	// Send our transformation to the currently bound shader, in the "MVP" uniform
	// For each model you render, since the MVP will be different (at least the M part)
	//  Don't change unless you are sure!!
		int fontScale=1;
float fontScaleValue = 36 ;
	glm::vec3 fontColor = getRGBfromHue(fontScale);
	glUseProgram(fontProgramID);


	char level_str[30];
	sprintf(level_str,"LEVEL: %d",flag);
	

	Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

	// Transform the text
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText = glm::translate(glm::vec3(-40,5,0));
	glm::mat4 scaleText = glm::scale(glm::vec3(fontScaleValue,fontScaleValue,fontScaleValue));
	Matrices.model *= (translateText * scaleText);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);
	if(flag<9)
	GL3Font.font->Render(level_str);
	}
	else if(dis==0 && blo==1){
	glUseProgram (programID);



	// Load identity to model matrix
	for(int i=0;i<10;i++){
		for(int j=0;j<15;j++)
		{
			if(a[i][j]==1 ||a[i][j]==2 || a[i][j]==3 ||a[i][j]==6){ 


				Matrices.model = glm::mat4(1.0f);
				posy[i][j]+=((i+j)/1.5);
				if(posy[i][j]>0)
					posy[i][j]=0;
				if(flag==4 && a[i][j]==6){



					if((i+j)%2==0){
						Matrices.model = glm::mat4(1.0f);
					
glm::mat4 translateTriangle1 = glm::translate (glm::vec3(0.0f+(j+1)*6-30, 0.0+posy[i][j], 0.0f+(i+1)*6-30)); // glTranslatef
				glm::mat4 rotateTriangle1 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1));
				glm::mat4 scaleTriangle1 = glm::scale (glm::vec3(1.5f, 0.4f, 1.5f)); // glTranslatef
				// rotate about vector (1,0,0)
				glm::mat4 triangleTransform1 = translateTriangle1 * rotateTriangle1*scaleTriangle1;
				Matrices.model *= triangleTransform1; 
				MVP = VP * Matrices.model; // MVP = p * V * M

				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

				draw3DObject(dcub4);
			}


			else
			{
				Matrices.model = glm::mat4(1.0f);
					
glm::mat4 translateTriangle1 = glm::translate (glm::vec3(0.0f+(j+1)*6-30, 0.0+posy[i][j], 0.0f+(i+1)*6-30)); // glTranslatef
				glm::mat4 rotateTriangle1 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1));
				glm::mat4 scaleTriangle1 = glm::scale (glm::vec3(1.5f, 0.4f, 1.5f)); // glTranslatef
				// rotate about vector (1,0,0)
				glm::mat4 triangleTransform1 = translateTriangle1 * rotateTriangle1*scaleTriangle1;
				Matrices.model *= triangleTransform1; 
				MVP = VP * Matrices.model; // MVP = p * V * M

				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

				draw3DObject(dcub5);
			}
				Matrices.model = glm::mat4(1.0f);

glm::mat4 translateTriangle1 = glm::translate (glm::vec3(0.0f+(13+1)*6-30, 0.0f+posy[i][j], 0.0f+(8+1)*6-30)); // glTranslatef
				glm::mat4 rotateTriangle1 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1));
				glm::mat4 scaleTriangle1 = glm::scale (glm::vec3(1.5f, 0.4f, 1.5f)); // glTranslate
				// rotate about vector (1,0,0)
				glm::mat4 triangleTransform1 = translateTriangle1 * rotateTriangle1*scaleTriangle1;
				Matrices.model *= triangleTransform1; 
				MVP = VP * Matrices.model; // MVP = p * V * M

				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

				draw3DObject(dcu);

				}
				else{
				glm::mat4 translateTriangle = glm::translate (glm::vec3(0.0f+(j+1)*6-30, 0.0f+posy[i][j], 0.0f+(i+1)*6-30)); // glTranslatef
				glm::mat4 rotateTriangle = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1));
				glm::mat4 scaleTriangle = glm::scale (glm::vec3(1.5f, 0.4f, 1.5f)); // glTranslatef
				// rotate about vector (1,0,0)
				glm::mat4 triangleTransform = translateTriangle * rotateTriangle*scaleTriangle;
				Matrices.model *= triangleTransform; 
				MVP = VP * Matrices.model; // MVP = p * V * M

				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
				draw3DObject(cuboid[i][j]);
				}
				
	/*			Matrices.model = glm::mat4(1.0f);

glm::mat4 translateTriangle1 = glm::translate (glm::vec3(0.0f+(j+1)*6-30, 0.0f+posy[i][j], 0.0f+(i+1)*6-30)); // glTranslatef
				glm::mat4 rotateTriangle1 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1));
				glm::mat4 scaleTriangle1 = glm::scale (glm::vec3(1.5f, 0.4f, 1.5f)); // glTranslatef
				// rotate about vector (1,0,0)
				glm::mat4 triangleTransform1 = translateTriangle1 * rotateTriangle1*scaleTriangle1;
				Matrices.model *= triangleTransform1; 
				MVP = VP * Matrices.model; // MVP = p * V * M

				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

				draw3DObject(rect[i][j]);
*/

			}
		}
	}
	spo-=2;
	if(spo<0)
		spo=0;
	int l1,r1,l2,r2;
	l1=(-18+posx1+l3+l6+l7)/6 +4;
	r1=(-6+posz1+r3+r6+r7+r8+r9)/6+4;
	l2=(-18+posx2+l3+l6+l7)/6+4;
	r2=(-6+posz2+r3+r6+r7+r8+r9)/6+4;

	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateTriangle1 = glm::translate (glm::vec3(-18.0f+posx1+l3+l6+l7, 3.0f+posy1+spo, -6.0f+posz1+r3+r4+r6+r7+r8+r9)); // glTranslatef

	glm::mat4 rotateTriangle1 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,-3));

	glm::mat4 scaleTriangle1 = glm::scale (glm::vec3(1.5f, 1.5f, 1.5f)); // glTranslatef
	// rotate about vector (1,0,0)
	glm::mat4 triangleTransform1 = translateTriangle1*rotateTriangle1 * scaleTriangle1;
	Matrices.model *= triangleTransform1; 
	MVP = VP * Matrices.model; // MVP = p * V * M

	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	draw3DObject(cub1);

	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateTriangle2 = glm::translate (glm::vec3(-18.0f+posx2+l3+l6+l7, 3.0f+posy2+spo, -6.0f+posz2+r3+r4+r6+r7+r8+r9)); // glTranslatef

	glm::mat4 rotateTriangle2 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,-3));

	glm::mat4 scaleTriangle2 = glm::scale (glm::vec3(1.5f, 1.5f, 1.5f)); // glTranslatef
	// rotate about vector (1,0,0)
	glm::mat4 triangleTransform2 = translateTriangle2*rotateTriangle2 * scaleTriangle2;
	Matrices.model *= triangleTransform2; 
	MVP = VP * Matrices.model; // MVP = p * V * M

	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	draw3DObject(cub2);
	if(a[r1][l1]==0 || a[r2][l2]==0 || r1<0 ||l1<0||r2<0||l2<0){
	//Matrices.projection = glm::ortho(-100.0f,100.0f,-50.0f,50.0f,0.1f, 500.0f);
		if(soff==0)
		system("mpg123  -vC star.mp3 &");
		double ctime=glfwGetTime();
		if(ctime-utime>0.05){
			utime=glfwGetTime();
			posy1-=2;
			posy2-=2;
			disable=1;
		}
		if(posy1<-15){
	view=0;
		attempts++;

			score-=10;
			moves-=stmove;
		init();
		if(flag==1){
			level1();
		}
		if(flag==2)
			level2();
		if(flag==3)
			level3();
		if(flag==4)
			level4();
		if(flag==5)
			level6();
		if(flag==6)
			level7();
		if(flag==7)
			level8();
		if(flag==8)
			level9();
		if(flag==9)
			utime1=glfwGetTime();
	}

	}
	
	if(a[r1][l1]==4 && a[r2][l2]==4){
		attempts=1;
		if(sound==0){
		if(soff==0)

		system("mpg123  -vC finish.mp3 &");
		sound=1;
	}
		double ctime=glfwGetTime();
		if(ctime-utime>0.05){
			utime=glfwGetTime();
			posy1-=2;
			posy2-=2;
		}
		if(posy1<-20){
		init();
		flag++;
		score+=100;
		if(flag==2)
		level2();
		if(flag==3){
			level3();
			l3=0;
			r3=18;
		}
		if(flag==4){
			level4();	
		}
		if(flag==5){
			level6();
			l6=-6;
			r6=-18;
		}
		if(flag==6){
			level7();
			l7=6;
			r7=6;
		}
		if(flag==7){
			level8();
			r8=6;
		}
		if(flag==8){
			level9();
			r9=-6;
		}
		if(flag==1)
			level1();
		}
		if(flag==9)
			utime1=glfwGetTime();
	}
	if(flag==4){
		for(int i=1;i<3;i++)
			for(int j=3;j<10;j++)
				a[i][j]=6;
			for(int i=6;i<10;i++)
			for(int j=9;j<15;j++)
				a[i][j]=6;
		a[8][13]=5;
	

	}
	Matrices.model = glm::mat4(1.0f);

glm::mat4 translateTriangle11 = glm::translate (glm::vec3(75,-20 ,0 )); // glTranslatef
				glm::mat4 rotateTriangle11 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1));
				glm::mat4 scaleTriangle11 = glm::scale (glm::vec3(2, 2, 1)); // glTranslatef
				// rotate about vector (1,0,0)
				glm::mat4 triangleTransform11 = translateTriangle11* rotateTriangle11*scaleTriangle11;
				Matrices.model *= triangleTransform11; 
				//MVP = VP * Matrices.model; // MVP = p * V * M
	MVP = Matrices.projection * Matrices.view * Matrices.model;

				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

				draw3DObject(triangle);

				Matrices.model = glm::mat4(1.0f);

glm::mat4 translateTriangle12 = glm::translate (glm::vec3(85,-30 ,0 )); // glTranslatef
				glm::mat4 rotateTriangle12 = glm::rotate((float)(270*M_PI/180.0f), glm::vec3(0,0,1));
				glm::mat4 scaleTriangle12 = glm::scale (glm::vec3(2, 2, 1)); // glTranslatef
				// rotate about vector (1,0,0)
				glm::mat4 triangleTransform12 = translateTriangle12* rotateTriangle12*scaleTriangle12;
				Matrices.model *= triangleTransform12; 
				//MVP = VP * Matrices.model; // MVP = p * V * M
	MVP = Matrices.projection * Matrices.view * Matrices.model;

				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

				draw3DObject(triangle1);

				Matrices.model = glm::mat4(1.0f);

glm::mat4 translateTriangle13 = glm::translate (glm::vec3(75,-40 ,0 )); // glTranslatef
				glm::mat4 rotateTriangle13 = glm::rotate((float)(180*M_PI/180.0f), glm::vec3(0,0,1));
				glm::mat4 scaleTriangle13 = glm::scale (glm::vec3(2, 2, 1)); // glTranslatef
				// rotate about vector (1,0,0)
				glm::mat4 triangleTransform13 = translateTriangle13* rotateTriangle13*scaleTriangle13;
				Matrices.model *= triangleTransform13; 
				//MVP = VP * Matrices.model; // MVP = p * V * M
	MVP = Matrices.projection * Matrices.view * Matrices.model;

				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

				draw3DObject(triangle2);

				Matrices.model = glm::mat4(1.0f);

glm::mat4 translateTriangle14 = glm::translate (glm::vec3(65,-30 ,0 )); // glTranslatef
				glm::mat4 rotateTriangle14 = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1));
				glm::mat4 scaleTriangle14 = glm::scale (glm::vec3(2, 2, 1)); // glTranslatef
				// rotate about vector (1,0,0)
				glm::mat4 triangleTransform14 = translateTriangle14* rotateTriangle14*scaleTriangle14;
				Matrices.model *= triangleTransform14; 
				//MVP = VP * Matrices.model; // MVP = p * V * M
	MVP = Matrices.projection * Matrices.view * Matrices.model;

				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

				draw3DObject(triangle3);

	if(flag==2){
		Matrices.model = glm::mat4(1.0f);

glm::mat4 translateTriangle1 = glm::translate (glm::vec3(0.0f+(2+1)*6-30, 0.0f, 0.0f+(4+1)*6-30)); // glTranslatef
				glm::mat4 rotateTriangle1 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1));
				glm::mat4 scaleTriangle1 = glm::scale (glm::vec3(1.5f, 0.4f, 1.5f)); // glTranslatef
				// rotate about vector (1,0,0)
				glm::mat4 triangleTransform1 = translateTriangle1 * rotateTriangle1*scaleTriangle1;
				Matrices.model *= triangleTransform1; 
				MVP = VP * Matrices.model; // MVP = p * V * M

				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

				draw3DObject(dcub2);

				Matrices.model = glm::mat4(1.0f);

glm::mat4 translateTriangle2 = glm::translate (glm::vec3(0.0f+(8+1)*6-30, 0.0f, 0.0f+(3+1)*6-30)); // glTranslatef
				glm::mat4 rotateTriangle2 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1));
				glm::mat4 scaleTriangle2 = glm::scale (glm::vec3(1.5f, 0.4f, 1.5f)); // glTranslatef
				// rotate about vector (1,0,0)
				glm::mat4 triangleTransform2 = translateTriangle2 * rotateTriangle2*scaleTriangle2;
				Matrices.model *= triangleTransform2; 
				MVP = VP * Matrices.model; // MVP = p * V * M

				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

				draw3DObject(dcub3);

		if(a[r1][l1]==2 || a[r2][l2]==2){
		if(a[6][4]==0 && l2tog==0){
		if(soff==0)

		system("mpg123  -vC star.mp3 &");

			a[6][4]=1;
			a[6][5]=1;
			l2f=1;
		}
		else if(a[6][4]==1 && l2tog==1){
		if(soff==0)

		system("mpg123  -vC star.mp3 &");

			a[6][4]=0;
			a[6][5]=0;
			l2f=0;
		}
		}
		else if(l2f==1){
			l2tog=1;
		}
		else if(l2f==0){
			l2tog=0;
		}
		if(a[r1][l1]==3 && a[r2][l2]==3){
		if(a[6][10]==0 && l2togl==0){
		if(soff==0)

		system("mpg123  -vC star.mp3 &");

			a[6][10]=1;
			a[6][11]=1;
			l2r=1;
		}
		else if(a[6][10]==1  && l2togl==1){
		if(soff==0)

		system("mpg123  -vC star.mp3 &");

			a[6][10]=0;
			a[6][11]=0;
			l2r=0;
		}
		}
		else if(l2r==1){
			l2togl=1;
		}
		else if(l2r==0){
			l2togl=0;
		}
	}
	if(flag==4){
		
		
		 if((a[r1][l1]==6 && a[r2][l2]==6 && posy1!=posy2)){
		a[r1][l1]=0;
	}

	}
if(flag==6){
	
Matrices.model = glm::mat4(1.0f);

glm::mat4 translateTriangle1 = glm::translate (glm::vec3(0.0f+(9+1)*6-30, 0.0f, 0.0f+(5+1)*6-30)); // glTranslatef
				glm::mat4 rotateTriangle1 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1));
				glm::mat4 scaleTriangle1 = glm::scale (glm::vec3(1.5f, 0.4f, 1.5f)); // glTranslatef
				// rotate about vector (1,0,0)
				glm::mat4 triangleTransform1 = translateTriangle1 * rotateTriangle1*scaleTriangle1;
				Matrices.model *= triangleTransform1; 
				MVP = VP * Matrices.model; // MVP = p * V * M

				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

				draw3DObject(dcub);
				//printf("%d %d\n",a[r1+1][l1+1],a[r2+1][l2+1]);
		if(a[r1][l1]==2 && a[r2][l2]==2){
			a[7][3]=1;
	}
}
	if(flag==7){
		Matrices.model = glm::mat4(1.0f);

glm::mat4 translateTriangle1 = glm::translate (glm::vec3(0.0f+(4+1)*6-30, 0.0f, 0.0f+(5+1)*6-30)); // glTranslatef
				glm::mat4 rotateTriangle1 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1));
				glm::mat4 scaleTriangle1 = glm::scale (glm::vec3(1.5f, 0.4f, 1.5f)); // glTranslatef
				// rotate about vector (1,0,0)
				glm::mat4 triangleTransform1 = translateTriangle1 * rotateTriangle1*scaleTriangle1;
				Matrices.model *= triangleTransform1; 
				MVP = VP * Matrices.model; // MVP = p * V * M

				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

				draw3DObject(dcub1);
				if(a[r1][l1]==7 && a[r2][l2]==7){
			posx1+=36;
			posx2+=36;
			posy1-=6;
			posz1+=18;
			posz2-=18;
			l8f=1;
	}
	if(r1==5 && l1==11 && r2==5 && l2==12)
		l8f=0;
	else if(r2==5 && l2==12)
		l8f=2;
	}

	if(flag==8){
		Matrices.model = glm::mat4(1.0f);

glm::mat4 translateTriangle1 = glm::translate (glm::vec3(0.0f+(13+1)*6-30, 0.0f, 0.0f+(4+1)*6-30)); // glTranslatef
				glm::mat4 rotateTriangle1 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1));
				glm::mat4 scaleTriangle1 = glm::scale (glm::vec3(1.5f, 0.4f, 1.5f)); // glTranslatef
				// rotate about vector (1,0,0)
				glm::mat4 triangleTransform1 = translateTriangle1 * rotateTriangle1*scaleTriangle1;
				Matrices.model *= triangleTransform1; 
				MVP = VP * Matrices.model; // MVP = p * V * M

				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

				draw3DObject(dcub1);
				if(a[r1][l1]==7 && a[r2][l2]==7){
			posx1-=6;
			posx2-=66;
			posy2-=6;
			//posz1+=18;
			//posz2-=18;
			l8f=1;
	}
	if(r1==5 && l1==7 && r2==4 && l2==7)
		l8f=0;
	else if(r2==4 && l2==7)
		l8f=2;
	}

float fontScaleValue = 10 ;
static int fontScale=280;
	glm::vec3 fontColor = getRGBfromHue (fontScale);

	glUseProgram(fontProgramID);


	char level_str[30];
	sprintf(level_str,"MOVES: %d",moves);
	Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

	// Transform the text
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText = glm::translate(glm::vec3(50,35,0));
	glm::mat4 scaleText = glm::scale(glm::vec3(fontScaleValue,fontScaleValue,fontScaleValue));
	Matrices.model *= (translateText * scaleText);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);
	GL3Font.font->Render(level_str);

	//display_string(50,35,level_str,fontScaleValue);


	// Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
	// glPopMatrix ();
}
	// Increment angles
	float increments = 1;

	//camera_rotation_angle++; // Simulating camera rotation
	// triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
	//rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
	GLFWwindow* window; // window desciptor/handle

	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		//        exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

	if (!window) {
		glfwTerminate();
		//        exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glfwSwapInterval( 1 );

	/* --- register callbacks with GLFW --- */

	/* Register function to handle window resizes */
	/* With Retina display on Mac OS X GLFW's FramebufferSize
	   is different from WindowSize */
	glfwSetFramebufferSizeCallback(window, reshapeWindow);
	glfwSetWindowSizeCallback(window, reshapeWindow);

	/* Register function to handle window close */
	glfwSetWindowCloseCallback(window, quit);

	/* Register function to handle keyboard input */
	glfwSetKeyCallback(window, keyboard);      // general keyboard input
	glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

	/* Register function to handle mouse click */
	glfwSetMouseButtonCallback(window, mouseButton); 
	glfwSetScrollCallback(window, mousezoom); // mouse button clicks

	return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
	/* Objects should be created before any other gl function and shaders */
	// Create the models
	createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	createRectangle ();
	createCuboid();
	createCircle();
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

	// Background color of the scene
	glClearColor (1.0f, 1.0f, 1.0f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);



const char* fontfile = "monaco.ttf";
	GL3Font.font = new FTExtrudeFont(fontfile); // 3D extrude style rendering

	if(GL3Font.font->Error())
	{
		cout << "Error: Could not load font `" << fontfile << "'" << endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// Create and compile our GLSL program from the font shaders
	fontProgramID = LoadShaders( "fontrender.vert", "fontrender.frag" );
	GLint fontVertexCoordAttrib, fontVertexNormalAttrib, fontVertexOffsetUniform;
	fontVertexCoordAttrib = glGetAttribLocation(fontProgramID, "vertexPosition");
	fontVertexNormalAttrib = glGetAttribLocation(fontProgramID, "vertexNormal");
	fontVertexOffsetUniform = glGetUniformLocation(fontProgramID, "pen");
	GL3Font.fontMatrixID = glGetUniformLocation(fontProgramID, "MVP");
	GL3Font.fontColorID = glGetUniformLocation(fontProgramID, "fontColor");

	GL3Font.font->ShaderLocations(fontVertexCoordAttrib, fontVertexNormalAttrib, fontVertexOffsetUniform);
	GL3Font.font->FaceSize(1);
	GL3Font.font->Depth(0);
	GL3Font.font->Outset(0, 0);
GL3Font.font->CharMap(ft_encoding_unicode);


	cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 1500;
	int height = 800;

	GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

	double last_update_time = glfwGetTime();

	/* Draw in loop */
	while (!glfwWindowShouldClose(window)) {

		// OpenGL Draw commands
		draw();

		// Swap Frame Buffer in double buffering
		glfwSwapBuffers(window);
 display_string(window);

		// Poll for Keyboard and mouse events
		glfwPollEvents();
		if(heli==1  && lmouse1==1)
			drag(window);
		// Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
		current_time = glfwGetTime(); // Time in seconds
		if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
			// do something every 0.5 seconds ..
			last_update_time = current_time;
		}
	}

	glfwTerminate();
	//    exit(EXIT_SUCCESS);
}
