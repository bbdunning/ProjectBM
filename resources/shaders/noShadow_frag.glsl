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


void main()
{
	float bias = .005;
	vec3 normal = normalize(fragNor);

	float cosAngIncidence = clamp(dot(normal, lightDir), 0, 1);
	float level = floor(cosAngIncidence * levels);
	cosAngIncidence = level/levels;
	vec3 H = normalize(lightDir-viewDirection);
	vec3 MatDif = vec3(1,1,1);

	vec4 texColor0 = texture(Texture0, vTexCoord);
  	MatDif = texColor0.xyz;
	
	float specularFactor = pow(max(dot(normal, H), 0.0), shine);
	specularFactor *= 0;

	vec3 shadeColor = vec3((MatDif * cosAngIncidence) +  //Diffuse Lighting
		(MatSpec * specularFactor * LightCol) +  //Blinn-Phong Specular
    	(MatDif * MatAmb));                   

	color = vec4(shadeColor, 1.0);
}