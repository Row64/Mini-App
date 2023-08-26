#ifndef TUI_OVERLAY_H
#define TUI_OVERLAY_H

#include "TUI_PaneBase.h"

namespace AppCore {
    
    // ************************************************************ //
    // TabBar Class                                                 //
    // ************************************************************ //
    class Overlay : virtual public TUI_PaneBase {
    
    public:

        Overlay( PipelineManager &parent );
        ~Overlay();
        
        int                 ListenForMessage(string inType, vector<string> inMessage) override final;
        void                DrawPane() override final;
        void                StartPane() override final;
        void                FinishPane() override final;

    protected:

        void                DrawUI();

        void                DrawMouseCursor();
        void                DrawComboList();
        void                FileDrop();

    private:

  
    };
    
} // end namespace AppCore

#endif // TUI_OVERLAY_H