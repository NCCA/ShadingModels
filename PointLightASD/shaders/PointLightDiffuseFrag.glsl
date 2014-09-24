#version 400 core

/// @brief[in] the vertex normal
in vec3 fragmentNormal;
/// @brief our output fragment colour
layout (location=0) out vec4 fragColour;

/// @brief material structure
struct Materials
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
};

// @brief light structure
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
// @param material passed from our program
uniform Materials material;

uniform Lights light;
in vec3 lightDir;
// out the blinn half vector
in vec3 halfVector;
in vec3 eyeDirection;


/// @brief a function to compute point light values
/// @param[in] _light the number of the current light
/// @param[in] _normal the current fragmentNormal
/// @param[in,out] _ambient the ambient colour to be contributed to
/// @param[in,out] _diffuse the diffuse colour to be contributed to
/// @param[in,out] _specular the specular colour to be contributed to

void pointLight(
								in vec3 _normal,
								inout vec4 _ambient,
								inout vec4 _diffuse,
								inout vec4 _specular
							)
{
	vec3 N = normalize(fragmentNormal);
	vec3 halfV;
	float ndothv;
	vec3 E = normalize(eyeDirection);
	vec3 L = normalize(lightDir);
	float lambertTerm = dot(N,L);
	if (lambertTerm > 0.0)
	{
		_diffuse+=material.diffuse*light.diffuse*lambertTerm;
		_ambient+=material.ambient*light.ambient;
		halfV = normalize(halfVector);
		ndothv = max(dot(N, halfV), 0.0);
		_specular+=material.specular*light.specular*pow(ndothv, material.shininess);
	}


}




void main ()
{

vec4 ambient=vec4(0.0);
vec4 diffuse=vec4(0.0);
vec4 specular=vec4(0.0);
pointLight(fragmentNormal,ambient,diffuse,specular);
fragColour=ambient+diffuse+specular;
}

