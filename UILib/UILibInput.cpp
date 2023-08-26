#ifndef UILIBINPUT_CPP
#define UILIBINPUT_CPP

#include "UILib.h"

#pragma warning(disable:4996)  //  cross-platform so avoid these Microsoft warnings

// #define LOG_UITEXTMAP // uncomment for logging of UITextMap functions

#define GET_XPOS_INDEX {\
    if(inTM.YRange.size()==0){return;}\
    array<int,2> range = inTM.YRange[lineI];\
    int prior=0;int lineEnd = range[1];\
    int maxI = (int)inTM.CList.size()-1;\
    int ind1 = lineEnd;if(ind1<0){ind1=0;}if(ind1>maxI){ind1=maxI;}\
    int ind2 = lineEnd-1;if(ind2<0){ind2=0;}if(ind2>maxI){ind2=maxI;}\
    if(xPos < inTM.CList[range[0]].x){ xI = range[0];} \
    else if(xPos > inTM.CList[ ind1 ].x - inTM.CList[ ind2 ].w*.5f){ xI = lineEnd;} \
    else{for(int i=range[0];i<=ind1;i++){i==0?prior=0:prior=i-1;if( xPos < inTM.CList[i].x - inTM.CList[prior].w*.5f){xI = i - 1;break;}}} \
}

#define GET_CLICK_INDEX {\
    if(inTM.YRange.size()==0){return;}\
    if(inTM.CList.size()==0){return;}\
    array<int,2> range = inTM.YRange[mouseL];\
    int prior=0;int lineEnd = range[1];\
    int maxI = (int)inTM.CList.size()-1;\
    int ind1 = lineEnd;if(ind1<0){ind1=0;}if(ind1>maxI){ind1=maxI;}\
    int ind2 = lineEnd-1;if(ind2<0){ind2=0;}if(ind2>maxI){ind2=maxI;}\
    if(mp.x < inTM.CList[range[0]].x){ clickI = range[0];}\
    else if(lineEnd==0){if(mp.x > inTM.CList[ ind1 ].x - inTM.CharW*.5f){clickI = lineEnd;}}\
    else if(mp.x > inTM.CList[ ind1 ].x - inTM.CList[ ind2 ].w*.5f){clickI = lineEnd;}\
    else{for(int i=range[0];i<=ind1;i++){i==0?prior=0:prior=i-1;if( mp.x < inTM.CList[i].x - inTM.CList[prior].w*.5f){clickI = i - 1;break;}}}\
}

namespace UILib {

