#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

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


GLuint programID,fontProgramID, textureProgramID;

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
int heli=0,lmouse1=0,mouse=0,blo=0;
float lx1,zoom=1,camera_rotation_angle1,camera_rotation_angle;
double lxg,lyg;
/**************************
 * Customizable functions *
 **************************/
int a[15][10];
void Matrix()
{
	for(int i=0;i<15;i++)
	{
		for(int j=0;j<10;j++)
		{
			a[i][j]=0;
		}
	}
	//a[0][7]=1;a[0][6]=1;a[0][5]=1;a[1][7]=1;a[1][6]=1;a[1][5]=1;a[1][4]=1;a[2][7]=1;a[2][6]=1;a[2][5]=1;a[2][4]=1;a[3][7]=1;a[3][6]=1;a[3][5]=1;a[3][4]=1;a[4][6]=1;a[4][5]=1;a[4][4]=1;a[5][6]=1;a[5][5]=1;a[5][4]=1;a[5][3]=1;a[6][5]=1;a[6][4]=1;a[6][3]=1;a[6][2]=1;a[7][5]=1;a[7][4]=1;a[7][2]=1;a[8][5]=1;a[8][4]=1;a[8][3]=1;a[8][2]=1;a[9][4]=1;a[9][3]=1;
	a[9][3]=1;a[9][4]=1;a[9][5]=1;a[8][3]=1;a[8][4]=1;a[8][5]=1;a[8][6]=1;a[7][3]=1;a[7][4]=1;a[7][5]=1;a[7][6]=1;a[6][3]=1;a[6][4]=1;a[6][5]=1;a[6][6]=1;a[5][3]=1;a[5][4]=1;a[5][5]=1;a[5][6]=1;a[4][3]=1;a[4][4]=1;a[4][5]=1;
	a[4][6]=1;a[4][7]=1;a[4][8]=1;a[3][3]=1;a[3][4]=1;a[3][5]=1;a[3][6]=1;a[3][7]=1;a[3][8]=1;a[2][3]=1;a[2][4]=1;a[2][5]=1;a[2][6]=1;a[2][8]=1;a[1][3]=1;a[1][4]=1;a[1][5]=1;a[1][6]=1;a[1][7]=1;a[1][8]=1;
}
void Matrix1()
{
	for(int i=0;i<15;i++)
	{
		for(int j=0;j<10;j++)
		{
			a[i][j]=0;
		}
	}
	a[14][3]=1;a[14][4]=1;a[14][5]=1;a[14][6]=1;a[14][7]=1;a[13][3]=1;a[13][4]=1;a[13][5]=1;a[13][6]=1;a[13][7]=1;a[12][3]=1;a[12][4]=1;a[12][5]=1;a[12][6]=1;a[12][7]=1;a[11][3]=1;a[11][4]=1;a[11][5]=1;a[11][6]=1;a[11][7]=1;a[8][3]=1;a[8][4]=1;a[8][5]=1;a[8][6]=1;a[8][7]=1;a[7][3]=1;a[7][4]=1;a[7][5]=1;a[7][6]=1;a[7][7]=1;a[6][3]=1;a[6][4]=1;a[6][5]=1;a[6][6]=1;a[6][7]=1;a[5][3]=1;a[5][4]=1;a[5][5]=1;a[5][7]=1;a[5][6]=1;a[2][3]=1;a[2][4]=1;a[2][5]=1;a[2][6]=1;a[2][7]=1;a[1][4]=1;a[1][5]=1;a[1][6]=1;a[1][7]=1;a[0][3]=1;a[0][4]=1;a[0][5]=1;a[0][6]=1;a[0][7]=1;a[2][2]=1;a[1][2]=1;a[0][2]=1;
}
void Matrix2()
{
	for(int i=0;i<15;i++)
	{
		for(int j=0;j<10;j++)
		{
			a[i][j]=0;
		}
	}
	a[14][7]=1;a[14][3]=1;a[14][4]=1;a[14][5]=1;a[14][6]=1;a[13][7]=1;a[13][3]=1;a[13][4]=1;a[13][5]=1;a[13][6]=1;a[12][7]=1;a[12][3]=1;a[12][4]=1;a[12][5]=1;a[12][6]=1;a[11][3]=1;a[11][1]=2;a[11][2]=2;a[10][1]=2;a[10][2]=2;a[9][2]=2;a[9][1]=2;a[8][2]=2;a[8][1]=2;a[7][2]=2;a[7][1]=2;a[6][2]=2;a[6][1]=2;a[5][2]=2;a[5][1]=2;a[5][3]=1;a[4][3]=1;a[4][4]=1;a[4][5]=1;a[3][3]=1;a[3][4]=1;a[3][5]=1;
	a[4][6]=2;a[4][7]=2;a[4][8]=2;a[4][9]=2;a[3][9]=2;a[3][8]=2;a[3][7]=2;a[3][6]=2;a[2][9]=2;a[2][7]=2;a[2][6]=2;a[1][9]=2;a[1][8]=2;a[1][7]=2;a[1][6]=2;a[4][7]=2;a[4][6]=2;a[5][7]=2;a[5][6]=2;a[6][7]=1;a[6][6]=1;	
}
void drag (GLFWwindow* window){
	double lx1;
	double ly1;
	glfwGetCursorPos(window, &lx1, &ly1);
	//printf("%f %f\n",lx1,lxg);
		cout<<"hhk";
	if(heli==1 && mouse==1){
		cout<<"hhk";
		camera_rotation_angle-=(lx1-lxg)/800;
		camera_rotation_angle1-=(lx1-lxg)/800;
	}
}
int moves=0;
float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
int count=0;
/*int heli=0,lmouse=0;
  float camera_rotation_angle,lxg,lx1,zoom;*/
