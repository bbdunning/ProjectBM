#version 330 core 
in vec3 fragNor;
in vec3 lightDir;

uniform vec3 LightCol;
uniform vec3 MatAmb;
uniform mat4 V;
uniform vec3 MatSpec;
uniform vec3 viewDirection;
uniform float shine;

out vec4 color;

uniform sampler2D Texture0;

in vec2 vTexCoord;
in float dCo;

void main()
{
	vec3 normal = normalize(fragNor);

	float cosAngIncidence = clamp(dot(normal, lightDir), 0, 1);
	vec3 H = normalize(lightDir-viewDirection);
	vec3 MatDif = vec3(1,1,1);

	vec4 texColor0 = texture(Texture0, vTexCoord);
  	MatDif = texColor0.xyz;

	color = vec4((MatDif * cosAngIncidence) +  //Diffuse Lighting
		(MatSpec * pow(max(dot(normal, H), 0.0), shine) * LightCol) +  //Blinn-Phong Specular
    	(MatDif * MatAmb), //Ambient Lighting
		1.0);                   
}