#version 330 core
#pragma optionNV(fastmath on)
#pragma optionNV(fastprecision on)
#pragma optionNV(ifcvt none)
#pragma optionNV(inline all)
#pragma optionNV(strict on)
#pragma optionNV(unroll all)
/// @brief flag to indicate if model has unit normals if not normalize
uniform bool Normalize;
// the eye position of the camera
uniform vec3 viewerPos;
/// @brief the current fragment normal for the vert being processed
out vec3 fragmentNormal;
/// @brief the vertex passed in
layout(location =0) in vec3 inVert;
/// @brief the normal passed in
layout(location =2) in vec3 inNormal;
/// @brief the in uv
layout(location =1) in vec2 inUV;
out vec3 vPosition;
struct Materials
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
};


struct Lights
{
		vec4 position;
		vec3 direction;
		vec4 ambient;
		vec4 diffuse;
		vec4 specular;
    float spotCosCutoff;
    float spotCosInnerCutoff;
    float spotExponent;

};
// our material
uniform Materials material;
#define numLights 3
// array of lights
uniform Lights light[numLights];
// direction of the lights used for shading
out vec3 lightDir[numLights];
// direction of the lights used for shading
out vec3 halfVector[numLights];

// the direction of the eye to light
out vec3 eyeDirection;
// model view matrix calculated in the App
uniform mat4 MV;
// model view projection calculated in the app
uniform mat4 MVP;
// normal matrix calculated in the app
uniform mat3 normalMatrix;
// model matrix calculated in the app
uniform mat4 M;
// number of lights enabled
uniform mat4 V;
void main()
{
 // calculate the fragments surface normal
 fragmentNormal = (normalMatrix*inNormal);


 if (Normalize == true)
 {
	fragmentNormal = normalize(fragmentNormal);
 }
 // calculate the vertex position
 gl_Position = MVP*vec4(inVert,1.0);

 vec4 worldPosition = M * vec4(inVert, 1.0);
 eyeDirection = normalize(viewerPos - worldPosition.xyz);
 // Get vertex position in eye coordinates
 // Transform the vertex to eye co-ordinates for frag shader
 /// @brief the vertex in eye co-ordinates  homogeneous
 vec4 eyeCord=MV*vec4(inVert,1);

 vPosition = eyeCord.xyz / eyeCord.w;;

 float dist;

	for(int i=0; i<3; ++i)
	{
		lightDir[i]=vec3(light[i].position.xyz-eyeCord.xyz);
		dist = length(lightDir[i]);
		lightDir[i]/= dist;
		halfVector[i] = normalize(eyeDirection + lightDir[i]);
	}
}