int flag220=0,flag111=0,flag101=0,flag26=0,flag=0,flag1=0,flag100=0,flag10=0,flag20=0,flag11=0,flag25=0;
float flag121=0,change=0,value3=0,value2=0,value5=0.4,value6=0.8,change1=0;
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Function is called first on GLFW_PRESS.

	if (action == GLFW_RELEASE) {
		switch (key) {
			case GLFW_KEY_LEFT:
				flag=0;
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
			case GLFW_KEY_UP:
				//flag=1;
				system("mpg123 -vC  sound1.mp3 &");
				moves++;
				if(abs(value3-value2)<0.01 && value5>value6 && abs(change-change1)<0.01)
				{
					//	printf("1\n");
					change1=change1+0.8;
					change=change+0.4;
					value5=0.4;
					value6=0.4;
				}
				else if(abs(value3-value2)<0.01 && value5<value6 &&abs(change-change1)<0.01)
				{
					//	printf("2\n");
					//	system("mpg123 -vC  sound1.mp3 &");
					change1=change1+0.4;
					change=change+0.8;
					value6=0.4;
					value5=0.4;
				}
				else if(change>change1+0.001)
				{
					//	printf("3\n");

					change+=0.4;
					change1+=0.8;
					value6=0.8;
					value5=0.4;
				}
				else if(change<change1-0.01)
				{
					//printf("4\n");
					change+=0.8;
					change1+=0.4;
					value5=0.8;
					value6=0.4;
				}
				else{
					//	printf("5\n");
					change=change+0.4;
					change1=change1+0.4;
					//	value5=0.4;
					//	value6=0.4;
				}
				break;
			case GLFW_KEY_DOWN:
				moves++;
				system("mpg123 -vC  sound1.mp3 &");
				if(abs(value3-value2)<0.01 && value5>value6 && abs(change-change1)<0.01)
				{
					printf("6\n");
					change1=change1-0.8;
					change=change-0.4;
					value5=0.4;
					value6=0.4;
				}
				else if(abs(value3-value2)<0.01 && value5<value6 &&abs(change-change1)<0.01)
				{
					printf("7\n");
					change1=change1-0.4;
					change=change-0.8;
					value6=0.4;
					value5=0.4;

				}
				else if(change<change1-0.01)
				{
					printf("8\n");
					change=change-0.4;
					change1=change1-0.8;
					value6=0.8;
					value5=0.4;
				}
				else if(change>change1+0.01)
				{
					printf("9\n");
					change=change-0.8;
					change1=change1-0.4;
					value5=0.8;
					value6=0.4;
				}
				else{
					printf("10\n");
					change=change-0.4;
					change1=change1-0.4;
					//	value6=0.4;
					//	value5=0.4;
				}

				break;
			case GLFW_KEY_LEFT:
				moves++;
				system("mpg123 -vC  sound1.mp3 &");
				if(abs(value3-value2)<0.01 && value6<value5 && abs(change-change1)<0.01)
				{
					value3=value3-0.4;
					value2=value2-0.8;
					value5=0.4;
					value6=0.4;

				}
				else if(abs(value3-value2)<0.01 && value6>value5 && abs(change-change1)<0.01)
				{
					value3=value3-0.8;
					value2=value2-0.4;
					value6=0.4;
					value5=0.4;
				}
				else if(value3>value2 && abs(change-change1)<0.01)
				{
					value2=value2-0.4;
					value3=value3-0.8;
					value6=0.8;
					value5=0.4;
				}
				else if(value3<value2 && abs(change-change1)<0.01)
				{
					value2=value2-0.8;
					value3=value3-0.4;
					value5=0.8;
					value6=0.4;
				}
				else if(abs(value3-value2)<0.01 && change!=change1)
				{
					value2=value2-0.4;
					value3=value3-0.4;
				}
				else{
					value2=value2-0.4;
					value3=value3-0.4;
				}




				break;
			case GLFW_KEY_RIGHT:
				moves++;
				system("mpg123 -vC  sound1.mp3 &");
				if(abs(value3-value2)<0.01 && value6<value5 && abs(change-change1)<0.01)
				{
					value3=value3+0.4;
					value2=value2+0.8;
					value5=0.4;
					value6=0.4;
				}
				else if(abs(value3-value2)<0.01 && value6>value5 && abs(change-change1)<0.01)
				{
					value3=value3+0.8;
					value2=value2+0.4;
					value6=0.4;
					value5=0.4;
				}
				else if(value3<value2 && abs(change-change1)<0.01)
				{
					value2=value2+0.4;
					value3=value3+0.8;
					value6=0.8;
				}
				else if(value3>value2)
				{
					value2=value2+0.8;
					value3=value3+0.4;
					value5=0.8;
				}
				else if(abs(value3-value2)<0.01 && change!=change1)
				{
					value2+=0.4;
					value3+=0.4;
				}
				else{
					value2=value2+0.4;
					value3=value3+0.4;
				}

				break;

			case GLFW_KEY_V:
				count++;
				break;
			default:
				break;
		}
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

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
	switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			if (action == GLFW_RELEASE)
			{
				triangle_rot_dir *= -1;
				lmouse1=0;
			}
			else if(action == GLFW_PRESS)
			{
				lmouse1=1;
			}

			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			if (action == GLFW_RELEASE) {

				rectangle_rot_dir *= -1;
			}
			break;

		default:
			break;
	}
	if(lmouse1==1){
		glfwGetCursorPos(window, &lxg, &lyg);
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if(GLFW_PRESS == action){
			//	lmouse1 = 1;
			mouse=1;
		}
		if(action==GLFW_RELEASE){
			//	lmouse1=0;
			mouse=0;
		}
	}
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
	int fbwidth=width, fbheight=height;
	/* With Retina display on Mac OS X, GLFW's FramebufferSize
	   is different from WindowSize */
	glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

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
	Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}

