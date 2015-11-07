#version 400 core
// this shader is modified from the OpenGL Shading Language cookbook
/// @brief the vertex passed in
layout (location =0) in vec3 inVert;
/// @brief the normal passed in
layout (location =2) in vec3 inNormal;
/// @brief the in uv
layout( location=1)in vec2 inUV;

#define numLights 14


layout (std140) uniform material
{
	// Ambient reflectivity
	vec3 Ka;
	// Diffuse reflectivity
	vec3 Kd;
	// Specular reflectivity
	vec3 Ks;
	// Specular shininess factor
	float shininess;
}mat;

layout (std140) uniform  light
{
	// Light position in eye coords.
	vec3 position;
	// Ambient light intensity
	vec3 La;
	// Diffuse light intensity
	vec3 Ld;
	// Specular light intensity
	vec3 Ls;
}light[numLights];


layout (std140) uniform transforms
{
	mat4 MVP;
	mat4 MV;
	mat3 normalMatrix;
};



// light output per vert
out vec3 lightIntensity;

void getEyeSpace(out vec3 norm, out vec4 position)
{
	norm = normalize(  normalMatrix * inNormal);
	position = MV * vec4(inVert,1.0);
}


vec3 phongModel( int lightNum,vec4 position, vec3 norm )
{
	vec3 s = normalize(vec3(light[lightNum].position - position.xyz));
	vec3 v = normalize(-position.xyz);
	vec3 r = reflect( -s, norm );
	vec3 ambient = light[lightNum].La * mat.Ka;
	float sDotN = max( dot(s,norm), 0.0 );
	vec3 diffuse = light[lightNum].Ld * mat.Kd * sDotN;
	vec3 spec = vec3(0.0);
	if( sDotN > 0.0 )
	{
		spec = light[lightNum].Ls * mat.Ks * pow( max( dot(r,v), 0.0 ), mat.shininess );
	}
	return ambient + diffuse + spec;
}

void main()
{
	vec3 eyeNorm;
	vec4 eyePosition;
	// Get the position and normal in eye space
	getEyeSpace(eyeNorm, eyePosition);
	// Evaluate the lighting equation.
	lightIntensity=vec3(0.0);
	for(int i=0; i<numLights; ++i)
	{
		lightIntensity += phongModel( i,eyePosition, eyeNorm );
	}
	//lightIntensity=light[0].La;
	// Convert position to clip coordinates and pass along
	gl_Position = MVP*vec4(inVert,1.0);

}









