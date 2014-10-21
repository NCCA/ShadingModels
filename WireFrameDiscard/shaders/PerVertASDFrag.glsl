#version 330 core

in vec3 frontColour;
in vec3 backColour;
in vec2 texCoord;
uniform float scale=200;
uniform float edge=0.2;
/// @brief our output fragment colour
layout (location =0)out vec4 fragColour;

void main ()
{
	bvec2 toDiscard = greaterThan( fract(texCoord * scale),vec2(edge,edge) );
	if( all(toDiscard) )
				 discard;
	if( gl_FrontFacing )
	{
		fragColour=vec4(frontColour,1.0);
	}
	else
	{
		fragColour=vec4(backColour,1.0);
	}

}
