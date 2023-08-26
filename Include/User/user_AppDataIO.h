#ifndef USER_APPDATAIO_H
#define USER_APPDATAIO_H

// ######################### Row64 Version Numbers #########################
// Make sure this lines up with Pack_Install_Files.py and the Installer app
#ifndef R64_VERSION_INFO
    #define R64_VERSION_INFO
    #define R64_VERSION_MAJOR "2"
    #define R64_VERSION_MINOR "0"
    #define R64_VERSION_QFE ""
#endif
// ##########################################################################

#ifndef NDEBUG
    #define SHOW_DEBUG_MODE_OPTIONS
    #define DEV_BUILD_FOLDER_RECIPES    // Uncomment this to use Recipes in the /build folder (for dev)
#endif
// #define SHOW_DEBUG_MODE_OPTIONS  // Uncomment this if you want to expose debug UI in release mode

#include "UILib.h"
using namespace UILib;

#include "Blocks.h"
using namespace Blocks;

#include "ch_Tools.h"
#include "ch_ThreadPool.h"
#include "ch_AppTimer.h"
#include "ch_AppMessage.h"
#include "ch_FileSystem.h"

#include <glm/glm.hpp>
#include <string>
#include <iostream>
using namespace std;

namespace AppCore {

    class PipelineManager;

    //1 Free, 2 Plus, 3 Pro, 4 Enterprise, 5 Develop
    enum app_upgrade_type {
        upgrade_free        = 1,
        upgrade_plus        = 2,
        upgrade_pro         = 3,
        upgrade_enterprise  = 4,
        upgrade_dev         = 5
    };
    
    // ************************************************************ //
    // AppDataIO Class                                              //
    //                                                              //
    // Global data and IO communication between windows and panes   //
    // ************************************************************ //
    class AppDataIO {
    
    public:

        AppDataIO( char* inPath );
        
        fs::path                AppPath;        // Path to Row64.exe
        fs::path                RootPath;       // Path to /build folder where Row64.exe is located

        glm::vec2               Dim = {0,0};  // Application window - width,height

        int                     ButtonBarIndex;   // for the main sprite sheet used by the application
        glm::vec2               ButtonBarDim = {1024,256};

        PPGManager              MM;
        FocusManager            FM;

        // ------------------------------------------------------ Theme ------------------------------------------------------

        vector<int>             CurrentTheme = {0x061B42, 0x225EA8, 0x7FCDBB, 0xFFFFD9};    // colors of current theme
        vector<int>             CurrentHighlight = {-1, -1};                                // highlight colors (-1 = no highlight color )
        float                   CurrentThemeSmooth = 0.1f;                                  // blend factor between linear and bezier color

        // --------------------------------------------------- PPG Options ------------------------------------------------------

        vector<PropertyPage>    OpenPPGs = {};
        string                  ColorPickerSource = "";
        void *                  ColorPickerData = nullptr;
        
        // ------------------------------------------------------ UI Options ------------------------------------------------------

        UIFormatScroller        DefaultScroller;
        ImRect                  CurrentBB;

        bool                    DebugIDE = false;

        // --------------------------------------------------- Hardware Details ---------------------------------------------------

        std::string             SelectedGPUDesc = "";
        double                  SelectedGPUVRam = 0;
        
       void PreRenderSetup();
       void PostRenderSetup();

    }; // end of AppDataIO class

// ---------- Global Pointers ----------

    #ifndef PMPTR
        extern PipelineManager* PMPTR;
    #endif

    #ifndef ADIO
        extern AppDataIO* ADIO;
    #endif

    void SetPMPTR( PipelineManager* inPM );
    void SetADIO( AppDataIO* inAD );

// -------------- UILib InputBoxMode Modes ----------

    enum inputbox_modes {
        inputbox_none = 0,
        inputbox_default = 1,
        inputbox_password = 2,
        inputbox_chart = 3
    };

    
    
} // end namespace AppCore

#endif // USER_APPDATAIO_H