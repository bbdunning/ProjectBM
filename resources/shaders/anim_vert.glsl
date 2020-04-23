#version  330 core

const int MAX_JOINTS = 50; //max joints in a skeleton
const int MAX_WEIGHTS = 3; //max joints for a vertex

layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
layout(location = 3) in ivec3 jointIndices; //ADD ME
layout(location = 4) in vec2 jointWeights; // ADD ME 

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform vec3 LightPos;
//contains current transform for all joints in model
uniform mat4 jointTransforms[MAX_JOINTS]// ADD ME

out vec3 fragNor;
out vec3 lightDir;
out float dCo;
out vec2 vTexCoord;

void main()
{
	vec4 totalLocalPos = vec4(0.0);

	for (int i=0; i<MAX_WEIGHTS;i++) {
		mat4 jointTransform = jointTransforms[jointIndices[i]];
		vec4 posePosition = jointTransform * vertPos;
		totalLocalPos += posePosition * jointWeights[i];
	}

	gl_Position = P * V * M * totalLocalPos;
	fragNor = (M * vec4(normalize(vertNor), 0.0)).xyz;

	//compute light direction from point light to object in world (not perspective) space
	lightDir = normalize(LightPos.xyz-(M*vertPos).xyz);

	/* diffuse coefficient for a directional light */
	dCo = 1.0; //replace with computation

	/* pass through the texture coordinates to be interpolated */
	vTexCoord = vec2(vertTex.x, 1-vertTex.y);
}