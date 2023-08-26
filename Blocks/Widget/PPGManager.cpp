#ifndef PPGMANAGER_CPP
#define PPGMANAGER_CPP


#include "c_PipelineManager.h"
#include "PPGManager.h"

using namespace AppCore;

using namespace std;

namespace Blocks {

    UIForm & PropertyPage::MakeNewForm( int inDir ) {
        Form = new UIForm( Name, inDir, Index );
        return *Form;
    }

    void PropertyPage::DeletePPGForm() {
        assert( ( Form != nullptr) && "PropertyPage attempting to access a null Form." );
        delete Form; Form = nullptr;
    }

    bool PropertyPage::FormIsFocused() {
        if ( !Form ) { return false; }
        return Form->GetFocus();
    }

    bool PropertyPage::IsOverlayMode( int inMode ) {
        return ( inMode >= mode_top_auto_hide && inMode <= mode_left_auto_hide );
    }

    bool PropertyPage::IsUnderlayMode( int inMode ) {
        return ( inMode == mode_fill_pane || inMode >= mode_top_dock && inMode <= mode_left_dock );
    }

    void PropertyPage::SetMode( int inMode ) {
        if      ( !IsOverlayMode(Mode)  && IsOverlayMode(inMode)  ) { ADIO->MM.MovePPGToTop(Index);    } // Switch to Overlay mode
        else if ( !IsUnderlayMode(Mode) && IsUnderlayMode(inMode) ) { ADIO->MM.MovePPGToBottom(Index); } // Switch to Underlay mode
        Mode = inMode;
    }

    int PropertyPage::GetMode() { return Mode; }

    void PropertyPage::UpdateBaseDim() { BaseDim = Dim; }

    void PropertyPage::UpdateAutoHide() {
        if ( Mode < mode_top_auto_hide || Mode > mode_left_auto_hide ) { return; }
        // status: 0 = closed, 1 = wait opening, 2 = opening, 3 = fully open, 4 = wait closing, 5 = closing
        static double WaitTime = 0.5f;
        static double TargetTime = 0.2f;
        static float ExpSize = 20;
        ImRect targetBB = GetBB(Pos.x, Pos.y, Dim.x, Dim.y); targetBB.Expand(ExpSize);
        bool InTarget = VecInBB( GetGlobalMousePos(), targetBB ) && !MouseDown();
        bool SignalStopPoll = false;
        if      ( Status == status_closed ) {
            if ( InTarget ) { Status = status_wait_opening; Timer.reset(); PMPTR->SetPoll( "PPGAutoHide_"+Name, true); }
        }
        else if ( Status == status_wait_opening ) {
            if ( Timer.elapseNR() > 0 ) { Status = status_opening; TimerOffset = 0; Timer.reset(); }
            else if ( !InTarget ) { Status = status_closed; SignalStopPoll = true; }
        }
        else if ( Status == status_opening ) {
            double pctOpen = ( Timer.elapseNR() + TimerOffset ) / TargetTime;
            Dim = BaseDim * (float) pctOpen;
            if ( Timer.elapseNR() > TargetTime ) { Status = status_opened; Dim = BaseDim; SignalStopPoll = true; }
            else if ( !InTarget ) { Status = status_closing; TimerOffset = TargetTime * (1-pctOpen); }
        }
        else if ( Status == status_opened ) {
            if ( FormIsFocused() ) {} // keep open when form is focused
            else if ( Index == ADIO->MM.GetActivePPG().first ) {} // do nothing if current ppg is being manipulated (dragged or resizing)
            else if ( ADIO->FM.HasCursorFocus("PPG") && ADIO->MM.MouseHit(Index) ) {}
            else if ( !InTarget ) { Status = status_wait_closing; Timer.reset(); PMPTR->SetPoll( "PPGAutoHide_"+Name, true); }
            BaseDim = Dim;
        }
        else if ( Status == status_wait_closing ) {
            if ( Timer.elapseNR() > WaitTime ) { Status = status_closing; TimerOffset = 0; Timer.reset(); }
            else if ( InTarget ) { Status = status_opened; SignalStopPoll = true; } 
        }
        else if ( Status == status_closing ) {
            double pctClose = ( Timer.elapseNR() + TimerOffset ) / TargetTime;
            Dim = BaseDim * (float)(1-pctClose);
            if ( Timer.elapseNR() > TargetTime ) { Status = status_closed; Dim = {0,0}; SignalStopPoll = true; }
            else if ( InTarget ) { Status = status_opening; TimerOffset = TargetTime * (1-pctClose); }
        }

        if ( SignalStopPoll ) { // Signal next frame to stop polling
            TPOOL->AddMsg( { "PM", "PPGAutoHide_"+Name, { "PPGAutoHide_"+Name, "false" } } );
        }

    }

