#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 Normal;
out vec3 Position;


uniform mat4 view_matrix;
uniform mat4 proj_matrix;
uniform mat4 model_matrix;

void main()
{
    TexCoords = aTexCoords;
    Normal = aNormal;
    Position = (model_matrix * vec4(aPos, 1.0)).xyz;
    gl_Position = proj_matrix * view_matrix * model_matrix * vec4(aPos, 1.0);

}