VAO *rectangle5,*rectangle4,*circle,*rectangle3,*rectangle2,*triangle, *rectangle,*rectangle1;

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
	triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
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

			-1.2,-1,0, //vertex 1
		1.2,-1,0, //vertex2
		1.2,-1,1, //vertex 5

		-1.2,-1,0,//vertex 1
		1.2,-1,1,//vertex 5
		-1.2,-1,1,//vertex 6

		-1.2,-1,0, //vertex 1
		-1.2,1,0, //vertex 4
		-1.2,-1,1, //vertex 6

		-1.2,-1,0, //vertex 4
		-1.2,-1,1, //vertex 6 
		-1.2,1,1, //vertex 7

		-1.2,1,0,
		1.2,1,0,
		1.2,1,1,

		-1.2,1,0,
		1.2,1,1,
		-1.2,1,1,

		1.2,1,0,
		1.2,-1,0,
		1.2,-1,1,

		1.2,1,1,
		1.2,-1,1,
		1.2,1,1,

		-1.2,-1,1,
		1.2,-1,1,
		-1.2,1,1,

		1.2,-1,1,
		-1.2,1,1,
		1.2,1,1

	};

	static const GLfloat color_buffer_data [] = {
		1,0,0, // color 1
		0,0,1, // color 2
		0,1,0, // color 3

		0,1,0, // color 3
		0.3,0.3,0.3, // color 4
		1,0,0,  // color 1
		1,0,0, // color 1
		0,0,1, // color 2
		0,1,0, // color 3

		0,1,0, // color 3
		0.3,0.3,0.3, // color 4
		1,0,0, // color 1
		1,0,0, // color 1
		0,0,1, // color 2
		0,1,0, // color 3

		0,1,0, // color 3
		0.3,0.3,0.3, // color 4
		1,0,0,  // color 1
		1,0,0, // color 1
		0,0,1, // color 2
		0,1,0, // color 3

		0,1,0, // color 3
		0.3,0.3,0.3, // color 4
		1,0,0,  // color 1
		1,0,0, // color 1
		0,0,1, // color 2
		0,1,0, // color 3

		0,1,0, // color 3
		0.3,0.3,0.3, // color 4
		1,0,0,  // color 1
		1,0,0, // color 1
		0,0,1, // color 2
		0,1,0, // color 3

		0,1,0, // color 3
		0.3,0.3,0.3, // color 4
		1,0,0  // color 1

	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
float x=0,y=0;
void createrectangle()
{
	static const GLfloat vertex_buffer_data1 [] = {
		-0.2f,-0.2f,-0.05f, // triangle 1 : begin
		-0.2f,-0.2f, 0.05f,
		-0.2f, 0.2f, 0.05f, // triangle 1 : end
		0.2f, 0.2f,-0.05f, // triangle 2 : begin
		-0.2f,-0.2f,-0.05f,
		-0.2f, 0.2f,-0.05f, // triangle 2 : end
		0.2f,-0.2f, 0.05f,
		-0.2f,-0.2f,-0.05f,
		0.2f,-0.2f,-0.05f,
		0.2f, 0.2f,-0.05f,
		0.2f,-0.2f,-0.05f,
		-0.2f,-0.2f,-0.05f,
		-0.2f,-0.2f,-0.05f,
		-0.2f, 0.2f, 0.05f,
		-0.2f, 0.2f,-0.05f,
		0.2f,-0.2f, 0.05f,
		-0.2f,-0.2f, 0.05f,
		-0.2f,-0.2f,-0.05f,
		-0.2f, 0.2f, 0.05f,
		-0.2f,-0.2f, 0.05f,
		0.2f,-0.2f, 0.05f,
		0.2f, 0.2f, 0.05f,
		0.2f,-0.2f,-0.05f,
		0.2f, 0.2f,-0.05f,
		0.2f,-0.2f,-0.05f,
		0.2f, 0.2f, 0.05f,

		0.2f,-0.2f, 0.05f,
		0.2f, 0.2f, 0.05f,
		0.2f, 0.2f,-0.05f,
		-0.2f, 0.2f,-0.05f,
		0.2f, 0.2f, 0.05f,
		-0.2f, 0.2f,-0.05f,
		-0.2f, 0.2f, 0.05f,
		0.2f, 0.2f, 0.05f,
		-0.2f, 0.2f, 0.05f,
		0.2f,-0.2f, 0.05f
	};
	GLfloat color_buffer_data1 [108]; 
	for (int v = 0; v < 12*3 ; v++){
		color_buffer_data1[3*v+0] = 1;
		color_buffer_data1[3*v+1] = 0.5;
		color_buffer_data1[3*v+2] = 0.6;
	}
	GLfloat color_buffer_data2 [108];
	for(int h=0;h<12*3;h++)
	{
		color_buffer_data2[3*h] = 0;
		color_buffer_data2[3*h+1] = 0;
		color_buffer_data2[3*h+2] = 0;
	}
	GLfloat color_buffer_data3 [108];
	for(int m=0;m<36;m++)
	{
		color_buffer_data3[3*m]=102.0/255.0;
		color_buffer_data3[3*m+1]=0;
		color_buffer_data3[3*m+2]=0;
	}
	GLfloat color_buffer_data4 [108];
	for(int m=0;m<36;m++)
	{
		color_buffer_data4[3*m]=1;
		color_buffer_data4[3*m+1]=0.5;
		color_buffer_data4[3*m+2]=0.3;
	}
	GLfloat color_buffer_data7 [108];
	for(int v=0;v<12*3;v++)
	{
		if(vertex_buffer_data1[3*v+0]==-0.2 && vertex_buffer_data1[3*v+1]==-0.2 &&vertex_buffer_data1[3*v+2]==-0.05){
			color_buffer_data7[3*v+0] = 1;
			color_buffer_data7[3*v+1] = 0.9;
			color_buffer_data7[3*v+2] = 0;
		}
		if(vertex_buffer_data1[3*v+0]==0.2 && vertex_buffer_data1[3*v+1]==0.2 && vertex_buffer_data1[3*v+2]==0.05){
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
	rectangle1 = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data1, color_buffer_data1, GL_FILL);
	//rectangle2 = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data1, color_buffer_data2, GL_LINE);
	rectangle2 = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data1, color_buffer_data7, GL_FILL);
	rectangle3 = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data1, color_buffer_data3, GL_FILL);
	rectangle4 = create3DObject(GL_TRIANGLES, 36,vertex_buffer_data1,color_buffer_data4,GL_FILL);
	rectangle5 = create3DObject(GL_TRIANGLES, 36,vertex_buffer_data1,color_buffer_data2,GL_LINE);

}
void createCircle()
{
	static GLfloat vertex_buffer_data [360*9];
	static  GLfloat vertex_buffer_data2 [360*9];

	for(int i=0;i<360;i++)
	{
		vertex_buffer_data[9*i]=0;
		vertex_buffer_data[9*i+1]=0;
		vertex_buffer_data[9*i+2]=0;
		vertex_buffer_data[9*i+3]=0.2*cos(i*M_PI/180);
		vertex_buffer_data[9*i+4]=0.2*sin(i*M_PI/180);
		vertex_buffer_data[9*i+5]=0;
		vertex_buffer_data[9*i+6]=0.2*cos((((i+1)%360))*M_PI/180);
		vertex_buffer_data[9*i+7]=0.2*sin(((i+1)%360)*M_PI/180);
		vertex_buffer_data[9*i+8]=0     ;    
	}

	static GLfloat color_buffer_data [360*9]={0};
	static  GLfloat color_buffer_data1 [360*9]={0};
	static  GLfloat color_buffer_data2 [360*9]={0};
	static  GLfloat color_buffer_data3 [360*9]={0};




	for (int i = 0; i<360*9 ; i+=3)
	{
		color_buffer_data[i]=0;
		color_buffer_data[i+1]=0;
		color_buffer_data[i+2]=0;
	}
	/*	for(int j=0;j<360*9;j+=3)
		{
		color_buffer_data1[j]=0;
		color_buffer_data1[j+1]=1;
		color_buffer_data1[j+2]=0;
		}*/
	circle = create3DObject(GL_TRIANGLES,360*3,vertex_buffer_data,color_buffer_data,GL_FILL);
	//circle2 = create3DObject(GL_TRIANGLES,360*3,vertex_buffer_data,color_buffer_data1,GL_FILL);
}

