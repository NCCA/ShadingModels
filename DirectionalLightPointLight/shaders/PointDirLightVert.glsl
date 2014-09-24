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

out vec3 position;
out vec3 normal;

void main()
{
	// Convert normal and position to eye coords
	 normal = normalize( normalMatrix * inNormal);
	 position = vec3(MV * vec4(inVert,1.0));
	// Convert position to clip coordinates and pass along
	gl_Position = MVP*vec4(inVert,1.0);
}









