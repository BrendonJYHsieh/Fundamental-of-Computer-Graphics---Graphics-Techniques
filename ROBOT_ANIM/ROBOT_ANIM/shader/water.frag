#version 430 core
out vec4 f_color;

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
};

struct PointLight {    
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;  

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  

struct Material
{
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};


in V_OUT
{
   vec3 position;
   vec3 normal;
   vec2 texture_coordinate;
   vec4 screenCoord;
} f_in;
 


uniform vec3 cameraPos;


uniform bool reflect_enable;
uniform bool refract_enable;

uniform sampler2D reflect_texture;
uniform sampler2D refract_texture;

uniform float u_time;

void main()
{   
    vec3 norm = -normalize(cross(dFdy(f_in.position),dFdx(f_in.position)));
    vec3 result={0.0,0.0,0.0};
    vec3 viewDir = normalize(cameraPos - f_in.position);

    if(true){
        float FresnelBase = 0.0;
		float FresnelScale = 10.0;
		float FresnelPower = 6.0;

        vec2 coord = (f_in.screenCoord.xy/f_in.screenCoord.w+(f_in.position.y/40));
        vec2 cp = -1.0 + 2.0 * f_in.texture_coordinate;
        float cl = length(cp);
        vec2 uv =  coord + (cp / cl) * cos(cl * 12.0 - u_time * 4.0) * 0.02;

        vec4 reflectColor = texture(reflect_texture, uv);
        vec4 refractColor = texture(refract_texture, uv);
        float fresnel = 0.0;
        if((-viewDir).y<0)
        {				
            fresnel = clamp( FresnelBase + FresnelScale * pow(1 - dot(f_in.normal, viewDir), FresnelPower), 0.0, 1.0);
        }
        else
        {				
            fresnel = clamp( FresnelBase + FresnelScale * pow(1 - dot(-f_in.normal, viewDir), FresnelPower), 0.0, 1.0);
        }
        //
        vec4 base = {0.0,0.0,0.05,1.0};
        f_color = refractColor*(1-fresnel)+reflectColor*fresnel;

        //f_color = refractColor*0.8+base*0.2;
        //f_color = reflectColor*0.8+base*0.2;
        // if(reflect_enable&&refract_enable){
        //    f_color = refractColor*(1-fresnel)+reflectColor*fresnel;
        // }
        // else if(reflect_enable){
        //    f_color = reflectColor*0.8+base*0.2;
        // }
        // else if(refract_enable){
        //    f_color = refractColor*0.8+base*0.2;
        // }
        // else{
        //     f_color = refractColor*(1-fresnel)+reflectColor*fresnel;
        // }
    }
}