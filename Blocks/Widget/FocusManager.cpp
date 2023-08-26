#ifndef FOCUSMANAGER_CPP
#define FOCUSMANAGER_CPP


#include "c_PipelineManager.h"
using namespace AppCore;

#include <iostream>
#include <string>
#include <vector>

using namespace std;
using namespace glm;

// #define LOG_FOCUS   // Uncomment for logging

namespace Blocks {

    void FocusManager::PreRenderSetup() {
        // ESCAPE key to force clear all other focuses
        // Exceptions are text input, which resolves escape itself,
        // and SystemUI
        if ( !HasInputFocus() && FocusWeight < fwt_system_ui ) {
            int curr_key = PMPTR->GetInputState().Key.Key;
            bool key_change = ( PMPTR->GetInputState().Key.Change == 1 );
            if ( key_change ) {
                if ( curr_key == 256 ) {
                    SetFocus( ft_escape, "FM ESCAPE" );
                }
            }
        }

        // Clear CONTEXT MENU when there is a click outside of its BB
        if ( HasFocus(ft_context_menu) ) {
            vec2 mPos = GetGlobalMousePos();
            if( !VecInBB(mPos, ADIO->CurrentBB) && ( DragStart() || RightDragStart() ) ) { 
                RevertToPane( ft_context_menu );
                ADIO->CurrentBB = {};
                return; 
            }
        }
    }

    void FocusManager::PostRenderSetup() {
        FocusFrameCount++;
        ClearFocus( ft_escape, "FM ESCAPE" );
    }

    std::array<int,3> FocusManager::GetFocusMap( int inType ) {
        auto it = WeightMap.find(inType); 
        if ( it != WeightMap.end() ) { 
            return it->second; 
        }
        return {-1,-1};
    }

    bool FocusManager::ClickOK( int inWeight ) {
        return ( inWeight >= FocusWeight ) && ADIO->MM.AvoidPPG();
    }

    bool FocusManager::PPGClickOK( int inPPGIndex, int inWeight ) {
        return ( inWeight >= FocusWeight ) && ADIO->MM.MouseHit( inPPGIndex );
    }

    bool FocusManager::AnyClickOK( int inWeight ) {
        return ( inWeight >= FocusWeight );
    }

    bool FocusManager::MouseOK( int inWeight ) {
        return ( inWeight >= MFocusWeight ) && ClickOK( inWeight );
    }

    bool FocusManager::PPGMouseOK( int inPPGIndex, int inWeight ) {
        return ( inWeight >= MFocusWeight ) && ADIO->MM.MouseHit( inPPGIndex );
    }

    bool FocusManager::AnyMouseOK( int inWeight ) {
        return ( inWeight >= MFocusWeight ) && AnyClickOK(( inWeight) );
    }

    int FocusManager::GetFocusType() {
        return FocusType;
    }

    std::string FocusManager::GetFocusName() {
        return FocusName;
    }

    int FocusManager::GetFocusWeight() {
        return FocusWeight;
    }

    uint32_t FocusManager::GetFocusFrameCount() {
        return FocusFrameCount;
    }

    int FocusManager::GetMFocusType() {
        return MFocusType;
    }

    std::string FocusManager::GetMFocusName() {
        return MFocusName;
    }

    int FocusManager::GetMFocusWeight() {
        return MFocusWeight;
    }

    int FocusManager::GetLastPaneType() {
        return LastPaneType;
    }

    std::string FocusManager::GetLastPaneName() {
        return LastPaneName;
    }


