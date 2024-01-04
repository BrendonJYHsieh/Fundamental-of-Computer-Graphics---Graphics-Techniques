#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform int numSamples;	// Specular shininess 
uniform int render_mode;	// Specular shininess 
uniform float mosaic_resolution;	// Specular shininess 
uniform sampler2D screenTexture;
uniform sampler2D depthMap;

uniform mat4 view_matrix;
uniform mat4 proj_matrix;
uniform mat4 pre_view_matrix;
uniform mat4 pre_proj_matrix;




#define HueLevCount 6
#define SatLevCount 7
#define ValLevCount 4
float[HueLevCount] HueLevels = float[] (0.0,80.0,160.0,240.0,320.0,360.0);
float[SatLevCount] SatLevels = float[] (0.0,0.15,0.3,0.45,0.6,0.8,1.0);
float[ValLevCount] ValLevels = float[] (0.0,0.3,0.6,1.0);

vec3 RGBtoHSV( float r, float g, float b) {
   float minv, maxv, delta;
   vec3 res;

   minv = min(min(r, g), b);
   maxv = max(max(r, g), b);
   res.z = maxv;            // v
   
   delta = maxv - minv;

   if( maxv != 0.0 )
      res.y = delta / maxv;      // s
   else {
      // r = g = b = 0      // s = 0, v is undefined
      res.y = 0.0;
      res.x = -1.0;
      return res;
   }

   if( r == maxv )
      res.x = ( g - b ) / delta;      // between yellow & magenta
   else if( g == maxv )
      res.x = 2.0 + ( b - r ) / delta;   // between cyan & yellow
   else
      res.x = 4.0 + ( r - g ) / delta;   // between magenta & cyan

   res.x = res.x * 60.0;            // degrees
   if( res.x < 0.0 )
      res.x = res.x + 360.0;
      
   return res;
}

vec3 HSVtoRGB(float h, float s, float v ) {
   int i;
   float f, p, q, t;
   vec3 res;

   if( s == 0.0 ) {
      // achromatic (grey)
      res.x = v;
      res.y = v;
      res.z = v;
      return res;
   }

   h /= 60.0;         // sector 0 to 5
   i = int(floor( h ));
   f = h - float(i);         // factorial part of h
   p = v * ( 1.0 - s );
   q = v * ( 1.0 - s * f );
   t = v * ( 1.0 - s * ( 1.0 - f ) );

   switch( i ) {
      case 0:
         res.x = v;
         res.y = t;
         res.z = p;
         break;
      case 1:
         res.x = q;
         res.y = v;
         res.z = p;
         break;
      case 2:
         res.x = p;
         res.y = v;
         res.z = t;
         break;
      case 3:
         res.x = p;
         res.y = q;
         res.z = v;
         break;
      case 4:
         res.x = t;
         res.y = p;
         res.z = v;
         break;
      default:      // case 5:
         res.x = v;
         res.y = p;
         res.z = q;
         break;
   }
   
   return res;
}

float nearestLevel(float col, int mode) {
   int levCount;
   if (mode==0) levCount = HueLevCount;
   if (mode==1) levCount = SatLevCount;
   if (mode==2) levCount = ValLevCount;
   
   for (int i =0; i<levCount-1; i++ ) {
     if (mode==0) {
        if (col >= HueLevels[i] && col <= HueLevels[i+1]) {
          return HueLevels[i+1];
        }
     }
     if (mode==1) {
        if (col >= SatLevels[i] && col <= SatLevels[i+1]) {
          return SatLevels[i+1];
        }
     }
     if (mode==2) {
        if (col >= ValLevels[i] && col <= ValLevels[i+1]) {
          return ValLevels[i+1];
        }
     }
   }
}

// averaged pixel intensity from 3 color channels
float avg_intensity(vec4 pix) {
 return (pix.r + pix.g + pix.b)/3.;
}

vec4 get_pixel(vec2 coords, float dx, float dy) {
 return texture2D(screenTexture,coords + vec2(dx, dy));
}

