#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 Position;
in vec3 crntPos;
in vec4 fragPosLight;

uniform sampler2D texture_diffuse1;

uniform vec3 lightPos;
uniform vec3 lightIntensity;
uniform vec3 Kd;            // Diffuse reflectivity
uniform vec3 Ka;	// Ambient reflectivity 
uniform vec3 Ks;	// Specular reflectivity
uniform float specularShininess;	// Specular shininess 

uniform sampler2D shadowMap;
uniform samplerCube shadowCubeMap;
uniform vec3 camPos;

float near_plane = 0.1;
float far_plane = 1000;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));	
}



vec3 ads()
{
	vec3 n = normalize(Normal);
		
	vec3 l = normalize( vec3(lightPos) - Position);

	float df = max( dot(l,n), 0.0);
	float sf = 0.0f;

	if (df > 0.0f) 
	{
		vec3 v = normalize(vec3(Position));
		vec3 r = reflect(l, n);
		sf = max(dot(r,v), 0.0f);
	}

	return lightIntensity * ( Ka + Kd * df  + Ks );
}



vec4 pointLight()
{	
	// used in two variables so I calculate it here to not have to do it twice
	vec3 lightVec = lightPos - crntPos;

	// intensity of light with respect to distance
	float dist = length(lightVec);
	float a = 0.0003f;
	float b = 0.00002f;
	float inten = 1.0f / (a * dist * dist + b * dist + 1.0f);

	// ambient lighting
	float ambient = 0.20f;

	// diffuse lighting
	vec3 normal = normalize(Normal);
	vec3 lightDirection = normalize(lightVec);
	float diffuse = max(dot(normal, lightDirection), 0.0f);

	// specular lighting
	float specular = 0.0f;
	if (diffuse != 0.0f)
	{
		float specularLight = 0.50f;
		vec3 viewDirection = normalize(camPos - crntPos);
		vec3 halfwayVec = normalize(viewDirection + lightDirection);
		float specAmount = pow(max(dot(normal, halfwayVec), 0.0f), 16);
		specular = specAmount * specularLight;
	};

	// Shadow value
	float shadow = 0.0f;
	vec3 fragToLight = crntPos - lightPos;
	float currentDepth = length(fragToLight);
	float bias = max(0.5f * (1.0f - dot(normal, lightDirection)), 0.0005f); 

	// Not really a radius, more like half the width of a square
	int sampleRadius = 2;
	float offset = 0.02f;
	for(int z = -sampleRadius; z <= sampleRadius; z++)
	{
		for(int y = -sampleRadius; y <= sampleRadius; y++)
		{
		    for(int x = -sampleRadius; x <= sampleRadius; x++)
		    {
		        float closestDepth = texture(shadowCubeMap, fragToLight + vec3(x, y, z) * offset).r;
				// Remember that we divided by the farPlane?
				// Also notice how the currentDepth is not in the range [0, 1]
				closestDepth *= far_plane;
				if (currentDepth > closestDepth + bias)
					shadow += 1.0f;     
		    }    
		}
	}
	// Average shadow
	shadow /= pow((sampleRadius * 2 + 1), 3);

	return (texture(texture_diffuse1,TexCoords) * (diffuse * (1.0f - shadow) * inten + ambient) + texture(texture_diffuse1,TexCoords).r * specular * (1.0f - shadow) * inten) * vec4(lightIntensity,1.0f);
}



void main()
{    

    //vec3 phong = ads() * texture(texture_diffuse1,TexCoords).rgb;
    //FragColor = vec4(phong, 1.0f);

	FragColor = pointLight();
}
