//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#if defined (__APPLE__)
    #define GLFW_INCLUDE_GLCOREARB
    #define GL_SILENCE_DEPRECATION
#else
    #define GLEW_STATIC
    #include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"

#include <iostream>

int glWindowWidth = 800;
int glWindowHeight = 600;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const unsigned int SHADOW_WIDTH = 16384;
const unsigned int SHADOW_HEIGHT = 16384;
const GLfloat near_plane = 1.0f, far_plane = 100.0f;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat4 lightRotation;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;
glm::vec3 lightPointPos1;
GLuint lightPointPos1Loc;
glm::vec3 lightPointColor1;
GLuint lightPointColor1Loc;
glm::vec3 lightPointPos2;
GLuint lightPointPos2Loc;
glm::vec3 lightPointColor2;
GLuint lightPointColor2Loc;
glm::vec3 lightPointPos3;
GLuint lightPointPos3Loc;
glm::vec3 lightPointPos4;
GLuint lightPointPos4Loc;
glm::vec3 lightPointColor3;
GLuint lightPointColor3Loc;
glm::vec3 lightPointColor4;
GLuint lightPointColor4Loc;
GLuint projectileSpawnedLoc;
bool lightMode;
GLuint lightModeLoc;

gps::Camera myCamera(
				glm::vec3(0.0f, 2.0f, 5.5f), 
				glm::vec3(0.0f, 0.0f, 0.0f),
				glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 0.01f;
bool recordCamera = false;
bool replayCamera = false;
FILE* cameraHistory;

bool pressedKeys[1024];
GLfloat lightAngle;

gps::Model3D nanosuit;
gps::Model3D lightCube;
gps::Model3D screenQuad;
gps::Model3D toothless;

gps::Model3D airplaneBody;
gps::Model3D airplaneSaw;
float sawAngle = 0;
float planeRotation = 0;
glm::vec3 originalPlanePosition = glm::vec3(-78.3925f, 21.1351f, 3.2545f);
glm::vec3 shootPlanePosition = glm::vec3(-78.1147f, 21.1624f, 0.74556f);

gps::Model3D scene;
glm::mat4 sceneModel;
float sceneRotationAngle = 0.0f;

gps::Model3D scene2;
glm::mat4 sceneModel2 = glm::mat4(1.0f);
float sceneRotationAngle2 = 0.0f;

gps::Model3D scene3;
glm::mat4 sceneModel3 = glm::mat4(1.0f);
float sceneRotationAngle3 = 0.0f;

gps::Model3D scene4;
glm::mat4 sceneModel4 = glm::mat4(1.0f);
float sceneRotationAngle4 = 0.0f;

gps::Model3D fireball;
glm::vec3 projectileDir;
glm::vec3 projectilePos;
float projectileSpeed = 0.25f;
glm::vec3 projectileTargetPosition;
bool projectileSpawned = false;
bool projectileSpawnedForCamera = false;
bool shootForPlane = false;
bool shootForPlaneForCamera = false;
float lastDistance = 0.0f;

bool stopSceneRotation = true;

gps::Model3D maurice;
glm::mat4 mauriceModel;
bool mauriceFollowToggeled = true;

gps::Shader myCustomShader;
gps::Shader myCustomShaderReflect;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;
gps::Shader skyboxShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;

gps::SkyBox mySkyBoxDay;
gps::SkyBox mySkyBoxNight;

bool showDepthMap;

char showMode = 0;

float delta = 0;
float movementSpeed = 10.0f;
void updateDelta(double elapsedSeconds) {
	delta = movementSpeed * elapsedSeconds;
}
double lastTimeStamp = glfwGetTime();


GLenum glCheckError_(const char *file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);

	glfwGetFramebufferSize(window, &retina_width, &retina_height);

	myCustomShader.useShaderProgram();

	glViewport(0, 0, retina_width, retina_height);

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");

	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

void initSkybox()
{
	std::vector<const GLchar*> faces;
	faces.push_back("skyboxDay/right.tga");
	faces.push_back("skyboxDay/left.tga");
	faces.push_back("skyboxDay/top.tga");
	faces.push_back("skyboxDay/bottom.tga");
	faces.push_back("skyboxDay/back.tga");
	faces.push_back("skyboxDay/front.tga");
	mySkyBoxDay.Load(faces);

	faces.clear();
	faces.push_back("skyboxNight/right.tga");
	faces.push_back("skyboxNight/left.tga");
	faces.push_back("skyboxNight/top.tga");
	faces.push_back("skyboxNight/bottom.tga");
	faces.push_back("skyboxNight/back.tga");
	faces.push_back("skyboxNight/front.tga");
	mySkyBoxNight.Load(faces);
}

void shootPenguins()
{
	if (!projectileSpawned)
	{
		sceneModel4 = glm::rotate(glm::mat4(1.0f), glm::radians(sceneRotationAngle4), glm::vec3(0.0f, 1.0f, 0.0f));
		projectilePos = glm::mat3(sceneModel4) * lightPointPos3;

		projectileTargetPosition = glm::vec3(-1.12162f, 9.36581f, -0.770288f);
		projectileDir = glm::normalize(projectileTargetPosition - projectilePos);
		lastDistance = glm::length(projectileTargetPosition - projectilePos);
		projectileSpawned = true;
	}
}

void shootPlane()
{
	if (!projectileSpawned)
	{
		model = glm::rotate(glm::mat4(1.0f), glm::radians(planeRotation), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec3 planePosition = glm::mat3(model) * shootPlanePosition;

		projectileTargetPosition = planePosition;
		sceneModel4 = glm::rotate(glm::mat4(1.0f), glm::radians(sceneRotationAngle4), glm::vec3(0.0f, 1.0f, 0.0f));
		projectilePos = glm::mat3(sceneModel4) * lightPointPos3;

		projectileDir = glm::normalize(glm::vec3(projectileTargetPosition - projectilePos));
		lastDistance = glm::length(projectileTargetPosition - projectilePos);
		shootForPlane = true;
		projectileSpawned = true;
	}
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;

	if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
		mauriceFollowToggeled = !mauriceFollowToggeled;
	}

	if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
		stopSceneRotation = !stopSceneRotation;
	}

	if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
		lightMode = !lightMode;
		myCustomShader.useShaderProgram();
		glUniform1i(lightModeLoc, lightMode ? 1 : 0);

	}
	
	if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
		shootPenguins();
	}

	if (key == GLFW_KEY_5 && action == GLFW_PRESS) {
		shootPlane();
	}

	if (key == GLFW_KEY_6 && action == GLFW_PRESS) {
		recordCamera = !recordCamera;
		if (recordCamera)
		{
			fopen_s(&cameraHistory, "cameraMovement.txt", "w");
		}
		else
		{
			fclose(cameraHistory);
		}
	}

	if (key == GLFW_KEY_7 && action == GLFW_PRESS) {
		if (!recordCamera)
			replayCamera = !replayCamera;
		if (replayCamera)
			fopen_s(&cameraHistory, "cameraMovement.txt", "r");
		else
		{
			fclose(cameraHistory);
			myCamera.rotate(myCamera.pitch, myCamera.yaw);
		}
	}

	if (key == GLFW_KEY_E && action == GLFW_PRESS) {
		showMode++;
		if (showMode > 2)
		{
			showMode = 0;
		}
		switch (showMode)
		{
			case 0:
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				break;
			case 1:
				glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
				break;
			default:
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
	}

	if (key == GLFW_KEY_EQUAL && action == GLFW_PRESS) {
		myCamera.speed += 0.03f;
	}

	if (key == GLFW_KEY_MINUS && action == GLFW_PRESS) {
		myCamera.speed -= 0.03f;
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	if (!replayCamera)
	{
		myCamera.mouseCallback(xpos, ypos);
	}
}

void processSceneMovement()
{
	double currentTimeStamp = glfwGetTime();
	updateDelta(currentTimeStamp - lastTimeStamp);
	lastTimeStamp = currentTimeStamp;

	if (!stopSceneRotation)
	{
		sceneRotationAngle += delta;
		sceneRotationAngle2 += delta;
		sceneRotationAngle3 -= delta;
		sceneRotationAngle4 -= 2*delta;
	}

	sawAngle += 1000*delta;
	planeRotation += delta;
}

void processMovement()
{
	if (pressedKeys[GLFW_KEY_J]) {
		lightAngle -= 1.0f;
	}

	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle += 1.0f;
	}

	if (!replayCamera)
	{
		if (pressedKeys[GLFW_KEY_W]) {
			myCamera.move(gps::MOVE_FORWARD);
		}

		if (pressedKeys[GLFW_KEY_S]) {
			myCamera.move(gps::MOVE_BACKWARD);
		}

		if (pressedKeys[GLFW_KEY_A]) {
			myCamera.move(gps::MOVE_LEFT);
		}

		if (pressedKeys[GLFW_KEY_D]) {
			myCamera.move(gps::MOVE_RIGHT);
		}

		if (pressedKeys[GLFW_KEY_LEFT_CONTROL]) {
			myCamera.move(gps::MOVE_DOWN);
		}

		if (pressedKeys[GLFW_KEY_SPACE]) {
			myCamera.move(gps::MOVE_UP);
		}
	}
}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    
    //window scaling for HiDPI displays
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    //for sRBG framebuffer
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

    //for antialising
    glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);


