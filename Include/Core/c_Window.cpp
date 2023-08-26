#ifndef C_WINDOW_CPP
#define C_WINDOW_CPP

#include "c_Window.h"

// // These not used
// #define GLFW_EXPOSE_NATIVE_WGL
// #define GLFW_EXPOSE_NATIVE_WIN32 
// #include <GLFW/glfw3native.h>

#include "stb_image.h"

using namespace std;
namespace AppCore {

    // ------- WindowBase ------- //
    
    WindowBase::WindowBase() :
        CanRender( false ) {
    }

    WindowBase::~WindowBase() {
    }

    // Mouse
    
    void WindowBase::UpdateMousePosition( double x, double y ) {
        InputState.Position.X = x;
        InputState.Position.Y = y;
    }

    void WindowBase::UpdateMouseButton( size_t button, bool pressed ) {
        if( button < 3 ) {
            InputState.Buttons[button].IsPressed = pressed;
        }
        if ( CanRender ) {
            RenderPipelines();
        }
    }
    
    // Display 

    std::vector<glm::uvec2> WindowBase::GetMonitorSizes() {
        std::vector<glm::uvec2> res;
        int count;
        auto monitors = glfwGetMonitors( &count );
        for ( int i = 0; i < count; i++ ) {
            auto vmode = glfwGetVideoMode( monitors[i] );
            res.push_back( uvec2( vmode->width, vmode->height ));
        }
        return res;
    }

    void WindowBase::UpdateWindowSize( int w, int h ) {
        DisplayState.WindowSize.W = w;
        DisplayState.WindowSize.H = h;
    }

    
    void WindowBase::UpdateFramebufferSize( int w, int h ) {
        DisplayState.FramebufferSize.W = w;
        DisplayState.FramebufferSize.H = h;
        DisplayState.ScaleFactor.X = (float) DisplayState.FramebufferSize.W / (float) DisplayState.WindowSize.W;
        DisplayState.ScaleFactor.Y = (float) DisplayState.FramebufferSize.H / (float) DisplayState.WindowSize.H;
    }

    // Access methods

    UILib::InputStateData & WindowBase::GetInputState() {
        return InputState;
    }

    UILib::DisplayStateData & WindowBase::GetDisplayState() {
        return DisplayState;
    }


    // ------- Window ------- //


    Window::Window() :
        Parameters() {
    }

    Window::~Window() {
        glfwDestroyWindow(Parameters.WindowPtr);
        glfwTerminate();
    }

    WindowParameters Window::GetParameters() const {
        return Parameters;
    }

    void Window::Create( const char *title, int width, int height ) {

        if( !glfwInit() ) {
            glfwTerminate();
            return;
        };

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE); 

        GLFWmonitor *monitor = glfwGetPrimaryMonitor();
        auto vmode = glfwGetVideoMode( monitor );

        // Creates GLFW window
        Parameters.WindowPtr = glfwCreateWindow( vmode->width, vmode->height, title, NULL, NULL);
        if( !Parameters.WindowPtr ) {
            printf("Could not create Window \n");
            return;
        }

        // Get the mouse pos in screen coordinates
        double mposX = 0, mposY = 0;
        glfwGetCursorPos( Parameters.WindowPtr, &mposX, &mposY );
        
        // Get the active monitor (where the mouse is)
        int count;
        GLFWmonitor **mList = glfwGetMonitors(&count);
        int xp=0; int yp=0; int mw = 0; int mh = 0;
        for ( int i = 0; i < count; i++ ) { 
            monitor = mList[i]; 
            glfwGetMonitorWorkarea( monitor, &xp, &yp, &mw, &mh );
            if ( (int) mposX >= xp && (int) mposX <= xp + mw ) { 
                break;
            }
        }

        // Set window size and position
        if ( width <= 0 || height <= 0 ) {
            width = mw * 9/10;  
            height = mh * 9/10;
        }        
        glfwSetWindowSize( Parameters.WindowPtr, width, height );
        glfwSetWindowPos(Parameters.WindowPtr,(mw - width) / 2 + xp,(mh - height) / 2); 


