#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 Position;

uniform sampler2D texture_diffuse1;

uniform vec3 lightPos;
uniform vec3 lightIntensity;
uniform vec3 Kd;            // Diffuse reflectivity
uniform vec3 Ka;	// Ambient reflectivity 
uniform vec3 Ks;	// Specular reflectivity
uniform float specularShininess;	// Specular shininess 


float near_plane = 0.1;
float far_plane = 1000;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));	
}

void main()
{    
    float depthValue = texture(texture_diffuse1, TexCoords).r;
    FragColor = vec4(vec3(LinearizeDepth(depthValue) / far_plane), 1.0);
}