    int GetPpgIndex(string inStr){
        for(int i=0;i<ADIO->OpenPPGs.size();i++){if(ADIO->OpenPPGs[i].Name == inStr){return i;}}
        return -1;
	}

    vector<int> GetPpgIndexes(string inStr) {
        vector<int> ppgIds = {};
        for(int i=0;i<ADIO->OpenPPGs.size();i++){if(ADIO->OpenPPGs[i].Name == inStr){ppgIds.push_back(i);}}
        return ppgIds;
    }
    
    PPGManager::PPGManager() {}
    PPGManager::~PPGManager() {}

    void PPGManager::PreRenderSetup(){
        
        // Reset at the beginning of each frame
        int lastPPGHit = PPGHit;
        PPGHit = -1;
        PPGTop.clear();
        PPGBot.clear();
        DeleteList = NextDeleteList;
        NextDeleteList.clear();
        DeleteAllPPGs = false;
        CloseUnpinned = false;
        OpenList.clear();
        
        // The ButtonBar texture was loaded and ADIO->ButtonBarIndex set before now during TUI initialization.
        // This texture is a resident texture that remains loaded for the application lifetime instead of being placed on the
        // dynamic texture list (IO.ImagePaths) where it gets reloaded for every frame.
        
        BuildHitRects();

        // if a drag begins from the PPGs we need to record this so it doesn't affect other UI behavior (like camera orbit)
        if(MouseRelease()){
            DragFromPPGId = -1;
            SetActivePPG();
            if ( ADIO->FM.HasDragFocus("PPG") ) { ADIO->FM.RevertToPane(); /*ADIO->FM.ClearFocus();*/ }
            else { ADIO->FM.RevertToPane( ft_ppg ); /*ADIO->FM.ClearFocus( ft_ppg );*/ }
        }
        if( DragStart() ){
            if(!AvoidPPG()){
                DragFromPPGId = PPGHit;
                ADIO->FM.SetFocus( ft_ppg, "PPG"+to_string(PPGHit));
                // if ( !ADIO->FM.HasFocus( ft_ppg_input ) ) { ADIO->FM.SetFocus( ft_ppg, "PPG"+to_string(PPGHit)); }
            }
        }
        if ( lastPPGHit != PPGHit ) {
            if ( ADIO->FM.HasCursorFocus("PPG") ) { ADIO->FM.ClearMFocus(); }
        }
        if ( PPGHit != -1 ) {
            ADIO->FM.SetMFocus( ft_ppg, "PPG"+to_string(PPGHit));
        }

        // Process Hotkeys
        if ( !ADIO->OpenPPGs.empty() ) {
            if ( PMPTR->GetInputState().Key.Change == 1 ) {
                int curr_key = PMPTR->GetInputState().Key.Key;
                if ( UILibIS->Alt ) {
                    if      ( curr_key == '-' ) { ADIO->OpenPPGs.back().Form->CollapseAll( true ); }    // Alt - to collapse all sections in top-most PPG
                    else if ( curr_key == '=' ) { ADIO->OpenPPGs.back().Form->CollapseAll( false ); }   // Alt = or Alt + to open all sections top-most PPG
                    else if ( curr_key == 'X' ) { RemoveAllPPGs(); }
                }
            }
        }

        Render = true;
    }

