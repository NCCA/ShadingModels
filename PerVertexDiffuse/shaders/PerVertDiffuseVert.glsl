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
uniform vec3 lightPos; // Light position in eye coords.
uniform vec3 Kd;           // Diffuse reflectivity
uniform vec3 Ld;            // Light source intensity
// light output per vert
smooth out vec3 lightIntensity;

void main()
{
	// Convert normal and position to eye coords
	vec3 tnorm = normalize( normalMatrix * inNormal);
	vec4 eyeCoords = MV * vec4(inVert,1.0);
	vec3 s = normalize(vec3(lightPos - eyeCoords.xyz));
	// The diffuse shading equation
	lightIntensity = Ld * Kd * max( dot( s, tnorm ), 0.0 );
	// Convert position to clip coordinates and pass along
	gl_Position = MVP*vec4(inVert,1.0);
}









