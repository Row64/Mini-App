#ifndef USER_LAYOUTS_CPP
#define USER_LAYOUTS_CPP

#include "c_PipelineManager.h"

namespace AppCore {


    void LayoutManager::InitLayouts() {
        
        PTM.AddTUI<Overlay> ( "Overlay" );
        
        PTM.AddTUI<ChartSheet> ( "Canvas" );
        
        // =============PaneTreeName====LazyUpdate===Name=============ParentName======WidthExp=====HeightExp===Split=======Mode====FloatX==FloatY===  
        PTM.AddPaneTree( "WorkSpace", 0,    {       {"Main",           "",              "*",        "*",         "y",        "",     "",     ""     },
                                                    {"Canvas",         "Main",          "*",        "*",         "",         "",     "",     ""     },
                                                    {"Overlay",        "Main",          "*",        "*",         "",         "",     "0",    "0"    }
        });
        
        
        SetCurrentLayout( "WorkSpace" );      // Or you can set by index of Pane Tree:
        //SetCurrentLayout( "WorkSpace X Split" );      // Or you can set by index of Pane Tree:

        // The issue has to do with the SliderMove method in DataSheet
    }
    
    void LayoutManager::InitStyles() {
        
        // Set default style elements here
        // ImGui::StyleColorsLight();
        ImGuiStyle &gui_style = ImGui::GetStyle();
        gui_style.WindowMinSize = ImVec2(20, 20);
        
        //fs::path bold_path = AppData.RootPath / "Data/FreeSansBold.ttf";
        
        //=========================== INITIALIZE FONTS ===========================
        InitFonts();

        //=========================== CUSTOM FORMATTING ===========================

        UISetDefaultScroller( AppData.DefaultScroller );

        //=========================== CUSTOM CURSORS ===========================

        UIAddCustomCursor( "DROPPER", (AppData.RootPath / "Data/Icons/cursor_dropper_18px.png").string(), 1, 16 );
        UIAddCustomCursor( "RESIZER_TLBR", (AppData.RootPath / "Data/Icons/resizer_tlbr_18px.png").string(), 9, 9 );
        UIAddCustomCursor( "RESIZER_BLTR", (AppData.RootPath / "Data/Icons/resizer_bltr_18px.png").string(), 9, 9 );
        UIAddCustomCursor( "GRABBER", (AppData.RootPath / "Data/Icons/hand_grabber_18px.png").string(), 9, 9 );
        UIAddCustomCursor( "PLUS", (AppData.RootPath / "Data/Icons/plus_drag_18px.png").string(), 9, 9 );

    }
} // end namespace AppCore

#endif // USER_LAYOUTS_CPP