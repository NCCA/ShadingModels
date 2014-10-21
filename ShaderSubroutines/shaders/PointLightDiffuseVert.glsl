#version 330
/// @brief flag to indicate if model has unit normals if not normalize
uniform bool Normalize;
// the eye position of the camera
uniform vec3 viewerPos;
/// @brief the current fragment normal for the vert being processed
out vec3 fragmentNormal;
/// @brief the vertex passed in
layout (location =0)in vec3 inVert;
/// @brief the normal passed in
layout (location =2) in vec3 inNormal;
/// @brief the in uv
layout (location =1) in vec2 inUV;

struct Lights
{
	vec4 position;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float spotCosCutoff;
	float spotCosInnerCutoff;
	float spotExponent;
	float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation;
};
// array of lights
uniform Lights light;
// direction of the lights used for shading
out vec3 lightDir;
// out the blinn half vector
out vec3 halfVector;
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
 // Transform the vertex to eye co-ordinates for frag shader
 /// @brief the vertex in eye co-ordinates  homogeneous
 vec4 eyeCord=MV*vec4(inVert,1);

 lightDir=vec3(light.position.xyz-eyeCord.xyz);
 vec4 worldPosition = M * vec4(inVert, 1.0);
 eyeDirection = normalize(viewerPos - worldPosition.xyz);

 float dist;
 lightDir = (light.position.xyz - worldPosition.xyz);
 dist = length(lightDir);
 lightDir/= dist;
 halfVector = normalize(eyeDirection + lightDir);

}





