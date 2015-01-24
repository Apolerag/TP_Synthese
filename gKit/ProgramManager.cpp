
#include <algorithm>

#include "ProgramManager.h"


namespace gk {
    
ProgramManager::ProgramManager( ) : m_compilers(), m_paths() {}

ProgramManager::~ProgramManager( )
{
    for(unsigned int i= 0; i < m_compilers.size(); i++)
        delete m_compilers[i];
}

int ProgramManager::searchPath( const std::string& path )
{
    if(std::find(m_paths.begin(), m_paths.end(), path) == m_paths.end())
        m_paths.push_back(path);
    return 0;
}

GLProgram *ProgramManager::createProgram( const std::string& label, const std::string& vertex, const std::string& fragment )
{
    GLCompiler *compiler= new GLCompiler;
    m_compilers.push_back(compiler);
    
    for(unsigned int i= 0; i < m_paths.size(); i++)
        compiler->searchPath(m_paths[i]);

    compiler->loadVertex(vertex);
    compiler->loadFragment(fragment);
    return compiler->make(label);       // nomme le programme
}

GLCompiler *ProgramManager::loadProgram( const std::string& source, const std::string& common )
{
    GLCompiler *compiler= new GLCompiler;
    m_compilers.push_back(compiler);
    
    for(unsigned int i= 0; i < m_paths.size(); i++)
        compiler->searchPath(m_paths[i]);

    compiler->loadCommon(common);
    compiler->load(source);
    
    return compiler;
}

GLProgram *ProgramManager::createProgram( const std::string& source, const std::string& common )
{
    return loadProgram(source, common)->make(source);      // nomme le programme == nom du fichier source
}

GLProgram *ProgramManager::program( const std::string& label )
{
    for(unsigned int i= 0; i < m_compilers.size(); i++)
        if(m_compilers[i]->program->label == label)
            return m_compilers[i]->program;
    
    return GLProgram::null();
}

int ProgramManager::reload( )
{
    for(unsigned int i= 0; i < m_compilers.size(); i++)
        m_compilers[i]->reload();
    
    return 0;
}

GLProgram *defaultProgram( )
{
    static GLProgram *object= NULL;
    if(object == NULL)
        object= createProgram("shaders/default.glsl");
    
    return object;
}

}       // namespace