        // Adds icons
        if (fs::exists( ADIO->RootPath / "Data/Icons") ) {
            vector<GLFWimage> icons;
            for (const auto & entry : fs::directory_iterator( ADIO->RootPath / "Data/Icons" ) ){
                if ( ( entry.path().stem().string().find("r64_icon") != 0 )  ) { continue; }
            #ifdef __GNUC__
                if ( entry.path().extension().string() == ".ico" ) { continue; }
            #endif
                GLFWimage img; int n;
                img.pixels = stbi_load(CONVERT_PATH(entry.path()), &img.width, &img.height, &n, 0);
                icons.push_back(img);
            }
            glfwSetWindowIcon(Parameters.WindowPtr, (int) icons.size(), icons.data());
        }

    }

    void Window::RenderingLoop( WindowBase &project ) const {

        
        // Set User Pointer
        glfwSetWindowUserPointer(Parameters.WindowPtr, reinterpret_cast<void *>(&project));

        //GLFWcursor* cursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
        //glfwSetCursor(Parameters.WindowPtr, cursor);
        
        // Set Callbacks
        glfwSetWindowRefreshCallback(Parameters.WindowPtr, &WindowRefreshEvent);
        glfwSetWindowSizeCallback(Parameters.WindowPtr, &WindowResizeEvent);
        glfwSetWindowCloseCallback(Parameters.WindowPtr, &WindowCloseEvent);

        glfwSetMouseButtonCallback(Parameters.WindowPtr, &MouseEvent);
        glfwSetCursorPosCallback(Parameters.WindowPtr, &MouseMoveEvent);
        glfwSetKeyCallback(Parameters.WindowPtr, &KeyEvent);
        glfwSetCharCallback(Parameters.WindowPtr, &CharEvent);
        glfwSetScrollCallback(Parameters.WindowPtr, &ScrollEvent);
        glfwSetDropCallback(Parameters.WindowPtr, &DropEvent);

        // When this input mode is enabled, any callback that receives 
        // modifier bits will have the GLFW_MOD_CAPS_LOCK bit set if 
        // Caps Lock was on when the event occurred and the 
        // GLFW_MOD_NUM_LOCK bit set if Num Lock was on.
        glfwSetInputMode(Parameters.WindowPtr, GLFW_LOCK_KEY_MODS, GLFW_TRUE);
        
        int w, h;
        glfwGetWindowSize(Parameters.WindowPtr, &w, &h);
        project.UpdateWindowSize(w, h);

        int fw, fh;
        glfwGetFramebufferSize(Parameters.WindowPtr, &fw, &fh);
        project.UpdateFramebufferSize( fw, fh);

        // while(!glfwWindowShouldClose(Parameters.WindowPtr)) {
        while ( !project.ShouldClose ) {
            
            if( project.Poll == true ) { glfwPollEvents(); }
            else { glfwWaitEvents(); }
            
            // static uint64_t frameNb = 0;
            if( project.CanRender ) {
                try {
                    UILib::InputStateData& ISD = project.GetInputState();
                    ISD.Resize = false;
                    project.RenderPipelines();
                    // std::cout << "Finished Rendering Frame " << frameNb << std::endl; frameNb++;

                } catch (int except) {
                    std::cout << "RenderingLoop Exception: " << except << std::endl;
                    break;
                }
            } else {
                std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
            }
            
        }

    }

    // --------------- Window callback functions --------------- 

     void Window::WindowCloseEvent(GLFWwindow* window) {
        WindowBase *project = reinterpret_cast<WindowBase*>(glfwGetWindowUserPointer(window));
        project->ShouldClose = true;        
    }

    void Window::WindowRefreshEvent(GLFWwindow* window) {
        WindowBase *project = reinterpret_cast<WindowBase*>(glfwGetWindowUserPointer(window));
        if ( project->CanRender ) {
            UILib::InputStateData& ISD = project->GetInputState();
            ISD.Resize = true;
            project->RenderPipelines();
        }
    }

    void Window::WindowResizeEvent(GLFWwindow* window, int width, int height) {
        int fwidth, fheight;
        glfwGetFramebufferSize(window, &fwidth, &fheight);

        WindowBase *project = reinterpret_cast<WindowBase*>(glfwGetWindowUserPointer(window));
        project->UpdateWindowSize( width, height );
        project->UpdateFramebufferSize( fwidth, fheight );
        project->OnWindowSizeChanged();
    }

    void Window::DropEvent(GLFWwindow* window, int count, const char** paths){
        if(count > 0){
            string dropPath = paths[0];
            WindowBase *project = reinterpret_cast<WindowBase*>(glfwGetWindowUserPointer(window));
            UILib::InputStateData& ISD = project->GetInputState();
            ISD.DropPath = paths[0];
        }
    }

    void Window::ScrollEvent(GLFWwindow* window, double xoffset, double yoffset){
        WindowBase *project = reinterpret_cast<WindowBase*>(glfwGetWindowUserPointer(window));
        UILib::InputStateData& ISD = project->GetInputState();
        ISD.ScrollY = (float)yoffset;
    }

    void Window::MouseEvent(GLFWwindow* window, int button, int action, int mods) {   
        WindowBase *project = reinterpret_cast<WindowBase*>(glfwGetWindowUserPointer(window));

        if( action == GLFW_RELEASE ){
            UILib::InputStateData& ISD = project->GetInputState();
            static auto before = std::chrono::system_clock::now();
            auto now = std::chrono::system_clock::now();
            double diff_ms = std::chrono::duration <double, std::milli> (now - before).count();
            before = now;
            if(diff_ms>10 && diff_ms<300){ISD.DoubleClick = 1;}
        }

        project->UpdateMouseButton( (size_t) button, action );

    }
    
    void Window::MouseMoveEvent(GLFWwindow* window, double xpos, double ypos) {
        WindowBase *project = reinterpret_cast<WindowBase*>(glfwGetWindowUserPointer(window));
        project->UpdateMousePosition( xpos, ypos);
    }
    
    void Window::KeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods){
        WindowBase *project = reinterpret_cast<WindowBase*>(glfwGetWindowUserPointer(window));
        UILib::InputStateData& ISD = project->GetInputState();
        vector<array<int,2>> *kBuffer = &ISD.Key.Buffer;
        if (action == GLFW_PRESS){
            kBuffer->push_back({1, key});
        }
        else if (action == GLFW_REPEAT){}
        else if (action == GLFW_RELEASE ){ 
            kBuffer->push_back({2, key});
            if ( project->CanRender ) {project->RenderPipelines();} // add update on mouse up for better key input
        }
        if (mods & GLFW_MOD_SHIFT) {ISD.Shift = true;}else{ISD.Shift = false;}
        if (mods & GLFW_MOD_CONTROL) {ISD.Ctrl = true;}else{ISD.Ctrl = false;}
        if (mods & GLFW_MOD_ALT) {ISD.Alt = true;}else{ISD.Alt = false;}
        if (mods & GLFW_MOD_SUPER) {ISD.Supr = true;}else{ISD.Supr = false;}
        if (mods & GLFW_MOD_CAPS_LOCK) {ISD.Caps = true;}else{ISD.Caps = false;}
        
    }

    void Window::CharEvent(GLFWwindow* window, unsigned int codepoint) {
        // See: https://www.glfw.org/docs/3.3/input_guide.html#input_char
        // GLFW supports text input in the form of a stream of Unicode code points, 
        // as produced by the operating system text input system. You can treat the 
        // code point argument as native endian UTF-32. This can be used for regular
        // text input.
        UILibIS->Key.CodePoint = codepoint;
        // std::cout << "CHAR EVENT: " << codepoint << std::endl;
    }


}   // end namespace AppCore


#endif	// C_WINDOW_CPP