//float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;

/* Render the scene with openGL */
/* Edit this function according to your assignment */
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

	// Compute Camera matrix (view)
	// Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
	//  Don't change unless you are sure!!
	//	Matrices.view = glm::lookAt(glm::vec3(-6,-6,4), glm::vec3(0,0,0), glm::vec3(1,1,2)); // Fixed camera for 2D (ortho) in XY plane

	// Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
	//  Don't change unless you are sure!!
	count=count%5;
	if(count==0)
	{
		
		Matrices.projection = glm::ortho(-4.0f/zoom,4.0f/zoom,-4.0f/zoom,4.0f/zoom,0.1f, 500.0f);
		Matrices.view = glm::lookAt(glm::vec3(-6,-6,4), glm::vec3(0,0,0), glm::vec3(1,1,2));
	}
	else if(count==1)
	{
		Matrices.projection = glm::ortho(-4.0f/zoom,4.0f/zoom,-4.0f/zoom,4.0f/zoom,0.1f, 500.0f);
		Matrices.view = glm::lookAt(glm::vec3(0,0,4), glm::vec3(0,0,0), glm::vec3(0,1,0));
	}
	else if(count==2)
	{
		Matrices.projection = glm::ortho(-4.0f/zoom,4.0f/zoom,-4.0f/zoom,4.0f/zoom,0.1f, 500.0f);
		Matrices.view = glm::lookAt(glm::vec3(-4,-4,2), glm::vec3(0,0,0), glm::vec3(1,1,2));
	}
