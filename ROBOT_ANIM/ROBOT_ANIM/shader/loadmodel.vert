#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 Normal;
out vec3 Position;
out vec3 crntPos;
out vec4 fragPosLight;

uniform mat4 view_matrix;
uniform mat4 proj_matrix;
uniform mat4 model_matrix;

uniform mat4 lightProjection;

void main()
{
    TexCoords = aTexCoords;
    Normal = aNormal;
    Position = (model_matrix * vec4(aPos, 1.0)).xyz;
    gl_Position = proj_matrix * view_matrix * model_matrix * vec4(aPos, 1.0);


    crntPos = Position;
    fragPosLight = lightProjection * vec4(crntPos, 1.0f);

}