    int GetUndoInput(){
        int key = UILibIS->Key.Key;int rtnI = 0;
        if(key == 'Z' && UILibIS->Ctrl && UILibIS->Shift ){rtnI = 2;}
        else if(key == 'Z' && UILibIS->Ctrl){rtnI = 1;} 
        else if(key == 'Y' && UILibIS->Ctrl){rtnI = 2;}
        else{return 0;}
        int keyChange = UILibIS->Key.Change;
        if(keyChange == 1){UILibCTX->HoldDown = 0;}
        auto currentTime = std::chrono::high_resolution_clock::now();
        if(key == UILibCTX->LastKey && keyChange != 1){ // slow down typing to a comfortable rate - avoid repeating letters to quickly
            float timeSpan = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - UILibCTX->LastKeyTime).count();
            float timeDelay = 0.2f;
            if(UILibCTX->HoldDown == 1){timeDelay = 0.03f;}if(timeSpan < timeDelay){return 0;}else{UILibCTX->HoldDown = 1;}
        }
        UILibCTX->LastKey = key;UILibCTX->LastKeyTime = currentTime;
        return rtnI;
    }
    string UIGetWordAtPosition(TextMap &inTM, vec2 inGPos, int &outStart, int &outEnd, int&outYLine){
        // pos is expected to be global pos, it's mapped into text space here
        // first get if the mouse is over a character
        outStart=-1;outEnd=-1;
        if(inTM.CList.size()==0){return "";}
        float xp = inGPos.x-inTM.Rect.Min.x;
        float yp = inGPos.y-inTM.Rect.Min.y-inTM.YBorder;
        if(yp <0){return "";}
        int yLine = int(yp/inTM.LineH);
        if(yLine>inTM.NbLines-1){return "";}
        int xMin=inTM.YRange[yLine][0];int xMax=(int)inTM.CList.size()-1;
        if(yLine<inTM.NbLines-1){xMax= inTM.YRange[yLine+1][0]-1;}
        int clickI=0;int prior=0;
        if(xp<inTM.CList[xMin].x){return "";}
        if(xp>inTM.CList[xMax].x){return "";}
        outYLine = yLine;
        for(int i=xMin+1;i<=xMax;i++){i==0?prior=0:prior=i-1;if(xp<inTM.CList[i].x){clickI=i-1;break;}}
        int rIndex = clickI;int lIndex = clickI;int ci=0;
        vector<int> dList = {40,123,44,43,94,45,42,47,92,60,61,62,38,37,41,125,39,34,32,33,58,59,46,'\n','\r'};
        int cStart = 0;
        int ch = (int)inTM.CList[clickI].Char;
        if(ch==32||ch==9){ // crawl spaces and tabs
            for(int i=clickI;i<inTM.CList.size();i++){int ci = (int)inTM.CList[i].Char;if (ci!=32&&ci!=9){break;}rIndex = i+1;}
            for (int i=clickI; i-->0;){int ci = (int)inTM.CList[i].Char;if (ci!=32&&ci!=9){break;}lIndex = i;}
        }
        else{
            for(int i=clickI;i<inTM.CList.size();i++){if (count(dList.begin(), dList.end(), (int)inTM.CList[i].Char)){break;}rIndex = i+1;}
            for (int i=clickI;i-->0;){if (count(dList.begin(), dList.end(), (int)inTM.CList[i].Char)){break;}lIndex = i;}
        }
        
        string outString = "";
        for(int i=lIndex;i<rIndex;i++){
            if(isalpha(inTM.CList[i].Char)){
                outString+=tolower(inTM.CList[i].Char);
            }
        }
        outStart=lIndex;outEnd=rIndex;

        if(outEnd > (int)inTM.CList.size()-1){outEnd=(int)inTM.CList.size()-1;}
        return outString;
        
    }
    string UIGetWordAtCaret(TextMap &inTM, int &outStart, int &outEnd, const vector<int> &inDlist){
        
        if(inTM.CList.size()==0){outStart=-1;return "";}
        if(inTM.SList.size()==0){outStart=-1;return "";}
        
        if(inTM.SList[0].S>inTM.CList.size()-1){inTM.SList[0].S = (int)inTM.CList.size()-1;}
        if(inTM.SList[0].E>inTM.CList.size()-1){inTM.SList[0].E = (int)inTM.CList.size()-1;}

        int clickI = inTM.SList[0].S;
        int rIndex = clickI;int lIndex = clickI;int ci=0;

        if(inTM.SList[0].S!=inTM.SList[0].E){
            lIndex = inTM.SList[0].S;
            rIndex = inTM.SList[0].E;
        }
        else{
            bool dMask[256] = {0};
            for ( int d : inDlist ) { dMask[d] = true; }

            int cStart = 0;
            int ch = (int)inTM.CList[clickI].Char;
            if(ch==32||ch==9){ // crawl spaces and tabs
                for(int i=clickI;i<inTM.CList.size();i++){int ci = (int)inTM.CList[i].Char;if (ci!=32&&ci!=9){break;}rIndex = i+1;}
                for (int i=clickI; i-->0;){int ci = (int)inTM.CList[i].Char;if (ci!=32&&ci!=9){break;}lIndex = i;}
            }
            else{
                for(int i=clickI;i<inTM.CList.size();i++){
                    if ( dMask[ inTM.CList[i].Char ] ) { break; }
                    rIndex = i+1;
                }
                for (int i=clickI;i-->0;){
                    if ( dMask[inTM.CList[i].Char] ){break;}
                    lIndex = i;
                }
            }
        }

        string outString = "";
        for(int i=lIndex;i<rIndex;i++){
            // if(isalpha(inTM.CList[i].Char)){
            outString+=tolower(inTM.CList[i].Char);
            // }
        }
        outStart=lIndex;outEnd=rIndex;
        return outString;
    }
    void DoubleClickTM(TextMap &inTM){
        
        // reset selection and crawl forward and backward in either direction
        if(inTM.CList.size()==0){return;}

        vec2 gp = GetGlobalMousePos(); // global mouse pos
        if( !VecInBB( gp, inTM.Rect ) ) { return; }

         // select between delimitors
        //                   (   {  ,  +  ^  -  *  /  \  <  =  >  &  %  )  }  TAB ,  #  '  " spc !  :  ;  . LF CR EOL
        vector<int> dList = {40,123,44,43,94,45,42,47,92,60,61,62,38,37,41,125,9,44,35,39,34,32,33,58,59,46,10,13,0,'[',']'};
        int cStart = 0;
        bool inBlock = false;
        
        vec2 mp = {gp.x - inTM.Rect.Min.x, gp.y - inTM.Rect.Min.y };  // put mp mouse x in the space of text edit window
        float yStart = inTM.YBorder;
        if(inTM.CList.size()>0){yStart = inTM.CList[0].y;}
        float yDown = mp.y - yStart;
        int mouseL = int( yDown / inTM.LineH);
        if(mouseL > inTM.NbLines - 1){mouseL = inTM.NbLines -1;}if(mouseL<0){mouseL=0;}

        int clickI = 0; GET_CLICK_INDEX
        int rIndex = clickI;int lIndex = clickI;int ci=0;
        int ch = (int)inTM.CList[clickI].Char;

        if(ch==32||ch==9){ // crawl spaces and tabs
            for(int i=clickI;i<inTM.CList.size();i++){int ci = (int)inTM.CList[i].Char;if (ci!=32&&ci!=9){break;}rIndex = i+1;}
            for (int i=clickI; i-->0;){int ci = (int)inTM.CList[i].Char;if (ci!=32&&ci!=9){break;}lIndex = i;}
        }
        else{
            for(int i=clickI;i<inTM.CList.size();i++){if (count(dList.begin(), dList.end(), (int)inTM.CList[i].Char)){break;}rIndex = i+1;}
            for (int i=clickI;i-->0;){if (count(dList.begin(), dList.end(), (int)inTM.CList[i].Char)){break;}lIndex = i;}
        }
        
        inTM.SList = { { lIndex,rIndex,lIndex,rIndex }};
        UILibIS->DoubleClick = false;
        inTM.LDrag = 2; // drag caret in single mode
    }
    void UIMouseInputTM( TextMap &inTM){
        
        // ====================================== DOUBLE CLICK ==========================================
        if(UILibIS->DoubleClick){DoubleClickTM(inTM);return;}
        // ===================================== STOP DRAGGING ==========================================
        if( !AnyMouseDown() ){inTM.LDrag = 0;inTM.MDrag = 0;inTM.RightLimit=0;return;}
        // ===================================== GENERAL SETUP ==========================================

        float y0 = inTM.Rect.Min.y; // + inTM.YBorder;
        vec2 gp = GetGlobalMousePos(); // global mouse pos
        vec2 mp = {gp.x - inTM.Rect.Min.x, gp.y - inTM.MinY - inTM.Rect.Min.y};
        int mouseL = int( mp.y / inTM.LineH);
        if(mouseL > inTM.NbLines - 1){mouseL = inTM.NbLines -1;}if(mouseL<0){mouseL=0;}
        //cout << "    mouseL: " << mouseL << ", inTM.MDrag: " << inTM.MDrag << "\n";
        
        // ==================================== START MULTI DRAG ========================================
        if(VecInBB(gp,inTM.ClickRect) && MiddleDragStart()){

            if(inTM.CList.size()==0){return;}
            //cout << "START MULTI DRAG" << "\n";
            inTM.DragPoint = mp;inTM.MDrag = 2;
            if(mp.x < 0){mp.x = 0;}if(mp.y < y0){mp.y = y0;}
            int diStart = int(inTM.DragPoint.y  / inTM.LineH);
            if(diStart > inTM.NbLines - 1){diStart = inTM.NbLines -1;}
            if(diStart < 0){diStart = 0;}
            int diEnd = mouseL;
            int yiStart = diStart;int yiEnd = diEnd;  
            if(diEnd < diStart){yiStart = yiEnd; yiEnd=diStart;}
            array<int,2> range;int prior=0;
            float endX = mp.x;
            inTM.RightLimit = 0;
            bool onRight = true;int minI;
            for(int i=yiStart;i<=yiEnd;i++){
                range = inTM.YRange[i];
                minI=range[1]-1;if(minI<0){minI=0;}
                if(endX < inTM.CList[range[1]].x - inTM.CList[minI].w*.5f){onRight = false;}
            }
            if(onRight){inTM.RightLimit=1;}
            
        }
        // ================================== SINGLE CARET DRAGGING ====================================
        else if(inTM.LDrag == 2){
            //cout << "SINGLE CARET DRAGGING" << "\n";
            if(inTM.SList.size()==0){return;}
            if (inTM.Format.HAlign <= .2) { // Left Aligned
                if(mp.x < 0){mp.x = 0;}
                if(mp.y < y0){mp.y = y0;}
            }
            else {
                if(mp.x < inTM.MinX){mp.x = inTM.MinX;}
                if(mp.y < y0+inTM.MinY){mp.y = y0+inTM.MinY;}
            }
            int clickI = 0; GET_CLICK_INDEX
            UITextSelection * sl = &inTM.SList[(int)inTM.SList.size()-1];
            if(clickI<0){clickI=0;}
            sl->R = clickI;
            sl->UpdateSE();
        }
        // ================================ MULTI CARET DRAGGING ====================================
        else if(inTM.MDrag == 2){
            //cout << "MULTI CARET DRAGGING" << "\n";
            if (inTM.Format.HAlign <= .2) { // Left Aligned
                if(mp.x < 0){mp.x = 0;}
                if(mp.y < y0){mp.y = y0;}
            }
            else {
                if(mp.x < inTM.MinX){mp.x = inTM.MinX;}
                if(mp.y < y0+inTM.MinY){mp.y = y0+inTM.MinY;}
            }
            int diStart = int(inTM.DragPoint.y  / inTM.LineH);
            if(diStart > inTM.NbLines - 1){diStart = inTM.NbLines -1;}
            if(diStart < 0){diStart = 0;}
            int diEnd = mouseL;
            int yiStart = diStart;int yiEnd = diEnd;  
            if(diEnd < diStart){yiStart = yiEnd; yiEnd=diStart;}
            // figure out if it is a line drag or a box drag - behvior is different
            // line drag will pickup leftmost chars like empty lines on the left, box drag only gets whats in the box
            // has the wrong font - need to store a default width
            float dragW = abs(mp.x - inTM.DragPoint.x);
            int dragType = 0; if(dragW > inTM.CharW*.5f){dragType = 1;} // 0 = line drag, 1 = box drag
            inTM.SList = {}; // clear selection
            array<int,2> range;int prior=0;int startI;float startX = inTM.DragPoint.x;
            float endX = mp.x;

            if(dragType==1 && inTM.RightLimit){  // check if we are on the right of all text.  If so line drag.
                bool onRight = true;
                for(int i=yiStart;i<=yiEnd;i++){
                    range = inTM.YRange[i];
                    int minRange=range[1]-1;if(minRange<0){minRange=0;}
                    if(endX < inTM.CList[ range[1] ].x - inTM.CList[ minRange ].w*.5f){onRight = false;}
                }
                if(onRight){dragType = 0;}
            }
            if(dragType==0){  // line drag -> start at left side and walk to right for closest match on x
                for(int i=yiStart;i<=yiEnd;i++){
                    range = inTM.YRange[i];startI = range[0];
                    int minRange=range[1]-1;if(minRange<0){minRange=0;}
                    if(startX > inTM.CList[ range[1] ].x - inTM.CList[ minRange ].w*.5f){ startI = range[1];}
                    else{
                        for(int j=range[0];j<=range[1];j++){
                            j==0?prior=0:prior=j-1;
                            if( startX < inTM.CList[j].x - inTM.CList[prior].w*.5f){startI = j - 1;break;}
                        }
                    }
                    if(startI<0){startI=0;}
                    inTM.SList.push_back(  { startI,startI,startI,startI } );
                }
            }
            else{ // box drag -> only kick off a selection if there's text within the drag box
                float posX;int endI;int minRange;
                for(int i=yiStart;i<=yiEnd;i++){
                    startI = -1; endI = -1;range = inTM.YRange[i];
					for(int j=range[0];j<=range[1];j++){
						j==0?prior=0:prior=j-1;
						posX = inTM.CList[j].x - inTM.CList[prior].w*.5f;
						if( startX < posX && startI==-1){startI = j - 1;}
						if( endX < posX && endI==-1){endI = j - 1;}
					}
                    minRange = range[1] - 1;if(minRange<0){minRange=0;}
					if(endX > inTM.CList[ range[1] ].x - inTM.CList[minRange].w*.5f){ endI = range[1];}
					if(startI>-1&& endI==-1){if(endX > startX ){endI = range[1]-1;}} // got a start but not an end, fill end with far right max
                    if(endI>-1&& startI==-1){if(endX < startX && endX < inTM.CList[range[1]].x ){startI = range[1];}} // got a end but not a start, fill start with far right max
                    if(startI>-1&&endI>-1){
                        if(startI<0){startI=0;}
                        if(endI<0){endI=0;}
                        if(startI<range[0]){startI=range[0];}
                        if(endI<range[0]){endI=range[0];}
                        if(startI < endI){inTM.SList.push_back( { startI,endI,startI,endI,true} );}
                        else{inTM.SList.push_back( { startI,endI,endI,startI,false} );}
                    }
				}
            }
        }
        // ====================================== ADD MULTI CARET ======================================
        else if(VecInBB(gp,inTM.ClickRect) && DragStart() && (UILibIS->Ctrl || UILibIS->Alt) ){
            //cout << "ADD MULTI CLICK CARET" << "\n";
            // click to multi-add (Ctrl=Sublime Hotkey, Alt=VSCode Hotkey)
            int clickI = 0; 
            GET_CLICK_INDEX
            inTM.SList.push_back( { clickI,clickI,clickI,clickI } );
            inTM.LDrag = 2; // drag caret in single mode
        }
        // ====================================== ADD SINGLE CARET ======================================
        else if(VecInBB(gp,inTM.ClickRect) && DragStart()){
            //cout << "--- DRAG START:  ADD SINGLE CARET, mouseL: " << mouseL << "\n";
            if(inTM.CList.size()==0){
                inTM.SList = { { 0,0,0,0 }};return;
            };
            int clickI = 0; // GET_CLICK_INDEX
            if(inTM.YRange.size()==0){return;}
            
            array<int,2> range = inTM.YRange[mouseL];
            int prior=0;int lineEnd = range[1];
            int maxCl = (int)inTM.CList.size()-1;
            if(lineEnd <0){lineEnd=0;}
            if(lineEnd > maxCl){lineEnd = maxCl;}
            int priorLineE = lineEnd - 1;
            if(priorLineE<0){priorLineE=0;}
            if(mp.x < inTM.CList[range[0]].x){ clickI = range[0];} 
            else if(mp.x > inTM.CList[ lineEnd ].x - inTM.CList[ priorLineE ].w*.5f){ clickI = lineEnd;} 
            else{for(int i=range[0];i<=lineEnd;i++){i==0?prior=0:prior=i-1;if( mp.x < inTM.CList[i].x - inTM.CList[prior].w*.5f){clickI = i - 1;break;}}} 
            
            inTM.SList = { { clickI,clickI,clickI,clickI }};
            inTM.LDrag = 2; // drag caret in single mode
        }
    }
    void UIKeyInputTM(TextMap &inTM, string &inText, int inNlType, string &inInsertS, string &inKeyOp){ // general key input (notebook)
        //inOp = string summary of operation for transform keys pressed like: backspace, enter, ect
        if(inTM.Focus==0){return;}
        inKeyOp="";
        // inEditMode: 0=Regular Text Editing, 1=Python Editing
        string front = "";string middle = "";string back = "";
        int key = GetKeyDown();

        if ( inTM.Whitelist.size() > 0 ) {
            bool foundKey = false;
            if ( KeyIsPrintable(key) ) { // Only check whitelist for printable keys
                for ( auto wKey : inTM.Whitelist ) {
                    if ( key == wKey ) { foundKey = true; break; }
                }
            }
            else { foundKey = true; }
            if ( !foundKey ) { return; }
        }
        if ( inTM.Blacklist.size() > 0 ) {
            for ( auto bKey : inTM.Blacklist ) {
                if ( key == bKey ) { return; }
            }
        }
        
        if(UILibIS->Ctrl){
            if(key==67||key==86||key==65||key==88||key==47||key==91||key==93){}
            else{return;}
        }
        else if(key==10){}
        // Ignore shift+enter, that's for executing the current cell.
        else if(UILibIS->Shift && key == 257){return;} 
        else if(key<=31){return;}
        else if(key == 258){key=9;}
        else if(key>=320 && key<=336){
            if(key<=329){key-=272;}else if(key==330){key='.';}else if(key==331){key='/';}else if(key==332){key='*';}
            else if(key==333){key='-';}else if(key==334){key='+';}else if(key==335){key=257;}else if(key==336){key='=';}
        }
        else if(key >= 255){
            if(key==259||key==261||key==262||key==263||key==264||key==265||key==257||
            key==266||key==267||key==268||key==269
            ){}
            else{return;}
        }
        else if(UILibIS->Shift){
            if(key=='1'){key='!';}if(key=='2'){key='@';}if(key=='3'){key='#';}if(key=='4'){key='$';}if(key=='5'){key='%';}
            if(key=='6'){key='^';}if(key=='7'){key='&';}if(key=='8'){key='*';}if(key=='9'){key='(';}if(key=='0'){key=')';}
            if(key=='`'){key='~';}if(key=='['){key='{';}if(key==']'){key='}';}if(key=='\\'){key='|';}if(key==';'){key=':';}
            if(key=='\''){key='"';}if(key==','){key='<';}if(key=='.'){key='>';}if(key=='/'){key='?';}if(key=='='){key='+';}
            if(key=='-'){key='_';}
        }
        else if( !UILibIS->Caps && !UILibIS->Shift && key > 64 && key < 91){key += 32;} // A-Z convert to lowercase
        
        int keyChange = UILibIS->Key.Change;
        if(keyChange == 1){UILibCTX->HoldDown = 0;}
        auto currentTime = std::chrono::high_resolution_clock::now();       // high_resolution_clock is the clock with the shortest tick period, could be either system_clock or steady_clock
        if(key == UILibCTX->LastKey && keyChange != 1){ // slow down typing to a comfortable rate - avoid repeating letters to quickly
            float timeSpan = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - UILibCTX->LastKeyTime).count();
            float timeDelay = 0.2f;
            if(UILibCTX->HoldDown == 1){timeDelay = 0.03f;}
            if(timeSpan < timeDelay){
                //cout << " less than time Delay, timeSpan: " << timeSpan << "\n";
                return;
            }
            else{UILibCTX->HoldDown = 1;}
        }
        
        UILibCTX->LastKey = key;UILibCTX->LastKeyTime = currentTime;
        inTM.Shift = 0;
        if(inTM.CList.size()==0){inTM.SList={};}

        // ======================= key input for multi-line selection =======================
        if(key==9 && UILibIS->Shift){ inTM.LineTransfom = 2;inTM.Change = 1;return;} // tab back
        if(key==9){ // tab forward, check if it's multi-line tab
            list<int> selLs = GetSelectedLineStarts(inTM);
            if(selLs.size() > 1){ inTM.LineTransfom = 1;inTM.Change = 1;return;}
        }
        if(key==91 && UILibIS->Ctrl){inTM.LineTransfom = 2;inTM.Change = 1;return;} // tab back
        if(key==93 && UILibIS->Ctrl){inTM.LineTransfom = 1;inTM.Change = 1;return;} // tab forward
        if(key==47 && UILibIS->Ctrl){inTM.LineTransfom = 3;inTM.Change = 1;return;} // multi-line comment
        
        // =================== regular selection independent key input ======================
        
        if(key==261){  // delete
            for(int i=0;i<inTM.SList.size();i++){
                if(inTM.SList[i].S == inTM.SList[i].E){ // delete char ahead
                    int delI = inTM.SList[i].S;
                    inTM.CList[delI].Del = 1;
                    inTM.SList[i].SetCaret(inTM.SList[i].S-1);
                }
                else{ // delete selection range
                    for(int j=inTM.SList[i].S;j<inTM.SList[i].E;j++){inTM.CList[j].Del = 1;}
                    inTM.SList[i].SetCaret(inTM.SList[i].S-1);
                }
            }
            inTM.Shift = 1;
            inTM.Change = 1;
            return;
        }
        
        else if(key==259){ // backspace
            inKeyOp = "backspace";
            if(inTM.CList.size()==0){inTM.SList={{0,0,0,0,false}};inTM.Shift=0;inInsertS="";return;}
            inTM.DeleteSel();
            bool applyShift=true; // don't apply shift when backspacing on first character
            if(inTM.SList.size()==1){if(inTM.SList[0].S==0&&inTM.SList[0].E==0){applyShift=false;}}
            
            for(int i=0;i<inTM.SList.size();i++){
                if(inTM.SList[i].S != 0){
                    if(inTM.SList[i].S == inTM.SList[i].E){
                        int delI = inTM.SList[i].S - 1;
                        inTM.CList[delI].Del = 1;
                        inTM.SList[i].SetCaret(delI-1);
                        // int Type = 0; // 0=char, 1=newline, 2=EOL
                        if(inTM.CList[delI].Type==1){// delete newline
                            inKeyOp = "backspaceNL";
                        }
                    }
                    else{inTM.SList[i].SetCaret(inTM.SList[i].S-1);}
                }
            }
            if(applyShift){inTM.Shift = 1;}
            return;
        }
        else if(key==269){ // end key
            for(int i=0;i<inTM.SList.size();i++){
                int lineI = inTM.GetLine(inTM.SList[i].R);
                array<int,2> range = inTM.YRange[lineI];
                if(UILibIS->Shift){
                    inTM.SList[i].R = range[1];
                    inTM.SList[i].UpdateSE();
                }
                else{inTM.SList[i].SetCaret(range[1]);}
            }
            return;
        }
        else if(key==268){ // home key
            for(int i=0;i<inTM.SList.size();i++){
                int lineI = inTM.GetLine(inTM.SList[i].R);
                array<int,2> range = inTM.YRange[lineI];
                if(UILibIS->Shift){
                    inTM.SList[i].R = range[0];
                    inTM.SList[i].UpdateSE();
                }
                else{inTM.SList[i].SetCaret(range[0]);}
            }
            return;
        }
        else if(key==266){ // Page Up key
            // for now go up 30 lines or to the first char.
            // In the future we can tune it to figure out the number of lines visible on the screen, and go up that amount
            // TODO: Get the text editor scrolling to catch up with the PageUp change
            for(int i=0;i<inTM.SList.size();i++){
                int lineI = inTM.GetLine(inTM.SList[i].R);
                int amountOver = abs(inTM.SList[i].R - inTM.YRange[lineI][0]);
                lineI -= 30;
                if(lineI<0){lineI=0;amountOver=0;}
                array<int,2> range = inTM.YRange[lineI];
                int newCaret = range[0] + amountOver;
                if(newCaret > range[1]){newCaret = range[1];}
                if(UILibIS->Shift){
                    inTM.SList[i].R = newCaret;
                    inTM.SList[i].UpdateSE();
                }
                else{inTM.SList[i].SetCaret(newCaret);}
            }
            return;
        }
        else if(key==267){ // Page Down key
            // for now go down 30 lines or to the first char.
            // In the future we can tune it to figure out the number of lines visible on the screen, and go up that amount
            // TODO: Get the text editor scrolling to catch up with the PageDown change
            for(int i=0;i<inTM.SList.size();i++){
                int lineI = inTM.GetLine(inTM.SList[i].R);
                int amountOver = abs(inTM.SList[i].R - inTM.YRange[lineI][0]);
                lineI += 30;bool hitMax=false;
                if(lineI>inTM.NbLines-1){lineI=inTM.NbLines-1;hitMax=true;}
                array<int,2> range = inTM.YRange[lineI];
                int newCaret = range[0] + amountOver;
                if(newCaret > range[1] || hitMax){newCaret = range[1];}
                if(UILibIS->Shift){
                    inTM.SList[i].R = newCaret;
                    inTM.SList[i].UpdateSE();
                }
                else{inTM.SList[i].SetCaret(newCaret);}
            }
            return;
        }
        else if(key==257||key==10){ // enter key
            inKeyOp = "enter";
            string nl = "\r\n";if(inNlType== 0){nl="\n";}
            for(int i=0;i<inTM.SList.size();i++){
                int addI = inTM.SList[i].S;
                int maxI = (int)inTM.CList.size()-1;if(addI <0){addI=0;}if(addI > maxI){addI=maxI;}
                inTM.CList[addI].Op = 1;inTM.SList[i].SetCaret(addI);
            }
            inInsertS = nl;inTM.Change = 1;
            inTM.Shift = 1;
            if(inTM.Language==1){inTM.LineTransfom=5;}
            return;

        }
        else if(UILibIS->Ctrl && (key==65 || key==97)){  // select all
            int nbChars = (int)inTM.CList.size()-1;inTM.SList = { { 0, nbChars, 0 , nbChars }}; 
        }
        else if(key==262){ // arrow right
            if(UILibIS->Shift){
                UITextSelection * sl;
                for(int i=0;i<inTM.SList.size();i++){
                    sl = &inTM.SList[i];
                    if(sl->R < inTM.CList.size()-1){ sl->R++;}
                    sl->UpdateSE();
                }
            }
            else{
                for(int i=0;i<inTM.SList.size();i++){
                    if(inTM.SList[i].S > inTM.SList[i].E){
                        if(inTM.SList[i].S < inTM.CList.size()){inTM.SList[i].SetCaret(inTM.SList[i].S+1);}
                    }
                    else{
                        if(inTM.SList[i].E < inTM.CList.size()){inTM.SList[i].SetCaret(inTM.SList[i].E+1);}
                    }
                }
            }
            inTM.SelChange = 1;
        }
        else if(key==263){ // arrow left
            if(UILibIS->Shift){
                UITextSelection * sl;
                for(int i=0;i<inTM.SList.size();i++){
                    sl = &inTM.SList[i];
                    if(sl->R > 0){ sl->R--;}
                    sl->UpdateSE();
                }
            }
            else{
                for(int i=0;i<inTM.SList.size();i++){
                    if(inTM.SList[i].S < inTM.SList[i].E){
                        if(inTM.SList[i].S >= 0){
                            int endI = inTM.SList[i].S-1;if(endI<0){endI=0;}inTM.SList[i].SetCaret(endI);
                        }
                    }
                    else{
                        if(inTM.SList[i].E >= 0){
                            int endI = inTM.SList[i].E-1;if(endI<0){endI=0;}inTM.SList[i].SetCaret(endI);
                        }
                    }
                }
            }
            inTM.SelChange = 1;
        }
        else if(key==264){ // arrow down
            if(UILibIS->Shift){
                for(int i=0;i<inTM.SList.size();i++){
                    int xI = 0;int lineI = inTM.GetLine(inTM.SList[i].R) + 1;
                    int maxLine = (int)inTM.YRange.size()-1;if(lineI > maxLine){lineI = maxLine;}
                    float xPos = inTM.CList[inTM.SList[i].D].x;GET_XPOS_INDEX
                    inTM.SList[i].R = xI;
                    inTM.SList[i].UpdateSE();
                }
            }
            else{
                for(int i=0;i<inTM.SList.size();i++){
                    int xI = 0;int lineI = inTM.GetLine(inTM.SList[i].E) + 1;
                    int maxLine = (int)inTM.YRange.size()-1;if(lineI > maxLine){lineI = maxLine;}
                    float xPos = inTM.CList[inTM.SList[i].E].x;GET_XPOS_INDEX
                    inTM.SList[i].SetCaret(xI);
                }
            }
            inTM.SelChange = 1;
        }
        else if(key==265){ // arrow up
            if(UILibIS->Shift){
                for(int i=0;i<inTM.SList.size();i++){
                    int xI = 0;int lineI = inTM.GetLine(inTM.SList[i].R) - 1;
                    if(lineI < 0){lineI = 0;}
                    float xPos = inTM.CList[inTM.SList[i].D].x;
                    GET_XPOS_INDEX
                    inTM.SList[i].R = xI;
                    inTM.SList[i].UpdateSE();
                }
            }
            else{
                for(int i=0;i<inTM.SList.size();i++){
                    int xI = 0;int lineI = inTM.GetLine(inTM.SList[i].E) - 1;
                    if(lineI < 0){lineI = 0;}
                    if(inTM.SList[i].E>(int)inTM.CList.size()-1){inTM.SList[i].E=(int)inTM.CList.size()-1;}
                    float xPos = inTM.CList[inTM.SList[i].E].x;
                    GET_XPOS_INDEX
                    inTM.SList[i].SetCaret(xI);inTM.SelChange = 1;
                }
            }
            inTM.SelChange = 1;
        }
        else if(UILibIS->Ctrl && key==67){ // copy
            // multiple selections will be seperated by "\r\n" character
            if(inTM.SList.size() == 0){return;}
            string copyStr = "";
            if(inTM.SList.size() == 1){
                if(inTM.SList[0].S == inTM.SList[0].E){return;} // FUTURE FEATURE HERE - caret selection copy line
                for(int j=inTM.SList[0].S;j<inTM.SList[0].E;j++){copyStr += inTM.CList[j].Char;}
                UISetClipboard(copyStr);
            }
            else{
                string newLine = "\r\n";if(inNlType== 0){newLine="\n";}
                for(int i=0;i<inTM.SList.size();i++){
                    if(inTM.SList[i].S != inTM.SList[i].E){
                        if(inTM.SList[i].S == inTM.SList[i].E){continue;} // FUTURE FEATURE HERE - caret selection copy line
                        for(int j=inTM.SList[i].S;j<inTM.SList[i].E;j++){copyStr += inTM.CList[j].Char;}
                        if(i != inTM.SList.size()-1){copyStr += newLine;}
                    }
                }
                UISetClipboard(copyStr);
            }
        }
        else if(UILibIS->Ctrl && key==88){ // cut 

            // multiple selections will be seperated by "\r\n" character
            if(inTM.SList.size() == 0){return;}
            string copyStr = "";
            if(inTM.SList.size() == 1){
                if(inTM.SList[0].S == inTM.SList[0].E){return;} // FUTURE FEATURE HERE - caret selection copy line
                for(int j=inTM.SList[0].S;j<inTM.SList[0].E;j++){copyStr += inTM.CList[j].Char;}
                UISetClipboard(copyStr);
            }
            else{
                string newLine = "\r\n";if(inNlType== 0){newLine="\n";}
                for(int i=0;i<inTM.SList.size();i++){
                    if(inTM.SList[i].S != inTM.SList[i].E){
                        if(inTM.SList[i].S == inTM.SList[i].E){continue;} // FUTURE FEATURE HERE - caret selection copy line
                        for(int j=inTM.SList[i].S;j<inTM.SList[i].E;j++){copyStr += inTM.CList[j].Char;}
                        if(i != inTM.SList.size()-1){copyStr += newLine;}
                    }
                }
                UISetClipboard(copyStr);
            }
            inTM.DeleteSel();
        }
        else if(UILibIS->Ctrl && key==86 ){ // paste

            string clipB = UIGetClipboard();
            if (clipB.empty()){return;}
            
            int cbSize = UIClipboardSize(true);
            UIConformNewlines(clipB, inNlType);

            if(inTM.SList.size()==0){
                int pasteI = cbSize;
                inTM.SList = { { pasteI,pasteI,pasteI,pasteI }};
            }
            else{
                int maxJ;
                for(int i=0;i<inTM.SList.size();i++){
                    if(inTM.SList[i].S != inTM.SList[i].E){
                        maxJ = inTM.SList[i].E;if(maxJ>(int)inTM.CList.size()){maxJ=(int)inTM.CList.size();}
                        for(int j=inTM.SList[i].S;j<maxJ;j++){inTM.CList[j].Del = 1;}  // delete
                        int addI = inTM.SList[i].S-1;
                        int maxI = (int)inTM.CList.size()-1;if(addI <0){addI=0;}if(addI > maxI){addI=maxI;}
                        inTM.CList[addI].Op = 2; // Insert After Char
                        inTM.Shift = cbSize - 1;
                    }
                    else{
                        int addI = inTM.SList[i].S;
                        int maxI = (int)inTM.CList.size()-1;if(addI <0){addI=0;}if(addI > maxI){addI=maxI;}
                        inTM.CList[addI].Op = 1;inTM.SList[i].SetCaret(addI);
                        inTM.Shift = (int)clipB.size();
                    }
                }
            }
            inInsertS = clipB;inTM.Change = 1;
        }
        else if(key=='"' || key=='\''){ // single & double quotes
            if(inTM.SList.size()==0){
                int newI = 1;
                inTM.SList = { { newI,newI,newI,newI }};
            }
            else{
                for(int i=0;i<inTM.SList.size();i++){
                    int maxI;
                    if(inTM.SList[i].S != inTM.SList[i].E){  // put quotes around the string
                        inTM.CList[inTM.SList[i].S].Op = 1;
                        inTM.CList[inTM.SList[i].E-1].Op = 2;
                        inTM.SList[i].Op = 1;
                    }
                    else{
                        int addI = inTM.SList[i].S;
                        maxI = (int)inTM.CList.size()-1;if(addI <0){addI=0;}if(addI > maxI){addI=maxI;}
                        inTM.CList[addI].Op = 1;
                        inTM.Shift = 1;
                        inTM.SList[i].SetCaret(addI);
                    }
                }
                inInsertS = key;inTM.Change = 1;
            }
        }
        else if(key=='('||key=='['||key=='{'){ // single & double quotes
            if(inTM.SList.size()==0){
                int newI = 1;
                inTM.SList = { { newI,newI,newI,newI }};
            }
            else{
                for(int i=0;i<inTM.SList.size();i++){
                    int maxI;
                    if(inTM.SList[i].S != inTM.SList[i].E){  // complete bracket on other side
                        inTM.CList[inTM.SList[i].S].Op = 1;
                        inTM.CList[inTM.SList[i].E-1].Op = 3;
                        inTM.SList[i].Op = 1;
                    }
                    else{
                        int addI = inTM.SList[i].S;
                        maxI = (int)inTM.CList.size()-1;if(addI <0){addI=0;}if(addI > maxI){addI=maxI;}
                        inTM.CList[addI].Op = 1;
                        inTM.Shift = 1;
                        inTM.SList[i].SetCaret(addI);
                    }
                }
                inInsertS = key;inTM.Change = 1;
            }

        }
        else{ // general typing - if you have a selection, clear it

            // if there is no selection - start adding to the end
            if(inTM.SList.size()==0){
                int newI = 1;
                inTM.SList = { { newI,newI,newI,newI }};
            }
            else{
                for(int i=0;i<inTM.SList.size();i++){
                    int maxI;
                    if(inTM.SList[i].S != inTM.SList[i].E){
                        maxI=inTM.SList[i].E;if(maxI>(int)inTM.CList.size()){maxI=(int)inTM.CList.size();}
                        for(int j=inTM.SList[i].S;j<maxI;j++){inTM.CList[j].Del = 1;}
                        inTM.SList[i].SetCaret(inTM.SList[i].S);int addI = inTM.SList[i].S-1;
                        maxI = (int)inTM.CList.size()-1;if(addI <0){addI=0;}if(addI > maxI){addI=maxI;}
                        inTM.CList[addI].Op = 2;
                        if(inTM.SList[i].S==0){inTM.Shift=1;}
                    }
                    else{
                        int addI = inTM.SList[i].S;
                        maxI = (int)inTM.CList.size()-1;if(addI <0){addI=0;}if(addI > maxI){addI=maxI;}
                        inTM.CList[addI].Op = 1;
                        inTM.Shift = 1;
                        inTM.SList[i].SetCaret(addI);
                    }
                }
            }
            inInsertS = key;inTM.Change = 1;
        }
    }
    void UIKeyInputTM_ReadOnly(TextMap &inTM, int inNlType){ // general key input (notebook)
        
        // key input for read only text: copy, arrow keys, select all, ect
        if(inTM.Focus==0){return;}
        string front = "";string middle = "";string back = "";
        int key = GetKeyDown();
        if(key==0){return;}

        int keyChange = UILibIS->Key.Change;
        if(keyChange == 1){UILibCTX->HoldDown = 0;}
        auto currentTime = std::chrono::high_resolution_clock::now();       // high_resolution_clock is the clock with the shortest tick period, could be either system_clock or steady_clock
        if(key == UILibCTX->LastKey && keyChange != 1){ // slow down typing to a comfortable rate - avoid repeating letters to quickly
            float timeSpan = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - UILibCTX->LastKeyTime).count();
            float timeDelay = 0.2f;
            if(UILibCTX->HoldDown == 1){timeDelay = 0.03f;}
            if(timeSpan < timeDelay){
                //cout << "       less than time Delay, timeSpan: " << timeSpan << "\n";
                return;
            }
            else{UILibCTX->HoldDown = 1;}
        }
        
        UILibCTX->LastKey = key;UILibCTX->LastKeyTime = currentTime;
        inTM.Shift = 0;
        
        if(UILibIS->Ctrl && (key==65 || key==97)){  // select all
            int nbChars = (int)inTM.CList.size()-1;inTM.SList = { { 0, nbChars, 0 , nbChars }}; 
        }
        else if(key==262){ // arrow right
            if(UILibIS->Shift){
                UITextSelection * sl;
                for(int i=0;i<inTM.SList.size();i++){
                    sl = &inTM.SList[i];
                    if(sl->R < inTM.CList.size()-1){ sl->R++;}
                    sl->UpdateSE();
                }
            }
            else{
                for(int i=0;i<inTM.SList.size();i++){
                    if(inTM.SList[i].S > inTM.SList[i].E){
                        if(inTM.SList[i].S < inTM.CList.size()){inTM.SList[i].SetCaret(inTM.SList[i].S+1);}
                    }
                    else{
                        if(inTM.SList[i].E < inTM.CList.size()){inTM.SList[i].SetCaret(inTM.SList[i].E+1);}
                    }
                }
            }
            inTM.SelChange = 1;
        }
        else if(key==263){ // arrow left
            if(UILibIS->Shift){
                UITextSelection * sl;
                for(int i=0;i<inTM.SList.size();i++){
                    sl = &inTM.SList[i];
                    if(sl->R > 0){ sl->R--;}
                    sl->UpdateSE();
                }
            }
            else{
                for(int i=0;i<inTM.SList.size();i++){
                    if(inTM.SList[i].S < inTM.SList[i].E){
                        if(inTM.SList[i].S >= 0){
                            int endI = inTM.SList[i].S-1;if(endI<0){endI=0;}inTM.SList[i].SetCaret(endI);
                        }
                    }
                    else{
                        if(inTM.SList[i].E >= 0){
                            int endI = inTM.SList[i].E-1;if(endI<0){endI=0;}inTM.SList[i].SetCaret(endI);
                        }
                    }
                }
            }
            inTM.SelChange = 1;
        }
        else if(key==264){ // arrow down
            if(UILibIS->Shift){
                for(int i=0;i<inTM.SList.size();i++){
                    int xI = 0;int lineI = inTM.GetLine(inTM.SList[i].R) + 1;
                    int maxLine = (int)inTM.YRange.size()-1;if(lineI > maxLine){lineI = maxLine;}
                    float xPos = inTM.CList[inTM.SList[i].D].x;GET_XPOS_INDEX
                    inTM.SList[i].R = xI;
                    inTM.SList[i].UpdateSE();
                }
            }
            else{
                for(int i=0;i<inTM.SList.size();i++){
                    int xI = 0;int lineI = inTM.GetLine(inTM.SList[i].E) + 1;
                    int maxLine = (int)inTM.YRange.size()-1;if(lineI > maxLine){lineI = maxLine;}
                    float xPos = inTM.CList[inTM.SList[i].E].x;GET_XPOS_INDEX
                    inTM.SList[i].SetCaret(xI);
                }
            }
            inTM.SelChange = 1;
        }
        else if(key==265){ // arrow up
            if(UILibIS->Shift){
                for(int i=0;i<inTM.SList.size();i++){
                    int xI = 0;int lineI = inTM.GetLine(inTM.SList[i].R) - 1;
                    if(lineI < 0){lineI = 0;}
                    float xPos = inTM.CList[inTM.SList[i].D].x;
                    GET_XPOS_INDEX
                    inTM.SList[i].R = xI;
                    inTM.SList[i].UpdateSE();
                }
            }
            else{
                for(int i=0;i<inTM.SList.size();i++){
                    int xI = 0;int lineI = inTM.GetLine(inTM.SList[i].E) - 1;
                    if(lineI < 0){lineI = 0;}
                    if(inTM.SList[i].E>(int)inTM.CList.size()-1){inTM.SList[i].E=(int)inTM.CList.size()-1;}
                    float xPos = inTM.CList[inTM.SList[i].E].x;
                    GET_XPOS_INDEX
                    inTM.SList[i].SetCaret(xI);inTM.SelChange = 1;
                }
            }
            inTM.SelChange = 1;
        }
        else if(UILibIS->Ctrl && key==67){ // copy
            // multiple selections will be seperated by "\r\n" character
            if(inTM.SList.size() == 0){return;}
            string copyStr = "";
            if(inTM.SList.size() == 1){
                if(inTM.SList[0].S == inTM.SList[0].E){return;} // FUTURE FEATURE HERE - caret selection copy line
                for(int j=inTM.SList[0].S;j<inTM.SList[0].E;j++){copyStr += inTM.CList[j].Char;}
                UISetClipboard(copyStr);
            }
            else{
                string newLine = "\r\n";if(inNlType== 0){newLine="\n";}
                for(int i=0;i<inTM.SList.size();i++){
                    if(inTM.SList[i].S != inTM.SList[i].E){
                        if(inTM.SList[i].S == inTM.SList[i].E){continue;} // FUTURE FEATURE HERE - caret selection copy line
                        for(int j=inTM.SList[i].S;j<inTM.SList[i].E;j++){copyStr += inTM.CList[j].Char;}
                        if(i != inTM.SList.size()-1){copyStr += newLine;}
                    }
                }
                UISetClipboard(copyStr);
            }
        }
    }
    void UIKeyInputSS(TextMap &inTM, string &inText, int inNlType, string &inInsertS){ // spreadsheet style key input
        
        if(inTM.Focus==0){return;}
        // inEditMode: 0=Regular Text Editing, 1=Python Editing
        string front = "";string middle = "";string back = "";
        int key = GetKeyDown();

        if ( inTM.Whitelist.size() > 0 ) {
            bool foundKey = false;
            if ( KeyIsPrintable(key) ) { // Only check whitelist for printable keys
                for ( auto wKey : inTM.Whitelist ) {
                    if ( key == wKey ) { foundKey = true; break; }
                }
            }
            else { foundKey = true; }
            if ( !foundKey ) { return; }
        }
        if ( inTM.Blacklist.size() > 0 ) {
            for ( auto bKey : inTM.Blacklist ) {
                if ( key == bKey ) { return; }
            }
        }

        if(UILibIS->Ctrl){
            if(key==67||key==86||key==65||key==88||key==47||key==91||key==93){}
            else{return;}
        }
        else if(key==10){}
        else if(key<=31){return;}
        else if(key == 258){key=9;}
        else if(key>=320 && key<=336){
            if(key<=329){key-=272;}else if(key==330){key='.';}else if(key==331){key='/';}else if(key==332){key='*';}
            else if(key==333){key='-';}else if(key==334){key='+';}else if(key==335){key=257;}else if(key==336){key='=';}
        }
        else if(key >= 255){if(key==259||key==261||key==262||key==263||key==264||key==265||key==257||key==268||key==269){}else{return;}}
        else if(UILibIS->Shift){
            if(key=='1'){key='!';}if(key=='2'){key='@';}if(key=='3'){key='#';}if(key=='4'){key='$';}if(key=='5'){key='%';}
            if(key=='6'){key='^';}if(key=='7'){key='&';}if(key=='8'){key='*';}if(key=='9'){key='(';}if(key=='0'){key=')';}
            if(key=='`'){key='~';}if(key=='['){key='{';}if(key==']'){key='}';}if(key=='\\'){key='|';}if(key==';'){key=':';}
            if(key=='\''){key='"';}if(key==','){key='<';}if(key=='.'){key='>';}if(key=='/'){key='?';}if(key=='='){key='+';}
            if(key=='-'){key='_';}
        }
        else if(!UILibIS->Caps && !UILibIS->Shift && key > 64 && key < 91){key += 32;} // A-Z convert to lowercase
        
        int keyChange = UILibIS->Key.Change;
        if(keyChange == 1){UILibCTX->HoldDown = 0;}
        auto currentTime = std::chrono::high_resolution_clock::now();       // high_resolution_clock is the clock with the shortest tick period, could be either system_clock or steady_clock
        if(key == UILibCTX->LastKey && keyChange != 1){ // slow down typing to a comfortable rate - avoid repeating letters to quickly
            float timeSpan = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - UILibCTX->LastKeyTime).count();
            float timeDelay = 0.2f;
            if(UILibCTX->HoldDown == 1){timeDelay = 0.03f;}
            if(timeSpan < timeDelay){
                //cout << " less than time Delay, timeSpan: " << timeSpan << "\n";
                return;
            }
            else{UILibCTX->HoldDown = 1;}
        }
        
        UILibCTX->LastKey = key;UILibCTX->LastKeyTime = currentTime;
        inTM.Shift = 0;
        if(inTM.CList.size()==0){inTM.SList={};}

        // ======================= key input for multi-line selection =======================
        if(key==9 && UILibIS->Shift){ inTM.LineTransfom = 2;inTM.Change = 1;return;} // tab back
        if(key==9){ // tab forward, check if it's multi-line tab
            list<int> selLs = GetSelectedLineStarts(inTM);
            if(selLs.size() > 1){ inTM.LineTransfom = 1;inTM.Change = 1;return;}
        }
        if(key==91 && UILibIS->Ctrl){inTM.LineTransfom = 2;inTM.Change = 1;return;} // tab back
        if(key==93 && UILibIS->Ctrl){inTM.LineTransfom = 1;inTM.Change = 1;return;} // tab forward
        if(key==47 && UILibIS->Ctrl){inTM.LineTransfom = 3;inTM.Change = 1;return;} // multi-line comment
        
        // =================== regular selection independent key input ======================
        
        if(key==261){  // delete
            for(int i=0;i<inTM.SList.size();i++){
                if(inTM.SList[i].S == inTM.SList[i].E){ // delete char ahead
                    int delI = inTM.SList[i].S;
                    inTM.CList[delI].Del = 1;
                    inTM.SList[i].SetCaret(inTM.SList[i].S-1);
                }
                else{ // delete selection range
                    for(int j=inTM.SList[i].S;j<inTM.SList[i].E;j++){inTM.CList[j].Del = 1;}
                    inTM.SList[i].SetCaret(inTM.SList[i].S-1);
                }
            }
            inTM.Shift = 1;
            inTM.Change = 1;
            return;
        }
        
        else if(key==259){ // backspace
            if(inTM.CList.size()==0){inTM.SList={{0,0,0,0,false}};inTM.Shift=0;inInsertS="";return;}
            inTM.DeleteSel();
            bool applyShift=true; // don't apply shift when backspacing on first character
            if(inTM.SList.size()==1){if(inTM.SList[0].S==0&&inTM.SList[0].E==0){applyShift=false;}}
            
            for(int i=0;i<inTM.SList.size();i++){
                if(inTM.SList[i].S != 0){
                    if(inTM.SList[i].S == inTM.SList[i].E){int delI = inTM.SList[i].S - 1;inTM.CList[delI].Del = 1;inTM.SList[i].SetCaret(delI-1);}
                    else{inTM.SList[i].SetCaret(inTM.SList[i].S-1);}
                }
            }
            if(applyShift){inTM.Shift = 1;}
            return;
        }
        else if(key==269){ // end key
            for(int i=0;i<inTM.SList.size();i++){
                int lineI = inTM.GetLine(inTM.SList[i].R);
                array<int,2> range = inTM.YRange[lineI];
                if(UILibIS->Shift){
                    inTM.SList[i].R = range[1];
                    inTM.SList[i].UpdateSE();
                }
                else{inTM.SList[i].SetCaret(range[1]);}
            }
            return;
        }
        else if(key==268){ // home key
            for(int i=0;i<inTM.SList.size();i++){
                int lineI = inTM.GetLine(inTM.SList[i].R);
                array<int,2> range = inTM.YRange[lineI];
                if(UILibIS->Shift){
                    inTM.SList[i].R = range[0];
                    inTM.SList[i].UpdateSE();
                }
                else{inTM.SList[i].SetCaret(range[0]);}
            }
            return;
        }
        else if((key==257||key==10) && UILibIS->Alt){ // enter key
            
            string nl = "\r\n";if(inNlType== 0){nl="\n";}
            for(int i=0;i<inTM.SList.size();i++){
                int addI = inTM.SList[i].S;inTM.CList[addI].Op = 1;inTM.SList[i].SetCaret(addI);
            }
            inInsertS = nl;inTM.Change = 1;
            inTM.Shift = 1;
            //if(inNlType== 0){inTM.Shift = 0;}
            if(inTM.Language==1){inTM.LineTransfom=5;}
            return;

        }
        else if(key==257||key==10){ return; }// enter key without alt - will exit the cell
        else if(UILibIS->Ctrl && (key==65 || key==97)){  // select all
            int nbChars = (int)inTM.CList.size()-1;inTM.SList = { { 0, nbChars, 0 , nbChars }}; 
        }
        else if(key==262){ // arrow right
            if(UILibIS->Shift){
                UITextSelection * sl;
                for(int i=0;i<inTM.SList.size();i++){
                    sl = &inTM.SList[i];
                    if(sl->R < inTM.CList.size()-1){ sl->R++;}
                    sl->UpdateSE();
                }
            }
            else{
                for(int i=0;i<inTM.SList.size();i++){
                    if(inTM.SList[i].S < inTM.CList.size()-1){inTM.SList[i].SetCaret(inTM.SList[i].S+1);}
                }
            }
            inTM.SelChange = 1;
        }
        else if(key==263){ // arrow left
            if(UILibIS->Shift){
                UITextSelection * sl;
                for(int i=0;i<inTM.SList.size();i++){
                    sl = &inTM.SList[i];
                    if(sl->R > 0){ sl->R--;}
                    sl->UpdateSE();
                }
            }
            else{
                for(int i=0;i<inTM.SList.size();i++){
                    if(inTM.SList[i].S > 0){inTM.SList[i].SetCaret(inTM.SList[i].S-1);}
                }
            }
            inTM.SelChange = 1;
        }
        else if(key==264){ // arrow down
            if(UILibIS->Shift){
                for(int i=0;i<inTM.SList.size();i++){
                    int xI = 0;int lineI = inTM.GetLine(inTM.SList[i].R) + 1;
                    int maxLine = (int)inTM.YRange.size()-1;if(lineI > maxLine){lineI = maxLine;}
                    float xPos = inTM.CList[inTM.SList[i].D].x;GET_XPOS_INDEX
                    inTM.SList[i].R = xI;
                    inTM.SList[i].UpdateSE();
                }
            }
            else{
                for(int i=0;i<inTM.SList.size();i++){
                    int xI = 0;int lineI = inTM.GetLine(inTM.SList[i].E) + 1;
                    int maxLine = (int)inTM.YRange.size()-1;if(lineI > maxLine){lineI = maxLine;}
                    float xPos = inTM.CList[inTM.SList[i].E].x;GET_XPOS_INDEX
                    inTM.SList[i].SetCaret(xI);
                }
            }
            inTM.SelChange = 1;
        }
        else if(key==265){ // arrow up
            if(UILibIS->Shift){
                for(int i=0;i<inTM.SList.size();i++){
                    int xI = 0;int lineI = inTM.GetLine(inTM.SList[i].R) - 1;
                    if(lineI < 0){lineI = 0;}
                    float xPos = inTM.CList[inTM.SList[i].D].x;GET_XPOS_INDEX
                    inTM.SList[i].R = xI;
                    inTM.SList[i].UpdateSE();
                }
            }
            else{
                for(int i=0;i<inTM.SList.size();i++){
                    int xI = 0;int lineI = inTM.GetLine(inTM.SList[i].E) - 1;
                    if(lineI < 0){lineI = 0;}
                    float xPos = inTM.CList[inTM.SList[i].E].x;GET_XPOS_INDEX
                    inTM.SList[i].SetCaret(xI);inTM.SelChange = 1;
                }
            }
            inTM.SelChange = 1;
        }
        else if(UILibIS->Ctrl && key==67){ // copy
            // multiple selections will be seperated by "\r\n" character
            if(inTM.SList.size() == 0){return;}
            string copyStr = "";
            if(inTM.SList.size() == 1){
                if(inTM.SList[0].S == inTM.SList[0].E){return;} // FUTURE FEATURE HERE - caret selection copy line
                for(int j=inTM.SList[0].S;j<inTM.SList[0].E;j++){copyStr += inTM.CList[j].Char;}
                UISetClipboard(copyStr);
            }
            else{
                string newLine = "\r\n";if(inNlType== 0){newLine="\n";}
                for(int i=0;i<inTM.SList.size();i++){
                    if(inTM.SList[i].S != inTM.SList[i].E){
                        if(inTM.SList[i].S == inTM.SList[i].E){continue;} // FUTURE FEATURE HERE - caret selection copy line
                        for(int j=inTM.SList[i].S;j<inTM.SList[i].E;j++){copyStr += inTM.CList[j].Char;}
                        if(i != inTM.SList.size()-1){copyStr += newLine;}
                    }
                }
                UISetClipboard(copyStr);
            }
        }
        else if(UILibIS->Ctrl && key==88){ // cut 

            // multiple selections will be seperated by "\r\n" character
            if(inTM.SList.size() == 0){return;}
            string copyStr = "";
            if(inTM.SList.size() == 1){
                if(inTM.SList[0].S == inTM.SList[0].E){return;} // FUTURE FEATURE HERE - caret selection copy line
                for(int j=inTM.SList[0].S;j<inTM.SList[0].E;j++){copyStr += inTM.CList[j].Char;}
                UISetClipboard(copyStr);
            }
            else{
                string newLine = "\r\n";if(inNlType== 0){newLine="\n";}
                for(int i=0;i<inTM.SList.size();i++){
                    if(inTM.SList[i].S != inTM.SList[i].E){
                        if(inTM.SList[i].S == inTM.SList[i].E){continue;} // FUTURE FEATURE HERE - caret selection copy line
                        for(int j=inTM.SList[i].S;j<inTM.SList[i].E;j++){copyStr += inTM.CList[j].Char;}
                        if(i != inTM.SList.size()-1){copyStr += newLine;}
                    }
                }
                UISetClipboard(copyStr);
            }
            inTM.DeleteSel();
        }
        else if(UILibIS->Ctrl && key==86 ){ // paste
            string clipB = UIGetClipboard();
            if (clipB.empty()){return;}
            int cbSize = UIClipboardSize(true);
            UIConformNewlines(clipB, inNlType);
            if(inTM.SList.size()==0){
                int pasteI = cbSize;
                inTM.SList = { { pasteI,pasteI,pasteI,pasteI }};
            }
            else{
                for(int i=0;i<inTM.SList.size();i++){
                    if(inTM.SList[i].S != inTM.SList[i].E){
                        for(int j=inTM.SList[i].S;j<inTM.SList[i].E;j++){inTM.CList[j].Del = 1;}  // delete
                        inTM.SList[i].SetCaret(inTM.SList[i].S);
                        int addI = inTM.SList[i].S-1;
                        int maxI = (int)inTM.CList.size()-1;if(addI <0){addI=0;}if(addI > maxI){addI=maxI;}
                        inTM.CList[addI].Op = 2; // Insert After Char
                        inTM.Shift = cbSize - 1;
                    }
                    else{
                        int addI = inTM.SList[i].S;
                        int maxI = (int)inTM.CList.size()-1;if(addI <0){addI=0;}if(addI > maxI){addI=maxI;}
                        inTM.CList[addI].Op = 1;inTM.SList[i].SetCaret(addI);
                        inTM.Shift = (int)clipB.size();
                    }
                }
            }
            inInsertS = clipB;inTM.Change = 1;
        }
        else{ // general typing - if you have a selection, clear it

            // if there is no selection - start adding to the end
            if(inTM.SList.size()==0){
                int newI = 1;
                inTM.SList = { { newI,newI,newI,newI }};
            }
            else{
                for(int i=0;i<inTM.SList.size();i++){
                    if(inTM.SList[i].S != inTM.SList[i].E){
                        for(int j=inTM.SList[i].S;j<inTM.SList[i].E;j++){inTM.CList[j].Del = 1;}
                        inTM.SList[i].SetCaret(inTM.SList[i].S);int addI = inTM.SList[i].S-1;
                        int maxI = (int)inTM.CList.size()-1;if(addI <0){addI=0;}if(addI > maxI){addI=maxI;}
                        inTM.CList[addI].Op = 2;
                        if(inTM.SList[i].S==0){inTM.Shift=1;}
                    }
                    else{
                        int addI = inTM.SList[i].S;
                        int maxI = (int)inTM.CList.size()-1;if(addI <0){addI=0;}if(addI > maxI){addI=maxI;}
                        inTM.CList[addI].Op = 1;
                        inTM.Shift = 1;
                        inTM.SList[i].SetCaret(addI);
                    }
                }
            }
            inInsertS = key;inTM.Change = 1;
        }

    }
    
    string UIGetKeyInput(string inStr, int &inInputIndex, int &inEndIndex, int &inChange, bool &inFocused) {
        
        // key is a mix of GLFW an ASCII codes, which were determined through reverse engineering key input
        // GLFW key map: https://www.glfw.org/docs/3.3.2/group__keys.html
        
        inChange = 0;
        string outStr = inStr;string front = "";string middle = "";string back = "";
        if(inEndIndex == -1){for(int i=0;i<inStr.size();i++){if(i<inInputIndex){front += inStr[i];}else{back += inStr[i];}}}
        else{for(int i=0;i<inStr.size();i++){
            if(i<inInputIndex){front += inStr[i];}else if(i<inEndIndex){middle += inStr[i];}else{back += inStr[i];}}
        }

        // Use CodePoint if it exists
        unsigned int key = UILibIS->Key.CodePoint;
        if ( key != 0 ) {
            UILibIS->Key.Reset = true;  // Signal to clear the text buffer
            if ( UILibCTX->InputValidateKey && !(*UILibCTX->InputValidateKey)(key) ) { return outStr; }
            // std::cout << "Processing CodePoint: " << key << " " << (char) key << std::endl;
            front += key;
            outStr = front + back;
            inInputIndex++;
            inEndIndex = -1;
            inChange = 1;
            return outStr;
        }

        // Otherwise, use the current key down
        if ( UILibIS->Key.Key < 0 ) { return outStr; } // unknown GLFW key is -1
        key =  UILibIS->Key.Key;

        if ( UILibCTX->InputValidateKey && !(*UILibCTX->InputValidateKey)(key) ) { return outStr; }
        
        if(UILibIS->Ctrl){
            if(key==67||key==86||key==65||key==88){}
            else{return outStr;}
        }
        else if(key==10){}
        else if(key<=31){return outStr;}
        else if(key>=320 && key<=336){
            if(key<=329){key-=272;}else if(key==330){key='.';}else if(key==331){key='/';}else if(key==332){key='*';}
            else if(key==333){key='-';}else if(key==334){key='+';}else if(key==335){key=257;}else if(key==336){key='=';}
        }
        else if(key >= 255){if(key==259||key==262||key==263||key==257||key==268||key==269){}else{return outStr;}}
        else if(UILibIS->Shift){
            if(key=='1'){key='!';}if(key=='2'){key='@';}if(key=='3'){key='#';}if(key=='4'){key='$';}if(key=='5'){key='%';}
            if(key=='6'){key='^';}if(key=='7'){key='&';}if(key=='8'){key='*';}if(key=='9'){key='(';}if(key=='0'){key=')';}
            if(key=='`'){key='~';}if(key=='['){key='{';}if(key==']'){key='}';}if(key=='\\'){key='|';}if(key==';'){key=':';}
            if(key=='\''){key='"';}if(key==','){key='<';}if(key=='.'){key='>';}if(key=='/'){key='?';}if(key=='='){key='+';}
            if(key=='-'){key='_';}
        }
        else if(!UILibIS->Caps && !UILibIS->Shift && key > 64 && key < 91){key += 32;} // A-Z convert to lowercase
        
        int keyChange = UILibIS->Key.Change;
        if(keyChange == 1){UILibCTX->HoldDown = 0;}
        auto currentTime = std::chrono::high_resolution_clock::now();
        if(key == UILibCTX->LastKey && keyChange != 1){ // slow down typing to a comfortable rate - avoid repeating letters to quickly
            float timeSpan = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - UILibCTX->LastKeyTime).count();
            float timeDelay = 0.2f;
            if(UILibCTX->HoldDown == 1){timeDelay = 0.03f;}
            if(timeSpan < timeDelay){return outStr;}
            else{UILibCTX->HoldDown = 1;}
        }
        UILibCTX->LastKey = key;
        UILibCTX->LastKeyTime = currentTime;
        // std::cout << "Processing Key: " << key << " " << (char) key << std::endl;

        if(key==259){ // backspace
            if(front.size() > 0){
                if(inEndIndex == -1){front.pop_back();}
                outStr = front + back;
                if(inEndIndex == -1){inInputIndex--;}
                inEndIndex = -1;
            }
            else if(front.size()==0 && inEndIndex != -1){outStr = front + back;inEndIndex = -1;}
        }
        else if(key==257||key==10){ // enter key
            inFocused = false; return outStr;
        }
        else if(UILibIS->Ctrl && (key==65 || key==97)){ // CTRL-A, select all
            inInputIndex=0;inEndIndex=(int)inStr.size();return outStr;
        }
        else if(UILibIS->Ctrl && key==67){ // copy
            if(inEndIndex == -1){return outStr;}
            string copyStr = middle;
            UISetClipboard(copyStr);
        }
        else if(UILibIS->Ctrl && key==88){ // cut 
            if(inEndIndex == -1){return outStr;}
            string copyStr = middle;
            UISetClipboard(copyStr);
            outStr = front + back;
            inEndIndex = -1;
        }
        else if(UILibIS->Ctrl && key==86 ){ // paste
            string clipB = UIGetClipboard();
            if ( UILibCTX->InputValidateKey ) {
                unsigned int clipKey;
                const char * c = clipB.c_str();
                while ( *c != 0 ) { 
                    clipKey = *c;
                    if ( !(*UILibCTX->InputValidateKey)(clipKey) ) { return outStr; }
                    c++;
                }
            }
            front += clipB;
            outStr = front + back;
            inInputIndex+=(int)clipB.size();
            if(inInputIndex > outStr.size()){inInputIndex = (int)outStr.size();}
            inEndIndex = -1;
        }
        else if(key==262){ if(inInputIndex != outStr.size()){inInputIndex++;}}// arrow right
        else if(key==263){ if(inInputIndex > 0){inInputIndex--;}} // arrow left
        else if(key==269){ // end key
            inInputIndex = (int) outStr.size(); inEndIndex = -1;
        }
        else if(key==268){ // home key
            inInputIndex = 0; inEndIndex = -1;
        }
        else {
            // NOTE: Anything that gets here should be process as a CodePoint, so do nothing.
            // front += key;
            // outStr = front + back;
            // inInputIndex++;
            // inEndIndex = -1;
        }
        inChange = 1;

        return outStr;
    }
    void UISelectBetweenDelim(string inStr, int &inInputIndex, int &inEndIndex){
        
        //cout <<" -------------------------- select between delimitors ------------------" << "\n";
        // select between delimitors
        //                   (   {  ,  +  ^  -  *  /  \  <  =  >  &  %  )  }   ,  '  " spc !  :  ;  .
        vector<int> dList = {40,123,44,43,94,45,42,47,92,60,61,62,38,37,41,125,44,39,34,32,33,58,59,46};
        int cStart = 0;
        bool inBlock = false;
        if (count(dList.begin(), dList.end(), (int)inStr[inInputIndex])){
            if(inInputIndex+1>(int)inStr.size()){return;}
            inInputIndex++;
        }
        for(int i=0;i<inStr.size();i++){
            if(i == inInputIndex){inBlock = true;}
            else if (count(dList.begin(), dList.end(), (int)inStr[i])){
                if(inBlock){inInputIndex = cStart;inEndIndex = i;return;}
                cStart = i+1;
                continue;
            }
        }
        if(inBlock){inInputIndex = cStart;inEndIndex = (int)inStr.size();return;}

    }

    int UIGetKeyDirection(){

        // return direction -  [0=right, 1 = left , 2= down, 3 = up]
        // includes time managment to seperate single key presses from fast hold downs.
        int key = GetKeyDown();
        if(key==335){key=257;}  // remap keypad enter key

        if(UILibIS->Ctrl){return -1;}
        else if(key<=31){return -1;}

        int keyChange = UILibIS->Key.Change;
        if(keyChange == 1){UILibCTX->HoldDown = 0;}
        auto currentTime = std::chrono::high_resolution_clock::now();
        if(key == UILibCTX->LastKey && keyChange != 1){ // slow down typing to a comfortable rate - avoid repeating letters to quickly
            float timeSpan = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - UILibCTX->LastKeyTime).count();
            float timeDelay = 0.2f;
            if(UILibCTX->HoldDown == 1){timeDelay = 0.03f;}
            if(timeSpan < timeDelay){return -1;}
            else{UILibCTX->HoldDown = 1;}
            float delta = timeDelay - timeSpan;
        }

        // ENTER = 257, TAB = 258, BACKSPACE = 259, INSERT = 260, DELETE = 261,
        // RIGHT = 262, LEFT = 263, DOWN = 264, UP = 265, PAGE_UP = 266, PAGE_DOWN = 267
        int dir = key - 262;
        if(dir > -6 && dir <= 5){
            UILibCTX->LastKey = key;
            UILibCTX->LastKeyTime = currentTime;
            return dir;
        }
        return -1;

    }

    // --------------------------------------------------------------------- //
    // CLIPBOARD                                                             //
    // --------------------------------------------------------------------- //

    string UIGetClipboard(){
        const char * currentClip = glfwGetClipboardString( (GLFWwindow*) UIGetWindowHandle() );
        if(currentClip == NULL){return "";}
        if(currentClip[0] == '\0'){return "";}
        string sOut; sOut = currentClip;
        return sOut;
    }

    const char * UIGetClipboardChars() {
        return glfwGetClipboardString( (GLFWwindow*) UIGetWindowHandle() );
    }

    void UISetClipboard( const string & inStr){
        glfwSetClipboardString( (GLFWwindow*) UIGetWindowHandle(), inStr.c_str());
    }

    int UIClipboardSize(bool isTextMap){

        string cbs = UIGetClipboard();
        if(!isTextMap){return (int)cbs.size();}
        
        int count=0;
        string str="";char* c = (char *)cbs.data();
        while( *c != 0){
            if(*c == '\r'){
                if(c[1] == '\n'){c+=2;count++;goto loopEnd;} // \r\n newline
            }
            ++c;
            count++;
            loopEnd:;
        }
        
        return count;
    }

    // --------------------------------------------------------------------- //
    // FOCUS                                                                 //
    // --------------------------------------------------------------------- //

    bool UIInputTextInitFocus( string * inHandle, ImRect inRect, int inFont, int inFontColor, int inMode, string inIdentifier, 
        std::function<bool()> * inEnterFunc, std::function<bool(bool)> * inExitFunc, std::function<bool(unsigned int&)> * inValidateFunc ) {
        if ( UILibCTX->InputExitFocus != nullptr ) { 
            if (!(*UILibCTX->InputExitFocus)(false) ) { return false; } 
        }
        if ( UILibCTX->InputHandle != nullptr ) {
            UILibCTX->InputHandle->assign( UILibCTX->InputText.data() );
        }
        UILibCTX->InputBoxMode = inMode;
        UILibCTX->InputBoxRect = inRect;
        UILibCTX->InputText = inHandle->data();  
        UILibCTX->InputStartI = 0;
        UILibCTX->InputEndI = -1;
        UILibCTX->InputDragging = 0;
        UILibCTX->InputChange = 0;
        UILibCTX->InputTextFocus = true;
        UILibCTX->InputTextColor = inFontColor;
        UILibCTX->InputTextFont = inFont;
        UILibCTX->InputHandle = inHandle;
        UILibCTX->InputID = inIdentifier;
        UILibCTX->InputExitFocus = inExitFunc;
        UILibCTX->InputValidateKey = inValidateFunc;
        if ( inEnterFunc ) {
            if ( (*inEnterFunc)() ) {
                UILibCTX->InputEnterFocus = inEnterFunc;
            }
            else {
                UILibCTX->InputHandle = nullptr;
                UILibCTX->InputEnterFocus = nullptr;
                UILibCTX->InputExitFocus = nullptr;
                UILibCTX->InputValidateKey = nullptr;
                UIInputTextExitFocus( true );
                return false;
            }
        }
        
        return true;
    }

    bool UIInputTextExitFocus( bool override ) {
        if ( UILibCTX->InputExitFocus != nullptr ) { 
            if (!(*UILibCTX->InputExitFocus)(override) ) { return false; } 
        }
        if ( UILibCTX->InputHandle != nullptr ) {
            UILibCTX->InputHandle->assign( UILibCTX->InputText.data() );
        }
        UILibCTX->InputBoxMode = 0;
        UILibCTX->InputBoxRect = {};
        UILibCTX->InputText = "";  
        UILibCTX->InputStartI = 0;
        UILibCTX->InputEndI = -1;
        UILibCTX->InputDragging = 0;
        UILibCTX->InputChange = 0;
        UILibCTX->InputTextFocus = false;
        UILibCTX->InputTextColor = 0;
        UILibCTX->InputTextFont = 0;
        UILibCTX->InputHandle = nullptr;
        UILibCTX->InputID = "";
        UILibCTX->InputEnterFocus = nullptr;
        UILibCTX->InputExitFocus = nullptr;
        UILibCTX->InputValidateKey = nullptr;
        return true;
    }

    string * UIInputTextGetFocusHandle() {
        return UILibCTX->InputHandle;
    }


    // ----------------------------------------------------------------------------- //
    //                              UITextMap Functions                              //
    // ----------------------------------------------------------------------------- //

    UITextMap::UITextMap( uint8_t inMode, const string & inText, const UITextFormat & inFormat, const ImRect & inRect ) :
        Mode( inMode ) {
        Set(inText, inRect, inFormat);
        ClickRect = Rect;
        LastBlinkTime = LastShiftTime = LastKeyTime = std::chrono::high_resolution_clock::now();
    }

    bool UITextMap::IsEmpty() {
        for ( const auto & cdata : CList ) {
            if ( cdata.c != 0 ) { return false; }
        }
        return true;
    }

    void UITextMap::Set( const string & inText ) {
        Set( inText, Rect, Format );
    }

    void UITextMap::Set( const string & inText, const ImRect & inRect, const UITextFormat & inFormat ) {
        // Sets the Text in the UITextMap and fills text metadata.
        //   - Existing text is replaced.
        //   - All newlines are converted to Linux style (\n).

        // Reset variables
        Width = 0;
        Height = 0;
        CList.clear();
        SList.clear();
        LList.clear();
        NbLines = 0;

        // Reserve the CList size
        size_t reserveSize = inText.empty() ? 1024 : AlignUp( inText.size(), (size_t)1024 );
        if ( CList.size() < reserveSize ) { CList.reserve( AlignUp( inText.size(), (size_t)1024 ) ); }

        // Set the draw Rect and Format
        Rect = inRect;
        Format = inFormat;
        if ( Format.Syntax.empty() ) { Format.Syntax.push_back({}); }

        // Get the Font
        ImFont * font = GImGui->IO.Fonts->Fonts[ Format.Syntax[0].Index ];
        font->Scale = std::max( Format.Syntax[0].Scale, 0.00001f);

        float baseW = font->GetCharAdvance('a')*font->Scale;    // base character width
        LineH = font->FontSize*font->Scale;
        
        float charW = 0; // current character width
        int charI = 0;   // current displayed character index

        CharData currC;
        LineData currL;

        const unsigned char * c = ( unsigned char *) inText.c_str();        

        // Helper functions
        auto ProcessNL = [&]( bool addChar ){
            if ( addChar ) {
                // Add the newline char
                currC.w = ( ModeIsInput() ? 0 : baseW );
                currC.i = charI;
                currC.c = '\n';
                CList.push_back( currC );
                // Increment the char count
                charI++;
                // Update the currL
                currL.width += currC.w;
                currL.count++;
                
            }
            // Update max line width
            Width = std::max( Width, currL.width );
            // Add to the LList
            LList.push_back( currL );
            // Increment the currC and currL
            currC.x = 0;
            currC.line++;
            currL.width = 0;
            currL.index = (int)CList.size();
            currL.count = 0;
        };
        
        auto ProcessChar = [&]( unsigned char inC ){
            // Add the char
            charW = font->GetCharAdvance((ImWchar)inC)*font->Scale;
            currC.w = charW;
            currC.i = charI;
            currC.c = inC;
            CList.push_back( currC );
            // Increment the char count
            charI++;
            // Update the currC and currL            
            currC.x += charW;
            currL.width += charW;
            currL.count++;
        };
        
        auto ProcessDispSpace = [&]( float w ){
            // Add the non-char display space
            charW = w;
            currC.w = charW;
            currC.i = -1;
            currC.c = 0;
            CList.push_back( currC );
            // Update the currC and currL            
            currC.x += charW;
            currL.width += charW;
            currL.count++;
        };
        
        // Compute Text Map
        if ( !ModeIsInput() && Format.Wrap ) {
            const unsigned char * w = c;    // pointer to start of word
            float wordW = 0;                // word width
            int   wordC = 0;                // number of chars in word
            float availW = Rect.GetWidth(); // available width for line
            auto ProcessWord = [&]() {
                if ( wordC == 0 ) { return; }
                // Wrap around if word can't fit
                if ( currC.x + wordW > availW ) { ProcessNL( false ); } 
                // Add the word
                while ( w != c ) { ProcessChar(*w); w++; }
            };
            while ( *c != 0 ) {
                if ( *c == ' ' ) {
                    ProcessWord();
                    if ( CList.size() > 0 && CList.back().c != ' ' ) { // first space after word gets appended to the end
                        ProcessChar(' '); w = c+1; wordW = 0; wordC = 0;
                    }
                    else {
                        w = c; wordW = font->GetCharAdvance((ImWchar)*c)*font->Scale; wordC = 1;
                    }
                }
                else if ( *c == '\n' ) {
                    ProcessWord();
                    ProcessNL( true );
                    w = c+1; wordW = 0; wordC = 0;
                }
                else if( *c == '\r' ){
                    ProcessWord();
                    ProcessNL( true );
                    w = c+1; wordW = 0; wordC = 0;
                    if( c[1] == '\n' ) { c++; w++; }
                }                
                else {
                    wordW += font->GetCharAdvance((ImWchar)*c)*font->Scale;
                    wordC++;
                }
                c++;
            }
            ProcessWord(); // process the last word
        }
        else {
            while ( *c != 0 ) {
                if ( *c == '\n' ) {
                    ProcessNL( true );
                }
                else if( *c == '\r' ){
                    if( c[1] == '\n' ) { c++; }
                    ProcessNL( true );
                }                
                else {
                    ProcessChar(*c);
                }
                c++;
            }
        }

        // End display space
        ProcessDispSpace( ModeIsInput() ? 0 : baseW);

        // Add the last line
        ProcessNL( false );

        // Vertical Alignment
        NbLines = (int)LList.size();
        Height = NbLines * LineH;
        if ( Format.Align.y < 0 ) { Format.Align.y = 0; }
        else if ( Format.Align.y > 1 ) { Format.Align.y = 1; }

        // Horizontal Alignment
        // Note: Width is computed above in the ProcessNL helper function.
        if ( Format.Align.x < 0 ) { Format.Align.x = 0; }
        else if ( Format.Align.x > 1 ) { Format.Align.x = 1; }

        // Auto-adjust Rect for floating input boxes
        if ( Mode == tm_floating_input ) {
            float adjX = Format.Wrap ? 0 : ( Width - Rect.GetWidth() ) * 0.5f;
            float adjY = ( Height - Rect.GetHeight() ) * 0.5f;
            Rect.Expand( {adjX, adjY} );
            ClickRect.Expand( { std::max(adjX,0.0f), std::max(adjY,0.0f) } );
        }

        // Reset Offset and Scroll variable
        Offset = vec2(0);
        if ( !ModeIsInput() ) { 
            ScrollPerc = {Format.Align.x, Format.Align.y};
            ScrollState = ivec2(0); 
        }

        // Set helper draw variables
        CharW = baseW;
        Change = tm_change_full;
        SelectionData::maxI = (int)CList.size()-1;

        // Reset font scale
        font->Scale = 1.0f; // reset font scale

        #ifdef LOG_UITEXTMAP
        std::cout << "Text: " << GetText() << std::endl;
        std::cout << "Height: " << Height << std::endl;
        std::cout << "LineH: " << LineH << std::endl;
        std::cout << "NbLines: " << NbLines << std::endl;
        #endif

    }

    void UITextMap::RebuildTextMap() {
        // This reconstructs the the entire TextMap from the existing CList characters & formatting.
        // The CList[i].c doesn't change, but the x, w, i, and line variables get rebuilt.
        // If the CList[i].syntax is unspecified, then it carries over the last known syntax.

        // Reset variables
        Width = 0;
        Height = 0;
        LList.clear();
        NbLines = 0;

        // Get the Font
        ImFont * font = GImGui->IO.Fonts->Fonts[ Format.Syntax[0].Index ];
        font->Scale = std::max( Format.Syntax[0].Scale, 0.00001f);

        float baseW = font->GetCharAdvance('a')*font->Scale;    // base character width
        LineH = font->FontSize*font->Scale;                     // base line height
        font->Scale = 1.0f; // reset font scale

        float charW = 0;     // current character width
        int charI = 0;       // current displayed character index
        
        int cI = 0;     // current CList index
        CharData currC;
        LineData currL;

        // Helper functions

        auto SetCharWidth = [&]( unsigned char inC ){
            // Get the char font
            if ( CList[cI].syntax != -1 ) { currC.syntax = CList[cI].syntax; }
            font = GImGui->IO.Fonts->Fonts[ Format.Syntax[currC.syntax].Index ];
            font->Scale = Format.Syntax[currC.syntax].Scale;
            // Set the charW
            charW = font->GetCharAdvance(inC)*font->Scale;
            // Reset the font scale
            font->Scale = 1.0f; // reset font scale 
        };

        auto ProcessNL = [&]( bool addChar ){
            if ( addChar ) {
                // Add the newline char
                if ( ModeIsInput() ) { charW = 0; }
                else { SetCharWidth('a'); }
                currC.w = charW;
                currC.i = charI;
                currC.c = '\n';
                CList[cI] = currC;
                // Increment the char count
                charI++;
                // Update the currL
                currL.width += charW;
                currL.count++;
            }
            // Update max line width
            Width = std::max( Width, currL.width );
            // Add to the LList
            LList.push_back( currL );
            // Increment the currC and currL
            currC.x = 0;
            currC.line++;
            currL.width = 0;
            currL.index = addChar ? cI + 1 : cI;
            currL.count = 0;
        };
        
        auto ProcessChar = [&]( unsigned char inC ){
            // Add to the CList
            SetCharWidth(inC);
            currC.w = charW;
            currC.i = charI;
            currC.c = inC;
            CList[cI] = currC;
            // Increment the char count
            charI++;
            // Update the currC and currL            
            currC.x += charW;
            currL.width += charW;
            currL.count++;
        };
        
        auto ProcessDispSpace = [&]( float w ){
            // Add to the CList
            charW = w;
            currC.w = charW;
            currC.i = -1;
            currC.c = 0;
            CList[cI] = currC;
            // Update the currC and currL            
            currC.x += charW;
            currL.width += charW;
            currL.count++;
        };
        
        // Compute Text Map
        if ( ModeIsInput() && Format.Wrap ) {
            int   wordI = 0;                // word char index
            float wordW = 0;                // word width
            int   wordC = 0;                // number of chars in word
            float availW = Rect.GetWidth(); // available width for line
            int origI = 0;
            auto ProcessWord = [&]() {
                if ( wordC == 0 ) { return; }
                origI = cI; cI = wordI; // ProcessChar uses cI, so we need to make sure we're writing to the correct CList character
                // Wrap around if word can't fit
                if ( currC.x + wordW > availW ) { ProcessNL( false ); } 
                // Add the word
                while ( cI < origI ) { ProcessChar( CList[cI].c ); cI++; }
                cI = origI;
            };
            for ( cI = 0; cI < CList.size()-1; cI++ ) {
                if ( CList[cI].c == ' ' ) {
                    ProcessWord();
                    if ( cI > 0 && CList[cI-1].c != ' ' ) { // first space after word gets appended to the end
                        ProcessChar( ' ' ); wordI = cI+1; wordW = 0; wordC = 0;
                    }
                    else {
                        wordI = cI; SetCharWidth(CList[cI].c); wordW = charW; wordC = 1;
                    }
                }
                else if ( CList[cI].c == '\n' ) {
                    ProcessWord();
                    ProcessNL( true );
                    wordI = cI+1; wordW = 0; wordC = 0;
                }
                else if( CList[cI].c == '\r' ){
                    ProcessWord();
                    ProcessNL( true );
                    wordI = cI+1; wordW = 0; wordC = 0;
                    if( cI+1 < CList.size() && CList[cI+1].c == '\n' ) { cI++; wordI++; }
                }                
                else {
                    SetCharWidth(CList[cI].c);
                    wordW += charW;
                    wordC++;
                }
            }
            ProcessWord(); // process the last word
        }
        else {
            for ( cI = 0; cI < CList.size()-1; cI++ ) {
                if ( CList[cI].c == '\n' ) {
                    ProcessNL( true );
                }
                else if( CList[cI].c == '\r' ){
                    if( cI+1 < CList.size() && CList[cI+1].c == '\n' ) { cI++; }
                    ProcessNL( true );
                }                
                else {
                    ProcessChar(CList[cI].c);
                }
            }
        }

        // End display space
        ProcessDispSpace( ModeIsInput() ? 0 : baseW );

        // Add the last line
        ProcessNL( false );

        // Vertical Alignment
        NbLines = (int)LList.size();
        Height = NbLines * LineH;
        if ( Format.Align.y < 0 ) { Format.Align.y = 0; }
        else if ( Format.Align.y > 1 ) { Format.Align.y = 1; }

        // Horizontal Alignment
        // Note: Width is computed above in the ProcessNL helper function.
        if ( Format.Align.x < 0 ) { Format.Align.x = 0; }
        else if ( Format.Align.x > 1 ) { Format.Align.x = 1; }

        // Auto-adjust Rect for floating input boxes
        if ( Mode == tm_floating_input ) {
            float adjX = Format.Wrap ? 0 : ( Width - Rect.GetWidth() ) * 0.5f;
            float adjY = ( Height - Rect.GetHeight() ) * 0.5f;
            Rect.Expand( {adjX, adjY} );
            ClickRect.Expand( { std::max(adjX,0.0f), std::max(adjY,0.0f) } );
        }

        // Set helper draw variables
        CharW = baseW;
        SelectionData::maxI = (int)CList.size()-1;
        ShiftOffsetToChar( !SList.empty() ? SList.back().GetCaret() : 0 );

        #ifdef LOG_UITEXTMAP
        std::cout << "Text: " << GetText() << std::endl;
        std::cout << "Height: " << Height << std::endl;
        std::cout << "LineH: " << LineH << std::endl;
        std::cout << "NbLines: " << NbLines << std::endl;
        #endif

    }

    void UITextMap::UpdateRect( const ImRect & inRect ) {
        if ( Mode == tm_floating_input ) {
            if ( !Format.Wrap || std::abs(inRect.GetWidth()-Rect.GetWidth())<0.001f ) {
                Rect.Translate( inRect.GetCenter() - Rect.GetCenter() );
                return;
            }
        }
        else if ( std::abs(inRect.GetWidth()-Rect.GetWidth())<0.001f && std::abs(inRect.GetHeight()-Rect.GetHeight())<0.001f ) { 
            Rect = inRect;
            return;
        }
        Rect = inRect;
        if ( !Format.Wrap ) { SyncOffsetX(); SyncOffsetY(); return; }
        RebuildTextMap();
    }

    void UITextMap::UpdateFormat( const UITextFormat & inFormat ) {
        bool NeedUpdate = false;
        // Update when there are Font Index or Scale changes. Font color changes do not require 
        // the text map to be rebuilt, since spacing would remain the same.
        if ( Format.Syntax.size() != inFormat.Syntax.size() ) { NeedUpdate = true; }
        else {
            for ( int i = 0; i < Format.Syntax.size(); i++ ) {
                if      ( Format.Syntax[i].Index != inFormat.Syntax[i].Index ) { NeedUpdate = true; break; }
                else if ( Format.Syntax[i].Scale != inFormat.Syntax[i].Scale ) { NeedUpdate = true; break; }
            }
        }
        if ( !NeedUpdate ) {
            if ( Format.Align != inFormat.Align ) { SyncOffsetX(); SyncOffsetY(); }
        }
        Format = inFormat;
        if ( Format.Syntax.empty() ) { Format.Syntax.push_back( {} ); }
        if ( NeedUpdate ) {
            RebuildTextMap();
            NeedUpdate = false;
        }
    }

    void UITextMap::Draw( ImRect & inRect, const ImRect & inClickRect ) {

        // Reset change
        Change = tm_change_none;

        // Set the selection data max so we don't get OOB selection indices
        SelectionData::maxI = (int)CList.size()-1;

        // Update draw Rect and ClickRect
        UpdateRect(inRect);
        ClickRect = inClickRect;
        TextRect = GetTextBoundingBox();

        // Process Key Input
        KeyInput();

        // Process Mouse Input
        MouseInput();

        // Get the Fonts
        vector<ImFont*> Fonts( Format.Syntax.size() );
        for ( int i = 0; i < Format.Syntax.size(); i++ ) {
            Fonts[i] = GImGui->IO.Fonts->Fonts[ Format.Syntax[i].Index ];
            Fonts[i]->Scale = std::max( Format.Syntax[i].Scale, 0.00001f);
        }       

        
        // Clip draw to the Rect
        LineStartI = std::max( 0, (int) (( Rect.Min.y - TextRect.Min.y ) / LineH) );
        LineEndI = std::min( NbLines, (int) (( Rect.Max.y - TextRect.Min.y ) / LineH + 1 ) );
        if ( Mode == tm_floating_input && Focus ) {
            ImGui::PushClipRect(Rect.Min, { Rect.Max.x+CharW * 0.2f, Rect.Max.y }, true);
        }
        else {
            ImGui::PushClipRect(Rect.Min, Rect.Max, true);
        }

        DrawSelection();

        float lPosX, lPosY, cPosX;
        float threshold = CharW*2;
        int cIndex = 0;
        ImU32 clr = 0; 
        for ( int i = LineStartI; i < LineEndI; i++ ) {
            lPosX = LinePosX( i );
            lPosY = LinePosY( i );
            for ( int j = 0; j < LList[i].count; j++ ) {
                cIndex = LList[i].index + j;
                if ( CList[cIndex].c == 0 ) { continue; }
                cPosX = lPosX + CList[cIndex].x;
                if ( cPosX > Rect.Max.x + threshold || cPosX < Rect.Min.x - threshold ) {
                    continue;
                }
                clr = UILib::ClWithAlpha( Format.Syntax[CList[cIndex].syntax].Color, 1 );
                UIRenderChar( (ImWchar) CList[cIndex].c, { cPosX, lPosY }, clr, Fonts[CList[cIndex].syntax] );
                if ( Format.ULine ) {
                    const float lineY = lPosY + (LineH * .94f);
                    UIAddGlobalLineAlpha(cPosX, lineY, cPosX+CharWidth(cIndex), lineY, Format.Syntax[CList[cIndex].syntax].Color, LineH * 0.059f, 1.0f);
                }
            } 
        }

        DrawCaret();

        DrawScroll();

        ImGui::PopClipRect();

        // Reset font scales
        for ( int i = 0; i < Format.Syntax.size(); i++ ) {
            Fonts[i]->Scale = 1.0f;
        }

        // Update the inRect handle for floating inputs
        if ( Mode == tm_floating_input ) {
            inRect = Rect;
        }

    #ifndef NDEBUG
        // Debug
        if ( DebugOn ) { DrawDebug(); }
    #endif

    }

    void UITextMap::SetCarets( const vector<int> & inCaretPos ) {
        SList.clear();
        for ( int i = 0; i < inCaretPos.size(); i++ ) {
            SList.push_back( inCaretPos[i] );
        }
        if ( SList.size() > 0 ) { ShiftOffsetToChar( SList.back().GetCaret() ); }
        ResetTime(LastBlinkTime);
    }

    vector<int> UITextMap::RemoveSelectionFromText() {

        // Track list of new caret positions
        vector<int> caretPos = {};

        // Remove the selections
        for ( int i = 0; i < SList.size(); i++ ) {
            if ( SList[i].S == SList[i].E ) { caretPos.push_back(SList[i].S); continue; }
            else {
                // Adjust carets & selections occuring after the current start index
                const int count = SList[i].E - SList[i].S;
                const int index = SList[i].S;
                for ( int j = i+1; j < SList.size(); j++ ) {
                    if ( SList[j].D > index ) { SList[j].D = std::max( index, SList[j].D-count ); }
                    if ( SList[j].S > index ) { SList[j].S = std::max( index, SList[j].S-count ); }
                    if ( SList[j].E > index ) { SList[j].E = std::max( index, SList[j].E-count ); }
                }
                for ( int & cp : caretPos ) {
                    if ( cp > index ) { cp = std::max(index, cp-count); }
                }
                // Add to caret positions
                caretPos.push_back(index);
                // Erase the selection
                CList.erase( CList.begin()+SList[i].S, CList.begin()+SList[i].E );
            }
        }

        // Dedup the caret positions keeping selection order
        if ( caretPos.size() > 1 ) {
            vector<int> removeI = {};
            for ( int i = 0; i < caretPos.size()-1; i++ ) {
                for ( int j = i+1; j < caretPos.size(); j++ ) {
                    if ( caretPos[i] == caretPos[j] ) { removeI.push_back(i); break; }
                }
            }
            for ( int i = (int)removeI.size()-1; i >= 0; i-- ) {
                caretPos.erase(caretPos.begin()+i);
            }
        }

        // Indicate change
        Change = tm_change_insert;

        return std::move(caretPos);

    }

    void UITextMap::InsertOrReplace( const string & inInsertS ) {
        
        // Remove selected text and get new caret positions
        vector<int> caretPos = RemoveSelectionFromText();

        // Prepare the new char data
        vector<CharData> cd(inInsertS.size());
        for ( int i = 0; i < cd.size(); i++ ) { cd[i].c = inInsertS[i]; }

        // Reserve the CList size
        CList.reserve( AlignUp( CList.size() + inInsertS.size() * SList.size(), (size_t) 1024 ) );

        // Insert the new text
        for ( int i = 0; i < caretPos.size(); i++ ) {
            // Insert into the CList
            int index = std::min( (int) CList.size()-1, caretPos[i] );
            CList.insert( CList.begin()+index, cd.begin(), cd.end() );
            // Adjust caret positions occurring after the insert index
            for ( int j = 0; j < caretPos.size(); j++ ) {
                if ( caretPos[j] > index ) { caretPos[j] += (int)inInsertS.size(); }
            }
            // Adjust the current caret position
            caretPos[i] = index + (int)inInsertS.size();
        }

        // Rebuild the TextMap
        RebuildTextMap();

        // Rebuild the selection list from the caret positions
        SetCarets( caretPos );

        // Indicate change
        Change = tm_change_insert;

    }

    void UITextMap::TabIndent( bool forward ) {
        vector<int> lines = GetSelectedLines();
        vector<int> lineI = {}, startI = {}, endI = {};
        // Get the list of lines and their start/end indices to modify.
        // Skips wordwrap continuation lines.
        for ( int line : lines ) {
            int sI = LList[line].index;
            if ( sI == 0 || CList[sI-1].c == '\n' ) {
                lineI.push_back( line );
                startI.push_back( sI );
                endI.push_back( sI + LList[line].count );
            }
        }
        if ( lineI.empty() ) { return; }
        // Increment the tabs at the start of the line.
        // Spaces are converted to tabs (4 Spaces = 1 Tab).
        for ( int i = 0; i < lineI.size(); i++ ) {
            const int sI = startI[i];
            const int eI = endI[i];
            // Get the number of tabs or spaces at the beginning of the line
            int nbTab = 0, nbSpace = 0;
            for ( int i = sI; i < eI; i++ ) {
                if      ( CList[i].c == '\t' ) { nbTab++; }
                else if ( CList[i].c == ' ' ) { nbSpace++; }
                else { break; }
            }
            int newNbTab = forward ? AlignDown( nbTab*4+nbSpace, 4) / 4 + 1
                                   : AlignUp( nbTab*4+nbSpace, 4) / 4 - 1;
            if ( newNbTab < 0 ) { newNbTab = 0; }
            // Erase the existing line start tabs & spaces
            CList.erase( CList.begin() + sI, CList.begin() + sI + nbTab + nbSpace );
            // Insert the new tabs
            if ( newNbTab > 0 ) {
                vector<CharData> newC( newNbTab, {'\t'} );
                CList.insert( CList.begin() + sI, newC.begin(), newC.end() );
            }
            // Adjust the start/end indices for subsequent lines
            int adj = newNbTab - nbTab - nbSpace;
            if ( adj == 0 ) { continue; }
            for ( int j = i+1; j < lineI.size(); j++ ) {
                startI[j] += adj; endI[j] += adj;
            }
            // Adjust the selection indices
            int adjI = ( forward ) ? sI : sI+adj;
            for ( auto & sel : SList ) {
                if ( sel.E <= adjI ) { continue; }
                if ( sel.S <= adjI ) {
                    if ( sel.D > sel.S ) { sel.D += adj; if ( sel.D < 0 ) { sel.D = 0; } else if ( sel.D > (int)CList.size() ) { sel.D = (int)CList.size(); } }
                    if ( sel.E > sel.S ) { sel.E += adj; if ( sel.E < 0 ) { sel.E = 0; } else if ( sel.E > (int)CList.size() ) { sel.E = (int)CList.size(); } }
                }
                else {
                    sel.S += adj; sel.E += adj; sel.D += adj;
                    if ( sel.S < 0 ) { sel.S = 0; } else if ( sel.S >= (int)CList.size() ) { sel.S = (int)CList.size()-1; }
                    if ( sel.E < 0 ) { sel.E = 0; } else if ( sel.E > (int)CList.size() ) { sel.E = (int)CList.size(); }
                    if ( sel.D < 0 ) { sel.D = 0; } else if ( sel.D > (int)CList.size() ) { sel.D = (int)CList.size(); }
                }
            }
        }
        RebuildTextMap();

        // Indicate change
        Change = tm_change_insert;
    }

    void UITextMap::KeyInput() {
        if ( !Focus ) { return; }
        
        string InsertS = "";

        // Use CodePoint if it exists
        unsigned int key = UILibIS->Key.CodePoint;
        if ( key != 0 ) {
            if (!ValidateKey(key)){return;}
            InsertS = UILibIS->Key.CodePoint;
            #ifdef LOG_UITEXTMAP
            std::cout << "Processing Codepoint: " << UILibIS->Key.CodePoint << std::endl;
            #endif
            InsertOrReplace( InsertS );
            UILibIS->Key.Reset = true;  // Signal to clear the text buffer
            return;
        }

        // Otherwise, use the current key down
        key =  UILibIS->Key.Key;
        string front = "";string middle = "";string back = "";
        
        if ( !ValidateKey(key) ) { return; }

        if(UILibIS->Ctrl){if(key==67||key==86||key==65||key==88||key==47||key==91||key==93||key==96){} else{return;}} // Ctrl Hotkeys
        else if(key==10 || key==335 ){key=257;} // Line feed and keypad Enter (remap to Enter key)
        else if(key<=31){return;}  // Non-printable device control keys
        else if(key==258){key=9;}  // Tab key (remap to '\t')
        else if(key==256){ if ( AutoFocus ) { SetFocus(false); } return; } // Escape key to exit focus
        else if(key >= 255){if(key==259||key==261||key==262||key==263||key==264||key==265||key==257||key==268||key==269){}else{return;}} // modification keys (i.e., backspace, del, enter, etc.)
        
        // When key is held down, slow down typing to a comfortable rate - avoid repeating letters to quickly
        auto currentTime = std::chrono::high_resolution_clock::now();
        if ( UILibIS->Key.Change == 1 ) { HoldDown = false; }
        if ( key == LastKey && UILibIS->Key.Change != 1 ) {
            float timeSpan = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - LastKeyTime).count();
            float timeDelay = 0.2f;
            if ( HoldDown ) { timeDelay = 0.03f; }
            if ( timeSpan < timeDelay ) {
                #ifdef LOG_UITEXTMAP
                cout << " less than time Delay, timeSpan: " << timeSpan << "\n";
                #endif
                return;
            }
            else { HoldDown = true; }
        }
        LastKey = key; LastKeyTime = currentTime;

        #ifdef LOG_UITEXTMAP
        std::cout << "Processing Key: " << key;
        if ( UILibIS->Alt ) { std::cout << ", ALT"; }
        if ( UILibIS->Ctrl ) { std::cout << ", CTRL"; }
        if ( UILibIS->Shift ) { std::cout << ", SHIFT"; }
        std::cout << std::endl;
        #endif
               
        // ======================= key input for multi-line selection =======================
        if ( (key==9 && UILibIS->Shift) || (key==91 && UILibIS->Ctrl) ) { // tab back ( SHIFT-Tab or CTRL-[ )
            if ( !ModeIsInput() ) { TabIndent(false);}
            return;
        }
        if ( key==9 || (key==93 && UILibIS->Ctrl) ) { // tab forward ( Tab or CTRL-] )
            bool TabForward = false;
            if ( ModeIsInput() ) {
                if ( key==93 && UILibIS->Ctrl ) { return; } // CTRL-] does nothing with basic input
            }
            else { // Use TabForward in editor mode on whole line or multi-line selection
                for ( auto & s : SList ) {
                    if ( s.S == s.E ) { continue; }
                    if (   ( CList[s.S].line != CList[s.E].line )
                        || ( s.S == LList[s.S].index && s.E == LList[s.S].index + LList[s.S].count ) ) { 
                        TabForward = true; break; 
                    }
                }
            }
            if ( TabForward ) { // tab forward each line (inserting tab)
                TabIndent(true);
            }
            else {              // insert a tab at each caret
                InsertS = '\t';
                InsertOrReplace( InsertS );
            }
            return;
        }
        if ( key==47 && UILibIS->Ctrl ) { // multi-line comment
            if ( Mode == tm_code_editor ) { /* TO DO */ }
            return;
        }
        
        // =================== regular selection independent key input ======================
        if ( key==261 ){ // delete
            for ( auto & s : SList ) { if ( s.S == s.E ) { s.E = std::min( ++s.E, (int)CList.size()-1 ); } }
            vector<int> cp = RemoveSelectionFromText(); RebuildTextMap(); SetCarets(cp);
            return;
        }
        if ( key==259 ) { // backspace
            for ( auto & s : SList ) { if ( s.S == s.E ) { s.S = std::max( --s.S, 0 ); } }
            vector<int> cp = RemoveSelectionFromText(); RebuildTextMap(); SetCarets(cp);
            return; 
        }
        if ( key==269 ) { // end key
            for ( auto & s : SList ) {
                const int caretI = s.GetCaret();
                int lineI = CList[caretI].line;
                int endI = LList[lineI].index + LList[lineI].count - 1;
                if ( CharVisible(caretI) ) { ShiftOffsetToChar(endI); }
                if ( UILibIS->Shift ) { s.UpdateSel( endI ); }
                else{ s.SetCaret(endI); }
            }
            return;
        }
        if ( key==268 ) { // home key
            for ( auto & s : SList ) {
                const int caretI = s.GetCaret();
                int lineI = CList[caretI].line;
                int startI = LList[lineI].index;
                if ( CharVisible(caretI) ) { ShiftOffsetToChar(startI); }
                if ( UILibIS->Shift ) { s.UpdateSel( startI ); }
                else{ s.SetCaret(startI); }
            }
            return;
        }
        if ( key==257 ) { // enter key
            if ( Mode == tm_input ) {
                if ( AutoFocus ) { SetFocus(false);}
            }
            else {
                InsertS = "\n";
                InsertOrReplace( InsertS );
            }
            return;
        }
        if ( UILibIS->Ctrl && key==65 ) {  // select all ( CTRL-A )
            SList = { { 0, (int) CList.size()-1 } };
            ResetTime(LastBlinkTime);
            return;
        }
        if ( key==262 ) { // arrow right
            int caretI = -1;
            if ( UILibIS->Shift ) {
                for ( auto & sel : SList ) { caretI = sel.GetCaret()+1; sel.UpdateSel( caretI ); }
            }
            else {
                for ( auto & sel : SList ) { 
                    caretI = sel.E; if ( sel.E == sel.S ) { caretI++; };
                    sel.SetCaret(caretI); 
                }
            }
            if ( caretI != -1 ) { ShiftOffsetToChar( caretI ); }
            ResetTime(LastBlinkTime);
            return;
        }
        if ( key==263 ) { // arrow left
            int caretI = -1;
            if ( UILibIS->Shift ) {
                for ( auto & sel : SList ) { caretI = sel.GetCaret()-1; sel.UpdateSel( caretI ); }
            }
            else {
                for ( auto & sel : SList ) { 
                    caretI = sel.S; if ( sel.E == sel.S ) { caretI--; };
                    sel.SetCaret(caretI);
                }
            }
            if ( caretI != -1 ) { ShiftOffsetToChar( caretI ); }
            ResetTime(LastBlinkTime);
            return;
        }
        if ( key==264 ) { // arrow down
            int charI = -1;
            if ( UILibIS->Shift ) {
                for ( auto & sel : SList ) { 
                    int caretI = sel.GetCaret();
                    int lineI = CList[caretI].line + 1;
                    charI = lineI >= NbLines ? (int)CList.size()-1 : GetActiveChar( { CharPosX(caretI), LinePosY(lineI)+LineH*0.5f } );
                    sel.UpdateSel(charI);
                }
            }
            else {
                for ( auto & sel : SList ) { 
                    int caretI = sel.GetCaret();
                    int lineI = CList[caretI].line + 1;
                    charI = lineI >= NbLines ? (int)CList.size()-1 : GetActiveChar( { CharPosX(caretI), LinePosY(lineI)+LineH*0.5f } );
                    sel.SetCaret(charI);
                }
            }
            if ( charI != -1 ) { ShiftOffsetToChar( charI ); }
            ResetTime(LastBlinkTime);
            return;
        }
        if ( key==265 ) { // arrow up
            int charI = -1;
            if ( UILibIS->Shift ) {
                for ( auto & sel : SList ) { 
                    int caretI = sel.GetCaret();
                    int lineI = CList[caretI].line - 1;
                    charI = lineI < 0 ? 0 : GetActiveChar( { CharPosX(caretI), LinePosY(lineI)+LineH*0.5f } );
                    sel.UpdateSel(charI);
                }
            }
            else {
                for ( auto & sel : SList ) { 
                    int caretI = sel.GetCaret();
                    int lineI = CList[caretI].line - 1;
                    charI = lineI < 0 ? 0 : GetActiveChar( { CharPosX(caretI), LinePosY(lineI)+LineH*0.5f } );
                    sel.SetCaret(charI);
                }
            }
            if ( charI != -1 ) { ShiftOffsetToChar( charI ); }
            ResetTime(LastBlinkTime);
            return;
        }
        if ( UILibIS->Ctrl && ( key==67 || key==88 ) ) { // copy and cut ( CTRL-C, CTRL-X )
            // multiple selections will be seperated by newline character
            if ( SList.empty() ) { return; }
            // Sort the selections
            auto sList = SList;
            std::sort( sList.begin(), sList.end(), 
                [](const SelectionData & a, const SelectionData & b ) {
                    if ( a.S < b.S ) { return true; }
                    else if ( a.S > b.S ) { return false; }
                    return a.E < b.E;
                }
            );
            // Get the unique selection ranges
            vector<int> startI = {};
            vector<int> endI = {};
            for ( const auto & sel : sList ) {
                if ( startI.empty() ) { startI.push_back( sel.S ); endI.push_back( sel.E ); }
                else if ( sel.S < endI.back() ) { 
                    if ( sel.E > endI.back() ) { endI.back() = sel.E; }
                }
                else { startI.push_back( sel.S ); endI.push_back( sel.E ); }
            }
            if ( startI.empty() ) { return; }
            #ifdef _WIN32
            string NL = "\r\n";
            #else
            string NL = "\n";
            #endif
            string copyStr = "";
            for ( int i = 0; i < startI.size(); i++ ) {
                if ( startI[i] == endI[i] ) { continue; }
                if ( i > 0 ) { copyStr += NL; }
                for ( int cI = startI[i]; cI < endI[i]; cI++ ) {
                    if ( CList[cI].c == 0 ) { continue; }
                    if ( CList[cI].c == '\n' ) { copyStr += NL; }
                    else { copyStr += CList[cI].c; }
                }
            }
            UISetClipboard( copyStr );
            // Remove selection if cut
            if ( key==88 ) {
                vector<int> cp = RemoveSelectionFromText(); RebuildTextMap(); SetCarets(cp);
            }
            return;
        }
        if ( UILibIS->Ctrl && key==86 ) { // paste ( CTRL-V )
        #ifdef _WIN32
            InsertS.clear();
            const char * c = UIGetClipboardChars();
            while ( *c != 0 ) {
                if( *c == '\r' ){
                    if( c[1] == '\n' ) { c++; }
                }                
                InsertS += *c; c++;
            }
        #else
            InsertS = UIGetClipboard();
        #endif            
            InsertOrReplace( InsertS );
            return;
        }
    #ifndef NDEBUG
        if ( UILibIS->Ctrl && key==96 ) { // paste ( CTRL-` )
            DebugOn = !DebugOn;
        }
    #endif

    }

    void UITextMap::MouseInput() {

        vec2 gp = GetGlobalMousePos();
        bool inClickRect = VecInBB(gp,ClickRect) && ( ScrollState.x == 0 && ScrollState.y == 0 );

        if ( AutoFocus && DragStart() && !VecInBB(gp,ClickRect) ) { SetFocus(false); return; }

        // ====================================== DOUBLE CLICK ==========================================
        if ( UILibIS->DoubleClick ) { 
            if ( Mute || !inClickRect ) { return; }
            #ifdef LOG_UITEXTMAP
            std::cout << "DOUBLE CLICK" << std::endl;
            #endif

            // select between delimiters
            auto IsDelimiter = [](char inC) {
                if ( inC <= 35 
                    || ( inC >= 37 && inC <= 47 )
                    || ( inC >= 58 && inC <= 63 )
                    || ( inC >= 91 && inC <= 94 )
                    || inC == 123 || inC == 125
                ) { return true; }
                return false;
            };
            
            int clickI = GetActiveChar(gp);
            int lineI = CList[clickI].line;
            int startI = LList[lineI].index;
            int endI = startI + LList[lineI].count;

            int rIndex = clickI;
            int lIndex = clickI;

            if ( CList[clickI].c == ' ' || CList[clickI].c == '\t' ) { // crawl space and tabs
                for ( int i = clickI; i < endI; i++ ) {
                    if ( CList[i].c != ' ' && CList[i].c != '\t' ) { break; } rIndex = i+1;
                }
                for ( int i = clickI; i-->startI; ) {
                    if ( CList[i].c != ' ' && CList[i].c != '\t' ) { break; } lIndex = i;
                }
            }
            else {
                for ( int i = clickI; i < endI; i++ ) {
                    if ( IsDelimiter( CList[i].c ) ) { break; } rIndex = i+1;
                }
                for ( int i = clickI; i-->startI; ) {
                    if ( IsDelimiter( CList[i].c ) ) { break; } lIndex = i;
                }
            }
            
            SList = { {lIndex, rIndex} };
            LDrag = m_drag;
            if ( AutoFocus ) { SetFocus(true); } else { ResetTime(LastBlinkTime); }
            UILibIS->DoubleClick = false;
            ShiftOffsetToChar( rIndex );
            return;
        }
        
        // ===================================== STOP DRAGGING ==========================================
        if( !AnyMouseDown() ) { 
            #ifdef LOG_UITEXTMAP
            if ( LDrag != m_none || MDrag != m_none ) {
                std::cout << "MOUSE UP END DRAGGING" << std::endl;
                for ( int i = 0; i < SList.size(); i++ ) {
                    std::cout << "Sel " << i << " D: " << SList[i].D << ", S: " << SList[i].S << ", E: " << SList[i].E << std::endl;
                }
            }
            #endif
            LDrag = m_none; MDrag = m_none;
            return; 
        }

        if ( Mute ) { return; }

        // ==================================== START MULTI DRAG ========================================
        if ( inClickRect && MiddleDragStart() ) {
            #ifdef LOG_UITEXTMAP
            std::cout << "START MULTI DRAG" << std::endl;
            #endif
            int clickI = GetActiveChar(gp);
            SList = { {clickI} };
            DragPoint = gp;
            MDrag = m_drag;
            if ( AutoFocus ) { SetFocus(true); } else { ResetTime(LastBlinkTime); ResetTime(LastShiftTime); }
        }
        // ================================== SINGLE CARET DRAGGING ====================================
        else if ( LDrag == m_drag ) {
            #ifdef LOG_UITEXTMAP
            std::cout << "SINGLE CARET DRAGGING" << std::endl;
            #endif
            if ( SList.empty() ) { LDrag = m_none; return; }
            SelectionData & sl = SList.back();
            int clickI = GetActiveChar(gp);
            sl.UpdateSel(clickI);
            // Reset Time Points
            auto currentTime = std::chrono::high_resolution_clock::now();
            float timeSpan = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - LastShiftTime).count();
            if ( timeSpan > 0.1 ) { LastShiftTime = currentTime; ShiftOffsetToChar( clickI ); }
            ResetTime(LastBlinkTime);
        }
        // ================================ MULTI CARET DRAGGING ====================================
        else if ( MDrag == m_drag ) {
            #ifdef LOG_UITEXTMAP
            std::cout << "MULTI CARET DRAGGING" << std::endl;
            #endif
            // Clear the SList
            SList.clear();
            // Get the start and end line indexes
            int yiStart = GetActiveLine(DragPoint.y);
            int yiEnd = GetActiveLine(gp.y);
            if ( yiStart > yiEnd ) { std::swap( yiStart, yiEnd ); }
            // Figure out if it is a line drag or a box drag - behavior is different
            // Line drag will pickup leftmost chars like empty lines on the left, box drag only gets whats in the box
            float dragW = abs( gp.x - DragPoint.x );
            int dragType = 0; // 0 = line drag, 1 = box drag
            if ( dragW > CharW * 0.5f ) { dragType = 1; } 
            // Switch to line drag if we are on the right side of all text
            if ( dragType == 1 && DragPoint.x > TextRect.Max.x ) {
                bool onRight = true;
                for ( int i = yiStart; i <= yiEnd; i++ ) {
                    if ( gp.x < LinePosX(i) + LList[i].width - CharW*0.5f ) { onRight = false; break; }
                }
                if ( onRight ) { dragType = 0; }
            }
            // Fill the SList
            if ( dragType == 0 ) {  // line drag -> start at left side and walk to right for closest match on x
                for ( int i = yiStart; i <= yiEnd; i++ ) {
                    int startI = GetActiveChar( { DragPoint.x, TextRect.Min.y + i*LineH } );
                    SList.push_back( {startI} );
                }
            }
            else { // box drag -> only kick off a selection if there's text within the drag box
                for ( int i = yiStart; i <= yiEnd; i++ ) {
                    int startI = GetActiveChar( { DragPoint.x, TextRect.Min.y + i*LineH } );
                    SList.push_back( {startI} );
                    SelectionData & sl = SList.back();
                    sl.UpdateSel( GetActiveChar( { gp.x, TextRect.Min.y + i*LineH } ) );
				}
            }
            // Reset Time Points
            auto currentTime = std::chrono::high_resolution_clock::now();
            float timeSpan = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - LastShiftTime).count();
            if ( timeSpan > 0.1 ) { 
                LastShiftTime = currentTime;
                if ( SList.size() > 0 ) { ShiftOffsetToChar( SList.back().GetCaret() ); }
            }
            ResetTime(LastBlinkTime);
        }
        // ====================================== ADD MULTI CARET ======================================
        else if( inClickRect && DragStart() && (UILibIS->Ctrl || UILibIS->Alt) ) {
            #ifdef LOG_UITEXTMAP
            std::cout << "ADD MULTI CLICK CARET" << std::endl;
            #endif
            int clickI = GetActiveChar(gp);
            SList.push_back( {clickI} );
            LDrag = m_drag;
            if ( AutoFocus ) { SetFocus(true); } else { ResetTime(LastBlinkTime); ResetTime(LastShiftTime); }
        }
        // ====================================== ADD SINGLE CARET ======================================
        else if ( inClickRect && DragStart() ) {
            #ifdef LOG_UITEXTMAP
            std::cout << "ADD SINGLE CARET" << std::endl;
            #endif
            int clickI = GetActiveChar(gp);
            SList = { {clickI} };
            LDrag = m_drag;
            if ( AutoFocus ) { SetFocus(true); } else { ResetTime(LastBlinkTime); ResetTime(LastShiftTime); }
        }

    }

    void UITextMap::DrawCaret() {
        if ( !Focus ) { return; }

        if ( SList.empty() ) { return; }

        bool blink = false;
        if ( LDrag != m_none || MDrag != m_none ) { blink = true; }
        else {
            auto currentTime = std::chrono::high_resolution_clock::now();
            float timeSpan = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - LastBlinkTime).count();
            int timeCounts = (int) floor( timeSpan / 0.6f );
            if (timeCounts % 2 == 0) { blink = true; }
        }

        if ( !blink ) { return; }

        int clr = 0x000000;
        ImRect caretBB;

        float caretW = CharW * 0.2f;

        for( int i = 0; i < SList.size(); i++ ) {
            if ( SList[i].S < 0 || SList[i].E < 0) { SList[i].SetCaret(0); }
            if ( !SList[i].RSide ) {
                caretBB = GetBB( CharPosX(SList[i].S), CharPosY(SList[i].S), caretW, LineH);
            }
            else {
                caretBB = GetBB( CharPosX(SList[i].E), CharPosY(SList[i].E), caretW, LineH);
            }
            UIDrawBB(caretBB, clr);
        }

    }

    void UITextMap::DrawSelection() {
        
        for ( int i = 0; i < SList.size(); i++ ) {
            if ( SList[i].S != SList[i].E ) {
                float startX = CList[SList[i].S].x;
                float selWidth = 0;
                int lastLine = CList[ SList[i].S ].line;
                ImRect selBB;
                for ( int j = SList[i].S+1; j < SList[i].E; j++ ) {
                    if ( CList[j].line != lastLine ) {
                        selWidth = LList[lastLine].width - startX;
                        selBB = GetBB( LinePosX(lastLine)+startX, LinePosY(lastLine), selWidth, LineH );
                        UIDrawAlphaBB( selBB, Format.HighlightClr, 1.0f );
                        startX = CList[j].x;
                        lastLine = CList[j].line;
                    }
                }
                if ( CList[ SList[i].E ].line != lastLine ) {
                    selWidth = LList[lastLine].width - startX;
                }
                else {
                    selWidth = CList[SList[i].E].x - startX;
                }
                selBB = GetBB( LinePosX(lastLine)+startX, LinePosY(lastLine), selWidth, LineH );
                UIDrawAlphaBB( selBB, Format.HighlightClr, 1.0f );
            }
        }
    }

    void UITextMap::DrawScroll() {
        if ( ModeIsInput() ) { return; }

        bool ScrollOK = VecInBB( GetGlobalMousePos(), Rect );

        if ( !ScrollOK && !(ScrollDrag.x || ScrollDrag.y) ) { return; }
        if ( !Scroller ) { Scroller = &UILibCTX->DefaultScroller; }

        float ViewWidth = Rect.GetWidth();
        float ViewHeight = Rect.GetHeight();
        bool XScrollOn = !Format.Wrap && Width > ViewWidth;
        bool YScrollOn = Height > ViewHeight;

        if(YScrollOn){
            float scHeight = Rect.GetHeight() - ( XScrollOn ? ScrollSize : 0 );
            UIAddScrollerV(Rect.Max.x - ScrollSize, Rect.Min.y, ScrollSize, scHeight, (float)Height, &ScrollPerc.y, &ScrollDrag.y, *Scroller, &ScrollState.y);
        }
        if(XScrollOn){
            float scWidth = Rect.GetWidth() - ( YScrollOn ? ScrollSize : 0 );
            UIAddScrollerH(Rect.Min.x, Rect.Max.y - ScrollSize, scWidth, ScrollSize, (float)Width, &ScrollPerc.x, &ScrollDrag.x, *Scroller, &ScrollState.x);
        }

        Offset.x = (float) ( ( ViewWidth - Width ) * ( ScrollPerc.x - Format.Align.x ) );
        Offset.y = (float) ( ( ViewHeight - Height ) * ( ScrollPerc.y - Format.Align.y ) );

        if ( ScrollOK && UILibIS->ScrollY != 0 ) {
            if ( !UILibIS->Shift && YScrollOn ) {
                ShiftOffsetY(-UILibIS->ScrollY*ScrollIncr);
                UILibIS->ScrollY = 0;
            } 
            else if ( UILibIS->Shift && XScrollOn ) {
                ShiftOffsetX(-UILibIS->ScrollY*ScrollIncr);
                UILibIS->ScrollY = 0;
            }
        }

    }

    void UITextMap::SetScrollX( float pct ) {
        if ( ModeIsInput() ) { return; }
        if ( Width < Rect.GetWidth() ) { return; }
        if ( pct < 0 ) { pct = 0; } else if ( pct > 1 ) { pct = 1; }
        ScrollPerc.x = pct; SyncOffsetX();
    }
    
    void UITextMap::SetScrollY( float pct ) {
        if ( ModeIsInput() ) { return; }
        if ( Height < Rect.GetHeight() ) { return; }
        if ( pct < 0 ) { pct = 0; } else if ( pct > 1 ) { pct = 1; }
        ScrollPerc.y = pct; SyncOffsetY();
    }

    void UITextMap::SetCaretFromPos( vec2 inPos ) {
        SetCarets( { GetActiveChar(inPos) } );        
    }

    void UITextMap::SyncOffsetX() {
        // Syncs the X Offset to the current ScrollPerc.
        // Input mode doesn't use ScrollPerc, so do nothing in that case.
        if ( ModeIsInput() ) { return; }
        Offset.x = (float) ( ( Rect.GetWidth() - Width ) * ( ScrollPerc.x - Format.Align.x ) );
    }

    void UITextMap::SyncOffsetY() {
        // Syncs the Y Offset to the current ScrollPerc.
        // Input mode doesn't use ScrollPerc, so do nothing in that case.
        if ( ModeIsInput() ) { return; }
        Offset.y = (float) ( ( Rect.GetHeight() - Height ) * ( ScrollPerc.y - Format.Align.y ) );
    }
    
    void UITextMap::ShiftOffsetX( float inSize ) {
        // Shift offset in the X direction.
        // A positive shift has the effect of scrolling to the right.
        // A negative shift has the effect of scrolling to the left.
        if ( Format.Wrap ) { return; }
        if ( Mode == tm_floating_input ) { return; }
        Offset.x -= inSize;
        if ( Mode == tm_input ) { return; }
        float ViewWidth = Rect.GetWidth();
        ScrollPerc.x = Offset.x / ( ViewWidth - Width ) + Format.Align.x;
        if      ( ScrollPerc.x < 0 ) { ScrollPerc.x = 0; Offset.x =  (float) ( ( ViewWidth - Width ) * ( ScrollPerc.x - Format.Align.x ) ); }
        else if ( ScrollPerc.x > 1 ) { ScrollPerc.x = 1; Offset.x =  (float) ( ( ViewWidth - Width ) * ( ScrollPerc.x - Format.Align.x ) ); }
    }

    void UITextMap::ShiftOffsetY( float inSize ) {
        // Shift offset in the Y direction.
        // A positive shift has the effect of scrolling down.
        // A negative shift has the effect of scrolling up.
        Offset.y -= inSize;
        if ( ModeIsInput() ) { return; }
        float ViewHeight = Rect.GetHeight();
        ScrollPerc.y = Offset.y / ( ViewHeight - Height ) + Format.Align.y;
        if      ( ScrollPerc.y < 0 ) { ScrollPerc.y = 0; Offset.y = (float) ( ( ViewHeight - Height ) * ( ScrollPerc.y - Format.Align.y ) ); }
        else if ( ScrollPerc.y > 1 ) { ScrollPerc.y = 1; Offset.y = (float) ( ( ViewHeight - Height ) * ( ScrollPerc.y - Format.Align.y ) ); }
    }

    void UITextMap::ShiftOffsetToChar( int inCharI ) {
        if ( inCharI < 0 || inCharI >= CList.size() ) { return; }
        if ( Width < Rect.GetWidth() ) {
            ScrollPerc.x = 0; SyncOffsetX();
        }
        else {
            float posX = CharPosX(inCharI);
            float adjX =  ( !ModeIsInput() && Width > Rect.GetWidth() ) ? ScrollSize : 0;
            float extraW = inCharI == 0 ? 0 : CharW;
            if  ( posX < Rect.Min.x ) { ShiftOffsetX( posX - extraW - Rect.Min.x ); }
            else if ( posX + CList[inCharI].w > Rect.Max.x - adjX ) { ShiftOffsetX( posX + extraW - (Rect.Max.x - adjX) ); }
        }
        if ( Height < Rect.GetHeight() ) {
            ScrollPerc.y = 0; SyncOffsetY();
        }
        else {
            float posY = CharPosY(inCharI);
            float adjY =  ( !ModeIsInput() && Height > Rect.GetHeight() ) ? ScrollSize : 0;
            if ( LList.size() == 1 ) { return; }
            if ( posY < Rect.Min.y ) { ShiftOffsetY( posY - LineH - Rect.Min.y ); }
            else if ( posY + LineH > Rect.Max.y - adjY ) { ShiftOffsetY( posY + LineH - (Rect.Max.y - adjY) ); }
        }   
    }
    
