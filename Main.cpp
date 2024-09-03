// Include standard headers
#define _CRT_SECURE_NO_WARNINGS

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
using namespace std;

#include <stdlib.h>
#include <string.h>
#include <Windows.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"
#include "objloader.hpp"

GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;


	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const* VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}



	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const* FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}



	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}


	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}



// Very, VERY simple OBJ loader.
// Here is a short list of features a real function would provide : 
// - Binary files. Reading a model should be just a few memcpy's away, not parsing a file at runtime. In short : OBJ is not very great.
// - Animations & bones (includes bones weights)
// - Multiple UVs
// - All attributes should be optional, not "forced"
// - More stable. Change a line in the OBJ file and it crashes.
// - More secure. Change another line and you can inject code.
// - Loading from memory, stream, etc

bool loadOBJ(
	const char* path,
	std::vector<glm::vec3>& out_vertices,
	std::vector<glm::vec2>& out_uvs,
	std::vector<glm::vec3>& out_normals
) {
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;


	FILE* file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
		getchar();
		return false;
	}

	while (1) {

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		// else : parse lineHeader

		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				fclose(file);
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
		else {
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	// For each vertex of each triangle
	for (unsigned int i = 0; i < vertexIndices.size(); i++) {

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		glm::vec3 normal = temp_normals[normalIndex - 1];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs.push_back(uv);
		out_normals.push_back(normal);

	}
	fclose(file);
	return true;
}



int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(800, 800, u8"Ηλιακό Σύστημα", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// BLACK background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("TransformVertexShader.vertexshader", "TextureFragmentShader.fragmentshader");

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

																															


	// EDW KANW LOAD TO TEXTURE TOY HLIOY.

	int width, height, nrChannels;
	unsigned char* data = stbi_load("sun.jpg", &width, &height, &nrChannels, 0);
	 
	if (data)
	{

	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	GLuint textureID;
	glGenTextures(1, &textureID);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

	// Read our  sun.obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> uvs;
	bool res = loadOBJ("sun.obj", vertices, uvs, normals);
	
	// Load it into a VBO

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);



	// FTIAXNW NEO VERTEXARRAYID KAI NEO MVP GIA TON PLANHTH MOY.
	GLuint VertexArrayID2;
	glGenVertexArrays(1, &VertexArrayID2);
	glBindVertexArray(VertexArrayID2);
	MatrixID = glGetUniformLocation(programID, "MVP2");

	// EDW KANW LOAD TO TEXUTRE GIA TON PLANHTH MOY
	unsigned char* data2 = stbi_load("planet.jpg", &width, &height, &nrChannels, 0);
																								

	if (data2)
	{

	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	GLuint textureID2;            
	glGenTextures(1, &textureID2);   

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID2);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data2);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID2 = glGetUniformLocation(programID, "myTextureSampler");

	// Read our planet.obj file
	std::vector<glm::vec3> vertices2;
	std::vector<glm::vec3> normals2;
	std::vector<glm::vec2> uvs2;
	bool res2 = loadOBJ("planet.obj", vertices2, uvs2, normals2);
	// Load it into a VBO

	GLuint vertexbuffer2;
	glGenBuffers(1, &vertexbuffer2);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer2);
	glBufferData(GL_ARRAY_BUFFER, vertices2.size() * sizeof(glm::vec3), &vertices2[0], GL_STATIC_DRAW);

	GLuint uvbuffer2;
	glGenBuffers(1, &uvbuffer2);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer2);
	glBufferData(GL_ARRAY_BUFFER, uvs2.size() * sizeof(glm::vec2), &uvs2[0], GL_STATIC_DRAW);

	glm::mat4 ModelMatrix2 = glm::mat4(1.0f);
	ModelMatrix2 = glm::translate(ModelMatrix2, vec3(25.0f, 0.0f, 0.0f)); // O PLANHTHS MOY EINAI STHN ARXIKH THESH (25,0,0)
	GLfloat x1 = 25.0f;													  // ARXIKOPOIW TIS METABLHTES POY EINAI APARAITHTES GIA THN KINHSH TOY PLANHTH MAS.
	GLfloat y1 = 0.0f;
	GLfloat x2;	
	GLfloat y2;
	GLfloat angle = 0.01f;                                                // H TAXYTHTA KINHSHS TOY PLANHTH MAS
	GLfloat PI = 3.14f;
																								



	// FTIAXNW NEO VERTEXARRAYID KAI NEO MVP GIA TON METEWRITH MOY.
	GLuint VertexArrayID3;
	glGenVertexArrays(1, &VertexArrayID3);
	glBindVertexArray(VertexArrayID3);
	MatrixID = glGetUniformLocation(programID, "MVP3");

	// EDW KANW LOAD TO TEXUTRE GIA TON METEWRITH MOY
	unsigned char* data3 = stbi_load("meteor.jpg", &width, &height, &nrChannels, 0);


	if (data3)
	{

	}
	else
	{
	std::cout << "Failed to load texture" << std::endl;
	}

	GLuint textureID3;
	glGenTextures(1, &textureID3);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID3);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data3);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID3 = glGetUniformLocation(programID, "myTextureSampler");

	// Read our meteor.obj file
	std::vector<glm::vec3> vertices3;
	std::vector<glm::vec3> normals3;
	std::vector<glm::vec2> uvs3;
	bool res3 = loadOBJ("meteor.obj", vertices3, uvs3, normals3);
	// Load it into a VBO

	GLuint vertexbuffer3;
	glGenBuffers(1, &vertexbuffer3);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer3);
	glBufferData(GL_ARRAY_BUFFER, vertices3.size() * sizeof(glm::vec3), &vertices3[0], GL_STATIC_DRAW);

	GLuint uvbuffer3;
	glGenBuffers(1, &uvbuffer3);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer3);
	glBufferData(GL_ARRAY_BUFFER, uvs3.size() * sizeof(glm::vec2), &uvs3[0], GL_STATIC_DRAW);

	glm::mat4 ModelMatrix3 = glm::mat4(1.0f);
	ModelMatrix3 = glm::translate(ModelMatrix3, vec3(0.0f, 0.0f, 80.0f));  // O METEWRITHS MOY EINAI STHN THESH TOY PARATHRH (0,0,80)





	// EDW KANW LOAD TO OBJECT THS GHS KAI TO TEXTURE THS. FTIAXNW NEOYS BUFFERS GIA NA MPORW NA TO SXHMATISW
	
	GLuint VertexArrayID4;
	glGenVertexArrays(1, &VertexArrayID4);
	glBindVertexArray(VertexArrayID4);
	MatrixID = glGetUniformLocation(programID, "MVP4");

	unsigned char* data4 = stbi_load("earth.jpg", &width, &height, &nrChannels, 0);


	if (data4)
	{

	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	GLuint textureID4;
	glGenTextures(1, &textureID4);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID4);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data4);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID4 = glGetUniformLocation(programID, "myTextureSampler");

	// Read our planet.obj file
	std::vector<glm::vec3> vertices4;
	std::vector<glm::vec3> normals4;
	std::vector<glm::vec2> uvs4;
	bool res4 = loadOBJ("planet.obj", vertices4, uvs4, normals4);
	// Load it into a VBO

	GLuint vertexbuffer4;
	glGenBuffers(1, &vertexbuffer4);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer4);
	glBufferData(GL_ARRAY_BUFFER, vertices4.size() * sizeof(glm::vec3), &vertices4[0], GL_STATIC_DRAW);

	GLuint uvbuffer4;
	glGenBuffers(1, &uvbuffer4);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer4);
	glBufferData(GL_ARRAY_BUFFER, uvs4.size() * sizeof(glm::vec2), &uvs4[0], GL_STATIC_DRAW);
	glm::mat4 ModelMatrix4 = glm::mat4(1.0f);
	ModelMatrix4 = glm::translate(ModelMatrix4, vec3(38.0f, 0.0f, 0.0f)); // H GHS MAS ARXIKA EINAI STO SHMEIO (38,0,0).
	GLfloat xold_e = 38.0f;												  // ARXIKOPOIW TIS METABLHTES POY EINAI APARAITHTES GIA THN KINHSH THS GHS.
	GLfloat yold_e = 0.0f;
	GLfloat xnew_e;
	GLfloat ynew_e;
	GLfloat angle_e = 0.012f;											  // H TAXYTHTA KINHSHS THS GHS.
																						




	GLfloat angle_per= 1.5f;
	GLfloat yold_m = 80.0f;
	GLfloat BHMA = 0.0005f;
	do {
		
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);
		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;


		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
		

		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);
	
		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);
	
		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());


		
		
		// EDW KANV THN PERISTROFH TOY PLANHTH MAS GYRW APO TON HLIO.
		x2 = x1 * cos(PI * 2 * (angle / 360)) - y1 * sin(PI * 2 * (angle / 360));
		y2 = y1 * cos(PI * 2 * (angle / 360)) + x1 * sin(PI * 2 * (angle / 360));
		ModelMatrix2 = glm::translate(ModelMatrix2, vec3 (x2 - x1 , 0.0f, y2 - y1));
		glm::mat4 MVP2 = ProjectionMatrix * ViewMatrix * ModelMatrix2;	
		MVP2 = glm::rotate(MVP2, angle_per, glm::vec3(0, 1, 0));
		x1 = x2;
		y1 = y2;

		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP2[0][0]);
		
		//angle_in_radians += 0.0002f; //  GWNIA GIA TO ROTATION TOY PLANHTH
		
		if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
			BHMA = 0.0009f;
		}
		if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
			BHMA = 0.0002f;
		}
		angle_per = angle_per + BHMA;
		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID2);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID2, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer2);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer2);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// Draw the triangle for the second obj !
		glDrawArrays(GL_TRIANGLES, 0, vertices2.size());

																								


																									

		GLfloat ynew_m = yold_m - 0.02f;
		if (ynew_m < 0.0f)
		{
			ModelMatrix3 = glm::translate(ModelMatrix3, vec3(0.0f, 0.0f, 80.0f));
			yold_m = 80.0f;
			
		}
		else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			ModelMatrix3 = glm::translate(ModelMatrix3, vec3(0.0f, 0.0f, ynew_m - yold_m));	
			yold_m = ynew_m;
		}

		glm::mat4 MVP3 = ProjectionMatrix * ViewMatrix * ModelMatrix3;
		

		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP3[0][0]);
		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID3);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID3, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer3);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer3);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// Draw the triangle for the third obj !
		glDrawArrays(GL_TRIANGLES, 0, vertices3.size());





		// EDW KANV THN PERISTROFH THS GHS  GYRW APO TON HLIO. ( OPWS EKANA KAI GIA TON PLANHTH)
		xnew_e = xold_e * cos(PI * 2 * (angle_e / 360)) - yold_e * sin(PI * 2 * (angle_e / 360));
		ynew_e = yold_e * cos(PI * 2 * (angle_e / 360)) + xold_e * sin(PI * 2 * (angle_e / 360));
		ModelMatrix4 = glm::translate(ModelMatrix4, vec3(xnew_e - xold_e, 0.0f, ynew_e - yold_e));
		glm::mat4 MVP4 = ProjectionMatrix * ViewMatrix * ModelMatrix4;
		xold_e = xnew_e;
		yold_e = ynew_e;
		
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP4[0][0]);
		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID4);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID4, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer4);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer4);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// Draw the triangle for the earth!
		glDrawArrays(GL_TRIANGLES, 0, vertices4.size());
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_Q) != GLFW_PRESS || (GetKeyState(VK_CAPITAL) & 0x0001) == 0 && glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) != GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS);

	
	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &vertexbuffer2);
	glDeleteBuffers(1, &uvbuffer2);
	glDeleteBuffers(1, &vertexbuffer3);
	glDeleteBuffers(1, &uvbuffer3);
	glDeleteBuffers(1, &vertexbuffer4);
	glDeleteBuffers(1, &uvbuffer4);

	glDeleteProgram(programID);
	glDeleteTextures(1, &textureID4);
	glDeleteTextures(1, &textureID3);
	glDeleteTextures(1, &textureID2);
	glDeleteTextures(1, &textureID);
	

	glDeleteVertexArrays(1, &VertexArrayID);
	glDeleteVertexArrays(1, &VertexArrayID2);
	glDeleteVertexArrays(1, &VertexArrayID3);
	glDeleteVertexArrays(1, &VertexArrayID4);
	glfwTerminate();

	return 0;

}

