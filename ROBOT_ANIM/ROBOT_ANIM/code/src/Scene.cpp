
#include "scene.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
using std::cerr;
using std::endl;

#include "defines.h"

using glm::vec3;


#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>


#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include "model.h"

extern GLFWwindow* window;
extern Scene* scene;


bool menu_open = true;

bool in_UI = true;

float in_UI_change_time = 0.0f;

bool lbutton_down = false;

double mouse_click_xpos = 0, mouse_click_ypos = 0;

float model_roate_angle = 0.0f;

int mode = 0;

unsigned int loadCubemap(vector<std::string> faces)
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	//process mouse cursor
	if (glfwGetKey(window, GLFW_KEY_U)) {
		if (glfwGetTime() - in_UI_change_time > 0.5f) {
			in_UI = !in_UI;
			set_using_UI(in_UI);
			if (in_UI) {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
			else {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				glfwSetCursorPos(window, 1024 / 2, 768 / 2);
			}
			in_UI_change_time = glfwGetTime();
		}
	}
}


void mouse_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (GLFW_PRESS == action) {
			lbutton_down = true;
			glfwGetCursorPos(window,&mouse_click_xpos,&mouse_click_ypos);
		}
		else if (GLFW_RELEASE == action) {
			lbutton_down = false;
		}
	}
}

void scroll_callback(GLFWwindow* window, double x, double y)
{
	scene->scale += y * 0.2f;
	if (scene->scale < 0.01f) scene->scale = 0.01f;
	else if(scene->scale > 10.0f) scene->scale = 10.0f;
}


/////////////////////////////////////////////////////////////////////////////////////////////
// Default constructor
/////////////////////////////////////////////////////////////////////////////////////////////
Scene::Scene()
{
	
}

void Scene::snow_update() {
	for (int i = 0; i < snow_amount; i++) {
		if (snow_pos[i].y > 0) {
			srand(clock());
			float y = 0.5;
			snow_pos[i].y -= y;
		}
		else {
			float x = rand() % 2000 - 1000;
			float z = rand() % 2000 - 1000;
			float y = rand() % 1000 + 200;
			snow_pos[i] = glm::vec3(x, y, z);
		}
		glm::mat4 snow_transfer(1);
		snow_transfer = glm::translate(snow_transfer, snow_pos[i]);
		modelMatrices[i] = snow_transfer;
	}
}