#ifndef NDEBUG
    void UITextMap::DrawDebug() {

        // Draw the ClickRect
        UIDrawBBOutline( ClickRect, 0x00FF00, 2 );
        UIAddTextWithFontBB(ClickRect, "ClickRect", 0x00FF00, 0, {0,0}, 0.75f );

        // Draw the Rect
        UIDrawBBOutline( Rect, 0xF8742A, 2 );
        UIAddTextWithFontBB( Rect, "Rect", 0xF8742A, 0, {0,0}, 0.75f );

        // Draw the TextRect
        UIDrawBBOutline( TextRect, 0x2FF3E0, 2 );
        UIAddTextWithFontBB( TextRect, "TextRect", 0x2FF3E0, 0, {0,0}, 0.75f );

        // Get the active char index
        int cI = GetActiveChar( GetGlobalMousePos() );

        // Draw the char boxes
        ImRect charBB;
        int fillClr, lineClr;
        float fillAlpha;
        float charW;
        
        for ( int i = 0; i < CList.size(); i++ ) {
            
            if ( CList[i].c == 0 ) { // Display space
                charW = CList[i].w;
                fillClr = 0xFF0000; fillAlpha = 0.5f;
                lineClr = 0xFF0000;
            }
            else if ( CList[i].c == '\n' ) {  // New Line
                charW = CharW;
                fillClr = 0x0000FF; fillAlpha = 0.2f;
                lineClr = 0x0000FF;
            }
            else {  // All other chars
                charW = CList[i].w;
                fillClr = 0x00AAFF; fillAlpha = 0.03f;
                lineClr = 0x00AAFF;
            }

            if ( i == cI ) {
                fillClr = 0xFF00FF;
                lineClr = 0xFF00FF;
            }

            charBB = GetBB( CharPosX(i)+1, CharPosY(i), charW, LineH );
            UIDrawAlphaBB(charBB, fillClr, fillAlpha);
            UIDrawBBOutline(charBB, lineClr, 1);
            UIAddTextWithFontBB(charBB, to_string(i), 0x000000, 0, {0.13, 0.05}, 0.75f );

        }

    }
