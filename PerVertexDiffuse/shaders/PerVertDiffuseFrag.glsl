#version 330 core

smooth in vec3 lightIntensity;
/// @brief our output fragment colour
layout (location =0)out vec4 fragColour;

void main ()
{
  fragColour=vec4(lightIntensity,1.0);
}


