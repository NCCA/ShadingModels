#version 400 core
uniform vec4 Colour;
/// @brief our output fragment colour
layout (location = 0) out vec4 fragColour;

void main ()
{
  fragColour = Colour;
}

