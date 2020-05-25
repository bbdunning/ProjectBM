#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat4 LS;
uniform vec3 LightPos;

out vec3 fragNor;
out vec3 lightDir;
out vec2 vTexCoord;

out vec3 fPos;
out vec4 fPosLS;
out vec3 vColor;

void main()
{
	gl_Position = P * V * M * vertPos;
	fragNor = (M * vec4(normalize(vertNor), 0.0)).xyz;

	//compute light direction from point light to object in world (not perspective) space
	lightDir = normalize(LightPos.xyz-(M*vertPos).xyz);

	/* pass through the texture coordinates to be interpolated */
	vTexCoord = vec2(vertTex.x, 1-vertTex.y);

	fPos = (M*vertPos).xyz;
	fPosLS = LS*M*vertPos;
	vColor = vec3(max(dot(fragNor, normalize(lightDir)), 0));
	// vColor = vec3(0.0,0.0,0.0);
	
}