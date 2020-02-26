#version 330 core 
in vec3 fragNor;
out vec4 color;

uniform sampler2D Texture0;

in vec2 vTexCoord;
in float dCo;

void main()
{
	vec3 normal = normalize(fragNor);
	// Map normal in the range [-1, 1] to color in range [0, 1];
	vec3 Ncolor = 0.5*normal + 0.5;
	color = vec4(Ncolor, 1.0);

  	//uncomment later on:  Outcolor = texColor0;
  
	Outcolor = vec4(vTexCoord.s, vTexCoord.t, 0, 1);
}
