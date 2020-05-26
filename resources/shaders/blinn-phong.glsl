#version 330 core 

uniform vec3 LightCol;
uniform vec3 MatAmb;
uniform mat4 V;
uniform vec3 MatSpec;
uniform vec3 viewDirection;
uniform float shine;
uniform float aspectRatioX;
uniform float aspectRatioY;

out vec4 color;

uniform sampler2D Texture0;
uniform sampler2D shadowDepth;

in vec3 fragNor;
in vec3 lightDir;
in vec2 vTexCoord;

in vec3 fPos;
in vec4 fPosLS;
in vec3 vColor;

const float levels = 3.0;

float TestShadow(vec4 LSfPos) {
  float bias = .01;

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

float getShadowMap(vec4 LSfPos) {
	vec3 shifted = 0.5 * (LSfPos.xyz + vec3(1.0));
	vec4 depth = texture(shadowDepth, vec2(shifted.x * aspectRatioX, shifted.y * aspectRatioY));
	return depth.x;
}

float getLightDist(vec4 LSfPos) {
	vec3 shifted = 0.5 * (LSfPos.xyz + vec3(1.0));
	// vec4 Ld = texture(shadowDepth, shifted.xy);
	return shifted.z;
}


void main()
{
	float bias = .005;
	vec3 normal = normalize(fragNor);

	float cosAngIncidence = clamp(dot(normal, lightDir), 0, 1);

	float level = floor(cosAngIncidence * levels);
	cosAngIncidence = level/levels;

	vec3 H = normalize(lightDir-viewDirection);
	vec4 texColor0 = texture(Texture0, vTexCoord);
  	vec3 MatDif = texColor0.xyz;

	float specularFactor = max(dot(normal, H), 0.0);
	level = floor(cosAngIncidence * levels);
	specularFactor = level/levels;

	vec3 shadeColor = vec3((MatDif * cosAngIncidence) +  //Diffuse Lighting
		(MatSpec * pow(specularFactor, shine) * LightCol) +  //Blinn-Phong Specular
    	(MatDif * MatAmb));                   

	color = vec4(shadeColor, 1.0);

	if (getLightDist(fPosLS) > getShadowMap(fPosLS) + bias) {
		color = vec4(shadeColor * .3, 1.0);
	}
}