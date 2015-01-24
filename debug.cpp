#define GLDEBUGGER

#include "App.h"
#include "Widgets/nvSDLContext.h"
#include "Logger.h"

#include "ProgramManager.h"
#include "GL/GLQuery.h"
#include "GL/GLTexture.h"
#include "GL/GLVertexArray.h"

//! classe utilitaire : permet de construire une chaine de caracteres formatee. cf sprintf.
struct Format
{
    char text[1024];
    
    Format( const char *_format, ... )
    {
        text[0]= 0;     // chaine vide
        
        // recupere la liste d'arguments supplementaires
        va_list args;
        va_start(args, _format);
        vsnprintf(text, sizeof(text), _format, args);
        va_end(args);
    }
    
    ~Format( ) {}
    
    // conversion implicite de l'objet en chaine de caracteres stantard
    operator const char *( )
    {
        return text;
    }
};

//~ class TP : public gk::App
class TP : public gk::AppDebugger
{
    nv::SdlContext m_widgets;
    
    gk::GLProgram *m_program;
    gk::GLVertexArray *m_vao;
    
public:
    // creation du contexte openGL et d'une fenetre
    TP( )
        :
        //~ gk::App()
        gk::AppDebugger()
    {
        // specifie le type de contexte openGL a creer :
        gk::AppSettings settings;
        settings.setGLVersion(3,3);     // version 3.3
        settings.setGLCoreProfile();      // core profile
        settings.setGLDebugContext();     // version debug pour obtenir les messages d'erreur en cas de probleme
        
        // cree le contexte et une fenetre
        if(createWindow(512, 512, settings) < 0)
            closeWindow();
        
        // initilalise les widgets
        m_widgets.init();
        m_widgets.reshape(windowWidth(), windowHeight());
    }
    
    ~TP( ) {}
    
    // a redefinir pour utiliser les widgets.
    void processWindowResize( SDL_WindowEvent& event )
    {
        m_widgets.reshape(event.data1, event.data2);
    }
    
    // a redefinir pour utiliser les widgets.
    void processMouseButtonEvent( SDL_MouseButtonEvent& event )
    {
        m_widgets.processMouseButtonEvent(event);
    }
    
    // a redefinir pour utiliser les widgets.
    void processMouseMotionEvent( SDL_MouseMotionEvent& event )
    {
        m_widgets.processMouseMotionEvent(event);
    }
    
    // a redefinir pour utiliser les widgets.
    void processKeyboardEvent( SDL_KeyboardEvent& event )
    {
        m_widgets.processKeyboardEvent(event);
    }
    
    int init( )
    {
        // compilation simplifiee
        m_program= gk::createProgram("tp1sphere.glsl");
        if(m_program == gk::GLProgram::null())
            return -1;
        
        m_vao= gk::createVertexArray();
        return 0;
    }
    
    int quit( )
    {
        return 0;
    }

    int draw( )
    {
        if(key(SDLK_ESCAPE))
            // fermer l'application si l'utilisateur appuie sur ESCAPE
            closeWindow();
        
        if(key('r'))
        {
            key('r')= 0;
            // recharge et recompile les shaders
            gk::reloadPrograms();
        }
        
        if(key('s'))
        {
            key('s')= 0;
            // enregistre l'image opengl
            gk::writeFramebuffer("screenshot.png");
        }
        
        //
        glViewport(0, 0, windowWidth(), windowHeight());
        m_widgets.reshape(windowWidth(), windowHeight());
        
        GLint64 start; glGetInteger64v(GL_TIMESTAMP, &start);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glUseProgram(m_program->name);
        glBindVertexArray(m_vao->name);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        
        glUseProgram(0);
        glBindVertexArray(0);

        GLint64 stop; glGetInteger64v(GL_TIMESTAMP, &stop);     // nano secondes
        GLuint64 cpu_time= (stop - start) / 1000;               // conversion en micro secondes
        GLuint64 gpu_time= 0 / 1000;
        
        // afficher le temps d'execution
        {
            m_widgets.begin();
            m_widgets.beginGroup(nv::GroupFlags_GrowDownFromLeft);
            
            m_widgets.doLabel(nv::Rect(), Format("cpu time % 6ldus", cpu_time));
            m_widgets.doLabel(nv::Rect(), Format("gpu time % 3ldms % 3ldus", gpu_time / 1000, gpu_time % 1000));
            
            static bool button= false;
            m_widgets.doButton(nv::Rect(), "click", &button);
            
            m_widgets.endGroup();
            m_widgets.end();
        }
        
        swapBuffers();  // debugger / break on frame
        present();      // app + debugger
        return 1;
    }
};


int main( int argc, char **argv )
{
    TP app;
    app.run();
    
    return 0;
}

