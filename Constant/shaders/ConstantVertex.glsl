#version 330 core
uniform mat4 MVP;
/// @brief the vertex passed in
layout (location=0) in vec3 inVert;

void main()
{
	gl_Position = MVP*vec4(inVert,1);
}