    void PPGManager::PostRenderSetup(){
        if(ADIO->OpenPPGs.size() == 0){return;}
        Render = false;
        int count = 0;
        if ( DeleteAllPPGs ) {
            CloseAllPPGs();
        }
        else {
            if ( DeleteAllFloatingPPGs ) {
                for ( int i = 0; i < ADIO->OpenPPGs.size(); i++ ) {
                    DeleteList.push_back( i );     
                    ADIO->OpenPPGs[i].ExitFlag = true;
                    if ( i == ActivePPG ) { SetActivePPG(); }
                }
            }
            ReorderPPGs();                      // reorder so queued PPGs move on top
            CloseDeleteListPPGs();              // get rid of PPGs marked to delete
            CloseUnPinnedPPGs();                // get rid of PPGs that aren't pinned
        }
    }

    bool PPGManager::MouseHit(int inPPGIndex){
        return ( inPPGIndex == PPGHit );
    }

    bool PPGManager::AvoidPPG(){
        return ( PPGHit == -1 );
    }

    bool PPGManager::IsTop( int inPPGIndex ) {
        return ( inPPGIndex == ADIO->OpenPPGs.size()-1 );
    }

    bool PPGManager::IsTopNextFrame( int inPPGIndex ) {
        if ( IsTop( inPPGIndex ) && PPGTop.size() == 0  ) return true;
        bool inTopList = false;
        for ( int pi : PPGTop ) {
            if ( inPPGIndex == pi ) { inTopList = true; }
        }
        return inTopList;
    }    

    PropertyPage & PPGManager::AddPPG( PropertyPage & inPPG ) {
        ADIO->OpenPPGs.push_back( inPPG );
        CloseUnpinned = true;
        OpenList.push_back(inPPG.Index);
        return ADIO->OpenPPGs.back();
    }

    PropertyPage & PPGManager::AddPPG( string inName, bool inPinned, vec2 inPos, vec2 inDim ) {
        // There are 4 kinds of PPGs that you might create in the app.
        // (1) Wired PPGs - These type of PPGs bound to a specific Sheet in the application.
        //       Even when the Sheet is not visible, not selected, or not loaded, this PPG
        //       will persist and still have the ability to update the Sheet it is bound to.
        //       Typically, this PPG holds the SheetName so it can retrieve the relevant Config
        //       or write to the appropriate disk location when needed.
        //       Examples: BarChartPPG or PieChartPPG
        // (2) Hard Bind PPGs - This type of PPG is bound to a specific Pane. It expires and 
        //       exits when the Pane changes Config or the item it is bound to goes out of scope.
        //       Typically, this PPG holds a pointer to the object it is bound to.
        //       Examples: MaterialsPPG or TransformsPPG
        // (3) Loose Bind PPGs - This type of PPG is not bound anything and simply relies on the
        //       SendAppMessage to propogate changes in the app, typically on the current active 
        //       Sheet. This PPG persists when you switch Panes, Tabs, etc.
        //       Example: ColorThemePPG
        // (4) Do Nothing PPGs - This type of PPG is purely a display. It does not proprogate changes
        //       to other parts of the app.
        //       Example: Image PPGs (i.e., Shoe Demo)
        PropertyPage newPPG = {inName, inPinned, inPos, inDim};
        newPPG.Index = (int)ADIO->OpenPPGs.size();
        newPPG.MinDim = { 50, 25 };
        return AddPPG( newPPG );
    }

    void PPGManager::RemovePPG( int inPPGIndex ) {
        if ( Render ) {
            DeleteList.push_back( inPPGIndex );
        } else {
            NextDeleteList.push_back( inPPGIndex );   
        }        
        ADIO->OpenPPGs[inPPGIndex].ExitFlag = true;
        if ( inPPGIndex == ActivePPG ) { SetActivePPG(); }
    }

    void PPGManager::RemoveAllPPGs() {
        DeleteAllPPGs = true;
    }

    void PPGManager::RemoveFloatingPPGs() {
        DeleteAllFloatingPPGs = true;
    }

    void PPGManager::RemoveExistingPPGs() {
        for ( size_t i = 0; i < ADIO->OpenPPGs.size(); i++ ) {
            RemovePPG((int)i);
        }
    }

