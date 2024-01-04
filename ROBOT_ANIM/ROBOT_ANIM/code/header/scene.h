#ifndef Scene_H
#define Scene_H
#include <string>
#include <vector>
#include <fstream>
#include <cctype>
#include <filesystem>
#include <map>
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp\Importer.hpp>
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing fla

#include "BufferObject.h"
#include "Shader.h"
#include "Texture2D.h"
#include "glslprogram.h"
#include "controls.hpp"
//#include "ParticleSystem.h"






class Model;


struct Particle
{
    glm::vec3 postion, velocity;
    glm::vec4 color;
    float life;
    Particle()
        : postion(0.0f), velocity(0.0f), color(1.0f), life(0.0f) { }
};


struct FireGen
{
    unsigned amount;
    unsigned buffer;
    unsigned buffer2;
    float R = 6.0f;
    Model* model;
    vector<Particle> particles;
    vector<glm::vec4> color;
    Shader* particleShader = nullptr;
    Texture2D* texture = nullptr;
    vector <glm::mat4> Matrices;
    glm::vec3 pos;
    FireGen(int Num, glm::vec3);
    void Draw(Shader* shader);
    unsigned int firstUnusedParticle();
    void respawnParticle(int now, Particle& particle, glm::vec3 offset);
    void Update(float dt, unsigned int newParticles, glm::vec3 offset);
};


using glm::mat4;
using std::string;
using std::vector;
using std::map;
namespace fs = std::filesystem;
class Scene
{
private:
    GLSLProgram* prog; //!< Shader program 
    


    int width, height;

	mat4 model = mat4(1.0f); // Model matrix

    void setMatrices(); //Set the camera matrices

    void compileAndLinkShader(); //Compile and link the shader



    float speed = 50.0f;
    float mosaic_resolution = 50.0f;

    vec3 lightPos = vec3(10.0f, 10.0f, 10.0f);
    float lightIntensity = 1.2f;

    vec3 ambient = vec3(0.7, 0.7, 0.7);
    vec3 diffuse = vec3(1, 1, 1);
    vec3 specular = vec3(0.2f, 0.2f, 0.2f);
    float specularShininess = 1.0f;

public:
	Scene(); //Constructor
    glm::mat4 ProjectionMatrix;
    glm::mat4 ViewMatrix;

    glm::mat4 Previous_ProjectionMatrix;
    glm::mat4 Previous_ViewMatrix;

    glm::vec3 cameraPos;

    glm::mat4 lightProjection;

    /* Shader */
    Shader* loadmodel_shader = new
        Shader(
            "shader/loadmodel.vert",
            nullptr, nullptr, nullptr,
            "shader/loadmodel.frag");

    Shader* water_shader = new
        Shader(
            "shader/water.vert",
            nullptr, nullptr, nullptr,
            "shader/water.frag");
    Shader* special_shader = new
        Shader(
            "shader/special.vert",
            nullptr, nullptr, nullptr,
            "shader/special.frag");

    Shader* depth_shader = new
        Shader(
            "shader/depth.vert",
            nullptr, nullptr, nullptr,
            "shader/depth.frag");

    Shader* depth_out_shader = new
        Shader(
            "shader/depth_out.vert",
            nullptr, nullptr, nullptr,
            "shader/depth_out.frag");
    Shader* skybox_shader = new
        Shader(
            "shader/skybox.vert",
            nullptr, nullptr, nullptr,
            "shader/skybox.frag");

    Shader* postprocess = new
        Shader(
            "shader/postprocess.vert",
            nullptr, nullptr, nullptr,
            "shader/postprocess.frag");

    Shader* particle_shader = new
        Shader(
            "shader/particle.vert",
            nullptr, nullptr, nullptr,
            "shader/particle.frag");
    

    Shader* snow_shader = new
        Shader(
            "shader/Instance.vert",
            nullptr, nullptr, nullptr,
            "shader/Instance.frag");

    Shader* shadowmap_shader = new
        Shader(
            "shader/shadowMap.vert",
            nullptr, nullptr, nullptr,
            "shader/shadowMap.frag");
    Shader* shadowcubemap_shader = new
        Shader(
            "shader/shadowCubeMap.vert",
            nullptr, nullptr, "shader/shadowCubeMap.geom",
            "shader/shadowCubeMap.frag");

    Model* snow_model = nullptr;
    unsigned int snow_amount = 10000;
    std::vector<glm::vec3>snow_pos;
    glm::mat4* modelMatrices;
    unsigned int snow_buffer;
    void snow_update();
    void render_snow(Shader*);

    /* Shader */

    void Texture_init();
    Texture2D* height_map;
    /* Water*/
    void set_reflection_MVP(glm::mat4 ProjectionMatrix, glm::mat4 ViewMatrix);
    void set_refraction_MVP(glm::mat4 ProjectionMatrix, glm::mat4 ViewMatrix);
    void render_reflection(glm::mat4 ProjectionMatrix, glm::mat4 ViewMatrix);
    void render_refraction(glm::mat4 ProjectionMatrix, glm::mat4 ViewMatrix);
    void Water_init();
    void Fire_init();
    void Shadow_init();
    void render_Water();
    void render_Shadow();
    unsigned int reflect_texture;
    unsigned int reflectFBO;
    unsigned int reflectRBO;
    unsigned int refract_texture;
    unsigned int refractFBO;
    unsigned int refractRBO;

    unsigned int shadowMapFBO;
    unsigned int shadowMap;
    unsigned int shadowMapWidth = 2048, shadowMapHeight = 2048;
    unsigned int pointShadowMapFBO;
    unsigned int depthCubemap;

    //vector<Texture2D>HeightMaps;
    int counter = 0;
    Model* plane_model;
    Model* swimpool_model;
    Model* lamp_model;
    void render_Swimpool(Shader*);
    void render_Lamp(Shader*);

    /* Depth texture */
    void DepthTexture_init();
    void render_DepthTexture();
    GLuint depthMapFBO;
    GLuint depthMap;
    /* Depth texture */

    /* Load Model*/
    void Model_init();
    Model* gura_model;
    void render_Gura(Shader*);
    void render_Ball(Shader*);
    void render_Test(Shader*);
    void render_Fire(Shader*);
   
    //ParticleSystem* particleSystem;
    FireGen* fire = nullptr;

    Model* ball_model;
    /* Load Model*/

    /* Post-processing */
    unsigned int quadVAO, quadVBO;
    unsigned int framebuffer;
    unsigned int textureColorbuffer;
    unsigned int rbo;
    void PostProcess_init();
    void PostProcess_enable();
    void PostProcess_disable();
    void render_PostProcess();
    /* Post-processing */

    float x, y, z;
    float xx, yy, zz;
    float t;
    int sample = 10;

    /* Sky Box */
    GLSLProgram* skybox; 
    GLuint cubemapTexture;
    GLuint skyboxVAO, skyboxVBO;
    void SkyBox_init();
    void render_SkyBox(Shader*);
    /* Sky Box */

    float scale = 5.0f;

    void setLightParams(); //Setup the lighting

    void setMaterial(); //Setup the lighting

    void initScene(); //Initialise the scene

    void update( float time_pass); //Update the scene

    void render();	//Render the scene

    void render_objs();

    void resize(int, int); //Resize

    void render_gui();

    void rotate_model();


};

#endif
