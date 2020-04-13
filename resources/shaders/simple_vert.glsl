#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform vec3 LightPos;

out vec3 fragNor;
out vec3 lightDir;
out float dCo;
out vec2 vTexCoord;

void main()
{
	gl_Position = P * V * M * vertPos;
	fragNor = (M * vec4(normalize(vertNor), 0.0)).xyz;

	//compute light direction from point light to object in world (not perspective) space
	lightDir = normalize(LightPos.xyz-(M*vertPos).xyz);

	/* diffuse coefficient for a directional light */
	dCo = 1.0; //replace with computation

	/* pass through the texture coordinates to be interpolated */
	vTexCoord = vec2(vertTex.x, 1-vertTex.y);
}