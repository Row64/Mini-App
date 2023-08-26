#ifndef TUI_OVERLAY_CPP
#define TUI_OVERLAY_CPP

#include "c_PipelineManager.h"

#if defined( _WIN32 )
    #include <windows.h>
    #include <tchar.h>
#endif

#include <iostream>
#include <chrono>
using namespace std;
using namespace chrono;

namespace AppCore {

    Overlay::Overlay( PipelineManager &parent ) :
       TUI_PaneBase( parent ) {
        PaneType = "Overlay";
    }
    
    Overlay::~Overlay() {
    }
    void Overlay::DrawPane() {
        StartPane();
        DrawUI();
        FinishPane();
    }

    void Overlay::StartPane() {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
        ImGui::SetNextWindowPos( ImVec2( X, Y ) );
        ImGui::SetNextWindowSize( ImVec2( Width, Height ) );
        ImGui::Begin( PaneName.c_str(), NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    }

    void Overlay::FinishPane() {
        ImGui::End();
        ImGui::PopStyleVar(6);
        ImGui::PopStyleColor(2);
    }
    
    int Overlay::ListenForMessage(string inType, vector<string> inMessage){
        // if(inType == "SheetZoomChange" || inType == "CodeZoomChange"){
        //     ZoomUpdate = true;
        // }
        return 0;
        
    }
    
    void Overlay::DrawUI(){
        
        ImGui::SetWindowFocus();  // Keep the focus on the Overlay UI.  ImGui can get order wrong if you don't
        
        FileDrop();
        DrawMouseCursor();

        ADIO->MM.ProcessQueuedMessages(); // Process queued PPG messages
        DrawPPGs(Width, Height);          // Draw PPGs

        DrawModalPopUps(Width, Height);

        // Draw Non-PPG Text Input
        if ( !ADIO->FM.HasFocus( ft_ppg_input ) ) {
            if( ADIO->FM.HasInputFocus() && UILibCTX->InputBoxMode == inputbox_default){
                UIDrawTextInput();
            }
            else if( ADIO->FM.HasInputFocus() && UILibCTX->InputBoxMode == inputbox_password){
                UIDrawPasswordInput();
            }
        }
        
        DrawComboList();

    }

    void Overlay::DrawMouseCursor(){
        if      ( AppData.FM.HasFocus( ft_cursor_color_picker ) ) { UISetCursor( uic_custom, "DROPPER" ); }
        else if ( AppData.FM.HasDragFocus(   ft_drag_hand     ) ) { UISetCursor( uic_custom, "GRABBER" ); }
        else if ( AppData.FM.HasCursorFocus( ft_cursor_hand   ) ) { UISetCursor( uic_hand ); }
        else if ( AppData.FM.HasCursorFocus( ft_cursor_resize_tlbr ) || AppData.FM.HasDragFocus( ft_drag_resize_tlbr ) ) { UISetCursor( uic_resize_tlbr ); }
        else if ( AppData.FM.HasCursorFocus( ft_cursor_resize_trbl ) || AppData.FM.HasDragFocus( ft_drag_resize_trbl ) ) { UISetCursor( uic_resize_trbl ); }
        else if ( AppData.FM.HasCursorFocus( ft_cursor_resize_h    ) || AppData.FM.HasDragFocus( ft_drag_resize_h    ) ) { UISetCursor( uic_resize_h ); }
        else if ( AppData.FM.HasCursorFocus( ft_cursor_resize_v    ) || AppData.FM.HasDragFocus( ft_drag_resize_v    ) ) { UISetCursor( uic_resize_v ); }
        else if ( AppData.FM.HasCursorFocus( ft_cursor_move        ) || AppData.FM.HasDragFocus( ft_drag_move        ) ) { UISetCursor( uic_resize_all ); }
        else if ( AppData.FM.HasCursorFocus( ft_cursor_plus        ) || AppData.FM.HasDragFocus( ft_drag_plus        ) ) { UISetCursor( uic_custom, "PLUS" ); }
        else{UISetCursor( uic_normal );}

    }

    void Overlay::DrawComboList(){
        if(UILibCTX->ComboMode == 1){
            UIDrawComboItems(UIComboGetFocusHandle(), UILibCTX->ComboRect, UILibCTX->ComboItems, UILibCTX->ComboScrollPerc, UILibCTX->ComboDragging);
        }
    }
    
    void Overlay::FileDrop(){
        // Drag and drop a file or project folder into the app. 
        // This will try to open the file or project.
        if(UILibIS->DropPath != ""){
            fs::path fPath = UILibIS->DropPath;
            if(ADIO->FM.ClickOK()){
                std::cout << "FileDrop: " << UILibIS->DropPath << std::endl;
                UILibIS->DropPath = "";
            }
        }
    }

} // end namespace AppCore

#endif // TUI_OVERLAY_CPP