    bool FocusManager::SetFocus( int inType, std::string inName ) {
        // inType:  0=None, 1=Spreadsheet, 2=DataFrame, 3=ImportSheet, 4=Chart, 5=Notebook
        // 10=Worksheet Formula, 11=DF Formula In (left), 12= DF Python (right),  13=DF Python Out, 14 = View Cell Details
        // 100+ = PPG
        if ( inType == FocusType && inName == FocusName ) { 
            #ifdef LOG_FOCUS
            // cout << "Keep Focus:  " << inType << " -> " << inName << "\n";
            #endif
            return true; 
        }
        std::array<int,3> info = GetFocusMap( inType );
        if ( info[0] >= FocusWeight ) {
            PMPTR->SetPoll( "FocusManager"+to_string(FocusType), false );                       // unset poll for the old focus
            if ( info[1] == 1 ) { PMPTR->SetPoll( "FocusManager"+to_string(inType), true ); }   // set poll for the new focus
            LastFocusType = FocusType;
            LastFocusName = FocusName;
            LastFocusWeight = FocusWeight;
            LastFocusIsPane = FocusIsPane;
            if ( FocusIsPane == 1 ) { 
                LastPaneType = FocusType; 
                LastPaneName = FocusName;
            }
            FocusType = inType;
            FocusName = inName;
            FocusWeight = info[0];
            FocusIsPane = info[2];
            FocusFrameCount = 0;
            #ifdef LOG_FOCUS
            cout << "Set Focus:   " << inType << " -> " << inName << ", Weight: " << info[0] << "\n";
            #endif
            return true;
        }
        return false;
    }

    void FocusManager::ClearFocus() {
        #ifdef LOG_FOCUS
        cout << "Clear Focus: " << FocusType << " -> " << FocusName << "\n";
        #endif
        PMPTR->SetPoll( "FocusManager"+to_string(FocusType), false );       // unset poll for the old focus
        LastFocusType = FocusType;
        LastFocusName = FocusName;
        LastFocusWeight = FocusWeight;
        LastFocusIsPane = FocusIsPane;
        if ( FocusIsPane == 1 ) { 
            LastPaneType = FocusType; 
            LastPaneName = FocusName;
        }
        FocusType = ft_none;
        FocusName = "";
        FocusWeight = fwt_none;
        FocusIsPane = 0;
        FocusFrameCount = 0;
    }

    void FocusManager::ClearFocus( int inType ) {
        if ( inType == FocusType ) {
            ClearFocus();
        }
    }

    void FocusManager::ClearFocus( int inType, std::string inName ) {
        if ( inType == FocusType && inName == FocusName ) {
            ClearFocus();
        }
    }

    void FocusManager::RevertFocus() {
        int lastFocusType = FocusType;
        string lastFocusName = FocusName;
        int lastFocusWeight = FocusWeight;
        int lastFocusIsPane = FocusIsPane;
        if ( FocusIsPane == 1 ) { 
            LastPaneType = FocusType; 
            LastPaneName = FocusName;
        }
        FocusType = LastFocusType;
        FocusName = LastFocusName;
        FocusWeight = LastFocusWeight;
        FocusIsPane = LastFocusIsPane;
        FocusFrameCount = 0;
        LastFocusType = lastFocusType;
        LastFocusName = lastFocusName;
        LastFocusWeight = lastFocusWeight;
        LastFocusIsPane = lastFocusIsPane;
        std::array<int,3> info = GetFocusMap( FocusType );
        PMPTR->SetPoll( "FocusManager"+to_string(LastFocusType), false );                       // unset poll for the old focus
        if ( info[1] == 1 ) { PMPTR->SetPoll( "FocusManager"+to_string(FocusType), true ); }    // set poll for the new focus
        #ifdef LOG_FOCUS
        cout << "Revert Focus: " << FocusType << ", " << FocusName << std::endl;
        #endif
    }

    void FocusManager::RevertFocus( int inType ) {
        if ( inType == FocusType ) {
            RevertFocus();
        }
    }

    void FocusManager::RevertFocus( int inType, std::string inName ) {
        if ( inType == FocusType && inName == FocusName ) {
            RevertFocus();
        }
    }

