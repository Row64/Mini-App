#ifndef FOCUSMANAGER_H
#define FOCUSMANAGER_H

#include <map>
#include <string>

namespace Blocks {

    // Focus Manager Types
    enum FocusTypes {

        // Basic Sheet Types
        ft_none = 0,
        ft_chartsheet,

        // Text Inputs
        ft_input_start_enum, // text input section start
        
        ft_input_end_enum,   // text input section end

        // PPG
        ft_ppg,
        ft_ppg_input,

        // Modal Pop Ups
        ft_modal_start_enum, // modal popup section start
        ft_context_menu,
        ft_modal_end_enum,   // modal popup section start

        // Modal Pop Up Text Inputs
        ft_modal_input_start_enum, // modal text input section start
        
        ft_modal_input_end_enum,   // modal text input section end

        // Cursors & Hover Conditions
        ft_cursor_start_enum, // cursor section start
        ft_cursor_color_picker,
        ft_cursor_hand,
        ft_cursor_resize_h,
        ft_cursor_resize_v,
        ft_cursor_resize_tlbr,
        ft_cursor_resize_trbl,
        ft_cursor_plus,
        ft_cursor_move,
        ft_cursor_end_enum,  // cursor section end
        
        ft_over_scroller,
        ft_over_filter_ui,
        
        // Dragging
        ft_drag_start_enum, // drag section start
        ft_drag_color,
        ft_drag_hand,
        ft_drag_resize_h,
        ft_drag_resize_v,
        ft_drag_resize_tlbr,
        ft_drag_resize_trbl,
        ft_drag_selection,
        ft_drag_cell_link,
        ft_drag_scroller,
        ft_drag_plus,
        ft_drag_move,
        ft_drag_end_enum,   // drag section end

        // Escape key to clear focus
        ft_escape

    };

    enum FocusWeights {
        fwt_none                =    0,
        fwt_basic               =   10,
        fwt_init_dragging       =   80,
        fwt_init_modal_ui       =   80,
        fwt_mouse_dragging      =   85,
        fwt_sheet_selection     =   85,
        fwt_rendering           =   85,
        fwt_modal_ui            =   90,
        fwt_escape              =   99,
        fwt_system_ui           =  100
    };
    
    class FocusManager{
        
    private:

        std::map<int, std::array<int,3> >  WeightMap = {     // { FocusType, { FocusWeight, Polled, IsPane } }
            { ft_none               ,   { fwt_none          ,  0,  1  } },
            { ft_chartsheet         ,   { fwt_basic         ,  0,  1  } },

            { ft_ppg                ,   { fwt_basic         ,  0,  0  } },
            { ft_ppg_input          ,   { fwt_basic         ,  1,  0  } },

            { ft_context_menu       ,   { fwt_modal_ui      ,  0,  0  } },

            { ft_cursor_color_picker,   { fwt_modal_ui      ,  0,  0  } },
            { ft_cursor_hand        ,   { fwt_init_dragging ,  0,  0  } },
            { ft_cursor_resize_h    ,   { fwt_init_dragging ,  0,  0  } },
            { ft_cursor_resize_v    ,   { fwt_init_dragging ,  0,  0  } },
            { ft_cursor_resize_tlbr ,   { fwt_init_dragging ,  0,  0  } },
            { ft_cursor_resize_trbl ,   { fwt_init_dragging ,  0,  0  } },
            { ft_cursor_plus        ,   { fwt_init_dragging ,  0,  0  } },
            { ft_cursor_move        ,   { fwt_init_dragging ,  0,  0  } },

            { ft_over_scroller      ,   { fwt_init_dragging ,  0,  0  } },
            { ft_over_filter_ui     ,   { fwt_init_modal_ui ,  0,  0  } },

            { ft_drag_color         ,   { fwt_mouse_dragging,  0,  0  } },
            { ft_drag_hand          ,   { fwt_mouse_dragging,  0,  0  } },
            { ft_drag_resize_h      ,   { fwt_mouse_dragging,  0,  0  } },
            { ft_drag_resize_v      ,   { fwt_mouse_dragging,  0,  0  } },
            { ft_drag_resize_tlbr   ,   { fwt_mouse_dragging,  0,  0  } },
            { ft_drag_resize_trbl   ,   { fwt_mouse_dragging,  0,  0  } },
            { ft_drag_selection     ,   { fwt_mouse_dragging,  0,  0  } },
            { ft_drag_cell_link     ,   { fwt_mouse_dragging,  0,  0  } },
            { ft_drag_scroller      ,   { fwt_mouse_dragging,  0,  0  } },
            { ft_drag_plus          ,   { fwt_mouse_dragging,  0,  0  } },
            { ft_drag_move          ,   { fwt_mouse_dragging,  1,  0  } },

            { ft_escape             ,   { fwt_escape        ,  0,  0  } }           

        };

