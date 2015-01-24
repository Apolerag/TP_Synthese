#version 330

#ifdef VERTEX_SHADER
    uniform mat4 mvpMatrix;
    uniform mat4 normalMatrix;
    
    in vec3 position;           //!< attribut
    in vec3 normal;             //!< attribut
    in vec3 texcoord;           //!< attribut
    
    out vec3 vertex_normal;
    out vec2 vertex_texcoord;

    void main( )
    {
        gl_Position= mvpMatrix * vec4(position, 1.0);
        vertex_normal= mat3(normalMatrix) * normal;
        vertex_texcoord= texcoord.st;
    }
#endif

#ifdef FRAGMENT_SHADER
    uniform vec4 color;
    uniform sampler2D diffuse_texture;
    
    in vec3 vertex_normal;
    in vec2 vertex_texcoord;
    
    out vec4 fragment_color;

    void main( )
    {
        fragment_color.rgb= 2 * color.rgb * abs(normalize(vertex_normal).z) * texture(diffuse_texture, vertex_texcoord).rgb;
    }
#endif

