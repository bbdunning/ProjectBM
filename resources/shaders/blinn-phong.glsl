#version 330 core 

uniform vec3 LightCol;
uniform vec3 MatAmb;
uniform mat4 V;
uniform vec3 MatSpec;
uniform vec3 viewDirection;
uniform float shine;

out vec4 color;

uniform sampler2D Texture0;
uniform sampler2D shadowDepth;

in vec3 fragNor;
in vec3 lightDir;
in vec2 vTexCoord;

in vec3 fPos;
in vec4 fPosLS;
in vec3 vColor;

float TestShadow(vec4 LSfPos) {
  float bias = .005;

	//1: shift the coordinates from -1, 1 to 0 ,1
  vec3 shifted = 0.5 * (LSfPos.xyz + vec3(1.0));
	//2: read off the stored depth (.) from the ShadowDepth, using the shifted.xy 
  vec4 Ld = texture(shadowDepth, shifted.xy);
	//3: compare to the current depth (.z) of the projected depth
	//4: return 1 if the point is shadowed
  if (Ld.x < shifted.z - bias)
    return 1.0;
  return 0.0;
}



// void main()
// {
// 	vec3 normal = normalize(fragNor);

// 	float cosAngIncidence = clamp(dot(normal, lightDir), 0, 1);
// 	vec3 H = normalize(lightDir-viewDirection);
// 	vec3 MatDif = vec3(1,1,1);

// 	vec4 texColor0 = texture(Texture0, vTexCoord);
//   	MatDif = texColor0.xyz;

// 	color = vec4((MatDif * cosAngIncidence) +  //Diffuse Lighting
// 		(MatSpec * pow(max(dot(normal, H), 0.0), shine) * LightCol) +  //Blinn-Phong Specular
//     	(MatDif * MatAmb), //Ambient Lighting
// 		1.0);                   

// 	// float Shade = TestShadow(fPosLS);
// 	if (TestShadow(fPosLS) == 1.0) {
// 		color = vec4(0.0,0.0,0.0,1.0);
// 	}
// }

float getLightDist(vec4 LSfPos) {
	vec3 shifted = 0.5 * (LSfPos.xyz + vec3(1.0));
	// vec4 Ld = texture(shadowDepth, shifted.xy);
	return shifted.z;
}

float getShadowMap(vec4 LSfPos) {
	vec3 shifted = 0.5 * (LSfPos.xyz + vec3(1.0));
	vec4 depth = texture(shadowDepth, shifted.xy);
	return depth.x;
}

void main() {

//   float Shade;
//   float amb = 0.3;

//   vec4 BaseColor = vec4(vColor, 1);
//   vec4 texColor0 = texture(Texture0, vTexCoord);

//   Shade = TestShadow(fPosLS);

//   color = amb*(texColor0) + (1.0-Shade)*texColor0*BaseColor;
//    color = amb*(texColor0) + (1.0-Shade)*texColor0;
//   color = vec4(vec3(1.0,1.0,1.0) * (1.0-Shade), 1.0);
  
//   color = vec4(vec3(1.0,1.0,1.0) * getLightDist(fPosLS), 1.0);
  color = vec4(vec3(1.0,1.0,1.0) * getShadowMap(fPosLS), 1.0);
//   if (getLightDist(fPosLS) > getShadowMap(fPosLS))
// 	color = vec4(vec3(0.0,0.0,0.0), 1.0);
//   else
// 	color = vec4(vec3(1.0,1.0,1.0), 1.0);
}