    void FocusManager::RevertToPane() {
        PMPTR->SetPoll( "FocusManager"+to_string(FocusType), false );       // unset poll for the old focus
        LastFocusType = FocusType;
        LastFocusName = FocusName;
        LastFocusWeight = FocusWeight;
        LastFocusIsPane = FocusIsPane;
        FocusType = LastPaneType;
        FocusName = LastPaneName;
        std::array<int,3> info = GetFocusMap( FocusType );
        if ( info[1] == 1 ) { PMPTR->SetPoll( "FocusManager"+to_string(FocusType), true ); }    // set poll for the new focus
        FocusWeight = info[0];
        FocusIsPane = info[2];
        FocusFrameCount = 0;
        if ( LastFocusIsPane == 1 ) { 
            LastPaneType = LastFocusType; 
            LastPaneName = LastFocusName;
        }
        #ifdef LOG_FOCUS
        cout << "Revert Focus To Pane: " << FocusType << ", " << FocusName << std::endl;
        #endif
    }

    void FocusManager::RevertToPane( int inType ) {
        if ( inType == FocusType ) {
            RevertToPane();
        }
    }

    void FocusManager::RevertToPane( int inType, std::string inName ) {
        if ( inType == FocusType && inName == FocusName ) {
            RevertToPane();
        }
    }

    bool FocusManager::SetMFocus( int inType, std::string inName ) {
        // inType:  0=None, 1=Spreadsheet, 2=DataFrame, 3=ImportSheet, 4=Chart, 5=Notebook
        // 10=Worksheet Formula, 11=DF Formula In (left), 12= DF Python (right),  13=DF Python Out, 14 = View Cell Details
        // 100+ = PPG
        if ( inType == MFocusType && inName == MFocusName ) { 
            #ifdef LOG_FOCUS
            // cout << "Keep MFocus:  " << inType << " -> " << inName << "\n";
            #endif
            return true; 
        }
        std::array<int,3> info = GetFocusMap( inType );
        if ( info[0] >= MFocusWeight && info[0] >= FocusWeight ) {  // In order grab mouse focus, the new weight must be equal or higher than BOTH focus weights.
            MFocusType = inType;
            MFocusName = inName;
            MFocusWeight = info[0];
            #ifdef LOG_FOCUS
            cout << "Set MFocus:   " << inType << " -> " << inName << ", Weight: " << info[0] << "\n";
            #endif
            return true;
        }
        return false;


    }

    void FocusManager::ClearMFocus() {
        #ifdef LOG_FOCUS
        cout << "Clear MFocus: " << FocusType << " -> " << MFocusName << "\n";
        #endif
        MFocusType = ft_none;
        MFocusName = "";
        MFocusWeight = fwt_none;
    }

    void FocusManager::ClearMFocus( int inType ) {
        if ( inType == MFocusType ) {
            ClearMFocus();
        }
    }

    void FocusManager::ClearMFocus( int inType, std::string inName ) {
        if ( inType == MFocusType && inName == MFocusName ) {
            ClearMFocus();
        }
    }

    bool FocusManager::HasFocus( int inType ) {
        return ( inType == FocusType );
    }

    bool FocusManager::HasFocus( int inType, std::string inName ) {
        return ( inType == FocusType && inName == FocusName );
    }

    bool FocusManager::HasFocusInRange( int inTypeMin, int inTypeMax ) {
        return ( FocusType >= inTypeMin && FocusType <= inTypeMax );
    }

    bool FocusManager::HasMFocus( int inType ) {
        return ( inType == MFocusType );
    }

    bool FocusManager::HasMFocus( int inType, std::string inName ) {
        return ( inType == MFocusType && inName == MFocusName );
    }

    bool FocusManager::HasCursorFocus() {
        if ( FocusWeight > MFocusWeight ) { return false; }
        if ( MFocusType > ft_cursor_start_enum && MFocusType < ft_cursor_end_enum ) return true;
        if ( FocusType > ft_cursor_start_enum && FocusType < ft_cursor_end_enum ) return true;
        return false;
    }

    bool FocusManager::HasCursorFocus( int inType ) {
        if ( FocusWeight > MFocusWeight ) { return false; }
        if ( MFocusType > ft_cursor_start_enum && MFocusType < ft_cursor_end_enum  && MFocusType == inType ) return true;
        if ( FocusType > ft_cursor_start_enum && FocusType < ft_cursor_end_enum  && FocusType == inType ) return true;
        return false;
    }