#if not defined (__APPLE__)
    // start GLEW extension handler
    glewExperimental = GL_TRUE;
    glewInit();
#endif

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
}

void initObjects() {
	nanosuit.LoadModel("objects/nanosuit/nanosuit.obj");
	lightCube.LoadModel("objects/cube/cube.obj");
	screenQuad.LoadModel("objects/quad/quad.obj");
	scene.LoadModel("objects/scene/scene.obj");
	maurice.LoadModel("objects/maurice/maurice.obj");
	toothless.LoadModel("objects/toothless/toothless.obj");
	airplaneBody.LoadModel("objects/airplane/airplane_body.obj");
	airplaneSaw.LoadModel("objects/airplane/airplane_saw.obj");
	scene2.LoadModel("objects/scene2/scene.obj");
	scene3.LoadModel("objects/scene3/scene3.obj");
	scene4.LoadModel("objects/scene5/scene5.obj");
	fireball.LoadModel("objects/fireball/fireball.obj");
}

void initShaders() {
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();
	screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();
	depthMapShader.loadShader("shaders/depthMap.vert", "shaders/depthMap.frag");
	depthMapShader.useShaderProgram();
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	lightShader.useShaderProgram();
}

void initUniforms() {
	myCustomShader.useShaderProgram();

	lightMode = true;
	lightModeLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightMode");
	glUniform1i(lightModeLoc, 1);

	projectileSpawnedLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projectileSpawned");
	glUniform1i(projectileSpawnedLoc, 0);

	lightPointPos1 = glm::vec3(-20.8484f, 9.07315f, -2.775f);
	lightPointPos1Loc = glGetUniformLocation(myCustomShader.shaderProgram, "lightPointPosEye1");
	glUniform3fv(lightPointPos1Loc, 1, glm::value_ptr(lightPointPos1));

	lightPointColor1 = glm::vec3(0.98f, 0.49f, 0.04f);
	lightPointColor1Loc = glGetUniformLocation(myCustomShader.shaderProgram, "lightPointColor1");
	glUniform3fv(lightPointColor1Loc, 1, glm::value_ptr(lightPointColor1));

	lightPointPos2 = glm::vec3(-0.13771f, -0.410187f, -12.7916f);
	lightPointPos2Loc = glGetUniformLocation(myCustomShader.shaderProgram, "lightPointPosEye2");
	glUniform3fv(lightPointPos2Loc, 1, glm::value_ptr(lightPointPos2));

	lightPointColor2 = glm::vec3(0.22f, 0.04f, 0.8f);
	lightPointColor2Loc = glGetUniformLocation(myCustomShader.shaderProgram, "lightPointColor2");
	glUniform3fv(lightPointColor2Loc, 1, glm::value_ptr(lightPointColor2));

	lightPointPos3 = glm::vec3(-4.98285f, 18.4609f, 20.1801f);
	lightPointPos3Loc = glGetUniformLocation(myCustomShader.shaderProgram, "lightPointPosEye3");
	glUniform3fv(lightPointPos3Loc, 1, glm::value_ptr(lightPointPos3));

	lightPointColor3 = glm::vec3(0.8f, 0.18f, 0.05f);
	lightPointColor3Loc = glGetUniformLocation(myCustomShader.shaderProgram, "lightPointColor3");
	glUniform3fv(lightPointColor3Loc, 1, glm::value_ptr(lightPointColor3));

	lightPointPos4 = glm::vec3(-4.98285f, 18.4609f, 20.1801f);
	lightPointPos4Loc = glGetUniformLocation(myCustomShader.shaderProgram, "lightPointPosEye4");
	glUniform3fv(lightPointPos4Loc, 1, glm::value_ptr(lightPointPos4));

	lightPointColor4 = glm::vec3(0.8f, 0.18f, 0.05f);
	lightPointColor4Loc = glGetUniformLocation(myCustomShader.shaderProgram, "lightPointColor4");
	glUniform3fv(lightPointColor4Loc, 1, glm::value_ptr(lightPointColor4));

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	
	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(10.0f, 10.0f, 10.0f);
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");	
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	//set light color
	lightColor = glm::vec3(0.79f, 0.59f, 0.54f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	glCheckError();
}

void initFBO() {
	//TODO - Create the FBO, the depth texture and attach the depth texture to the FBO
	glGenFramebuffers(1, &shadowMapFBO);

	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture,0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix() {
	glm::mat4 lightView = glm::lookAt(glm::mat3(lightRotation) * lightDir * 5.0f, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 lightProjection = glm::ortho(-80.0f, 80.0f, -80.0f, 80.0f, near_plane, far_plane);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
	return lightSpaceTrMatrix;
}

void drawObjects(gps::Shader shader, bool depthPass) {
		
	shader.useShaderProgram();

	if (lightMode)
	{
		model = glm::mat4(1.0f);
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
			glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}

		scene.Draw(shader);

		if (mauriceFollowToggeled)
		{
			mauriceModel = glm::translate(glm::mat4(1.0f), myCamera.cameraPosition - 0.3f * myCamera.cameraFrontDirection - glm::vec3(0.0f, 0.175f, 0.0f));
			mauriceModel = glm::rotate(mauriceModel, glm::radians(180.0f + myCamera.yaw), glm::vec3(0.0f, 1.0f, 0.0f));
			mauriceModel = glm::rotate(mauriceModel, glm::radians(-myCamera.pitch), glm::vec3(1.0f, 0.0f, 0.0f));
		}
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(mauriceModel));

		// do not send the normal matrix if we are rendering in the depth map
		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
			glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}

		maurice.Draw(shader);

		model = glm::rotate(glm::mat4(1.0f), glm::radians(planeRotation), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::translate(model, originalPlanePosition);
		model = glm::rotate(model, glm::radians(sawAngle), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::translate(model, -originalPlanePosition);

		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
			glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}

		airplaneSaw.Draw(shader);

		model = glm::rotate(glm::mat4(1.0f), glm::radians(planeRotation), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
			glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}
		airplaneBody.Draw(shader);

		sceneModel2 = glm::rotate(glm::mat4(1.0f), glm::radians(sceneRotationAngle2), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec3 lightPointPos1Aux = glm::mat3(sceneModel2) * lightPointPos1;
		model = sceneModel2;
		glUniform3fv(glGetUniformLocation(shader.shaderProgram, "lightPointPosEye1"), 1, glm::value_ptr(lightPointPos1Aux));
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(sceneModel2));

		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
			glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}

		scene2.Draw(shader);

		sceneModel3 = glm::rotate(glm::mat4(1.0f), glm::radians(sceneRotationAngle3), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec3 lightPointPos2Aux = glm::mat3(sceneModel3) * lightPointPos2;
		model = sceneModel3;
		glUniform3fv(glGetUniformLocation(shader.shaderProgram, "lightPointPosEye2"), 1, glm::value_ptr(lightPointPos2Aux));
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(sceneModel3));

		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
			glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}

		scene3.Draw(shader);

		sceneModel4 = glm::rotate(glm::mat4(1.0f), glm::radians(sceneRotationAngle4), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec3 lightPointPos3Aux = glm::mat3(sceneModel4) * lightPointPos3;
		model = sceneModel4;
		glUniform3fv(glGetUniformLocation(shader.shaderProgram, "lightPointPosEye3"), 1, glm::value_ptr(lightPointPos3Aux));
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		scene4.Draw(shader);

		if (projectileSpawned)
		{
			glUniform1i(glGetUniformLocation(shader.shaderProgram, "projectileSpawned"), projectileSpawned);
			glUniform3fv(glGetUniformLocation(shader.shaderProgram, "lightPointPosEye4"), 1, glm::value_ptr(projectilePos));
			model = glm::translate(glm::mat4(1.0f), projectilePos);
			glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

			if (!depthPass) {
				normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
				glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
			}

			fireball.Draw(shader);
		}
		else
		{
			glUniform1i(glGetUniformLocation(shader.shaderProgram, "projectileSpawned"), 0);
		}
	}
	else
	{
		model = glm::mat4(1.0f);
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
			glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}

		scene.Draw(shader);

		if (mauriceFollowToggeled)
		{
			mauriceModel = glm::translate(glm::mat4(1.0f), myCamera.cameraPosition - 0.3f * myCamera.cameraFrontDirection - glm::vec3(0.0f, 0.175f, 0.0f));
			mauriceModel = glm::rotate(mauriceModel, glm::radians(180.0f + myCamera.yaw), glm::vec3(0.0f, 1.0f, 0.0f));
			mauriceModel = glm::rotate(mauriceModel, glm::radians(-myCamera.pitch), glm::vec3(1.0f, 0.0f, 0.0f));
		}
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(mauriceModel));

		// do not send the normal matrix if we are rendering in the depth map
		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
			glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}

		maurice.Draw(shader);

		model = glm::rotate(glm::mat4(1.0f), glm::radians(planeRotation), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::translate(model, originalPlanePosition);
		model = glm::rotate(model, glm::radians(sawAngle), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::translate(model, glm::vec3(78.3925f, -21.1351f, -3.2545f));

		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
			glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}

		airplaneSaw.Draw(shader);

		model = glm::rotate(glm::mat4(1.0f), glm::radians(planeRotation), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
			glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}
		airplaneBody.Draw(shader);

		sceneModel2 = glm::rotate(glm::mat4(1.0f), glm::radians(sceneRotationAngle2), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec3 lightPointPos1Aux = glm::mat3(sceneModel2) * lightPointPos1;
		model = sceneModel;
		glUniform3fv(glGetUniformLocation(shader.shaderProgram, "lightPointPosEye1"), 1, glm::value_ptr(lightPointPos1Aux));
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(sceneModel2));

		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
			glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}

		scene2.Draw(shader);

		sceneModel3 = glm::rotate(glm::mat4(1.0f), glm::radians(sceneRotationAngle3), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec3 lightPointPos2Aux = glm::mat3(sceneModel3) * lightPointPos2;
		model = sceneModel;
		glUniform3fv(glGetUniformLocation(shader.shaderProgram, "lightPointPosEye2"), 1, glm::value_ptr(lightPointPos2Aux));
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(sceneModel3));

		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
			glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}

		scene3.Draw(shader);

		sceneModel4 = glm::rotate(glm::mat4(1.0f), glm::radians(sceneRotationAngle4), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec3 lightPointPos3Aux = glm::mat3(sceneModel4) * lightPointPos3;
		model = sceneModel4;
		glUniform3fv(glGetUniformLocation(shader.shaderProgram, "lightPointPosEye3"), 1, glm::value_ptr(lightPointPos3Aux));
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(sceneModel4));

		scene4.Draw(shader);

		if (projectileSpawned)
		{
			glUniform1i(glGetUniformLocation(shader.shaderProgram, "projectileSpawned"), projectileSpawned);
			glUniform3fv(glGetUniformLocation(shader.shaderProgram, "lightPointPosEye4"), 1, glm::value_ptr(projectilePos));
			model = glm::translate(glm::mat4(1.0f), projectilePos);
			glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

			if (!depthPass) {
				normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
				glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
			}

			fireball.Draw(shader);
		}
		else
		{
			glUniform1i(glGetUniformLocation(shader.shaderProgram, "projectileSpawned"), 0);
		}
	}

	glCheckError();
}

