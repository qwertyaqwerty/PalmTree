// Std. Includes
#include <string>
#include <iostream>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GL includes
#include "shader.h"
#include "camera.h"
#include "model.h"

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

// Other Libs
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//projects
#include "wind.h"

// Properties
GLuint screenWidth = 800, screenHeight = 600;

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Do_Movement();

// Camera
Camera camera(glm::vec3(0.0f, 2.0f, 10.0f));
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// The MAIN function, from here we start our application and run our Game loop
int main()
{
	// Init GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "LearnOpenGL", nullptr, nullptr); // Windowed
	glfwMakeContextCurrent(window);

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// Options
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// Initialize GLEW to setup the OpenGL Function pointers
	glewExperimental = GL_TRUE;
	glewInit();

	// Define the viewport dimensions
	glViewport(0, 0, screenWidth, screenHeight);

	// Setup some OpenGL options
	glEnable(GL_DEPTH_TEST);



	// Setup and compile our shaders
	Shader shader("shaders/bending.vs", "shaders/vegetation.frag");

	// Load models
	Model ourModel("object/palm_tree.obj");

	// Draw in wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	GLint stiffness = TextureFromFile("ENV_MP_Iraq_PlantsSansTrans_D1.jpg", string("object"), false);
	GLint leafEdge = TextureFromFile("ENV_MP_Iraq_PlantsSansTrans_D3.jpg", string("object"), false);
	GLint leafThickness = TextureFromFile("ENV_MP_Iraq_PlantsSansTrans_D4.jpg", string("object"), false);

	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		// Set frame time
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Check and call events
		glfwPollEvents();
		Do_Movement();

		// Clear the colorbuffer
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.Use();   // <-- Don't forget this one!
		// Transformation matrices
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		//std::cout << "view:" << glm::to_string(view) << std::endl;
		// Draw the loaded model

		//std::cout << "model:" << glm::to_string(model) << std::endl;

		glm::vec3 wind = getWind(currentFrame);
		glUniform1f(glGetUniformLocation(shader.Program, "time"), currentFrame);
		glUniform3fv(glGetUniformLocation(shader.Program, "wind"), 1, glm::value_ptr(wind));
		glUniform1f(glGetUniformLocation(shader.Program, "detailScale"), getDetailBendScale(currentFrame));

		glm::vec3 light(-1.0, -1.2, -10.0);
		glUniform3fv(glGetUniformLocation(shader.Program, "lightDirection"), 1, glm::value_ptr(light));

		glUniform3fv(glGetUniformLocation(shader.Program, "eyePosition"), 1, glm::value_ptr(camera.Position));

		glActiveTexture(GL_TEXTURE10);
		glUniform1i(glGetUniformLocation(shader.Program, "leafEdge"), 10);
		glBindTexture(GL_TEXTURE_2D, leafEdge);

		glActiveTexture(GL_TEXTURE11);
		glUniform1i(glGetUniformLocation(shader.Program, "leafStiffness"), 11);
		glBindTexture(GL_TEXTURE_2D, stiffness);

		glActiveTexture(GL_TEXTURE12);
		glUniform1i(glGetUniformLocation(shader.Program, "leafThickness"), 12);
		glBindTexture(GL_TEXTURE_2D, leafThickness);

		glm::mat4 model;
		model = glm::rotate(model, glm::radians(285.0f), glm::vec3(1.0f, 0.37f, 0.5f)); // Translate it down a bit so it's at the center of the scene
		model = glm::translate(model, glm::vec3(0.0f, 0.8f, 0.0f)); // Translate it down a bit so it's at the center of the scene
		model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));	// It's a bit too big for our scene, so scale it down

		for (int ridx = 0; ridx < 3; ++ridx)
			for (int idx = 0; idx < 3; ++idx)
			{
				glm::mat4 instanceModel;
				instanceModel = glm::rotate(model, glm::radians((ridx * 4 + idx) / 9.0f * 360.0f), glm::vec3(0.0f, 0.0f, 1.0f));
				instanceModel = glm::translate(instanceModel, glm::vec3(ridx * 50.0f, idx * 50.0f + ridx * 10.0f, 0.0f));
				glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(instanceModel));
				glUniform1f(glGetUniformLocation(shader.Program, "detailPhase"), (ridx * 4 + idx) * 0.3);
				ourModel.Draw(shader);
			}

		glActiveTexture(GL_TEXTURE10);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE11);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE12);
		glBindTexture(GL_TEXTURE_2D, 0);

		// Swap the buffers
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}

#pragma region "User input"

// Moves/alters the camera positions based on user input
void Do_Movement()
{
	// Camera controls
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE)
		keys[key] = false;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

#pragma endregion