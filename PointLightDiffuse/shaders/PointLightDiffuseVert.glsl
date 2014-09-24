#version 400 core
/// @brief the current fragment normal for the vert being processed
out vec3 fragmentNormal;
/// @brief the vertex passed in
layout (location =0) in vec3 inVert;
/// @brief the normal passed in
layout (location =2) in vec3 inNormal;
/// @brief the in uv
layout( location=1)in vec2 inUV;

// transform matrix values
uniform mat4 MVP;
uniform mat3 normalMatrix;
out vec4 lightPos;

void main(void)
{
// calculate the fragments surface normal
 fragmentNormal = (normalMatrix*inNormal);

 // calculate the vertex position
 gl_Position = MVP*vec4(inVert,1.0);

}






