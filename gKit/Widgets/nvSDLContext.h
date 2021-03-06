
#ifndef NV_SDL2_CONTEXT_H
#define NV_SDL2_CONTEXT_H

#include "SDLPlatform.h"

#include "Widgets/nvWidgets.h"


namespace nv {

class SdlContext : public UIContext
{
public:
    SdlContext( );
    ~SdlContext( );
    
    int init( );
    
    // process sdl mouse events
    void processMouseMotionEvent( SDL_MouseMotionEvent& event );
    void processMouseButtonEvent( SDL_MouseButtonEvent& event );
    
    // process sdl keyboard events
    void processKeyboardEvent( SDL_KeyboardEvent& event );
    
    // process sdl dropfile events
    // void processDropfileEvent( );
    
private:
    //  Translate non-ascii keys from SDL to nvWidgets
    unsigned int translateKey( SDL_Keycode k );
    
    int m_mouseX;
    int m_mouseY;
    int m_mouseButton;
};

}       // namespace

#endif
