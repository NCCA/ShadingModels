#version 330 core
#pragma optionNV(fastmath on)
#pragma optionNV(fastprecision on)
#pragma optionNV(ifcvt none)
#pragma optionNV(inline all)
#pragma optionNV(strict on)
#pragma optionNV(unroll all)

/// @brief[in] the vertex normal
in vec3 fragmentNormal;
// the direction for our eye
in vec3 eyeDirection;

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
		vec3 direction;
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
/// @param lights passed from our program
#define numLights 3

uniform Lights light[numLights];
in vec3 halfVector[numLights];
in vec3 lightDir[numLights];

// our vertex position calculated in vert shader
in vec3 vPosition;

/// @brief a function to compute point light values
/// @param[in] _light the number of the current light

vec4 directionalLight(int _lightNum)
{

		float nDotVP;
		float nDotHV;
		float pf;
		float attenuation;
		nDotVP= max(0.0,dot(fragmentNormal, normalize(light[_lightNum].position.xyz)));
		nDotHV= max(0.0,dot(fragmentNormal,halfVector[_lightNum]));
		// the clamp below get rid of this branch
		//    if(nDotVP==0.0)
		//        pf=0.0;
		//    else
		//        pf=pow(nDotHV,material.shininess);
		float d;            // distance from surface to light position
		vec3 VP;            // direction from surface to light position

		// Compute vector from surface to light position
		VP = vec3 (light[_lightNum].position) - vPosition;

		// Compute distance between surface and light position
		d = length (VP);

		attenuation = 1.f / (light[_lightNum].constantAttenuation +
												 light[_lightNum].linearAttenuation * d +
												 light[_lightNum].quadraticAttenuation * d * d);

		pf=clamp(nDotVP,0.0,pow(nDotHV,material.shininess))* attenuation;
		vec4 ambient=light[_lightNum].ambient*material.ambient* attenuation;
		vec4 diffuse=light[_lightNum].diffuse*material.diffuse*nDotVP* attenuation;
		vec4 specular=light[_lightNum].specular*material.specular*pf* attenuation;
		return ambient + diffuse + specular;

}

/// @brief a function to compute point light values
/// @param[in] _light the number of the current light

vec4 pointLight(int _lightNum)
{
	vec3 N = normalize(fragmentNormal);
	vec3 halfV;
	float ndothv;
	float attenuation;
	vec3 E = normalize(eyeDirection);
	vec3 L = normalize(lightDir[_lightNum]);
	float lambertTerm = dot(N,L);
	vec4 diffuse=vec4(0);
	vec4 ambient=vec4(0);
	vec4 specular=vec4(0);
	if (lambertTerm > 0.0)
	{
	float d;            // distance from surface to light position
	vec3 VP;            // direction from surface to light position

	// Compute vector from surface to light position
	VP = vec3 (light[_lightNum].position) - vPosition;

	// Compute distance between surface and light position
		d = length (VP);
		attenuation = 1.f / (light[_lightNum].constantAttenuation +
											 light[_lightNum].linearAttenuation * d +
											 light[_lightNum].quadraticAttenuation * d * d);

		diffuse+=material.diffuse*light[_lightNum].diffuse*lambertTerm*attenuation;
		ambient+=material.ambient*light[_lightNum].ambient*attenuation;
		halfV = normalize(halfVector[_lightNum]);
		ndothv = max(dot(N, halfV), 0.0);
		specular+=material.specular*light[_lightNum].specular*pow(ndothv, material.shininess)* attenuation;
	}
return ambient + diffuse + specular;
}


vec4 spotLight (int _lightNum)
{
		float nDotVP;       // normal * light direction
		float nDotR;        // normal * light reflection vector
		float pf=0;           // power factor
		float spotDot;      // cosine of angle between spotlight
		float spotAttenuation;     // spotlight attenuation factor;
		float attenuation;  // computed attenuation factor
		float d;            // distance from surface to light position
		vec3 VP;            // direction from surface to light position
		vec3 reflection;    // direction of maximum highlights

		// Compute vector from surface to light position
		VP = vec3 (light[_lightNum].position) - vPosition;

		// Compute distance between surface and light position
		d = length (VP);

		// Normalize the vector from surface to light position
		VP = normalize (VP);

		// Compute attenuation
	/*	attenuation = 1.f / (light[_lightNum].constantAttenuation +
												 light[_lightNum].linearAttenuation * d +
												 light[_lightNum].quadraticAttenuation * d * d);
*/
			attenuation=1.0f;
		// See if point on surface is inside cone of illumination
		spotDot = dot (-VP, normalize (light[_lightNum].direction));

		if (spotDot < light[_lightNum].spotCosCutoff)
		{
				spotAttenuation = 0.f;
		}
		else
		{
				// we are going to ramp from the outer cone value to the inner using
				// smoothstep to create a smooth value for the falloff
				float spotValue=smoothstep(light[_lightNum].spotCosCutoff,light[_lightNum].spotCosInnerCutoff,spotDot);
				spotAttenuation = pow (spotValue, light[_lightNum].spotExponent);
		}

		// Combine the spot and distance attenuation
		attenuation *= spotAttenuation;
		// calc the reflection for the highlight
		reflection = normalize (reflect (-normalize (VP), normalize(fragmentNormal)));

		nDotVP = max (0.f, dot (fragmentNormal, VP));
		nDotR = max (0.f, dot (normalize (fragmentNormal), reflection));
// the clamp code below is the same as this but removes the branch which is quicker
//    if (nDotVP == 0.f)
//        pf = 0.f;
//    else
//        pf = pow (nDotR, material.shininess);
		pf=clamp(nDotVP,0.0,pow (nDotR, material.shininess));
		// combine the light / material values
		vec4 ambient = material.ambient * light[_lightNum].ambient * attenuation;
		vec4 diffuse = material.diffuse * light[_lightNum].diffuse * nDotVP * attenuation;
		vec4 specular = material.specular * light[_lightNum].specular * pf * attenuation;

		return ambient + diffuse + specular;
}


void main ()
{

fragColour=vec4(0.0);

for(int i=0; i<3; ++i)
{
	if(light[i].position.w == 0)
	{
		fragColour+=directionalLight(i);
	}
	else if(light[i].spotCosCutoff ==180)
	{
		fragColour+=pointLight(i);
	}
	else
	{
		fragColour+=spotLight(i);
	}

}

}

