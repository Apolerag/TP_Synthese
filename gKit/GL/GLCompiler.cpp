
#include "GL/GLCompiler.h"


namespace gk {

GLCompiler::GLCompiler( )
    :
    program(GLProgram::null()),
    paths(),
    common(),
    sources(GLShader::SHADERTYPE_LAST)
{}
    
GLCompiler::~GLCompiler( ) {}
 
GLCompiler& GLCompiler::searchPath( const std::string& path )
{
    paths.push_back(path);
    return *this;
}

GLCompiler& GLCompiler::loadCommon( const std::string& filename )
{
    if(filename.empty() == true)
        return *this;
    common.load(IOFileSystem::findFile(filename, paths));
    return *this;
}


GLCompiler& GLCompiler::load( const std::string& filename )
{
    IOFile file(IOFileSystem::findFile(filename, paths));
    if(file.exists() == false)
    {
        ERROR("error reading program '%s'. failed.\n", filename.c_str());
        return *this;
    }
    
    const char *stages[]= {
        "VERTEX_SHADER",
        "FRAGMENT_SHADER",
        "GEOMETRY_SHADER",
        "CONTROL_SHADER",
        "EVALUATION_SHADER",
        "COMPUTE_SHADER",
        ""
    };
    
    MESSAGE("program '%s'...\n", filename.c_str());
    
    std::string source= file.readText();
    for(unsigned int i= 0; i < GLShader::SHADERTYPE_LAST; i++)
    {
        if(source.find(stages[i]) != std::string::npos)
        {
            MESSAGE("  %s shader...\n", GLShader::labels[i]);
            sources[i]= SourceSection(file, source);
            sources[i].define(stages[i]);
        }
    }
    
    return *this;
}

GLCompiler& GLCompiler::loadVertex( const std::string& filename )
{
    sources[GLShader::VERTEX].load(IOFileSystem::findFile(filename, paths));
    if(sources[GLShader::VERTEX].file.exists() == false)
        ERROR("error reading vertex shader '%s'. failed.\n", filename.c_str());
    
    sources[GLShader::VERTEX].define("VERTEX_SHADER");
    return *this;
}

GLCompiler& GLCompiler::loadControl( const std::string& filename )
{
    sources[GLShader::CONTROL].load(IOFileSystem::findFile(filename, paths));
    if(sources[GLShader::CONTROL].file.exists() == false)
        ERROR("error reading control shader '%s'. failed.\n", filename.c_str());

    sources[GLShader::CONTROL].define("CONTROL_SHADER");
    return *this;
}

GLCompiler& GLCompiler::loadEvaluation( const std::string& filename )
{
    sources[GLShader::EVALUATION].load(IOFileSystem::findFile(filename, paths));
    if(sources[GLShader::EVALUATION].file.exists() == false)
        ERROR("error reading evaluation shader '%s'. failed.\n", filename.c_str());
    
    sources[GLShader::EVALUATION].define("EVALUATION_SHADER");
    return *this;
}

GLCompiler& GLCompiler::loadGeometry( const std::string& filename )
{
    sources[GLShader::GEOMETRY].load(IOFileSystem::findFile(filename, paths));
    if(sources[GLShader::GEOMETRY].file.exists() == false)
        ERROR("error reading geometry shader '%s'. failed.\n", filename.c_str());
    
    sources[GLShader::GEOMETRY].define("GEOMETRY_SHADER");
    return *this;
}

GLCompiler& GLCompiler::loadFragment( const std::string& filename )
{
    sources[GLShader::FRAGMENT].load(IOFileSystem::findFile(filename, paths));
    if(sources[GLShader::FRAGMENT].file.exists() == false)
        ERROR("error reading fragment shader '%s'. failed.\n", filename.c_str());
    
    sources[GLShader::FRAGMENT].define("FRAGMENT_SHADER");
    return *this;
}

GLCompiler& GLCompiler::loadCompute( const std::string& filename )
{
    sources[GLShader::COMPUTE].load(IOFileSystem::findFile(filename, paths));
    if(sources[GLShader::COMPUTE].file.exists() == false)
        ERROR("error reading compute shader '%s'. failed.\n", filename.c_str());
    
    sources[GLShader::COMPUTE].define("COMPUTE_SHADER");
    return *this;
}

GLProgram *GLCompiler::reload( )
{
    MESSAGE("reload program '%s'... ", program->label.c_str());
    
    const char *stages[]= {
        "VERTEX_SHADER",
        "FRAGMENT_SHADER",
        "GEOMETRY_SHADER",
        "CONTROL_SHADER",
        "EVALUATION_SHADER",
        "COMPUTE_SHADER",
        ""
    };
    
    bool update= false;
    for(unsigned int i= 0; i < GLShader::SHADERTYPE_LAST; i++)
        // recharge les sources des shaders deja compiles
        if(sources[i].file.modified() == 1)
        {
            MESSAGE("%s shader ", GLShader::labels[i]);
            sources[i].reload();
            
            // verifie que le source modifie n'inclut pas un nouveau type de shader
            for(unsigned int s= i+1; s < GLShader::SHADERTYPE_LAST; s++)
                if(sources[s].source.empty() && sources[i].source.find(stages[s]) != std::string::npos)
                {
                    MESSAGE("%s shader ", GLShader::labels[s]);
                    sources[s]= SourceSection(sources[i].file, sources[i].source);
                    sources[s].define(stages[s]);
                }
            
            update= true;
        }
    
    if(update == false)
    {
        MESSAGE("no modifications.\n");
        return program;
    }
    
    MESSAGE("\n");
    return make();  // rebuild program
}


static
int getLine( std::string& buffer, const std::string& source, int& string_id, int& line_id )
{
    buffer.clear();
    int line= 1;
    for(unsigned int i= 0; i < source.size(); i++)
    {
        if(line == line_id)
            buffer.push_back(source[i]);
        
        if(source[i] == '\n')
        {
            line++;
            if(line > line_id)
                return 0;
        }
    }
    
    if(buffer.empty() == false)
        return 0;       // trouve
    
    // non trouve, iteration sur les sources
    if(source.size() > 0 && source[source.size() -1] != '\n')
        line++;
        
    string_id++;
    line_id= line_id - line +1;
    return -1;
}
    
static
void printErrors( const char *log, const SourceSection& common, const SourceSection& source, const std::string& version )
{
    const char *files[]= {
        "version",
        "common definitions",
        common.file.filename.c_str(),
        "shader definitions",
        source.file.filename.c_str()
    };
    
    const char *strings[]= {
        version.c_str(),
        common.definitions.c_str(),
        common.source.c_str(),
        source.definitions.c_str(),
        source.source.c_str()
    };
    
    // affiche la ligne du source + l'erreur associee
    int last_string= -1;
    int last_line= -1;
    
    for(int i= 0; log[i] != 0; i++)
    {
        int string_id= 0, line_id= 0, position= 0;
        if(sscanf(&log[i], "%d ( %d ) %n", &string_id, &line_id, &position) == 2        // nvidia syntax
        || sscanf(&log[i], "ERROR : %d : %d %n", &string_id, &line_id, &position) == 2  // ati syntax
        || sscanf(&log[i], "WARNING : %d : %d %n", &string_id, &line_id, &position) == 2)  // ati syntax
        {
            if(string_id != last_string || line_id != last_line)
            {
                MESSAGE("\n");
                // affiche la ligne correspondante dans le source
                std::string line;
                for(unsigned int k= 0; k < 5; k++)
                    if(getLine(line, strings[k], string_id, line_id) == 0)
                    {
                        MESSAGE("%s\n", line.c_str());
                        break;
                    }
            }
            
            if(string_id < 5)
                // affiche la localisation fichier/ligne de l'erreur 
                MESSAGE("%s:%d\n", files[string_id], line_id);
            else
                // afficher la ligne complete en cas d'erreur d'intrepretation du message d'erreur ...
                position= 0;
        }
        else
            // afficher la ligne complete en cas d'erreur d'intrepretation du message d'erreur ...
            position= 0;
        
        // affiche l'erreur
        //~ for(i+= position; log[i] != 0; i++)
        for(; log[i] != 0; i++)
        {
            MESSAGE("%c", log[i]);
            if(log[i] == '\n')
                break;
        }
        
        last_string= string_id;
        last_line= line_id;
    }
}

GLProgram *GLCompiler::make( const std::string& label )
{
    bool errors= false;
    for(unsigned int i= 0; i < GLShader::SHADERTYPE_LAST; i++)
    {
        if(sources[i].source.empty())
            continue;
        
        // extraire la version du shader...
        std::string version;
        {
            unsigned long int b= sources[i].source.find("#version");
            if(b != std::string::npos)
            {
                unsigned long int e= sources[i].source.find('\n', b);   // fin de ligne normalisee, cf IOFilesystem::readText()
                if(e != std::string::npos)
                {
                    version= sources[i].source.substr(0, e +1);
                    sources[i].source.erase(0, e +1);
                    
                    DEBUGLOG("version: %s\n", version.c_str());
                }
            }
        }
    
        // ... et la copier en premiere ligne avant les autres definitions
        const char *strings[]= {
            version.c_str(),
            common.definitions.c_str(),
            common.source.c_str(),
            sources[i].definitions.c_str(),
            sources[i].source.c_str()
        };
        
        // cree le shader
        GLShader *shader= program->shader(i);
        if(shader == GLShader::null())
        {
            if(program == GLProgram::null())
                program= (new GLProgram(label))->create();
            
            shader= (new GLShader())->create(i);
            program->attachShader(shader);
        }
        
        // compile le shader
        glShaderSource(shader->name, 5, strings, NULL);
        glCompileShader(shader->name);
        
        GLint status= 0;
        glGetShaderiv(shader->name, GL_COMPILE_STATUS, &status);
        if(status == GL_FALSE)
        {
            errors= true;
            GLint length= 0;
            glGetShaderiv(shader->name, GL_INFO_LOG_LENGTH, &length);
            if(length > 0)
            {
                char log[length +1];
                glGetShaderInfoLog(shader->name, sizeof(log), NULL, log);
                ERROR("error compiling %s shader:\n", GLShader::labels[i]);
                printErrors(log, common, sources[i], version);
            }
            else
                MESSAGE("error compiling %s shader: no log. failed.\n", GLShader::labels[i]);
        }
        
        {
            GLint length= 0;
            glGetShaderiv(shader->name, GL_SHADER_SOURCE_LENGTH, &length);
            if(length > 0)
            {
                char log[length +1];
                glGetShaderSource(shader->name, sizeof(log), NULL, log);
                DEBUGLOG("shader:\n%s\n", log);
            }
        }
    }
    
    if(program == GLProgram::null())
        return program;
    if(errors == true)
    {
        program->resources();
        return program;
    }
    
    // link
    GLint status= 0;
    glLinkProgram(program->name);
    glGetProgramiv(program->name, GL_LINK_STATUS, &status);
    if(status == GL_FALSE)
    {
        GLint length= 0;
        glGetProgramiv(program->name, GL_INFO_LOG_LENGTH, &length);
        if(length > 0)
        {
            char log[length +1];
            glGetProgramInfoLog(program->name, sizeof(log), NULL, log);
            MESSAGE("error linking program:\n%s\nfailed.\n", log);
        }
        else
            MESSAGE("error linking program: no log. failed.\n");
        
        return program;
    }
    
    program->resources();
    MESSAGE("done.\n");
    return program;
}

}       // namespace
