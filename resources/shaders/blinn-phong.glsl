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

	//calculate how shadowed fragment is
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
	total = clamp(total + .2, 0.0, 1.0);
	float lightFactor = 1.0 - (total);  //shadowCoords.w
	

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
	MatDif = MatDif * lightFactor;

	vec3 shadeColor = vec3((MatDif * cosAngIncidence) +  //Diffuse Lighting
		(MatSpec * pow(specularFactor, shine) * LightCol) +  //Blinn-Phong Specular
    	(MatDif * MatAmb));                   

	color = vec4(shadeColor, 1.0);
}


//  #define SAMPLES_COUNT 64
//  #define SAMPLES_COUNT_DIV_2 32
//  #define INV_SAMPLES_COUNT (1.0f / SAMPLES_COUNT)
//  uniform sampler2D decal;   // decal texture  
//  uniform sampler3D jitter;  // jitter map    
//  uniform sampler2D spot;    // projected spotlight image  
//  uniform sampler2DShadow shadowMap;  // shadow map    
//  uniform float fwidth;  
//  uniform vec2 jxyscale;      
//  // these are passed down from vertex shader  
//  varying vec4 shadowMapPos;  
//  varying vec3 normal;  
//  varying vec2 texCoord;  
//  varying vec3 lightVec;  
//  varying vec3 view;      
//  void main(void)  
//  {    
// 	float shadow = 0;    
// 	float fsize = shadowMapPos.w * fwidth;    
// 	vec3 jcoord = vec3(gl_FragCoord.xy * jxyscale, 0);    
// 	vec4 smCoord = shadowMapPos;      
// 	// take cheap "test" samples first      
// 	for (int i = 0; i<4; i++) {      
// 		vec4 offset = texture3D(jitter, jcoord);      
// 		jcoord.z += 1.0f / SAMPLES_COUNT_DIV_2;        
// 		smCoord.xy = offset.xy * fsize + shadowMapPos;      
// 		shadow += texture2DProj(shadowMap, smCoord) / 8;        
// 		smCoord.xy = offset.zw * fsize + shadowMapPos;      
// 		shadow += texture2DProj(shadowMap, smCoord) / 8;    
// 		}    
// 	vec3 N = normalize(normal);  
// 	vec3 L = normalize(lightVec);  
// 	vec3 V = normalize(view);    
// 	vec3 R = reflect(-V, N);      
// 	// calculate diffuse dot product  
// 	float NdotL = max(dot(N, L), 0);    
// 	// if all the test samples are either zeroes or ones, or diffuse dot  
// 	// product is zero, we skip expensive shadow-map filtering  
// 	if ((shadow - 1) * shadow * NdotL != 0) {    
// 		// most likely, we are in the penumbra      
// 		shadow *= 1.0f / 8; // adjust running total      
// 		// refine our shadow estimate    
// 		for (int i = 0; i<SAMPLES_COUNT_DIV_2 - 4; i++) {      
// 			vec4 offset = texture3D(jitter, jcoord);      
// 			jcoord.z += 1.0f / SAMPLES_COUNT_DIV_2;        
// 			smCoord.xy = offset.xy * fsize + shadowMapPos;      
// 			shadow += texture2DProj(shadowMap, smCoord)* INV_SAMPLES_COUNT;        
// 			smCoord.xy = offset.zw * fsize + shadowMapPos;      
// 			shadow += texture2DProj(shadowMap, smCoord)* INV_SAMPLES_COUNT;    
// 		}  
// 	}    
// 	// all done Ð modulate lighting with the computed shadow value  
// 	vec3 color = texture2D(decal, texCoord).xyz;    
// 	gl_FragColor.xyz = (color * NdotL + pow(max(dot(R, L), 0), 64)) *                      
// 		shadow * texture2DProj(spot, shadowMapPos) +   
// 		color * 0.1;  
// 	} 