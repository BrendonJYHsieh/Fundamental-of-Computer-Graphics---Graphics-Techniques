#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 aInstanceMatrix;
layout (location = 7) in vec4 aColor; //3 + 4

uniform mat4 proj_matrix;
uniform mat4 view_matrix;

out vec2 TexCoords;
out vec4 ParticleColor;

void main()
{
    TexCoords = aTexCoords;
    ParticleColor = aColor;
    gl_Position = proj_matrix * view_matrix * aInstanceMatrix * vec4((aPos), 1.0);
}