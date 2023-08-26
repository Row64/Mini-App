#ifndef  UILIB_CPP
#define  UILIB_CPP

#include "UILib.h"
#include <math.h>
#include <cmath>
#include <iostream>
#include <sstream>
#include <fstream>

#pragma warning(disable:4996)  //  cross-platform so avoid these Microsoft warnings

#include <boost/filesystem.hpp>
#include <boost/dll/runtime_symbol_info.hpp>
namespace fs = boost::filesystem;

#ifndef CONVERT_PATH
    #if defined ( __APPLE__)
        #define CONVERT_PATH( pathvar ) pathvar.c_str()
    #elif defined ( _WIN32 )
        #define CONVERT_PATH( pathvar ) pathvar.string().c_str()
    #else
        #define CONVERT_PATH( pathvar ) pathvar.c_str()
    #endif
#endif

#define INCLUDE_STB_IMPLEMENTATION_DEFINES  // uncomment to implement STB macros

// STB implementation macros must be defined only once in a source file.
// If the macro INCLUDE_STB_IMPLEMENTATION_DEFINES is defined, then the
// STB macros are defined here, and the remainder of the project just needs
// to include the relevant stb_*.h file.
#ifdef INCLUDE_STB_IMPLEMENTATION_DEFINES
    #define STB_IMAGE_IMPLEMENTATION
    #include "stb_image.h" 
    #define STB_IMAGE_WRITE_IMPLEMENTATION
    #include "stb_image_write.h"
    #define STB_RECT_PACK_IMPLEMENTATION
    #include "stb_rect_pack.h"
#else
    #include "stb_image.h"
#endif

#if defined(__linux__)
#include <arpa/inet.h>  // need for ntohl
#elif defined ( _WIN32 )
#pragma comment(lib, "Ws2_32.lib") // need for ntohl
#endif

#include "poly2tri.h"

namespace UILib {

// --------------------------------------------------------------------- //
// [UILib] Formatting Structs                                            //
// --------------------------------------------------------------------- //

    UIDataCalendar::UIDataCalendar() : ViewYear(-1), ViewMonth(-1) {
        std::time_t t = std::time(0); // current time ( seconds since epoch 1970-01-01T00:00 )
        DatetimeFromS( (int64_t) t, Year, Month, Day, Hour, Minute, Second );
        SetView( Year, Month );
        ComboID = "_UIDataCalendar"+to_string(calIndex++);
        SyncToCombo();
    }

    UIDataCalendar::UIDataCalendar( int inYear, int inMon, int inDay, int inH, int inM ) :
        Year(inYear), Month(inMon), Day(inDay), Hour(inH), Minute(inM), ViewYear(-1), ViewMonth(-1) {
        SetView( Year, Month );
        ComboID = "_UIDataCalendar"+to_string(calIndex++);
        SyncToCombo();
    }

    UIDataCalendar::~UIDataCalendar() {
        UIRemoveComboBox( ComboID+"H" );
        UIRemoveComboBox( ComboID+"M" );
        UIRemoveComboBox( ComboID+"A" );
    }

    void UIDataCalendar::SetDate( int inYear, int inMon, int inDay, int inH, int inM, int inS) {
        Year = inYear; Month = inMon; Day = inDay; Hour = inH; Minute = inM; Second = inS;
        SetView( Year, Month );
        SyncToCombo();
    }

    void UIDataCalendar::SetView( int inYear, int inMon ) {
        if ( ViewYear != inYear || ViewMonth != inMon ) {
            ViewYear = inYear;
            ViewMonth = inMon; if ( ViewMonth < 1 ) { ViewMonth = 1; } else if ( ViewMonth > 12 ) { ViewMonth = 12; }
            ViewWeekday = Weekday( ViewYear, ViewMonth, 1 );
            ViewRow = ViewWeekday == 0 ? 1 : 0;
        }
    }

    int UIDataCalendar::DaysInMonth( int inMon, bool isLeapYear ) {
        switch ( inMon ) {
            case 1 : return 31;                     // January
            case 2 : return isLeapYear ? 29 : 28;   // February
            case 3 : return 31;                     // March
            case 4 : return 30;                     // April
            case 5 : return 31;                     // May
            case 6 : return 30;                     // June
            case 7 : return 31;                     // July
            case 8 : return 31;                     // August
            case 9 : return 30;                     // September
            case 10 : return 31;                     // October
            case 11 : return 30;                    // November
            case 12 : return 31;                    // December
            default : return 0;
        }
    }

    string UIDataCalendar::MonthAsString( int inMon, bool abbrev ) {
        switch ( inMon ) {
            case 1 : return abbrev ? "Jan" : "January";
            case 2 : return abbrev ? "Feb" : "February";
            case 3 : return abbrev ? "Mar" : "March";
            case 4 : return abbrev ? "Apr" : "April";
            case 5 : return abbrev ? "May" : "May";
            case 6 : return abbrev ? "Jun" : "June";
            case 7 : return abbrev ? "Jul" : "July";
            case 8 : return abbrev ? "Aug" : "August";
            case 9 : return abbrev ? "Sep" : "September";
            case 10 : return abbrev ? "Oct" : "October";
            case 11 : return abbrev ? "Nov" : "November";
            case 12 : return abbrev ? "Dec" : "December";
            default : return "";
        }
    }

    bool UIDataCalendar::IsLeapYear( int inYear ) {
        return ( inYear%400==0) || ( inYear%4==0 && inYear%100!=0);
    }

    void UIDataCalendar::SyncToCombo() {
        UIComboBox * HComboBox = UIAddComboBox(ComboID+"H", 0);
        if      ( Hour == 0 || Hour == 12 ) { HComboBox->Index = 11; }
        else if ( Hour > 11               ) { HComboBox->Index = Hour - 13; }
        else                                { HComboBox->Index = Hour - 1;  }
        UIComboBox * MComboBox = UIAddComboBox(ComboID+"M", 0);
        MComboBox->Index = Minute;
        UIComboBox * AComboBox = UIAddComboBox(ComboID+"A", 0);
        AComboBox->Index = ( Hour > 11 ? 1 : 0 );            
    }

    void UIDataCalendar::SyncFromCombo() {
        // Get the combo boxes. If they don't exist, then create them.
        UIComboBox * HComboBox = UIGetComboBox(ComboID+"H"); if ( !HComboBox ) { SyncToCombo(); return; }
        UIComboBox * MComboBox = UIGetComboBox(ComboID+"M"); if ( !MComboBox ) { SyncToCombo(); return; }
        UIComboBox * AComboBox = UIGetComboBox(ComboID+"A"); if ( !AComboBox ) { SyncToCombo(); return; }
        // Only sync when all combos are closed
        bool SyncOk = !HComboBox->Open && !MComboBox->Open && !AComboBox->Open;
        if ( SyncOk ) {
            Hour = HComboBox->Index + 1;
            if ( Hour == 12 ) { Hour = 0; }
            if ( AComboBox->Index == 1 ) { Hour += 12; }
            Minute = MComboBox->Index;
        }
        if ( HComboBox->Open != ComboOpen[0] ) { ComboOpen[0] = HComboBox->Open; Status = HComboBox->Open ? status_combo_opened : status_combo_closed; }
        if ( MComboBox->Open != ComboOpen[1] ) { ComboOpen[1] = MComboBox->Open; Status = MComboBox->Open ? status_combo_opened : status_combo_closed; }
        if ( AComboBox->Open != ComboOpen[2] ) { ComboOpen[2] = AComboBox->Open; Status = AComboBox->Open ? status_combo_opened : status_combo_closed; }
    }


    void UIDataCalendar::CloseComboBoxes() {
        if (   ( UIComboGetFocusedName() == ComboID+"H" ) 
            || ( UIComboGetFocusedName() == ComboID+"M" )
            || ( UIComboGetFocusedName() == ComboID+"A" ) ) {
            UIComboExitFocus();
        }        
    }

    // NOTE: These are chrono-Compatible algorithms for converting to and from civil time adapted from 
    // this library (MIT-Licensed):
    // http://howardhinnant.github.io/date_algorithms.html
    // https://github.com/HowardHinnant/date
    // These algorithms implement a proleptic Gregorian calendar, which is the same calendar method adopted by Numpy.
    // See: https://numpy.org/doc/stable/reference/arrays.datetime.html#datetime64-conventions-and-assumptions

    void UIDataCalendar::DatetimeFromS( int64_t inTime, int & outY, int & outM, int & outD, int & outH, int & outMin, int & outS ) {
        // Fills the Calendar datetime from inTime.
        // inTime is seconds from epoch, which is 1970-01-01T00:00.
        outS = inTime % 86400;
        if ( outS < 0 ) { outS += 86400; inTime -= 86400; }
        outH = outS / 3600;
        outMin = outS / 60 % 60;
        outS %= 60;
        inTime /= 86400;   // convert to days from epoch
        inTime += 719468;  // shift the epoch from 1970-01-01 to 0000-03-01
        const int era = static_cast<int>(inTime >= 0 ? inTime : inTime - 146096) / 146097;
        const uint32_t doe = static_cast<uint32_t>(inTime - era * 146097);
        const uint32_t yoe = (doe - doe/1460 + doe/36524 - doe/146096) / 365;
        outY = static_cast<int>(yoe) + era * 400;
        const uint32_t doy = doe - (365*yoe + yoe/4 - yoe/100);
        outM = (5*doy + 2)/153;
        outD = doy - (153*outM+2)/5 + 1;
        outM = outM < 10 ? outM+3 : outM-9;
        if (outM <= 2) { outY++; }
    }

    int64_t UIDataCalendar::DatetimeToS( int inY, int inM, int inD, int inH, int inMin, int inS ) {
        // Converts a datetime to seconds from epoch, which is 1970-01-01T00:00.
        if ( inM <= 2 ) { inY--; }
        const int era = static_cast<int>(inY >= 0 ? inY : inY-399) / 400;
        const uint32_t yoe = static_cast<uint32_t>(inY - era * 400);
        const uint32_t doy = (153*(inM > 2 ? inM-3 : inM+9) + 2)/5 + inD-1;
        const uint32_t doe = yoe * 365 + yoe/4 - yoe/100 + doy;
        int64_t outTime = static_cast<int64_t>( era * 146097 + static_cast<int>(doe) - 719468 ); // Days
        outTime *= 86400; // Convert to seconds
        outTime += inH*3600 + inMin*60 + inS; // Total whole seconds
        return outTime;
    }

    int64_t UIDataCalendar::DatetimeToD( int inY, int inM, int inD ) {
        // Converts a datetime to days from epoch, which is 1970-01-01T00:00.
        if ( inM <= 2 ) { inY--; }
        const int era = static_cast<int>(inY >= 0 ? inY : inY-399) / 400;
        const uint32_t yoe = static_cast<uint32_t>(inY - era * 400);
        const uint32_t doy = (153*(inM > 2 ? inM-3 : inM+9) + 2)/5 + inD-1;
        const uint32_t doe = yoe * 365 + yoe/4 - yoe/100 + doy;
        return static_cast<int64_t>( era * 146097 + static_cast<int>(doe) - 719468 ); // Days
    }

    int UIDataCalendar::Weekday( int inYear, int inMon, int inDay ) {
        // Returns the day of week from 0 to 6, where 0 = Sunday
        int64_t DaysFromEpoch = DatetimeToD( inYear, inMon, inDay );
        return static_cast<unsigned>(DaysFromEpoch >= -4 ? (DaysFromEpoch+4) % 7 : (DaysFromEpoch+5) % 7 + 6);
    }

// --------------------------------------------------------------------- //
// [UILib] Core                                                          //
// --------------------------------------------------------------------- //

    #ifndef UILibCTX
    UILibContext*     UILibCTX = NULL;
    #endif
    
    #ifndef UILibIS
    InputStateData*   UILibIS = NULL;
    #endif

    #ifndef UILibDS
    DisplayStateData* UILibDS = NULL;
    #endif
    
    #ifndef UILibWindowHandle
    void* UILibWindowHandle = NULL;
    #endif

    UILibContext* UICreateContext() {
        // UICreateContext creates the ImGuiContext, which as the ImGuiIO.
        // This also creates the UILibIO, which will eventually replace ImGuiIO.
        // TO DO: Eventually plan to replace ImGui structs with UILib versions.

        UILibContext* ctx = new UILibContext();
        if (UILibCTX == NULL) { UISetCurrentContext(ctx); }
        ctx->ImGuiCTX = ImGui::CreateContext();
        UISetScheme();
        return ctx;
    }

    void UIDestroyContext(UILibContext* ctx) {
        if (ctx == NULL) { ctx = UILibCTX; }
        if (UILibCTX == ctx) { UISetCurrentContext(NULL); }
        ImGui::DestroyContext( ctx->ImGuiCTX );
        delete ctx;
    }

    void UISetCurrentContext(UILibContext* ctx) {
        UILibCTX = ctx;
    }
    
    UILibContext* UIGetCurrentContext() {
        return UILibCTX;
    }

    void UIRender( bool clearFrame ) {
        ImGui::Render();
        if ( clearFrame ) {
            UIClearFrame();
        }
    }

    void UIClearFrame() {
        UILibCTX->IO.ImagePaths.clear();    // Clear image paths list
        UILibCTX->InputChange = 0;          // Clear the key input change for the current frame
        UILibIS->Key.CodePoint = 0;         // Clear codepoint if it was not processed current frame
    }

    void UISetInputStatePointer( InputStateData* inMS ) {
        UILibIS = inMS;
    }

    InputStateData* UIGetInputState() {
        return UILibIS;
    }

    // Syncing GLFW Input with Pipeline Draw Timeline - Simplifies Immediate Mode UI
    void LogKeyState(string inStr){
        if ( UILibIS == NULL ) { return; }
        InputStateData & InputState = *UILibIS;
        auto *kState = &InputState.Key;
        string KeyVal;
        if(kState->Key == 0){KeyVal = "0";}
        else{KeyVal = (char)kState->Key;}
        string LastVal;
        if(kState->Last == 0){LastVal = "0";}
        else{LastVal = (char)kState->Last;}
        cout << inStr << "\t" << KeyVal << "\t" << LastVal << "\t" << kState->Change << "\t" << kState->IsPressed << "\n";
    }

    void UIInputUpdate() {
        if ( UILibIS == NULL ) { return; }
        InputStateData & InputState = *UILibIS;

        // update mouse to sync with pipeline
        auto *b1 = &InputState.Buttons[0];  // left click 
        auto *b2 = &InputState.Buttons[1];  // right click
        auto *b3 = &InputState.Buttons[2];  // middle click

        if(b1->IsPressed == true && b1->Last == false){InputState.Buttons[0].Change = 1;}
        else if(b1->IsPressed == false && b1->Last == true){InputState.Buttons[0].Change = 2;}
        else{InputState.Buttons[0].Change = 0;}

        if(b2->IsPressed == true && b2->Last == false){InputState.Buttons[1].Change = 1;}
        else if(b2->IsPressed == false && b2->Last == true){InputState.Buttons[1].Change = 2;}
        else{InputState.Buttons[1].Change = 0;}

        if(b3->IsPressed == true && b3->Last == false){InputState.Buttons[2].Change = 1;}
        else if(b3->IsPressed == false && b3->Last == true){InputState.Buttons[2].Change = 2;}
        else{InputState.Buttons[2].Change = 0;}

        InputState.Buttons[0].Last = InputState.Buttons[0].IsPressed;
        InputState.Buttons[1].Last = InputState.Buttons[1].IsPressed;
        InputState.Buttons[2].Last = InputState.Buttons[2].IsPressed;

        // update mouse position to sync with pipeline
        InputState.Position.Delta.X = InputState.Position.X - InputState.Position.LastX;
        InputState.Position.Delta.Y = InputState.Position.Y - InputState.Position.LastY;
        double deltaSum = abs(InputState.Position.Delta.X + InputState.Position.Delta.Y);
        if(deltaSum >= 2){InputState.DoubleClick = 0;}
        InputState.Position.LastX = InputState.Position.X;
        InputState.Position.LastY = InputState.Position.Y;
        
        // The simplified key state is synced with the vulkan draw timeline.  It converts events into states that are easy to read when drawing.
        // It collapses/records multi-key press events into the KeyData struct so that only one key can be down at any time
        // Modifier keys work but are limited to Ctrl/Shift/Alt/Super
        // It organizes Key.Change so that every key down (Change=1) will have a matching key up (Change=2) before a new key goes down.
        // example: press "k" down, while it's down press "a" down.  Double down more like sublime:  kkkkkkkkkkkkkaaaaaaaaaaa with a "k" up added before a starts
        // If you want complex multi-key combination events then read them right off of GLFW in c_Window.cpp - Window::KeyEvent
        // Otherwise this is simple and easy to use, inspired by how ImGUI handles key states
        
        auto *kState = &InputState.Key;
        int bufSize = (int)size(kState->Buffer);

        if ( kState->Reset ) {
            kState->Key = 0;
            kState->Last = 0;
            kState->Change = 0;
            kState->IsPressed = 0;
            kState->Buffer.clear();
            kState->Reset = false;
        }
        else if(bufSize == 0){
            kState->Change = 0;
            kState->Last = kState->Key;
            //LogKeyState("-\t");
        }
        else{
            vector<array<int,2>> newBuffer = {};
            if(kState->IsPressed == 0){
                // take the first key down buffered event (if you have one).  Ignore up events (already up)
                bool recordNew = false;
                for(int i=0;i<bufSize;i++){
                    if(recordNew){ newBuffer.push_back( kState->Buffer[i]);}
                    else if(kState->Buffer[i][0] == 1){
                        kState->Key = kState->Buffer[i][1];
                        kState->Last = 0;
                        kState->Change = 1;
                        kState->IsPressed = 1;
                        recordNew = true;
                        // string logStr; logStr =(char) kState->Buffer[i][1];
                        //LogKeyState("DnEvent:" + logStr);
                    }
                }
                if(!recordNew){
                    kState->Last = kState->Key;
                    kState->Change = 0;
                }
                kState->Buffer = newBuffer;
            }
            else if(kState->IsPressed == 1){
                bool recordNew = false;
                for(int i=0;i<bufSize;i++){
                    if(recordNew){newBuffer.push_back( kState->Buffer[i]);}
                    else if(kState->Buffer[i][0] == 1){
                        // if you are already down, close the old key, carry over the new key to make it pressed
                        newBuffer.push_back( kState->Buffer[i]);  // carry over
                        kState->Last = kState->Key;
                        kState->Key = 0;
                        kState->Change = 2;
                        kState->IsPressed = 0;
                        recordNew = true;
                        // string logStr; logStr =(char) kState->Buffer[i][1];
                        //LogKeyState("DoubleDnToUp:" + logStr);

                    }
                    else if(kState->Buffer[i][0] == 2){
                        // take the first key up buffered event (if you have one)
                        kState->Last = kState->Key;
                        kState->Key = 0;
                        kState->Change = 2;
                        kState->IsPressed = 0;
                        recordNew = true;
                        // string logStr; logStr =(char) kState->Buffer[i][1];
                        //LogKeyState("UpEvent:" + logStr);
                    }
                }
                if(!recordNew){
                    kState->Last = kState->Key;
                    kState->Change = 0;
                }
                kState->Buffer = newBuffer;
            }
        }

    }

    int GetKeyDown() {
        InputStateData& ins = *UILibIS;
        return ins.Key.Key;
    }

    bool KeyIsPrintable( int key ) {
        // See: https://www.glfw.org/docs/3.3/group__keys.html
        return ( key < 128 || ( key >= 320 && key <= 334) );
    }
    
    float GetMouseScroll() {  // reading will set to zero so you can get delta next update
        InputStateData& m = *UILibIS;
        float scrollY = m.ScrollY;
        m.ScrollY = 0;
        return scrollY;
    }

    bool AnyScroll() {
        InputStateData& m = *UILibIS;
        return (m.ScrollY != 0);
    }

    highp_dvec2 GetMousePosD(){  // get mouse pos as double
        InputStateData& m = *UILibIS;
        return highp_dvec2((double)m.Position.X, (double)m.Position.Y);
    }
    vec2 GetMousePos() {
        InputStateData& m = *UILibIS;
        return vec2(m.Position.X, m.Position.Y);
    }
    vec2 GetMouseDelta() {
        InputStateData& m = *UILibIS;
        return vec2(m.Position.Delta.X, m.Position.Delta.Y);
    }
    vec2 GetLocalMousePos(){
        InputStateData& m = *UILibIS;
        ImVec2 wPos = GImGui->CurrentWindow->Pos;
        return vec2(m.Position.X - wPos.x, m.Position.Y - wPos.y);
    }
    vec2 GetGlobalMousePos(){
        InputStateData& m = *UILibIS;
        return vec2(m.Position.X, m.Position.Y);
    }
    bool VecInBB(const vec2& inVec, const ImRect& inBB){
        return inBB.Contains( inVec );
    }

    bool VecInBBClip(const vec2& inVec, const ImRect& inBB){
        auto clipRect = GImGui->CurrentWindow->ClipRect;
        return VecInBB( inVec, inBB ) && VecInBB( inVec, clipRect );
    }
    
    bool VecIsClipped(const vec2& inVec) {
        auto clipRect = GImGui->CurrentWindow->ClipRect;
        return !( VecInBB( inVec, clipRect ) );
    }

    bool VecInTriangle(const vec2& s, const vec2& a, const vec2& b, const vec2& c) {
        float as_x = s[0]-a[0];
		float as_y = s[1]-a[1];
		bool s_ab = (b[0]-a[0])*as_y-(b[1]-a[1])*as_x > 0;
		if((c[0]-a[0])*as_y-(c[1]-a[1])*as_x > 0 == s_ab) return false;
		if((c[0]-b[0])*(s[1]-b[1])-(c[1]-b[1])*(s[0]-b[0]) > 0 != s_ab) return false;
		return true;
    }

    bool VecInQuad(const vec2& s, const vec2& a, const vec2& b, const vec2& c, const vec2& d) {
        // A, B, C, D are clockwise quad points
        return VecInTriangle( s, a, b, c) || VecInTriangle( s, a, c, d );
    }

    bool VecInLine(const vec2& s, const vec2& a, const vec2& b, float inThickness) {
        vec2 N = glm::normalize( b-a );
        vec2 p0 = a + vec2( -N.y,  N.x )*inThickness;
        vec2 p1 = a + vec2(  N.y, -N.x )*inThickness;
        vec2 p2 = b + vec2(  N.y, -N.x )*inThickness;
        vec2 p3 = b + vec2( -N.y,  N.x )*inThickness;
        // UIAddGlobalLine( a.x, a.y, b.x, b.y, 0x000000, 2 );
        // UIAddGlobalCircle( p0.x, p0.y, 5, 0xFF0000, true, 0, 10);
        // UIAddGlobalCircle( p1.x, p1.y, 5, 0x00FF00, true, 0, 10);
        // UIAddGlobalCircle( p2.x, p2.y, 5, 0x0000FF, true, 0, 10);
        // UIAddGlobalCircle( p3.x, p3.y, 5, 0xFF00FF, true, 0, 10);
        return VecInQuad( s, p0, p1, p2, p3 );
    }

    int RGBtoInt(float inR, float inG, float inB){
        int rv = (int)(inR*255);
        int gv = (int)(inG*255);
        int bv = (int)(inB*255);
        return ((rv&0x0ff)<<16)|((gv&0x0ff)<<8)|(bv&0x0ff);
    }

    int RGBtoInt(vec3 inRGB) {
        return RGBtoInt( inRGB.x, inRGB.y, inRGB.z );
    }

    int RGBtoInt(vec4 inRGBA) {
        return RGBtoInt( inRGBA.x, inRGBA.y, inRGBA.z );
    }
    
