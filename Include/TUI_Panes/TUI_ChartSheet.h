#ifndef TUI_CHARTSHEET_H
#define TUI_CHARTSHEET_H

#include "TUI_PaneBase.h"

namespace Blocks {
    class ChartSConfig;
}
class CLO_Base;
class CDR_Base;

namespace AppCore {

    // ************************************************************ //
    // ChartSheet Class                                             //
    // ************************************************************ //

    class ChartSheet : virtual public TUI_PaneBase {
    
    private: 
       
    public:

        ChartSheet( PipelineManager &parent );
        ~ChartSheet();

        int PaneIndex = 0;          // for dual pane views, 0 = A, 1 = B
        bool PaneActive = true;
        bool Linked = false;

        Blocks::ChartSConfig *      Config = nullptr;
        std::unique_ptr<CLO_Base>   LoadObj;    // Always create the LoadObj when ConfigHS() is called, and delete it when Config is unlinked
        std::unique_ptr<CDR_Base>   DrawObj;    // Always create the DrawObj when ConfigHS() is called, and delete it when Config is unlinked

        // =============== Override Functions from PaneBase ===============

        void            InitPane() override final;
        void            DrawPane() override final;

        int             ListenForMessage(string inType, vector<string> inMessage) override final;
        void *          GetSelf() override final { return this; };

        void            ConfigHS();

    protected:
        

        // =============== Listen For Messaage Receiver Functions ===============
        
        // =============== Receiver Function Helpers ===============

    };
    

} // end namespace AppCore

#endif // TUI_CHARTSHEET_H