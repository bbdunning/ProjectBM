#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform float outlineOffet;

void main()
{
    vec3 pos = vertPos.xyz;
    // vec3 scaledPos = vec3(pos + (vertNor * outlineOffset));

	// gl_Position = P * V * M * vec4(scaledPos, 1.0);
	gl_Position = P * V * M * vertPos;
}