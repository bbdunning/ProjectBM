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

in vec3 vertexToEye;

const float levels = 2.0;
// const vec4 _OutlineColor = vec4(0,0,0,1);
// const float _LitOutlineThickness = .8;
// const float _UnlitOutlineThickness = .8;

const int pcfCount = 1;
const float totalTexels = (pcfCount * 2.0 + 1.0) * (pcfCount * 2.0 + 1.0);

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
	float bias = .01;
	float mapSize = 4096.0;
	float texelSize = 1.0 / mapSize;
	float total = 0.0;
	vec3 shifted = 0.5 * (fPosLS.xyz + vec3(1.0));

	//calculate how SHADOW factor of fragment
	for (int x = -pcfCount; x <= pcfCount; x++) {
		for (int y = -pcfCount; y <= pcfCount; y++) {
			vec4 depth = texture(shadowDepth, vec2(shifted.x * aspectRatioX, shifted.y * aspectRatioY) + vec2(x, y) * texelSize);
			float objectNearestLight = depth.x;

			if (shifted.z > objectNearestLight + bias) {
				total += 1.0;
			}
		}
	}
	total /= totalTexels;
	total = clamp(total, 0.0, 1.0) * .5;
	float lightFactor = 1.0 - (total);  //shadowCoords.w
	

	vec3 normal = normalize(fragNor);
	float cosAngIncidence = clamp(dot(normal, lightDir), 0, 1);

	//bin light value for TOON shading
	float level = floor(cosAngIncidence * levels);
	cosAngIncidence = level/levels;

	vec3 H = normalize(lightDir-viewDirection);
	vec4 texColor0 = texture(Texture0, vTexCoord);
  	vec3 MatDif = texColor0.xyz;

	float specularFactor = max(dot(normal, H), 0.0);
	level = floor(specularFactor * levels);
	specularFactor = level/levels;
	MatDif = MatDif * lightFactor;

	vec3 shadeColor = vec3((MatDif * cosAngIncidence) +  //Diffuse Lighting
		(MatSpec * pow(specularFactor, shine) * LightCol) +  //Blinn-Phong Specular
    	(MatDif * MatAmb));                   

	color = vec4(shadeColor, 1.0);
}