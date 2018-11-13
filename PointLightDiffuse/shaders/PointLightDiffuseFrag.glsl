#version 330 core

/// @brief[in] the vertex normal
in vec3 fragmentNormal;
/// @brief our output fragment colour
layout (location =0)out vec4 fragColour;
// @brief light structure
struct Light
{
    vec3 position;
    vec4 diffuse;
};


uniform vec4 diffuse;

uniform Light light;

void main ()
{
    // set the output colour to black
    fragColour= vec4(0.);
    // normalize the vertex normal
    vec3 N = normalize(fragmentNormal);
    // The Light source vector
    // get the Light vector
    vec3 L = normalize(light.position);
    // calculate diffuse based on Lambert's law (L.N)
    fragColour += diffuse  *light.diffuse * dot(L, N);


}