// returns pixel color
float IsEdge(in vec2 coords){
  float dxtex = 1.0 /float(textureSize(screenTexture,0)) ;
  float dytex = 1.0 /float(textureSize(screenTexture,0));
  float pix[9];
  int k = -1;
  float delta;

  // read neighboring pixel intensities
  for (int i=-1; i<2; i++) {
   for(int j=-1; j<2; j++) {
    k++;
    pix[k] = avg_intensity(get_pixel(coords,float(i)*dxtex,
                                          float(j)*dytex));
   }
  }

  // average color differences around neighboring pixels
  delta = (abs(pix[1]-pix[7])+
          abs(pix[5]-pix[3]) +
          abs(pix[0]-pix[8])+
          abs(pix[2]-pix[6])
           )/4.;

  return clamp(5.5*delta,0.0,1.0);
}

vec3 WorldPosFromDepth(float depth, mat4 project, mat4 view) {
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(TexCoords * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = inverse(project) * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

    vec4 worldSpacePosition = inverse(view) * viewSpacePosition;

    return worldSpacePosition.xyz;
}

void main()
{ 
	if(render_mode==0){
		FragColor = texture2D(screenTexture, TexCoords);
	}
	else if(render_mode==1){
		vec2 uv = floor(TexCoords * mosaic_resolution) / mosaic_resolution;
    	FragColor = texture2D( screenTexture, uv );
	}
	else if(render_mode==2){
		vec4 colorOrg = texture2D( screenTexture, TexCoords );
		vec3 vHSV =  RGBtoHSV(colorOrg.r,colorOrg.g,colorOrg.b);
		vHSV.x = nearestLevel(vHSV.x, 0);
		vHSV.y = nearestLevel(vHSV.y, 1);
		vHSV.z = nearestLevel(vHSV.z, 2);
		float edg = IsEdge(TexCoords);
		vec3 vRGB = (edg >= 0.3)? vec3(0.0,0.0,0.0):HSVtoRGB(vHSV.x,vHSV.y,vHSV.z);
		FragColor = vec4(vRGB , 1.0);
	}
   else if(render_mode==3){
      vec2 resolution = vec2(1024,768);
      const vec4 luminance_vector = vec4(0.3, 0.59, 0.11, 0.0);
      vec2 uv = (gl_FragCoord.xy / resolution)-vec2(1.0);
      vec2 n = 1.0/resolution.xy;
      vec4 CC = texture2D(screenTexture, uv);
      vec4 RD = texture2D(screenTexture, uv + vec2( n.x, -n.y));
      vec4 RC = texture2D(screenTexture, uv + vec2( n.x,  0.0));
      vec4 RU = texture2D(screenTexture, uv + n);
      vec4 LD = texture2D(screenTexture, uv - n);
      vec4 LC = texture2D(screenTexture, uv - vec2( n.x,  0.0));
      vec4 LU = texture2D(screenTexture, uv - vec2( n.x, -n.y));
      vec4 CD = texture2D(screenTexture, uv - vec2( 0.0,  n.y));
      vec4 CU = texture2D(screenTexture, uv + vec2( 0.0,  n.y));

      FragColor = vec4(2.0*abs(length(
         vec2(
            -abs(dot(luminance_vector, RD - LD))
            +4.0*abs(dot(luminance_vector, RC - LC))
            -abs(dot(luminance_vector, RU - LU)),
            -abs(dot(luminance_vector, LD - LU))
            +4.0*abs(dot(luminance_vector, CD - CU))
            -abs(dot(luminance_vector, RD - RU))
         )
      )-0.5));
   }
   else{
      float zOverW = texture2D(depthMap, TexCoords).r;
      vec4 H = vec4(TexCoords.x * 2 - 1, (1 - TexCoords.y) * 2 - 1, zOverW, 1); 
      mat4 g_ViewProjectionInverseMatrix = inverse(proj_matrix * view_matrix); 
      vec4 D = g_ViewProjectionInverseMatrix * H;

      vec4 worldPos = D / D.w;
      vec4 currentPos = H;
      vec4 previousPos =  pre_proj_matrix * pre_view_matrix * worldPos;
      previousPos /= previousPos.w; 
      vec2 velocity = (currentPos - previousPos).xy/10.f;
      vec4 color = texture2D(screenTexture, TexCoords); 
      vec2 TexCoord = TexCoords + velocity;
      for(int i = 1; i < numSamples; ++i, TexCoord += velocity) {   
         // Sample the color buffer along the velocity vector.    
         vec4 currentColor = texture2D(screenTexture, TexCoord);   
         // Add the current color to our color sum.   
         color += currentColor; }
      FragColor = color/numSamples;
   }
	//FragColor = color / pow(mosaic, 2.0);
    //
}