    bool FocusManager::HasCursorFocus( std::string inName ) {
        if ( FocusWeight > MFocusWeight ) { return false; }
        if ( MFocusType > ft_cursor_start_enum && MFocusType < ft_cursor_end_enum  && MFocusName == inName ) return true;
        if ( FocusType > ft_cursor_start_enum && FocusType < ft_cursor_end_enum  && FocusName == inName ) return true;
        return false;
    }

    bool FocusManager::HasDragFocus() {
        return ( FocusType > ft_drag_start_enum && FocusType < ft_drag_end_enum );
    }

    bool FocusManager::HasDragFocus( int inType ) {
        return ( FocusType > ft_drag_start_enum && FocusType < ft_drag_end_enum  && FocusType == inType );
    }

    bool FocusManager::HasDragFocus( std::string inName ) {
        return ( FocusType > ft_drag_start_enum && FocusType < ft_drag_end_enum  && FocusName == inName );
    }

    bool FocusManager::HasInputFocus() {
        return ( FocusType > ft_input_start_enum && FocusType < ft_input_end_enum ) || ( FocusType > ft_modal_input_start_enum && FocusType < ft_modal_input_end_enum ) || FocusType == ft_ppg_input;
    }

    bool FocusManager::HasInputFocus( int inType ) {
        return ( ( ( FocusType > ft_input_start_enum && FocusType < ft_input_end_enum ) || ( FocusType > ft_modal_input_start_enum && FocusType < ft_modal_input_end_enum ) || FocusType == ft_ppg_input ) && FocusType == inType );
    }

    bool FocusManager::HasInputFocus( std::string inName ) {
        return ( ( ( FocusType > ft_input_start_enum && FocusType < ft_input_end_enum ) || ( FocusType > ft_modal_input_start_enum && FocusType < ft_modal_input_end_enum ) || FocusType == ft_ppg_input ) && FocusName == inName );
    }

    void FocusManager::LogFocus() {
        printf( "CURRENT FocusType: %i, FocusName: %s, MFocusType: %i, MFocusName: %s\n", FocusType, FocusName.c_str(), MFocusType, MFocusName.c_str() );
    }

    void FocusManager::SetUITransformBBFocus( int inType, std::string inName, bool MuteSize, bool MuteDrag ) {

        if ( inType <= 0 ) {
            if ( HasDragFocus( inName ) ) { RevertToPane(); }
            else if ( HasCursorFocus( inName ) ) { ClearMFocus(); }
            return;
        }

        // enums: tt_none, tt_drag, tt_resize_tl, tt_resize_br, tt_resize_tr, tt_resize_bl, tt_resize_l, tt_resize_r, tt_resize_t, tt_resize_b
        vector<int> overType = { ft_cursor_hand, ft_cursor_resize_tlbr, ft_cursor_resize_trbl, ft_cursor_resize_h, ft_cursor_resize_v };
        vector<int> downType = {   ft_drag_hand,   ft_drag_resize_tlbr,   ft_drag_resize_trbl,   ft_drag_resize_h,   ft_drag_resize_v };
        vector<bool> allowType = {    !MuteDrag,             !MuteSize,             !MuteSize,          !MuteSize,          !MuteSize };

        for ( int i = 0; i < overType.size(); i++ ) {
            if ( allowType[i] && (inType == 2*i || inType == 2*i+1) ) {
                if ( MouseDown() ) {
                    ClearMFocus( overType[i], inName );
                    SetFocus( downType[i], inName );
                }
                else {
                    SetMFocus( overType[i], inName );
                    RevertToPane( downType[i], inName );
                }                
            }
            else {
                ClearMFocus( overType[i], inName );
                RevertToPane( downType[i], inName );
            }
        }
       
    }

}

#endif /* FOCUSMANAGER_CPP */
