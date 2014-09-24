#version 400 core
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
out vec3 frontColour;
out vec3 backColour;
out vec2 texCoord;

void getEyeSpace(out vec3 norm, out vec4 position)
{
	norm = normalize(  normalMatrix * inNormal);
	position = MV * vec4(inVert,1.0);
}

subroutine vec3 shadeModelType( vec4 position, vec3 normal);
subroutine uniform shadeModelType shadeModel;

subroutine( shadeModelType )

vec3 phongModel( vec4 position, vec3 norm )
{
	vec3 s = normalize(vec3(light.position - position.xyz));
	vec3 v = normalize(-position.xyz);
	vec3 r = reflect( -s, norm );
	vec3 ambient = light.La * material.Ka;
	float sDotN = max( dot(s,norm), 0.0 );
	vec3 diffuse = light.Ld * material.Kd * sDotN;
	vec3 spec = vec3(0.0);
	if( sDotN > 0.0 )
	{
		spec = light.Ls * material.Ks * pow( max( dot(r,v), 0.0 ), material.shininess );
	}
	return ambient + diffuse + spec;
}

subroutine( shadeModelType )
vec3 diffuseOnly( vec4 position, vec3 norm )
{
		vec3 s = normalize( vec3(light.position - position.xyz) );
		return light.Ld * material.Kd * max( dot(s, norm), 0.0 );
}


void main()
{
	vec3 eyeNorm;
	vec4 eyePosition;
	// Get the position and normal in eye space
	getEyeSpace(eyeNorm, eyePosition);
	// Evaluate the lighting equation.
	frontColour = phongModel( eyePosition, eyeNorm );
	backColour=phongModel(eyePosition, -eyeNorm);
	// Convert position to clip coordinates and pass along
	gl_Position = MVP*vec4(inVert,1.0);
	texCoord=inUV;
}









