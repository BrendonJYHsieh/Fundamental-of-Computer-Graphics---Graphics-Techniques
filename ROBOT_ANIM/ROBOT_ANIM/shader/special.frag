#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
uniform float time;
uniform sampler2D texture_diffuse1;

float r = 0.75;

void main()
{    
      vec2 position = TexCoords;
      float pi = 3.14;
      float distance_to_center = distance(position, vec2(r, r));
      float rotation_index = 6.0 * distance_to_center * pi * sin(time/20.0); // 6 is rotation speed

      
      mat2 rotation_matrix = mat2(vec2(sin(rotation_index), -cos(rotation_index)),
                                vec2(cos(rotation_index), sin(rotation_index)));
      position = position * rotation_matrix;

      FragColor = texture(texture_diffuse1, position);;
}
