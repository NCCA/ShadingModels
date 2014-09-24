#version 400 core

in vec3 frontColour;
in vec3 backColour;

/// @brief our output fragment colour
layout (location =0)out vec4 fragColour;

void main ()
{
	if( gl_FrontFacing )
	{
		fragColour=vec4(frontColour,1.0);
	}
	else
	{
		fragColour=vec4(backColour,1.0);
	}

}