void Scene::set_reflection_MVP(glm::mat4 P, glm::mat4 V) {
	
	if (true) {
		glm::vec3 p0 = glm::vec3(-1, 0, 1);
		glm::vec3 n = glm::vec3(0, 1, 0);

		float mA = n.x;
		float mB = n.y;
		float mC = n.z;
		float mD = -n.x * p0.x - n.y * p0.y - n.z * p0.z;;
		glm::mat4 reflectMat;
		reflectMat[0][0] = -2 * n.x * n.x + 1;
		reflectMat[0][1] = -2 * n.x * n.y;
		reflectMat[0][2] = -2 * n.x * n.z;
		reflectMat[0][3] = -2 * n.x * mD;

		reflectMat[1][0] = -2 * n.x * n.y;
		reflectMat[1][1] = -2 * n.y * n.y + 1;
		reflectMat[1][2] = -2 * n.y * n.z;
		reflectMat[1][3] = -2 * n.y * mD;

		reflectMat[2][0] = -2 * n.z * n.x;
		reflectMat[2][1] = -2 * n.z * n.y;
		reflectMat[2][2] = -2 * n.z * n.z + 1;
		reflectMat[2][3] = -2 * n.z * mD;

		reflectMat[3][0] = 0;
		reflectMat[3][1] = 0;
		reflectMat[3][2] = 0;
		reflectMat[3][3] = 1;

		glm::mat4 viewPrime = V * reflectMat;
		float fov = 2.0 * atan(1.0 / P[1][1]) * 180.0 / 3.1415926;
		glm::mat4 _projection_matrix = glm::perspective<float>(glm::radians(fov), 1.0f, 0.01, 1000.0f);
		glm::vec4 newClipPlane = glm::transpose(glm::inverse(viewPrime)) * glm::vec4(n, mD);
		glm::vec4 q = glm::vec4((glm::sign(newClipPlane.x) + _projection_matrix[2][0]) / _projection_matrix[0][0],
			(glm::sign(newClipPlane.y) + _projection_matrix[2][1]) / _projection_matrix[1][1],
			-1.0f, (1.0f + _projection_matrix[2][2]) / _projection_matrix[3][2]);
		glm::vec4 c = newClipPlane * (2.0f / glm::dot(newClipPlane, q));
		_projection_matrix[0][2] = c.x;
		_projection_matrix[1][2] = c.y;
		_projection_matrix[2][2] = c.z + 1.0f;
		_projection_matrix[3][2] = c.w;
		ProjectionMatrix = _projection_matrix;
		ViewMatrix = viewPrime;
	}

}
void Scene::set_refraction_MVP(glm::mat4 P, glm::mat4 V) {

	glm::vec3 p0 = glm::vec3(-1, 0, 1);
	glm::vec3 n = glm::vec3(0, 1, 0);
	float mA = n.x;
	float mB = n.y;
	float mC = n.z;
	float mD = -n.x * p0.x - n.y * p0.y - n.z * p0.z;;
	glm::mat4 viewPrime = V * glm::scale(glm::vec3(1, glm::clamp(1.0f - (0.3f), 0.001f, 1.0f), 1));
	float fov = 2.0 * atan(1.0 / P[1][1]) * 180.0 / 3.1415926;
	//glm::perspective(glm::radians(FoV), , 0.1f, 1000.0f);
	glm::mat4 _projection_matrix = glm::perspective<float>(glm::radians(fov), 1.0f, 0.01, 1000.0f);
	glm::vec4 newClipPlane = glm::transpose(glm::inverse(viewPrime)) * glm::vec4(n, mD);
	(&_projection_matrix[0][0])[2] = newClipPlane.x + (&_projection_matrix[0][0])[3];//x
	(&_projection_matrix[0][0])[6] = newClipPlane.y + (&_projection_matrix[0][0])[7];//y
	(&_projection_matrix[0][0])[10] = newClipPlane.z + (&_projection_matrix[0][0])[11];//z
	(&_projection_matrix[0][0])[14] = newClipPlane.w + (&_projection_matrix[0][0])[15];//w
	ProjectionMatrix = _projection_matrix;
	ViewMatrix = viewPrime;
}
void Scene::Model_init() {
	gura_model = new Model("Assets/person/gura.obj");
	plane_model = new Model("Assets/plane/plane.obj");
	swimpool_model = new Model("Assets/swimpool/swimpool.obj");
	ball_model = new Model("Assets/ball/ball.obj");
	snow_model = new Model("Assets/snow/snow.obj");
	lamp_model = new Model("Assets/Lamp/Lamp.obj");
	for (unsigned int i = 0; i < snow_amount; i++)
	{
		float x = rand() % 2000 - 1000;
		float z = rand() % 2000 - 1000;
		float y = rand() % 1000 + 200;
		snow_pos.push_back(glm::vec3(x, y, z));
	}
	modelMatrices = new glm::mat4[snow_amount];
	glGenBuffers(1, &snow_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, snow_buffer);
	glBufferData(GL_ARRAY_BUFFER, snow_amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);
	for (unsigned int i = 0; i < snow_model->meshes.size(); i++)
	{
		unsigned int VAO = snow_model->meshes[i].VAO;
		glBindVertexArray(VAO);
		// set attribute pointers for matrix (4 times vec4)
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);
		glBindVertexArray(0);
	}
}

void Scene::Texture_init() {
	height_map = new Texture2D("Assets/height_map/135.png");
}

void Scene::SkyBox_init() {
	float skyboxVertices[] = {
				-1.0f,  1.0f, -1.0f,
				-1.0f, -1.0f, -1.0f,
				 1.0f, -1.0f, -1.0f,
				 1.0f, -1.0f, -1.0f,
				 1.0f,  1.0f, -1.0f,
				-1.0f,  1.0f, -1.0f,

				-1.0f, -1.0f,  1.0f,
				-1.0f, -1.0f, -1.0f,
				-1.0f,  1.0f, -1.0f,
				-1.0f,  1.0f, -1.0f,
				-1.0f,  1.0f,  1.0f,
				-1.0f, -1.0f,  1.0f,

				 1.0f, -1.0f, -1.0f,
				 1.0f, -1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f, -1.0f,
				 1.0f, -1.0f, -1.0f,

				-1.0f, -1.0f,  1.0f,
				-1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f, -1.0f,  1.0f,
				-1.0f, -1.0f,  1.0f,

				-1.0f,  1.0f, -1.0f,
				 1.0f,  1.0f, -1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				-1.0f,  1.0f,  1.0f,
				-1.0f,  1.0f, -1.0f,

				-1.0f, -1.0f, -1.0f,
				-1.0f, -1.0f,  1.0f,
				 1.0f, -1.0f, -1.0f,
				 1.0f, -1.0f, -1.0f,
				-1.0f, -1.0f,  1.0f,
				 1.0f, -1.0f,  1.0f
	};
	glGenVertexArrays(1, &skyboxVAO);
	glBindVertexArray(skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glBindVertexArray(0);
	vector<std::string> faces
	{
		"Assets/skybox/right.jpg",
		"Assets/skybox/left.jpg",
		"Assets/skybox/top.jpg",
		"Assets/skybox/bottom.jpg",
		"Assets/skybox/front.jpg",
		"Assets/skybox/back.jpg"

	};
	cubemapTexture = loadCubemap(faces);
}

void Scene::Water_init() {
	glGenTextures(1, &reflect_texture);
	glBindTexture(GL_TEXTURE_2D, reflect_texture);

	glGenFramebuffers(1, &reflectFBO);
	glGenRenderbuffers(1, &reflectRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, reflectFBO);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 768, 0.1, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, reflect_texture, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, reflectRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1024, 768);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, reflectRBO);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);


	//refract
	glGenTextures(1, &refract_texture);
	glBindTexture(GL_TEXTURE_2D, refract_texture);

	glGenFramebuffers(1, &refractFBO);
	glGenRenderbuffers(1, &refractRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, refractFBO);


	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 768, 0.1, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, refract_texture, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, refractRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1024, 768);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, refractRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Scene::Fire_init() {

	GLuint VERTEX_BUFFER = 0;
	GLuint COLOUR_BUFFER = 1;
	GLuint TEXTURE_BUFFER = 2;
	glBindAttribLocation(particle_shader->Program, VERTEX_BUFFER, "position");
	glBindAttribLocation(particle_shader->Program, COLOUR_BUFFER, "colour");
	glBindAttribLocation(particle_shader->Program, TEXTURE_BUFFER, "texCoord");
	glLinkProgram(particle_shader->Program);
	glUseProgram(particle_shader->Program);

	GLint sampler = glGetUniformLocation(particle_shader->Program, "tex0");
	glUniform1i(sampler, 0);
}