    void PPGManager::MovePPGToTop( int inPPGIndex ) {
        // Remove the PPG from the existing PPGTop & PPGBot lists, then add to PPGTop list.
        for ( int i = 0; i < PPGTop.size(); i++ ) {
            if ( PPGTop[i] == inPPGIndex ) {
                PPGTop.erase( PPGTop.begin() + i ); break;
            }
        }
        for ( int i = 0; i < PPGBot.size(); i++ ) {
            if ( PPGBot[i] == inPPGIndex ) {
                PPGBot.erase( PPGBot.begin() + i ); break;
            }
        }
        PPGTop.push_back(inPPGIndex);
    }

    void PPGManager::MovePPGToBottom( int inPPGIndex ) {
        // Remove the PPG from the existing PPGTop & PPGBot lists, then add to PPGBot list.
        if ( inPPGIndex < 0 || inPPGIndex >= ADIO->OpenPPGs.size() ) { return; }
        if ( PropertyPage::IsOverlayMode( ADIO->OpenPPGs[inPPGIndex].Mode ) ) { return; }
        for ( int i = 0; i < PPGTop.size(); i++ ) {
            if ( PPGTop[i] == inPPGIndex ) {
                PPGTop.erase( PPGTop.begin() + i ); break;
            }
        }
        for ( int i = 0; i < PPGBot.size(); i++ ) {
            if ( PPGBot[i] == inPPGIndex ) {
                PPGBot.erase( PPGBot.begin() + i ); break;
            }
        }
        PPGBot.push_back(inPPGIndex);
    }

    bool PPGManager::DragFromPPG() {
        return ( DragFromPPGId >= 0 );
    }

    int PPGManager::DragFromPPGIndex() {
        return DragFromPPGId;
    }

    void PPGManager::SetActivePPG( int inPPGIndex, string inAction ) {
        ActivePPG = inPPGIndex;
        ActivePPGAction = inAction;
        if ( inPPGIndex >= 0 ) {
            vec2 mPos = GetGlobalMousePos();
            ActivePPGOffset = {mPos.x - ADIO->OpenPPGs[inPPGIndex].Pos.x, mPos.y - ADIO->OpenPPGs[inPPGIndex].Pos.y};
        }
        else {
            ActivePPGOffset = {};
        }
        
    }

    pair<int, string> PPGManager::GetActivePPG() {
        return pair<int,string>( ActivePPG, ActivePPGAction );
    }

    vec2 PPGManager::GetActivePPGOffset() {
        return ActivePPGOffset;
    }

    void PPGManager::CloseUnPinnedPPGs(){
        // CloseUnPinnedPPGs() is called in the post-frame cleanup is CloseUnpinned == true. This removes unpinned
        // PPGs so that they are not drawn in the next frame. In some cases, a UI element may want to remove unpinned 
        // PPGS before the current frame draw is submitted. For example, when clicking on a grid of images, when you 
        // click on one, you want it to dispose of the last one you were drilling in on. However, if you don't dispose
        // of the old PPG in the current frame, you end up having to reload image texutres twice, which causes a UI lag.
        if ( !CloseUnpinned ) { return; }
        int count = 0;
        int numPPGs = (int) ADIO->OpenPPGs.size();
        vector<int> nIndexList( numPPGs, 0 );               // Mapping from old index to new index
        for ( int i = 0; i < numPPGs; i++ ) {
            int ci = i - count;                              // need to offset i by the count, since we are deleting items in ADIO->OpenPPGs as we cycle through
            if( ADIO->OpenPPGs[ci].Pinned ){                 // Keeps PPG open (PPG is pinned)
                ADIO->OpenPPGs[ci].Index -= count;
                nIndexList[i] = ADIO->OpenPPGs[ci].Index;
            }
            else if ( InOpenListList(i) ) {                 // Keeps PPG open (PPG is in OpenList list)
                ADIO->OpenPPGs[ci].Index -= count;
                nIndexList[i] = ADIO->OpenPPGs[ci].Index;
            }
            else {                                          // Deletes PPG
                UIRemovePPGComboBoxes( i );
                ADIO->OpenPPGs[ci].ExitFunction( ADIO->OpenPPGs[ci] );   // call the PPG ExitFunction
                ADIO->OpenPPGs.erase(ADIO->OpenPPGs.begin()+ci);
                count++;
            }
        }
        RemapLists( nIndexList );
        RemapChildren( nIndexList );
        CloseUnpinned = false;
    }

