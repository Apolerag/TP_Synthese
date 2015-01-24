
#ifndef _GL_COMPILER_H
#define _GL_COMPILER_H

#include "Logger.h"

#include "IOFile.h"
#include "IOFileSystem.h"

#include "GL/GLProgram.h"


namespace gk {

//! utilisation interne. representation d'une partie du source d'un shader program.
//! \ingroup OpenGL.
struct SourceSection
{
    std::string definitions;
    std::string source;
    IOFile file;

    SourceSection( )
        :
        definitions(),
        source(),
        file()
    {}
    
    SourceSection( const std::string& filename )
        :
        definitions(),
        source(),
        file()
    {
        load(filename);
    }
    
    SourceSection( const IOFile& _file, const std::string& _source )
        :
        definitions(),
        source(_source),
        file(_file)
    {}
    
    //! definit une valeur : #define what value.
    SourceSection& define( const std::string& what, const std::string& value= "" )
    {
        std::string d;
        d.append("#define ").append(what);
        if(value.empty() == false)
            d.append(" ").append(value);
        
        definitions.append(d).append("\n");
        return *this;
    }
    
    //! charge le fichier texte filename.
    SourceSection& load( const std::string& filename )
    {
        file= IOFile(filename);
        source= file.readText();
        return *this;
    }
    
    //! recharge le fichier texte, si necessaire.
    SourceSection& reload( )
    {
        file.reloadText(source);
        return *this;
    }
};


//! assemble les sources des shaders, les parametres, les compile et produit un shader program utilisable.
//! \ingroup OpenGL.
class GLCompiler
{
    GLCompiler( const GLCompiler& );
    GLCompiler& operator= ( const GLCompiler& );
    
public:
    GLProgram *program;
    
    std::vector<std::string> paths;

    SourceSection common;
    std::vector<SourceSection> sources;

    GLCompiler( );
    ~GLCompiler( );
    
    //! ajoute un repertoire dans lequel chercher les fichiers sources a charger.
    GLCompiler& searchPath( const std::string& path );

    //! charge un source commun a tous les shaders du program. en gros un fichier include.
    GLCompiler& loadCommon( const std::string& filename );
    //! charge un seul fichier contenant les sources de tous les shaders a compiler.
    //! le source de chaque shader est defini par une directive du pre processeur. VERTEX_SHADER, FRAGMENT_SHADER, GEOMETRY_SHADER, etc.
    GLCompiler& load( const std::string& filename );

    //! charge le source du vertex shader.
    GLCompiler& loadVertex( const std::string& filename );
    //! charge le source du control shader.
    GLCompiler& loadControl( const std::string& filename );
    //! charge le source de l'evaluation shader.
    GLCompiler& loadEvaluation( const std::string& filename );
    //! charge le source du geometry shader.
    GLCompiler& loadGeometry( const std::string& filename );
    //! charge le source du fragment shader.
    GLCompiler& loadFragment( const std::string& filename );
    //! charge le source d'un compute shader.
    GLCompiler& loadCompute( const std::string& filename );
    
    //! compile les shaders et les linke dans un program.
    GLProgram *make( const std::string& label= "" );
    
    //! recharge les sources et recompile le program, si necessaire.
    GLProgram *reload( );
};

}       // namespace

#endif
