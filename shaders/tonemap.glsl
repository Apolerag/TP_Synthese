
#version 330

#ifdef VERTEX_SHADER
out vec2 vertex_texcoord;

void main(void)
{
    vec3 quad[4]= vec3[4]( 
        vec3(-1, -1, -1), vec3( 1, -1, -1), 
        vec3(-1,  1, -1), vec3( 1,  1, -1) );
    
    gl_Position= vec4(quad[gl_VertexID], 1.0);
    vertex_texcoord= quad[gl_VertexID].xy * 0.5 + 0.5;
}
#endif


#ifdef FRAGMENT_SHADER
uniform float compression;
uniform float saturation;
uniform float heat;

uniform sampler2D image;
uniform sampler2D colors;

in vec2 vertex_texcoord;
out vec4 fragment_color;

void main(void)
{
    const vec3 rgby= vec3(0.3, 0.59, 0.11);
    float k1= 1.0 / pow(saturation, 1.0 / compression); // normalisation : saturation == blanc
    
    vec3 color= texture2D(image, vertex_texcoord).rgb;
    float y= dot(color, rgby);  // normalisation de la couleur : (color / y) == teinte
    
    if(y > saturation)
        color= vec3(y, y, y);
    
    if(heat > 0.0)
        // applique une fausse couleur
        color= texture2D(colors, vec2(saturation - y / saturation, 0.5)).rgb;
    else
        // applique la compression (gamma)
        color= (color / y) * k1 * pow(y, 1.0 / compression);
    
    fragment_color= vec4(color, 1.0);
}
#endif
