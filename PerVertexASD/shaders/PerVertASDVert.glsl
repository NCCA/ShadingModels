#version 330 core
// this shader is modified from the OpenGL Shading Language cookbook
/// @brief the vertex passed in
layout (location =0) in vec3 inVert;
/// @brief the normal passed in
layout (location =2) in vec3 inNormal;
/// @brief the in uv
layout( location=1)in vec2 inUV;

// transform matrix values
uniform mat4 MVP;
uniform mat3 normalMatrix;
uniform mat4 MV;

struct LightInfo
{
	// Light position in eye coords.
	vec3 position;
	// Ambient light intensity
	vec3 La;
	// Diffuse light intensity
	vec3 Ld;
	// Specular light intensity
	vec3 Ls;
};
uniform LightInfo light;

struct MaterialInfo
{
	// Ambient reflectivity
	vec3 Ka;
	// Diffuse reflectivity
	vec3 Kd;
	// Specular reflectivity
	vec3 Ks;
	// Specular shininess factor
	float shininess;
};

uniform MaterialInfo material;

// light output per vert
out vec3 lightIntensity;

void main()
{
	// Convert normal and position to eye coords
	vec3 tnorm = normalize( normalMatrix * inNormal);
	vec4 eyeCoords = MV * vec4(inVert,1.0);
	vec3 s = normalize(vec3(light.position - eyeCoords.xyz));
	vec3 v = normalize(-eyeCoords.xyz);

	vec3 r = reflect( -s, tnorm );
	vec3 ambient = light.La * material.Ka;
	float sDotN = max( dot(s,tnorm), 0.0 );
	vec3 diffuse = light.Ld * material.Kd * sDotN;
	vec3 spec = vec3(0.0);
	if( sDotN > 0.0 )
	{
		spec = light.Ls * material.Ks * pow( max( dot(r,v), 0.0 ), material.shininess );
	}
  lightIntensity = ambient+ diffuse + spec;

	// Convert position to clip coordinates and pass along
	gl_Position = MVP*vec4(inVert,1.0);
}









