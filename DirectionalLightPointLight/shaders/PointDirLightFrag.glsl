#version 400 core

struct LightInfo
{
	// Light position in eye coords.
	vec4 position;
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



in vec3 position;
in vec3 normal;

/// @brief our output fragment colour
layout (location =0)out vec4 fragColour;


vec3 phongModel( )
{
	vec3 s;

	if( light.position.w == 0.0 )
		 s = normalize(vec3(light.position.xyz));
	 else
		 s = normalize(vec3(light.position.xyz - position));

	vec3 v = normalize(-position);
	vec3 r = reflect( -s, normal );
	vec3 ambient = light.La * material.Ka;
	float sDotN = max( dot(s,normal), 0.0 );
	vec3 diffuse = light.Ld * material.Kd * sDotN;
	vec3 spec = vec3(0.0);
	if( sDotN > 0.0 )
	{
		spec = light.Ls * material.Ks * pow( max( dot(r,v), 0.0 ), material.shininess );
	}
	return ambient + diffuse + spec;
}

vec3 phongModelHalfVector( )
{
	vec3 n = normalize(normal);
	vec3 s;

	if( light.position.w == 0.0 )
		 s = normalize(vec3(light.position.xyz));
	 else
		 s = normalize(vec3(light.position.xyz - position));

	vec3 v = normalize(-position);
	vec3 h = normalize( v + s );
	vec3 ambient = light.La * material.Ka;
	float sDotN = max( dot(s,normal), 0.0 );
	vec3 diffuse = light.Ld * material.Kd * sDotN;
	vec3 spec = vec3(0.0);
	if( sDotN > 0.0 )
	{
		spec = light.Ls * material.Ks * pow( max( dot(h,n), 0.0 ), material.shininess );
	}
	return ambient + diffuse + spec;
}

void main ()
{
	fragColour=vec4(phongModel(),1.0);
}


