#ifndef TUI_PANEBASE_H
#define TUI_PANEBASE_H

#include <cmath>
#define _USE_MATH_DEFINES

#include "UILib.h"
using namespace UILib;

#include "ch_AppTimer.h"

#include <iostream>
#include <vector>
#include <string>
#include <bitset>
#include <glm/glm.hpp>

using namespace std;
using namespace glm;

namespace AppCore {
    
    class PipelineManager;
    class AppDataIO;

    // ************************************************************ //
    // TUI_PaneBase Class                                           //
    // ************************************************************ //
    class TUI_PaneBase {

    public:

       TUI_PaneBase( PipelineManager &parent );
        virtual ~TUI_PaneBase();
        
        void                    UpdateDimensions( float inX, float inY, float inW, float inH);
        void                    UpdateMode( string inMode );
        
        virtual void            InitPane();
        virtual void            DrawPane();
        virtual int             ListenForMessage(string inType, vector<string> inMessage);  // Returns 0 = Ignored, 1 = Received, 2 = Halt Further Messages
        string                  PaneName = "";
        string                  PaneType = "PaneBase";

        virtual void *          GetSelf()  { return this; };

        float                   X = 0.0f;
        float                   Y = 0.0f;
        float                   Width = 0.0f;
        float                   Height = 0.0f;
        string                  Mode = "";

    protected:
    
        PipelineManager &       Parent;
        AppDataIO &             AppData;

        virtual void            StartPane();
        virtual void            FinishPane();

    };

}

#endif // TUI_PANEBASE_CPP