    void UISetCursor(int inCType, string inCName){

        GLFWwindow * window = (GLFWwindow*) UIGetWindowHandle();
        UICursor * c = &UILibCTX->Cursor;
        
        if ( inCType == uic_custom ) {
            if ( c->Type == uic_custom && c->Name == inCName ) { return; }   // exit if requested cursor is same as current cursor
            UICursor * newc = UIGetCursor( inCName );
            if ( newc != nullptr ) {
                if ( c->Type != uic_custom ) { glfwDestroyCursor( c->Cursor ); }    // destroy the previous cursor if it is not a custom cursor
                *c = *newc;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                glfwSetCursor( window, c->Cursor);
            }
            return;  
        }
        
        if ( inCType != c->Type ) {
            
            if(inCType == uic_hidden) { glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN); return; }

            int cType = GLFW_CURSOR_NORMAL;             
            if      (inCType == uic_normal    ) { cType = GLFW_CURSOR_NORMAL;    }
            else if (inCType == uic_hand      ) { cType = GLFW_HAND_CURSOR;      }
            else if (inCType == uic_ibeam     ) { cType = GLFW_IBEAM_CURSOR;     }
            else if (inCType == uic_crosshair ) { cType = GLFW_CROSSHAIR_CURSOR; }
            else if (inCType == uic_resize_h  ) { cType = GLFW_HRESIZE_CURSOR;   }
            else if (inCType == uic_resize_v  ) { cType = GLFW_VRESIZE_CURSOR;   }
            else if (inCType == uic_resize_tlbr  ) { cType = GLFW_RESIZE_NWSE_CURSOR;   }
            else if (inCType == uic_resize_trbl  ) { cType = GLFW_RESIZE_NESW_CURSOR;   }
            else if (inCType == uic_resize_all   ) { cType = GLFW_RESIZE_ALL_CURSOR;    std::cout << "Set Cursor Type: GLFW_RESIZE_ALL_CURSOR" << std::endl; }
            
            if ( c->Type != uic_custom ) { glfwDestroyCursor( c->Cursor ); }    // destroy the previous cursor if it is not a custom cursor
            c->Type = inCType;
            c->Cursor = glfwCreateStandardCursor(cType);
            c->Name = "";
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            glfwSetCursor( window, c->Cursor);
            
        }
        
    }
    void UISetDisplayStatePointer( DisplayStateData* inDS ) {
        UILibDS = inDS;
    }

    DisplayStateData* UIGetDisplayState() {
        return UILibDS;
    }

    vec2 UIGetWindowSize() {
        DisplayStateData& d = *UILibDS;
        return vec2(d.WindowSize.W, d.WindowSize.H);
    }

    vec2 UIGetFramebufSize() {
        DisplayStateData& d = *UILibDS;
        return vec2(d.FramebufferSize.W, d.FramebufferSize.H);
    }

    void UISetWindowHandlePointer( void* inWin ) {
        UILibWindowHandle = inWin;
    }

    void* UIGetWindowHandle() {
        return UILibWindowHandle;
    }

    void UISetWindowTitle( string inTitle ) {
        if ( UILibWindowHandle == NULL ) { return; }
        glfwSetWindowTitle( (GLFWwindow*) UILibWindowHandle, inTitle.c_str() );
    }

    void UISetDefaultScroller( UIFormatScroller inScroller ) {
        UILibCTX->DefaultScroller = inScroller;
    }

    void UISetDefaultFonts( int inFont, int inFontB, int inFontI, int inFontBI ) {
        UILibCTX->DefaultFont = inFont;
        UILibCTX->DefaultFontB = inFontB;
        UILibCTX->DefaultFontI = inFontI;
        UILibCTX->DefaultFontBI = inFontBI;
    }

    void UIAddCustomCursor( string inName, string inPath, int inHotX, int inHotY ) {
        GLFWimage img; int n;
        for ( auto & c : UILibCTX->CursorList ) {
            if ( c.Name == inName ) {
                glfwDestroyCursor( c.Cursor );
                img.pixels = stbi_load( inPath.c_str(), &img.width, &img.height, &n, 0);
                c.Cursor = glfwCreateCursor( &img, inHotX, inHotY );
                return;
            }
        }
        UICursor newC;
        img.pixels = stbi_load( inPath.c_str(), &img.width, &img.height, &n, 0);
        newC.Name = inName;
        newC.Cursor = glfwCreateCursor( &img, inHotX, inHotY );
        newC.Type = uic_custom;
        UILibCTX->CursorList.push_back( newC );        
    }

    void UIRemoveCustomCursor( string inName ) {
        auto & cList = UILibCTX->CursorList;
        for ( int i = 0; i < cList.size(); i++ ) {
            if ( cList[i].Name == inName ) {
                glfwDestroyCursor( cList[i].Cursor );
                cList.erase( cList.begin() + i );
                return;
            }
        }
    }

    UICursor * UIGetCursor( string inName ) {
        for ( auto & c : UILibCTX->CursorList ) {
            if ( c.Name == inName ) {
                return &c;
            }
        }
        return nullptr;
    }

    bool UIAddCornerShadeButton(ImRect inRect, vector<int> inColors, vector<float> inShadeFactors, float inThickness, float inRounding, std::string cornerBits ){
        
        bool rtn = false;
        vec2 mPos = GetGlobalMousePos();
        if(VecInBB(mPos, inRect)){
            if(MouseDown()){
                UIDrawAlphaRoundCornerBB(inRect, 0x000000, inShadeFactors[1], inRounding, cornerBits);
                if(DragStart()){rtn = true;}
            }
            else{
                UIDrawAlphaRoundCornerBB(inRect, 0x000000, inShadeFactors[0], inRounding, cornerBits);
            }
        }

        UIDrawRoundBBOutlineCorner(inRect, inColors[1], inRounding, inThickness, cornerBits);

        return rtn;

    }
    bool UIAddShadeButton(ImRect inRect, vector<int> inColors, vector<float> inShadeFactors, float inThickness, float inRounding  ){

        
        //UIDrawRoundBBOutline(inRect, 0xFF0000, inRounding, inThickness);

        bool rtn = false;
        vec2 mPos = GetGlobalMousePos();
        if(VecInBB(mPos, inRect)){
            if(MouseDown()){
                UIDrawAlphaRoundBB(inRect, 0x000000, inShadeFactors[1], inRounding);
                if(DragStart()){rtn = true;}
            }
            else{UIDrawAlphaRoundBB(inRect, 0x000000, inShadeFactors[0], inRounding);}
        }
        if(inThickness > .01){
            UIDrawRoundBBOutline(inRect, inColors[1], inRounding, inThickness);
        }
        return rtn;

    }
    bool UIAddButtonRoundRect(ImRect inR, string inText, vector<int> inColors, float inRounding, float inThickness, bool &inMuteUI){
        // inColors: [text, border, fill, over fill, click fill]
        vec2 mPos = GetGlobalMousePos();
        int clr = inColors[2];
        if(VecInBB(mPos, inR)){
            if(MouseDown()){clr = inColors[4];if(DragStart() && !inMuteUI){return true;}}else{clr = inColors[3];}
        }
        UIDrawRoundBB(inR, clr, inRounding);
        if(inThickness > 0){
            UIDrawRoundBBOutline(inR, inColors[1], inRounding, inThickness);
        }
        
        UIAddGlobalText( inR.Min.x, inR.Min.y, inR.GetWidth(), inR.GetHeight(), inText, inColors[0], {.5f, .5f} );
        return false;
    }
    bool UIAddButtonRoundRectF(ImRect inR, string inText, vector<int> inColors, float inRounding, float inThickness, bool &inMuteUI, int inFontI){
        // inColors: [text, border, fill, over fill, click fill]
        vec2 mPos = GetGlobalMousePos();
        int clr = inColors[2];
        if(VecInBB(mPos, inR)){
            if(MouseDown()){clr = inColors[4];if(DragStart() && !inMuteUI){return true;}}else{clr = inColors[3];}
        }
        UIDrawRoundBB(inR, clr, inRounding);
        if(inThickness > 0){
            UIDrawRoundBBOutline(inR, inColors[1], inRounding, inThickness);
        }
        
        UIAddTextWithFontBB( inR, inText, inColors[0], inFontI, {.5f, .5f} );

        return false;
    }
    int UIAddCornerButton(const ImRect & inR, const string & inText, const vector<int> & inColors, float inRounding, float inThickness, bool &inMuteUI, int inFontI, const std::string & cornerBits) {
        // inColors: [text, border, fill, over fill, click fill]
        // Returns ButtonMouseStatus
        vec2 mPos = GetGlobalMousePos();
        int clr = inColors[2];
        int res = bms_none;
        if(VecInBB(mPos, inR)){
            if( MouseDown() ) {
                clr = inColors[4]; res = bms_down;
                if ( DragStart() && !inMuteUI ) { res = bms_click; } 
            }
            else { clr = inColors[3]; res = bms_over; }
        }
        UIDrawAlphaRoundCornerBB(inR, clr, 1.0f, inRounding, cornerBits);
        if(inThickness > 0){
            UIDrawRoundBBOutlineCorner(inR, inColors[1], inRounding, inThickness, cornerBits);
        }
        UIAddTextWithFontBB( inR, inText, inColors[0], inFontI, {.5f, .5f} );
        return res;
    }

    bool UIAddButtonRect(ImRect inR, string inText, vector<int> inColors, bool &inMuteUI){
        vec2 mPos = GetGlobalMousePos();
        int clr = inColors[0];
        if(VecInBB(mPos, inR)){
            if(MouseDown()){clr = inColors[2];if(DragStart() && !inMuteUI){return true;}}else{clr = inColors[1];}
        }
        UIDrawBB(inR, clr);
        UIAddTextString( inR.Min.x, inR.Min.y, inR.GetWidth(), inR.GetHeight(), inText, 0x000000, {.5f, .5f} );
        return false;
    }

    bool UIAddButtonRectClipped(ImRect inR, string inText, vector<int> inColors, bool &inMuteUI){
        vec2 mPos = GetGlobalMousePos();
        int clr = inColors[0];
        if(VecInBBClip(mPos, inR)){
            if(MouseDown()){clr = inColors[2];if(DragStart() && !inMuteUI){return true;}}else{clr = inColors[1];}
        }
        UIDrawBB(inR, clr);
        UIAddTextString( inR.Min.x, inR.Min.y, inR.GetWidth(), inR.GetHeight(), inText, 0x000000, {.5f, .5f} );
        return false;
    }

    bool UIAddButtonRectJustified(ImRect inR, string inText, vector<int> inColors, bool &inMuteUI, vec2 inAlign) {
        vec2 mPos = GetGlobalMousePos();
        int clr = inColors[0];
        if(VecInBB(mPos, inR)){
            if(MouseDown()){clr = inColors[2];if(DragStart() && !inMuteUI){return true;}}else{clr = inColors[1];}
        }
        UIDrawBB(inR, clr);
        UIAddTextString( inR.Min.x, inR.Min.y, inR.GetWidth(), inR.GetHeight(), inText, 0x000000, {inAlign[0], inAlign[1]} );
        return false;
    }
    
    ImVec2 Pos(float x, float y, ImVec2 winPos){
        // Position to Window Local Space
        return ImVec2(x + winPos.x, y + winPos.y);
    }
    ColorSpace::Lab ClToLab(unsigned int hexValue){
        double cr = ((hexValue >> 16) & 0xFF);  // Extract the RR byte
        double cg = ((hexValue >> 8) & 0xFF);   // Extract the GG byte
        double cb = ((hexValue) & 0xFF);        // Extract the BB byte
        ColorSpace::Rgb rgb(cr, cg, cb);
        ColorSpace::Lab lab;
        rgb.To<ColorSpace::Lab>(&lab);
        return lab;
    }
    int LabtoInt(ColorSpace::Lab inLab){

        if(inLab.l < 0){inLab.l = 0;}
        if(inLab.l > 100){inLab.l = 100;}
        if(inLab.a < -86.185){inLab.a = -86.185;}
        if(inLab.a > 98.254){inLab.a = 98.254;}
        if(inLab.b < -107.863){inLab.b = -107.863;}
        if(inLab.b > 94.482){inLab.b = 94.482;}

        ColorSpace::Rgb rgb;
        inLab.To<ColorSpace::Rgb>(&rgb);
        int rv = (int)(rgb.r);
        int gv = (int)(rgb.g);
        int bv = (int)(rgb.b);
        // int rv = (int)(rgb.r*255);
        // int gv = (int)(rgb.g*255);
        // int bv = (int)(rgb.b*255);

        return ((rv&0x0ff)<<16)|((gv&0x0ff)<<8)|(bv&0x0ff);
    }
    ImU32 Cl(unsigned int hexValue){
        double cr = ((hexValue >> 16) & 0xFF);  // Extract the RR byte
        double cg = ((hexValue >> 8) & 0xFF);   // Extract the GG byte
        double cb = ((hexValue) & 0xFF);        // Extract the BB byte
        ImU32 clr = IM_COL32( cr, cg, cb, 255);
        return clr;
    }

    vec3 HexToRGBVec(int hexValue){
        
        float rv =((hexValue >> 16) & 0xFF) / 255.0f;  // Extract the RR byte
        float gv =((hexValue >> 8) & 0xFF) / 255.0f;   // Extract the GG byte
        float bv =((hexValue) & 0xFF) / 255.0f;        // Extract the BB byte

        return {rv,gv,bv};

    }
    void HexToHSV(const string & inHexStr, float &inH, float &inS, float &inV){

        unsigned int hexValue =  strtoul(inHexStr.c_str(), NULL, 16); 
        int cr = ((hexValue >> 16) & 0xFF);   // Extract the RR byte
        int cg =((hexValue >> 8) & 0xFF);    // Extract the GG byte
        int cb =((hexValue) & 0xFF);        // Extract the BB byte
        float fr = (float)cr/255;
        float fg = (float)cg/255;
        float fb = (float)cb/255;
        ColorConvertRGBtoHSV(fr, fg, fb, inH, inS, inV);
    }
    int HexToInt(const string & inHexStr) {
        return strtoul(inHexStr.c_str(), NULL, 16);
    }
    string IntToHexColor(unsigned int hexValue){

        int cr = ((hexValue >> 16) & 0xFF);  // Extract the RR byte
        int cg = ((hexValue >> 8) & 0xFF);   // Extract the GG byte
        int cb = ((hexValue) & 0xFF);        // Extract the BB byte
        
        char hexout[8];
        sprintf(hexout,"%02X%02X%02X",cr,cg,cb);
        string sOut; sOut = hexout;
        return sOut;

    }
    int HVStoInt(float &inH, float &inS, float &inV){
        float lR;float lG;float lB;
        ColorConvertHSVtoRGB(inH, inS, inV, lR, lG, lB);
        int rv = (int)(lR*255);
        int gv = (int)(lG*255);
        int bv = (int)(lB*255);
        int rgb = ((rv&0x0ff)<<16)|((gv&0x0ff)<<8)|(bv&0x0ff);
        return rgb;
    }
    void IntToHSV(unsigned int hexValue, float &inH, float &inS, float &inV){
        double cr = ((hexValue >> 16) & 0xFF);  // Extract the RR byte
        double cg = ((hexValue >> 8) & 0xFF);   // Extract the GG byte
        double cb = ((hexValue) & 0xFF);        // Extract the BB byte
        float fr = (float)cr/255;
        float fg = (float)cg/255;
        float fb = (float)cb/255;
        ColorConvertRGBtoHSV(fr, fg, fb, inH, inS, inV);
    }
    void IntToVec4(unsigned int color, vec4 &v) {
        float cr = (float)((color >> 16) & 0xFF);  // Extract the RR byte
        float cg = (float)((color >> 8) & 0xFF);   // Extract the GG byte
        float cb = (float)((color) & 0xFF);        // Extract the BB byte
        v.r = cr/255.0f;
        v.g = cg/255.0f;
        v.b = cb/255.0f;
        v.a = 1.0f;
    }
    ImU32 ClearCl(unsigned int hexValue){
        double cr = ((hexValue >> 16) & 0xFF);  // Extract the RR byte
        double cg = ((hexValue >> 8) & 0xFF);   // Extract the GG byte
        double cb = ((hexValue) & 0xFF);        // Extract the BB byte
        ImU32 clr = IM_COL32( cr, cg, cb, 0);
        return clr;
    }
    float YToGlobal(float inY){
        ImVec2 wPos = GImGui->CurrentWindow->Pos;
        return inY - wPos.y;
    }
    ImRect GetCenteredBB(float inX, float inY, float inW, float inH){
        ImVec2 wPos = GImGui->CurrentWindow->Pos;
        float gX = wPos.x + inX;
        float gY = wPos.y + inY;
        ImRect gBB = ImRect(gX-inW*0.5f, gY-inH*0.5f, gX+inW*0.5f, gY+inH*0.5f);
        return gBB;
    }
    ImRect GetGlobalFromP(float inX1, float inY1, float inX2, float inY2){
        ImVec2 wPos = GImGui->CurrentWindow->Pos;
        float gX1 = wPos.x + inX1;
        float gY1 = wPos.y + inY1;
        float gX2 = wPos.x + inX2;
        float gY2 = wPos.y + inY2;
        ImRect gBB = ImRect(gX1,  gY1, gX2, gY2);
        return gBB;
    }
    ImRect GetGlobalBB(float inX, float inY, float inW, float inH){
        ImVec2 wPos = GImGui->CurrentWindow->Pos;
        float gX = wPos.x + inX;
        float gY = wPos.y + inY;
        ImRect gBB = ImRect(gX,  gY, gX + inW, gY + inH);
        return gBB;
    }
    ImRect GetBB(float inX, float inY, float inW, float inH){
        ImRect gBB = ImRect(inX,  inY, inX + inW, inY + inH);
        return gBB;
    }

    ImRect GetBoundingBB(const ImRect & inR, vec2 inRPoint, float rad) {
        // Gets the overall BB when the input BB that is then rotated arounding a rotation point.
        float s=sin(rad), c=cos(rad);        
        auto p0 = UIRotate( inR.GetTL() - inRPoint, c, s ) + inRPoint;
        auto p1 = UIRotate( inR.GetTR() - inRPoint, c, s ) + inRPoint;
        auto p2 = UIRotate( inR.GetBR() - inRPoint, c, s ) + inRPoint;
        auto p3 = UIRotate( inR.GetBL() - inRPoint, c, s ) + inRPoint;

        return ImRect( std::min( { p0.x, p1.x, p2.x, p3.x } ), std::min( { p0.y, p1.y, p2.y, p3.y } ), 
                       std::max( { p0.x, p1.x, p2.x, p3.x } ), std::max( { p0.y, p1.y, p2.y, p3.y } ) );
    }

    ImRect GetArcBoundingBB( vec2 inCenter, float inRadius, double startAng, double endAng ) {
        if ( startAng > endAng ) { std::swap( startAng, endAng ); }

        // Check the center, start, and end points
        vec2 minP = inCenter;
        vec2 maxP = inCenter;
        
        vec2 pnt = inCenter + vec2( cos(startAng), sin(startAng) )*inRadius;
        minP = glm::min( minP, pnt );
        maxP = glm::max( maxP, pnt );
        // UIAddGlobalCircle( pnt.x, pnt.y, 8, 0xFFFF00, true, 1, 32 );

        pnt = inCenter + vec2( cos(endAng), sin(endAng) )*inRadius;
        minP = glm::min( minP, pnt );
        maxP = glm::max( maxP, pnt );
        // UIAddGlobalCircle( pnt.x, pnt.y, 8, 0x00FFFF, true, 1, 32 );

        // Check if 0, PI/2, PI, and 3*PI/2 are in the angle range
        double halfPI = IM_PI * 0.5f;
        int startI = (int) std::ceil( startAng / halfPI );
        int endI = std::min( startI + 3, (int) std::floor( endAng / halfPI ) );
        double ang;
        for ( int i = startI; i <= endI; i++ ) {
            ang = i * halfPI;
            pnt = inCenter + vec2( cos(ang), sin(ang) )*inRadius;
            minP = glm::min( minP, pnt );
            maxP = glm::max( maxP, pnt );
            // UIAddGlobalCircle( pnt.x, pnt.y, 8, 0x00FF00, true, 1, 32 );
        }

        return { minP, maxP };
    }

    ImRect BBToGlobal( const ImRect & inRect ) {
        ImVec2 wPos = GImGui->CurrentWindow->Pos;
        ImRect outRect = inRect;
        outRect.Translate( wPos );
        return outRect;
    }
    ImRect BBToLocal( const ImRect & inRect ) {
        ImVec2 wPos = GImGui->CurrentWindow->Pos;
        ImRect outRect = inRect;
        outRect.Translate( {-wPos.x, -wPos.y} );
        return outRect;
    }

    void CollateBB( ImRect & inRect ) {
        // Flips inverted ImRects
        if ( inRect.Min.x > inRect.Max.x ) { std::swap( inRect.Min.x, inRect.Max.x ); }
        if ( inRect.Min.y > inRect.Max.y ) { std::swap( inRect.Min.y, inRect.Max.y ); }
    }

    glm::vec2 VecToGlobal( const glm::vec2 & inVec ) {
        ImVec2 wPos = GImGui->CurrentWindow->Pos;
        return inVec + wPos;
    }
    glm::vec2 VecToLocal( const glm::vec2 & inVec ) {
        ImVec2 wPos = GImGui->CurrentWindow->Pos;
        return inVec - wPos;
    }
    ImRect RectFromAspect(int XorY, ImRect inAspectRect, ImRect inBaseRect ){

        // given a reference rect, return a new rect where:
        //    - it modifies the BaseRect to match the aspect ration of AspectRect
        //    - it modifies either the X or Y length based on XorY: 0=X, 1=Y
        //    - the ImRect.Min top left cord will remain the same, only the ImRect.Max will change to get the new width/height right
        // this is very useful to have an image match the aspect ratio of pixel cordinates or UV cordinates and not be stretched
        vec2 dim = {inAspectRect.GetWidth(), inAspectRect.GetHeight()};
        float aspect = dim.x/dim.y;
        if(XorY == 0){
            float newW = aspect * inBaseRect.GetHeight();
            inBaseRect.Max.x = inBaseRect.Min.x + newW;
        }
        else{
            float newH = aspect / inBaseRect.GetWidth();
            inBaseRect.Max.y = inBaseRect.Min.y + newH;
        }
        return inBaseRect;
    }
    ImRect RescaleToFitBox(const ImRect & inBox, const ImRect & inContainer ){
        // NOTE: This function DOES NOT center the rescaled box.

        // take inBox and Fit it in the container by scaling down if required
        float deltaX = inBox.GetWidth() - inContainer.GetWidth();
        float deltaY = inBox.GetHeight() - inContainer.GetHeight();
        int xFit = true;if(deltaY > deltaX){xFit = false;}
        float newWidth = inBox.GetWidth();
        float newHeight = inBox.GetHeight();
        float scl = 1;
        if(xFit && deltaX > 0){
            scl = inContainer.GetWidth() / inBox.GetWidth();
            newWidth = inContainer.GetWidth();
            newHeight = newHeight * scl;
        }
        else if(deltaY > 0){
            scl = inContainer.GetHeight() / inBox.GetHeight();
            newHeight = inContainer.GetHeight();
            newWidth = newWidth * scl;
        }
        return { inContainer.Min.x, inContainer.Min.y, inContainer.Min.x+newWidth, inContainer.Min.y+newHeight};

    }
    ImRect RescaleToFitBox(const ImRect & inBox, const ImRect & inGoalBox, float & outScl ){

        float aspect = inBox.GetWidth()/inBox.GetHeight();
        float goalAspect = inGoalBox.GetWidth()/inGoalBox.GetHeight();
        float scl = 1.0f;
        if(aspect <= goalAspect){scl = inGoalBox.GetHeight()/inBox.GetHeight(); }
        else{scl = inGoalBox.GetWidth()/inBox.GetWidth(); }
        outScl = scl;
        ImRect outBox = {0,0,inBox.GetWidth()*scl, inBox.GetHeight()*scl};
        outBox.Translate(inGoalBox.GetCenter() - outBox.GetCenter());
        return outBox;
        
    }

    glm::vec2 UIRotate(const vec2& v, double radians ) {
        // Clockwise rotation
        return vec2(v.x * glm::cos(radians) - v.y * glm::sin(radians), v.x * glm::sin(radians) + v.y * glm::cos(radians)); 
    }

    glm::vec2 UIRotate(const vec2& v, float cos_a, float sin_a) { 
        // Clockwise rotation
        return vec2(v.x * cos_a - v.y * sin_a, v.x * sin_a + v.y * cos_a); 
    }

    glm::vec2 UIRotate(const vec2& v, vec2 inRPoint, double radians ) {
        // Clockwise rotation
        return UIRotate( v - inRPoint, radians ) + inRPoint;
    }
    
    float UIGetVectorAngle( const vec2& v1, const vec2& v2 ) {
        const vec2 nv1 = glm::normalize(v1);
        const vec2 nv2 = glm::normalize(v2); 
        return glm::acos(glm::dot(nv1,nv2));
    }

    double UIGetAngle( vec2 v ) {
        // Returns radians in the x-y plane space where 0 radians is (1,0) and rotation is clockwise.
        //      3/2 PI
        //         |
        // PI ----------- 0
        //         |
        //      1/2 PI
        if      ( v.y == 0 ) { return ( v.x < 0 ) ? M_PI : 0; }
        else if ( v.y  < 0 ) { return 2 * M_PI - glm::acos(glm::dot(glm::normalize(v),{1,0})); }
        else                 { return glm::acos(glm::dot(glm::normalize(v),{1,0})); }
    }

    vec2 UIGetNormal( vec2 v ) {
        v.y *= -1; std::swap( v.x, v.y );
        return v;
    }

    double UICrossProduct(vec2 a, vec2 b){
        return a.x * b.y - b.x * a.y;
    }

    bool UILineSegIntersect( vec2 inP0, vec2 inP1, vec2 inP2, vec2 inP3, vec2 & pOut ) {
        // Gets the intersection of two line segments.
        // Returns true if an intersection is found.
        // Returns false if the lines segments do not intersect.
        float s1_x, s1_y, s2_x, s2_y, s, t;
		s1_x = inP1.x - inP0.x;  s1_y = inP1.y - inP0.y;
		s2_x = inP3.x - inP2.x;  s2_y = inP3.y - inP2.y;
		s = (-s1_y * (inP0.x - inP2.x) + s1_x * (inP0.y - inP2.y)) / (-s2_x * s1_y + s1_x * s2_y);
		t = ( s2_x * (inP0.y - inP2.y) - s2_y * (inP0.x - inP2.x)) / (-s2_x * s1_y + s1_x * s2_y);
		if (s >= 0 && s <= 1 && t >= 0 && t <= 1){ // Collision within line segments
			pOut.x = inP0.x + (t * s1_x);
			pOut.y = inP0.y + (t * s1_y);
			return true;
		}
		return false; // No collision
    }

    ImRect GetSprite(float inX, float inY, float inW, float inH){
        // input sprite in pixel coordinates to turn to uv's later
        ImRect gBB = ImRect(inX,  inY, inX + inW, inY + inH);
        return gBB;
    }
    ImRect VecBB(const vec2& inA, const vec2& inB){
        return ImRect(inA, inB);
    }
    void LogPos(string inTxt, vec2 inPos){
        cout << inTxt << inPos.x << ", " << inPos.y << "\n";
    }
    void LogBB(ImRect inBB, bool inNL){
        if(inNL){
            cout << "min(" << inBB.Min.x <<", "<< inBB.Min.y <<"), max(" << inBB.Max.x <<", "<< inBB.Max.y << ")\n";
        }
        else{
            cout << "min(" << inBB.Min.x <<", "<< inBB.Min.y <<"), max(" << inBB.Max.x <<", "<< inBB.Max.y << ")";
        }
    }

    string FloatToString( float inNum, int inPrecision ) {
        if ( inPrecision < 0 ) { return to_string(inNum);}
        std::ostringstream out; 
        out.precision(inPrecision);
        out << std::fixed << inNum;
        return out.str();
    }

    string VecToString( vec2 inVec ) { return "(" + FloatToString(inVec.x) + "," + FloatToString(inVec.y) + ")"; }

    string VecToString( vec3 inVec ) { return "(" + FloatToString(inVec.x) + "," + FloatToString(inVec.y) + "," + FloatToString(inVec.z) + ")"; }

    string VecToString( vec4 inVec ) { return "(" + FloatToString(inVec.x) + "," + FloatToString(inVec.y) + "," + FloatToString(inVec.z) + "," + FloatToString(inVec.w) + ")"; }

    string RectToString( ImRect inRect ) { return "(" + FloatToString(inRect.Min.x) + "," + FloatToString(inRect.Min.y) + "," + FloatToString(inRect.GetWidth()) + "," + FloatToString(inRect.GetHeight()) + ")"; }
    
    bool GetOverState(const ImRect& inBB){
        return ( inBB.Contains( GetGlobalMousePos() ) );
    }
    bool MouseDown(){ // left mouse button
        InputStateData& m = *UILibIS;
        if(m.Buttons[0].IsPressed == 0){ return false; }
        return true;
    }
    bool AnyMouseDown(){ // true any of the 3 mouse buttons are down, false if none down
        InputStateData& m = *UILibIS;
        if(m.Buttons[0].IsPressed){return true; }
        if(m.Buttons[1].IsPressed){return true; }
        if(m.Buttons[2].IsPressed){return true; }
        return false;
    }

    bool MiddleMouseDown(){
        InputStateData& m = *UILibIS;
        if(m.Buttons[2].IsPressed == 0){ return false; }
        return true;
    }
    bool RightMouseDown(){
        InputStateData& m = *UILibIS;
        if(m.Buttons[1].IsPressed == 0){ return false; }
        return true;
    }
    bool MouseRelease(){
        if(UILibIS->Buttons[0].Change == 2){ return true; }
        return false;
    }
    bool MouseDownOrRelease( int i ) {
        InputStateData& m = *UILibIS;
        return m.Buttons[i].IsPressed || (UILibIS->Buttons[i].Change == 2);
    }
    bool DragStart(){  // left mouse button
        if(UILibIS->Buttons[0].Change == 1){ return true; }
        return false;
    }
    bool DoubleClicked (){
        return UILibIS->DoubleClick == true;
    }
    bool MiddleDragStart(){
        if(UILibIS->Buttons[2].Change == 1){ return true; }
        return false;
    }

    bool RightDragStart(){
        if(UILibIS->Buttons[1].Change == 1){ return true; }
        return false;
    }
    bool DragStartLR(){
        if(UILibIS->Buttons[0].Change == 1){ return true; }
        if(UILibIS->Buttons[1].Change == 1){ return true; }
        return false;
    }
    bool GetActiveState(bool inOver, ImGuiID inId){
        // TO DO: We want to eventually replace this with our version in the future.
        // But we currently don't have an internal UI library focus manager, which is
        // needed to track which UI element is active across frames.
        if(DragStart() && inOver){
            ImGuiWindow* window = GImGui->CurrentWindow;
            ImGui::SetActiveID(inId, window);
            ImGui::SetFocusID(inId, window);
            ImGui::FocusWindow(window);
        }
        if(GImGui->ActiveId == inId ){
            if(MouseDown()){ return true; }
            ImGui::ClearActiveID();
            return false;
        }
        return false;
    }
    void UIDrawAlphaRoundCornerBB(ImRect inRect, unsigned int inC, float inAlpha, float inRounding, std::string cornerBits){
        
        std::bitset<4> bDef(cornerBits);
        unsigned int cornerFlags = bDef.to_ulong();

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImU32 clr = ClWithAlpha(inC, inAlpha);
        drawList->AddRectFilled(inRect.Min, inRect.Max, clr, inRounding, cornerFlags);

    }
    void UIDrawAlphaRoundBB(ImRect inRect, unsigned int inC, float inAlpha, float inRounding){

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImU32 clr = ClWithAlpha(inC, inAlpha);
        drawList->AddRectFilled(inRect.Min, inRect.Max, clr, inRounding, 15);

    }

    void UIDrawAlphaRoundBBRotated(ImRect inRect, unsigned int inC, float inAlpha, float inRounding, vec2 inRPoint, float rad) {
        float s=sin(rad), c=cos(rad);
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        int startI = drawList->VtxBuffer.Size;
        UIDrawAlphaRoundBB( inRect, inC, inAlpha, inRounding );
        auto& buf = drawList->VtxBuffer;
        for (int i = startI; i < buf.Size; i++) {
            buf[i].pos = UIRotate(buf[i].pos-inRPoint, c, s) + inRPoint;
        }
    }

    void UIDrawRoundBB(ImRect inRect, unsigned int inC, float inRounding){
        
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImU32 clr = Cl( inC ); 
        drawList->AddRectFilled(inRect.Min, inRect.Max, clr, inRounding, 15);
        //drawList->AddRectFilled(p1, p2, clr, inRounding, 15);
    }
    void UIDrawRoundBBOutlineCorner(ImRect inRect, unsigned int inC, float inRounding, float inThickness, std::string cornerBits){
        
        std::bitset<4> bDef(cornerBits);
        unsigned int cornerFlags = bDef.to_ulong();
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImU32 clr = Cl( inC ); 
        drawList->AddRect(inRect.Min, inRect.Max, clr, inRounding, cornerFlags, inThickness);

    }
    void UIDrawRoundBBOutline(ImRect inRect, unsigned int inC, float inRounding, float inThickness){

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImU32 clr = Cl( inC ); 
        drawList->AddRect(inRect.Min, inRect.Max, clr, inRounding, 15, inThickness);
        
    }

    void UIDrawRoundBBOutlineRotated(ImRect inRect, unsigned int inC, float inRounding, float inThickness, vec2 inRPoint, float rad) {
        float s=sin(rad), c=cos(rad);
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        int startI = drawList->VtxBuffer.Size;
        UIDrawRoundBBOutline( inRect, inC, inRounding, inThickness );
        auto& buf = drawList->VtxBuffer;
        for (int i = startI; i < buf.Size; i++) {
            buf[i].pos = UIRotate(buf[i].pos-inRPoint, c, s) + inRPoint;
        }
    }

    void UIDashRect(ImRect inRect, unsigned int inC, float inThickness){
        
        vec2 p1 = {inRect.Min.x,inRect.Min.y};
        vec2 p2 = {inRect.Max.x,inRect.Min.y};
        vec2 p3 = {inRect.Max.x,inRect.Max.y};
        vec2 p4 = {inRect.Min.x,inRect.Max.y};
        float offset = 0;float dash = 5;float gap = 5;
        UIDashedLine(p1, p2, dash, gap, offset, inC, inThickness);
        UIDashedLine(p2, p3, dash, gap, offset, inC, inThickness);
        UIDashedLine(p3, p4, dash, gap, offset, inC, inThickness);
        UIDashedLine(p4, p1, dash, gap, offset, inC, inThickness);
        
    }
    void UIDashRectAni(ImRect inRect, unsigned int inC, float inThickness, float inDash, float inGap){ // animated dashed rectangle

        vec2 p1 = {inRect.Min.x,inRect.Min.y};
        vec2 p2 = {inRect.Max.x,inRect.Min.y};
        vec2 p3 = {inRect.Max.x,inRect.Max.y};
        vec2 p4 = {inRect.Min.x,inRect.Max.y};
        static auto lastTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float timeSpan = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
        int timeCounts = (int)floor(timeSpan / 0.0065f);
        int speed = 60;
        int modCount = timeCounts % speed;
        float loopPerc = (float)modCount / (float)speed;
        UIDashedLine(p1, p2, inDash, inGap, loopPerc, inC, inThickness);
        UIDashedLine(p2, p3, inDash, inGap, loopPerc, inC, inThickness);
        UIDashedLine(p3, p4, inDash, inGap, loopPerc, inC, inThickness);
        UIDashedLine(p4, p1, inDash, inGap, loopPerc, inC, inThickness);
    }
    void UIDashedLine(vec2 &inP1, vec2 &inP2, float inDashLen, float inGapLen, float inOffset, unsigned int inC, float inThickness){
        // inOffset is the dash shift percentage between 0 - 1
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        float stride = inDashLen + inGapLen;
        vec2 lineV = inP2 - inP1;
        vec2 nLine = normalize( lineV);
        vec2 stepBack = nLine * inThickness * -.5f;
        vec2  strideV = nLine * stride;
        vec2 dashV = nLine * inDashLen;
        float len = length(lineV) + inThickness;
        int nbSeg = int(len / stride);
        vec2 start = inP1  + stepBack;
        vec2 pos = start + strideV * inOffset;
        float lenSum = stride * inOffset;
        float shiftLen = inOffset*stride;
        float frontFill = shiftLen > inGapLen;
        vec2 dashE;
        for(int i=0;i<nbSeg;i++){
            if(i==0 && frontFill){
                float fillSize = shiftLen - inGapLen;
                vec2 fillEnd = start + nLine * fillSize;
                UIAddLine(start.x, start.y, fillEnd.x, fillEnd.y, inC, inThickness);
            }  
            if(i==nbSeg-1){
                float clipLen = len - lenSum;
                if(clipLen > inDashLen){clipLen = inDashLen;}
                vec2 clipPos = nLine * clipLen + pos; 
                UIAddLine(pos.x, pos.y, clipPos.x, clipPos.y, inC, inThickness);
            }
            else{
                dashE = pos + dashV;
                UIAddLine(pos.x, pos.y, dashE.x, dashE.y, inC, inThickness);
                pos += strideV;
                lenSum += stride;
            }
        }
    }
    void UIDrawBBLocalOutline(ImRect inRect, unsigned int inC, float inThickness){
        
        ImVec2 wPos = GImGui->CurrentWindow->Pos;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImU32 clr = Cl( inC );
        drawList->AddRect(inRect.Min+wPos, inRect.Max+wPos, clr, 0, 0, inThickness);

    }
    void UIDrawBBOutline(ImRect inRect, unsigned int inC, float inThickness){
        
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImU32 clr = Cl( inC );
        drawList->AddRect(inRect.Min, inRect.Max, clr, 0, 0, inThickness);

    }

    void UIDrawAlphaBBOutline(ImRect inRect, unsigned int inC, float inThickness, float inAlpha) {

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImU32 clr = UILib::ClWithAlpha( inC, inAlpha );
        drawList->AddRect(inRect.Min, inRect.Max, clr, 0, 0, inThickness);

    }

    ImU32 ClWithAlpha(unsigned int hexValue, float inAlpha){
        // inAlpha is 0 to 1
        double cr = ((hexValue >> 16) & 0xFF);  // Extract the RR byte
        double cg = ((hexValue >> 8) & 0xFF);   // Extract the GG byte
        double cb = ((hexValue) & 0xFF);        // Extract the BB byte
        //ImU32 clr = IM_COL32( cr, cg, cb, 0);

        uint alpha = (int)round(inAlpha * 255);
        ImU32 clr = IM_COL32( cr, cg, cb, alpha);

        return clr;
    }

    int UIGetBlendClr( const vector<int> & inClrs, float inPct ) {
        assert( inClrs.size() >= 1 );
        if ( inClrs.size() == 1 ) { return inClrs[0]; }
        else if ( inPct <= 0.0f ) { return inClrs.front(); }
        else if ( inPct >= 1.0f ) { return inClrs.back(); }
        float incr = 1.0f / (float) (inClrs.size()-1);
        int nbIncr = (int) inClrs.size() - 1;
        for ( int i = 0; i < nbIncr; i++ ) {
            if ( inPct > i*incr && inPct <= (i+1)*incr ) {
                return RGBtoInt( glm::mix( HexToRGBVec( inClrs[i] ), HexToRGBVec( inClrs[i+1] ), ( inPct - i*incr ) / incr ) );
            }
        }
        return inClrs.back();
    }

    float UIGetBlendVal( const vector<float> & inVals, float inPct ) {
        assert( inVals.size() >= 1 );
        if ( inVals.size() == 1 ) { return inVals[0]; }
        else if ( inPct <= 0.0f ) { return inVals.front(); }
        else if ( inPct >= 1.0f ) { return inVals.back(); }
        float incr = 1.0f / (float) (inVals.size()-1);
        int nbIncr = (int) inVals.size() - 1;
        for ( int i = 0; i < nbIncr; i++ ) {
            if ( inPct > i*incr && inPct <= (i+1)*incr ) {
                return glm::mix( inVals[i], inVals[i+1], ( inPct - i*incr ) / incr );
            }
        }
        return inVals.back();
    }

    void UIDrawAlphaBB(ImRect inRect, unsigned int inC, float inAlpha){


        //UIDrawAlphaBB
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        //ImU32 clr = Cl( inC ); 
        ImU32 clr = ClWithAlpha(inC, inAlpha);
        
        drawList->AddRectFilled(inRect.Min, inRect.Max, clr);

    }

    void UIDrawAlphaBBRotated(ImRect inRect, unsigned int inC, float inAlpha, vec2 inRPoint, float rad ) {
        float s=sin(rad), c=cos(rad);
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        int startI = drawList->VtxBuffer.Size;
        ImU32 clr = ClWithAlpha(inC, inAlpha);
        drawList->AddRectFilled(inRect.Min, inRect.Max, clr);
        auto& buf = drawList->VtxBuffer;
        for (int i = startI; i < buf.Size; i++) {
            buf[i].pos = UIRotate(buf[i].pos-inRPoint, c, s) + inRPoint;
        }
    }

    void UIDrawMultiColorBB(ImRect inRect, std::vector<int> inC, std::vector<float> inAlpha){
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImU32 clr1 = ClWithAlpha(inC[0],inAlpha[0]);
        ImU32 clr2 = ClWithAlpha(inC[1],inAlpha[1]);
        ImU32 clr3 = ClWithAlpha(inC[2],inAlpha[2]);
        ImU32 clr4 = ClWithAlpha(inC[3],inAlpha[3]);
        drawList->AddRectFilledMultiColor(inRect.Min,inRect.Max,clr1,clr2,clr3,clr4);
    }
    void UIDrawBB(ImRect inRect, unsigned int inC){
        
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImU32 clr = Cl( inC ); 
        drawList->AddRectFilled(inRect.Min, inRect.Max, clr);

    }

    void UIDrawAlphaBBWithCutout( ImRect inR, unsigned int inC, float inAlpha, ImRect cutR ) {
        vector<vec2> pnts = {};
        if ( !inR.Overlaps( cutR ) ){           // no overlap
            UIDrawAlphaBB(inR, inC, inAlpha);
        }
        else if ( cutR.Contains( inR ) ) {      // all cut-out
        }
        else if ( cutR.Min.x - inR.Min.x <  0.01f && cutR.Min.y - inR.Min.y <  0.01f ){ // top left
            if      ( cutR.Max.x - inR.Max.x > -0.01f ) { // all top
                inR.Min.y = cutR.Max.y; UIDrawAlphaBB(inR, inC, inAlpha);
            }
            else if ( cutR.Max.y - inR.Max.y > -0.01f ) { // all left
                inR.Min.x = cutR.Max.x; UIDrawAlphaBB(inR, inC, inAlpha);
            }
            else {
                pnts = { {cutR.Max.x, cutR.Max.y}, {cutR.Max.x, inR.Min.y}, {inR.Max.x, inR.Min.y}, {inR.Max.x, inR.Max.y}, {inR.Min.x, inR.Max.y}, {inR.Min.x, cutR.Max.y} };
            }
        }
        else if ( cutR.Max.x - inR.Max.x > -0.01f && cutR.Min.y - inR.Min.y <  0.01f ){ // top right
            pnts = { {cutR.Min.x, cutR.Max.y}, {inR.Max.x, cutR.Max.y}, {inR.Max.x, inR.Max.y}, {inR.Min.x, inR.Max.y}, {inR.Min.x, inR.Min.y}, {cutR.Min.x, inR.Min.y}};
        }
        else if ( cutR.Min.x - inR.Min.x <  0.01f && cutR.Max.y - inR.Max.y > -0.01f ){ // bottom left
            pnts = { {cutR.Max.x, cutR.Min.y}, {inR.Min.x, cutR.Min.y}, {inR.Min.x, inR.Min.y}, {inR.Max.x, inR.Min.y}, {inR.Max.x, inR.Max.y}, {cutR.Max.x, inR.Max.y} };
        }
        else if ( cutR.Max.x - inR.Max.x > -0.01f && cutR.Max.y - inR.Max.y > -0.01f ){ // bottom right
            if      ( cutR.Min.x - inR.Min.x < 0.01f ) { // all bottom
                inR.Max.y = cutR.Min.y; UIDrawAlphaBB(inR, inC, inAlpha);
            }
            else if ( cutR.Min.y - inR.Min.y < 0.01f ) { // all right
                inR.Max.x = cutR.Min.x; UIDrawAlphaBB(inR, inC, inAlpha);
            }
            else {
                pnts = { {cutR.Min.x, cutR.Min.y}, {cutR.Min.x, inR.Max.y}, {inR.Min.x, inR.Max.y}, {inR.Min.x, inR.Min.y}, {inR.Max.x, inR.Min.y}, {inR.Max.x, cutR.Min.y} };
            }
        }
        // UILib uses ImGui's polygon fill. However, since ImGui does not support a concave polygon fill, we break up into multiple rectangles here.
        else if ( cutR.Min.x - inR.Min.x <  0.01f ) {                      // left border
            ImRect fr = ImRect( inR.Min.x, inR.Min.y, inR.Max.x, cutR.Min.y ); UIDrawAlphaBB(fr, inC, inAlpha);
            fr = ImRect( cutR.Max.x, cutR.Min.y, inR.Max.x, cutR.Max.y ); UIDrawAlphaBB(fr, inC, inAlpha);
            fr = ImRect( inR.Min.x, cutR.Max.y, inR.Max.x, inR.Max.y ); UIDrawAlphaBB(fr, inC, inAlpha);

        }
        else if ( cutR.Max.x - inR.Max.x > -0.01f ) {                      // right border
            ImRect fr = ImRect( inR.Min.x, inR.Min.y, inR.Max.x, cutR.Min.y ); UIDrawAlphaBB(fr, inC, inAlpha);
            fr = ImRect( inR.Min.x, cutR.Min.y, cutR.Min.x, cutR.Max.y ); UIDrawAlphaBB(fr, inC, inAlpha);
            fr = ImRect( inR.Min.x, cutR.Max.y, inR.Max.x, inR.Max.y ); UIDrawAlphaBB(fr, inC, inAlpha);                        
        }
        else if ( cutR.Min.y - inR.Min.y <  0.01f ) {                      // top border
            ImRect fr = ImRect( inR.Min.x, inR.Min.y, cutR.Min.x, inR.Max.y ); UIDrawAlphaBB(fr, inC, inAlpha);
            fr = ImRect( cutR.Min.x, cutR.Max.y, cutR.Max.x, inR.Max.y ); UIDrawAlphaBB(fr, inC, inAlpha);
            fr = ImRect( cutR.Max.x, inR.Min.y, inR.Max.x, inR.Max.y ); UIDrawAlphaBB(fr, inC, inAlpha);
        }
        else if ( cutR.Max.y - inR.Max.y > -0.01f ) {                      // bottom border
            ImRect fr = ImRect( inR.Min.x, inR.Min.y, cutR.Min.x, inR.Max.y ); UIDrawAlphaBB(fr, inC, inAlpha);
            fr = ImRect( cutR.Min.x, inR.Min.y, cutR.Max.x, cutR.Min.y ); UIDrawAlphaBB(fr, inC, inAlpha);
            fr = ImRect( cutR.Max.x, inR.Min.y, inR.Max.x, inR.Max.y ); UIDrawAlphaBB(fr, inC, inAlpha);
        }
        else {                                                                        // cut-out cell within
            ImRect fr = ImRect( inR.Min.x, inR.Min.y, cutR.Min.x, inR.Max.y ); UIDrawAlphaBB(fr, inC, inAlpha);
            fr = ImRect( cutR.Min.x, inR.Min.y, cutR.Max.x, cutR.Min.y ); UIDrawAlphaBB(fr, inC, inAlpha);
            fr = ImRect( cutR.Min.x, cutR.Max.y, cutR.Max.x, inR.Max.y ); UIDrawAlphaBB(fr, inC, inAlpha);
            fr = ImRect( cutR.Max.x, inR.Min.y, inR.Max.x, inR.Max.y ); UIDrawAlphaBB(fr, inC, inAlpha);
        }       
        if ( pnts.size() > 0 ) { UIAddFilledAlphaPolyG(pnts, inC, inAlpha); }
    }


    void UICheckBox(float inX, float inY, float inWH, int &inChecked, int inClr){
        
        ImVec2 wPos = GImGui->CurrentWindow->Pos;
        ImRect cRect = GetGlobalBB(inX, inY, inWH, inWH);
        UIDrawBBOutline(cRect, 0xABABAB, 1);
        float thickness = ImMax(inWH / 5.0f, 1.0f);
        inWH -= thickness*0.5f;
        if(inChecked){
            float sz = inWH - 2;
            ImVec2 pos = {inX-1 + wPos.x , inY-1 + wPos.y};
            pos += ImVec2(thickness*0.25f, thickness*0.25f);
            float third = sz / 3.0f;
            float bx = 2 + pos.x + third;
            float by = 1 + pos.y + sz - third*0.5f;
            ImU32 cClr = UILib::Cl( inClr );
            ImGuiContext& g = *GImGui;
            ImGuiWindow* window = g.CurrentWindow;
            window->DrawList->PathLineTo(ImVec2(bx - third, by - third));
            window->DrawList->PathLineTo(ImVec2(bx, by));
            window->DrawList->PathLineTo(ImVec2(bx + third*2, by - third*2));
            window->DrawList->PathStroke(cClr, false, thickness);
        }
        
    }

    void UITransformBB( ImRect & inRect, UITransformBBData & inTD, const ImRect & boundingRect, bool inMute, bool MuteSize, bool MuteDrag ) {

        vec2 mPos = GetMousePos();
        
        using ts = UITransformBBData::TransformStatus;
        using tt = UITransformBBData::TransformType;

        if ( inTD.Status == ts::ts_click || inTD.Status == ts::ts_down ) {  // click or mouse down statuses
            if ( MouseDown() ) { 
                inTD.Status = ts::ts_down; 
            }
            else { inTD.Status = ts::ts_release; }
        }
        else {                                                              // none, over, or release statuses
            ImRect clickBB = inRect; if ( !MuteSize ) { clickBB.Expand(10); }
            if ( !inMute && VecInBB( mPos, clickBB ) ) {
                if ( !MuteSize ) {
                    float rsSize = 10;
                    float pcIn = 0.2f;
                    vec2 rDim = { inRect.GetWidth(), inRect.GetHeight() };
                    ImRect tlBB = GetBB(inRect.Min.x-rsSize, inRect.Min.y-rsSize, rsSize*(1+pcIn), rsSize*(1+pcIn) );
                    ImRect brBB = GetBB(inRect.Min.x+rDim.x-rsSize*pcIn, inRect.Min.y+rDim.y-rsSize*pcIn, rsSize*(1+pcIn), rsSize*(1+pcIn) );
                    ImRect trBB = GetBB(inRect.Min.x+rDim.x-rsSize*pcIn, inRect.Min.y-rsSize, rsSize*(1+pcIn), rsSize*(1+pcIn) );
                    ImRect blBB = GetBB(inRect.Min.x-rsSize, inRect.Min.y+rDim.y-rsSize*pcIn, rsSize*(1+pcIn), rsSize*(1+pcIn) );
                    ImRect lBB = GetBB(inRect.Min.x-rsSize, inRect.Min.y+rsSize*pcIn, rsSize*(1+pcIn), rDim.y-2*rsSize*pcIn );
                    ImRect rBB = GetBB(inRect.Min.x+rDim.x-rsSize*pcIn, inRect.Min.y+rsSize*pcIn, rsSize*(1+pcIn), rDim.y-2*rsSize*pcIn );
                    ImRect tBB = GetBB(inRect.Min.x+rsSize*pcIn, inRect.Min.y-rsSize, rDim.x-2*rsSize*pcIn, rsSize*(1+pcIn) );
                    ImRect bBB = GetBB(inRect.Min.x+rsSize*pcIn, inRect.Min.y+rDim.y-rsSize*pcIn, rDim.x-2*rsSize*pcIn, rsSize*(1+pcIn) );
                    if ( VecInBB(mPos, tlBB ) && !MuteSize ) {
                        inTD.Type = tt::tt_resize_tl;
                        inTD.Status = DragStart() ? ts::ts_click : inTD.Status = ts::ts_over;  
                    }
                    else if ( VecInBB(mPos, brBB ) ) {
                        inTD.Type = tt::tt_resize_br;
                        inTD.Status = DragStart() ? ts::ts_click : inTD.Status = ts::ts_over;  
                    }
                    else if ( VecInBB(mPos, trBB ) ) {
                        inTD.Type = tt::tt_resize_tr;
                        inTD.Status = DragStart() ? ts::ts_click : inTD.Status = ts::ts_over;  
                    }
                    else if ( VecInBB(mPos, blBB ) ) {
                        inTD.Type = tt::tt_resize_bl;
                        inTD.Status = DragStart() ? ts::ts_click : inTD.Status = ts::ts_over;  
                    }
                    else if ( VecInBB(mPos, lBB ) ) {
                        inTD.Type = tt::tt_resize_l;
                        inTD.Status = DragStart() ? ts::ts_click : inTD.Status = ts::ts_over;  
                    }
                    else if ( VecInBB(mPos, rBB ) ) {
                        inTD.Type = tt::tt_resize_r;
                        inTD.Status = DragStart() ? ts::ts_click : inTD.Status = ts::ts_over;  
                    }
                    else if ( VecInBB(mPos, tBB ) ) {
                        inTD.Type = tt::tt_resize_t;
                        inTD.Status = DragStart() ? ts::ts_click : inTD.Status = ts::ts_over;  
                    }
                    else if ( VecInBB(mPos, bBB ) ) {
                        inTD.Type = tt::tt_resize_b;
                        inTD.Status = DragStart() ? ts::ts_click : inTD.Status = ts::ts_over;  
                    }
                    else if ( !MuteDrag && VecInBB(mPos, inRect ) ) {
                        inTD.Type = tt::tt_drag;
                        if ( DragStart() ) { 
                            inTD.Status = ts::ts_click;
                            inTD.Offset = { mPos.x - inRect.Min.x, mPos.y - inRect.Min.y }; 
                        }
                        else { inTD.Status = ts::ts_over; }
                    }
                    else {
                        inTD.Status = ts::ts_none; inTD.Type = tt::tt_none;
                    }
                }
                else if ( !MuteDrag && VecInBB(mPos, inRect ) ) {
                    inTD.Type = tt::tt_drag;
                    if ( DragStart() ) { 
                        inTD.Status = ts::ts_click;
                        inTD.Offset = { mPos.x - inRect.Min.x, mPos.y - inRect.Min.y }; 
                    }
                    else { inTD.Status = ts::ts_over; }
                }
                else {
                    inTD.Status = ts::ts_none; inTD.Type = tt::tt_none;
                }
            }
            else {
                inTD.Status = ts::ts_none; inTD.Type = tt::tt_none;
            }
            
        }

        // std::cout << "Status: " << inTD.Status << ", Type: " << inTD.Type << std::endl;

        if ( inTD.Status != ts::ts_click && inTD.Status != ts::ts_down ) { return; }

        vec2 mDelta = GetMouseDelta();
        vec2 rDim = { inRect.GetWidth(), inRect.GetHeight() };
        vec2 minDim = vec2(5);
        if ( inTD.Type == tt::tt_resize_tl && !MuteSize) {
            vec2 newPos = inRect.Min + mDelta;
            vec2 newDim = rDim - mDelta;
            if      ( mDelta.x < 0 && mPos.x > inRect.Min.x     ) { newPos.x = inRect.Min.x; newDim.x = rDim.x; }
            else if ( mDelta.x > 0 && mPos.x < inRect.Min.x     ) { newPos.x = inRect.Min.x; newDim.x = rDim.x; }
            else if ( mPos.x > inRect.Min.x + rDim.x - minDim.x ) { newPos.x = inRect.Min.x; newDim.x = rDim.x; }
            if      ( mDelta.y < 0 && mPos.y > inRect.Min.y     ) { newPos.y = inRect.Min.y; newDim.y = rDim.y; }
            else if ( mDelta.y > 0 && mPos.y < inRect.Min.y     ) { newPos.y = inRect.Min.y; newDim.y = rDim.y; }
            else if ( mPos.y > inRect.Min.y + rDim.y - minDim.y ) { newPos.y = inRect.Min.y; newDim.y = rDim.y; }                   
            inRect = GetBB(newPos.x, newPos.y, newDim.x, newDim.y );
            if ( inRect.Min.x < boundingRect.Min.x ) { inRect.Min.x = boundingRect.Min.x; }
            if ( inRect.Min.y < boundingRect.Min.y ) { inRect.Min.y = boundingRect.Min.y; }
        }
        else if ( inTD.Type == tt::tt_resize_tr && !MuteSize) {
            vec2 newPos = inRect.Min + vec2( 0, mDelta.y );
            vec2 newDim = rDim + vec2( mDelta.x, -mDelta.y );
            if      ( mDelta.x > 0 && mPos.x < inRect.Min.x + rDim.x ) {                          newDim.x = rDim.x; }
            else if ( mDelta.x < 0 && mPos.x > inRect.Min.x + rDim.x ) {                          newDim.x = rDim.x; }
            else if ( mPos.x < inRect.Min.x + minDim.x               ) {                          newDim.x = rDim.x; }
            if      ( mDelta.y < 0 && mPos.y > inRect.Min.y          ) { newPos.y = inRect.Min.y; newDim.y = rDim.y; }
            else if ( mDelta.y > 0 && mPos.y < inRect.Min.y          ) { newPos.y = inRect.Min.y; newDim.y = rDim.y; }
            else if ( mPos.y > inRect.Min.y + rDim.y - minDim.y      ) { newPos.y = inRect.Min.y; newDim.y = rDim.y; }
            inRect = GetBB(newPos.x, newPos.y, newDim.x, newDim.y );
            if ( inRect.Max.x > boundingRect.Max.x ) { inRect.Max.x = boundingRect.Max.x; }
            if ( inRect.Min.y < boundingRect.Min.y ) { inRect.Min.y = boundingRect.Min.y; }
        }
        else if ( inTD.Type == tt::tt_resize_bl && !MuteSize) {
            vec2 newPos = inRect.Min + vec2( mDelta.x, 0 );
            vec2 newDim = rDim + vec2( -mDelta.x, mDelta.y );
            if      ( mDelta.x < 0 && mPos.x > inRect.Min.x          ) { newPos.x = inRect.Min.x; newDim.x = rDim.x; }
            else if ( mDelta.x > 0 && mPos.x < inRect.Min.x          ) { newPos.x = inRect.Min.x; newDim.x = rDim.x; }
            else if ( mPos.x > inRect.Min.x + rDim.x - minDim.x      ) { newPos.x = inRect.Min.x; newDim.x = rDim.x; }
            if      ( mDelta.y > 0 && mPos.y < inRect.Min.y + rDim.y ) {                          newDim.y = rDim.y; }
            else if ( mDelta.y < 0 && mPos.y > inRect.Min.y + rDim.y ) {                          newDim.y = rDim.y; }
            else if ( mPos.y < inRect.Min.y + minDim.y               ) {                          newDim.y = rDim.y; }
            inRect = GetBB(newPos.x, newPos.y, newDim.x, newDim.y );
            if ( inRect.Min.x < boundingRect.Min.x ) { inRect.Min.x = boundingRect.Min.x; }
            if ( inRect.Max.y > boundingRect.Max.y ) { inRect.Max.y = boundingRect.Max.y; }
        }
        else if ( inTD.Type == tt::tt_resize_br && !MuteSize) {
            vec2 newPos = inRect.Min;
            vec2 newDim = rDim + mDelta;
            if      ( mDelta.x > 0 && mPos.x < inRect.Min.x + rDim.x ) { newDim.x = rDim.x; }
            else if ( mDelta.x < 0 && mPos.x > inRect.Min.x + rDim.x ) { newDim.x = rDim.x; }
            else if ( mPos.x < inRect.Min.x + minDim.x               ) { newDim.x = rDim.x; }
            if      ( mDelta.y > 0 && mPos.y < inRect.Min.y + rDim.y ) { newDim.y = rDim.y; }
            else if ( mDelta.y < 0 && mPos.y > inRect.Min.y + rDim.y ) { newDim.y = rDim.y; }
            else if ( mPos.y < inRect.Min.y + minDim.y               ) { newDim.y = rDim.y; }
            inRect = GetBB(newPos.x, newPos.y, newDim.x, newDim.y );
            if ( inRect.Max.x > boundingRect.Max.x ) { inRect.Max.x = boundingRect.Max.x; }
            if ( inRect.Max.y > boundingRect.Max.y ) { inRect.Max.y = boundingRect.Max.y; }
        }
        else if ( inTD.Type == tt::tt_resize_l && !MuteSize) {
            vec2 newPos = inRect.Min + vec2( mDelta.x, 0 );
            vec2 newDim = rDim + vec2( -mDelta.x, 0 );
            if      ( mDelta.x < 0 && mPos.x > inRect.Min.x     ) { newPos.x = inRect.Min.x; newDim.x = rDim.x; }
            else if ( mDelta.x > 0 && mPos.x < inRect.Min.x     ) { newPos.x = inRect.Min.x; newDim.x = rDim.x; }
            else if ( mPos.x > inRect.Min.x + rDim.x - minDim.x ) { newPos.x = inRect.Min.x; newDim.x = rDim.x; }                    
            inRect = GetBB(newPos.x, newPos.y, newDim.x, newDim.y );
            if ( inRect.Min.x < boundingRect.Min.x ) { inRect.Min.x = boundingRect.Min.x; }
        }
        else if ( inTD.Type == tt::tt_resize_r && !MuteSize) {
            vec2 newPos = inRect.Min;
            vec2 newDim = rDim + vec2( mDelta.x, 0 );
            if      ( mDelta.x > 0 && mPos.x < inRect.Min.x + rDim.x ) { newDim.x = rDim.x; }
            else if ( mDelta.x < 0 && mPos.x > inRect.Min.x + rDim.x ) { newDim.x = rDim.x; }
            else if ( mPos.x < inRect.Min.x + minDim.x               ) { newDim.x = rDim.x; }
            inRect = GetBB(newPos.x, newPos.y, newDim.x, newDim.y );
            if ( inRect.Max.x > boundingRect.Max.x ) { inRect.Max.x = boundingRect.Max.x; }
        }
        else if ( inTD.Type == tt::tt_resize_t && !MuteSize) {
            vec2 newPos = inRect.Min + vec2( 0, mDelta.y );
            vec2 newDim = rDim + vec2( 0, -mDelta.y );
            if      ( mDelta.y < 0 && mPos.y > inRect.Min.y     ) { newPos.y = inRect.Min.y; newDim.y = rDim.y; }
            else if ( mDelta.y > 0 && mPos.y < inRect.Min.y     ) { newPos.y = inRect.Min.y; newDim.y = rDim.y; }
            else if ( mPos.y > inRect.Min.y + rDim.y - minDim.y ) { newPos.y = inRect.Min.y; newDim.y = rDim.y; }
            inRect = GetBB(newPos.x, newPos.y, newDim.x, newDim.y );
            if ( inRect.Min.y < boundingRect.Min.y ) { inRect.Min.y = boundingRect.Min.y; }
        }
        else if ( inTD.Type == tt::tt_resize_b && !MuteSize) {
            vec2 newPos = inRect.Min;
            vec2 newDim = rDim + vec2( 0, mDelta.y );
            if      ( mDelta.y > 0 && mPos.y < inRect.Min.y + rDim.y ) { newDim.y = rDim.y; }
            else if ( mDelta.y < 0 && mPos.y > inRect.Min.y + rDim.y ) { newDim.y = rDim.y; }
            else if ( mPos.y < inRect.Min.y + minDim.y               ) { newDim.y = rDim.y; }                    
            inRect = GetBB(newPos.x, newPos.y, newDim.x, newDim.y );
            if ( inRect.Max.y > boundingRect.Max.y ) { inRect.Max.y = boundingRect.Max.y; }
        }
        else if ( inTD.Type == tt::tt_drag && !MuteDrag ) {
            vec2 winOffset = { inRect.GetWidth()*inTD.BoundingPct.x, inRect.GetHeight()*inTD.BoundingPct.y };
            ImRect winBound = { boundingRect.Min - winOffset, boundingRect.Max + winOffset };
            vec2 newPos = { mPos.x - inTD.Offset.x, mPos.y - inTD.Offset.y };
            inRect = GetBB(newPos.x, newPos.y, inRect.GetWidth(), inRect.GetHeight() );
            FitBBinBB( inRect, winBound );
        }

    }


    void UIDrawComboBox(UIComboBox * inCombo, ImRect inRect, vector<string> &inItems ){

        if ( UIComboGetFocusedName() == inCombo->Name && inCombo->Open == false ) {
            UIComboExitFocus();
        }
        
        // inRect BB is in Global Coordinates
        vec2 mPos = GetGlobalMousePos();
        if(DragStart()){
            if( VecInBB(mPos, inRect) ){ 
                float cellH = 21, maxH = 150;
                float cbHeight = std::min(inItems.size() * cellH, maxH);
                ImRect listBB = GetBB(inRect.Min.x,inRect.Max.y-1,inRect.GetWidth(),cbHeight);
                UIComboInitFocus( 1, inCombo, listBB, inItems );
            }
            else if ( UIComboGetFocusedName() == inCombo->Name ) {
                float cellH = 21, maxH = 150;
                float cbHeight = std::min(inItems.size() * cellH, maxH);
                ImRect listBB = GetBB(inRect.Min.x,inRect.Max.y-1,inRect.GetWidth(),cbHeight);
                if( !VecInBB(mPos, listBB) ){  UIComboExitFocus(); }
            }
        }
        
        UIDrawBB(inRect, 0xFFFFFF);
        if(inCombo->Index > -1){
            string comboString = inItems[inCombo->Index];
            float tx = inRect.Min.x + 5;
            float tw = inRect.GetWidth() - 15;
            ImRect txtBB = GetBB(inRect.Min.x + 5, inRect.Min.y, inRect.GetWidth() - 10, inRect.GetHeight() );
            UIAddTextWithFontBB(txtBB, comboString, 0x000000, UILibCTX->DefaultFont, {0, 0.5} );
        }
        
        if(inCombo->Open == 1){UIDrawBBOutline(inRect, 0x2BA9E0, 1.0f);}
        else{UIDrawBBOutline(inRect, 0xABABAB, 1.0f);}
        
        if(inCombo->Open == 0 || UILibCTX->ComboMode == 0){
            UIAddTriangleGlobal( inRect.Min.x + inRect.GetWidth()-12, inRect.Min.y + inRect.GetHeight() * 0.5f, 4.5, true, 0x898989);
        }

    }

    void UIDrawComboItems(UIComboBox * inCombo, ImRect inRect, vector<string> &inItems, float &inScrollPerc, bool &inDragging){
        if ( inCombo == nullptr ) { return; }
        float maxH = 150;
        
        UIDrawBB(inRect, 0xFFFFFF);
        UIDrawBBOutline(inRect, 0xABABAB, 1.0f);
        vec2 mPos = GetGlobalMousePos();
        ImGui::PushClipRect(inRect.Min, inRect.Max, true);

        float viewH = inRect.GetHeight();
        float spacer = 5;
        float xPos = inRect.Min.x + spacer;
        float yPos = inRect.Min.y;
        float cellW = inRect.GetWidth() - spacer;
        float cellH = 21;

        int nbRows = (int)inItems.size();
        float contentHeight = (nbRows-1) * cellH + cellH * .5f + 4;
        float leftOverHeight = contentHeight - viewH;
        bool yScrollOn = true;
        int RowsPerBox = (int)( viewH / cellH);

        if(viewH < maxH){
            RowsPerBox = (int)nbRows-1;
            yScrollOn = false;
        }
        
        float viewOffsetY = 0;
        int currentRow = 0;
        float TrackW = 10;
        float offScreenHeight = 0;
        float yShift = 0;

        if(yScrollOn){
            offScreenHeight = contentHeight - viewH;
            viewOffsetY = inScrollPerc * offScreenHeight;
            currentRow = (int)floor( viewOffsetY / cellH);
            float nbOffScreenTop = floor( viewOffsetY / cellH );
            yShift = nbOffScreenTop * cellH - viewOffsetY;
            cellW -= TrackW;
            UIAddScrollerV(inRect.Max.x - TrackW, yPos, TrackW, viewH, contentHeight, &inScrollPerc, &inDragging, UILibCTX->DefaultScroller);
        }
        
        cellW = inRect.GetWidth() ;
        if(yScrollOn){cellW -= TrackW;}

        for(int i=0;i<=RowsPerBox;i++){
            int rowIndex = currentRow + i;
            ImRect cell = GetGlobalBB(inRect.Min.x, yPos + yShift, cellW , cellH-1);
            if( VecInBB(mPos, cell) ){
                UIDrawBB(cell, 0x2EAADC);
                UIAddTextWithFont(xPos, yPos+ yShift, cellW, cellH, inItems[rowIndex], 0xFFFFFF, UILibCTX->DefaultFontB, {0,.5} );
                if( DragStart() ){
                    inCombo->Index = rowIndex;
                    inCombo->Open = false;
                }
            }
            else{
                UIAddTextString(xPos, yPos+ yShift, cellW, cellH, inItems[rowIndex], 0x000000, {0,.5});
            }
            yPos += cellH - 1;
        }
        
        ImGui::PopClipRect();

    }

    UIComboBox * UIAddComboBox( const string & inComboName, int inIndex, int inPPGIndex ) {
        UIComboBox * cc;
        bool exists = false;
        for ( auto & cb : UILibCTX->ComboBoxList ) {
            if ( cb.Name == inComboName ) { 
                cc = &cb;
                exists = true;
                break;
            }
        }
        if (!exists) {
            UILibCTX->ComboBoxList.push_back( { inComboName } );
            cc = &UILibCTX->ComboBoxList.back();
            cc->Index = inIndex;
            cc->PPGIndex = inPPGIndex;
        }
        return cc;
    }

    UIComboBox * UIAddComboBox( const string & inComboName, ImRect inRect, vector<string> &inItems, int inIndex, int inPPGIndex ) {
        UIComboBox * cc = UIAddComboBox( inComboName, inIndex, inPPGIndex );
        UIDrawComboBox( cc, inRect, inItems );
        return cc;
    }

    UIComboBox * UIGetComboBox( const string & inComboName ) {
        for ( int i = 0; i < UILibCTX->ComboBoxList.size(); i++ ) {
            if ( UILibCTX->ComboBoxList[i].Name == inComboName ) { 
                return &UILibCTX->ComboBoxList[i];
            }
        }
        return nullptr;
    }

    void UIRemoveComboBox( const string & inComboName ) {
        for ( int i = 0; i < UILibCTX->ComboBoxList.size(); i++ ) {
            if ( UILibCTX->ComboBoxList[i].Name == inComboName ) { 
                UILibCTX->ComboBoxList.erase (UILibCTX->ComboBoxList.begin()+i);
                break;
            }
        }
    }

    void UIRemovePPGComboBoxes( int inPPGIndex ) {
        int nCombos = (int) UILibCTX->ComboBoxList.size();
        for ( int i = nCombos-1; i >= 0; i-- ) {
            if ( UILibCTX->ComboBoxList[i].PPGIndex == inPPGIndex) {
                std::cout << "removing combo box: " << UILibCTX->ComboBoxList[i].Name << std::endl;
                UILibCTX->ComboBoxList.erase (UILibCTX->ComboBoxList.begin()+i);
            }
            else if ( UILibCTX->ComboBoxList[i].PPGIndex > inPPGIndex ) {
                UILibCTX->ComboBoxList[i].PPGIndex--;
            }
        }
    }

    void UIUpdateComboBoxPPGIndex( vector<int> inNewIds ) {
        for ( int i = 0; i < UILibCTX->ComboBoxList.size(); i++ ) {
            if ( UILibCTX->ComboBoxList[i].PPGIndex > -1) {
                int newIndex = inNewIds[UILibCTX->ComboBoxList[i].PPGIndex];
                std::cout << "changing combo " << UILibCTX->ComboBoxList[i].Name << " index from: " << UILibCTX->ComboBoxList[i].PPGIndex << " to: " << newIndex << std::endl;
                UILibCTX->ComboBoxList[i].PPGIndex = newIndex;
            }
        }
    }

    void UIComboInitFocus( int inMode, UIComboBox * inHandle, ImRect inRect, vector<string> inList ) {
        if ( UILibCTX->ComboHandle != nullptr ) {
            UILibCTX->ComboHandle->Open = false;
        }
        UILibCTX->ComboMode = inMode;
        UILibCTX->ComboHandle = inHandle;
        UILibCTX->ComboHandle->Open = true;
        UILibCTX->ComboRect = inRect;
        UILibCTX->ComboItems = inList;
        UILibCTX->ComboScrollPerc = 0;
        UILibCTX->ComboDragging = 0;
    }

    void UIComboExitFocus() {
        if ( UILibCTX->ComboHandle != nullptr ) {
            UILibCTX->ComboHandle->Open = false;
        }
        UILibCTX->ComboMode = 0;
        UILibCTX->ComboHandle = nullptr;
        UILibCTX->ComboRect = {};
        UILibCTX->ComboItems = {};
        UILibCTX->ComboScrollPerc = 0;
        UILibCTX->ComboDragging = 0;
    }

    UIComboBox * UIComboGetFocusHandle() {
        return UILibCTX->ComboHandle;
    }

    string UIComboGetFocusedName() {
        if ( UILibCTX->ComboHandle != nullptr ) {
            return UILibCTX->ComboHandle->Name;
        }
        return "";
    }

    void UILoadingIndicatorCircle( ImRect inRect, int inClr, int inCount, float inPct, int nbSegments) {
        // https://github.com/ocornut/imgui/issues/1901#issuecomment-444929973

        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        
        const float indicator_radius = inRect.GetWidth() * 0.5f;
        const float circle_radius = indicator_radius / 10.0f;
        const auto degree_offset = 2.0f * IM_PI / inCount;
        ImVec2 pos = inRect.Min;

        for (int i = 0; i < inCount; ++i) {
            const auto x = indicator_radius * std::sin(degree_offset * i);
            const auto y = indicator_radius * std::cos(degree_offset * i);
            const auto growth = std::max(0.0f, std::sin(inPct - i * degree_offset));

            drawList->AddCircleFilled(ImVec2(pos.x + indicator_radius + x, pos.y + indicator_radius - y),
                                                    circle_radius + growth * circle_radius,
                                                    Cl(inClr), nbSegments);
        }
        
    }

    void UILoadBar( const ImRect & inR, float inPerc, const string & inMessage, int inFontI, vector<int> inClr ){
        // inClr = { font color, bar fill color, bar empty color }

        assert( inClr.size() >= 3 && "UILoadBar argument inClr must have a size of 3." );

        float w = inR.GetWidth();
        float h = inR.GetHeight();

        if(inPerc < .999){
            float frontW = inPerc * w;
            float backW  = w - frontW;
            ImRect frontR = GetBB(inR.Min.x, inR.Min.y, frontW, h);
            UIDrawAlphaRoundCornerBB(frontR, inClr[1], 1, 7, "0101");

            ImRect backR = GetBB(inR.Min.x+frontW, inR.Min.y, backW, h);
            UIDrawAlphaRoundCornerBB(backR, inClr[2], 1, 7, "1010");
        }
        else{
            ImRect frontR = GetBB(inR.Min.x, inR.Min.y, w, h);
            UIDrawAlphaRoundCornerBB(frontR, inClr[1], 1, 7, "1111");
        }

        ImFont* font=GImGui->IO.Fonts->Fonts[inFontI];
        float lineH=font->FontSize;
        float frontT = .7f * w;
        float backT  = w - frontT;
        float tY = inR.Min.y - (lineH * 0.3f) - lineH;
        UIAddTextWithFont(inR.Min.x+2, tY, frontT, lineH, inMessage, inClr[0], inFontI, {0,0});
        string percStr = to_string( int(round(inPerc*100)) ) + "%";


        UIAddTextWithFont(inR.Min.x+frontT-2, tY, backT, lineH, percStr, inClr[0], inFontI, {1,0});

    }

    void UICheckList(ImRect inRect, vector<string> &inTextList, vector<int> &inStateList, float &inScrollPerc, bool &inDragging){
        
        ImVec2 wPos = GImGui->CurrentWindow->Pos;
        ImRect globalRect = GetGlobalBB(inRect.Min.x, inRect.Min.y, inRect.GetWidth(), inRect.GetHeight());
        UIDrawBBOutline(globalRect, 0xABABAB, 1);  
        
        float spacer = 27;
        float xPos = inRect.Min.x + spacer;
        float yPos = inRect.Min.y;
        float cellW = inRect.GetWidth() - spacer;
        float cellH = 21;
        float viewH = inRect.GetHeight();

        float totalH = ( inStateList.size() ) * cellH;
        bool scrollOn = ( totalH > viewH + 0.001f );
        float yShift = 0;
        float TrackW = 10;
        
        int startRow = 0;
        int RowsPerBox = std::min( (int) inStateList.size()-1, (int) ( viewH / cellH ) + 1 );

        if ( scrollOn ) {
            float offScreen = totalH - viewH;
            yShift = -inScrollPerc * offScreen;
            startRow = (int) floor( -yShift / cellH );
            cellW -= TrackW;
            UIAddScrollerV(inRect.Max.x - TrackW, yPos, TrackW, viewH, totalH, &inScrollPerc, &inDragging, UILibCTX->DefaultScroller);
        }
        
        vec2 mPos = GetGlobalMousePos();
        ImRect listBox = GetGlobalBB(inRect.Min.x, inRect.Min.y, inRect.GetWidth() - TrackW , inRect.GetHeight());
        ImGui::PushClipRect(listBox.Min, listBox.Max, true);

        yPos += yShift + startRow*cellH;
        for(int64_t i=0;i<=RowsPerBox;i++){
            int64_t rowIndex = startRow + i;
            if ( rowIndex >= (int64_t) inStateList.size() ) { break; }
            ImRect cell = GetGlobalBB(inRect.Min.x, yPos, inRect.GetWidth() - TrackW , cellH-1);
            UIAddTextString(xPos, yPos, cellW, cellH, inTextList[rowIndex], 0x000000, {0,.5});
            UICheckBox( inRect.Min.x + 6, yPos + 3, 15.0f, inStateList[rowIndex] );
            if( VecInBB(mPos, cell) ){
                UIDrawAlphaBB(cell, 0x228BC5, .1f);
                if( DragStart() ){
                    if(inStateList[rowIndex] == 0){inStateList[rowIndex] = 1;}
                    else{inStateList[rowIndex] = 0;}
                }
            }
            yPos += cellH;
        }
        ImGui::PopClipRect();
        
    }

    void UICheckListByCallback( ImRect inRect, std::function<string (int64_t)> inStrCallback, vector<int> &inStateList, float &inScrollPerc, bool &inDragging ) {
        // Same as UICheckList, but instead of passing in a vector of string (inTextList), you pass in a callback (inStrCallback) to get
        // the string at a given index.

        ImVec2 wPos = GImGui->CurrentWindow->Pos;
        ImRect globalRect = GetGlobalBB(inRect.Min.x, inRect.Min.y, inRect.GetWidth(), inRect.GetHeight());
        UIDrawBBOutline(globalRect, 0xABABAB, 1);  
        
        float spacer = 27;
        float xPos = inRect.Min.x + spacer;
        float yPos = inRect.Min.y;
        float cellW = inRect.GetWidth() - spacer;
        float cellH = 21;
        float viewH = inRect.GetHeight();

        float totalH = ( inStateList.size() ) * cellH;
        bool scrollOn = ( totalH > viewH + 0.001f );
        float yShift = 0;
        float TrackW = 10;
        
        int startRow = 0;
        int RowsPerBox = std::min( (int) inStateList.size()-1, (int) ( viewH / cellH ) + 1 );

        if ( scrollOn ) {
            float offScreen = totalH - viewH;
            yShift = -inScrollPerc * offScreen;
            startRow = (int) floor( -yShift / cellH );
            cellW -= TrackW;
            UIAddScrollerV(inRect.Max.x - TrackW, yPos, TrackW, viewH, totalH, &inScrollPerc, &inDragging, UILibCTX->DefaultScroller);
        }
        
        vec2 mPos = GetGlobalMousePos();
        ImRect listBox = GetGlobalBB(inRect.Min.x, inRect.Min.y, inRect.GetWidth() - TrackW , inRect.GetHeight());
        ImGui::PushClipRect(listBox.Min, listBox.Max, true);

        yPos += yShift + startRow*cellH;
        for(int64_t i=0;i<=RowsPerBox;i++){
            int64_t rowIndex = startRow + i;
            if ( rowIndex >= (int64_t) inStateList.size() ) { break; }
            ImRect cell = GetGlobalBB(inRect.Min.x, yPos, inRect.GetWidth() - TrackW , cellH-1);
            UIAddTextString(xPos, yPos, cellW, cellH, inStrCallback(rowIndex), 0x000000, {0,.5});
            UICheckBox( inRect.Min.x + 6, yPos + 3, 15.0f, inStateList[rowIndex] );
            if( VecInBB(mPos, cell) ){
                UIDrawAlphaBB(cell, 0x228BC5, .1f);
                if( DragStart() ){
                    if(inStateList[rowIndex] == 0){inStateList[rowIndex] = 1;}
                    else{inStateList[rowIndex] = 0;}
                }
            }
            yPos += cellH;
        }
        ImGui::PopClipRect();
        
    }

    void UIAddScrollerH(float inX, float inY, float inW, float inH, float inContentW, float* inVal, bool* inDragging, const UIFormatScroller & inFormat, int* const inState, bool inMute ){  
        // Draws the horizontal scroll bar.
        // Pass in a pointer to an int to get the current state of the mouse & scroll bar interaction.
        // State: 0 = none, 1 = clicked bar, 2 = currently dragging bar, 3 = clicked track, 4 = released from bar, 5 = hover ove bar
        
        string idStr = to_string(int(round(inX))) + to_string(int(round(inY))) + "#scrollH";
        ImGuiID id = GImGui->CurrentWindow->GetID(idStr.c_str() );

        ImRect trackBB = GetBB(inX, inY, inW, inH);
        float ratio = inW / inContentW;

        float barW = round(inW * ratio);
        if(barW < inH){barW = inH;}
        
        float dragW = inW - barW;
        const ImRect dragBB = GetBB(inX, inY, dragW, inH);

        const bool overTrack = GetOverState(trackBB) && !inMute;
        const bool active = GetActiveState(overTrack, id);
        
        if ( inFormat.TrackAlpha > 0 ) { UIDrawAlphaBB(trackBB, inFormat.TrackBkgd, inFormat.TrackAlpha); }
        if ( inFormat.TrackBorderSize > 0 ) { UIDrawBBOutline(trackBB, inFormat.TrackBorderColor, inFormat.TrackBorderSize); }

        float sliderX = inX + (float) *inVal * dragW ;
        ImRect barBB = GetBB(sliderX, inY, barW, inH);
        const bool overBar = GetOverState(barBB) && !inMute;

        float mouseX = GetMousePos().x;
        int sliderClr = inFormat.BarBkgd;
        float sliderAlpha = inFormat.BarAlpha;
        int state = 0;

        if(overBar && DragStart()){ // Start Dragging the Bar
            *inDragging = true;
            if ( inFormat.DownBarBkgd >= 0 ) { sliderClr = inFormat.DownBarBkgd; }
            if ( inFormat.DownBarAlpha >= 0 ) { sliderAlpha = inFormat.DownBarAlpha; }
            state = 1;
        }
        else if(*inDragging && MouseDown()){ // Currently Dragging the Bar
            *inVal += (float) UILibIS->Position.Delta.X / dragW;
            *inVal = std::min( (float) 1, std::max((float)  0, (float) *inVal ) );
            if ( inFormat.DownBarBkgd >= 0 ) { sliderClr = inFormat.DownBarBkgd; }
            if ( inFormat.DownBarAlpha >= 0 ) { sliderAlpha = inFormat.DownBarAlpha; }
            state = 2;
        }
        else if(overTrack && DragStart()){ // Click Track Outside Bar
            float xFromVal = (float)(*inVal * dragW + inX);
            float pageShift = (float)(inW * ratio);
            if(mouseX < sliderX){ sliderX = ClipFloat( xFromVal - pageShift, inX, dragW + inX); } // Page Up
            else{sliderX = ClipFloat( xFromVal + pageShift, inX, dragW + inX); } // Page Down
            *inVal = (sliderX - inX) / dragW;
            state = 3;
        }
        else if(*inDragging && !MouseDown()){  // Mouse up, stop dragging
            *inDragging = false;
            sliderX = (float) *inVal * dragW + inX;
            state = 4;
        }
        else if(overBar) {  // Mouse is hovered over bar
            if ( inFormat.OverBarBkgd >= 0 ) { sliderClr = inFormat.OverBarBkgd; }
            if ( inFormat.OverBarAlpha >= 0 ) { sliderAlpha = inFormat.OverBarAlpha; }
            state = 5;
        }
        else{
            sliderX = (float) *inVal * dragW + inX;
            state = overTrack ? 5 : 0;
        }
        if ( inState != NULL ) { *inState = state; }

        barBB = GetBB(sliderX, inY + 2, barW, inH - 4);
        if ( inFormat.BarRounding == 0 ) {
            if ( sliderAlpha > 0 ) { UIDrawAlphaBB(barBB, sliderClr, sliderAlpha); }
        }
        else {
            if ( sliderAlpha > 0 ) { UIDrawAlphaRoundBB(barBB, sliderClr, sliderAlpha, inFormat.BarRounding); }
        }

    }

    void UIAddScrollerV(float inX, float inY, float inW, float inH, float inContentH, float* inVal, bool *inDragging, const UIFormatScroller & inFormat, int* const inState, bool inMute ){   
        // Draws the vertical scroll bar.
        // Pass in a pointer to an int to get the current state of the mouse & scroll bar interaction.
        // State: 0 = none, 1 = clicked bar, 2 = currently dragging bar, 3 = clicked track, 4 = released from bar, 5 = hover ove bar
        
        string idStr = to_string(int(round(inX))) + to_string(int(round(inY))) + "#scrollV";
        ImGuiID id = GImGui->CurrentWindow->GetID(idStr.c_str() );

        ImRect trackBB = GetBB(inX, inY, inW, inH);
        float ratio = inH / inContentH;

        float barH = round(inH * ratio);
        if(barH < inW){barH = inW;}

        float dragH = inH - barH;
        ImRect dragBB = GetBB(inX, inY, inW, dragH);

        bool overTrack = GetOverState(trackBB) && !inMute;
        bool active = GetActiveState(overTrack, id);

        if ( inFormat.TrackAlpha > 0 ) { UIDrawAlphaBB(trackBB, inFormat.TrackBkgd, inFormat.TrackAlpha); }
        if ( inFormat.TrackBorderSize > 0 ) { UIDrawBBOutline(trackBB, inFormat.TrackBorderColor, inFormat.TrackBorderSize); }

        float sliderY = inY + (float) *inVal * dragH ;
        ImRect barBB = GetBB(inX, sliderY, inW, barH);
        bool overBar = GetOverState(barBB) && !inMute;

        float mouseY = GetMousePos().y;
        int sliderClr = inFormat.BarBkgd;
        float sliderAlpha = inFormat.BarAlpha;
        int state = 0;
        
        if(overBar && DragStart()){ // Start Dragging the Bar
            *inDragging = true;
            if ( inFormat.DownBarBkgd >= 0 ) { sliderClr = inFormat.DownBarBkgd; }
            if ( inFormat.DownBarAlpha >= 0 ) { sliderAlpha = inFormat.DownBarAlpha; }
            state = 1;
        }
        else if(*inDragging && MouseDown()){ // Currently Dragging the Bar
            *inVal += (float) UILibIS->Position.Delta.Y / dragH;
            *inVal = std::min( (float) 1, std::max((float)  0, (float) *inVal ) );
            if ( inFormat.DownBarBkgd >= 0 ) { sliderClr = inFormat.DownBarBkgd; }
            if ( inFormat.DownBarAlpha >= 0 ) { sliderAlpha = inFormat.DownBarAlpha; }
            state = 2;
        }
        else if(overTrack && DragStart()){ // Click Track Outside Bar
            float yFromVal = (float) *inVal * dragH + inY;
            float pageShift = inH * ratio;
            if(mouseY < sliderY){ sliderY = ClipFloat( yFromVal - pageShift, inY, dragH + inY); } // Page Up
            else{sliderY = ClipFloat( yFromVal + pageShift, inY, dragH + inY); } // Page Down
            *inVal = (sliderY - inY) / dragH;
            state = 3;
        }
        else if(*inDragging && !MouseDown()){  // Mouse up, stop dragging
            *inDragging = false;
            sliderY = (float) *inVal * dragH + inY;
            state = 4;
        }
        else if(overBar) {  // Mouse is hovered over bar
            if ( inFormat.OverBarBkgd >= 0 ) { sliderClr = inFormat.OverBarBkgd; }
            if ( inFormat.OverBarAlpha >= 0 ) { sliderAlpha = inFormat.OverBarAlpha; }
            state = 5;
        }
        else{
            sliderY = (float) *inVal * dragH + inY;
            state = overTrack ? 5 : 0;
        }
        if ( inState != NULL ) { *inState = state; }

        barBB = GetBB(inX + 2, sliderY, inW - 4, barH);
        if ( inFormat.BarRounding == 0 ) {
            if ( sliderAlpha > 0 ) { UIDrawAlphaBB(barBB, sliderClr, sliderAlpha); }
        }
        else {
            if ( sliderAlpha > 0 ) { UIDrawAlphaRoundBB(barBB, sliderClr, sliderAlpha, inFormat.BarRounding); }
        }

    }

    void UIAddScrollerH(float inX, float inY, float inW, float inH, float inContentW, double* inVal, bool* inDragging, const UIFormatScroller & inFormat, int* const inState, bool inMute ){    
        // Draws the horizontal scroll bar.
        // Pass in a pointer to an int to get the current state of the mouse & scroll bar interaction.
        // State: 0 = none, 1 = clicked bar, 2 = currently dragging bar, 3 = clicked track, 4 = released from bar, 5 = hover ove bar

        string idStr = to_string(int(round(inX))) + to_string(int(round(inY))) + "#scrollH";
        ImGuiID id = GImGui->CurrentWindow->GetID(idStr.c_str() );

        ImRect trackBB = GetBB(inX, inY, inW, inH);
        float ratio = inW / inContentW;

        float barW = round(inW * ratio);
        if(barW < inH){barW = inH;}
        
        float dragW = inW - barW;
        const ImRect dragBB = GetBB(inX, inY, dragW, inH);

        const bool overTrack = GetOverState(trackBB) && !inMute;
        const bool active = GetActiveState(overTrack, id);
        
        if ( inFormat.TrackAlpha > 0 ) { UIDrawAlphaBB(trackBB, inFormat.TrackBkgd, inFormat.TrackAlpha); }
        if ( inFormat.TrackBorderSize > 0 ) { UIDrawBBOutline(trackBB, inFormat.TrackBorderColor, inFormat.TrackBorderSize); }

        float sliderX = inX + (float) *inVal * dragW ;
        ImRect barBB = GetBB(sliderX, inY, barW, inH);
        const bool overBar = GetOverState(barBB) && !inMute;

        float mouseX = GetMousePos().x;
        int sliderClr = inFormat.BarBkgd;
        float sliderAlpha = inFormat.BarAlpha;
        int state = 0;

        if(overBar && DragStart()){ // Start Dragging the Bar
            *inDragging = true;
            if ( inFormat.DownBarBkgd >= 0 ) { sliderClr = inFormat.DownBarBkgd; }
            if ( inFormat.DownBarAlpha >= 0 ) { sliderAlpha = inFormat.DownBarAlpha; }
            state = 1;
        }
        else if(*inDragging && MouseDown()){ // Currently Dragging the Bar
            *inVal += UILibIS->Position.Delta.X / dragW;
            *inVal = std::min( (double) 1, std::max((double)  0, (double) *inVal ) );
            if ( inFormat.DownBarBkgd >= 0 ) { sliderClr = inFormat.DownBarBkgd; }
            if ( inFormat.DownBarAlpha >= 0 ) { sliderAlpha = inFormat.DownBarAlpha; }
            state = 2;
        }
        else if(overTrack && DragStart()){ // Click Track Outside Bar
            float xFromVal = (float)(*inVal * dragW + inX);
            float pageShift = (float)(inW * ratio);
            if(mouseX < sliderX){ sliderX = ClipFloat( xFromVal - pageShift, inX, dragW + inX); } // Page Up
            else{sliderX = ClipFloat( xFromVal + pageShift, inX, dragW + inX); } // Page Down
            *inVal = (sliderX - inX) / dragW;
            state = 3;
        }
        else if(*inDragging && !MouseDown()){  // Mouse up, stop dragging
            *inDragging = false;
            sliderX = (float) *inVal * dragW + inX;
            state = 4;
        }
        else if(overBar) {  // Mouse is hovered over bar
            if ( inFormat.OverBarBkgd >= 0 ) { sliderClr = inFormat.OverBarBkgd; }
            if ( inFormat.OverBarAlpha >= 0 ) { sliderAlpha = inFormat.OverBarAlpha; }
            state = 5;
        }
        else{
            sliderX = (float) *inVal * dragW + inX;
            state = overTrack ? 5 : 0;
        }
        if ( inState != NULL ) { *inState = state; }

        barBB = GetBB(sliderX, inY + 2, barW, inH - 4);
        if ( inFormat.BarRounding == 0 ) {
            if ( sliderAlpha > 0 ) { UIDrawAlphaBB(barBB, sliderClr, sliderAlpha); }
        }
        else {
            if ( sliderAlpha > 0 ) { UIDrawAlphaRoundBB(barBB, sliderClr, sliderAlpha, inFormat.BarRounding); }
        }

    }

    void UIAddScrollerV(float inX, float inY, float inW, float inH, float inContentH, double* inVal, bool *inDragging, const UIFormatScroller & inFormat, int* const inState, bool inMute ){   
        // Draws the vertical scroll bar.
        // Pass in a pointer to an int to get the current state of the mouse & scroll bar interaction.
        // State: 0 = none, 1 = clicked bar, 2 = currently dragging bar, 3 = clicked track, 4 = released from bar, 5 = hover ove bar
        
        string idStr = to_string(int(round(inX))) + to_string(int(round(inY))) + "#scrollV";
        ImGuiID id = GImGui->CurrentWindow->GetID(idStr.c_str() );

        ImRect trackBB = GetBB(inX, inY, inW, inH);
        float ratio = inH / inContentH;

        float barH = round(inH * ratio);
        if(barH < inW){barH = inW;}

        float dragH = inH - barH;
        ImRect dragBB = GetBB(inX, inY, inW, dragH);

        bool overTrack = GetOverState(trackBB) && !inMute;
        bool active = GetActiveState(overTrack, id);

        if ( inFormat.TrackAlpha > 0 ) { UIDrawAlphaBB(trackBB, inFormat.TrackBkgd, inFormat.TrackAlpha); }
        if ( inFormat.TrackBorderSize > 0 ) { UIDrawBBOutline(trackBB, inFormat.TrackBorderColor, inFormat.TrackBorderSize); }

        float sliderY = inY + (float) *inVal * dragH ;
        ImRect barBB = GetBB(inX, sliderY, inW, barH);
        bool overBar = GetOverState(barBB) && !inMute;

        float mouseY = GetMousePos().y;
        int sliderClr = inFormat.BarBkgd;
        float sliderAlpha = inFormat.BarAlpha;
        int state = 0;
        
        if(overBar && DragStart()){ // Start Dragging the Bar
            *inDragging = true;
            if ( inFormat.DownBarBkgd >= 0 ) { sliderClr = inFormat.DownBarBkgd; }
            if ( inFormat.DownBarAlpha >= 0 ) { sliderAlpha = inFormat.DownBarAlpha; }
            state = 1;
        }
        else if(*inDragging && MouseDown()){ // Currently Dragging the Bar
            *inVal += UILibIS->Position.Delta.Y / dragH;
            *inVal = std::min( (double) 1, std::max((double)  0, (double) *inVal ) );
            if ( inFormat.DownBarBkgd >= 0 ) { sliderClr = inFormat.DownBarBkgd; }
            if ( inFormat.DownBarAlpha >= 0 ) { sliderAlpha = inFormat.DownBarAlpha; }
            state = 2;
        }
        else if(overTrack && DragStart()){ // Click Track Outside Bar
            float yFromVal = (float) *inVal * dragH + inY;
            float pageShift = inH * ratio;
            if(mouseY < sliderY){ sliderY = ClipFloat( yFromVal - pageShift, inY, dragH + inY); } // Page Up
            else{sliderY = ClipFloat( yFromVal + pageShift, inY, dragH + inY); } // Page Down
            *inVal = (sliderY - inY) / dragH;
            state = 3;
        }
        else if(*inDragging && !MouseDown()){  // Mouse up, stop dragging
            *inDragging = false;
            sliderY = (float) *inVal * dragH + inY;
            state = 4;
        }
        else if(overBar) {  // Mouse is hovered over bar
            if ( inFormat.OverBarBkgd >= 0 ) { sliderClr = inFormat.OverBarBkgd; }
            if ( inFormat.OverBarAlpha >= 0 ) { sliderAlpha = inFormat.OverBarAlpha; }
            state = 5;
        }
        else{
            sliderY = (float) *inVal * dragH + inY;
            state = overTrack ? 5 : 0;
        }
        if ( inState != NULL ) { *inState = state; }

        barBB = GetBB(inX + 2, sliderY, inW - 4, barH);
        if ( inFormat.BarRounding == 0 ) {
            if ( sliderAlpha > 0 ) { UIDrawAlphaBB(barBB, sliderClr, sliderAlpha); }
        }
        else {
            if ( sliderAlpha > 0 ) { UIDrawAlphaRoundBB(barBB, sliderClr, sliderAlpha, inFormat.BarRounding); }
        }

    }

    int UIToggleButtons(float inX, float inY, float inHeight, int inColor, float inRound, vector<string> inStrings, int* inIndex, float inPad, int inFontIndex){
        // Returns the index of the toggle button field clicked

        // get the lengths for all the strings
        float textLen = 0;
        size_t n = inStrings.size();
        
        ImGuiContext& g = *GImGui;
        PushFont(g.IO.Fonts->Fonts[inFontIndex]);
        float xPos = inX;
        float spacer = 4; // space between tabs
        float hSpace = spacer * 0.5f;

        int res = -1;
        for(int i=0;i<n;i++){
            
            string idStr = "##tab" + to_string(xPos) + to_string(inY);
            const ImGuiID id = GImGui->CurrentWindow->GetID(idStr.c_str() );

            const char* charStr = inStrings[i].c_str();
            float textWidth =  CalcTextSize(charStr).x;
            float tabWidth = inPad * 2.0f + textWidth;
            
            ImRect tabBB = GetGlobalBB(xPos - hSpace, inY, tabWidth + spacer, inHeight);

            const bool overTab = GetOverState(tabBB);
            const bool active = GetActiveState(overTab, id);
            if ( active ) {
                if ( *inIndex != i ){ *inIndex = i; }
                res = i; 
            }            

            //int tabClr = overTab ? 0x48926b : 0x267E50;

            int fontClr = 0x228BC5;
            if(*inIndex == i){
                fontClr = 0xFFFFFF;
            }

            ImRect bRect = GetGlobalBB(xPos, inY, tabWidth, inHeight);

            if(i==0){
                if(*inIndex == i){
                    UIDrawAlphaRoundCornerBB(bRect, inColor, 1, inRound, "0101");
                }
                UIDrawRoundBBOutlineCorner(bRect, inColor, inRound, 1.5, "0101");
            }
            else if(i==(int)n-1){
                if(*inIndex == i){
                    UIDrawAlphaRoundCornerBB(bRect, inColor, 1, inRound, "1010");
                }
                UIDrawRoundBBOutlineCorner(bRect, inColor, inRound, 1.5, "1010");
            }
            else{
                if(*inIndex == i){
                    UIDrawBB(bRect, inColor);
                }
                UIDrawBBOutline(bRect, inColor, 1.5);
            }
            UIAddTextString(xPos + inPad,inY,textWidth, inHeight, inStrings[i], fontClr, vec2(.5,.5));

            xPos += tabWidth - 1;// - .6f;

        }
        PopFont();

        return res;

    }
    void UIAddTabs(float inX, float inY, float inHeight, vector<string> inStrings, int* inIndex, float inPad, int inFontIndex){
        
        // get the lengths for all the strings
        float textLen = 0;
        size_t n = inStrings.size();
        
        ImGuiContext& g = *GImGui;
        PushFont(g.IO.Fonts->Fonts[inFontIndex]);
        float xPos = inX;
        float spacer = 7; // space between tabs
        float hSpace = spacer * 0.5f;

        for(int i=0;i<n;i++){
            
            string idStr = "##tab" + to_string(xPos) + to_string(inY);
            const ImGuiID id = GImGui->CurrentWindow->GetID(idStr.c_str() );

            const char* charStr = inStrings[i].c_str();
            float textWidth =  CalcTextSize(charStr).x;
            float tabWidth = inPad * 2.0f + textWidth;
            
            ImRect tabBB = GetGlobalBB(xPos - hSpace, inY, tabWidth + spacer, inHeight);

            const bool overTab = GetOverState(tabBB);
            const bool active = GetActiveState(overTab, id);

            if(active && *inIndex != i){
                *inIndex = i;
            }

            int tabClr = overTab ? 0x59AEF8 : 0x118BF4;
            int fontClr = 0xFFFFFF;
            if(*inIndex == i){
                tabClr = 0xf4f5f5;
                fontClr = 0x228BC5;
            }

            UIAddRoundCornerRect(xPos, inY, tabWidth, inHeight, tabClr, true, 0, 4, "0011");
            UIAddTextString(xPos + inPad,inY,textWidth, inHeight, inStrings[i], fontClr, vec2(.5,.5));

            xPos += tabWidth + spacer;

        }
        PopFont();

    }
    
    void UIAddSliderH(float inX, float inY, float inW, float inH, float* inVal, float inMin, float inMax, bool &inMuteUI, bool &inChange){   
        
        string idStr = "##sliderH" + to_string(inX) + to_string(inY);

        const ImGuiID id = GImGui->CurrentWindow->GetID(idStr.c_str() );
        const ImRect trackBB = GetGlobalBB(inX, inY, inW, inH);
        float sliderW = 9.0f;  // circle radius
        ImRect sliderBB = GetBB(1,0,sliderW,inH-2);
        float halfSliderW = sliderW * 0.5f;
        // the area of mouse drag - dragBB is smaller than the length of the slider so the box doesn't go over the edges
        const ImRect dragBB = GetGlobalBB(inX + halfSliderW, inY, inW - sliderW, inH);
        float dragW = dragBB.GetSize().x;  
        const bool overTrack = ( inMuteUI ) ? false : GetOverState(trackBB);
        const bool active = GetActiveState(overTrack, id);
        float range = inMax - inMin;
        
        if(active){
            float mouseX = GetMousePos().x;  // global
            float sPerc = ClipFloat( (mouseX - dragBB.Min.x)/ dragW, 0.0f, 1.0f);
            *inVal = range * sPerc + inMin;
            inChange = true;
        }
        else{
            inChange = false;
        }
        float perc = (*inVal - inMin) / range;
        float grabX = perc * dragW + dragBB.Min.x;
        ImVec2 wPos = GImGui->CurrentWindow->Pos;
        float midX = grabX - wPos.x;
        float midY = inY + inH * 0.5f;
        float railHeight = 1.2f;  // thickness of the rail the dot slides on
        float railY = inY + (inH - railHeight) * 0.5f;
        float frontRailWidth = perc * dragW;
        unsigned int frontColor = 0x878787;
        if(overTrack || active){frontColor = 0x2EAADC;}
        const ImRect frontRailBB = GetGlobalBB(inX, railY, frontRailWidth, railHeight);
        UIDrawBB(frontRailBB, frontColor);
        const ImRect backRailBB = GetGlobalBB(grabX- wPos.x, railY, dragW - frontRailWidth, railHeight);
        UIDrawBB(backRailBB, 0xD6D6D6);
        unsigned int dotColor = active ? 0x268BAA : overTrack ? 0x2EAADC : 0x878787;
        UIAddCircle(midX, midY, halfSliderW, dotColor, true, 4, 64);

    }
    
    void UIAddSliderV(float inX, float inY, float inW, float inH, float* inVal, float inMin, float inMax, bool &inMuteUI, bool &inChange){   
        
        string idStr = "##sliderV" + to_string(inX) + to_string(inY);

        const ImGuiID id = GImGui->CurrentWindow->GetID(idStr.c_str() );
        const ImRect trackBB = GetGlobalBB(inX, inY, inW, inH);
        float sliderH = 9.0f;  // circle radius
        ImRect sliderBB = GetBB(0,1,inW-2,sliderH);
        float halfSliderH = sliderH * 0.5f;
        // the area of mouse drag - dragBB is smaller than the length of the slider so the box doesn't go over the edges
        const ImRect dragBB = GetGlobalBB(inX, inY+halfSliderH, inW, inH-sliderH);
        float dragH = dragBB.GetSize().y;  
        const bool overTrack = ( inMuteUI ) ? false : GetOverState(trackBB);
        const bool active = GetActiveState(overTrack, id);
        float range = inMax - inMin;
        
        if(active){
            float mouseY = GetMousePos().y;  // global
            float sPerc = ClipFloat( (mouseY - dragBB.Min.y)/ dragH, 0.0f, 1.0f);
            *inVal = range * sPerc + inMin;
            inChange = true;
        }
        else{
            inChange = false;
        }
        float perc = (*inVal - inMin) / range;
        float grabY = perc * dragH + dragBB.Min.y;
        ImVec2 wPos = GImGui->CurrentWindow->Pos;
        float midY = grabY - wPos.y;
        float midX = inX + inW * 0.5f;
        float railWidth = 1.2f;  // thickness of the rail the dot slides on
        float railX = inX + (inW - railWidth) * 0.5f;
        float frontRailHeight = perc * dragH;
        unsigned int frontColor = 0x878787;
        if(overTrack || active){frontColor = 0x2EAADC;}
        const ImRect frontRailBB = GetGlobalBB(railX, grabY- wPos.y, railWidth, dragH - frontRailHeight);
        UIDrawBB(frontRailBB, frontColor);
        const ImRect backRailBB = GetGlobalBB(railX, inY, railWidth, frontRailHeight);
        UIDrawBB(backRailBB, 0xD6D6D6);
        unsigned int dotColor = active ? 0x268BAA : overTrack ? 0x2EAADC : 0x878787;
        UIAddCircle(midX, midY, halfSliderH, dotColor, true, 4, 64);

    }

    void UIAddTimelineSlider(float inX, float inY, float inW, float inH, bool &inMuteUI, UIDataTimeline & inD){   

        const ImRect trackBB = GetGlobalBB(inX, inY, inW, inH);
        const ImRect clickBB = GetGlobalBB(inX-0.5f*inD.MarkerWidth, inY, inW+inD.MarkerWidth, inH);
        const double ValRange = inD.EndVal - inD.StartVal;

        // Draw the background, lines/ticks, and label text
        if ( inD.BkgdAlpha > 0 ) { UIDrawAlphaBB( trackBB, inD.BkgdColor, inD.BkgdAlpha ); }
        if ( inD.LineSize > 0 ) { 

            // Top line
            UIAddGlobalLine( trackBB.Min.x, trackBB.Min.y, trackBB.Max.x, trackBB.Min.y, inD.LineColor, inD.LineSize );

            // Tick Marks
            double tVal;
            float tPosX;
            int64_t startI, endI;
            string txtLabel = "";
            vec2 txtDim;

            // Minor Ticks
            if ( inD.MinorDiv > 0 ) {
                startI = (int64_t) ( ( inD.StartVal - inD.BaseVal ) / inD.MinorDiv );
                endI = ( int64_t ) ( ( inD.EndVal - inD.BaseVal ) / inD.MinorDiv ) + 1;
                if ( inD.StartVal - (double) startI * inD.MinorDiv > 0.00001 ) { startI++; }
                if ( (double) endI * inD.MinorDiv - inD.EndVal > 0.00001 ) { endI--; }
                for ( int64_t i = startI; i <= endI; i++ ) {
                    tVal = i * inD.MinorDiv + inD.BaseVal;
                    tPosX = trackBB.Min.x + (float) ( ( tVal - inD.StartVal ) / ValRange ) * inW;
                    UIAddGlobalLine( tPosX, trackBB.Min.y, tPosX, trackBB.Min.y + inD.MinorTickSize, inD.LineColor, inD.LineSize );
                }
            }

            // Major Ticks and Text Labels
            if ( inD.MajorDiv > 0 ) {
                startI = (int64_t) ( ( inD.StartVal - inD.BaseVal ) / inD.MajorDiv );
                endI = ( int64_t ) ( ( inD.EndVal - inD.BaseVal ) / inD.MajorDiv ) + 1;
                if ( inD.StartVal - (double) startI * inD.MajorDiv > 0.00001 ) { startI++; }
                if ( (double) endI * inD.MajorDiv - inD.EndVal > 0.00001 ) { endI--; }
                for ( int64_t i = startI; i <= endI; i++ ) {
                    tVal = i * inD.MajorDiv + inD.BaseVal;
                    tPosX = trackBB.Min.x + (float) ( ( tVal - inD.StartVal ) / ValRange ) * inW;
                    UIAddGlobalLine( tPosX, trackBB.Min.y, tPosX, trackBB.Min.y + inD.MajorTickSize, inD.LineColor, inD.LineSize );
                    txtLabel = inD.LabelFunc( tVal );
                    if ( !txtLabel.empty() ) {
                        if ( abs(tVal - inD.EndVal) < 0.00001 && inD.HideLast ) { continue; }
                        txtDim = UIGetTextDim( txtLabel, inD.Font, inD.FontScale );
                        UIAddTextWithFont( tPosX+inD.TextAlign.x, std::max( trackBB.Min.y, trackBB.Min.y+inD.MajorTickSize-txtDim.y+inD.TextAlign.y ), 
                                            txtDim.x, txtDim.y, txtLabel, inD.LineColor, inD.Font, vec2(0), inD.FontScale );
                    }
                }
            }

        }

        // Draw marker
        float mPosX = trackBB.Min.x + (float) ( ( inD.CurrVal - inD.StartVal ) / ValRange ) * inW - 0.5f* inD.MarkerWidth;
        UIAddRect( mPosX, trackBB.Min.y, inD.MarkerWidth, inH, inD.MarkerColor, true, 0 );

        // Draw marker text
        string mLabel = inD.LabelFunc( inD.CurrVal );
        if ( !mLabel.empty() ) {
            vec2 mDim = UIGetTextDim( mLabel, inD.Font, inD.FontScale );
            ImRect mBB = GetBB( mPosX+0.5f*inD.MarkerWidth+inD.TextAlign.x, std::max(trackBB.Min.y, trackBB.Max.y-mDim.y+inD.TextAlign.y), mDim.x, mDim.y );
            if ( mBB.Max.x > trackBB.Max.x ) {
                mBB.TranslateX( -( inD.MarkerWidth + 2*inD.TextAlign.x + mDim.x ) );
            }
            UIDrawBB( mBB, inD.BkgdColor );
            UIAddTextWithFontBB( mBB, mLabel, inD.MarkerColor, inD.Font, vec2(0), inD.FontScale );
        }

        // Handle Events
        // Status: 0 = none, 1 = over, 2 = down, 3 = dragging, 4 = release
        if ( inMuteUI && inD.Status != 3 ) { inD.Status = 0; return; }     // Exit if UI is muted
        highp_dvec2 mPos = GetMousePosD();
        if ( MouseDown() ) {
            if ( DragStart() ) {                                    // Mouse down
                if ( VecInBB( mPos, clickBB) ) {
                    inD.Status = 2;
                    inD.CurrVal = ( ( mPos.x - (double) trackBB.Min.x ) / (double) inW ) * ValRange + inD.StartVal;
                    if ( inD.CurrVal < inD.StartVal ) { inD.CurrVal = inD.StartVal; }
                    else if ( inD.CurrVal > inD.EndVal ) { inD.CurrVal = inD.EndVal; }
                }
                else { inD.Status = 0; }
            }
            else if ( inD.Status >= 1 && inD.Status <= 3 ) {        // Mouse dragging
                inD.Status = 3;
                inD.CurrVal = ( ( mPos.x - (double) trackBB.Min.x ) / (double) inW ) * ValRange + inD.StartVal;
                if ( inD.CurrVal < inD.StartVal ) { inD.CurrVal = inD.StartVal; }
                else if ( inD.CurrVal > inD.EndVal ) { inD.CurrVal = inD.EndVal; }
            }
        }
        else {
            if ( inD.Status == 2 || inD.Status == 3 ) { inD.Status = 4; }   // Mouse release
            else if ( VecInBB( mPos, clickBB) ) { inD.Status = 1; }         // Mouse over
            else { inD.Status = 0; }                                        // Mouse up not over track --> none
        }

        // Draw Marker
        if ( inD.Status == 1 ) {
            double pVal = ( ( mPos.x - (double) trackBB.Min.x ) / (double) inW ) * ValRange + inD.StartVal;
            if ( pVal < inD.StartVal ) { pVal = inD.StartVal; }
            else if ( pVal > inD.EndVal ) { pVal = inD.EndVal; }
            mPosX = trackBB.Min.x + (float) ( ( pVal - inD.StartVal ) / ValRange ) * inW - 0.5f* inD.MarkerWidth;
            UIAddRectAlpha( mPosX, trackBB.Min.y, inD.MarkerWidth, inH, inD.PreviewColor, true, 0, inD.PreviewAlpha );
        }

    }

    void UIAddCalendarWidget( const ImRect & inRect, UIDataCalendar & inData, bool Mute ) {
        // Note: The Calendar widget ONLY draws the calendar numbers, labels, and navigation.
        // It equallys spaces the month days based on the available space provided by the inRect.
        // It DOES NOT draw the calendar background and border. The background and border is 
        // drawn externally.
        
        vec2 mPos = GetGlobalMousePos();

        ImRect TopBB = { inRect.Min.x, inRect.Min.y, inRect.Max.x, inRect.Min.y + inData.HeaderHeight };
        ImRect CalBB = { inRect.Min.x, TopBB.Max.y, inRect.Max.x, inRect.Max.y };
        if ( inData.ShowTime ) { CalBB.Max.y -= inData.TimeHeight[0] + inData.TimeHeight[1]; }

        vec2 dDim = { CalBB.GetWidth() / 7, CalBB.GetHeight() / 7 };
        vec2 tDim = UIGetTextDim( "30", inData.DateFont[0], inData.DateFontScale[0] );
        float margin = std::max( 5.0f, (dDim.x-tDim.x)*0.5f ); // left & right margin
        TopBB.Min.x += margin; TopBB.Max.x -= margin;

        // Top Month & Year
        string txt = inData.MonthAsString( inData.ViewMonth, false ) + " " + std::to_string( inData.ViewYear );
        UIAddTextWithFontBB( TopBB, txt, inData.HeaderFontColor, inData.HeaderFont, {0, 0.5f}, inData.HeaderFontScale );

        // Top Navigation
        float navAlpha = 0.07f;
        float navSize = 17;
        float navEsp = 2;
        float navRound = 3;
        float navThick = 1.5f;
        int navDir = 0; // 0 = none, -1 = go to prev month, 1 = go to next month
        // Right navigation
        ImRect navBB = GetBB( TopBB.Max.x-navSize, TopBB.Min.y+(inData.HeaderHeight-navSize)*0.5f, navSize, navSize );
        if ( !Mute && VecInBB(mPos,navBB) ) {
            UIDrawAlphaRoundBB( navBB, 0x000000, navAlpha, navRound );
            if ( DragStart() ) { navDir = 1; }
        }
        navBB.Expand(-navEsp); navBB.Expand( { -navBB.GetWidth()*0.25f, 0 } );
        UIAddChevronRight( navBB, inData.NavColor, 1.0f, navThick );
        // Left navigation
        navBB = GetBB( TopBB.Max.x-2*navSize, TopBB.Min.y+(inData.HeaderHeight-navSize)*0.5f, navSize, navSize );
        if ( !Mute && VecInBB(mPos,navBB) ) {
            UIDrawAlphaRoundBB( navBB, 0x000000, navAlpha, navRound );
            if ( DragStart() ) { navDir = -1; }
        }
        navBB.Expand(-navEsp); navBB.Expand( { -navBB.GetWidth()*0.25f, 0 } );
        UIAddChevronLeft( navBB, inData.NavColor, 1.0f, navThick );

        // Days of Week Labels
        ImRect txtBB;
        txtBB = GetBB( CalBB.Min.x, CalBB.Min.y, dDim.x, dDim.y );
        UIAddTextWithFontBB( txtBB, "Su", inData.DateFontColor[2], inData.DateFont[2], {0.5f, 0.5f}, inData.DateFontScale[2] );
        txtBB = GetBB( CalBB.Min.x + dDim.x*1, CalBB.Min.y, dDim.x, dDim.y );
        UIAddTextWithFontBB( txtBB, "Mo", inData.DateFontColor[2], inData.DateFont[2], {0.5f, 0.5f}, inData.DateFontScale[2] );
        txtBB = GetBB( CalBB.Min.x + dDim.x*2, CalBB.Min.y, dDim.x, dDim.y );
        UIAddTextWithFontBB( txtBB, "Tu", inData.DateFontColor[2], inData.DateFont[2], {0.5f, 0.5f}, inData.DateFontScale[2] );
        txtBB = GetBB( CalBB.Min.x + dDim.x*3, CalBB.Min.y, dDim.x, dDim.y );
        UIAddTextWithFontBB( txtBB, "We", inData.DateFontColor[2], inData.DateFont[2], {0.5f, 0.5f}, inData.DateFontScale[2] );
        txtBB = GetBB( CalBB.Min.x + dDim.x*4, CalBB.Min.y, dDim.x, dDim.y );
        UIAddTextWithFontBB( txtBB, "Th", inData.DateFontColor[2], inData.DateFont[2], {0.5f, 0.5f}, inData.DateFontScale[2] );
        txtBB = GetBB( CalBB.Min.x + dDim.x*5, CalBB.Min.y, dDim.x, dDim.y );
        UIAddTextWithFontBB( txtBB, "Fr", inData.DateFontColor[2], inData.DateFont[2], {0.5f, 0.5f}, inData.DateFontScale[2] );
        txtBB = GetBB( CalBB.Min.x + dDim.x*6, CalBB.Min.y, dDim.x, dDim.y );
        UIAddTextWithFontBB( txtBB, "Sa", inData.DateFontColor[2], inData.DateFont[2], {0.5f, 0.5f}, inData.DateFontScale[2] );
        
        // Clicked date
        int cYear = -1, cMon = -1, cDay = -1;

        // Month Before
        int count = inData.ViewRow * 7 + inData.ViewWeekday;
        int mon = inData.ViewMonth - 1; if ( mon < 1 ) { mon = 12; }
        int year = mon==12?inData.ViewYear-1:inData.ViewYear;
        int day = inData.DaysInMonth( mon, inData.IsLeapYear(year) ) - count + 1;
        int wday = 0; // current weekday
        int row = 1;  // current row - start at row 1, since row 0 are the weekday labels
        float txtAlpha = 0.35f;
        while ( count > 0 ) {
            txtBB = GetBB( CalBB.Min.x + dDim.x*wday, CalBB.Min.y + dDim.y*row, dDim.x, dDim.y );
            if ( !Mute && VecInBB( mPos, txtBB ) ) {
                txtBB.Expand(-2);
                UIDrawAlphaBB( txtBB, inData.ShadeColor[0], inData.ShadeAlpha[0] );
                txtBB.Expand(2);
                if ( DragStart() ) { cYear = year; cMon = mon; cDay = day; }
            }
            UIAddAlphaTextWithFontBB( txtBB, std::to_string(day), inData.DateFontColor[0], txtAlpha, inData.DateFont[0], {0.5f, 0.5f}, inData.DateFontScale[0] );
            if ( wday == 6 ) { row++; wday = 0; } else { wday++; }
            day++; count--;
        }

        // Current Month
        mon = inData.ViewMonth;
        year = inData.ViewYear;
        count = inData.DaysInMonth( mon, inData.IsLeapYear(year) );
        day = 1;
        bool selected = false;
        while ( count > 0 ) {
            selected = (day == inData.Day && mon == inData.Month && year == inData.Year);
            txtBB = GetBB( CalBB.Min.x + dDim.x*wday, CalBB.Min.y + dDim.y*row, dDim.x, dDim.y );
            if ( selected ) {
                txtBB.Expand(-2);
                UIDrawBB( txtBB, inData.DateSelectedColor );
                txtBB.Expand(2);
            }
            if ( !Mute && VecInBB( mPos, txtBB ) ) {
                txtBB.Expand(-2);
                if ( selected ) { UIDrawAlphaBB( txtBB, inData.ShadeColor[1], inData.ShadeAlpha[1] ); }
                else                     { UIDrawAlphaBB( txtBB, inData.ShadeColor[0], inData.ShadeAlpha[0] ); }
                txtBB.Expand(2);
                if ( DragStart() ) { cYear = year; cMon = mon; cDay = day; }
            }
            if ( selected ) { UIAddTextWithFontBB( txtBB, std::to_string(day), inData.DateFontColor[1], inData.DateFont[1], {0.5f, 0.5f}, inData.DateFontScale[1] ); }
            else            { UIAddTextWithFontBB( txtBB, std::to_string(day), inData.DateFontColor[0], inData.DateFont[0], {0.5f, 0.5f}, inData.DateFontScale[0] ); }
            if ( wday == 6 ) { row++; wday = 0; } else { wday++; }
            day++; count--;
        }

        // Next Month
        mon = inData.ViewMonth + 1; if ( mon > 12 ) { mon = 1; }
        year = mon==1?inData.ViewYear+1:inData.ViewYear;
        count = inData.DaysInMonth( mon, inData.IsLeapYear(year) );
        day = 1;
        while ( count > 0 ) {
            if ( row > 6 ) { break; }
            txtBB = GetBB( CalBB.Min.x + dDim.x*wday, CalBB.Min.y + dDim.y*row, dDim.x, dDim.y );
            if ( !Mute && VecInBB( mPos, txtBB ) ) {
                txtBB.Expand(-2);
                UIDrawAlphaBB( txtBB, inData.ShadeColor[0], inData.ShadeAlpha[0] );
                txtBB.Expand(2);
                if ( DragStart() ) { cYear = year; cMon = mon; cDay = day; }
            }
            UIAddAlphaTextWithFontBB( txtBB, std::to_string(day), inData.DateFontColor[0], txtAlpha, inData.DateFont[0], {0.5f, 0.5f}, inData.DateFontScale[0] );
            if ( wday == 6 ) { row++; wday = 0; } else { wday++; }
            day++; count--;
        }

        // Reset the Status
        inData.Status = UIDataCalendar::status_none;

        // Draw Time Options
        if ( inData.ShowTime && inData.TimeHeight[0] > 0 ) {
            
            ImRect BotBB = { inRect.Min.x, CalBB.Max.y, inRect.Max.x, CalBB.Max.y + inData.TimeHeight[0] };
            UIAddLine( BotBB.Min.x, BotBB.Min.y, BotBB.Max.x, BotBB.Min.y, inData.LineColor, 1 );
            UIAddLine( BotBB.Min.x, BotBB.Max.y, BotBB.Max.x, BotBB.Max.y, inData.LineColor, 1 );
            
            bool ClickCancel = false;
            string bText = "Cancel";
            vec2 bDim = UIGetTextDim( bText, inData.DateFont[0], inData.DateFontScale[0] );
            bDim.x += 9; bDim.y += 3;
            navBB = GetBB( BotBB.Min.x, BotBB.Max.y+(inData.TimeHeight[1]-bDim.y)*0.5f, bDim.x, bDim.y );
            if ( !Mute && VecInBB(mPos,navBB) ) {
                UIDrawAlphaRoundBB( navBB, inData.ShadeColor[0], inData.ShadeAlpha[0], navRound );
                if ( DragStart() ) { ClickCancel = true; }
            }
            UIAddTextWithFontBB( navBB, bText, inData.DateFontColor[0], inData.DateFont[0], {0.5f, 0.5f}, inData.DateFontScale[0] );

            bool ClickOK = false;
            bText = "Ok";
            bDim = UIGetTextDim( bText, inData.DateFont[0], inData.DateFontScale[0] );
            bDim.x += 9; bDim.y += 3;
            navBB = GetBB( BotBB.Max.x-bDim.x, BotBB.Max.y+(inData.TimeHeight[1]-bDim.y)*0.5f, bDim.x, bDim.y );
            if ( !Mute && VecInBB(mPos,navBB) ) {
                UIDrawAlphaRoundBB( navBB, inData.ShadeColor[0], inData.ShadeAlpha[0], navRound );
                if ( DragStart() ) { ClickOK = true; }
            }
            UIAddTextWithFontBB( navBB, bText, inData.DateFontColor[0], inData.DateFont[0], {0.5f, 0.5f}, inData.DateFontScale[0] );

            tDim.y += 6;
            tDim.x += 30;
            float spc = 9.0f;
            margin = ( BotBB.GetWidth() - tDim.x*3 - spc*2 ) * 0.5f;

            ImRect cRect;
            
            static vector<string> HList = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12" };
            cRect = GetBB( BotBB.Min.x+margin, BotBB.Min.y+(inData.TimeHeight[0]-tDim.y)*0.5f, tDim.x, tDim.y );
            UIComboBox * HComboBox = UIAddComboBox( inData.ComboID+"H", cRect, HList );
            
            static vector<string> MList = { "00", "01", "02", "03", "04", "05", "06", "07", "08", "09",
                                            "10", "11", "12", "13", "14", "15", "16", "17", "18", "19",
                                            "20", "21", "22", "23", "24", "25", "26", "27", "28", "29",
                                            "30", "31", "32", "33", "34", "35", "36", "37", "38", "39",
                                            "40", "41", "42", "43", "44", "45", "46", "47", "48", "49",
                                            "50", "51", "52", "53", "54", "55", "56", "57", "58", "59"  };
            cRect.TranslateX(tDim.x+spc);
            UIComboBox * MComboBox = UIAddComboBox( inData.ComboID+"M", cRect, MList );

            static vector<string> AList = { "AM", "PM" };
            cRect.TranslateX(tDim.x+spc);
            UIComboBox * AComboBox = UIAddComboBox( inData.ComboID+"A", cRect, AList, 0 );
            
            inData.SyncFromCombo();

            if  ( ClickCancel ) { inData.Status = UIDataCalendar::status_click_cancel; }
            else if ( ClickOK ) { inData.Status = UIDataCalendar::status_click_ok; }

        }

        if ( inData.Status != UIDataCalendar::status_none ) { return; }

        if ( cYear != -1 ) {
            inData.SetDate( cYear, cMon, cDay, inData.Hour, inData.Minute );
            inData.Status = UIDataCalendar::status_date_change;
        }
        else if ( navDir == -1 ) {
            mon = inData.ViewMonth-1; if ( mon < 1 ) { mon = 12; }
            year = mon==12?inData.ViewYear-1:inData.ViewYear;
            inData.SetView( year, mon );
            inData.Status = UIDataCalendar::status_view_change;
        }
        else if ( navDir == 1 ) {
            mon = inData.ViewMonth + 1; if ( mon > 12 ) { mon = 1; }
            year = mon==1?inData.ViewYear+1:inData.ViewYear;
            inData.SetView( year, mon );
            inData.Status = UIDataCalendar::status_view_change;
        }

    }

    void UIPlaceRect(float inX, float inY, ImRect inRect, int inXType, int inYType, unsigned int inC){
        
        // inXType:  0=Left, 1=Mid, 2=Right
        // inYType:  0=Top, 1=Mid, 2=Bottom
        ImVec2 rSize = inRect.GetSize();
        float xPos = inX;
        if(inXType == 1){ xPos -= rSize.x * 0.5f; }
        else if(inXType == 2){ xPos -= rSize.x; }
        float yPos = inY;
        if(inYType == 1){ yPos -= rSize.y * 0.5f; }
        else if(inYType == 2){ yPos -= rSize.y; }
        UIAddRect(xPos, yPos, rSize.x, rSize.y, inC, true, 1);

    }
    double ClipDouble(double n, double lower, double upper) {
        return std::max(lower, std::min(n, upper));
    }
    float ClipFloat(float n, float lower, float upper) {
        return std::max(lower, std::min(n, upper));
    }
    void UIColorHuePickerV( float inX, float inY, float inW,  float inH, float &inHue, bool &inDragging, bool &inMuteUI, int &inChange){
        
        // Pick Hue
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 wPos = window->Pos;
        const ImGuiID id = GImGui->CurrentWindow->GetID("##huePicker");
        const ImRect dragBB = GetGlobalBB( inX, inY, inW, inH);
        const bool over = GetOverState(dragBB);
        bool active = GetActiveState(over, id);
        float mouseX = GetGlobalMousePos().x;
        inChange = 0;
        if(over && !inMuteUI && DragStart()){
            inDragging = true;inChange=1;
        }
        else if(inDragging){
            if(MouseDown()){ // Currently Dragging the Bar
                float hx = mouseX - inX;
                hx = ClipFloat( hx, 0, inW);
                inHue = hx / inW;
            }
            else{inDragging = false;inChange=2;}
        }
        ImVec2 p1 = ImVec2(inX + wPos.x, inY + wPos.y);
        int al = 255;
        const ImU32 hueSteps[6 + 1] = { IM_COL32(255,0,0,al), IM_COL32(255,255,0,al), IM_COL32(0,255,0,al), IM_COL32(0,255,255,al), IM_COL32(0,0,255,al), IM_COL32(255,0,255,al), IM_COL32(255,0,0,al) };
        for (int i = 0; i < 6; ++i){
            drawList->AddRectFilledMultiColor(ImVec2(p1.x + i * (inW / 6), p1.y), ImVec2(p1.x + (i + 1) * (inW / 6), p1.y + inH), hueSteps[i], hueSteps[i + 1], hueSteps[i + 1], hueSteps[i]);
        }
    }
    void UIColorHuePicker( float inX, float inY, float inW,  float inH, float &inHue, bool &inDragging, bool &inMuteUI, int &inChange){
        
        // Pick Hue
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 wPos = window->Pos;
        const ImGuiID id = GImGui->CurrentWindow->GetID("##huePicker");
        const ImRect dragBB = GetGlobalBB( inX, inY, inW, inH);
        const bool over = GetOverState(dragBB);
        bool active = GetActiveState(over, id);
        float mouseY = GetGlobalMousePos().y;
        inChange = 0;
        if(over && !inMuteUI && DragStart() ){inDragging = true;inChange=1;}
        else if(inDragging){
            if(MouseDown()){ // Currently Dragging the Bar
                float hy = mouseY - inY;
                hy = ClipFloat( hy, 0, inH);
                inHue = hy / inH;
            }
            else{inDragging = false;inChange=2;}
        }
        ImVec2 p1 = ImVec2(inX + wPos.x, inY + wPos.y);
        int al = 255;
        const ImU32 hueSteps[6 + 1] = { IM_COL32(255,0,0,al), IM_COL32(255,255,0,al), IM_COL32(0,255,0,al), IM_COL32(0,255,255,al), IM_COL32(0,0,255,al), IM_COL32(255,0,255,al), IM_COL32(255,0,0,al) };
        for (int i = 0; i < 6; ++i){
            drawList->AddRectFilledMultiColor(ImVec2(p1.x, p1.y + i * (inH / 6)), ImVec2(p1.x + inW, p1.y + (i + 1) * (inH / 6)), hueSteps[i], hueSteps[i], hueSteps[i + 1], hueSteps[i + 1]);
        }
    
    }
    void UIColorSVPicker( float inX, float inY, float inW, float inH, float inHue, float &inSat, float &inVal, bool &inDragging, bool &inMuteUI, int &inChange){
        
        // Pick Saturation and Brightness
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 wPos = window->Pos;
        const ImGuiID id = GImGui->CurrentWindow->GetID("##satValPicker");
        ImVec2 p1 = ImVec2(inX + wPos.x, inY + wPos.y);
        ImVec2 p2 = ImVec2(p1.x + inW, p1.y + inH);
        const ImRect dragBB = GetGlobalBB( inX, inY, inW, inH);
        const bool over = GetOverState(dragBB);
        bool active = GetActiveState(over, id);
        //static bool inDragging = false;
        vec2 mouse = GetGlobalMousePos();
        inChange=0;
        if(over && !inMuteUI  && DragStart()){ // mouse down frame
            inDragging = true;inChange=1;
        }
        else if(inDragging){
            if(MouseDown()){ // Currently Dragging the Bar
                float valy = mouse.y - inY;
                valy = ClipFloat( valy, 0, inH);
                inVal = 1  - valy / inH;
                float satx = mouse.x - inX;
                satx = ClipFloat( satx, 0, inW);
                inSat = satx / inW;
            }
            else{inDragging = false;inChange=2;}
        }
        ImU32 whiteClr = Cl( 0xFFFFFF);
        ImU32 blackClr = Cl( 0x000000);
        float lR;float lG;float lB;
        ColorConvertHSVtoRGB(inHue, 1, 1, lR, lG, lB);
        ImU32 hueClr = ColorConvertFloat4ToU32( ImVec4(lR,lG,lB,1) );
        drawList->AddRectFilledMultiColor(p1, p2, whiteClr, hueClr, hueClr, whiteClr);
        drawList->AddRectFilledMultiColor(p1, p2, 0, 0, blackClr, blackClr);
    
    }
    void UIAddBorderRect(float inX, float inY, float inW, float inH, unsigned int inFillC, unsigned int inBorderC, float inThickness){
        
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 wPos = window->Pos;
        ImVec2 p1 = ImVec2(inX + wPos.x, inY + wPos.y);
        ImVec2 p2 = ImVec2(p1.x + inW, p1.y + inH);
        ImU32 fc = Cl( inFillC ); 
        ImU32 bc = Cl( inBorderC );
        drawList->AddRectFilled(p1, p2, fc);
        drawList->AddRect(p1, p2, bc, 0, 0, inThickness);
       
    }
    void UIAddTriangle(float inX, float inY, float inRadius, bool inOpen, unsigned int inC){
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 wPos = window->Pos;
        float dir = 0 ;
        if(inOpen){dir = 0.5f;}
        float startAngle = (float)(M_PI*2 + M_PI * dir);
        float r1 =  startAngle;
        float r2 = (float)(M_PI*2 * .333333333 + startAngle);
        float r3 = (float)(M_PI*2 * .333333333 * 2 + startAngle);
        ImVec2 p1 = ImVec2( cos(r1)*inRadius + inX + wPos.x, sin(r1)*inRadius + inY + wPos.y);
        ImVec2 p2 = ImVec2( cos(r2)*inRadius + inX + wPos.x, sin(r2)*inRadius + inY + wPos.y);
        ImVec2 p3 = ImVec2( cos(r3)*inRadius + inX + wPos.x, sin(r3)*inRadius + inY + wPos.y);
        ImU32 clr = UILib::Cl( inC );
        drawList->AddTriangleFilled(p1, p2, p3, clr);
    }
    void UIAddFlipTriangle(float inX, float inY, float inRadius, bool inOpen, unsigned int inC){
        // triangle flipped the opposite way of normal UI, typically right side collapsers
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 wPos = window->Pos;
        float dir = 1 ;
        if(inOpen){dir = 0.5f;}
        float startAngle = (float)(M_PI*2 + M_PI * dir);
        float r1 =  startAngle;
        float r2 = (float)(M_PI*2 * .333333333 + startAngle);
        float r3 = (float)(M_PI*2 * .333333333 * 2 + startAngle);
        ImVec2 p1 = ImVec2( cos(r1)*inRadius + inX + wPos.x, sin(r1)*inRadius + inY + wPos.y);
        ImVec2 p2 = ImVec2( cos(r2)*inRadius + inX + wPos.x, sin(r2)*inRadius + inY + wPos.y);
        ImVec2 p3 = ImVec2( cos(r3)*inRadius + inX + wPos.x, sin(r3)*inRadius + inY + wPos.y);
        ImU32 clr = UILib::Cl( inC );
        drawList->AddTriangleFilled(p1, p2, p3, clr);
    }
    void UIAddTriangleGlobal(float inX, float inY, float inRadius, bool inOpen, unsigned int inC){
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        float dir = 0 ;
        if(inOpen){dir = 0.5f;}
        float startAngle = (float)(M_PI*2 + M_PI * dir);
        float r1 =  startAngle;
        float r2 = (float)(M_PI*2 * .333333333 + startAngle);
        float r3 = (float)(M_PI*2 * .333333333 * 2 + startAngle);
        ImVec2 p1 = ImVec2( cos(r1)*inRadius + inX, sin(r1)*inRadius + inY);
        ImVec2 p2 = ImVec2( cos(r2)*inRadius + inX, sin(r2)*inRadius + inY);
        ImVec2 p3 = ImVec2( cos(r3)*inRadius + inX, sin(r3)*inRadius + inY);
        ImU32 clr = UILib::Cl( inC );
        drawList->AddTriangleFilled(p1, p2, p3, clr);
    }
    void UIAddTriangleGlobal( vec2 inCenter, float inB, float inH, char inDir, int inC ) {
        // Draws an isosceles triangle flat arrow head with specified triangle Base and Height.
        // inDir: 'u' = up, 'd' = down, 'l' = left, 'r' = right. All other chars default to up.
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        vec2 p1, p2, p3;
        char dir = tolower(inDir);
        if ( dir == 'd' ) {
            p1 = { inCenter.x - inB*0.5f, inCenter.y - inH*0.5f };
            p2 = { inCenter.x + inB*0.5f, inCenter.y - inH*0.5f };
            p3 = { inCenter.x           , inCenter.y + inH*0.5f };
        }
        else if ( dir == 'l' ) {
            p1 = { inCenter.x - inH*0.5f, inCenter.y            };
            p2 = { inCenter.x + inH*0.5f, inCenter.y - inB*0.5f };
            p3 = { inCenter.x + inH*0.5f, inCenter.y + inB*0.5f };
        }
        else if ( dir == 'r' ) {
            p1 = { inCenter.x - inH*0.5f, inCenter.y - inB*0.5f };
            p2 = { inCenter.x + inH*0.5f, inCenter.y            };
            p3 = { inCenter.x - inH*0.5f, inCenter.y + inB*0.5f };
        }
        else {
            p1 = { inCenter.x - inB*0.5f, inCenter.y + inH*0.5f };
            p2 = { inCenter.x           , inCenter.y - inH*0.5f };
            p3 = { inCenter.x + inB*0.5f, inCenter.y + inH*0.5f };
        }
        ImU32 clr = UILib::Cl( inC );
        drawList->AddTriangleFilled(p1, p2, p3, clr);
    }
    void UIAddTriangleGlobalAlpha( vec2 inCenter, float inB, float inH, char inDir, int inC, float inAlpha ) {
        // Draws an isosceles triangle flat arrow head with specified triangle Base and Height.
        // inDir: 'u' = up, 'd' = down, 'l' = left, 'r' = right. All other chars default to up.
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        vec2 p1, p2, p3;
        char dir = tolower(inDir);
        if ( dir == 'd' ) {
            p1 = { inCenter.x - inB*0.5f, inCenter.y - inH*0.5f };
            p2 = { inCenter.x + inB*0.5f, inCenter.y - inH*0.5f };
            p3 = { inCenter.x           , inCenter.y + inH*0.5f };
        }
        else if ( dir == 'l' ) {
            p1 = { inCenter.x - inH*0.5f, inCenter.y            };
            p2 = { inCenter.x + inH*0.5f, inCenter.y - inB*0.5f };
            p3 = { inCenter.x + inH*0.5f, inCenter.y + inB*0.5f };
        }
        else if ( dir == 'r' ) {
            p1 = { inCenter.x - inH*0.5f, inCenter.y - inB*0.5f };
            p2 = { inCenter.x + inH*0.5f, inCenter.y            };
            p3 = { inCenter.x - inH*0.5f, inCenter.y + inB*0.5f };
        }
        else {
            p1 = { inCenter.x - inB*0.5f, inCenter.y + inH*0.5f };
            p2 = { inCenter.x           , inCenter.y - inH*0.5f };
            p3 = { inCenter.x + inB*0.5f, inCenter.y + inH*0.5f };
        }
        ImU32 clr = ClWithAlpha(inC, inAlpha);
        drawList->AddTriangleFilled(p1, p2, p3, clr);
    }
    void UIAddFilledAlphaPolyG(vector<vec2> inPnts, unsigned int inC , float inAlpha){
        // global verions, takes global points
        //Anti-aliased filling requires points to be in clockwise order
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        size_t n = inPnts.size();
        vector<ImVec2> pList;
        for(int i=0;i<n;i++){pList.push_back({inPnts[i].x, inPnts[i].y});}    

        ImVec2* pListArr = &pList[0];
        ImU32 clr = ClWithAlpha(inC, inAlpha);

        drawList->AddConvexPolyFilled(pListArr, (int) n, clr);

    }
    void UIAddFilledAlphaPoly(vector<vec2> inPnts, unsigned int inC , float inAlpha){
        // local verions, takes points local to window
        //Anti-aliased filling requires points to be in clockwise order
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 wPos = window->Pos;

        size_t n = inPnts.size();
        vector<ImVec2> pList;
        for(int i=0;i<n;i++){
            pList.push_back({inPnts[i].x + wPos.x, inPnts[i].y  + wPos.y});
        }    

        ImVec2* pListArr = &pList[0];
        ImU32 clr = ClWithAlpha(inC, inAlpha);

        drawList->AddConvexPolyFilled(pListArr, (int) n, clr);

    }
    void UIAddFilledPolyline(vector<vec2> inPnts, unsigned int inC ){

        //Anti-aliased filling requires points to be in clockwise order
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 wPos = window->Pos;

        size_t n = inPnts.size();
        vector<ImVec2> pList;
        for(int i=0;i<n;i++){
            pList.push_back({inPnts[i].x + wPos.x, inPnts[i].y  + wPos.y});
        }    

        ImVec2* pListArr = &pList[0];
        ImU32 clr = UILib::Cl( inC );

        drawList->AddConvexPolyFilled(pListArr, (int) n, clr);

    }
    
    void UIAddPolylineG(vector<vec2> inPnts, unsigned int inC, float inThickness, bool inClosed ){
        
        //Anti-aliased filling requires points to be in clockwise order
        ImGuiContext& g = *GImGui;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
          
        size_t n = inPnts.size();
        vector<ImVec2> pList;
        for(int i=0;i<n;i++){
            pList.push_back({inPnts[i].x, inPnts[i].y});
        }    

        ImVec2* pListArr = &pList[0];
        ImU32 clr = UILib::Cl( inC );

        drawList->AddPolyline(pListArr, (int) n, clr, inClosed, inThickness);

    }
    void UIAddPolyline(vector<vec2> inPnts, unsigned int inC, float inThickness, bool inClosed ){
        
        //Anti-aliased filling requires points to be in clockwise order
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 wPos = window->Pos;

        size_t n = inPnts.size();
        vector<ImVec2> pList;
        for(int i=0;i<n;i++){
            pList.push_back({inPnts[i].x + wPos.x, inPnts[i].y  + wPos.y});
        }    

        ImVec2* pListArr = &pList[0];
        ImU32 clr = UILib::Cl( inC );

        drawList->AddPolyline(pListArr, (int) n, clr, inClosed, inThickness);

    }
    void UIAddRoundCornerRect(float inX, float inY, float inW, float inH, unsigned int inC, bool inFill, float inThickness, float inRounding, std::string cornerBits){
        
        std::bitset<4> bDef(cornerBits);
        unsigned int cornerFlags = bDef.to_ulong();
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 wPos = window->Pos;
        ImU32 clr = UILib::Cl( inC );  
        ImVec2 p1 = ImVec2(inX + wPos.x, inY + wPos.y);
        ImVec2 p2 = ImVec2(p1.x + inW, p1.y + inH);
        if(inFill){
            drawList->AddRectFilled(p1, p2, clr, inRounding, cornerFlags);
        }
        else{
            drawList->AddRect(p1, p2, clr, inRounding, cornerFlags, inThickness);
        }
        


    }
    void UIAddCircle(float inX, float inY, float inRadius, unsigned int inC, bool inFill, float inThickness, int nbSegments){
        
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 wPos = window->Pos;
        ImVec2 center = ImVec2(inX + wPos.x, inY + wPos.y);
        ImU32 clr = UILib::Cl( inC );  
        
        if(inFill){
           drawList-> AddCircleFilled(center, inRadius, clr, nbSegments);
        }
        else{
            drawList->AddCircle(center, inRadius, clr, nbSegments, inThickness);
        }
        
    }

    void UIAddGlobalCircle(float inX, float inY, float inRadius, unsigned int inC, bool inFill, float inThickness, int nbSegments) {
        
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 center = ImVec2(inX, inY);
        ImU32 clr = UILib::Cl( inC );  
        
        if(inFill){
           drawList-> AddCircleFilled(center, inRadius, clr, nbSegments);
        }
        else{
            drawList->AddCircle(center, inRadius, clr, nbSegments, inThickness);
        }
    }

    void UIAddAlphaCircle(float inX, float inY, float inRadius, unsigned int inC, bool inFill, float inAlpha, float inThickness, int nbSegments){
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 wPos = window->Pos;
        ImVec2 center = ImVec2(inX + wPos.x, inY + wPos.y);
        
        //ImU32 clr = UILib::Cl( inC );  
        ImU32 clr = ClWithAlpha(inC, inAlpha);

        if(inFill){
           drawList-> AddCircleFilled(center, inRadius, clr, nbSegments);
        }
        else{
            drawList->AddCircle(center, inRadius, clr, nbSegments, inThickness);
        }

    }

    void UIAddGlobalCircleAlpha(float inX, float inY, float inRadius, unsigned int inC, bool inFill, float inAlpha, float inThickness, int nbSegments) {
        
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 center = ImVec2(inX, inY);
        ImU32 clr = ClWithAlpha(inC, inAlpha); 
        
        if(inFill){
           drawList-> AddCircleFilled(center, inRadius, clr, nbSegments);
        }
        else{
            drawList->AddCircle(center, inRadius, clr, nbSegments, inThickness);
        }
    }

    void UIDrawArcAlpha(vec2 inCenter, float inRadius, double startAng, double endAng, unsigned int inC, bool inFill, float inAlpha, float inThickness, int nbSegments ) {
        // start and endAng are in radians
        
        const float aaSize = 1.0f;
        const float radius = inRadius - aaSize * 0.5f;
        const bool hasLine = ( inThickness > 0.0f );
        double sliceAng = ( endAng - startAng ) / nbSegments;
        double ang = startAng;
        vec2 dir = vec2( cos(ang), sin(ang) );
        vec2 p0 = inCenter + dir * radius;
        vec2 p1;
        vec2 q0 = p0 + dir * aaSize;
        vec2 q1;
        vector<vec2> pL;
        if ( hasLine ) {
            pL.reserve( nbSegments );
            pL.push_back( p0 );
        }
        
        for (int i = 0; i < nbSegments; i++ ) {
            ang = startAng + (i+1)*sliceAng;
            dir = vec2( cos(ang), sin(ang) );
            p1 = inCenter + dir * radius;
            q1 = p1 + dir * aaSize;
            if ( inFill ) {
                UIAddTriPrim( inCenter, p0, p1, inC, inAlpha);
                UIAddQuadPrim( p0, q0, q1, p1, inC, inC, inC, inC, inAlpha, 0, 0, inAlpha );
            }
            if ( hasLine ) {
                pL.push_back( p1 );
            }
            p0 = p1;
            q0 = q1;
        }

        if ( hasLine ) { 
            UIAddGlobalPolylineAlpha(pL, inC, inThickness, inAlpha, false ); 
        }
    }

    void UIDrawGradientArcAlpha(vec2 inCenter, float inRadius, double startAng, double endAng, const vector<int> & inClrs, int inDir, float inAlpha, int nbSegments, int nbRDivs ) {
        // inDir is the gradient direction 0 = horizontal (left to right), 1 = vertical (top to bottom)
        assert( inDir == 0 || inDir == 1 );

        if ( inClrs.size() < 2 ) {
            if ( inClrs.empty() ) { return; }
            UIDrawArcAlpha( inCenter, inRadius, startAng, endAng, inClrs[0], true, inAlpha, 0, nbSegments );
            return;
        }

        ImRect arcBB = GetArcBoundingBB( inCenter, inRadius, startAng, endAng );
        // UIDrawBBOutline( arcBB, 0x00FF00, 4 );
        
        float range = arcBB.Max[inDir] - arcBB.Min[inDir];
        if ( range <= 1.0f ) {  // arc angle is so small that it appears as a line
            if ( inDir == 0 ) {
                UIAddQuadPrim( arcBB.GetTL(), arcBB.GetTR(), arcBB.GetBR(), arcBB.GetBL(), inClrs.front(), inClrs.back(), inClrs.back(), inClrs.front(), inAlpha, inAlpha, inAlpha, inAlpha );
            }
            else {
                UIAddQuadPrim( arcBB.GetTL(), arcBB.GetTR(), arcBB.GetBR(), arcBB.GetBL(), inClrs.front(), inClrs.front(), inClrs.back(), inClrs.back(), inAlpha, inAlpha, inAlpha, inAlpha );
            }
            return;
        }

        const float aaSize = 1.0f;
        const float radius = inRadius - aaSize * 0.5f;

        if ( nbRDivs < 2 ) { nbRDivs = 2; }
        double sliceAng = ( endAng - startAng ) / nbSegments;
        double ang = startAng;
        float incrR = radius / nbRDivs;
        vec2 dir0 = vec2( cos(ang), sin(ang) );
        vec2 dir1;
        vec2 p00, p01, p11, p10;    // rotation order: 00, 01, 11, 10
        int c00, c01, c11, c10;

        for (int i = 0; i < nbSegments; i++ ) {
            
            ang = startAng + (i+1)*sliceAng;
            dir1 = vec2( cos(ang), sin(ang) );

            // Draw the inner triangle
            p00 = inCenter + dir0*incrR; c00 = UIGetBlendClr( inClrs, ( p00[inDir] - arcBB.Min[inDir] ) / range );
            p10 = inCenter + dir1*incrR; c10 = UIGetBlendClr( inClrs, ( p10[inDir] - arcBB.Min[inDir] ) / range );
            c11 = UIGetBlendClr( inClrs, ( inCenter[inDir] - arcBB.Min[inDir] ) / range );  // use c11 for the center color
            UIAddTriPrim( inCenter, p00, p10, c11, c00, c10, inAlpha, inAlpha, inAlpha );
            // UIAddGlobalPolylineAlpha( { inCenter, p10, p00 }, 0x00FFFF, 3, 1, true );
            
            // Draw the outer quads
            for ( int j = 1; j < nbRDivs; j++ ) {
                p01 = inCenter + dir0*incrR*(float)(j+1); c01 = UIGetBlendClr( inClrs, ( p01[inDir] - arcBB.Min[inDir] ) / range );
                p11 = inCenter + dir1*incrR*(float)(j+1); c11 = UIGetBlendClr( inClrs, ( p11[inDir] - arcBB.Min[inDir] ) / range );
                UIAddQuadPrim( p00, p01, p11, p10, c00, c01, c11, c10, inAlpha, inAlpha, inAlpha, inAlpha );
                // UIAddGlobalPolylineAlpha( { p00, p01, p11, p10 }, 0x00FFFF, 3, 1, true );
                p00 = p01; c00 = c01;
                p10 = p11; c10 = c11;
            }

            // Draw the AA quad
            p01 = p01 + dir0*aaSize;
            p11 = p11 + dir1*aaSize;
            UIAddQuadPrim( p00, p01, p11, p10, c00, c01, c11, c10, inAlpha, 0, 0, inAlpha );

            dir0 = dir1;
        }

    }

    void UIDrawGradientArcAlpha(vec2 inCenter, float inRadius, double startAng, double endAng, std::function<int(float)> clrFunc, int inDir, float inAlpha, int nbSegments, int nbRDivs ) {
        // inDir is the gradient direction 0 = horizontal (left to right), 1 = vertical (top to bottom)
        // clrFunc is a function that takes the perecentage and returns a color
        assert( inDir == 0 || inDir == 1 );

        ImRect arcBB = GetArcBoundingBB( inCenter, inRadius, startAng, endAng );
        // UIDrawBBOutline( arcBB, 0x00FF00, 4 );
        
        float range = arcBB.Max[inDir] - arcBB.Min[inDir];
        if ( range <= 1.0f ) {  // arc angle is so small that it appears as a line
            int c0 = clrFunc(0);
            int c1 = clrFunc(1);
            if ( inDir == 0 ) {
                UIAddQuadPrim( arcBB.GetTL(), arcBB.GetTR(), arcBB.GetBR(), arcBB.GetBL(), c0, c1, c1, c0, inAlpha, inAlpha, inAlpha, inAlpha );
            }
            else {
                UIAddQuadPrim( arcBB.GetTL(), arcBB.GetTR(), arcBB.GetBR(), arcBB.GetBL(), c0, c0, c1, c1, inAlpha, inAlpha, inAlpha, inAlpha );
            }
            return;
        }

        const float aaSize = 1.0f;
        const float radius = inRadius - aaSize * 0.5f;

        if ( nbRDivs < 2 ) { nbRDivs = 2; }
        double sliceAng = ( endAng - startAng ) / nbSegments;
        double ang = startAng;
        float incrR = radius / nbRDivs;
        vec2 dir0 = vec2( cos(ang), sin(ang) );
        vec2 dir1;
        vec2 p00, p01, p11, p10;    // rotation order: 00, 01, 11, 10
        int c00, c01, c11, c10;

        for (int i = 0; i < nbSegments; i++ ) {
            
            ang = startAng + (i+1)*sliceAng;
            dir1 = vec2( cos(ang), sin(ang) );

            // Draw the inner triangle
            p00 = inCenter + dir0*incrR; c00 = clrFunc( ( p00[inDir] - arcBB.Min[inDir] ) / range );
            p10 = inCenter + dir1*incrR; c10 = clrFunc( ( p10[inDir] - arcBB.Min[inDir] ) / range );
            c11 = clrFunc( ( inCenter[inDir] - arcBB.Min[inDir] ) / range );  // use c11 for the center color
            UIAddTriPrim( inCenter, p00, p10, c11, c00, c10, inAlpha, inAlpha, inAlpha );
            // UIAddGlobalPolylineAlpha( { inCenter, p10, p00 }, 0x00FFFF, 3, 1, true );
            
            // Draw the outer quads
            for ( int j = 1; j < nbRDivs; j++ ) {
                p01 = inCenter + dir0*incrR*(float)(j+1); c01 = clrFunc( ( p01[inDir] - arcBB.Min[inDir] ) / range );
                p11 = inCenter + dir1*incrR*(float)(j+1); c11 = clrFunc( ( p11[inDir] - arcBB.Min[inDir] ) / range );
                UIAddQuadPrim( p00, p01, p11, p10, c00, c01, c11, c10, inAlpha, inAlpha, inAlpha, inAlpha );
                // UIAddGlobalPolylineAlpha( { p00, p01, p11, p10 }, 0x00FFFF, 3, 1, true );
                p00 = p01; c00 = c01;
                p10 = p11; c10 = c11;
            }

            // Draw the AA quad
            p01 = p01 + dir0*aaSize;
            p11 = p11 + dir1*aaSize;
            UIAddQuadPrim( p00, p01, p11, p10, c00, c01, c11, c10, inAlpha, 0, 0, inAlpha );

            dir0 = dir1;
        }
    }

    void UIDrawRadialGradientArcAlpha(vec2 inCenter, float inRadius, double startAng, double endAng, const vector<int> & inClrs, const vector<float> & inAlphas, int nbSegments, int nbRDivs ) {
        // inClrs and inAlphas in order from inner to outer

        assert( inClrs.size() == inAlphas.size() && inClrs.size() >= 2 && "UIDrawRadialGradientArcAlpha requires at least two colors and alphas." );

        const float aaSize = ( inAlphas.back() > 0 ? 1.0f : 0 ); // use AA only if the outer alpha is not 0
        const float radius = inRadius - aaSize * 0.5f;

        if ( nbRDivs < 2 ) { nbRDivs = 2; }
        double sliceAng = ( endAng - startAng ) / nbSegments;
        double ang = startAng;
        float incrR = radius / nbRDivs;
        float incrPct = 0;
        vec2 dir0 = vec2( cos(ang), sin(ang) );
        vec2 dir1;
        vec2 p00, p01, p11, p10;    // rotation order: 00, 01, 11, 10
        int c00, c01, c11, c10;
        float a00, a01, a11, a10;

        for (int i = 0; i < nbSegments; i++ ) {
            
            ang = startAng + (i+1)*sliceAng;
            dir1 = vec2( cos(ang), sin(ang) );

            // Draw the inner triangle
            incrPct = 1.0f / (float) nbRDivs;
            p00 = inCenter + dir0*incrR; c00 = UIGetBlendClr( inClrs, incrPct ); a00 = UIGetBlendVal(inAlphas, incrPct );
            p10 = inCenter + dir1*incrR; c10 = c00; a10 = a00;
            c11 = inClrs[0]; a11 = inAlphas[0];  // use c11 for the center color
            UIAddTriPrim( inCenter, p00, p10, c11, c00, c10, a11, a00, a10 );
            // UIAddGlobalPolylineAlpha( { inCenter, p10, p00 }, 0x00FFFF, 3, 1, true );
            
            // Draw the outer quads
            for ( int j = 1; j < nbRDivs; j++ ) {
                incrPct = (float)(j+1) / (float) nbRDivs;
                p01 = inCenter + dir0*incrR*(float)(j+1); c01 = UIGetBlendClr( inClrs, incrPct ); a01 = UIGetBlendVal(inAlphas, incrPct );
                p11 = inCenter + dir1*incrR*(float)(j+1); c11 = c01; a11 = a01;
                UIAddQuadPrim( p00, p01, p11, p10, c00, c01, c11, c10, a00, a01, a11, a10 );
                // UIAddGlobalPolylineAlpha( { p00, p01, p11, p10 }, 0x00FFFF, 3, 1, true );
                p00 = p01; c00 = c01; a00 = a01;
                p10 = p11; c10 = c11; a10 = a11;
            }

            // Draw the AA quad
            if ( aaSize > 0 ) {
                p01 = p01 + dir0*aaSize;
                p11 = p11 + dir1*aaSize;
                UIAddQuadPrim( p00, p01, p11, p10, c00, c01, c11, c10, a00, 0, 0, a10 );
            }

            dir0 = dir1;
        }
        
    }

    void UIAddRect(float inX, float inY, float inW, float inH, unsigned int inC, bool inFill, float inThickness){
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 wPos = window->Pos;
        ImU32 clr = UILib::Cl( inC );  
        ImVec2 p1 = ImVec2(inX + wPos.x, inY + wPos.y);
        ImVec2 p2 = ImVec2(p1.x + inW, p1.y + inH);
        if(inFill){
            drawList->AddRectFilled(p1, p2, clr);
        }
        else{
            drawList->AddRect(p1, p2, clr, 0, 0, inThickness);
        }
    }

    void UIAddRectAlpha(float inX, float inY, float inW, float inH, unsigned int inC, bool inFill, float inThickness, float inAlpha) {
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 wPos = window->Pos;
        ImU32 clr = ClWithAlpha(inC, inAlpha);
        ImVec2 p1 = ImVec2(inX + wPos.x, inY + wPos.y);
        ImVec2 p2 = ImVec2(p1.x + inW, p1.y + inH);
        if(inFill){
            drawList->AddRectFilled(p1, p2, clr);
        }
        else{
            drawList->AddRect(p1, p2, clr, 0, 0, inThickness);
        }
    }

    void UIAddAlphaRoundRect(float inX, float inY, float inW, float inH, unsigned int inC, float inAlpha, bool inFill, float inThickness, float inRounding){

        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 wPos = window->Pos;
        ImU32 clr = ClWithAlpha(inC, inAlpha);
        ImVec2 p1 = ImVec2(inX + wPos.x, inY + wPos.y);
        ImVec2 p2 = ImVec2(p1.x + inW, p1.y + inH);
        if(inFill){
            drawList->AddRectFilled(p1, p2, clr, inRounding, 15);
        }
        else{
            drawList->AddRect(p1, p2, clr, inRounding, 15, inThickness);
        }


    }
    void UIAddAlphaRoundRect(float inX, float inY, float inW, float inH, unsigned int inC, float inAlpha, bool inFill, float inThickness, float inRounding, ImDrawCornerFlags corners){

        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 wPos = window->Pos;
        ImU32 clr = ClWithAlpha(inC, inAlpha);
        ImVec2 p1 = ImVec2(inX + wPos.x, inY + wPos.y);
        ImVec2 p2 = ImVec2(p1.x + inW, p1.y + inH);
        if(inFill){
            drawList->AddRectFilled(p1, p2, clr, inRounding, corners);
        }
        else{
            drawList->AddRect(p1, p2, clr, inRounding, corners, inThickness);
        }


    }
    void UIAddGlobalAlphaRoundRect(float inX, float inY, float inW, float inH, unsigned int inC, float inAlpha, bool inFill, float inThickness, float inRounding, ImDrawCornerFlags corners){

        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImU32 clr = ClWithAlpha(inC, inAlpha);
        ImVec2 p1 = ImVec2(inX ,inY );
        ImVec2 p2 = ImVec2(p1.x + inW, p1.y + inH);
        if(inFill){
            drawList->AddRectFilled(p1, p2, clr, inRounding, corners);
        }
        else{
            drawList->AddRect(p1, p2, clr, inRounding, corners, inThickness);
        }


    }
    void UIAddRoundRect(float inX, float inY, float inW, float inH, unsigned int inC, bool inFill, float inThickness, float inRounding){

        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 wPos = window->Pos;
        ImU32 clr = UILib::Cl( inC );  
        ImVec2 p1 = ImVec2(inX + wPos.x, inY + wPos.y);
        ImVec2 p2 = ImVec2(p1.x + inW, p1.y + inH);
        if(inFill){
            drawList->AddRectFilled(p1, p2, clr, inRounding, 15);
        }
        else{
            drawList->AddRect(p1, p2, clr, inRounding, 15, inThickness);
        }
    }
    void UIAddRoundRect(float inX, float inY, float inW, float inH, unsigned int inC, bool inFill, float inThickness, float inRounding, ImDrawCornerFlags corners){

        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 wPos = window->Pos;
        ImU32 clr = UILib::Cl( inC );  
        ImVec2 p1 = ImVec2(inX + wPos.x, inY + wPos.y);
        ImVec2 p2 = ImVec2(p1.x + inW, p1.y + inH);
        if(inFill){
            drawList->AddRectFilled(p1, p2, clr, inRounding, corners);
        }
        else{
            drawList->AddRect(p1, p2, clr, inRounding, corners, inThickness);
        }
    }
    void UIAddButton(ImRect inRect, string inText, int inC, int inOverC, int inTextC, float inRounding, bool &inButtonDown, const bool & inMute){
        
        const bool overB = GetOverState(inRect);
        int clr = inC;
        if(!inMute && overB){clr = inOverC;}
        UIDrawRoundBB(inRect, clr, inRounding);
        const char* charStr = inText.c_str();
        ImVec2 tSize =  CalcTextSize(charStr);
        float xPos = inRect.Min.x + (inRect.GetWidth() - tSize.x) * .5f;
        float yPos = inRect.Min.y + (inRect.GetHeight() - tSize.y) * .5f;
        UIAddGlobalText(xPos, yPos, tSize.x, tSize.y, inText, inTextC, {.5,.5});
        if(!inMute && overB && DragStart()){inButtonDown = true;}
        else{inButtonDown = false;}

    }

    ImRect UIGetUniformFitBB( const ImRect & inContainer, float inW, float inH ) {
        // Adapted from UIFormImage treatment of imgfit_uniform type
        float imageAspect = inW / inH;
        float contentW = inContainer.GetWidth();
        float contentH = inContainer.GetHeight();
        float bbW = contentW;
        float bbH = bbW / imageAspect;
        if ( bbH > contentH ) {
            bbH = contentH;
            bbW = bbH * imageAspect;
            return { inContainer.Min.x + (contentW-bbW)*0.5f, inContainer.Min.y, inContainer.Max.x - (contentW-bbW)*0.5f, inContainer.Max.y };  // Fits to container height
        }
        return { inContainer.Min.x, inContainer.Min.y + (contentH-bbH)*0.5f, inContainer.Max.x, inContainer.Max.y - (contentH-bbH)*0.5f };      // Fits to container width
    }    

    ImRect FitBoxinBB(ImRect inBox, ImRect inContainer){
        ImRect fitBox = inBox;
        if(inBox.Min.x < inContainer.Min.x){
            float xShift = inContainer.Min.x - inBox.Min.x;
            fitBox.Min.x += xShift;
            fitBox.Max.x += xShift;
        }
        else if(inBox.Max.x > inContainer.Max.x){
            float yShift = inBox.Max.x - inContainer.Max.x;
            fitBox.Min.x -= yShift;
            fitBox.Max.x -= yShift;
        }

       if(inBox.Min.y < inContainer.Min.y){
            float xShift = inContainer.Min.y - inBox.Min.y;
            fitBox.Min.y += xShift;
            fitBox.Max.y += xShift;
        }
        else if(inBox.Max.y > inContainer.Max.y){
            float yShift = inBox.Max.y - inContainer.Max.y;
            fitBox.Min.y -= yShift;
            fitBox.Max.y -= yShift;
        }
        return fitBox;
        
    }

    void FitBBinBB( ImRect & inBB, const ImRect & boundingBB ) {
        if ( inBB.Min.x < boundingBB.Min.x ) {
            float xShift = boundingBB.Min.x - inBB.Min.x;
            inBB.Min.x += xShift;
            inBB.Max.x += xShift;
        }
        else if ( inBB.Max.x > boundingBB.Max.x ) {
            float xShift = inBB.Max.x - boundingBB.Max.x;
            inBB.Min.x -= xShift;
            inBB.Max.x -= xShift;
        }

       if ( inBB.Min.y < boundingBB.Min.y ) {
            float yShift = boundingBB.Min.y - inBB.Min.y;
            inBB.Min.y += yShift;
            inBB.Max.y += yShift;
        }
        else if ( inBB.Max.y > boundingBB.Max.y ) {
            float yShift = inBB.Max.y - boundingBB.Max.y;
            inBB.Min.y -= yShift;
            inBB.Max.y -= yShift;
        }
    }

    bool BBIsSame( const ImRect & inA, const ImRect & inB ) {
        if ( inA.Min.x != inB.Min.x || inA.Min.y != inB.Min.y || inA.Max.x != inB.Max.x || inA.Max.y != inB.Max.y ) { return false; }
        return true;
    }
        
    void UIAddArrowLine(float inX1, float inY1, float inX2, float inY2, unsigned int inC, float inThickness, float inW_Arrow, float inH_Arrow){
         
        UIAddLine(inX1, inY1, inX2, inY2, inC, inThickness);
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 wPos = window->Pos;
        vec2 aVec = vec2(inX2 - inX1, inY2 - inY1);
        vec2 nVec = normalize( aVec );
        vec2 v1 = glm::rotate( nVec, float(0.5f * M_PI) );
        vec2 v2 = glm::rotate( nVec, float(-0.5f * M_PI) );
        vec2 arrowBase = nVec * inH_Arrow * -1.0f ;
        vec2 tip = vec2(inX2  + wPos.x, inY2 + wPos.y);
        vec2 basePoint = tip + arrowBase;
        v1 = normalize(v1) * inW_Arrow * 0.5f ;
        v2 = normalize(v2) * inW_Arrow * 0.5f ;
        vec2 p1 = basePoint + v1;
        vec2 p2 = basePoint + v2;
        ImVec2 imP1 = ImVec2(p1.x, p1.y);
        ImVec2 imP2 = ImVec2(tip.x, tip.y);
        ImVec2 imP3 = ImVec2(p2.x, p2.y);
        ImU32 clr = UILib::Cl( inC );
         // Note: Anti-aliased filling requires points to be in clockwise order.
        drawList->AddTriangleFilled(imP3, imP2, imP1, clr);

    }

    void UIAddChevronLeft( const ImRect & inRect, unsigned int inC, float inAlpha, float inThickness) {
        vector<vec2> p = { {inRect.Max.x, inRect.Min.y}, {inRect.Min.x, inRect.GetCenter().y}, {inRect.Max.x, inRect.Max.y} };
        UIAddGlobalPolylineAlpha( p, inC, inThickness, inAlpha, false );
    }

    void UIAddChevronRight( const ImRect & inRect, unsigned int inC, float inAlpha, float inThickness) {
        vector<vec2> p = { {inRect.Min.x, inRect.Min.y}, {inRect.Max.x, inRect.GetCenter().y}, {inRect.Min.x, inRect.Max.y} };
        UIAddGlobalPolylineAlpha( p, inC, inThickness, inAlpha, false );
    }

    
    void UIAddQuadPrim( vec2 A, vec2 B, vec2 C, vec2 D, int clA, int clB, int clC, int clD, float aA, float aB, float aC, float aD ) {
         // A, B, C, D are clockwise quad points
         // NOTE: You have to set the UV to dl->_Data->TexUvWhitePixel otherwise, DX11 version will not render alphas correctly.

        ImDrawList* dl = ImGui::GetWindowDrawList();

        dl->PrimReserve(6, 4);

        // Code below is modified from ImDrawList::PrimRectUV() functions
        ImDrawIdx idx = (ImDrawIdx)dl->_VtxCurrentIdx;
        dl->_IdxWritePtr[0] = idx;  dl->_IdxWritePtr[1] = (ImDrawIdx)(idx+1);   dl->_IdxWritePtr[2] = (ImDrawIdx)(idx+2);
        dl->_IdxWritePtr[3] = idx;  dl->_IdxWritePtr[4] = (ImDrawIdx)(idx+2);   dl->_IdxWritePtr[5] = (ImDrawIdx)(idx+3);
        dl->_VtxWritePtr[0].pos = A;    dl->_VtxWritePtr[0].uv = dl->_Data->TexUvWhitePixel;      dl->_VtxWritePtr[0].col = ClWithAlpha( clA, aA );      dl->_VtxWritePtr[0].texID = -1;
        dl->_VtxWritePtr[1].pos = B;    dl->_VtxWritePtr[1].uv = dl->_Data->TexUvWhitePixel;      dl->_VtxWritePtr[1].col = ClWithAlpha( clB, aB );      dl->_VtxWritePtr[1].texID = -1;
        dl->_VtxWritePtr[2].pos = C;    dl->_VtxWritePtr[2].uv = dl->_Data->TexUvWhitePixel;      dl->_VtxWritePtr[2].col = ClWithAlpha( clC, aC );      dl->_VtxWritePtr[2].texID = -1;
        dl->_VtxWritePtr[3].pos = D;    dl->_VtxWritePtr[3].uv = dl->_Data->TexUvWhitePixel;      dl->_VtxWritePtr[3].col = ClWithAlpha( clD, aD );      dl->_VtxWritePtr[3].texID = -1;
        dl->_VtxWritePtr += 4;
        dl->_VtxCurrentIdx += 4;
        dl->_IdxWritePtr += 6;
    }

    void UIAddTriPrim( vec2 A, vec2 B, vec2 C, int clA, int clB, int clC, float aA, float aB, float aC ) {
        // A, B, C are clockwise triangle points
        // NOTE: You have to set the UV to dl->_Data->TexUvWhitePixel otherwise, DX11 version will not render alphas correctly.

        ImGuiContext& g = *GImGui;ImGuiWindow* window = g.CurrentWindow; 
        ImDrawList* dl = ImGui::GetWindowDrawList();

        dl->PrimReserve(3, 3);
        // Code below is modified from ImDrawList::PrimRectUV() functions
        ImDrawIdx idx = (ImDrawIdx)dl->_VtxCurrentIdx;
        dl->_IdxWritePtr[0] = idx;  dl->_IdxWritePtr[1] = (ImDrawIdx)(idx+1);   dl->_IdxWritePtr[2] = (ImDrawIdx)(idx+2);
        dl->_VtxWritePtr[0].pos = A;    dl->_VtxWritePtr[0].uv = dl->_Data->TexUvWhitePixel;      dl->_VtxWritePtr[0].col = ClWithAlpha( clA, aA );      dl->_VtxWritePtr[0].texID = -1;
        dl->_VtxWritePtr[1].pos = B;    dl->_VtxWritePtr[1].uv = dl->_Data->TexUvWhitePixel;      dl->_VtxWritePtr[1].col = ClWithAlpha( clB, aB );      dl->_VtxWritePtr[1].texID = -1;
        dl->_VtxWritePtr[2].pos = C;    dl->_VtxWritePtr[2].uv = dl->_Data->TexUvWhitePixel;      dl->_VtxWritePtr[2].col = ClWithAlpha( clC, aC );      dl->_VtxWritePtr[2].texID = -1;
        dl->_VtxWritePtr += 3;
        dl->_VtxCurrentIdx += 3;
        dl->_IdxWritePtr += 3;
    }
    
    void UIAddTriPrim( vec2 A, vec2 B, vec2 C, int cl, float a) {
        // A, B, C are clockwise triangle points

        ImGuiContext& g = *GImGui;ImGuiWindow* window = g.CurrentWindow; 
        ImDrawList* dl = ImGui::GetWindowDrawList();

        dl->PrimReserve(3, 3);

        // Code below is modified from ImDrawList::PrimRectUV() functions
        ImDrawIdx idx = (ImDrawIdx)dl->_VtxCurrentIdx;
        dl->_IdxWritePtr[0] = idx;  dl->_IdxWritePtr[1] = (ImDrawIdx)(idx+1);   dl->_IdxWritePtr[2] = (ImDrawIdx)(idx+2);
        dl->_VtxWritePtr[0].pos = A;    dl->_VtxWritePtr[0].uv = dl->_Data->TexUvWhitePixel;      dl->_VtxWritePtr[0].col = ClWithAlpha( cl, a );      dl->_VtxWritePtr[0].texID = -1;
        dl->_VtxWritePtr[1].pos = B;    dl->_VtxWritePtr[1].uv = dl->_Data->TexUvWhitePixel;      dl->_VtxWritePtr[1].col = ClWithAlpha( cl, a );      dl->_VtxWritePtr[1].texID = -1;
        dl->_VtxWritePtr[2].pos = C;    dl->_VtxWritePtr[2].uv = dl->_Data->TexUvWhitePixel;      dl->_VtxWritePtr[2].col = ClWithAlpha( cl, a );      dl->_VtxWritePtr[2].texID = -1;
        dl->_VtxWritePtr += 3;
        dl->_VtxCurrentIdx += 3;
        dl->_IdxWritePtr += 3;
    }

    void UITriangulate( const vector<vec2> & inPnts, vector<vec2> & outPnts ){

        vector<p2t::Point*> pLine;
        for ( vec2 pnt:inPnts ) {
            pLine.push_back(new p2t::Point(pnt.x, pnt.y));
        }

        p2t::CDT cdt = p2t::CDT(pLine);        
        cdt.Triangulate();
        vector<p2t::Triangle*> currentOutput = cdt.GetTriangles();
        for ( p2t::Triangle *ct : currentOutput ) {
            outPnts.push_back( {ct->GetPoint(0)->x, ct->GetPoint(0)->y} );
            outPnts.push_back( {ct->GetPoint(2)->x, ct->GetPoint(2)->y} );
            outPnts.push_back( {ct->GetPoint(1)->x, ct->GetPoint(1)->y} );
        }
        
        for ( p2t::Point* pnt : pLine ) {
            delete pnt;
        }

    }

    void UITriangulate( const vector<highp_dvec2> & inPnts, vector<highp_dvec2> & outPnts, double inSclFactor, double inMarginError){
        // Scale factor is the amout you scale up to avoid percision & collision factors
        // the same scale factor will be removed on the way out

        if ( inPnts.size() < 3) { return; }
        if ( inSclFactor ==  0) { return; } // scales everything to nothing, avoid divide by 0

        p2t::Point startPnt( inPnts[inPnts.size()-1].x*inSclFactor, inPnts[inPnts.size()-1].y*inSclFactor );
        p2t::Point* oldPnt = &startPnt; 

        // Fill poly2tri structure, and remove duplicates
        vector<p2t::Point*> allPnts; allPnts.reserve( inPnts.size() ); // track all points so we can cleanup at the end
        vector<p2t::Point*> pLine; pLine.reserve( inPnts.size() );
        for ( const highp_dvec2 & vPnt : inPnts ) {
            p2t::Point* pnt = new p2t::Point(vPnt.x*inSclFactor, vPnt.y*inSclFactor);
            if ( abs( pnt->x-oldPnt->x ) > inMarginError && abs( pnt->y-oldPnt->y ) > inMarginError ) {
                pLine.push_back(pnt);
            }
            allPnts.push_back(pnt);
            oldPnt = pnt;
        }

        if ( pLine.size() < 3 ) { goto CLEANUP; }
        try {
            p2t::CDT cdt = p2t::CDT(pLine);
            cdt.Triangulate();
            vector<p2t::Triangle*> currentOutput = cdt.GetTriangles();
            for ( p2t::Triangle *ct : currentOutput ) {
                outPnts.push_back( {ct->GetPoint(0)->x/inSclFactor, ct->GetPoint(0)->y/inSclFactor} );
                outPnts.push_back( {ct->GetPoint(2)->x/inSclFactor, ct->GetPoint(2)->y/inSclFactor} );
                outPnts.push_back( {ct->GetPoint(1)->x/inSclFactor, ct->GetPoint(1)->y/inSclFactor} );
            }            
        }
        catch (const std::runtime_error & e){
            std::cout << "UITriangulate() Caught Runtime Error: " << e.what() << std::endl;; // information from length_error printed
            goto CLEANUP;
        }
        catch(char *e){
            printf( "UITriangulate() Caught Exception: %s\n",e );
            goto CLEANUP;
        }

        CLEANUP:
        for ( p2t::Point* pnt : allPnts ) {
            delete pnt;
        }

    }

    void UIDropShadow(ImRect inBox, float inBlurDist, float inAlpha, int inRes1, int inRes2, unsigned int inC, float inXO, float inYO){
        
        inBox.Translate({inXO,inYO});
        ImGuiContext& g = *GImGui;ImGuiWindow* window = g.CurrentWindow;ImDrawList* dl = ImGui::GetWindowDrawList();
        vector<vec2> cVec = {{1,1},{-1,1},{-1,-1},{1,-1}};
        vector<vec2> pList = {{inBox.Min.x, inBox.Min.y},{inBox.Max.x, inBox.Min.y},{inBox.Max.x, inBox.Max.y},{inBox.Min.x, inBox.Max.y}};
        double angStep = M_PI * .5;double ang1 = M_PI * -1;double ang2 = M_PI * -.5;float pxb;float pyb;
        float outDist;float outStep = inBlurDist / inRes1;
        vector<vec2> sweepS = {};vector<vec2> sweep1 = {};vector<vec2> sweep2 = {};
        double subAng = 0;
        double angleStep = (M_PI * .5) / inRes2;

        float aStep = inAlpha/((float)inRes1-1.0f);  //  alpha step
        
        vector<vec2> fillL = {};
        for(int i=0;i<pList.size();i++){
            vec2 cp = pList[i] + cVec[i] * inBlurDist;  // corner point
            subAng = ang1;
            for(int j=0;j<=inRes2;j++){
                sweep1 = {};
                outDist = inBlurDist;
                pxb =  (float)cos(subAng);pyb =  (float)sin(subAng);
                for(int k=0;k<=inRes1;k++){sweep1.push_back( {outDist * pxb + cp.x, outDist * pyb + cp.y} );outDist += outStep;}
                fillL.push_back(sweep1[0]);
                if(sweep2.size() == 0){sweepS = sweep1;}
                else{
                    float alpha = inAlpha;
                    for(int k=1;k<inRes1;k++){
                        ImU32 clr1 = ClWithAlpha(inC, alpha);
                        ImU32 clr2 = ClWithAlpha(inC, alpha-aStep);
                        const ImVec2 uv = dl->_Data->TexUvWhitePixel;
                        dl->PrimReserve(6, 4);
                        dl->PrimWriteIdx((ImDrawIdx)(dl->_VtxCurrentIdx)); dl->PrimWriteIdx((ImDrawIdx)(dl->_VtxCurrentIdx+1)); dl->PrimWriteIdx((ImDrawIdx)(dl->_VtxCurrentIdx+2));
                        dl->PrimWriteIdx((ImDrawIdx)(dl->_VtxCurrentIdx)); dl->PrimWriteIdx((ImDrawIdx)(dl->_VtxCurrentIdx+2)); dl->PrimWriteIdx((ImDrawIdx)(dl->_VtxCurrentIdx+3));
                        dl->PrimWriteVtx( {sweep1[k].x,sweep1[k].y}, uv, clr2);
                        dl->PrimWriteVtx( {sweep2[k].x,sweep2[k].y}, uv, clr2);
                        dl->PrimWriteVtx( {sweep2[k-1].x,sweep2[k-1].y}, uv, clr1);
                        dl->PrimWriteVtx( {sweep1[k-1].x,sweep1[k-1].y}, uv, clr1);
                        alpha -= aStep;
                    }
                }
                sweep2 = sweep1;
                subAng += angleStep;
            }
            ang1 += angStep;
            ang2 += angStep;
        }
        float alpha = inAlpha;
        for(int k=1;k<inRes1;k++){
            ImU32 clr1 = ClWithAlpha(inC, alpha);
            ImU32 clr2 = ClWithAlpha(inC, alpha-aStep);
            const ImVec2 uv = dl->_Data->TexUvWhitePixel;
            dl->PrimReserve(6, 4);
            dl->PrimWriteIdx((ImDrawIdx)(dl->_VtxCurrentIdx)); dl->PrimWriteIdx((ImDrawIdx)(dl->_VtxCurrentIdx+1)); dl->PrimWriteIdx((ImDrawIdx)(dl->_VtxCurrentIdx+2));
            dl->PrimWriteIdx((ImDrawIdx)(dl->_VtxCurrentIdx)); dl->PrimWriteIdx((ImDrawIdx)(dl->_VtxCurrentIdx+2)); dl->PrimWriteIdx((ImDrawIdx)(dl->_VtxCurrentIdx+3));
            dl->PrimWriteVtx( {sweepS[k].x,sweepS[k].y}, uv, clr2);
            dl->PrimWriteVtx( {sweep1[k].x,sweep1[k].y}, uv, clr2);
            dl->PrimWriteVtx( {sweep1[k-1].x,sweep1[k-1].y}, uv, clr1);
            dl->PrimWriteVtx( {sweepS[k-1].x,sweepS[k-1].y}, uv, clr1);
            alpha -= aStep;
        }
        fillL.push_back(sweepS[0]);
        UIGlobalFilledPLineA(fillL, inC , inAlpha);

    }

    void UIDropShadow(ImRect inBox, float inBlurDist, float inAlpha, int inRes, unsigned int inC, float inXO, float inYO){

        // inBlurDist*=2; //Size up to match Adobe
        ImRect InnerBox = ImRect(inBox.Min,{inBox.Max.x-(inXO*2),inBox.Max.y-(inYO*2)});
        InnerBox.Translate({inXO*2,inYO*2});
        ImRect shadowBox = ImRect({InnerBox.Min.x-inBlurDist,InnerBox.Min.y-inBlurDist},{InnerBox.Max.x+inBlurDist,InnerBox.Max.y+inBlurDist});
        // ImRect shadowBox2 = ImRect({shadowBox.Min.x-inX1,shadowBox.Min.y-inY1},{shadowBox.Max.x+inXO,shadowBox.Max.y+inYO});
        // UIDrawAlphaRoundBB(shadowBox,inC,inAlpha,inBlurDist);
        // UIDrawAlphaRoundBB(shadowBox2,inC,inAlpha/2,inBlurDist);

        UIDrawAlphaBB(InnerBox,inC,inAlpha);
        //Top
        UIAddQuadPrim(InnerBox.Min,InnerBox.GetTR(),{InnerBox.GetTR().x, shadowBox.GetTR().y},{InnerBox.Min.x, shadowBox.Min.y},inC,inC,inC,inC,inAlpha,inAlpha,0,0);
        // UIAddQuadPrim({InnerBox.Min.x, shadowBox.Min.y},{InnerBox.GetTR().x, shadowBox.GetTR().y},{InnerBox.GetTR().x, shadowBox2.GetTR().y},{InnerBox.Min.x, shadowBox2.Min.y},inC,inC,inC,inC,inAlpha/2,inAlpha/2,0,0);

        //Bottom
        UIAddQuadPrim(InnerBox.Max,InnerBox.GetBL(),{InnerBox.GetBL().x, shadowBox.GetBL().y},{InnerBox.Max.x,shadowBox.Max.y},inC,inC,inC,inC,inAlpha,inAlpha,0,0);
        // UIAddQuadPrim({InnerBox.Max.x,shadowBox.Max.y},{InnerBox.GetBL().x, shadowBox.GetBL().y},{InnerBox.GetBL().x, shadowBox2.GetBL().y},{InnerBox.Max.x,shadowBox2.Max.y},inC,inC,inC,inC,inAlpha/2,inAlpha/2,0,0);

        //Left
        UIAddQuadPrim(InnerBox.Min,InnerBox.GetBL(),{shadowBox.GetBL().x, InnerBox.GetBL().y},{shadowBox.Min.x, InnerBox.Min.y},inC,inC,inC,inC,inAlpha,inAlpha,0,0);
        // UIAddQuadPrim({shadowBox.Min.x, InnerBox.Min.y},{shadowBox.GetBL().x, InnerBox.GetBL().y},{shadowBox2.GetBL().x, InnerBox.GetBL().y},{shadowBox2.Min.x, InnerBox.Min.y},inC,inC,inC,inC,inAlpha/2,inAlpha/2,0,0);

        //Right
        UIAddQuadPrim(InnerBox.Max,InnerBox.GetTR(),{shadowBox.GetTR().x,InnerBox.GetTR().y},{shadowBox.Max.x,InnerBox.Max.y},inC,inC,inC,inC,inAlpha,inAlpha,0,0);
        // UIAddQuadPrim({shadowBox.Max.x,InnerBox.Max.y},{shadowBox.GetTR().x,InnerBox.GetTR().y},{shadowBox2.GetTR().x,InnerBox.GetTR().y},{shadowBox2.Max.x,InnerBox.Max.y},inC,inC,inC,inC,inAlpha/2,inAlpha/2,0,0);
        
        //TL
        int NbSegs = inRes;
        vec2 OldTLP = {shadowBox.Min.x,InnerBox.Min.y};
        vec2 NewTLP = {};
        float rad = inBlurDist;
        double sliceAngle = M_PI/2 - 0;
        double angleStep = sliceAngle / NbSegs;
        double TLang = M_PI;
        for (int i = 0; i <= NbSegs; i++)
        {
            float px = (float)(InnerBox.Min.x + rad * cos(TLang));
            float py = (float)(InnerBox.Min.y + rad * sin(TLang));
            NewTLP = {px,py}; 
            UIAddTriPrim(InnerBox.Min,NewTLP,OldTLP,inC,inC,inC,inAlpha,0,0);
            OldTLP = NewTLP;
            TLang += angleStep;
        }

        //BR
        vec2 OldBRP = {InnerBox.Max.x,shadowBox.Max.y};
        vec2 NewBRP = {};
        double BRang = 0;
        for (int i = 0; i <= NbSegs; i++)
        {
            float px = (float)(InnerBox.Max.x + rad * cos(BRang));
            float py = (float)(InnerBox.Max.y + rad * sin(BRang));
            NewBRP = {px,py}; 
            UIAddTriPrim(InnerBox.Max,NewBRP,OldBRP,inC,inC,inC,inAlpha/2,0,0);
            OldBRP = NewBRP;
            BRang += angleStep;
        }

        //BL
        vec2 OldBLP = {InnerBox.GetBL().x,shadowBox.GetBL().y};
        vec2 NewBLP = {};
        double BLang = M_PI/2;
        for (int i = 0; i <= NbSegs; i++)
        {
            float px = (float)(InnerBox.GetBL().x + rad * cos(BLang));
            float py = (float)(InnerBox.GetBL().y + rad * sin(BLang));
            NewBLP = {px,py}; 
            UIAddTriPrim(InnerBox.GetBL(),NewBLP,OldBLP,inC,inC,inC,inAlpha,0,0);
            OldBLP = NewBLP;
            BLang += angleStep;
        }

        //TR
        vec2 OldTRP = {InnerBox.GetTR().x,shadowBox.GetTR().y};
        vec2 NewTRP = {};
        double TRang = 3*M_PI/2;
        for (int i = 0; i <= NbSegs; i++)
        {
            float px = (float)(InnerBox.GetTR().x + rad * cos(TRang));
            float py = (float)(InnerBox.GetTR().y + rad * sin(TRang));
            NewTRP = {px,py}; 
            UIAddTriPrim(InnerBox.GetTR(),NewTRP,OldTRP,inC,inC,inC,inAlpha,0,0);
            OldTRP = NewTRP;
            TRang += angleStep;
        }
        
    }

    void UIGlobalFilledPLineA(vector<vec2> inPnts, unsigned int inC, float inA){

        ImGuiContext& g = *GImGui;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        size_t n = inPnts.size();
        vector<ImVec2> pList;
        for(int i=0;i<n;i++){pList.push_back({inPnts[i].x, inPnts[i].y});} 
        ImVec2* pListArr = &pList[0];
        ImU32 clr = ClWithAlpha(inC, inA);
        drawList->AddConvexPolyFilled(pListArr, (int) n, clr);

    }
    void UIGlobalFilledPLine(vector<vec2> inPnts, unsigned int inC ){

        ImGuiContext& g = *GImGui;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        size_t n = inPnts.size();
        vector<ImVec2> pList;
        for(int i=0;i<n;i++){pList.push_back({inPnts[i].x, inPnts[i].y});} 
        ImVec2* pListArr = &pList[0];
        ImU32 clr = UILib::Cl( inC );
        drawList->AddConvexPolyFilled(pListArr, (int) n, clr);

    }
    void UIAddGlobalPolyline(const vector<vec2> & inPnts, unsigned int inC, float inThickness, bool inClosed ){
        // TO DO: This function is inefficient because it makes a copy of inPnts so it can convert from glm::vec2
        // to ImVec2. We should eventaully write our own version of AddPolyLine to handle glm::vec2.
        ImGuiContext& g = *GImGui;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
       
        size_t n = inPnts.size();
        vector<ImVec2> pList;
        for(int i=0;i<n;i++){
            pList.push_back({inPnts[i].x, inPnts[i].y});
        }    

        ImVec2* pListArr = &pList[0];
        ImU32 clr = UILib::Cl( inC );

        drawList->AddPolyline(pListArr, (int) n, clr, inClosed, inThickness);

    }

    void UIAddGlobalPolyline(const vector<highp_dvec2> & inPnts, unsigned int inC, float inThickness, bool inClosed ){
        // TO DO: This function is inefficient because it makes a copy of inPnts so it can convert from glm::highp_dvec2
        // to ImVec2. We should eventaully write our own version of AddPolyLine to handle glm::highp_dvec2.

        ImGuiContext& g = *GImGui;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
       
        size_t n = inPnts.size();
        vector<ImVec2> pList;
        for(int i=0;i<n;i++){
            pList.push_back( { (float) inPnts[i].x, (float) inPnts[i].y } );
        }    

        ImVec2* pListArr = &pList[0];
        ImU32 clr = UILib::Cl( inC );

        drawList->AddPolyline(pListArr, (int) n, clr, inClosed, inThickness);

    }

    void UIAddGlobalPolylineAlpha(const vector<vec2> & inPnts, unsigned int inC, float inThickness, float inAlpha, bool inClosed ) {

        ImGuiContext& g = *GImGui;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
       
        size_t n = inPnts.size();
        vector<ImVec2> pList;
        for(int i=0;i<n;i++){
            pList.push_back({inPnts[i].x, inPnts[i].y});
        }    

        ImVec2* pListArr = &pList[0];
        ImU32 clr = UILib::ClWithAlpha( inC, inAlpha );  

        drawList->AddPolyline(pListArr, (int) n, clr, inClosed, inThickness);

    }
    void UICircleGlobal(ImVec2 inP, float inRadius, unsigned int inC, bool inFill, float inThickness, int nbSegments){
        
        ImGuiContext& g = *GImGui;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImU32 clr = UILib::Cl( inC );
        if(inFill){
           drawList-> AddCircleFilled(inP, inRadius, clr, nbSegments);
        }
        else{
            drawList->AddCircle(inP, inRadius, clr, nbSegments, inThickness);
        }
    }
    void UIAddGlobalLine(float inX1, float inY1, float inX2, float inY2, unsigned int inC1, float inThickness){
        ImGuiContext& g = *GImGui;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 p1 = {inX1,inY1};
        ImVec2 p2 = {inX2,inY2};
        ImU32 c1 = UILib::Cl( inC1 );  
        drawList->AddLine( p1, p2, c1, inThickness);
    }

    void UIAddGlobalLineAngled(float inX1, float inY1, float inX2, float inY2, unsigned int inC1, float inThickness, vec2 inRPoint, float rad){
        ImGuiContext& g = *GImGui;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        int startI = ImGui::GetWindowDrawList()->VtxBuffer.Size;
        ImVec2 p1 = {inX1,inY1};
        ImVec2 p2 = {inX2,inY2};
        ImU32 c1 = UILib::Cl( inC1 );  
        drawList->AddLine( p1, p2, c1, inThickness);
        auto& buf = ImGui::GetWindowDrawList()->VtxBuffer;
        for (int i = startI; i < buf.Size; i++) {
		    buf[i].pos = UIRotate( buf[i].pos - inRPoint, cos(rad), sin(rad)) + inRPoint;
        }
    }

    void UIAddGlobalLineAlpha(float inX1, float inY1, float inX2, float inY2, unsigned int inC1, float inThickness, float inA){
        ImGuiContext& g = *GImGui;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 p1 = {inX1,inY1};
        ImVec2 p2 = {inX2,inY2};
        ImU32 c1 = UILib::ClWithAlpha( inC1, inA );  
        drawList->AddLine( p1, p2, c1, inThickness);
    }

    void UIAddGlobalLineAngledAlpha(float inX1, float inY1, float inX2, float inY2, unsigned int inC1, float inThickness, vec2 inRPoint, float rad, float inA) {
        ImGuiContext& g = *GImGui;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        int startI = ImGui::GetWindowDrawList()->VtxBuffer.Size;
        ImVec2 p1 = {inX1,inY1};
        ImVec2 p2 = {inX2,inY2};
        ImU32 c1 = UILib::ClWithAlpha( inC1, inA ); 
        drawList->AddLine( p1, p2, c1, inThickness);
        auto& buf = ImGui::GetWindowDrawList()->VtxBuffer;
        for (int i = startI; i < buf.Size; i++) {
		    buf[i].pos = UIRotate( buf[i].pos - inRPoint, cos(rad), sin(rad)) + inRPoint;
        }
    }

    void UIAddLine(float inX1, float inY1, float inX2, float inY2, unsigned int inC1, float inThickness){
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 wPos = window->Pos;
        ImVec2 p1 = UILib::Pos(inX1,inY1, wPos);
        ImVec2 p2 = UILib::Pos(inX2,inY2, wPos);
        ImU32 c1 = UILib::Cl( inC1 ); 
        drawList->AddLine( p1, p2, c1, inThickness);
    }
    void UIAddLineAlpha(float inX1, float inY1, float inX2, float inY2, unsigned int inC1, float inThickness, float inA){
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 wPos = window->Pos;
        ImVec2 p1 = UILib::Pos(inX1,inY1, wPos);
        ImVec2 p2 = UILib::Pos(inX2,inY2, wPos);
        ImU32 c1 = UILib::ClWithAlpha( inC1, inA );  
        drawList->AddLine( p1, p2, c1, inThickness);
    }
    void UIAddGradientFade(float inX, float inY, float inW, float inH, unsigned int inC, bool inVertical){
        
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 wPos = window->Pos;
        ImU32 c1 = UILib::Cl( inC );
        ImU32 c2 = UILib::ClearCl( inC );
        ImVec2 p1 = ImVec2(inX + wPos.x, inY + wPos.y);
        ImVec2 p2 = ImVec2(p1.x + inW, p1.y + inH);
        if(inVertical){
            drawList->AddRectFilledMultiColor(p1, p2, c1, c1, c2, c2);
        }
        else{
            drawList->AddRectFilledMultiColor(p1, p2, c1, c2, c2, c1);
        }
    }
    void UIAddGradientRect(float inX, float inY, float inW, float inH, unsigned int inC1, unsigned int inC2, bool inVertical){
        
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 wPos = window->Pos;
        ImU32 c1 = UILib::Cl( inC1 );  
        ImU32 c2 = UILib::Cl( inC2 );
        ImVec2 p1 = ImVec2(inX + wPos.x, inY + wPos.y);
        ImVec2 p2 = ImVec2(p1.x + inW, p1.y + inH);
        if(inVertical){
            drawList->AddRectFilledMultiColor(p1, p2, c1, c1, c2, c2);
        }
        else{
            drawList->AddRectFilledMultiColor(p1, p2, c1, c2, c2, c1);
        }
    }

    // ============================================================================================ //
    //                                        POLYLINES                                             //
    // ============================================================================================ //