void Scene::Shadow_init() {
	glGenFramebuffers(1, &shadowMapFBO);
	// Texture for Shadow Map FBO
	glGenTextures(1, &shadowMap);
	glBindTexture(GL_TEXTURE_2D, shadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	// Prevents darkness outside the frustrum
	float clampColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, clampColor);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
	// Needed since we don't touch the color buffer
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Matrices needed for the light's perspective
	float farPlane = 1000.0f;
	glm::mat4 orthgonalProjection = glm::ortho(-35.0f, 35.0f, -35.0f, 35.0f, 0.1f, farPlane);
	glm::mat4 perspectiveProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, farPlane);
	glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	lightProjection = perspectiveProjection * lightView;

	shadowmap_shader->Use();
	glUniformMatrix4fv(glGetUniformLocation(shadowmap_shader->Program, "lightProjection"), 1, GL_FALSE, glm::value_ptr(lightProjection));



	// Framebuffer for Cubemap Shadow Map
	glGenFramebuffers(1, &pointShadowMapFBO);

	// Texture for Cubemap Shadow Map FBO
	glGenTextures(1, &depthCubemap);

	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	for (unsigned int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
			shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, pointShadowMapFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	// Matrices needed for the light's perspective on all faces of the cubemap
	glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, farPlane);
	glm::mat4 shadowTransforms[] =
	{
	shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)),
	shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)),
	shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)),
	shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)),
	shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)),
	shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0))
	};
	// Export all matrices to shader
	shadowcubemap_shader->Use();
	glUniformMatrix4fv(glGetUniformLocation(shadowcubemap_shader->Program, "shadowMatrices[0]"), 1, GL_FALSE, glm::value_ptr(shadowTransforms[0]));
	glUniformMatrix4fv(glGetUniformLocation(shadowcubemap_shader->Program, "shadowMatrices[1]"), 1, GL_FALSE, glm::value_ptr(shadowTransforms[1]));
	glUniformMatrix4fv(glGetUniformLocation(shadowcubemap_shader->Program, "shadowMatrices[2]"), 1, GL_FALSE, glm::value_ptr(shadowTransforms[2]));
	glUniformMatrix4fv(glGetUniformLocation(shadowcubemap_shader->Program, "shadowMatrices[3]"), 1, GL_FALSE, glm::value_ptr(shadowTransforms[3]));
	glUniformMatrix4fv(glGetUniformLocation(shadowcubemap_shader->Program, "shadowMatrices[4]"), 1, GL_FALSE, glm::value_ptr(shadowTransforms[4]));
	glUniformMatrix4fv(glGetUniformLocation(shadowcubemap_shader->Program, "shadowMatrices[5]"), 1, GL_FALSE, glm::value_ptr(shadowTransforms[5]));
	glUniform3f(glGetUniformLocation(shadowcubemap_shader->Program, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
	glUniform1f(glGetUniformLocation(shadowcubemap_shader->Program, "farPlane"), farPlane);
}

void Scene::DepthTexture_init() {
	glEnable(GL_DEPTH_TEST);
	glGenFramebuffers(1, &depthMapFBO);
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1024, 768, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	//glDrawBuffer(GL_NONE);
	//glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Scene::render_DepthTexture() {
	glEnable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glUniformMatrix4fv(glGetUniformLocation(depth_shader->Program, "model_matrix"), 1, GL_FALSE, &model[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(depth_shader->Program, "view_matrix"), 1, GL_FALSE, &ViewMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(depth_shader->Program, "proj_matrix"), 1, GL_FALSE, &ProjectionMatrix[0][0]);
	render_SkyBox(depth_shader);
	render_Swimpool(depth_shader);
	render_Gura(depth_shader);
	render_Ball(depth_shader);
	//render_Test(depth_shader);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void Scene::render_SkyBox(Shader* shader) {
	glDepthFunc(GL_LEQUAL);
	shader->Use();
	glUniformMatrix4fv(glGetUniformLocation(shader->Program, "view_matrix"), 1, GL_FALSE, &glm::mat4(glm::mat3(ViewMatrix))[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader->Program, "proj_matrix"), 1, GL_FALSE, &ProjectionMatrix[0][0]);
	//skybox cube
	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS);
}

void Scene::render_PostProcess() {
	postprocess->Use();
	glBindVertexArray(quadVAO);
	glUniformMatrix4fv(glGetUniformLocation(postprocess->Program, "view_matrix"), 1, GL_FALSE, &ViewMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(postprocess->Program, "proj_matrix"), 1, GL_FALSE, &ProjectionMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(postprocess->Program, "pre_view_matrix"), 1, GL_FALSE, &Previous_ViewMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(postprocess->Program, "pre_proj_matrix"), 1, GL_FALSE, &Previous_ProjectionMatrix[0][0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);	// use the color attachment texture as the texture of the quad plane
	glUniform1i(glGetUniformLocation(postprocess->Program, "screenTexture"), 1);
	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glUniform1i(glGetUniformLocation(postprocess->Program, "depthMap"), 10);
	glUniform1i(glGetUniformLocation(postprocess->Program, "render_mode"), mode);
	glUniform1f(glGetUniformLocation(postprocess->Program, "mosaic_resolution"), mosaic_resolution);
	glUniform1i(glGetUniformLocation(postprocess->Program, "numSamples"), sample);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glActiveTexture(GL_TEXTURE12);
}

void Scene::render_reflection(glm::mat4 ProjectionMatrix, glm::mat4 ViewMatrix) {
	glBindFramebuffer(GL_FRAMEBUFFER, reflectFBO);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	set_reflection_MVP(ProjectionMatrix, ViewMatrix);
	render_objs();
}

void Scene::render_refraction(glm::mat4 ProjectionMatrix, glm::mat4 ViewMatrix) {
	glBindFramebuffer(GL_FRAMEBUFFER, refractFBO);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	set_refraction_MVP(ProjectionMatrix, ViewMatrix);
	render_objs();
	glDisable(GL_CULL_FACE);
}

void Scene::render_Test(Shader* shader) {
	shader->Use();
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(x, 10, z));
	model = glm::scale(model, glm::vec3(20, 20, 20));
	glUniformMatrix4fv(glGetUniformLocation(shader->Program, "model_matrix"), 1, GL_FALSE, &model[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader->Program, "view_matrix"), 1, GL_FALSE, &ViewMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader->Program, "proj_matrix"), 1, GL_FALSE, &ProjectionMatrix[0][0]);
	glUniform1i(glGetUniformLocation(shader->Program, "texture_diffuse1"), 10);
	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_2D, depthCubemap);
	plane_model->Draw(*shader);
}

