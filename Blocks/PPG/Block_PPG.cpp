#ifndef BLOCK_PPG_CPP
#define BLOCK_PPG_CPP

#include "c_PipelineManager.h"

 #include "Block_PPG.h"

// #include "IO_Tools.h"
// using namespace IO_Tools;
using namespace AppCore;

namespace Blocks {

    void DrawPPGs(float inW, float inH){
        for(int i=0;i<ADIO->OpenPPGs.size();i++){
            PropertyPage & ppg = ADIO->OpenPPGs[i];
            DrawBasePPG(ppg, i, true);
            if ( !ppg.Collapsed ) { ppg.DrawFunction(ppg, inW, inH); }
            ppg.EvalCount++;
            if ( ppg.EvalCount == INT_MAX ) { ppg.EvalCount = 1; }
            ppg.UpdateAutoHide();
        }
    }

    PropertyPage * OpenSubPPG(PropertyPage &inPPG, string inName, vec2 inDim, int inRefIndex, float inW, float inH, vec2 inPos){
        
        inPPG.Pinned = true; // ppg that opened sub menu should be pinned
        int inPPGIndex = inPPG.Index; 

        // figure out if you are on the left or the right
        float newPW = inDim.x; //270;
        float newPH = inDim.y; // 360;
        float midPpg = inPPG.Pos.x  + inPPG.Dim.x * .5f;
        float midScreen = inW * .5f;
        float spacerBetweenPpg = 10;
        float ppx;
        float ppy = ( inPos.y > 0 ) ? inPos.y : inPPG.Pos.y ;
        if(midPpg < midScreen){ // you are on the right side, open sub PPG on left
            ppx = inPPG.Pos.x  + inPPG.Dim.x + spacerBetweenPpg;
        }
        else{ //you are on the left side, open sub PPG on right 
            ppx = inPPG.Pos.x - spacerBetweenPpg - newPW;
        }

        ImRect subRect = {ppx,ppy,ppx+newPW,ppy+newPH};
        ImRect boundaries = GetGlobalBB(0,0,inW, inH);
        ImRect fitRect = FitBoxinBB(subRect, boundaries);

        PropertyPage & subPpg = ADIO->MM.AddPPG( inName, false, {fitRect.Min.x,fitRect.Min.y},{newPW,newPH} );
        subPpg.RefIndex = inRefIndex;
        ADIO->OpenPPGs[inPPGIndex].Children.push_back( subPpg.Index );
        ADIO->MM.MovePPGToTop( inPPGIndex );

        return &subPpg;

    }

    
    void DrawBasePPG(PropertyPage & inPPG, int inI, bool inDrawName){
        
        // If it is the first time evaluating the PPG, call the InitFunction.
        if ( inPPG.EvalCount == 0 ) { 
            inPPG.InitFunction( inPPG );
            inPPG.UpdateBaseDim();
        }

        // Get mode settings
        bool CanDrag = true;
        bool CanResizeL  = true, CanResizeR  = true, CanResizeT  = true, CanResizeB  = true;
        bool CanResizeTL = true, CanResizeTR = true, CanResizeBR = true, CanResizeBL = true;
        bool ShowDropShadow = inPPG.ShowDropShadow;
        int ppgMode = inPPG.GetMode();
        if ( ppgMode != PropertyPage::mode_float && !inPPG.ModeRefPane.empty() ) {
            auto * Pane = PMPTR->GetPTM().GetTUI( inPPG.ModeRefPane );
            if ( Pane ) {
                CanDrag = false;
                ShowDropShadow = false;
                if      ( ppgMode == PropertyPage::mode_fill_pane ) {
                    CanResizeL = CanResizeR = CanResizeT = CanResizeB = CanResizeTL = CanResizeTR = CanResizeBR = CanResizeBL = false;
                    inPPG.Pos.x = Pane->X; inPPG.Pos.y = Pane->Y; 
                    inPPG.Dim.x = Pane->Width; inPPG.Dim.y = Pane->Height;
                }
                else if ( ppgMode == PropertyPage::mode_top_dock || ppgMode == PropertyPage::mode_top_auto_hide ) {
                    CanResizeT = CanResizeTL = CanResizeTR = CanResizeBL = CanResizeBR = false;
                    inPPG.Pos.x = Pane->X; inPPG.Pos.y = Pane->Y; 
                    inPPG.Dim.x = Pane->Width;
                }
                else if ( ppgMode == PropertyPage::mode_right_dock || ppgMode == PropertyPage::mode_right_auto_hide ) {
                    CanResizeR = CanResizeTR = CanResizeBR = CanResizeTL = CanResizeBL = false;
                    inPPG.Pos.x = Pane->X+Pane->Width-inPPG.Dim.x; inPPG.Pos.y = Pane->Y; 
                    inPPG.Dim.y = Pane->Height;
                }
                else if ( ppgMode == PropertyPage::mode_bottom_dock || ppgMode == PropertyPage::mode_bottom_auto_hide ) {
                    CanResizeB = CanResizeBL = CanResizeBR = CanResizeTL = CanResizeTR = false;
                    inPPG.Pos.x = Pane->X; inPPG.Pos.y = Pane->Y+Pane->Height-inPPG.Dim.y; 
                    inPPG.Dim.x = Pane->Width;
                }
                else if ( ppgMode == PropertyPage::mode_left_dock || ppgMode == PropertyPage::mode_left_auto_hide ) {
                    CanResizeL = CanResizeTR = CanResizeBR = CanResizeTL = CanResizeBL = false;
                    inPPG.Pos.x = Pane->X; inPPG.Pos.y = Pane->Y;
                    inPPG.Dim.y = Pane->Height;
                }
            }
        }

        vec2 mPos = GetGlobalMousePos();

        // Get the PPG rectangle in global space. PPGs are always in Global space, so GetBB and GetGlobalBB would return the same thing.
        ImRect pRect = GetBB(inPPG.Pos.x, inPPG.Pos.y, inPPG.Dim.x, inPPG.Dim.y);
        float rsSize = ADIO->MM.DragArea;

        // Update pRect if Resizing or Dragging
        auto ap = ADIO->MM.GetActivePPG();
        if( ap.first == inI ) {
            if( MouseDown() ) {
                vec2 mDelta = GetMouseDelta();
                if( ap.second == "ResizeTL" ) {
                    vec2 newPos = inPPG.Pos + mDelta;
                    vec2 newDim = inPPG.Dim - mDelta;
                    if      ( mDelta.x < 0 && mPos.x > inPPG.Pos.x                ) { newPos.x = inPPG.Pos.x; newDim.x = inPPG.Dim.x; }
                    else if ( mDelta.x > 0 && mPos.x < inPPG.Pos.x                ) { newPos.x = inPPG.Pos.x; newDim.x = inPPG.Dim.x; }
                    else if ( mPos.x > inPPG.Pos.x + inPPG.Dim.x - inPPG.MinDim.x ) { newPos.x = inPPG.Pos.x; newDim.x = inPPG.Dim.x; }
                    if      ( mDelta.y < 0 && mPos.y > inPPG.Pos.y                ) { newPos.y = inPPG.Pos.y; newDim.y = inPPG.Dim.y; }
                    else if ( mDelta.y > 0 && mPos.y < inPPG.Pos.y                ) { newPos.y = inPPG.Pos.y; newDim.y = inPPG.Dim.y; }
                    else if ( mPos.y > inPPG.Pos.y + inPPG.Dim.y - inPPG.MinDim.y ) { newPos.y = inPPG.Pos.y; newDim.y = inPPG.Dim.y; }                   
                    pRect = GetBB(newPos.x, newPos.y, newDim.x, newDim.y );
                    if ( pRect.Min.x < 0 ) { pRect.Min.x = 0; }
                    if ( pRect.Min.y < 0 ) { pRect.Min.y = 0; }
                }
                else if( ap.second == "ResizeTR" ) {
                    vec2 newPos = inPPG.Pos + vec2( 0, mDelta.y );
                    vec2 newDim = inPPG.Dim + vec2( mDelta.x, -mDelta.y );
                    if      ( mDelta.x > 0 && mPos.x < inPPG.Pos.x + inPPG.Dim.x  ) {                         newDim.x = inPPG.Dim.x; }
                    else if ( mDelta.x < 0 && mPos.x > inPPG.Pos.x + inPPG.Dim.x  ) {                         newDim.x = inPPG.Dim.x; }
                    else if ( mPos.x < inPPG.Pos.x + inPPG.MinDim.x               ) {                         newDim.x = inPPG.Dim.x; }
                    if      ( mDelta.y < 0 && mPos.y > inPPG.Pos.y                ) { newPos.y = inPPG.Pos.y; newDim.y = inPPG.Dim.y; }
                    else if ( mDelta.y > 0 && mPos.y < inPPG.Pos.y                ) { newPos.y = inPPG.Pos.y; newDim.y = inPPG.Dim.y; }
                    else if ( mPos.y > inPPG.Pos.y + inPPG.Dim.y - inPPG.MinDim.y ) { newPos.y = inPPG.Pos.y; newDim.y = inPPG.Dim.y; }
                    pRect = GetBB(newPos.x, newPos.y, newDim.x, newDim.y );
                    if ( pRect.Max.x > ADIO->Dim.x ) { pRect.Max.x = ADIO->Dim.x; }
                    if ( pRect.Min.y < 0 ) { pRect.Min.y = 0; }
                }
                else if( ap.second == "ResizeBR" ) {
                    vec2 newPos = inPPG.Pos;
                    vec2 newDim = inPPG.Dim + mDelta;
                    if      ( mDelta.x > 0 && mPos.x < inPPG.Pos.x + inPPG.Dim.x  ) { newDim.x = inPPG.Dim.x; }
                    else if ( mDelta.x < 0 && mPos.x > inPPG.Pos.x + inPPG.Dim.x  ) { newDim.x = inPPG.Dim.x; }
                    else if ( mPos.x < inPPG.Pos.x + inPPG.MinDim.x               ) { newDim.x = inPPG.Dim.x; }
                    if      ( mDelta.y > 0 && mPos.y < inPPG.Pos.y + inPPG.Dim.y  ) { newDim.y = inPPG.Dim.y; }
                    else if ( mDelta.y < 0 && mPos.y > inPPG.Pos.y + inPPG.Dim.y  ) { newDim.y = inPPG.Dim.y; }
                    else if ( mPos.y < inPPG.Pos.y + inPPG.MinDim.y               ) { newDim.y = inPPG.Dim.y; }
                    pRect = GetBB(newPos.x, newPos.y, newDim.x, newDim.y );
                    if ( pRect.Max.x > ADIO->Dim.x ) { pRect.Max.x = ADIO->Dim.x; }
                    if ( pRect.Max.y > ADIO->Dim.y ) { pRect.Max.y = ADIO->Dim.y; }
                }
                else if( ap.second == "ResizeBL" ) {
                    vec2 newPos = inPPG.Pos + vec2( mDelta.x, 0 );
                    vec2 newDim = inPPG.Dim + vec2( -mDelta.x, mDelta.y );
                    if      ( mDelta.x < 0 && mPos.x > inPPG.Pos.x                ) { newPos.x = inPPG.Pos.x; newDim.x = inPPG.Dim.x; }
                    else if ( mDelta.x > 0 && mPos.x < inPPG.Pos.x                ) { newPos.x = inPPG.Pos.x; newDim.x = inPPG.Dim.x; }
                    else if ( mPos.x > inPPG.Pos.x + inPPG.Dim.x - inPPG.MinDim.x ) { newPos.x = inPPG.Pos.x; newDim.x = inPPG.Dim.x; }
                    if      ( mDelta.y > 0 && mPos.y < inPPG.Pos.y + inPPG.Dim.y  ) {                         newDim.y = inPPG.Dim.y; }
                    else if ( mDelta.y < 0 && mPos.y > inPPG.Pos.y + inPPG.Dim.y  ) {                         newDim.y = inPPG.Dim.y; }
                    else if ( mPos.y < inPPG.Pos.y + inPPG.MinDim.y               ) {                         newDim.y = inPPG.Dim.y; }
                    pRect = GetBB(newPos.x, newPos.y, newDim.x, newDim.y );
                    if ( pRect.Min.x < 0 ) { pRect.Min.x = 0; }
                    if ( pRect.Max.y > ADIO->Dim.y ) { pRect.Max.y = ADIO->Dim.y; }
                }
                else if( ap.second == "ResizeL" ) {
                    vec2 newPos = inPPG.Pos + vec2( mDelta.x, 0 );
                    vec2 newDim = inPPG.Dim + vec2( -mDelta.x, 0 );
                    if      ( mDelta.x < 0 && mPos.x > inPPG.Pos.x                ) { newPos.x = inPPG.Pos.x; newDim.x = inPPG.Dim.x; }
                    else if ( mDelta.x > 0 && mPos.x < inPPG.Pos.x                ) { newPos.x = inPPG.Pos.x; newDim.x = inPPG.Dim.x; }
                    else if ( mPos.x > inPPG.Pos.x + inPPG.Dim.x - inPPG.MinDim.x ) { newPos.x = inPPG.Pos.x; newDim.x = inPPG.Dim.x; }                    
                    pRect = GetBB(newPos.x, newPos.y, newDim.x, newDim.y );
                    if ( pRect.Min.x < 0 ) { pRect.Min.x = 0; }
                }
                else if( ap.second == "ResizeR" ) {
                    vec2 newPos = inPPG.Pos;
                    vec2 newDim = inPPG.Dim + vec2( mDelta.x, 0 );
                    if      ( mDelta.x > 0 && mPos.x < inPPG.Pos.x + inPPG.Dim.x  ) { newDim.x = inPPG.Dim.x; }
                    else if ( mDelta.x < 0 && mPos.x > inPPG.Pos.x + inPPG.Dim.x  ) { newDim.x = inPPG.Dim.x; }
                    else if ( mPos.x < inPPG.Pos.x + inPPG.MinDim.x               ) { newDim.x = inPPG.Dim.x; }
                    pRect = GetBB(newPos.x, newPos.y, newDim.x, newDim.y );
                    if ( pRect.Max.x > ADIO->Dim.x ) { pRect.Max.x = ADIO->Dim.x; }
                }
                else if( ap.second == "ResizeT" ) {
                    vec2 newPos = inPPG.Pos + vec2( 0, mDelta.y );
                    vec2 newDim = inPPG.Dim + vec2( 0, -mDelta.y );
                    if      ( mDelta.y < 0 && mPos.y > inPPG.Pos.y                ) { newPos.y = inPPG.Pos.y; newDim.y = inPPG.Dim.y; }
                    else if ( mDelta.y > 0 && mPos.y < inPPG.Pos.y                ) { newPos.y = inPPG.Pos.y; newDim.y = inPPG.Dim.y; }
                    else if ( mPos.y > inPPG.Pos.y + inPPG.Dim.y - inPPG.MinDim.y ) { newPos.y = inPPG.Pos.y; newDim.y = inPPG.Dim.y; }
                    pRect = GetBB(newPos.x, newPos.y, newDim.x, newDim.y );
                    if ( pRect.Min.y < 0 ) { pRect.Min.y = 0; }
                }
                else if( ap.second == "ResizeB" ) {
                    vec2 newPos = inPPG.Pos;
                    vec2 newDim = inPPG.Dim + vec2( 0, mDelta.y );
                    if      ( mDelta.y > 0 && mPos.y < inPPG.Pos.y + inPPG.Dim.y  ) { newDim.y = inPPG.Dim.y; }
                    else if ( mDelta.y < 0 && mPos.y > inPPG.Pos.y + inPPG.Dim.y  ) { newDim.y = inPPG.Dim.y; }
                    else if ( mPos.y < inPPG.Pos.y + inPPG.MinDim.y               ) { newDim.y = inPPG.Dim.y; }                    
                    pRect = GetBB(newPos.x, newPos.y, newDim.x, newDim.y );
                    if ( pRect.Max.y > ADIO->Dim.y ) { pRect.Max.y = ADIO->Dim.y; }
                }
                else if ( ap.second == "Drag" ) {
                    vec2 PpgDragOffset = ADIO->MM.GetActivePPGOffset();
                    vec2 winMin = vec2( 0, 15 );
                    vec2 winMax = ADIO->Dim - vec2( 0, 20 );
                    vec2 newPos = { mPos.x - PpgDragOffset.x, mPos.y - PpgDragOffset.y };
                    vec2 mPosRel = glm::clamp(mPos, winMin, winMax) - inPPG.Pos;
                    newPos = glm::clamp( newPos, winMin - mPosRel, winMax - mPosRel );
                    pRect = GetBB(newPos.x, newPos.y, inPPG.Dim.x, inPPG.Dim.y);
                }

                inPPG.Pos = { pRect.Min.x, pRect.Min.y };
                inPPG.Dim = { pRect.GetWidth(), pRect.GetHeight() };
                inPPG.UpdateBaseDim();
            }
        }

        // Mute the PPG unless it is the mouse hit PPG. If clicked, the PPG is moved to the top.
        inPPG.Mute = !ADIO->FM.PPGClickOK( inPPG.Index );
        if( DragStart() && ADIO->FM.PPGClickOK( inPPG.Index ) ) {
            ADIO->MM.MovePPGToTop( inPPG.Index );
            inPPG.Mute = false;
        }

        // Top bar BB
        ImRect UIBarBB;
        if ( inPPG.ShowActionBar ) {
            UIBarBB = GetBB(pRect.Min.x,pRect.Min.y,inPPG.Dim.x, 20);
        }

        // Draw the PPG drop shadow
        if ( inPPG.Dim[1] > 25 ) {
            float dropAlpha = .18f;
            if ( inPPG.Collapsed ) {
                if ( inPPG.ShowActionBar ) {
                    if ( ShowDropShadow ) { UIDropShadow(UIBarBB, 7, dropAlpha, 15, 15, 0x222222, 2, 2); }
                    UIDrawBB(UIBarBB, 0xFFFFFF);
                }
            } else {
                if ( ShowDropShadow ) { UIDropShadow(pRect, 13, dropAlpha, 15, 15, 0x222222, 1, 1); }
                UIDrawBB(pRect, 0xFFFFFF);
            }
        }

        // Draw the top bar
        if ( inPPG.ShowActionBar ) {

            UIDrawBB(UIBarBB, 0xEBEEEF);
            float xShift = 7;
            
            // Backlink Button
            if ( inPPG.BackLink != "" ) {

                ImRect blSprite = GetSprite(58,0, 36, 34);
                ImRect bluvRect = GetUV(blSprite, ADIO->ButtonBarDim);
                ImRect blRect = GetBB(pRect.Min.x, pRect.Min.y, 17, 19);
                UIImage( blRect, ADIO->ButtonBarIndex, bluvRect );

                xShift += 15;

                float shadeFactor = .1f;
                if( VecInBB(mPos, blRect)){
                    UIDrawAlphaBB(blRect, 0x000000, shadeFactor);
                    if( DragStart() && !inPPG.Mute ){       // backlink button closes the current PPG and opens the backlink PPG
                        inPPG.UpdateFunction( inPPG, "BackLink", { inPPG.BackLink } );
                        return;
                    }
                }
            }
            
            // Draw the PPG Name
            if(inDrawName){
                int fIndex = 0; //GetGeneralFont("default");
                UIAddTextWithFont( pRect.Min.x + xShift, pRect.Min.y, inPPG.Dim.x - 20, 20, inPPG.Name+inPPG.Name2, 0x000000, fIndex, vec2(0, 0.5));
            }

            // X icon to close
            ImRect fSprite = GetSprite(52,40, 38, 34);
            ImRect uvRect = GetUV(fSprite, ADIO->ButtonBarDim);
            ImRect imgRect1 = GetBB(pRect.Min.x+inPPG.Dim.x-19, pRect.Min.y, 17, 19);
            UIImage( imgRect1, ADIO->ButtonBarIndex, uvRect );

            float shadeFactor = .1f;
            if( VecInBB(mPos, imgRect1)){
                UIDrawAlphaBB(imgRect1, 0x000000, shadeFactor);
                if( DragStart() && !inPPG.Mute ){     // hit X button to close PPG
                    ADIO->MM.RemovePPG( inI );      // the ppg ExitFunction is called by PPGManager when the PPG is deleted
                    inPPG.UpdateFunction( inPPG, "Close", { "DrawBasePPG"} );
                    return;
                }
            }

            // Pinned / UnPinned Icon
            float fsY = 0;
            if(inPPG.Pinned){fsY += 40;}
            ImRect pSprite = GetSprite(2,fsY, 54, 35);
            ImRect uvRect2 = GetUV(pSprite, ADIO->ButtonBarDim);
            ImRect imgRect2 = GetBB(pRect.Min.x+inPPG.Dim.x-47, pRect.Min.y+1, 27, 18);
            UIImage( imgRect2, ADIO->ButtonBarIndex, uvRect2 );
            if( VecInBB(mPos, imgRect2)){
                UIDrawAlphaBB(imgRect2, 0x000000, shadeFactor);
                if( DragStart() && !inPPG.Mute ){     // hit button to switch Pinned
                    if(inPPG.Pinned){inPPG.Pinned=false;}
                    else{inPPG.Pinned=true;}
                }
            }
            
        }
        
        // PPG Drag Operations
        if ( ADIO->MM.MouseHit( inPPG.Index ) && ADIO->FM.PPGClickOK( inPPG.Index, fwt_init_dragging ) ) {
            
            float pcIn = 0.2f;
            {
                bool inArea = false;
                
                // TL Corner Resize
                ImRect rsRect = GetBB(inPPG.Pos.x-rsSize, inPPG.Pos.y-rsSize, rsSize*(1+pcIn), rsSize*(1+pcIn) );
                // UIDrawAlphaBB( rsRect, 0xFF0000, 0.75f );
                if( CanResizeTL && VecInBB(mPos, rsRect) ) {
                    inArea = true;
                    if(DragStart() ) {
                        ADIO->FM.SetFocus( ft_drag_resize_tlbr, "PPG" );
                        ADIO->MM.SetActivePPG( inI, "ResizeTL" );
                        inPPG.Pinned = true;
                    }
                    else {
                        ADIO->FM.SetMFocus( ft_cursor_resize_tlbr, "PPG" );
                    }
                }
            
                // BR Corner Resize
                rsRect = GetBB(inPPG.Pos.x+inPPG.Dim.x-rsSize*pcIn, inPPG.Pos.y+inPPG.Dim.y-rsSize*pcIn, rsSize*(1+pcIn), rsSize*(1+pcIn) );
                // UIDrawAlphaBB( rsRect, 0xFF0000, 0.75f );
                if( CanResizeBR && VecInBB(mPos, rsRect) ) { 
                    inArea = true;
                    if(DragStart() ) {
                        ADIO->FM.SetFocus( ft_drag_resize_tlbr, "PPG" );
                        ADIO->MM.SetActivePPG( inI, "ResizeBR" );
                        inPPG.Pinned = true;
                    }
                    else {
                        ADIO->FM.SetMFocus( ft_cursor_resize_tlbr, "PPG" );
                    }
                }

                if (!inArea) { ADIO->FM.ClearMFocus( ft_cursor_resize_tlbr, "PPG" ); }
                
            }


            {   
                bool inArea = false;

                // TR Corner Resize
                ImRect rsRect = GetBB(inPPG.Pos.x+inPPG.Dim.x-rsSize*pcIn, inPPG.Pos.y-rsSize, rsSize*(1+pcIn), rsSize*(1+pcIn) );
                // UIDrawAlphaBB( rsRect, 0xFF0000, 0.75f );
                if( CanResizeTR && VecInBB(mPos, rsRect) ) { 
                    inArea = true;
                    if(DragStart() ) {
                        ADIO->FM.SetFocus( ft_drag_resize_trbl, "PPG" );
                        ADIO->MM.SetActivePPG( inI, "ResizeTR" );
                        inPPG.Pinned = true;
                    }
                    else {
                        ADIO->FM.SetMFocus( ft_cursor_resize_trbl, "PPG" );
                    }
                }            

                // BL Corner Resize
                rsRect = GetBB(inPPG.Pos.x-rsSize, inPPG.Pos.y+inPPG.Dim.y-rsSize*pcIn, rsSize*(1+pcIn), rsSize*(1+pcIn) );
                // UIDrawAlphaBB( rsRect, 0xFF0000, 0.75f );
                if( CanResizeBL && VecInBB(mPos, rsRect) ) { 
                    inArea = true;
                    if(DragStart() ) {
                        ADIO->FM.SetFocus( ft_drag_resize_trbl, "PPG" );
                        ADIO->MM.SetActivePPG( inI, "ResizeBL" );
                        inPPG.Pinned = true;
                    }
                    else {
                        ADIO->FM.SetMFocus( ft_cursor_resize_trbl, "PPG" );
                    }
                }

                if (!inArea) { ADIO->FM.ClearMFocus( ft_cursor_resize_trbl, "PPG" ); }

            }

            {   
                bool inArea = false;

                // L Resize
                ImRect rsRect = GetBB(inPPG.Pos.x-rsSize, inPPG.Pos.y+rsSize*pcIn, rsSize*(1+pcIn), inPPG.Dim.y-2*rsSize*pcIn );
                // UIDrawAlphaBB( rsRect, 0x00FF00, 0.75f );
                if( CanResizeL && VecInBB(mPos, rsRect) ) { 
                    inArea = true;
                    if(DragStart() ) {
                        ADIO->FM.SetFocus( ft_drag_resize_h, "PPG" );
                        ADIO->MM.SetActivePPG( inI, "ResizeL" );
                        inPPG.Pinned = true;
                    }
                    else {
                        ADIO->FM.SetMFocus( ft_cursor_resize_h, "PPG" );
                    }
                }
            
                // R Resize
                rsRect = GetBB(inPPG.Pos.x+inPPG.Dim.x-rsSize*pcIn, inPPG.Pos.y+rsSize*pcIn, rsSize*(1+pcIn), inPPG.Dim.y-2*rsSize*pcIn );
                // UIDrawAlphaBB( rsRect, 0x00FF00, 0.75f );
                if( CanResizeR && VecInBB(mPos, rsRect) ) { 
                    inArea = true;
                    if(DragStart() ) {
                        ADIO->FM.SetFocus( ft_drag_resize_h, "PPG" );
                        ADIO->MM.SetActivePPG( inI, "ResizeR" );
                        inPPG.Pinned = true;
                    }
                    else {
                        ADIO->FM.SetMFocus( ft_cursor_resize_h, "PPG" );
                    }
                }

                if (!inArea) { ADIO->FM.ClearMFocus( ft_cursor_resize_h, "PPG" ); }

            }

            {   
                bool inArea = false;

                // T Resize
                ImRect rsRect = GetBB(inPPG.Pos.x+rsSize*pcIn, inPPG.Pos.y-rsSize, inPPG.Dim.x-2*rsSize*pcIn, rsSize*(1+pcIn) );
                // UIDrawAlphaBB( rsRect, 0x00FFFF, 0.75f );
                if( CanResizeT && VecInBB(mPos, rsRect) ) { 
                    inArea = true;
                    if(DragStart() ) {
                        ADIO->FM.SetFocus( ft_drag_resize_v, "PPG" );
                        ADIO->MM.SetActivePPG( inI, "ResizeT" );
                        inPPG.Pinned = true;
                    }
                    else {
                        ADIO->FM.SetMFocus( ft_cursor_resize_v, "PPG" );
                    }
                }
            

                // B Resize
                rsRect = GetBB(inPPG.Pos.x+rsSize*pcIn, inPPG.Pos.y+inPPG.Dim.y-rsSize*pcIn, inPPG.Dim.x-2*rsSize*pcIn, rsSize*(1+pcIn) );
                // UIDrawAlphaBB( rsRect, 0x00FFFF, 0.75f );
                if( CanResizeB && VecInBB(mPos, rsRect) ) { 
                    inArea = true;
                    if(DragStart() ) {
                        ADIO->FM.SetFocus( ft_drag_resize_v, "PPG" );
                        ADIO->MM.SetActivePPG( inI, "ResizeB" );
                        inPPG.Pinned = true;
                    }
                    else {
                        ADIO->FM.SetMFocus( ft_cursor_resize_v, "PPG" );
                    }
                }

                if (!inArea) { ADIO->FM.ClearMFocus( ft_cursor_resize_v, "PPG" ); }

            }

            {   // Top bar drag rectangle
                ImRect dragRect = GetBB(pRect.Min.x+rsSize, pRect.Min.y, inPPG.Dim.x-47, 20);
                if( CanDrag && VecInBB(mPos, dragRect)){
                    if(DragStart() ) {
                        ADIO->FM.SetFocus( ft_drag_hand, "PPG" );
                        ADIO->MM.SetActivePPG( inI, "Drag" );
                        inPPG.Pinned = true;
                    }
                    else {
                        if ( UILibIS->DoubleClick ) {
                            inPPG.Pinned = true;
                            inPPG.Collapsed = !inPPG.Collapsed;
                            UILibIS->DoubleClick = false;
                        }
                        ADIO->FM.SetMFocus( ft_cursor_hand, "PPG" );
                    }
                }
                else { ADIO->FM.ClearMFocus( ft_cursor_hand, "PPG" ); }
            }

        }
        
        if ( inPPG.ShowActionBar ) {
            UIDrawBBOutline(UIBarBB, 0xDBDBDB, 1);
        }

        if ( !inPPG.Collapsed ) {
            UIDrawBBOutline(pRect, 0xCCCCCC, 1);
        }

        // std::cout << "Drawing Base PPG: " << inPPG.Index << ", mute: " << ((inPPG.Mute) ? "true" : "false") << std::endl;
        
    }
    
    
} // end namespace Blocks

#endif // BLOCK_PPG_CPP