//	glm::mat4 VP = Matrices.projection * Matrices.view;
        if(count==3){
		heli=1;
			printf("%f\n",camera_rotation_angle);
		Matrices.projection = glm::ortho(-4.0f/zoom,4.0f/zoom,-4.0f/zoom,4.0f/zoom,0.1f, 500.0f);

		Matrices.view = glm::lookAt(glm::vec3(-6*cos(camera_rotation_angle*M_PI/180),-6*sin(camera_rotation_angle*M_PI/180),4), glm::vec3(0,0,0), glm::vec3(1,1,2));
	}
	else if(count==4)
	{
//		Matrices.projection = glm::ortho(-4.0f/zoom,4.0f/zoom,-4.0f/zoom,4.0f/zoom,0.1f, 500.0f);
	  //      Matrices.view = glm::lookAt(glm::vec3(-6*cos(camera_rotation_angle*M_PI/180),-6*sin(camera_rotation_angle*M_PI/180),4), glm::vec3(0,0,0), glm::vec3(1,1,2));
        Matrices.projection = glm::perspective(0.8f,(GLfloat) 600 / (GLfloat) 600,0.1f, 500.0f);
	        Matrices.view = glm::lookAt(glm::vec3(value3-0.3,change,value6+0.2), glm::vec3(value3+3,change+0.4,value6-2), glm::vec3(0,0,1));
	}
	// Send our transformation to the currently bound shader, in the "MVP" uniform
	// For each model you render, since the MVP will be different (at least the M part)
	//  Don't change unless you are sure!!
	glm::mat4 MVP;	// MVP = Projection * View * Model