    void PPGManager::SendPPGMessage( string inType, vector<string> inMessage ) {
        // Sends message to all PPGs. There is no need to call this directly, since it is called by PipelineManager. 
        // Use PMPTR->SendAppMessage( "PPG", ... ), to send a PPG message.
        // For consistency, the first element of inMessage should always name of the source object.
        // 0 = Ignored, 1 = Received, 2 = Halt Further Messages
        for ( auto & ppg : ADIO->OpenPPGs ) { if ( ppg.UpdateFunction( ppg, inType, inMessage ) == 2 ) return; };
    }

    void PPGManager::QueuePPGMessage( string inType, vector<string> inMessage ) {
        MsgQueue.push( make_pair( inType, inMessage ) );
    }

    void PPGManager::ProcessQueuedMessages() {
        while ( MsgQueue.size() > 0 ) {
            auto msg = MsgQueue.front();
            MsgQueue.pop();
            SendPPGMessage( msg.first, msg.second );
        }
    }

    // Private Methods

    void PPGManager::BuildHitRects(){
        vec2 mPos = GetGlobalMousePos();
        PPGRects.clear();
        for(PropertyPage &pp: ADIO->OpenPPGs){
            if ( !pp.Collapsed ) {
                PPGRects.push_back( {pp.Pos.x-DragArea, pp.Pos.y-DragArea, pp.Pos.x+pp.Dim.x+DragArea, pp.Pos.y+pp.Dim.y+DragArea} );
            }
            else {
                PPGRects.push_back( {pp.Pos.x-DragArea, pp.Pos.y-DragArea, pp.Pos.x+pp.Dim.x+DragArea, pp.Pos.y+20+DragArea} );
            }
            if ( PPGRects.back().Contains( {mPos.x, mPos.y} ) ) { PPGHit = (int) PPGRects.size() - 1; }
        }
        // std::cout << "PPG Hit: " << PPGHit << std::endl;        
    }

    void PPGManager::CloseDeleteListPPGs() {
        if( DeleteList.size() == 0 ) { return; }
        int count = 0;
        int numPPGs = (int) ADIO->OpenPPGs.size();
        vector<int> nIndexList( ADIO->OpenPPGs.size() );            // Mapping from old index to new index
        for ( int i = 0; i < numPPGs; i++ ) {
            int ci = i - count;
            if ( InDeleteList(i) ) {                 // Deletes PPG
                UIRemovePPGComboBoxes( i );
                ADIO->OpenPPGs[ci].ExitFunction( ADIO->OpenPPGs[ci] );   // call the PPG ExitFunction
                ADIO->OpenPPGs.erase(ADIO->OpenPPGs.begin()+ci);
                count++;
                nIndexList[i] = -1;
            }
            else {                                   // Keeps PPG open
                ADIO->OpenPPGs[ci].Index -= count;
                nIndexList[i] = ADIO->OpenPPGs[ci].Index;
            }
        }
        RemapLists( nIndexList );
        RemapChildren( nIndexList );
        DeleteList.clear();
    }

    void PPGManager::CloseAllPPGs() {
        int numPPGs = (int) ADIO->OpenPPGs.size();
        for ( int i = 0; i < numPPGs; i++ ) {
            UIRemovePPGComboBoxes( i );
            ADIO->OpenPPGs[i].ExitFunction( ADIO->OpenPPGs[i] );   // call the PPG ExitFunction
        }
        ADIO->OpenPPGs = {};
        DeleteAllPPGs = false;
        DeleteAllFloatingPPGs = false;
    }

