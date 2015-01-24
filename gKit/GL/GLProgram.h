
#ifndef _GL_PROGRAM_H
#define _GL_PROGRAM_H

#include "GL/GLPlatform.h"
#include "GLResource.h"
#include "GL/ProgramName.h"

#include "Logger.h"


namespace gk {

//! representation d'un shader object.
//! \ingroup OpenGL.
//! \todo a virer
class GLShader : public GLResource
{
    // non copyable
    GLShader( const GLShader& );
    GLShader& operator= ( const GLShader& );

public:
    //! types de shaders.
    enum
    {
        VERTEX= 0,      //!< type d'un vertex shader,
        FRAGMENT,       //!< type d'un fragment shader,
        GEOMETRY,       //!< type d'un geometry shader,
        CONTROL,        //!< type d'un control shader,
        EVALUATION,     //!< type d'un evaluation shader,
        COMPUTE,        //!< type d'un compute shader
        SHADERTYPE_LAST
    };
    
    unsigned int type;  //!< type de shader, constante gk::GLShader::VERTEX, etc.
    
    //! constructeur, 
    //! \param type correspond a une contante gk::GLShader::VERTEX, etc.
    GLShader( ) : GLResource(), type(SHADERTYPE_LAST) {}
    
    GLShader( const char *_label ) : GLResource(_label), type(SHADERTYPE_LAST) {}
    
    GLShader *create( const unsigned int _type )
    {
        assert(name == 0 && "create shader error");
        if(_type >= SHADERTYPE_LAST)
            return this;
        
        type= _type;
        name= glCreateShader(types[type]);
        if(name == 0)
            return this;
        manage();
        if(label.empty())
            DEBUGLOG("create %s shader %d\n", labels[type], name);
        else
            DEBUGLOG("create %s shader %d, label '%s'\n", labels[type], name, label.c_str());
        return this;
    }
    
    void release( )
    {
        if(name != 0)
            glDeleteShader(name);
        name= 0;
    }
    
    virtual ~GLShader( ) {}
    
    static
    GLShader *null( )
    {
        static GLShader object("null shader");
        return &object;
    }
    
    static GLenum types[];
    static const char *labels[];
};


//! representation d'un shader program.
//! \ingroup OpenGL.
class GLProgram : public GLResource
{
    // non copyable
    GLProgram( const GLProgram& );
    GLProgram& operator= ( const GLProgram& );
    
protected:
    struct parameter
    {
        std::string name;
        int location;
        int index;
        int size;
        GLenum type;
        unsigned int flags;
        
        parameter( )
            :
            name(),
            location(-1),
            index(-1),
            size(0),
            type(0),
            flags(0u)
        {}
        
        parameter( const char *_name, const int _location, const int _index, 
            const int _size= 0, const GLenum _type= 0, const unsigned int _flags= 0u)
            :
            name(_name),
            location(_location),
            index(_index),
            size(_size),
            type(_type),
            flags(_flags)
        {}
        
        ~parameter( ) {}
    };
    
    std::vector<parameter> m_feedbacks;
    std::vector<parameter> m_attributes;
    std::vector<parameter> m_uniforms;
    std::vector<parameter> m_samplers;
    std::vector<parameter> m_images;
    
    std::vector<parameter> m_storage_buffers;
    std::vector<parameter> m_uniform_buffers;
    
public:
    std::vector<GLShader *> shaders;    //! \todo inutile, remplacer par std::vector<GLuint>
    
    //! constructeur par defaut.
    GLProgram( )
        :
        GLResource(),
        shaders(GLShader::SHADERTYPE_LAST, GLShader::null()) 
    {}
    
    //! constructuer d'un programme nomme, cf khr_debug.
    GLProgram( const std::string& _label )
        :
        GLResource(_label),
        shaders(GLShader::SHADERTYPE_LAST, GLShader::null())
    {}
    
    //! creation d'un shader program opengl.
    GLProgram *create( )
    {
        assert(name == 0 && "create progran error");
        name= glCreateProgram();
        if(name == 0)
            return this;
        
        manage();
        if(label.empty())
            DEBUGLOG("create program %d\n", name);
        else
            DEBUGLOG("create program %d, label '%s'\n", name, label.c_str());
        return this;
    }
    
    //! destruction du shader program opengl.
    virtual void release( )
    {
        if(name != 0)
            glDeleteProgram(name);
        name= 0;
    }
    
    //! desctructeur.
    virtual ~GLProgram( ) {}
    
    //! ajoute un shader au program.
    int attachShader( GLShader *shader )
    {
        assert(shader != NULL);
        if(name == 0)
            return -1;
        if(shader->type >= GLShader::SHADERTYPE_LAST)
            return -1;
        shaders[shader->type]= shader;
        glAttachShader(name, shader->name);
        return 0;
    }
    
    //! renvoie un des shaders du program. \param type represente quel shader, cf les constantes GLShader::VERTEX, etc.
    GLShader *shader( const unsigned int type )
    {
        if(type >= shaders.size())
            return GLShader::null();
        return shaders[type];
    }
    
    //! enumeration des ressources du programme. necessaire avant d'appeller attribute()/uniform()...
    int resources( );
    
    //! renvoie le nom d'un uniform.
    const char *uniformName( const ProgramUniform& uniform ) const
    {
        return m_uniforms[uniform.index].name.c_str();
    }

    //! renvoie le nom d'un attribut.
    const char *attributeName( const ProgramAttribute& attribute ) const
    {
        return m_attributes[attribute.index].name.c_str();
    }
    
    //! renvoie le nom d'un sampler (cf GLTexture).
    const char *samplerName( const ProgramSampler& sampler ) const
    {
        return m_samplers[sampler.index].name.c_str();
    }
    
    //! renvoie le nom d'une image.
    const char *imageName( const ProgramImage& image ) const
    {
        return m_images[image.index].name.c_str();
    }
    
    //! recherche un attribut.
    ProgramAttribute attribute( const char *name ) const;
    
    //! recherche un uniform.
    ProgramUniform uniform( const char *name ) const;
    //! recherche un smapler.
    ProgramUniform sampler( const char *name ) const;
    //! recherche une image.
    ProgramUniform image( const char *name ) const;
    //~ ProgramUniform subroutineUniform( const char *name );

    //! recherche un uniform buffer.
    ProgramBuffer uniformBuffer( const char *name ) const;
    //! recherche un shader storage buffer.
    ProgramBuffer storageBuffer( const char *name ) const;
    //~ ProgramBuffer atomicBuffer( const char *name );
    
    //~ ProgramIndex subroutine( const char *name );
    //! recherche un varying a enregistrer dans un buffer / feedback. cf glTransformFeedback.
    ProgramFeedback feedback( const char *name ) const;
    
    //! renvoie un objet c++ pour le program opengl 0.
    static
    GLProgram *null( )
    {
        static GLProgram object("null program");
        return &object;
    }
};


}       // namespace 

#endif