void processProjectileMovement()
{

	if (shootForPlane)
	{
		model = glm::rotate(glm::mat4(1.0f), glm::radians(planeRotation), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec3 planePosition = glm::mat3(model) * shootPlanePosition;
		projectileTargetPosition = planePosition;
	}

	if (projectileSpawned)
	{		sceneModel4 = glm::rotate(glm::mat4(1.0f), glm::radians(sceneRotationAngle4), glm::vec3(0.0f, 1.0f, 0.0f));

			projectileDir = glm::normalize(glm::vec3(projectileTargetPosition - projectilePos));
			projectilePos += projectileDir * projectileSpeed;

			float currentLength = glm::length(projectilePos - projectileTargetPosition);
			if (currentLength > lastDistance)
			{
				projectileSpawned = false;
				shootForPlane = false;
			}
			lastDistance = currentLength;
	}
}

void renderScene() {
	// depth maps creation pass
	//TODO - Send the light-space transformation matrix to the depth map creation shader and
	//		 render the scene in the depth map

	depthMapShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	drawObjects(depthMapShader, true);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	// render depth map on screen - toggled with the M key

	if (showDepthMap) {
		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT);

		myCustomShader.useShaderProgram();
		lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

		screenQuadShader.useShaderProgram();

		//bind the depth map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);
		glDisable(GL_DEPTH_TEST);
		screenQuad.Draw(screenQuadShader);
		glEnable(GL_DEPTH_TEST);
	}
	else {

		// final scene rendering pass (with shadows)

		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myCustomShader.useShaderProgram();

		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

		//bind the shadow map
		glActiveTexture(GL_TEXTURE3);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
			1,
			GL_FALSE,
			glm::value_ptr(computeLightSpaceTrMatrix()));

		drawObjects(myCustomShader, false);

		lightMode ? mySkyBoxDay.Draw(skyboxShader, view, projection) : mySkyBoxNight.Draw(skyboxShader, view, projection);
	}
}

