#ifndef C_WINDOW_H
#define C_WINDOW_H

#if defined( __linux__ )
    #include <algorithm>
    #include <iterator>
#endif
#if defined( _WIN32 )
    //#define NOMINMAX
    #include <windows.h>
    #include <stdexcept>
    #include <algorithm>
#endif

#include <GLFW/glfw3.h>

#if defined( _WIN32 )
#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/glfw3native.h>
#endif

#include <iostream>
#include <cstring>
#include <string>
#include <thread>
#include <chrono>
#include <memory>
#include <vector>
#include <array>
#include <cmath>

#include "ch_Glm.h"
#include "user_AppDataIO.h"

namespace AppCore {


    // ************************************************************ //
    // WindowBase Class                                             //
    // ************************************************************ //

    class WindowBase {

    public:

        bool ShouldClose = false;

        WindowBase();
        virtual ~WindowBase();

        // Virtual methods that can be overrided by derived classes
        virtual void OnWindowSizeChanged() = 0;
                
        virtual void RenderPipelines() = 0;

        virtual void SetPoll( std::string inName, bool inPoll ) = 0;
        virtual void RemovePoll( std::string inName ) = 0;
        virtual void ResetPoll() = 0;
        virtual void QueueForwardFrames( int inCount = 1 ) = 0;

        std::vector<glm::uvec2> GetMonitorSizes();

        // Methods to update mouse and display data
        void UpdateMousePosition( double x, double y );
        void UpdateMouseButton( size_t button, bool pressed );
        
        void UpdateWindowSize( int w, int h );
        void UpdateFramebufferSize( int w, int h );
        
        // Methods to access data
        UILib::InputStateData &    GetInputState();
        UILib::DisplayStateData &  GetDisplayState();

        bool                        CanRender;
        bool                        Pause = false; 
        bool                        Poll = false;

    protected:

        UILib::InputStateData       InputState;
        UILib::DisplayStateData     DisplayState;

    };

    // ************************************************************ //
    // Window Parameters                                            //
    // ************************************************************ //
    struct WindowParameters {
        GLFWwindow         *WindowPtr;

        WindowParameters() :
            WindowPtr() {
        }

    };

    // ************************************************************ //
    // Window Class                                                 //
    // ************************************************************ //

    class Window {
    
    public:

        Window();
        ~Window();

        void                Create( const char * title, int width = 500, int height = 500 );
        void                RenderingLoop( WindowBase & project ) const;
        
        WindowParameters    GetParameters() const;
        // string              GetClipboard();
        // void                SetClipboard(string inStr);

    private:
    
        WindowParameters  Parameters;
        // Callback method
        static void WindowCloseEvent(GLFWwindow* window);
        static void WindowRefreshEvent(GLFWwindow* window);
        static void WindowResizeEvent(GLFWwindow* window, int width, int height);
        static void FramebufferSizeEvent(GLFWwindow* window, int width, int height);

        static void MouseEvent(GLFWwindow* window, int button, int action, int mods);
        static void MouseMoveEvent(GLFWwindow* window, double xpos, double ypos);

        static void KeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void CharEvent(GLFWwindow* window, unsigned int codepoint);

        static void ScrollEvent(GLFWwindow* window, double xoffset, double yoffset);
        static void DropEvent(GLFWwindow* window, int count, const char** paths);        

    };


}   // end namespace AppCore

#endif	// C_WINDOW_H