glm::mat4 VP = Matrices.projection * Matrices.view;
	// Load identity to model matrix
	//Matrices.model = glm::mat4(1.0f);

	/* Render your scene */

	/*glm::mat4 translateTriangle = glm::translate (glm::vec3(-2.0f, 0.0f, 0.0f)); // glTranslatef
	  glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
	  glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
	  Matrices.model *= triangleTransform; 
	  MVP = VP * Matrices.model; // MVP = p * V * M

	//  Don't change unless you are sure!!
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	// draw3DObject draws the VAO given to it using current MVP matrix
	//draw3DObject(triangle);*/

	// Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
	// glPopMatrix ();
	/*int ti=(glfwGetTime());
	  printf("%d\n",ti );
	  int ti1,ti2,ti3;
	  ti-=utime1;
	//printf("%d\n",ti );
	ti1=ti/3600;
	ti2=ti/60;
	ti3=(ti-(ti2*60));

	float fontScaleValue = 0.5f;
	int fontScale=350;
	glm::vec3 fontColor= getRGBfromHue(fontScale);

	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText = glm::translate(glm::vec3(2,3,0));
	glm::mat4 scaleText = glm::scale(glm::vec3(fontScaleValue,fontScaleValue,fontScaleValue));
	Matrices.model *= (translateText * scaleText);
	MVP = VP1 * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);
	GL3Font.font->Render(level_strl);

	char level_strl[30];
	sprintf(level_strl,"TIME: %d:%d:%d",ti1,ti2,ti3);
	glUseProgram(fontProgramID);*/
	printf("%d\n",moves);
	Matrices.model = glm::mat4(1.0f);


	glm::mat4 translateRectangle = glm::translate (glm::vec3(value3-0.7, change, value6));        // glTranslatef
	//	glm::mat4 rotateRectangle = glm::rotate((float)(value3), glm::vec3(0.5,change,0.4)); // rotate about vector (-1,1,1)
	glm::mat4 scaleRectangle = glm::scale (glm::vec3(1,1,4));
	Matrices.model *= (translateRectangle*scaleRectangle);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	draw3DObject(rectangle3);
	Matrices.model = glm::mat4(1.0f);


	glm::mat4  translateRectangle1 = glm::translate (glm::vec3(value2-0.7,change1, value5));      
	// glTranslatef
	//	glm::mat4 rotateRectangle1 = glm::rotate((float)(value3), glm::vec3(0.5,change,0.4)); // rotate about vector (-1,1,1)
	glm::mat4 scaleRectangle1 = glm::scale (glm::vec3(1,1,4));

	Matrices.model *= (translateRectangle1*scaleRectangle1);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	draw3DObject(rectangle3);
	if(abs(value3-2.4)<0.01 && abs(value2-2.4)<0.01 && abs(change+0.4)<0.01 && abs(change1+0.4)<0.01 && flag10==0)
	{
		flag100=1;
		value5=value5-0.02;
		value6=value6-0.02;
	}
	if((value3<-0.78 || value2<-0.78) && flag10==0)
	{
		flag220=1;
		value5=value5-0.02;
		value6=value6-0.02;
	}
	if((value3>-0.02 && value3<1.22 && (change<-0.38 || change1<-0.38)) && flag10==0)
	{
		flag220=1;
		value5=value5-0.02;
		value6=value6-0.02;
	}

	if((change>1.58 || change1>1.58) && flag10==0)
	{
		flag220=1;
		value5=value5-0.02;
		value6=value6-0.02;
	}
	if((change<-1.18 || change1<-1.18) && flag10==0)
	{
		flag220=1;
		value5=value5-0.02;
		value6=value6-0.02;
	}
	if(value5<-2.0 &&flag220==0)
	{
		value2=-2;
		value3=-2;
		change=0;
		change1=0;
		flag10=1;
		value5=0.4;
		value6=0.8;
	}
	if(value5<-2.0 && flag220==1)
	{
		value2=0;
		value3=0;
		change=0;
		change1=0;
		value5=0.4;
		value6=0.8;
	}
	//printf("%d\n",flag100);
	if(flag100==0)
	{

		Matrix();
	}
	if(flag100==1 && flag10==1 && flag111==0)
	{
		flag11=1;
		//	printf("1\n");
		if(flag121==0)
		{
			Matrices.model = glm::mat4(1.0f);
			glm::mat4 translateCircle = glm::translate (glm::vec3(-2.4,0.7,0.3));
			Matrices.model *= (translateCircle);
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(circle);
		}

		Matrix1();

		if((abs(value2+1.6)<0.01 || abs(value3+1.6)<0.01) && (abs(change-0.8)<0.01 || abs(change1-0.8)<0.01))
		{
			//		printf("%f\n",(change-0.8));

			flag25=1;
		}
		if(flag121==0)
		{
			Matrices.model = glm::mat4(1.0f);
			glm::mat4 translateCircle1 = glm::translate (glm::vec3(0,0.7,0.3));
			Matrices.model *= (translateCircle1);
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(circle);
		}
		//	printf("%f %f\n",change,change1);
		if(abs(value2-0.8)<0.01 && abs(value3-0.8)<0.01 && abs(change-0.8)<0.01 && abs(change1-0.8)<0.01)
		{
			//	printf("1\n");
			flag26=1;
		}
		if(abs(value2-2.8)<0.01 && abs(value3-2.8)<0.01 && abs(change-1.2)<0.01 && abs(change-1.2)<0.01)
		{
			//	printf("Entered\n");
			flag101=1;

			value5=value5-0.02;
			value6=value6-0.02;
			flag121=1;
		}

		if(value5<-2.0)
		{
			value5=0.4;
			value6=0.8;
			value2=-2;
			value3=-2;
			flag111=1;
			change=0;
			change1=0;

		}
		if(value2<-2.79 || value3<-2.79 || value2>3.21 || value3>3.21)
		{
			printf("2\n");
			value5=value5-0.02;
			value6=value6-0.02;
		}
		if(change>1.21 || change<-0.79 || change1>1.21 || change1<-0.79)
		{
			printf("3\n");
			value5=value5-0.02;
			value6=value6-0.02;
		}




	}
	//printf("%f %f\n",value2,change);
	if(flag25==1)
	{
		a[10][6]=1;a[9][6]=1;
	}
	if(flag26==1)
	{
		a[4][6]=1;a[3][6]=1;
	}
	if(flag101==1 && flag111==1)
	{
		Matrix2();

		if(value2>-1.19 && value2<1.21 && value3<1.21 && value3>-1.19 && abs(change-change1)<0.01 && abs(value2-value3)<0.01 && (abs(change-2)<0.01 || abs(change1-1.6)<0.01))
		{
			value5=value5-0.02;
			value6=value6-0.02;
		}
		//	printf("%f %f %f %f\n",value2,value3,change,change1);
		if(value2>1.21 && value2<2.81 && value2>1.21 && value3<2.81 && abs(change-change1)<0.01 && abs(value2-value3)<0.01 && change>-1.19 && change<0.01 && change1>-1.19 && change<0.01)
		{
			value5=value5-0.02;
			value6=value6-0.02;
		}
	}

	// draw3DObject draws the VAO given to it using current MVP matrix
	for(int i=0;i<15;i++)
	{
		for(int j=0;j<10;j++)
		{
			//	printf("%d\n",i);
			if(a[i][j]==1 )
			{
				if(((i+j)%2)==0)
				{
					Matrices.model = glm::mat4(1.0f);


					glm::mat4 translateRectangle = glm::translate (glm::vec3(-x+2.5,-y+2, 0.15));
					//glm::mat4 scaleRectangle2 = glm::scale(glm::vec3(2,2,1));
					// glTranslatef
					//glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
					Matrices.model *= (translateRectangle);
					MVP = VP * Matrices.model;
					glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);


					draw3DObject(rectangle1);
					/*	if(flag11=1)
						{
						Matrices.model = glm::mat4(1.0f);
						glm::mat4 translateCircle = glm::translate (glm::vec3(-2,0,0));
						Matrices.model *= (translateCircle);
						MVP = VP * Matrices.model;
						glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
						draw3DObject(circle);
						}*/
				}

				else {
					Matrices.model = glm::mat4(1.0f);


					glm::mat4 translateRectangle = glm::translate (glm::vec3(-x+2.5,-y+2, 0.15));        // glTranslatef
					//glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
					glm::mat4 scaleRectangle3 = glm::scale(glm::vec3(2,2,1));
					Matrices.model *= (translateRectangle);
					MVP = VP * Matrices.model;
					glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

					draw3DObject(rectangle2);
				}
			}
			else if(a[i][j]==2)
			{
				Matrices.model=glm::mat4(1.0f);
				glm::mat4 translateRectangle4 = glm::translate (glm::vec3(-x+2.5,-y+2,0.15));
				Matrices.model *= (translateRectangle4);
				MVP = VP * Matrices.model;
				glUniformMatrix4fv(Matrices.MatrixID, 1,GL_FALSE, &MVP[0][0]);
				draw3DObject(rectangle4);
				draw3DObject(rectangle5);
			}


			x=0.4*i;
			y=0.4*j;
			//	}

	}
}
float fontScaleValue = 1 ;
static int fontScale=280;
glm::vec3 fontColor = getRGBfromHue (fontScale);