#if 0

    void UIDrawPolyline( const vector<vec2>& inPnts, int inC, float inA, float inThickness ) {
        // inCapType: 0 = none, 1 = round
        if ( inPnts.size() < 2 ) { return; }
        
        const float aaSize = 1.2f;
        const float T = ( inThickness - aaSize ) * 0.5f;
            
        const vector<int>   aaClrs   = { inC, inC };
        const vector<float> aaAlphas = { inA, 0   };
        
        // Only 2 Points
        if ( inPnts.size() == 2 ) {

            vec2 a, b, c, d;    // quad points
            vec2 D, N, pN;      // vector direction & normal
            double angD;        // radians in plane space

            // Get the parallel and normal vectors and angles
            D = glm::normalize(inPnts[1] - inPnts[0]);
            N = UIGetNormal(D);
            angD = UIGetAngle( D );

            // Get the Quad points
            pN = N*T;
            a = inPnts[0]+pN;
            b = inPnts[0]-pN;
            c = inPnts[1]-pN;
            d = inPnts[1]+pN;
 
            // Base Quad
            UIAddQuadPrim(  a, b, c, d, inC, inC, inC, inC, inA, inA, inA, inA );

            // AA Quads 
            UIAddQuadPrim( a, d, d+N*aaSize, a+N*aaSize, inC, inC, inC, inC, inA, inA, 0, 0 );
            UIAddQuadPrim( d, c, c+D*aaSize, d+D*aaSize, inC, inC, inC, inC, inA, inA, 0, 0 );
            UIAddQuadPrim( c, b, b-N*aaSize, c-N*aaSize, inC, inC, inC, inC, inA, inA, 0, 0 );
            UIAddQuadPrim( b, a, a-D*aaSize, b-D*aaSize, inC, inC, inC, inC, inA, inA, 0, 0 );

            // AA Corners
            UIDrawRadialGradientArcAlpha( a, aaSize, angD-3*M_PI_2, angD-M_PI  , aaClrs, aaAlphas );
            UIDrawRadialGradientArcAlpha( d, aaSize, angD         , angD+M_PI_2, aaClrs, aaAlphas );
            UIDrawRadialGradientArcAlpha( c, aaSize, angD-M_PI_2  , angD       , aaClrs, aaAlphas );
            UIDrawRadialGradientArcAlpha( b, aaSize, angD-M_PI    , angD-M_PI_2, aaClrs, aaAlphas );

            // Debug
            // UIAddGlobalLine( inPnts[1].x, inPnts[1].y, inPnts[0].x, inPnts[0].y, 0x000000, 3 );
            // UIAddGlobalLine( a.x, a.y, b.x, b.y, 0xFF0000, 2 );
            // UIAddGlobalLine( b.x, b.y, c.x, c.y, 0x00FF00, 2 );
            // UIAddGlobalLine( c.x, c.y, d.x, d.y, 0x0000FF, 2 );
            // UIAddGlobalLine( d.x, d.y, a.x, a.y, 0xFF00FF, 2 );
            // UIAddGlobalCircle( inPnts[0].x, inPnts[0].y, 10, 0xFFFF00, true, 1, 16 );
            // UIAddGlobalCircle( inPnts[1].x, inPnts[1].y, 10, 0xFFFF00, true, 1, 16 );
            // UIAddGlobalCircle( a.x, a.y, 10, 0xFF0000, true, 1, 16 );
            // UIAddGlobalCircle( b.x, b.y, 10, 0x00FF00, true, 1, 16 );
            // UIAddGlobalCircle( c.x, c.y, 10, 0x0000FF, true, 1, 16 );
            // UIAddGlobalCircle( d.x, d.y, 10, 0xFF00FF, true, 1, 16 );           

            return;

        }

        // For each line segment, these are the letters

        //  b ------------------------ c
        //  |                          |
        //  p0 ----------------------- p1
        //  |                          |
        //  a ------------------------ d

        // More than 2 Points
        assert( inPnts.size() >= 3 );

        vec2 a, b, c, d; // quad points
        vec2 aO, bO, cO, dO; // outer quad points for AA
        vec2 D0, D1;     // direction vectors
        vec2 N0, N1;     // normal vectors
        vec2 pN, H, P;   // scaled normal,  half vector, and and perpendicular to half vector
        double angH;     // half angle radians
        double angD;     // angle of the vector
        double L, L2;   // project out form the angled point
        int C0 = -1, C1 = -1; // -1 or 1 to store cross product direction
        bool CFlip = false, lastFlip = false;; // indicates whether cross product has flipped

        constexpr double angThresh = 0.261799; // 15 degrees        

        // First line segement
        D0 = glm::normalize(inPnts[0] - inPnts[1]);
        N0 = UIGetNormal(D0);
        angD = UIGetAngle(D0);

        pN = N0*T;
        a = inPnts[0]-pN;
        b = inPnts[0]+pN;
        
        // Draw start cap AA quads
        pN = D0*aaSize;
        aO = a+pN;
        bO = b+pN;
        UIAddQuadPrim( b, a, aO, bO, inC, inC, inC, inC, inA, inA, 0, 0 );
        UIDrawRadialGradientArcAlpha( a, aaSize, angD-M_PI_2, angD, aaClrs, aaAlphas );
        UIDrawRadialGradientArcAlpha( b, aaSize, angD, angD+M_PI_2, aaClrs, aaAlphas );

        // Get outer points for side AA quad draw
        pN = N0*aaSize;
        aO = a-pN;
        bO = b+pN;        

        // Debug points
        // UIAddGlobalCircle( inPnts[0].x, inPnts[0].y, 10, 0xFFFF00, true, 1, 16 );
        // UIAddGlobalCircle( a.x, a.y, 10, 0xFF0000, true, 1, 16 );
        // UIAddGlobalCircle( b.x, b.y, 10, 0x00FF00, true, 1, 16 );
        // UIAddTextString( inPnts[0].x + 10, inPnts[0].y + 10, 100, 35, to_string(C0), 0x000000, vec2(0) );
        
        // Middle line segments
        size_t lastI = inPnts.size()-1;
        for ( size_t i = 1; i < lastI; i++ ) {
            
            // Get the current point vector
            D1 = glm::normalize(inPnts[i+1] - inPnts[i]);
            N1 = UIGetNormal(D1);

            H = glm::normalize( (D0+D1)*0.5f );
            angH = UIGetVectorAngle(D0, D1)*0.5f;
            C1 = UICrossProduct(D0,D1) >= 0 ? 1 : -1;
            CFlip = ( (C1 > 0 && C0 < 0) || (C1 < 0 && C0 > 0) );

            // Debug
            // vec2 hP = inPnts[i] + H * T;
            // UIAddGlobalLine( inPnts[i].x, inPnts[i].y, hP.x, hP.y, 0xFF0000, 5 );
            // vec2 nP = inPnts[i] + N1 * T;
            // UIAddGlobalLine( inPnts[i].x, inPnts[i].y, nP.x, nP.y, 0x00FF00, 5 );
    
            // Get quad points
            if ( angH < angThresh ) { 
                
                P = UIGetNormal(H);
                L = T / glm::cos(angH);
                L2 = L / glm::tan(angH);
        
                pN = P * (float ) L;                
                if ( CFlip ) {
                    c = inPnts[i] + H * (float) L2;                   // Point on inner side of V
                    cO = c + H * std::min( (float) (2 * aaSize), (float) ( aaSize / glm::sin(angH) ) ); // AA outer point on inner side of V
                    d = inPnts[i] - pN;
                    dO = d - N0 * aaSize;
                }
                else {
                    c = inPnts[i] + pN;
                    cO = c + N0 * aaSize;
                    d = inPnts[i] + H * (float) L2;                   // Point on inner side of V
                    dO = d + H * std::min( (float) (2 * aaSize), (float) ( aaSize / glm::sin(angH) ) ); // AA outer point on inner side of V
                }

                // Base Quad
                UIAddQuadPrim( a, b, c, d, inC, inC, inC, inC, inA, inA, inA, inA );

                // Draw the side AA quads
                UIAddQuadPrim( a, d, dO, aO, inC, inC, inC, inC, inA, inA, 0, 0 );
                UIAddQuadPrim( c, b, bO, cO, inC, inC, inC, inC, inA, inA, 0, 0 );

                // Debug
                // UIAddGlobalLine( a.x, a.y, b.x, b.y, 0xFF0000, 2 );
                // UIAddGlobalLine( b.x, b.y, c.x, c.y, 0x00FF00, 2 );
                // UIAddGlobalLine( c.x, c.y, d.x, d.y, 0x0000FF, 2 );
                // UIAddGlobalLine( d.x, d.y, a.x, a.y, 0xFF00FF, 2 );
                // UIAddGlobalCircle( a.x, a.y, 5, 0xFF0000, true, 1, 16 );
                // UIAddGlobalCircle( b.x, b.y, 5, 0x00FF00, true, 1, 16 );
                // UIAddGlobalCircle( c.x, c.y, 10, 0x0000FF, true, 1, 16 );
                // UIAddGlobalCircle( d.x, d.y, 10, 0xFF00FF, true, 1, 16 );
                // UIAddGlobalCircle( cO.x, cO.y, 5, 0xABABAB, true, 1, 16 );
                // UIAddGlobalCircle( dO.x, dO.y, 5, 0xABABAB, true, 1, 16 );

                // Base Triangle
                if ( CFlip ) {
                    c = inPnts[i] + pN;
                    cO = c + N0 * aaSize;
                    d = inPnts[i] + H * (float) L2; 
                    dO = d + H * (float) ( aaSize / glm::sin(angH) ); 
                }
                a = d;
                b = c;
                c = inPnts[i] - pN;
                bO = b - H * aaSize;
                cO = c - H * aaSize;
                UIAddTriPrim( a, b, c, inC, inA );

                // Draw the flat cap AA
                angD = UIGetAngle(H*-1);
                UIAddQuadPrim(c, b, bO, cO, inC, inC, inC, inC, inA, inA, 0, 0 );
                if ( CFlip ) {
                    UIDrawRadialGradientArcAlpha( b, aaSize, angD-UIGetVectorAngle( N1, H*-1), angD, aaClrs, aaAlphas );
                    UIDrawRadialGradientArcAlpha( c, aaSize, angD, angD+UIGetVectorAngle(N0*-1,H*-1), aaClrs, aaAlphas );
                }
                else {
                    UIDrawRadialGradientArcAlpha( b, aaSize, angD-UIGetVectorAngle( N0, H*-1), angD, aaClrs, aaAlphas );
                    UIDrawRadialGradientArcAlpha( c, aaSize, angD, angD+UIGetVectorAngle(N1*-1,H*-1), aaClrs, aaAlphas );
                }                
                
                // Debug
                // UIAddGlobalLine( a.x, a.y, b.x, b.y,0x9E005D, 2 );
                // UIAddGlobalLine( b.x, b.y, c.x, c.y,0x9E005D, 2 );
                // UIAddGlobalLine( c.x, c.y, d.x, d.y,0x9E005D, 2 );
                // UIAddGlobalCircle( c.x, c.y, 10, 0x0000FF, true, 1, 16 );
                // UIAddGlobalCircle( bO.x, bO.y, 5, 0x9E005D, true, 1, 16 );  
                // UIAddGlobalCircle( cO.x, cO.y, 5, 0x9E005D, true, 1, 16 );  
                // vec2 hP2 = hP + P * T;
                // UIAddGlobalLine( hP.x, hP.y, hP2.x, hP2.y, 0x0000FF, 5 );
                // UIAddGlobalLine( inPnts[i-1].x, inPnts[i-1].y, inPnts[i].x, inPnts[i].y, 0x000000, 3 );
                // UIAddGlobalCircle( inPnts[i].x, inPnts[i].y, 10, 0xFFFF00, true, 1, 16 );
                // UIAddTextString( inPnts[i].x + 10, inPnts[i].y + 10, 100, 35, to_string(C1), 0x000000, vec2(0) );

                // Update variables
                if ( CFlip ) { 
                    std::swap(b,c); C1 *= -1; 
                    cO = c + N1 * aaSize; // Update the cO for side AA
                    std::swap(c,d); std::swap(cO,dO);
                } 
                else {
                    cO = c - N1 * aaSize; // Update the cO for side AA
                }

            }
            else { 

                L = T / glm::sin(angH);
                pN = H * (float) L; 
                c = inPnts[i]-pN;
                d = inPnts[i]+pN;
                pN += H*aaSize;
                cO = inPnts[i]-pN;
                dO = inPnts[i]+pN;
                if ( CFlip ) { std::swap(c,d); std::swap(cO,dO); C1 *= -1; }

                // Base Quad
                UIAddQuadPrim( a, b, c, d, inC, inC, inC, inC, inA, inA, inA, inA );

                // Draw the side AA quads
                UIAddQuadPrim( a, d, dO, aO, inC, inC, inC, inC, inA, inA, 0, 0 );
                UIAddQuadPrim( c, b, bO, cO, inC, inC, inC, inC, inA, inA, 0, 0 );

                // Debug
                // UIAddGlobalLine( inPnts[i-1].x, inPnts[i-1].y, inPnts[i].x, inPnts[i].y, 0x000000, 3 );
                // UIAddGlobalLine( a.x, a.y, b.x, b.y, 0xFF0000, 2 );
                // UIAddGlobalLine( b.x, b.y, c.x, c.y, 0x00FF00, 2 );
                // UIAddGlobalLine( c.x, c.y, d.x, d.y, 0x0000FF, 2 );
                // UIAddGlobalLine( d.x, d.y, a.x, a.y, 0xFF00FF, 2 );
                // UIAddGlobalCircle( inPnts[i].x, inPnts[i].y, 10, 0xFFFF00, true, 1, 16 );
                // UIAddGlobalCircle( a.x, a.y, 5, 0xFF0000, true, 1, 16 );
                // UIAddGlobalCircle( b.x, b.y, 5, 0x00FF00, true, 1, 16 );
                // UIAddGlobalCircle( c.x, c.y, 10, 0x0000FF, true, 1, 16 );
                // UIAddGlobalCircle( d.x, d.y, 10, 0xFF00FF, true, 1, 16 );
                // UIAddTextString( inPnts[i].x + 10, inPnts[i].y + 10, 100, 35, to_string(C1), 0x000000, vec2(0) );
                // UIAddGlobalCircle( cO.x, cO.y, 5, 0xABABAB, true, 1, 16 );
                // UIAddGlobalCircle( dO.x, dO.y, 5, 0xABABAB, true, 1, 16 );
            }

            a = d; aO = dO;
            b = c; bO = cO;
            D0 = D1 * -1.0f;
            N0 = N1 * -1.0f;
            C0 = C1;
            lastFlip = CFlip;

        }

        // Last segment

        D1 = glm::normalize(inPnts[lastI] - inPnts[lastI-1]);
        N1 = UIGetNormal(D1);
        angD = UIGetAngle(D1);

        // Get quad points
        pN = N1*T;
        c = inPnts[lastI]-pN;
        d = inPnts[lastI]+pN;
        
        // Base Quad
        UIAddQuadPrim( a, b, c, d, inC, inC, inC, inC, inA, inA, inA, inA );

        // Draw the last side AA quads
        pN = N1*aaSize;
        cO = c-pN;
        dO = d+pN;
        UIAddQuadPrim( a, d, dO, aO, inC, inC, inC, inC, inA, inA, 0, 0 );
        UIAddQuadPrim( c, b, bO, cO, inC, inC, inC, inC, inA, inA, 0, 0 );

        // Draw end cap AA quad
        pN = D1*aaSize;
        cO = c+pN;
        dO = d+pN;
        UIAddQuadPrim( d, c, cO, dO, inC, inC, inC, inC, inA, inA, 0, 0 );
        UIDrawRadialGradientArcAlpha( c, aaSize, angD-M_PI_2, angD, aaClrs, aaAlphas );
        UIDrawRadialGradientArcAlpha( d, aaSize, angD, angD+M_PI_2, aaClrs, aaAlphas );

        // Debug
        // UIAddGlobalLine( inPnts[lastI-1].x, inPnts[lastI-1].y, inPnts[lastI].x, inPnts[lastI].y, 0x000000, 3 );
        // UIAddGlobalLine( a.x, a.y, b.x, b.y, 0xFF0000, 2 );
        // UIAddGlobalLine( b.x, b.y, c.x, c.y, 0x00FF00, 2 );
        // UIAddGlobalLine( c.x, c.y, d.x, d.y, 0x0000FF, 2 );
        // UIAddGlobalLine( d.x, d.y, a.x, a.y, 0xFF00FF, 2 );
        // UIAddGlobalCircle( inPnts[lastI].x, inPnts[lastI].y, 10, 0xFFFF00, true, 1, 16 );
        // UIAddGlobalCircle( a.x, a.y, 5, 0xFF0000, true, 1, 16 );
        // UIAddGlobalCircle( b.x, b.y, 5, 0x00FF00, true, 1, 16 );
        // UIAddGlobalCircle( c.x, c.y, 10, 0x0000FF, true, 1, 16 );
        // UIAddGlobalCircle( d.x, d.y, 10, 0xFF00FF, true, 1, 16 );
        // UIAddGlobalCircle( cO.x, cO.y, 5, 0xABABAB, true, 1, 16 );
        // UIAddGlobalCircle( dO.x, dO.y, 5, 0xABABAB, true, 1, 16 );

    }