void Scene::render_Water() {
	cameraPos = getCameraPosition();
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(73.5, 15.8, 41.7));

	water_shader->Use();
	glUniformMatrix4fv(glGetUniformLocation(water_shader->Program, "model_matrix"), 1, GL_FALSE, &model[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(water_shader->Program, "view_matrix"), 1, GL_FALSE, &ViewMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(water_shader->Program, "proj_matrix"), 1, GL_FALSE, &ProjectionMatrix[0][0]);
	glUniform3fv(glGetUniformLocation(water_shader->Program, "cameraPos"), 1, &cameraPos[0]);
	glUniform1f(glGetUniformLocation(water_shader->Program, "u_time"), counter*0.01);
	glActiveTexture(GL_TEXTURE13);
	glBindTexture(GL_TEXTURE_2D, reflect_texture);
	glUniform1i(glGetUniformLocation(water_shader->Program, "reflect_texture"), 13);
	glActiveTexture(GL_TEXTURE14);
	glBindTexture(GL_TEXTURE_2D, refract_texture);
	glUniform1i(glGetUniformLocation(water_shader->Program, "refract_texture"), 14);
	glUniform1f(glGetUniformLocation(water_shader->Program, "iTime"), counter * 0.01);
	plane_model->Draw(*water_shader);
}

