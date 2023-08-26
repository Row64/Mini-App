#ifndef TUI_PANEBASE_CPP
#define TUI_PANEBASE_CPP

#include "TUI_PaneBase.h"
#include "c_PipelineManager.h"

namespace AppCore {

    TUI_PaneBase::TUI_PaneBase( PipelineManager &parent ) :
        X(),
        Y(),
        Width(),
        Height(),
        Parent( parent ),
        AppData( parent.GetAppData() ) {
    }

    TUI_PaneBase::~TUI_PaneBase() {
    }
    
    void TUI_PaneBase::UpdateDimensions( float inX, float inY, float inW, float inH ) {
        X = inX;
        Y = inY;
        Width = inW;
        Height = inH;
    }
    
    void TUI_PaneBase::UpdateMode( string inMode ) {
        Mode = inMode;
    }

    void TUI_PaneBase::InitPane() {
        // Override InitPane() to put initialization code that is called ONCE in the lifetime of the pane.
        // InitPane() is called by the PaneTreeManager::AddTUI after the the object is constructed and assigned a PaneName.
    }
    
    void TUI_PaneBase::DrawPane() {
        StartPane();
        FinishPane();
    }
    
    void TUI_PaneBase::StartPane() {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImGui::SetNextWindowPos( ImVec2( X, Y ) );
        ImGui::SetNextWindowSize( ImVec2( Width, Height ) );
        ImGui::Begin( PaneName.c_str(), NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    }

    void TUI_PaneBase::FinishPane() {
        ImGui::End();
        ImGui::PopStyleVar(6);
        ImGui::PopStyleColor(2);
    }

    int TUI_PaneBase::ListenForMessage(string inType, vector<string> inMessage){ return 0; }
    
}

#endif // TUI_PANEBASE_CPP