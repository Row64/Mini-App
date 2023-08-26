#ifndef C_LAYOUTMANAGER_CPP
#define C_LAYOUTMANAGER_CPP

#include "c_PipelineManager.h"

namespace AppCore {

    // ---------- LayoutManager ----------

    LayoutManager::LayoutManager( PipelineManager &parent ) :
        Parent( parent ),
        AppData( parent.GetAppData() ),
        PTM( parent ),
        PollMap( {} ) {
    }

    LayoutManager::~LayoutManager() {
    }

    void LayoutManager::Init( size_t resource_count ) {
        InitStyles();
        InitLayouts();
        PTM.InitPipelines( resource_count );
        UpdateCurrentLayout();
    }

    void LayoutManager::SetCurrentLayout( string inName ) {
        CurrentLayoutName = inName;
        CurrentLayout = PTM.GetPaneTree( inName );
        CurrentTUI = PTM.GetTUIVector( CurrentLayout );
    }


    void LayoutManager::SetCurrentLayout( int inIndex ) {
        CurrentLayout = PTM.GetPaneTree( inIndex );
        CurrentLayoutName = CurrentLayout->Name;
        CurrentTUI = PTM.GetTUIVector( CurrentLayout );
    }
    

    void LayoutManager::UpdateCurrentDim() { // updates dimensions without redraw
        array<int, 2> window_size = { (int) UIGetWindowSize().x, (int) UIGetWindowSize().y};
        PTM.GetDimensions( CurrentLayout , &window_size );
        PTM.UpdateDimensions( CurrentLayout );
    }
    
    void LayoutManager::UpdateCurrentLayout() {
        array<int, 2> window_size = { (int) UIGetWindowSize().x, (int) UIGetWindowSize().y};
        PTM.GetDimensions( CurrentLayout , &window_size );
        PTM.UpdateDimensions( CurrentLayout );
        PTM.UpdateModes(  CurrentLayout );
        PTM.GetTUIPipeline().ReInit();
        printf("ReInit Complete: %s\n", "TUI" );

    }

    bool LayoutManager::SetPoll( string inName, bool inPoll ) {
        auto iter = PollMap.find( inName );
        if ( iter != PollMap.end() ) {
            iter->second = inPoll;
            if ( inPoll ) { return true; }
        } else {
            PollMap.insert( pair<string,bool>( inName, inPoll ) );
        }
        return GetPoll();
    }

    bool LayoutManager::RemovePoll( string inName ) {
        PollMap.erase( inName );
        return GetPoll();
    }

    bool LayoutManager::GetPoll() {
        if ( PollMap.size() == 0 ) { return CurrentLayout->Poll; }
        for (auto iter = PollMap.begin(); iter != PollMap.end(); ++iter) {
            if (iter->second == true ) { return true; }
        }
        return CurrentLayout->Poll;
    }

    bool LayoutManager::ResetPoll() {
        PollMap.clear();
        return CurrentLayout->Poll;
    }

   
    void LayoutManager::DXRender( ID3D11DeviceContext* ctx ) {
        PTM.GetTUIPipeline().Render( ctx, CurrentTUI );
    }

    void LayoutManager::OnWindowSizeChanged_Pre() {
        UpdateCurrentDim();
        PTM.GetTUIPipeline().OnWindowSizeChanged_Pre();
    }

    void LayoutManager::OnWindowSizeChanged_Post() {
        PTM.GetTUIPipeline().OnWindowSizeChanged_Post();   
    }


} // end namespace AppCore

#endif // C_LAYOUTMANAGER_CPP