void Scene::render_Shadow() {
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, shadowMapWidth, shadowMapHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, pointShadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	shadowcubemap_shader->Use();

	glUniform3f(glGetUniformLocation(shadowcubemap_shader->Program, "lightPos"), lightPos.x, lightPos.y, lightPos.z);


	// Matrices needed for the light's perspective
	float farPlane = 1000.0f;
	glm::mat4 orthgonalProjection = glm::ortho(-35.0f, 35.0f, -35.0f, 35.0f, 0.1f, farPlane);
	glm::mat4 perspectiveProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, farPlane);
	glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	lightProjection = perspectiveProjection * lightView;

	// Matrices needed for the light's perspective on all faces of the cubemap
	glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, farPlane);
	glm::mat4 shadowTransforms[] =
	{
	shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)),
	shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)),
	shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)),
	shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)),
	shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)),
	shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0))
	};
	// Export all matrices to shader
	
	glUniformMatrix4fv(glGetUniformLocation(shadowcubemap_shader->Program, "shadowMatrices[0]"), 1, GL_FALSE, glm::value_ptr(shadowTransforms[0]));
	glUniformMatrix4fv(glGetUniformLocation(shadowcubemap_shader->Program, "shadowMatrices[1]"), 1, GL_FALSE, glm::value_ptr(shadowTransforms[1]));
	glUniformMatrix4fv(glGetUniformLocation(shadowcubemap_shader->Program, "shadowMatrices[2]"), 1, GL_FALSE, glm::value_ptr(shadowTransforms[2]));
	glUniformMatrix4fv(glGetUniformLocation(shadowcubemap_shader->Program, "shadowMatrices[3]"), 1, GL_FALSE, glm::value_ptr(shadowTransforms[3]));
	glUniformMatrix4fv(glGetUniformLocation(shadowcubemap_shader->Program, "shadowMatrices[4]"), 1, GL_FALSE, glm::value_ptr(shadowTransforms[4]));
	glUniformMatrix4fv(glGetUniformLocation(shadowcubemap_shader->Program, "shadowMatrices[5]"), 1, GL_FALSE, glm::value_ptr(shadowTransforms[5]));
	glUniform3f(glGetUniformLocation(shadowcubemap_shader->Program, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
	glUniform1f(glGetUniformLocation(shadowcubemap_shader->Program, "farPlane"), farPlane);


	render_Swimpool(shadowcubemap_shader);
	render_Gura(shadowcubemap_shader);

	// Switch back to the default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// Switch back to the default viewport
	glViewport(0, 0, 1024, 768);
}

void Scene::render_Swimpool(Shader* shader) {
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0, 0, 0));
	model = glm::scale(model, glm::vec3(20, 20, 20));
	shader->Use();
	glUniformMatrix4fv(glGetUniformLocation(shader->Program, "model_matrix"), 1, GL_FALSE, &model[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader->Program, "view_matrix"), 1, GL_FALSE, &ViewMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader->Program, "proj_matrix"), 1, GL_FALSE, &ProjectionMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader->Program, "lightProjection"), 1, GL_FALSE, glm::value_ptr(lightProjection));
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	glUniform1i(glGetUniformLocation(shader->Program, "shadowCubeMap"), 2);
	glUniform3f(glGetUniformLocation(shader->Program, "camPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	swimpool_model->Draw(*shader);
}

void Scene::render_Gura(Shader* shader) {
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(60.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1, 1, 1));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
	shader->Use();
	glUniformMatrix4fv(glGetUniformLocation(shader->Program, "model_matrix"), 1, GL_FALSE, &model[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader->Program, "view_matrix"), 1, GL_FALSE, &ViewMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader->Program, "proj_matrix"), 1, GL_FALSE, &ProjectionMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader->Program, "lightProjection"), 1, GL_FALSE, glm::value_ptr(lightProjection));
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	glUniform1i(glGetUniformLocation(shader->Program, "shadowCubeMap"), 2);
	glUniform3f(glGetUniformLocation(shader->Program, "camPos"), cameraPos.x,cameraPos.y,cameraPos.z);
	gura_model->Draw(*shader);
}

void Scene::render_Lamp(Shader* shader) {
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, lightPos);
	model = glm::scale(model, glm::vec3(1, 1, 1));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
	shader->Use();
	glUniformMatrix4fv(glGetUniformLocation(shader->Program, "model_matrix"), 1, GL_FALSE, &model[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader->Program, "view_matrix"), 1, GL_FALSE, &ViewMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader->Program, "proj_matrix"), 1, GL_FALSE, &ProjectionMatrix[0][0]);
	lamp_model->Draw(*shader);
}

void Scene::render_Ball(Shader* shader) {
	shader->Use();
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(60.0f, 10.0f, -30.0f));
	model = glm::scale(model, glm::vec3(5, 5, 5));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
	glUniformMatrix4fv(glGetUniformLocation(shader->Program, "model_matrix"), 1, GL_FALSE, &model[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader->Program, "view_matrix"), 1, GL_FALSE, &ViewMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader->Program, "proj_matrix"), 1, GL_FALSE, &ProjectionMatrix[0][0]);
	glUniform1f(glGetUniformLocation(shader->Program, "time"), counter / 5.0);
	ball_model->Draw(*shader);
}

