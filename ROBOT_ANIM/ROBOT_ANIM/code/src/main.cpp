// APPLICATION ENTRY POINT 


#include <iostream>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include "scene.h"


#include "controls.hpp"





#define WIN_WIDTH 1024
#define WIN_HEIGHT 768

#define MOVE_VELOCITY 0.01f
#define ROTATE_VELOCITY 0.001f

//The GLFW Window
GLFWwindow *window;

//The Scene
Scene* scene;


//To keep track of cursor location
double lastCursorPositionX, lastCursorPositionY, cursorPositionX, cursorPositionY;

/////////////////////////////////////////////////////////////////////////////////////////////
//Callback function for keypress use to toggle animate (not used at the moment)
// and to check for R to reset camera
/////////////////////////////////////////////////////////////////////////////////////////////
static void key_callback(GLFWwindow* window, int key, int cancode, int action, int mods)
{
	//if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE)
	//	if (scene)
	//		scene->animate(!(scene->animating()));
	//if (key == 'R' && action == GLFW_RELEASE)
	//		camera.reset();
}




/////////////////////////////////////////////////////////////////////////////////////////////
// Initialise 
/////////////////////////////////////////////////////////////////////////////////////////////
void initializeGL() {

    glClearColor(0.5f,0.5f,0.5f,1.0f);
    
	lastCursorPositionX=0.0;
	lastCursorPositionY=0.0;
	cursorPositionX=0.0;
	cursorPositionY=0.0;
	

	// Create the scene class and initialise it for the camera
	scene = new Scene();
    scene->initScene();

}

/////////////////////////////////////////////////////////////////////////////////////////////
// Update
/////////////////////////////////////////////////////////////////////////////////////////////
void update( float t ) 
{ 
	
	//Get the current cursor position
	glfwGetCursorPos(window,&cursorPositionX,&cursorPositionY);

	//See how much the cursor has moved
	float deltaX = (float)(lastCursorPositionX - cursorPositionX);
	float deltaY = (float)(lastCursorPositionY - cursorPositionY);


	/*
	//Using a different way (i.e. instead of callback) to check for LEFT mouse button
	if (glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_LEFT) )
	{
		//std::cout << "Left button \n";
		//Rotate the camera. The 0.001f is a velocity mofifier to make the speed sensible

	//	std::cout <<"deltaX " << deltaX << " deltaY " << deltaY << "\n";

		camera.rotate(deltaX*ROTATE_VELOCITY, deltaY*ROTATE_VELOCITY);
		
	}
	
	//Using a different way (i.e. instead of callback) to check for RIGHT mouse button
	if (glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_RIGHT) )
	{
		//std::cout << "Right button \n";
		//Rotate the camera. The 0.01f is a velocity mofifier to make the speed sensible
		camera.pan(deltaX*MOVE_VELOCITY, deltaY*MOVE_VELOCITY);

	}
	//To adjust Roll with MIDDLE mouse button
	if (glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_MIDDLE) )
	{
	
		camera.roll(deltaX* ROTATE_VELOCITY);
		
	}
	*/
		
	//Store the current cursor position
	lastCursorPositionX = cursorPositionX;
	lastCursorPositionY = cursorPositionY;
}


/////////////////////////////////////////////////////////////////////////////////////////////
// Main loop updates scene and renders until we quit
/////////////////////////////////////////////////////////////////////////////////////////////
void mainLoop() {

	long long f_startTime = glfwGetTime();

	while( ! glfwWindowShouldClose(window) && !glfwGetKey(window, GLFW_KEY_ESCAPE) ) {
		//GLUtils::checkForOpenGLError(__FILE__,__LINE__);
		
		static double fCurrentTime;

		float time_pass = glfwGetTime() - fCurrentTime;

		fCurrentTime = glfwGetTime();

		float fInterval = (fCurrentTime - (double)f_startTime);

		update(fInterval);

		scene->update(time_pass);

		scene->render();

		glfwSwapBuffers(window);
		glfwPollEvents();

		f_startTime = fCurrentTime;

	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
// resize
/////////////////////////////////////////////////////////////////////////////////////////////
void resizeGL(int w, int h ) {
    scene->resize(w,h);
}


/////////////////////////////////////////////////////////////////////////////////////////////
// main entry point
/////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
	// Initialize GLFW
	if( !glfwInit() ) exit( EXIT_FAILURE );

	// Select OpenGL 4.3 with a forward compatible core profile.
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, FALSE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, TRUE);

	// Open the window
	string title = "Skeletal Animation w/ Assimp";
	window = glfwCreateWindow( WIN_WIDTH, WIN_HEIGHT, title.c_str(), NULL, NULL );
	if( ! window ) {
		glfwTerminate();
		exit( EXIT_FAILURE );
	}
	glfwMakeContextCurrent(window);

	//Key callback
	glfwSetKeyCallback(window,key_callback);

	//Mouse callback, not used at the moment
	//glfwSetMouseButtonCallback(window,mouse_callback);


	// Load the OpenGL functions.
	//gl::exts::LoadTest didLoad = gl::sys::LoadFunctions();
	//if (!didLoad) {

	//	glfwTerminate();
	//	exit(EXIT_FAILURE);
	//}
	bool err = gladLoadGL() == 0;
	if (err)
	{
		fprintf(stderr, "Failed to initialize OpenGL loader!\n");
		return 1;
	}



	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 430"); //glsl_version可以使用字符串"#version 150"替代
	ImGuiStyle& style = ImGui::GetStyle();


	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(300, 150));

	// Initialization
	initializeGL();

	resizeGL(WIN_WIDTH,WIN_HEIGHT);


	// Enter the main loop
	mainLoop();

	// Close window and terminate GLFW
	glfwTerminate();

	delete scene;

	// Exit program
	exit( EXIT_SUCCESS );
}




