#ifndef C_PIPELINEMANAGER_H
#define C_PIPELINEMANAGER_H

#include "UILib.h"
#include "StringLib.h"
#include "MathLib.hpp"

#include "c_LayoutManager.h"

using namespace SLib;
using namespace MLib;

namespace AppCore {

    // ************************************************************ //
    // PipelineManager Class                                        //
    // ************************************************************ //
    class PipelineManager : public DXBase, public WindowBase {

    public:

        PipelineManager( std::string const & title, AppDataIO & inAppData );
        ~PipelineManager();
            
        std::string &               GetTitle();
        AppDataIO &                 GetAppData();
        LayoutManager &             GetLayoutManager();
        PaneTreeManager &           GetPTM();
        PaneTree *                  GetPaneTree( string inName );

        void                        SetPoll( string inName, bool inPoll ) final override;
        void                        RemovePoll( string inName ) final override;
        void                        ResetPoll() final override ;
        void                        QueueForwardFrames( int inCount = 1 ) final override;

        void                        InitPipelines( WindowParameters window );
        void                        RenderPipelines() final override;

        void                        UpdateUIInputs();

        void                        SendAppMessage(string Filter, string inType, vector<string> inMessage);
        
        void                        OnWindowSizeChanged() final override;


    private:

        std::string                 Title;
        AppDataIO &                 AppData;
        LayoutManager               LM;
        
        // Drawing methods

        void                        OnWindowSizeChanged_Pre();
        void                        OnWindowSizeChanged_Post();
        void                        LogKeyState(string inStr);

        // Listener Methods
        void                        CheckThreadPoolMessages();
        
    };


} // end namespace AppCore


#endif  // C_PIPELINEMANAGER_H