void Scene::render_snow(Shader* shader) {
	snow_update();
	glBindBuffer(GL_ARRAY_BUFFER, snow_buffer);
	glBufferData(GL_ARRAY_BUFFER, snow_amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);
	shader->Use();
	glUniformMatrix4fv(glGetUniformLocation(shader->Program, "view_matrix"), 1, GL_FALSE, &ViewMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader->Program, "proj_matrix"), 1, GL_FALSE, &ProjectionMatrix[0][0]);
	for (unsigned int i = 0; i < snow_model->meshes.size(); i++)
	{
		glBindVertexArray(snow_model->meshes[i].VAO);
		glDrawElementsInstanced(GL_TRIANGLES, snow_model->meshes[i].indices.size(), GL_UNSIGNED_INT, 0, snow_amount);
		glBindVertexArray(0);
	}
}

void Scene::render_Fire(Shader* shader) {
	static float pre_time = glfwGetTime();
	particle_shader->Use();
	glUniformMatrix4fv(glGetUniformLocation(fire->particleShader->Program, "view_matrix"), 1, GL_FALSE, &ViewMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(fire->particleShader->Program, "proj_matrix"), 1, GL_FALSE, &ProjectionMatrix[0][0]);
	fire->Update(glfwGetTime() - pre_time, 5, glm::vec3(0.0f, 1.0f, 0.0f));
	fire->Draw(particle_shader);
	pre_time = glfwGetTime();
}

void Scene::render_objs() {
	render_SkyBox(skybox_shader);
	setLightParams();
	render_Swimpool(loadmodel_shader);
	render_Gura(loadmodel_shader);
	render_Lamp(loadmodel_shader);
	render_Ball(special_shader);
	render_Fire(particle_shader);
	render_snow(snow_shader);
}