#endif

    float UITextMap::LinePosX( int inLineI ) {
        // Returns the line X position in global space
        if ( inLineI < 0 || inLineI >= LList.size() ) { return 0; }
        const LineData & ld = LList[inLineI];
        return Rect.Min.x + ( Rect.GetWidth() - ld.width ) * Format.Align.x + Offset.x;
    }

    float UITextMap::LinePosY( int inLineI ) {
        // Returns the line Y position in global space
        if ( inLineI < 0 || inLineI >= LList.size() ) { return 0; }
        return Rect.Min.y + ( Rect.GetHeight() - Height ) * Format.Align.y + inLineI*LineH + Offset.y;
    }

    float UITextMap::CharPosX( int inCharI ) {
        // Returns the character X position in global space 
        if ( inCharI < 0 || inCharI >= CList.size() ) { return 0; }
        const CharData & cd = CList[inCharI];
        return LinePosX( cd.line ) + cd.x;
    }

    float UITextMap::CharPosY( int inCharI ) {
        // Returns the character Y position in global space 
        if ( inCharI < 0 || inCharI >= CList.size() ) { return 0; }
        const CharData & cd = CList[inCharI];
        return LinePosY( cd.line );
    }

    float UITextMap::CharWidth( int inCharI ) {
        // Returns the character width
        if ( inCharI < 0 || inCharI >= CList.size() ) { return 0; }
        if ( inCharI == CList.size()-1 || CList[inCharI].line < CList[inCharI+1].line ) {
            return LList[ CList[inCharI].line ].width - CList[inCharI].x;
        }
        return CList[inCharI+1].x - CList[inCharI].x;
    }

    bool UITextMap::CharVisible( int inCharI ) {
        // Returns whether the character is visible
        if ( inCharI < 0 || inCharI >= CList.size() ) { return false; }
        const vec2 pos = { CharPosX(inCharI), CharPosY(inCharI) };
        vec2 adj = vec2(0);
        if ( !ModeIsInput() ) {
            if ( Width > Rect.GetWidth() ) { adj.x = ScrollSize; }
            if ( Height > Rect.GetHeight() ) { adj.y = ScrollSize; }
        }
        if ( pos.x + CList[inCharI].w < Rect.Min.x || pos.x > Rect.Max.x - adj.x ) { return false; }
        if ( pos.y + LineH < Rect.Min.y || pos.y > Rect.Max.y - adj.y ) { return false; }
        return true;
    }

    ImRect UITextMap::GetTextBoundingBox() {
        // Returns the text bounding box (used to compute TextRect)
        return GetBB( 
            Rect.Min.x + ( Rect.GetWidth()  - Width  ) * Format.Align.x + Offset.x,
            Rect.Min.y + ( Rect.GetHeight() - Height ) * Format.Align.y + Offset.y,
            Width,
            Height
        );
    }

    vec2 UITextMap::GetTextDim() {
        // Returns the dimension of the text (excludes the last display space width)
        return { Width, Height };
    }

    int UITextMap::GetActiveLine( float yPos ) {
        // Get the active line index given a global y position
        if ( yPos < TextRect.Min.y + LineH ) { return 0; }
        if ( yPos >= TextRect.Max.y - LineH ) { return NbLines-1; }
        return std::min( NbLines-1, std::max( 0, (int) ( ( yPos - TextRect.Min.y ) / LineH ) ) );
    }

    int UITextMap::GetActiveChar( vec2 inPos ) {
        // Get the CList active char index based on the position.
        // This is used by Mouse click and drag to determine which
        // char should be acted on. 
        if ( NbLines <= 0 ) { return -1; }
        int lineI = GetActiveLine(inPos.y);
        float adjX = inPos.x - LinePosX( lineI );
        int cI[2] = { LList[lineI].index, LList[lineI].index+LList[lineI].count-1 };
        
        int retIndex = -1;
        if ( cI[0] == cI[1] ) { retIndex = cI[0]; }
        else if ( adjX < CList[cI[0]].x ){ retIndex = cI[0]; }
        else if ( adjX > CList[cI[1]].x - (CList[cI[1]].x - CList[cI[1]-1].x)*.5f ){ retIndex = cI[1];}
        else {
            float lastX = CList[cI[0]].x;
            for ( int i = cI[0]+1; i <= cI[1]; i++ ) {
                if ( adjX < CList[i].x - (CList[i].x-lastX)*.5f) { retIndex = i-1; break; }
                lastX = CList[i].x;
            }
        }

        // If the CList value does not refer to a Text character, crawl ahead until you hit one.
        if ( retIndex != -1 ) {
            while ( CList[retIndex].c == 0 ) { 
                retIndex++; if ( retIndex >= (int)CList.size() ) { retIndex = (int)CList.size()-1; break; }
            }
        }

        return retIndex;
    }

    void UITextMap::SetFocus(bool inFocus) {
        // Sets the input focus (for blinking caret)
        if ( inFocus == Focus ) { return; }
        #ifdef LOG_UITEXTMAP
        std::cout << "Setting Focus: " << ( inFocus ? "true" : "false" ) << std::endl; 
        #endif
        Focus = inFocus;
        if ( Focus ) { ResetTime(LastBlinkTime); ResetTime(LastShiftTime); }
        Change = tm_focus_change;
    }

    bool UITextMap::Focused() { return Focus; }

    int UITextMap::GetChange() { return Change; }

    string UITextMap::GetText() {
        string text; text.reserve(CList.size());
        for ( const CharData & cdata : CList ) {
            if ( cdata.c != 0 ) { text += cdata.c; }
        }
        return std::move(text);
    }

    vector<int> UITextMap::GetSelectedLines() {
        // Returns a unique list of the selected line indexes
        vector<int> res = {};
        for ( const auto & s : SList ) {
            for ( int i = CList[s.S].line; i <= CList[s.E].line; i++ ) {
                res.push_back(i);
            }
        }
        std::sort( res.begin(), res.end() );
        res.erase( std::unique( res.begin(), res.end() ), res.end() );
        return std::move(res);
    }

    vector<int> UITextMap::GetTextRange( int start, int end ) {
        // Convert from CList range to the corresponding range of characters in Text.
        // Assumes end index is exclusive.
        vector<int> res(2,-1);
        // Crawl ahead to find first char found in Text
        for ( int j = start; j <= end; j++ ) {
            if ( CList[j].i >= 0 ) { res[0] = CList[j].i; break; }
        }
        if ( res[0] == -1 ) { return {}; } // no chars found
        // Crawl backwards to find the last char found in Text
        if ( CList[end].i >= 0 ) { res[1] = CList[end].i; }
        else {
            for ( int j = end-1; j > start; j-- ) {
                if ( CList[j].i >= 0 ) { res[1] = CList[j].i+1; break; }
            }
            if ( res[1] == -1 ) { res[1] = res[0]+1; }
        }
        return std::move(res);
    }

    void UITextMap::ResetTime( std::chrono::high_resolution_clock::time_point & tp ) {
        tp = std::chrono::high_resolution_clock::now();
    }

} // end namespace UILib

#endif /* UILIBINPUT_CPP */
