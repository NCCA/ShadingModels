#version 400 core

/// @brief[in] the vertex normal
in vec3 fragmentNormal;
/// @brief our output fragment colour
layout (location =0)out vec4 fragColour;
// @brief light structure
struct Lights
{
    vec4 position;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float spotCosCutoff;
    float spotCosInnerCutoff;
    float spotExponent;
    float constantAttenuation;
    float linearAttenuation;
    float quadraticAttenuation;
};


uniform vec4 diffuse;

uniform Lights light;

void main ()
{
    // set the output colour to black
    fragColour= vec4(0.);
    // normalize the vertex normal
    vec3 N = normalize(fragmentNormal);
    // The Light source vector
    // get the Light vector
    vec3 L = normalize(light.position.xyz);
    // calculate diffuse based on Lambert's law (L.N)
    fragColour += diffuse  *light.diffuse * dot(L, N);


}


