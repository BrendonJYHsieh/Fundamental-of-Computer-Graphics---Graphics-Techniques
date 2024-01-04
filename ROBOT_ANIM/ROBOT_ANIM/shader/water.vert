#version 430 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texture_coordinate;


uniform mat4 view_matrix;
uniform mat4 proj_matrix;
uniform mat4 model_matrix;
uniform sampler2D u_heightMap;
uniform float u_time;

out V_OUT
{
   vec3 position;
   vec3 normal;
   vec2 texture_coordinate;
   vec4 screenCoord;
} v_out;



void main()
{ 
    gl_Position = proj_matrix * view_matrix * model_matrix * vec4(position, 1.0f);
    v_out.position = vec3(model_matrix * vec4(position, 1.0));
    v_out.texture_coordinate = texture_coordinate;
    vec4 o = proj_matrix * view_matrix * model_matrix * vec4(position, 1.0f);;
    o = o*0.5;
    v_out.normal = normal;
    v_out.screenCoord.xy = o.xy+o.w;
    v_out.screenCoord.zw = o.zw*2;
}