void cleanup() {
	glDeleteTextures(1,& depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}

void recordCameraMovement()
{
	if (!recordCamera)
		return;

	fprintf(cameraHistory, "%f %f %f ", myCamera.cameraPosition.x, myCamera.cameraPosition.y, myCamera.cameraPosition.z);
	fprintf(cameraHistory, "%f %f %f ", myCamera.cameraTarget.x, myCamera.cameraTarget.y, myCamera.cameraTarget.z);
	fprintf(cameraHistory, "%f %f %f ", myCamera.cameraUpDirection.x, myCamera.cameraUpDirection.y, myCamera.cameraUpDirection.z);
	fprintf(cameraHistory, "%f %f ", myCamera.yaw, myCamera.pitch);
	fprintf(cameraHistory, "%d %d %d\n", lightMode, projectileSpawned, shootForPlane);
}

void replayCameraMovement()
{
	if (!replayCamera)
		return;

	glm::vec3 cameraPosition, cameraTarget, cameraUpDirection;
	float yaw, pitch;
	bool lightModeAux, projectileSpawnedAux, shootForPlaneAux;
	if (fscanf_s(cameraHistory, "%f %f %f ", &cameraPosition.x, &cameraPosition.y, &cameraPosition.z) == 3 &&
		fscanf_s(cameraHistory, "%f %f %f ", &cameraTarget.x, &cameraTarget.y, &cameraTarget.z) == 3 &&
		fscanf_s(cameraHistory, "%f %f %f", &cameraUpDirection.x, &cameraUpDirection.y, &cameraUpDirection.z) == 3 &&
		fscanf_s(cameraHistory, "%f %f", &yaw, &pitch) == 2 &&
		fscanf_s(cameraHistory, "%d %d %d", &lightModeAux, &projectileSpawnedAux, &shootForPlaneAux) == 3)
	{
		myCamera.cameraPosition = cameraPosition;
		myCamera.cameraTarget = cameraTarget;
		myCamera.cameraUpDirection = cameraUpDirection;

		myCamera.yaw = yaw;
		myCamera.pitch = pitch;

		myCamera.cameraFrontDirection = glm::normalize(glm::vec3(myCamera.cameraTarget - myCamera.cameraPosition));

		myCustomShader.useShaderProgram();
		glUniform1i(lightModeLoc, lightMode ? 1 : 0);
		lightMode = lightModeAux;

		if (projectileSpawned == false && projectileSpawnedAux == true)
		{
			if (!shootForPlaneAux)
				shootPenguins();
			else
				shootPlane();
		}
	}

	if (feof(cameraHistory)) {
		replayCamera = false;
		fclose(cameraHistory);
	}

}

int main(int argc, const char * argv[]) {

	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}

	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	initFBO();
	initSkybox();


	while (!glfwWindowShouldClose(glWindow)) {
		processSceneMovement();
		processMovement();

		recordCameraMovement();

		replayCameraMovement();

		processProjectileMovement();

		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	cleanup();

	return 0;
}