        // Key Input & Mouse Down Focus
        std::string         FocusName = "";
        int                 FocusType = ft_none;
        int                 FocusWeight = fwt_none;
        int                 FocusIsPane = 0;
        uint32_t            FocusFrameCount = 0u;

        std::string         LastFocusName = "";
        int                 LastFocusType = ft_none;
        int                 LastFocusWeight = fwt_none;
        int                 LastFocusIsPane = 0;
        int                 LastPaneType = ft_none;
        std::string         LastPaneName = "";

        // Mouse Over Focus
        std::string         MFocusName = "";
        int                 MFocusType = ft_none;
        int                 MFocusWeight = fwt_none;

    public:

        FocusManager() = default;
        ~FocusManager() = default;

        void                PreRenderSetup();
        void                PostRenderSetup();

        std::array<int,3>   GetFocusMap( int inType );
        bool                ClickOK( int inWeight = fwt_basic );
        bool                PPGClickOK( int inPPGIndex, int inWeight = fwt_basic );
        bool                AnyClickOK( int inWeight = fwt_basic );

        bool                MouseOK( int inWeight = fwt_basic );                    // Checks ClickOK() and also MFocusWeight
        bool                PPGMouseOK( int inPPGIndex, int inWeight = fwt_basic ); // Checks PPGClickOK() and also MFocusWeight
        bool                AnyMouseOK( int inWeight = fwt_basic );                 // Checks AnyClickOK() and also MFocusWeight

        int                 GetFocusType();
        std::string         GetFocusName();
        int                 GetFocusWeight();
        uint32_t            GetFocusFrameCount();
        int                 GetMFocusType();
        std::string         GetMFocusName();
        int                 GetMFocusWeight();
        int                 GetLastPaneType();
        std::string         GetLastPaneName();
        
        bool                SetFocus( int inType, std::string inName = "" );
        void                ClearFocus();                                           // Clears focus
        void                ClearFocus( int inType );                               // Clears focus if type matches
        void                ClearFocus( int inType, std::string inName );           // Clears focus if type and name match
        void                RevertFocus();                                          // Revert focus
        void                RevertFocus( int inType );                              // Revert focus if type matches
        void                RevertFocus( int inType, std::string inName );          // Revert focus if type and name match
        void                RevertToPane();                                         // Revert focus to last pane
        void                RevertToPane( int inType );                             // Revert focus to last pane if type matches
        void                RevertToPane( int inType, std::string inName );         // Revert focus to last pane if type and name match

        bool                SetMFocus( int inType, std::string inName = "" );
        void                ClearMFocus();                                           // Clears focus
        void                ClearMFocus( int inType );                               // Clears focus if type matches
        void                ClearMFocus( int inType, std::string inName );           // Clears focus if type and name match

        bool                HasFocus( int inType );                                  // Checks if Focus mactches the inType
        bool                HasFocus( int inType, std::string inName );              // Checks if Focus matches the inType and inName
        bool                HasFocusInRange( int inTypeMin, int inTypeMax );         // Checks if Focus falls within the range (inclusive)
        bool                HasMFocus( int inType );                                 // Checks if MFocus mactches the inType
        bool                HasMFocus( int inType, std::string inName );             // Checks if MFocus matches the inType and inName
        bool                HasCursorFocus();                                        // Checks if Focus or MFocus is a cursor type
        bool                HasCursorFocus( int inType );                            // Checks if Focus or MFocus is a cursor type that matches inType
        bool                HasCursorFocus( std::string inName );                    // Checks if Focus or MFocus is a cursor type that matches inName
        bool                HasDragFocus();                                          // Checks if Focus is a drag type
        bool                HasDragFocus( int inType );                              // Checks if Focus is a drag type that matches inType
        bool                HasDragFocus( std::string inName );                      // Checks if Focus is a drag type that matches inName
        bool                HasInputFocus();                                         // Checks if Focus is an input type
        bool                HasInputFocus( int inType );                             // Checks if Focus is an input type that matches inType
        bool                HasInputFocus( std::string inName );                     // Checks if Focus is an input type that matches inName                                      

        void                LogFocus();

        // Special focus helpers
        void                SetUITransformBBFocus( int inType, std::string inName, bool MuteSize = false, bool MuteDrag = false );
    };

}



#endif /* FOCUSMANAGER_H */