glUseProgram(fontProgramID);


char level_str[30];
sprintf(level_str,"MOVES: %d",moves);
Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

// Transform the text
Matrices.model = glm::mat4(1.0f);
glm::mat4 translateText = glm::translate(glm::vec3(1,2,0));
glm::mat4 scaleText = glm::scale(glm::vec3(fontScaleValue,fontScaleValue,fontScaleValue));
Matrices.model *= (translateText * scaleText);
MVP = Matrices.projection * Matrices.view * Matrices.model;
// send font's MVP and font color to fond shaders
glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);
GL3Font.font->Render(level_str);

// Increment angles
float increments = 1;
int ti=glfwGetTime();
int ti1,ti2,ti3;
//ti-=utime1;
ti1=ti/3600;
ti2=ti/60;
ti3=(ti-(ti2*60));
float fontScaleValue1 = 1 ;
int fontScale1=280;
glm::vec3 fontColor1= getRGBfromHue(fontScale1);
glUseProgram(fontProgramID);


char level_strl[30];
sprintf(level_strl,"TIME: %d:%d:%d",ti1,ti2,ti3);




Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

// Transform the text
Matrices.model = glm::mat4(1.0f);
glm::mat4 translateText1 = glm::translate(glm::vec3(0,3,0));
glm::mat4 scaleText1 = glm::scale(glm::vec3(fontScaleValue1,fontScaleValue1,fontScaleValue1));
Matrices.model *= (translateText1 * scaleText1);
MVP = Matrices.projection * Matrices.view * Matrices.model;
// send font's MVP and font color to fond shaders
glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
glUniform3fv(GL3Font.fontColorID, 1, &fontColor1[0]);
GL3Font.font->Render(level_strl);

//camera_rotation_angle++; // Simulating camera rotation
/*triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
  rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;*/
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
	glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

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
	createrectangle();
	//Matrix();
	//Matrix1();
	createCircle();
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

	// Background color of the scene
	glClearColor (0.3f, 0.3f, 0.3f, 0.3f); // R, G, B, A
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
	int width = 600;
	int height = 600;

	GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

	double last_update_time = glfwGetTime(), current_time;

	/* Draw in loop */
	while (!glfwWindowShouldClose(window)) {
		draw();
		if(heli==1  && lmouse1==1){
			drag(window);
		}
		// OpenGL Draw commands


		// Swap Frame Buffer in double buffering
		glfwSwapBuffers(window);

		// Poll for Keyboard and mouse events
		glfwPollEvents();

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