void Scene::PostProcess_init() {
	float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
			  // positions   // texCoords
			  -1.0f,  1.0f,  0.0f, 1.0f,
			  -1.0f, -1.0f,  0.0f, 0.0f,
			   1.0f, -1.0f,  1.0f, 0.0f,

			  -1.0f,  1.0f,  0.0f, 1.0f,
			   1.0f, -1.0f,  1.0f, 0.0f,
			   1.0f,  1.0f,  1.0f, 1.0f
	};
	// screen quad VAO
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glBindVertexArray(0);
	// framebuffer configuration
	// -------------------------
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	// create a color attachment texture
	glGenTextures(1, &textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 768, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
	// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1024, 768); // use a single renderbuffer object for both a depth AND stencil buffer.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
	// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
}
void Scene::PostProcess_enable() {
	// bind to framebuffer and draw scene as we normally would to color texture 
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void Scene::PostProcess_disable() {
	// now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
	// clear all relevant buffers
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
	glClear(GL_COLOR_BUFFER_BIT);
}


/////////////////////////////////////////////////////////////////////////////////////////////
//Initialise the scene
/////////////////////////////////////////////////////////////////////////////////////////////
void Scene::initScene()
{
	prog = new GLSLProgram();
	skybox = new GLSLProgram();
	Model_init();
	Texture_init();
	PostProcess_init();
	SkyBox_init();
	Water_init();
	Fire_init();
	Shadow_init();
	DepthTexture_init();

    //|Compile and link the shader  
	compileAndLinkShader();

	prog->initialiseBoneUniforms();

    glEnable(GL_DEPTH_TEST);

 	//Set up the lighting
	setLightParams();


	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	// Hide the mouse and enable unlimited mouvement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	//Set the mouse at the center of the screen
	glfwPollEvents();
	glfwSetCursorPos(window, 1024 / 2, 768 / 2);

	glfwSetKeyCallback(window, key_callback);


	glfwSetMouseButtonCallback(window, mouse_callback);

	glfwSetScrollCallback(window,scroll_callback);

	set_using_UI(in_UI);


	fire = new FireGen(100000,glm::vec3(60,0,30));

}

/////////////////////////////////////////////////////////////////////////////////////////////
//Update
/////////////////////////////////////////////////////////////////////////////////////////////
void Scene::update(float time_pass)
{
	// Vector of bone transformation matrices. 
	std::vector<Matrix4f> Transforms;
	

}


/////////////////////////////////////////////////////////////////////////////////////////////
// Set up the lighting variables in the shader
/////////////////////////////////////////////////////////////////////////////////////////////
void Scene::setLightParams()
{
	loadmodel_shader->Use();
	glUniform3fv(glGetUniformLocation(loadmodel_shader->Program, "lightIntensity"), 1, &glm::vec3(lightIntensity, lightIntensity, lightIntensity)[0]);
	glUniform3fv(glGetUniformLocation(loadmodel_shader->Program, "lightPos"), 1, &lightPos[0]);
	glUniform3fv(glGetUniformLocation(loadmodel_shader->Program, "Ka"), 1, &ambient[0]);
	glUniform3fv(glGetUniformLocation(loadmodel_shader->Program, "Kd"), 1, &diffuse[0]);
	glUniform3fv(glGetUniformLocation(loadmodel_shader->Program, "Ks"), 1, &specular[0]);
	glUniform1f(glGetUniformLocation(loadmodel_shader->Program, "Kd"), specularShininess);
}

void Scene::render_gui()
{
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	if (menu_open) {
		ImGui::Begin("Menu", &menu_open, ImGuiWindowFlags_MenuBar);
		
		if (ImGui::Button("Normal")) {
			mode = 0;
		}
		ImGui::SameLine();
		if (ImGui::Button("Mosaic")) {
			mode = 1;
		}
		ImGui::SameLine();
		if (ImGui::Button("Toon")) {
			mode = 2;
		}
		ImGui::SameLine();
		if (ImGui::Button("Sketch")) {
			mode = 3;
		}
		ImGui::SameLine();
		if (ImGui::Button("Motion Blur")) {
			mode = 4;
		}

		ImGui::SliderFloat("Mosaic_Resoultion", &mosaic_resolution, 50., 500.);
		if (ImGui::SliderFloat("Camera Speed", &speed, 0.f, 100.0f)) {
			set_camera_speed(speed);
		}
		ImGui::SliderFloat("Scale", &scale, 0.01f, 10.0f);
		ImGui::SliderInt("Sample", &sample, 1, 100);
		ImGui::SliderFloat3("lightPos", &lightPos[0], -100.0f, 100.0f);
		ImGui::SliderFloat("lightIntensity", &lightIntensity, 0.0f,3.0f);
		ImGui::SliderFloat3("ambient", &ambient[0], 0.0f, 1.0f);
		ImGui::SliderFloat3("diffuse", &diffuse[0], 0.0f, 1.0f);
		ImGui::SliderFloat3("specular", &specular[0], 0.0f, 1.0f);
		ImGui::SliderFloat("specularShininess", &specularShininess, 0.01f, 5.0f);
		ImGui::End();
	}

	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Scene::rotate_model() {
	ImGuiIO& io = ImGui::GetIO();
	if (!io.WantCaptureMouse && in_UI && lbutton_down) {
		double xpos = 0, ypos = 0;
		glfwGetCursorPos(window, &xpos, &ypos);
		model_roate_angle -= (mouse_click_xpos - xpos);
		mouse_click_xpos = xpos;
		if (model_roate_angle >= 360.0f) model_roate_angle -= 360.0f;
		else if (model_roate_angle < 0.0f) model_roate_angle += 360.0f;
	}

	model = glm::rotate(model, glm::radians(model_roate_angle), glm::vec3(0,1,0));
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Render the scene
/////////////////////////////////////////////////////////////////////////////////////////////
void Scene::render()
{
	glEnable(GL_DEPTH_TEST);
	counter++;
	setMatrices();
	glm::mat4 tmp_p = ProjectionMatrix;
	glm::mat4 tmp_v = ViewMatrix;
	
	
	render_Shadow();
	render_reflection(ProjectionMatrix, ViewMatrix);
	ProjectionMatrix = tmp_p;
	ViewMatrix = tmp_v;
	render_refraction(ProjectionMatrix, ViewMatrix);
	ProjectionMatrix = tmp_p;
	ViewMatrix = tmp_v;

	render_DepthTexture();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	PostProcess_enable();

	
	render_Water();
	render_objs();

	PostProcess_disable();

	render_PostProcess();
	render_gui();

	Previous_ProjectionMatrix = tmp_p;
	Previous_ViewMatrix = tmp_v;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//Send the MVP matrices to the GPU
/////////////////////////////////////////////////////////////////////////////////////////////
void Scene::setMatrices()
{
	computeMatricesFromInputs();
	ProjectionMatrix = getProjectionMatrix();
	ViewMatrix = getViewMatrix();
	cameraPos = glm::inverse(ViewMatrix)[3];
}

/////////////////////////////////////////////////////////////////////////////////////////////
// resize the viewport
/////////////////////////////////////////////////////////////////////////////////////////////
void Scene::resize(int w, int h)
{
    glViewport(0,0,w,h);
    width = w;
    height = h;
	//camera.setAspectRatio((float)width/height);

}

/////////////////////////////////////////////////////////////////////////////////////////////
// Compile and link the shader
/////////////////////////////////////////////////////////////////////////////////////////////
void Scene::compileAndLinkShader()
{
   
	try {
    	prog->compileShader("shader/diffuse.vert");
    	prog->compileShader("shader/diffuse.frag");
    	prog->link();
    	prog->validate();
    } catch(GLSLProgramException & e) {
 		cerr << e.what() << endl;
 		exit( EXIT_FAILURE );
    }
}






FireGen::FireGen(int Num, glm::vec3 _pos)
{
	pos = _pos;
	particles.resize(Num);
	Matrices.resize(Num);
	color.resize(Num);
	amount = Num;
	particleShader = new Shader("./shader/particle.vert",
		nullptr, nullptr, nullptr,
		"./shader/particle.frag");
	model = new Model("./assets/Fire/cone.obj");
	texture = new Texture2D("./assets/Fire/fire.png");
	glGenBuffers(1, &buffer2);
	glGenBuffers(1, &buffer);
}

unsigned int lastUsedParticle = 0;
unsigned int FireGen::firstUnusedParticle()
{
	for (unsigned int i = lastUsedParticle; i < this->amount; ++i)
	{
		if (this->particles[i].life <= 0.0f) {
			lastUsedParticle = i;
			return i;
		}
	}
	for (unsigned int i = 0; i < lastUsedParticle; ++i) {
		if (this->particles[i].life <= 0.0f) {
			lastUsedParticle = i;
			return i;
		}
	}
	lastUsedParticle = 0;
	return 0;
}

void FireGen::respawnParticle(int now, Particle& particle, glm::vec3 offset)
{
	glm::vec3 pos = this->pos;
	glm::vec3 velocity = glm::vec3{ 0, 1, 0 };
	velocity = glm::normalize(velocity);
	pos = pos + velocity * 2.0f;
	float r = R * sqrt((float)rand() / RAND_MAX);
	float theta = ((float)rand() / RAND_MAX) * 2 * PI;
	int randx = r * cos(theta);
	int randz = r * sin(theta);
	float randy = ((rand() % 51) - 25) / 25.0f;
	float rColor = 0.5f + ((rand() % 100) / 100.0f);
	particle.postion = glm::vec3(pos.x + randx, pos.y + randy, pos.z + randz);
	particle.color = glm::vec4(rColor, rColor, rColor, 1.0f);
	color[now] = particle.color;
	particle.life = 1.0f;
	this->Matrices[now] = glm::translate(glm::mat4(1.0f), particle.postion);
	this->Matrices[now] = glm::scale(this->Matrices[now], glm::vec3(particle.life, particle.life, particle.life));
	this->Matrices[now] = glm::rotate(this->Matrices[now], glm::radians((float)(rand() % 360)), glm::vec3(0.0, 1.0, 0.0));
	velocity = glm::vec3{ 0, 1, 0 };
	particle.velocity = velocity * 0.1f;
}

void FireGen::Update(float dt, unsigned int newParticles, glm::vec3 offset)
{
	for (unsigned int i = 0; i < newParticles; ++i)
	{
		int unusedParticle = firstUnusedParticle();
		respawnParticle(i, particles[unusedParticle], offset);
	}

	float r = R * 0.3f * sqrt((float)rand() / RAND_MAX);
	float theta = ((float)rand() / RAND_MAX) * 2 * PI;

	int randx = r * cos(theta);
	int randz = r * sin(theta);
	for (unsigned int i = 0; i < amount; ++i)
	{
		Particle& p = particles[i];
		p.life -= dt;
		if (p.life > 0.0f)
		{
			p.postion.y += dt * 8.0f;
			if (p.postion.x - pos.x > 0)
				p.postion.x -= 0.01f;
			else
				p.postion.x += 0.01f;
			if (p.postion.z - pos.z > 0)
				p.postion.z -= 0.01f;
			else
				p.postion.z += 0.01f;

			p.postion.x += randx * 0.015f;
			p.postion.z += randz * 0.015f;

			p.color.a -= dt * 1.0f;
			color[i] = p.color;
			this->Matrices[i] = glm::translate(glm::mat4(1.0f), p.postion);
			this->Matrices[i] = glm::scale(this->Matrices[i], glm::vec3(p.life, p.life, p.life));
			this->Matrices[i] = glm::rotate(this->Matrices[i], glm::radians((float)(rand() % 360)), glm::vec3(0.0, 1.0, 0.0));

		}
		else
		{
			this->Matrices[i] = glm::scale(this->Matrices[i], glm::vec3(0.0, 0.0, 0.0));
		}
	}
}

void FireGen::Draw(Shader* shader)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	shader->Use();
	texture->bind(0);
	glUniform1i(glGetUniformLocation(shader->Program, "tex"), 0);
	glBindBuffer(GL_ARRAY_BUFFER, this->buffer);
	glBufferData(GL_ARRAY_BUFFER, this->amount * sizeof(glm::mat4), &this->Matrices[0], GL_STATIC_DRAW);
	for (unsigned int i = 0; i < this->model->meshes.size(); i++)
	{
		unsigned int VAO = this->model->meshes[i].VAO;
		glBindVertexArray(VAO);
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);
		glBindVertexArray(0);
	}
	glBindBuffer(GL_ARRAY_BUFFER, this->buffer2);
	glBufferData(GL_ARRAY_BUFFER, this->amount * sizeof(glm::vec4), &this->color[0], GL_STATIC_DRAW);
	for (unsigned int i = 0; i < this->model->meshes.size(); i++)
	{
		unsigned int VAO = this->model->meshes[i].VAO;
		glBindVertexArray(VAO);
		glEnableVertexAttribArray(7);
		glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
		glVertexAttribDivisor(7, 1);
		glBindVertexArray(0);
	}

	for (unsigned int i = 0; i < this->model->meshes.size(); i++)
	{
		glBindVertexArray(this->model->meshes[i].VAO);
		glDrawElementsInstanced(GL_TRIANGLES, this->model->meshes[i].indices.size(), GL_UNSIGNED_INT, 0, amount);
		glBindVertexArray(0);
	}

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_BLEND);
}
