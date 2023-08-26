#ifndef C_PIPELINEMANAGER_CPP
#define C_PIPELINEMANAGER_CPP


#include "c_PipelineManager.h"

namespace AppCore {

    PipelineManager::PipelineManager( std::string const & title, AppDataIO & inAppData ) :
        WindowBase(), DXBase( CanRender, DisplayState ),
        Title( title ),
        AppData( inAppData ),
        LM( *this ) {
            SetPMPTR( &*this );
    }

    PipelineManager::~PipelineManager() {
    }

    std::string & PipelineManager::GetTitle() {
        return Title;
    }

    AppDataIO & PipelineManager::GetAppData() {
        return AppData;
    }

    LayoutManager & PipelineManager::GetLayoutManager() {
        return LM;
    }

    PaneTreeManager & PipelineManager::GetPTM() {
        return LM.PTM;
    }

    PaneTree * PipelineManager::GetPaneTree( string inName ) {
        return LM.PTM.GetPaneTree( inName );
    }

    
    void PipelineManager::SetPoll( string inName, bool inPoll ) {
        Poll = LM.SetPoll( inName, inPoll );
    }

    void PipelineManager::RemovePoll( string inName ) {
        Poll = LM.RemovePoll( inName );
    }

    void PipelineManager::ResetPoll() {
        Poll = LM.ResetPoll();
    }

    void PipelineManager::QueueForwardFrames( int inCount ) {
        // TO DO - need to check this is working
        // When the App is in glfwWaitEvents mode, the render function is only called when
        // GLFW detects an event in its event queue. Sometimes, you may need extra frames
        // to complete an action or animation without stalling out. This function will add 
        // empty events to GLFW's queue to trigger the main thread rendering for a extra frames.
        for ( int i = 0; i < inCount; i++ ) {
            glfwPostEmptyEvent();
        }
    }

    // Prepare pipelines

    void PipelineManager::InitPipelines( WindowParameters window ) {

        if ( !InitDX( window ) ) {
            throw std::runtime_error( "Could not initialize Vulkan or DX11!" );
        }

        UILib::UISetWindowHandlePointer( window.WindowPtr );
        UISetWindowTitle( Title );
        
        // Set initial display sizes
        int w, h;
        glfwGetWindowSize(window.WindowPtr, &w, &h);
        UpdateWindowSize(w, h);
        UpdateFramebufferSize(w, h);

        // Init TUI Pipeline
        LM.Init( 0 );


    }
    
    void PipelineManager::UpdateUIInputs(){
        
        UIInputUpdate();

    #ifndef NDEBUG
        auto *kState = &InputState.Key;
        if ( kState->Change == 1 && kState->Key==96 && InputState.Ctrl ) {     // Ctrl + ` to toggle Text Map debugging mode ON / OFF
            ADIO->DebugIDE = !ADIO->DebugIDE; return;
        }
    #endif
    }


    // Drawing
    void PipelineManager::RenderPipelines() {
        
        CheckThreadPoolMessages();

        if(Pause){return;}

        UpdateUIInputs();
        
        AppData.PreRenderSetup();

        ADIO->Dim = UIGetWindowSize();

        LM.DXRender( g_pd3dDeviceContext );

        g_pSwapChain->Present(1, 0); // Present with vsync

        AppData.PostRenderSetup();
        
    }

    void PipelineManager::OnWindowSizeChanged() {
        OnWindowSizeChanged_Pre();
        DXBase::SyncWindowSizeChange();
        if( CanRender ) {
            OnWindowSizeChanged_Post();
        }
    }

    void PipelineManager::OnWindowSizeChanged_Pre() {
        LM.OnWindowSizeChanged_Pre();
    }

    void PipelineManager::OnWindowSizeChanged_Post() {
        LM.OnWindowSizeChanged_Post();
    }

    
    /// --------------------------------------- MESSAGES -----------------------------------------

    void PipelineManager::SendAppMessage(string inFilter, string inType, vector<string> inMessage){
        
        // NOTE: Please make sure the inMessage vector has the correct number of elements needed
        // for the particular message type you are sending. Each type in ListenForMessage is responsible
        // for checking whether it has the correct number of message args.
        
        // Cycle through ALL of the TUI and SUI panes regardless of whether they are visible or in the layout.
        // 0 = Ignored, 1 = Received, 2 = Halt Further Messages
        if(inFilter == "TUI"){
            auto TUIPanes = PMPTR->GetPTM().GetAllTUIVector();
            for(TUI_PaneBase* tuiPane:TUIPanes){
                if ( tuiPane->ListenForMessage(inType, inMessage) == 2 ) break;
            }
        }
        else if ( inFilter == "PM" ) {
            if ( inType == "SetPoll" ) {
                SetPoll( inMessage[0], (inMessage[1] == "true") ? true : false );
            }
        }
        else if ( inFilter == "PPG" ) {
            ADIO->MM.SendPPGMessage( inType, inMessage );
        }
    }
    
    /// --------------------------------------- LISTENERS -----------------------------------------

    void PipelineManager::CheckThreadPoolMessages() {
        int maxmsg = 5, i = 0;
        while ( TPOOL->GetMsgCount() > 0 ) {
            auto msg = TPOOL->GetNextMsg();
            if ( msg.Filter == "SKIP" ) { break; }
            // else if ( msg.Filter == "PPG" ) { ADIO->MM.QueuePPGMessage( msg.Type, msg.Message ); }
            else { SendAppMessage( msg.Filter, msg.Type, msg.Message ); }
            i++;
            if ( i > maxmsg ) { break; }
        }
        TPOOL->ExecMainQueue();
    }


} // end namespace AppCore


#endif  // C_PIPELINEMANAGER_CPP