#endif

    void UIDrawMiterPolyline( const vector<vec2>& inPnts, int inC, float inA, float inThickness, double angThresh ) {
        // Draws a polyline where point connections use a miter join (pointed cap). If the angle between line segments
        // is under the angle threshold (angThresh),the the point connection uses a flat cap (or bevel cap).
        // The default angle threshold is 15 degrees.
        
        if ( inPnts.size() < 2 ) { return; }
        
        const float aaSize = 1.2f;
        const float aaCornerSize = aaSize * 1.25f;
        const float T = ( inThickness - aaSize ) * 0.5f;
            
        const vector<int>   aaClrs   = { inC, inC };
        const vector<float> aaAlphas = { inA, 0   };
        
        // Only 2 Points
        if ( inPnts.size() == 2 ) {

            // Get the parallel and normal vectors and angles
            vec2 D = glm::normalize(inPnts[1] - inPnts[0]);  // direction vector
            vec2 N = UIGetNormal(D);                         // normal vector (always to the right of direction)

            // Get the Quad points
            vec2 pN = N*T;
            vec2 a = inPnts[0]+pN;
            vec2 b = inPnts[0]-pN;
            vec2 c = inPnts[1]-pN;
            vec2 d = inPnts[1]+pN;
 
            // Base Quad
            UIAddQuadPrim(  a, b, c, d, inC, inC, inC, inC, inA, inA, inA, inA );

            // AA Quads 
            UIAddQuadPrim( a, d, d+N*aaSize, a+N*aaSize, inC, inC, inC, inC, inA, inA, 0, 0 );
            UIAddQuadPrim( d, c, c+D*aaSize, d+D*aaSize, inC, inC, inC, inC, inA, inA, 0, 0 );
            UIAddQuadPrim( c, b, b-N*aaSize, c-N*aaSize, inC, inC, inC, inC, inA, inA, 0, 0 );
            UIAddQuadPrim( b, a, a-D*aaSize, b-D*aaSize, inC, inC, inC, inC, inA, inA, 0, 0 );

            // AA Corners
            vec2 X = (N-D)*0.5f*aaCornerSize;
            UIAddTriPrim( a, a+N*aaSize, a+X       , inC, inC, inC, inA, 0, 0 );
            UIAddTriPrim( a, a+X       , a-D*aaSize, inC, inC, inC, inA, 0, 0 );
            X = (-D-N)*0.5f*aaCornerSize;
            UIAddTriPrim( b, b-D*aaSize, b+X       , inC, inC, inC, inA, 0, 0 );
            UIAddTriPrim( b, b+X       , b-N*aaSize, inC, inC, inC, inA, 0, 0 );
            X = (D-N)*0.5f*aaCornerSize;
            UIAddTriPrim( c, c-N*aaSize, c+X       , inC, inC, inC, inA, 0, 0 );
            UIAddTriPrim( c, c+X       , c+D*aaSize, inC, inC, inC, inA, 0, 0 );
            X = (D+N)*0.5f*aaCornerSize;
            UIAddTriPrim( d, d+D*aaSize, d+X       , inC, inC, inC, inA, 0, 0 );
            UIAddTriPrim( d, d+X       , d+N*aaSize, inC, inC, inC, inA, 0, 0 );

            // Debug
            // UIAddGlobalLine( inPnts[1].x, inPnts[1].y, inPnts[0].x, inPnts[0].y, 0x000000, 3 );
            // UIAddGlobalLine( a.x, a.y, b.x, b.y, 0xFF0000, 2 );
            // UIAddGlobalLine( b.x, b.y, c.x, c.y, 0x00FF00, 2 );
            // UIAddGlobalLine( c.x, c.y, d.x, d.y, 0x0000FF, 2 );
            // UIAddGlobalLine( d.x, d.y, a.x, a.y, 0xFF00FF, 2 );
            // UIAddGlobalCircle( inPnts[0].x, inPnts[0].y, 10, 0xFFFF00, true, 1, 16 );
            // UIAddGlobalCircle( inPnts[1].x, inPnts[1].y, 10, 0xFFFF00, true, 1, 16 );
            // UIAddGlobalCircle( a.x, a.y, 10, 0xFF0000, true, 1, 16 );
            // UIAddGlobalCircle( b.x, b.y, 10, 0x00FF00, true, 1, 16 );
            // UIAddGlobalCircle( c.x, c.y, 10, 0x0000FF, true, 1, 16 );
            // UIAddGlobalCircle( d.x, d.y, 10, 0xFF00FF, true, 1, 16 );

            return;

        }

        // For each line segment, these are the letters

        //  b ------------------------ c
        //  |                          |
        //  p0 ----------------------- p1
        //  |                          |
        //  a ------------------------ d

        // More than 2 Points
        assert( inPnts.size() >= 3 );

        vec2 a, b, c, d;     // quad points
        vec2 aD, bD, cD, dD; // outer AA points off the end (extending out the direction)
        vec2 aN, bN, cN, dN; // outer AA points off the side (extending out the normal)
        vec2 D0, D1;     // direction vectors (always pointing away from the joint point)
        vec2 N0, N1;     // normal vectors (always to the right of the direction vector)
        vec2 pD, pN;     // scaled direction and normal vectors
        vec2 H, P;       // half vector, and and perpendicular to half vector
        vec2 X;          // corner vector
        double angH;     // half angle radians between D0 and D1
        double L;        // length to project out to the inner side of the V intersection point
        double F;        // half of the length of the flat capped joints
        double l0, l1;    // length of incoming and outgoing lines
        double CP;        // variable to store the current cross product
        int C0 = -1, C1 = -1; // -1 or 1 to store cross product orientation
        bool CFlip = false; // indicates whether cross product has flipped
        double angClip;     // angle threshold for detecting clipping for very small angles

        // ----- First Segment -----
        D0 = inPnts[0] - inPnts[1];
        l0 = glm::length(D0);
        D0 = glm::normalize(D0);
        N0 = UIGetNormal(D0);

        pN = N0*T;
        a = inPnts[0]-pN;
        b = inPnts[0]+pN;
        
        // Start Cap AA
        pN = N0*aaSize;
        pD = D0*aaSize;
        aN = a-pN; aD = a+pD;
        bN = b+pN; bD = b+pD;
        UIAddQuadPrim( b, a, aD, bD, inC, inC, inC, inC, inA, inA, 0, 0 );
        X = (D0-N0)*0.5f*aaCornerSize;
        UIAddTriPrim( a, aN, a+X, inC, inC, inC, inA, 0, 0 ); 
        UIAddTriPrim( a, a+X, aD, inC, inC, inC, inA, 0, 0 ); 
        X = (D0+N0)*0.5f*aaCornerSize;
        UIAddTriPrim( b, bD, b+X, inC, inC, inC, inA, 0, 0 );
        UIAddTriPrim( b, b+X, bN, inC, inC, inC, inA, 0, 0 );

        // Debug
        // UIAddGlobalCircle( inPnts[0].x, inPnts[0].y, 10, 0xFFFF00, true, 1, 16 );
        // UIAddGlobalCircle( a.x, a.y, 10, 0xFF0000, true, 1, 16 );
        // UIAddGlobalCircle( b.x, b.y, 10, 0x00FF00, true, 1, 16 );
        // UIAddGlobalText( inPnts[0].x + 10, inPnts[0].y + 10, 100, 35, to_string(C0), 0x000000, vec2(0) );
        // UIAddGlobalCircle( aN.x, aN.y, 5, 0xABABAB, true, 1, 16 );
        // UIAddGlobalCircle( bN.x, bN.y, 5, 0xABABAB, true, 1, 16 );
        // UIAddGlobalCircle( aD.x, aD.y, 5, 0x9E005D, true, 1, 16 );
        // UIAddGlobalCircle( bD.x, bD.y, 5, 0x9E005D, true, 1, 16 );
        
        // ----- Middle Segments -----
        size_t lastI = inPnts.size()-1;
        for ( size_t i = 1; i < lastI; i++ ) {

            Start_Loop_Iteration:
            
            // Compute the current point data
            D1 = inPnts[i+1] - inPnts[i];
            l1 = glm::length(D1);                          // length from [i] to [i+1]
            D1 = glm::normalize(D1);                       // direction vector
            N1 = UIGetNormal(D1);                          // normal vector (always points to right of direction)
            H = glm::normalize( (D0+D1)*0.5f );            // half vector
            angH = UIGetVectorAngle(D0, D1)*0.5f;          // half angle size (radians)
            CP = UICrossProduct(D0,D1);                    // cross product
            C1 = CP >= 0 ? 1 : -1;                         // cross product orientation
            CFlip = ( (C1 > 0 && C0 < 0) || (C1 < 0 && C0 > 0) ); // did we flip orientation?

            angClip = glm::atan( T / std::min(l0,l1) );

            // Debug
            // vec2 hP = inPnts[i] + H * T;
            // vec2 nP = inPnts[i] + N1 * T;
            // UIAddGlobalLine( inPnts[i].x, inPnts[i].y, hP.x, hP.y, 0xFF0000, 5 ); // half vector ( red )
            // UIAddGlobalLine( inPnts[i].x, inPnts[i].y, nP.x, nP.y, 0x00FF00, 5 ); // normal vector ( green )
            // if ( i == 192 ) {
            //     float xPos = inPnts[i].x-150;
            //     float yPos = inPnts[i].y-20;
            //     UIAddGlobalText( xPos, yPos   , 100, 35, "D0: "+VecToString(D0), 0x000000, vec2(0) );
            //     UIAddGlobalText( xPos, yPos+20, 100, 35, "D1: "+VecToString(D1), 0x000000, vec2(0) );
            //     UIAddGlobalText( xPos, yPos+40, 100, 35, "CP: "+to_string(CP), 0x000000, vec2(0) );
            // }

            // ------- Parallel Join Section -------
            // If the current segment is parallel (in the same direction) to the incoming
            // segment, then carry over the a-b points and go on to the next segment
            if ( std::abs(D1.x+D0.x) < 0.001f && std::abs(D1.y+D0.y) < 0.001f ) {
                l0 += l1; i++;
                goto Start_Loop_Iteration;
            }
            // ------- Flat Join Section -------
            // If the half angle is less than the threshold angle, draw a flat join.
            else if ( angH < angThresh || angH < angClip ) { 
                
                P = UIGetNormal(H);
                F = T / glm::cos(angH);
                L = F / glm::tan(angH);

                // Debug
                // vec2 hP2 = hP + P * T;
                // UIAddGlobalLine( hP.x, hP.y, hP2.x, hP2.y, 0x0000FF, 5 ); // perpendicular vector ( blue )

                if ( angH < 0.008 ) {   // 1/2 degree threshold
                    if ( l0 <= l1 ) {   // if incoming segment is shorter, then don't draw it
                        pN = P * (float) F;
                        c = inPnts[i] - pN;
                        d = inPnts[i] + pN;
                        pN = N0 * aaSize;
                        cN = c - pN;
                        dN = d + pN;
                        goto End_Loop_Iteration;
                    }
                }

                // If the angle is small, draw the full quad.
                if ( angH < angClip ) {
                    pN = P * (float) F;
                    c = inPnts[i] + pN;
                    d = inPnts[i] - pN;
                    pN = N0 * aaSize;
                    cN = c + pN;
                    dN = d - pN;
                    pD = H * aaSize;
                    cD = c - H * aaSize;
                    dD = d - H * aaSize;

                    // Base Quad
                    UIAddQuadPrim( a, b, c, d, inC, inC, inC, inC, inA, inA, inA, inA );

                    // Side AA
                    UIAddQuadPrim( a, d, dN, aN, inC, inC, inC, inC, inA, inA, 0, 0 );
                    UIAddQuadPrim( c, b, bN, cN, inC, inC, inC, inC, inA, inA, 0, 0 );

                    // Cap AA
                    UIAddQuadPrim( d, c, cD, dD, inC, inC, inC, inC, inA, inA, 0, 0 );
                    X = (N0-H)*0.5f*aaCornerSize;
                    UIAddTriPrim( c, cN, c+X, inC, inC, inC, inA, 0, 0 );
                    UIAddTriPrim( c, c+X, cD, inC, inC, inC, inA, 0, 0 );
                    X = (-N1-H)*0.5f*aaCornerSize;
                    UIAddTriPrim( d, dD, d+X, inC, inC, inC, inA, 0, 0 );
                    UIAddTriPrim( d, d+X, dN, inC, inC, inC, inA, 0, 0 ); 

                    // Debug
                    // UIAddGlobalLine( inPnts[i-1].x, inPnts[i-1].y, inPnts[i].x, inPnts[i].y, 0x000000, 3 );
                    // UIAddGlobalLine( a.x, a.y, b.x, b.y, 0xFF0000, 2 );
                    // UIAddGlobalLine( b.x, b.y, c.x, c.y, 0x00FF00, 2 );
                    // UIAddGlobalLine( c.x, c.y, d.x, d.y, 0x0000FF, 2 );
                    // UIAddGlobalLine( d.x, d.y, a.x, a.y, 0xFF00FF, 2 );
                    // UIAddGlobalCircle( a.x, a.y, 5, 0xFF0000, true, 1, 16 );
                    // UIAddGlobalCircle( b.x, b.y, 5, 0x00FF00, true, 1, 16 );
                    // UIAddGlobalCircle( c.x, c.y, 10, 0x0000FF, true, 1, 16 );
                    // UIAddGlobalCircle( d.x, d.y, 10, 0xFF00FF, true, 1, 16 );
                    // UIAddGlobalCircle( cN.x, cN.y, 5, 0xABABAB, true, 1, 16 );
                    // UIAddGlobalCircle( dN.x, dN.y, 5, 0xABABAB, true, 1, 16 );
                    // UIAddGlobalCircle( bD.x, bD.y, 5, 0x9E005D, true, 1, 16 );
                    // UIAddGlobalCircle( cD.x, cD.y, 5, 0x9E005D, true, 1, 16 );
                    // CFlip ? UIAddGlobalCircle( inPnts[i].x, inPnts[i].y, 10, 0x00FFFF, true, 1, 16 )
                    //   : UIAddGlobalCircle( inPnts[i].x, inPnts[i].y, 10, 0xFFFF00, true, 1, 16 );
                    

                    // Swap the c & d for next loop
                    std::swap(c,d);
                    std::swap(cN,dN);
                    if ( CFlip ) { C1 *= -1; }
                        
                    goto End_Loop_Iteration;
                }

                // Incoming quad c & d points border the flat join cap triangle
                //
                // Not Flipped:                           Flipped:
                //
                //             c--[i]---undef             [i-1]    c     [i+1]
                //           /  \      / \                 \      /  \      /
                //          /    \    /   \                 \    /    \    /
                //         /      \  /     \                 \  /      \  /
                //        [i-1]    d    [i+1]                  d--[i]---undef
                //

                // Set the incoming quad c & d points based on the orientation
                if ( CFlip ) {
                    c = inPnts[i] + H * (float) L;         // Inner side of V
                    cN = c + H * std::min(2*aaSize, (float) ( aaSize / glm::sin(angH) ) );
                    d = inPnts[i] - P * (float) F;         // Flat join cap corner
                    dN = d - N0 * aaSize;
                }
                else {
                    c = inPnts[i] + P * (float) F;         // Flat join cap corner
                    cN = c + N0 * aaSize;
                    d = inPnts[i] + H * (float) L;         // Inner side of V
                    dN = d + H * std::min(2*aaSize, (float) ( aaSize / glm::sin(angH) ) );
                }

                // Base Quad
                UIAddQuadPrim( a, b, c, d, inC, inC, inC, inC, inA, inA, inA, inA );

                // Side AA
                UIAddQuadPrim( a, d, dN, aN, inC, inC, inC, inC, inA, inA, 0, 0 );
                UIAddQuadPrim( c, b, bN, cN, inC, inC, inC, inC, inA, inA, 0, 0 );

                // Debug
                // UIAddGlobalLine( a.x, a.y, b.x, b.y, 0xFF0000, 2 );
                // UIAddGlobalLine( b.x, b.y, c.x, c.y, 0x00FF00, 2 );
                // UIAddGlobalLine( c.x, c.y, d.x, d.y, 0x0000FF, 2 );
                // UIAddGlobalLine( d.x, d.y, a.x, a.y, 0xFF00FF, 2 );
                // UIAddGlobalCircle( a.x, a.y, 5, 0xFF0000, true, 1, 16 );
                // UIAddGlobalCircle( b.x, b.y, 5, 0x00FF00, true, 1, 16 );
                // UIAddGlobalCircle( c.x, c.y, 10, 0x0000FF, true, 1, 16 );
                // UIAddGlobalCircle( d.x, d.y, 10, 0xFF00FF, true, 1, 16 );
                // UIAddGlobalCircle( cN.x, cN.y, 5, 0xABABAB, true, 1, 16 );
                // UIAddGlobalCircle( dN.x, dN.y, 5, 0xABABAB, true, 1, 16 ); 

                // Flat Join Cap Triangle
                //
                // Not Flipped:                           Flipped:
                //
                //             b--[i]---c                 [i-1]    a    [i+1]
                //           /  \      / \                 \      /  \     /
                //          /    \    /   \                 \    /    \   /
                //         /      \  /     \                 \  /      \ /
                //        [i-1]    a    [i+1]                  c--[i]---b            
                //
                
                // Convert incoming quad c & d points to the cap triangle a-b-c points
                if ( CFlip ) {
                    a = c; aN = cN;
                    b = inPnts[i] + P * (float) F; 
                    bN = b + P * aaSize; 
                    bD = b - H * aaSize;
                    c = d;
                    cN = dN;
                    cD = c - H * aaSize;
                }
                else {
                    a = d;
                    b = c;
                    bN = cN;
                    bD = b - H * aaSize;
                    c = inPnts[i] - P * (float) F;
                    cN = c - P * aaSize;
                    cD = c - H * aaSize;                    
                }

                // Cap Triangle
                UIAddTriPrim( a, b, c, inC, inA );

                // Cap AA
                UIAddQuadPrim(c, b, bD, cD, inC, inC, inC, inC, inA, inA, 0, 0 );
                if ( CFlip ) {
                    X = (N1-H)*0.5f*aaCornerSize;
                    UIAddTriPrim( b, bN, b+X, inC, inC, inC, inA, 0, 0 );
                    UIAddTriPrim( b, b+X, bD, inC, inC, inC, inA, 0, 0 );
                    X = (-N0-H)*0.5f*aaCornerSize;
                    UIAddTriPrim( c, cD, c+X, inC, inC, inC, inA, 0, 0 );
                    UIAddTriPrim( c, c+X, cN, inC, inC, inC, inA, 0, 0 ); 
                }
                else {
                    X = (N0-H)*0.5f*aaCornerSize;
                    UIAddTriPrim( b, bN, b+X, inC, inC, inC, inA, 0, 0 );
                    UIAddTriPrim( b, b+X, bD, inC, inC, inC, inA, 0, 0 );
                    X = (-N1-H)*0.5f*aaCornerSize;
                    UIAddTriPrim( c, cD, c+X, inC, inC, inC, inA, 0, 0 );
                    UIAddTriPrim( c, c+X, cN, inC, inC, inC, inA, 0, 0 ); 
                }
                
                // Debug
                // UIAddGlobalLine( inPnts[i-1].x, inPnts[i-1].y, inPnts[i].x, inPnts[i].y, 0x000000, 3 );
                // UIAddGlobalLine( a.x, a.y, b.x, b.y,0x9E005D, 2 );
                // UIAddGlobalLine( b.x, b.y, c.x, c.y,0x9E005D, 2 );
                // UIAddGlobalLine( c.x, c.y, d.x, d.y,0x9E005D, 2 );
                // CFlip ? UIAddGlobalCircle( inPnts[i].x, inPnts[i].y, 10, 0x00FFFF, true, 1, 16 )
                //       : UIAddGlobalCircle( inPnts[i].x, inPnts[i].y, 10, 0xFFFF00, true, 1, 16 );
                // UIAddGlobalCircle( c.x, c.y, 10, 0x0000FF, true, 1, 16 );
                // UIAddGlobalCircle( bD.x, bD.y, 5, 0x9E005D, true, 1, 16 );
                // UIAddGlobalCircle( cD.x, cD.y, 5, 0x9E005D, true, 1, 16 );
                // UIAddGlobalText( inPnts[i].x + 10, inPnts[i].y + 10, 100, 35, to_string(C1), 0x000000, vec2(0) );
                // UIAddGlobalText( inPnts[i].x + 10, inPnts[i].y + 10, 100, 35, to_string(i), 0x000000, vec2(0) );
                
                // Set c and d so they will transfer to the next segment properly
                if ( CFlip ) { 
                    C1 *= -1;
                    c = a; cN = aN;
                    d = b; dN = bN;
                }
                // else {}; // nothing to change when not flipped

                


            }
            // ------- Miter Join Section -------
            else { 

                L = T / glm::sin(angH); 
                pN = H * (float) L; 
                c = inPnts[i]-pN;
                d = inPnts[i]+pN;
                L *= aaSize / T;
                pN = H * (float) L;
                
                if ( CFlip ) { 
                    C1 *= -1; 
                    std::swap(c,d);
                    cN = c+pN;
                    dN = d-pN;
                }
                else {
                    cN = c-pN;
                    dN = d+pN;
                }

                // Base Quad
                UIAddQuadPrim( a, b, c, d, inC, inC, inC, inC, inA, inA, inA, inA ); 

                // Side AA
                UIAddQuadPrim( a, d, dN, aN, inC, inC, inC, inC, inA, inA, 0, 0 );
                UIAddQuadPrim( c, b, bN, cN, inC, inC, inC, inC, inA, inA, 0, 0 );

                // Debug
                // UIAddGlobalLine( inPnts[i-1].x, inPnts[i-1].y, inPnts[i].x, inPnts[i].y, 0x000000, 3 );
                // UIAddGlobalLine( a.x, a.y, b.x, b.y, 0xFF0000, 2 );
                // UIAddGlobalLine( b.x, b.y, c.x, c.y, 0x00FF00, 2 );
                // UIAddGlobalLine( c.x, c.y, d.x, d.y, 0x0000FF, 2 );
                // UIAddGlobalLine( d.x, d.y, a.x, a.y, 0xFF00FF, 2 );
                // CFlip ? UIAddGlobalCircle( inPnts[i].x, inPnts[i].y, 10, 0x00FFFF, true, 1, 16 )
                //       : UIAddGlobalCircle( inPnts[i].x, inPnts[i].y, 10, 0xFFFF00, true, 1, 16 );
                // UIAddGlobalCircle( a.x, a.y, 5, 0xFF0000, true, 1, 16 );
                // UIAddGlobalCircle( b.x, b.y, 5, 0x00FF00, true, 1, 16 );
                // UIAddGlobalCircle( c.x, c.y, 10, 0x0000FF, true, 1, 16 );
                // UIAddGlobalCircle( d.x, d.y, 10, 0xFF00FF, true, 1, 16 );
                // UIAddGlobalText( inPnts[i].x + 10, inPnts[i].y + 10, 100, 35, to_string(C1), 0x000000, vec2(0) );
                // UIAddGlobalText( inPnts[i].x + 10, inPnts[i].y + 10, 100, 35, to_string(i), 0x000000, vec2(0) );
                // UIAddGlobalCircle( cN.x, cN.y, 5, 0xABABAB, true, 1, 16 );
                // UIAddGlobalCircle( dN.x, dN.y, 5, 0xABABAB, true, 1, 16 );
                
            }

            End_Loop_Iteration:
            a = d; aN = dN; // aD = dD;
            b = c; bN = cN; // bD = cD;
            D0 = D1 * -1.0f;
            N0 = N1 * -1.0f;
            C0 = C1;
            l0 = l1;

        }

        // ------ Last Segment ------

        D1 = glm::normalize(inPnts[lastI] - inPnts[lastI-1]);
        N1 = UIGetNormal(D1);

        // Get quad points
        pN = N1*T;
        c = inPnts[lastI]-pN;
        d = inPnts[lastI]+pN;
        
        // Base Quad
        UIAddQuadPrim( a, b, c, d, inC, inC, inC, inC, inA, inA, inA, inA );

        // Last segment side AA quads
        pN = N1*aaSize;
        pD = D1*aaSize;
        cN = c-pN; cD = c+pD;
        dN = d+pN; dD = d+pD;
        UIAddQuadPrim( a, d, dN, aN, inC, inC, inC, inC, inA, inA, 0, 0 ); // side AA
        UIAddQuadPrim( c, b, bN, cN, inC, inC, inC, inC, inA, inA, 0, 0 ); // side AA

        // End Cap AA
        UIAddQuadPrim( d, c, cD, dD, inC, inC, inC, inC, inA, inA, 0, 0 );
        X = (D1-N1)*0.5f*aaCornerSize;
        UIAddTriPrim( c, cN, c+X, inC, inC, inC, inA, 0, 0 );
        UIAddTriPrim( c, c+X, cD, inC, inC, inC, inA, 0, 0 );
        X = (D1+N1)*0.5f*aaCornerSize;
        UIAddTriPrim( d, dD, d+X, inC, inC, inC, inA, 0, 0 );
        UIAddTriPrim( d, d+X, dN, inC, inC, inC, inA, 0, 0 );
        
        // Debug
        // UIAddGlobalLine( inPnts[lastI-1].x, inPnts[lastI-1].y, inPnts[lastI].x, inPnts[lastI].y, 0x000000, 3 );
        // UIAddGlobalLine( a.x, a.y, b.x, b.y, 0xFF0000, 2 );
        // UIAddGlobalLine( b.x, b.y, c.x, c.y, 0x00FF00, 2 );
        // UIAddGlobalLine( c.x, c.y, d.x, d.y, 0x0000FF, 2 );
        // UIAddGlobalLine( d.x, d.y, a.x, a.y, 0xFF00FF, 2 );
        // UIAddGlobalCircle( inPnts[lastI].x, inPnts[lastI].y, 10, 0xFFFF00, true, 1, 16 );
        // UIAddGlobalCircle( a.x, a.y, 5, 0xFF0000, true, 1, 16 );
        // UIAddGlobalCircle( b.x, b.y, 5, 0x00FF00, true, 1, 16 );
        // UIAddGlobalCircle( c.x, c.y, 10, 0x0000FF, true, 1, 16 );
        // UIAddGlobalCircle( d.x, d.y, 10, 0xFF00FF, true, 1, 16 );
        // UIAddGlobalCircle( cN.x, cN.y, 5, 0xABABAB, true, 1, 16 );
        // UIAddGlobalCircle( dN.x, dN.y, 5, 0xABABAB, true, 1, 16 );
        // UIAddGlobalCircle( cD.x, cD.y, 5, 0x9E005D, true, 1, 16 );
        // UIAddGlobalCircle( dD.x, dD.y, 5, 0x9E005D, true, 1, 16 );

    }

    // ============================================================================================ //
    //                                    IMAGES / TEXTURES                                         //
    // ============================================================================================ //

    void UISprite(int inTextureId, vec2 inPos, vector<float> inPixelXYWH, vec2 inImageDim, float inScale){

        // place a sprite from a rect within an image.  inScale is the scale change from pixel res to screen draw.
        // For example, if the image is double size, set inScale = .5
        ImRect spriteRect = GetSprite(inPixelXYWH[0],inPixelXYWH[1], inPixelXYWH[2], inPixelXYWH[3]);
        ImRect spriteUV = GetUV(spriteRect, inImageDim);
        float iW = inPixelXYWH[2] * inScale;
        float iH = inPixelXYWH[3] * inScale;
        ImRect imageRect = GetGlobalBB(inPos.x, inPos.y, iW, iH);
        UIImage( imageRect, inTextureId, spriteUV );

    }
    ImRect GetUV(ImRect fSprite, vec2 ImageDim){
        float u1 = fSprite.Min.x / ImageDim.x;
        float v1 = fSprite.Min.y / ImageDim.y;
        float u2 = fSprite.Max.x / ImageDim.x;
        float v2 = fSprite.Max.y / ImageDim.y;
        return ImRect( {u1, v1}, {u2, v2} );
    }
    
    int UIAddImage(ImRect inBox, string inPath, ImRect inUvBox){
        
        // compare forcing slashes to be similar
        fs::path bPath = ((fs::path)inPath).make_preferred();
        string fPath = bPath.string();

        int texID;
        int offset = UILibCTX->IO.NumberOfResidentTextures;
        // check whether path is already in image paths list
        for ( int i = 0; i < UILibCTX->IO.ImagePaths.size(); i++ ) {
            if ( fPath == UILibCTX->IO.ImagePaths[i] ) {
                texID = i + offset;
                UIImage(inBox, texID, inUvBox);
                return texID;
            }
        }

        // add image if it is not in list
        UILibCTX->IO.ImagePaths.push_back( fPath );
        texID = (int)UILibCTX->IO.ImagePaths.size() - 1 + offset;
        UIImage(inBox, texID, inUvBox);
        return texID;

    }

    int UIGetImageIndex( string inPath ) {
        fs::path bPath = ((fs::path)inPath).make_preferred();
        string fPath = bPath.string();
        int texID;
        int offset = UILibCTX->IO.NumberOfResidentTextures;

        // check whether path is already in image paths list
        for ( int i = 0; i < UILibCTX->IO.ImagePaths.size(); i++ ) {
            if ( fPath == UILibCTX->IO.ImagePaths[i] ) {
                texID = i + offset;
                return texID;
            }
        }

        // add image if it is not in list
        UILibCTX->IO.ImagePaths.push_back( fPath );
        texID = (int)UILibCTX->IO.ImagePaths.size() - 1 + offset;
        return texID;
    }
    
    void GetPngSize(const string & inPath, int & inW, int & inH) {
        std::ifstream in(inPath);
        unsigned int width, height;
        in.seekg(16);
        in.read((char *)&width, 4);
        in.read((char *)&height, 4);
        inW = ntohl(width);
        inH = ntohl(height);
    }

    void UIImage(ImRect inBox, int inTextureId, ImRect inUvBox){
        UIImage( inBox, inTextureId, {inUvBox.Min.x,inUvBox.Min.y}, {inUvBox.Max.x,inUvBox.Max.y});
    }
    void UIImage(ImRect inBox, int inTextureId, vec2 uvMin, vec2 uvMax) {
        
        ImGuiContext& g = *GImGui;ImGuiWindow* window = g.CurrentWindow; 
        ImDrawList* dl = ImGui::GetWindowDrawList();

        dl->PrimReserve(6, 4);

        // Code below is modified from ImDrawList::PrimRectUV() functions
        ImU32 col = ClWithAlpha( 0x000000, 0.0f );
        ImVec2 uv_a( uvMin.x, uvMin.y ), uv_c( uvMax.x, uvMax.y );
        ImVec2 uv_b( uvMax.x, uvMin.y ), uv_d( uvMin.x, uvMax.y );
        ImVec2 a( inBox.Min.x, inBox.Min.y ), c( inBox.Max.x, inBox.Max.y );
        ImVec2 b( inBox.Max.x, inBox.Min.y ), d( inBox.Min.x, inBox.Max.y );

        ImDrawIdx idx = (ImDrawIdx)dl->_VtxCurrentIdx;
        dl->_IdxWritePtr[0] = idx;  dl->_IdxWritePtr[1] = (ImDrawIdx)(idx+1);   dl->_IdxWritePtr[2] = (ImDrawIdx)(idx+2);
        dl->_IdxWritePtr[3] = idx;  dl->_IdxWritePtr[4] = (ImDrawIdx)(idx+2);   dl->_IdxWritePtr[5] = (ImDrawIdx)(idx+3);
        dl->_VtxWritePtr[0].pos = a;    dl->_VtxWritePtr[0].uv = uv_a;      dl->_VtxWritePtr[0].col = col;      dl->_VtxWritePtr[0].texID = inTextureId;
        dl->_VtxWritePtr[1].pos = b;    dl->_VtxWritePtr[1].uv = uv_b;      dl->_VtxWritePtr[1].col = col;      dl->_VtxWritePtr[1].texID = inTextureId;
        dl->_VtxWritePtr[2].pos = c;    dl->_VtxWritePtr[2].uv = uv_c;      dl->_VtxWritePtr[2].col = col;      dl->_VtxWritePtr[2].texID = inTextureId;
        dl->_VtxWritePtr[3].pos = d;    dl->_VtxWritePtr[3].uv = uv_d;      dl->_VtxWritePtr[3].col = col;      dl->_VtxWritePtr[3].texID = inTextureId;
        dl->_VtxWritePtr += 4;
        dl->_VtxCurrentIdx += 4;
        dl->_IdxWritePtr += 6;

    }


    // ============================================================================================ //
    //                                          UI OBJECTS                                          //
    // ============================================================================================ //

    void UISetContextMenu( string inName, const vector<string> & inOptions ) {
        UILibCTX->ContextMenu.Name = inName;
        UILibCTX->ContextMenu.Options = inOptions;
    }

    UIContextMenu * UIGetContextMenu() {
        return &UILibCTX->ContextMenu;
    }

    void UIResetStatusMessage() {
        UILibCTX->StatusMessage = UIStatusMessage();
        UILibCTX->StatusMessage_ = UIStatusMessage();
    }

    void UISetStatusMessage( string inM ) {
        UILibCTX->StatusMessage = UIStatusMessage( inM );
        UILibCTX->StatusMessage_ = UIStatusMessage();
        UIAddToStatusMessageHistory(inM);
    }

    void UISetFormattedStatusMessage( string inM, int inFCol, int inBCol, int inFInd) {
        UILibCTX->StatusMessage = UIStatusMessage( inM, inFCol, inBCol, inFInd );
        UILibCTX->StatusMessage_ = UIStatusMessage();
        UIAddToStatusMessageHistory(inM);
    }

    void UISetTimedStatusMessage( string inM, double inDur, string nextM ) {
        UILibCTX->StatusMessage = UIStatusMessage( inM, inDur );
        if ( nextM == "" ) {
            UILibCTX->StatusMessage_ = UIStatusMessage();
        } else {
            UILibCTX->StatusMessage_ = UIStatusMessage( nextM );
        }
        UIAddToStatusMessageHistory(inM);
    }

    void UISetFormattedTimedStatusMessage( string inM, int inFCol, int inBCol, int inFInd, double inDur, string nextM ) {
        UILibCTX->StatusMessage = UIStatusMessage( inM, inFCol, inBCol, inFInd, inDur );
        if ( nextM == "" ) {
            UILibCTX->StatusMessage_ = UIStatusMessage();
        } else {
            UILibCTX->StatusMessage_ = UIStatusMessage( nextM );
        }
        UIAddToStatusMessageHistory(inM);
    }

    void UISetFormattedTimedStatusMessage( string inM, int inFCol, int inBCol, int inFInd, double inDur, string nextM, int nextFCol, int nextBCol, int nextFInd ) {
        UILibCTX->StatusMessage = UIStatusMessage( inM, inFCol, inBCol, inFInd, inDur );
        UILibCTX->StatusMessage_ = UIStatusMessage( nextM, nextFCol, nextBCol, nextFInd );
        UIAddToStatusMessageHistory(inM);
    }
    
    UIStatusMessage * UIGetStatusMessage() {
        if ( UILibCTX->StatusMessage.Duration > 0 ) {
            auto now  = std::chrono::high_resolution_clock::now();
            auto diff = std::chrono::duration<double>(now - UILibCTX->StatusMessage.StartTime).count();
            if ( diff > UILibCTX->StatusMessage.Duration ) {
                UILibCTX->StatusMessage = UILibCTX->StatusMessage_;
                UILibCTX->StatusMessage_ = UIStatusMessage();
            }
        }
        return &UILibCTX->StatusMessage;
    }

    void UIAddToStatusMessageHistory( string inM ) {
        // Keep history of 20 status messages
        UILibCTX->StatusMessageHistory.push_back( inM );
        if ( UILibCTX->StatusMessageHistory.size() > 20 ) {
            UILibCTX->StatusMessageHistory.erase (UILibCTX->StatusMessageHistory.begin(),UILibCTX->StatusMessageHistory.begin()+1);
        }
    }

    vector<string> * UIGetStatusMessageHistory() {
        return &UILibCTX->StatusMessageHistory;
    }


} // end namespace UILib


#endif //  UILIB_CPP