#ifndef C_LAYOUTMANAGER_H
#define C_LAYOUTMANAGER_H

#include "UILib.h"
#include "user_Includes.h"
#include "c_PaneTreeManager.h"

using namespace std;

namespace AppCore {

    // ************************************************************ //
    // LayoutManager Class                                          //
    //                                                              //
    // Holds all Layouts used                                       //
    // ************************************************************ //
    class LayoutManager {

    public:

        LayoutManager( PipelineManager &parent );
        ~LayoutManager();
        
        void                    Init( size_t resource_count );
        
        void                    SetCurrentLayout( string inName );
        void                    SetCurrentLayout( int inIndex );
        void                    UpdateCurrentLayout();
        void                    UpdateCurrentDim();
        
        bool                    SetPoll( string inName, bool inPoll );
        bool                    RemovePoll( string inName );
        bool                    GetPoll();
        bool                    ResetPoll();
        
        void                    InitStyles();
        void                    InitLayouts();      // Add your layouts here
        
        void                    DXRender( ID3D11DeviceContext* ctx );
    
        void                    OnWindowSizeChanged_Pre();
        void                    OnWindowSizeChanged_Post();
        
        PipelineManager &           Parent;
        AppDataIO &                 AppData;

        string                      CurrentLayoutName;
        PaneTree                    *CurrentLayout;
        vector<TUI_PaneBase*>       CurrentTUI;

        PaneTreeManager         PTM;

        map<string,bool>        PollMap;   // Map of app components that have registered a Poll / Wait

    };

} // end namespace AppCore

#endif // C_LAYOUTMANAGER_H