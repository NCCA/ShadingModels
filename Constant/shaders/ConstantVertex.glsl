#version 330 core
uniform mat4 MVP;
/// @brief the vertex passed in
layout (location=0) in vec3 inVert;
layout (location=1) in vec3 inNormal;

out vec3 norm;
void main()
{
  norm = inNormal;
  gl_Position = MVP*vec4(inVert,1);
}