    void PPGManager::SortTopAndBottom() {
        // Sort the PPGTop and PPGBot vectors. This adjust the vectors to push PPGs in 
        // an Overlay mode to the top, while PPGS with Underlay mode to the bottom.

        using PP = PropertyPage;

        // Remove any Underlays from PPGTop to PPGBot
        for ( int i = (int) PPGTop.size()-1; i >= 0; i-- ) {
            if ( PP::IsUnderlayMode( ADIO->OpenPPGs[PPGTop[i]].Mode ) ) {
                PPGTop.erase( PPGTop.begin() + i );
            }
        }
        
        // Remove any Overlays from PPGBot to PPGTop
        for ( int i = (int) PPGBot.size()-1; i >= 0; i-- ) {
            if ( PP::IsOverlayMode( ADIO->OpenPPGs[PPGBot[i]].Mode ) ) {
                PPGBot.erase( PPGBot.begin() + i );
            }
        }

        // Shift Overlays to the back
        // PPGTop order: bottom --> top
        vector<int> overlays = {};
        for ( int i = (int) PPGTop.size()-1; i >= 0; i-- ) {
            if ( PP::IsOverlayMode( ADIO->OpenPPGs[PPGTop[i]].Mode ) ) {
                overlays.push_back( PPGTop[i] );
                PPGTop.erase( PPGTop.begin() + i );
            }
        }
        // If PPGTop has floating PPGs, so we need to insert overlays not currently in PPGTop 
        // above the floating PPGS but below the moved to top overlays.
        if ( !PPGTop.empty() ) {
            for ( int i = (int) ADIO->OpenPPGs.size()-1; i >= 0; i-- ) {
                if ( PP::IsOverlayMode( ADIO->OpenPPGs[i].Mode ) ) {
                    bool found = false;
                    for ( int ppgI : overlays ) {
                        if ( ppgI == i ) { found = true; break; }
                    }
                    if ( !found ) { overlays.insert( overlays.begin(), i ); }
                }
            }
        }
        if ( !overlays.empty() ) { PPGTop.insert( PPGTop.end(), overlays.begin(), overlays.end() ); }

        // Shift Underlays to the back
        // PPGBot order: top --> bottom
        vector<int> underlays = {};
        for ( int i = (int) PPGBot.size()-1; i >= 0; i-- ) {
            if ( PP::IsUnderlayMode( ADIO->OpenPPGs[PPGBot[i]].Mode ) ) {
                underlays.push_back( PPGBot[i] );
                PPGBot.erase( PPGBot.begin() + i );
            }
        }
        // If PPGBot has floating PPGs, so we need to insert underlays not currently in PPGBot below the floating PPGS
        if ( !PPGBot.empty() ) {
            for ( int i = 0; i < ADIO->OpenPPGs.size(); i++ ) {
                if ( PP::IsUnderlayMode( ADIO->OpenPPGs[i].Mode ) ) {
                    bool found = false;
                    for ( int ppgI : underlays ) {
                        if ( ppgI == i ) { found = true; break; }
                    }
                    if ( !found ) { underlays.insert( underlays.begin(), i ); }
                }
            }
        }
        if ( !underlays.empty() ) { PPGBot.insert( PPGBot.end(), underlays.begin(), underlays.end() ); }

    }

