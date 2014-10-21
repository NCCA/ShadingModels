#version 330 core

/// @brief[in] the vertex normal
in vec3 fragmentNormal;
/// @brief our output fragment colour
layout (location =0) out vec4 fragColour;

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

void directionalLight(
												in vec3 _normal,
												inout vec4 _ambient,
												inout vec4 _diffuse,
												inout vec4 _specular
											)
{

		float nDotVP;
		float nDotHV;
		float pf;

		nDotVP= max(0.0,dot(_normal, normalize(light.position.xyz)));
		nDotHV= max(0.0,dot(_normal,halfVector));
		// the clamp below get rid of this branch
		//    if(nDotVP==0.0)
		//        pf=0.0;
		//    else
		//        pf=pow(nDotHV,material.shininess);

		pf=clamp(nDotVP,0.0,pow(nDotHV,material.shininess));
		_ambient+=light.ambient*material.ambient;
		_diffuse+=light.diffuse*material.diffuse*nDotVP;
		_specular+=light.specular*material.specular*pf;
}




void main ()
{
vec4 ambient=vec4(0.0);
vec4 diffuse=vec4(0.0);
vec4 specular=vec4(0.0);
directionalLight(fragmentNormal,ambient,diffuse,specular);
fragColour=ambient+diffuse+specular;
}