    void PPGManager::ReorderPPGs() {
        if( PPGTop.empty() && PPGBot.empty() ) { return; }  // No PPG move requested
        if( PPGTop.size() == 1 && PPGTop[0] == (int)ADIO->OpenPPGs.size() - 1                   // Requested top PPG is already on top
            && PPGBot.size() == 1 && PPGBot[0] == (int)ADIO->OpenPPGs.size() - 1 ) { return; }  // AND bottom PPG is already on bottom

        SortTopAndBottom();

        int ppgCount = (int) ADIO->OpenPPGs.size();
        vector<int> nIndexList( ppgCount );            // Mapping from old index to new index
        vector<bool> isTop( ppgCount, false );
        vector<bool> isBot( ppgCount, false );

        vector<PropertyPage> topPPGs = {};
        vector<PropertyPage> botPPGs = {};

        // Cache the top PPGs
        int newI = (int) ( ppgCount - PPGTop.size() );
        for ( int ppgI : PPGTop ) { 
            topPPGs.push_back( ADIO->OpenPPGs[ppgI] );
            nIndexList[ppgI] = topPPGs.back().Index = newI;
            isTop[ppgI] = true; 
            newI++;
        }

        // Cache the bottom PPGs
        // Note: Cycle through PPGBot backwards, because the bottom most PPG is last in the list.
        newI = (int) PPGBot.size()-1;
        for ( int i = (int)PPGBot.size()-1; i >= 0; i-- ) {
            const int ppgI = PPGBot[i];
            botPPGs.push_back( ADIO->OpenPPGs[ppgI] );
            nIndexList[ppgI] = botPPGs.back().Index = newI;
            isBot[ppgI] = true; 
            newI--;
        }

        newI = 0;
        int offsetI = (int) PPGBot.size();
        for ( int i = 0; i < ppgCount; i++ ) {
            if ( isTop[i] ) {
                ADIO->OpenPPGs.erase( ADIO->OpenPPGs.begin() + newI );  // erase the existing top PPGs
            }
            else if ( isBot[i] ) {
                ADIO->OpenPPGs.erase( ADIO->OpenPPGs.begin() + newI );  // erase the existing bottom PPGs
            }
            else {
                ADIO->OpenPPGs[newI].Index = newI + offsetI;            // otherwise adjust the PPG index
                nIndexList[i] = newI + offsetI;
                newI++;
            }
        }
        ADIO->OpenPPGs.insert( ADIO->OpenPPGs.end(), topPPGs.begin(), topPPGs.end() );  // append top PPGs to the end
        ADIO->OpenPPGs.insert( ADIO->OpenPPGs.begin(), botPPGs.begin(), botPPGs.end() );// insert bottom PPGs to the start

        if(ActivePPG != -1){
            ActivePPG = nIndexList[ActivePPG];                                          // update the index of active PPG
        }
        RemapLists( nIndexList );
        RemapChildren( nIndexList );
        UIUpdateComboBoxPPGIndex( nIndexList );
        PPGTop.clear();
        PPGBot.clear();

        // std::cout << "------------ PPG Reorder Complete ------------" << std::endl;
        // for ( int i = 0; i < ADIO->OpenPPGs.size(); i++ ) {
        //     std::cout << "[" << i << "]: " << ADIO->OpenPPGs[i].Name << ", Index: " << ADIO->OpenPPGs[i].Index << std::endl;
        // }
        // std::cout << "----------------------------------------------" << std::endl;
    
    }

    bool PPGManager::InOpenListList( int inPPGIndex ) {
        for ( int oi : OpenList ) {
            if ( inPPGIndex == oi ) { return true; }
        }
        return false;
    }

    bool PPGManager::InDeleteList( int inPPGIndex ) {
        for ( int i = 0; i < DeleteList.size(); i++ ) {
            if ( inPPGIndex == DeleteList[i] ) { DeleteList.erase( DeleteList.begin() + i ); return true; }
        }
        return false;
    }

    void PPGManager::RemapLists( vector<int> inMap ) {
        for ( int i = 0; i < OpenList.size(); i++ ) {
            OpenList[i] = inMap[OpenList[i]];
        }
        for ( int i = 0; i < DeleteList.size(); i++ ) {
            DeleteList[i] = inMap[DeleteList[i]];
        }
        for ( int i = 0; i < PPGTop.size(); i++ ) {
            PPGTop[i] = inMap[PPGTop[i]];
        }
        for ( int i = 0; i < PPGBot.size(); i++ ) {
            PPGBot[i] = inMap[PPGBot[i]];
        }
    }

    void PPGManager::RemapChildren( vector<int> inMap ) {
        for ( int i = 0; i < ADIO->OpenPPGs.size(); i++ ) {
            for ( int j = (int) ADIO->OpenPPGs[i].Children.size()-1; j >= 0; j-- ) { 
                int & index = ADIO->OpenPPGs[i].Children[j];
                int newIndex = inMap[index];
                if ( newIndex < 0 ) { 
                    ADIO->OpenPPGs[i].Children.erase( ADIO->OpenPPGs[i].Children.begin() + j) ; 
                } else {
                    index = newIndex;
                }
            }
        }
    }

    

}

#endif /* PPGMANAGER_CPP */
