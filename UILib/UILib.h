#ifndef  UILIB_H
#define  UILIB_H

#if defined( _WIN32 )
    //#define NOMINMAX
    //#define WIN32_LEAN_AND_MEAN     // http://forums.codeguru.com/showthread.php?560037-RESOLVED-VS-2017-and-Windows-h-issue
    #include <windows.h>
    #include <stdexcept>
    #include <algorithm>
#endif

#include <cmath>
#define _USE_MATH_DEFINES

#include <iostream>
#include <sstream>
#include <iomanip>

#include <vector>
#include <array>
#include <map>
#include <list>
#include <string>
#include <bitset>
#include <chrono>
#include <functional>
#include <unordered_set>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
//#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtx/rotate_vector.hpp>

#include <ColorSpace.h>
#include <Conversion.h>
#include <Comparison.h>

using namespace std;
using namespace glm;

#include "imgui/imgui.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
    #define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui/imgui_internal.h"
using namespace ImGui;

namespace UILib {
    
    // Forward Declarations

    struct UIForm;
    struct UIFormatScroller;

    // --------------------------------------------------------------------- //
    // Data Structures                                                       //
    // --------------------------------------------------------------------- //

    struct UIComboBox {
        string Name = "";
        int Index = 0;     // index of the current selected combo item
        bool Open = 0;
        int PPGIndex = -1;
    };

    enum uicursor_types {
        uic_none = 0,
        uic_normal,
        uic_hand,
        uic_ibeam,
        uic_crosshair,
        uic_resize_h,
        uic_resize_v,
        uic_resize_tlbr,
        uic_resize_trbl,
        uic_resize_all,
        uic_hidden,
        uic_disable,
        uic_custom
    };

    struct UICursor {
        int             Type = uic_none;
        string          Name = "";
        GLFWcursor *    Cursor = NULL;
    };

    struct UITransformBBData {
        enum TransformType {
            tt_none = 0,
            tt_drag,
            tt_resize_tl,
            tt_resize_br,
            tt_resize_tr,
            tt_resize_bl,
            tt_resize_l,
            tt_resize_r,
            tt_resize_t,
            tt_resize_b
        };
        enum TransformStatus {
            ts_none = 0,
            ts_over,
            ts_click,
            ts_down,
            ts_release
        };
        int             Type   = tt_none;
        int             Status = ts_none;
        vec2            Offset = vec2(0);
        vec2            BoundingPct = vec2(0);   // Percent of the draggable box allowed to be dragged outside the bounding container
    };

    enum ButtonMouseStatus {
        bms_none = 0,
        bms_over,
        bms_click,
        bms_down,
        bms_release
    };

    struct CharCell{
        int SIndex; // index in orginal string
        int Index; // index in TextMap
        unsigned char Char;
        float x;
        float y;
        float w;
        int Type = 0; // 0=char, 1=newline, 2=EOL
        int Syntax = 0;  // code syntax: 0=uncast, 1=op, 2=set op, 3=int, 4=float, 5=Unquoted Text, 6=Single Quoted, 7=Double Quoted, 8=Comment, op
        int Op = 0;  // Operation on Char: 0 = None, 1 = Insert Before Char, 2 = Insert After Char, 3=quote bracket '(','[','{'
        int Del = 0; // 0=keep, 1 = Delete, 2 = Deleted
        CharCell()=default;
        CharCell(int inSIndex, int inIndex, unsigned char inChar, float inX, float inY, float inW, int inType) {
            SIndex = inSIndex;Index = inIndex;Char = inChar;x = inX;y = inY;w = inW;Type = inType;
        };
        
    };

    struct TextFormat {
        unsigned int BackColor = 0xFFFFFF;
        unsigned int TextColor = 0x000000;
        string BackColorStr = ""; // hex string representation of back color, used to minimize conversion
        string TextColorStr = ""; // hex string representation of text color, used to minimize conversion
        float HAlign = 0;   // alignment variables for text justification
        float VAlign = .5f; // spreadsheet default is vertical centered
        bool Bold = false;
        bool Italic = false;
        bool ULine = false; // Underline
        bool Wrap = false;
    };

    struct UITextSelection{
        // Selection Object for Multi-Line Text Boxes
        // drag/release cords stored and mapped to start/end index - used to quick flip backwards selections
        int D = -1; // drag index
        int R = -1; // release index 
        int S = -1; // start index
        int E = -1; // end index
        int Op = 0; // selection operation being performed on text, 0=none, 1=quote text
        bool RSide = true;  // true if dragging on right side, false if left drag
        UITextSelection(){}
        UITextSelection(int inD, int inR, int inS, int inE){D=inD;R=inR;S=inS;E=inE;}
        UITextSelection(int inD, int inR, int inS, int inE, bool inRSide){D=inD;R=inR;S=inS;E=inE;RSide=inRSide;}
        void SetCaret(int inI){D=inI;R=inI;S=inI;E=inI;RSide = true;}
        void Log(){ printf("DR-SE: [%03d-%03d | %03d-%03d] \n", D,R,S,E); }
        void UpdateSE(){ if(D<R){RSide=true;S=D;E=R;}else{RSide=false;E=D;S=R;}}// update S+E from D+R
        
    };
    struct TextMap{
        
        /*
        EXAMPLE GET POSITION: if you want to get a character x,y position for character 3:
        xPos = Rect.x + Clist[3].x
        yPos = Rect.y + Clist[3].y

        NOTE: In the CList, newlines are counted as a single character With Linux newlines \n this doesn't change anything
        But if you have Windows \r\n then your index is CList is not the same as your index in a string 
        */

        vector<CharCell> CList = {};  // character cell list
        vector<array<int,2>> YRange = {}; // a list of x values (start,end) for each y line
        vector<UITextSelection> SList = {}; // selection list
        TextFormat Format = {};
        float LineH;  // line & character height
        float CharW;  // default character width
        int Language = 0; // Lanuage 0=Text Editing, 1=Python
        int Focus = 0;  // is it focused - keystrokes will become typing in this cell
        int MouseFocus = 0;  // is the mouse over this text area - typically scroll bars are visible and take mouse scroll events
        int Change = 2;  // did the text change on current frame, if changed then remake, 0 = no change, 1 = insert text, 2 = full replacement of text (undo)
        int SelChange = 0; // did the selection change on current frame
        int NbLines = 0;
        float Width = 0;   // max text width
        float Height = 0;  // max text height
        array<int,4> Font = {0,0,0,0};  // [default, bold, italic, bold italic]
        float Scale = 1;
        ImRect Rect;  /// rect for drawing text
        ImRect ClickRect; // rect where clicking will place affect placement of caret
        string Text;  // The text in this TextMap as a string
        float XBorder = 0;  // 7
        float YBorder = 0;  // 10
        float MinX = 0; // minimum X value.  Could less than 0 because centered/right format alignment can push out of the box (relative to the TextMap.Rect.Min.x)
        float MinY = 0; // minimum X value.  Could less than 0 because centered/right format alignment can push out of the box
        vec2 DragPoint = {}; // global position of drag start
        int LDrag = 0; // Left Mouse: 0=not dragging, 1=mouse down, 2=dragging or mouse up
        int MDrag = 0; // Middle Mouse: 0=not dragging, 1=mouse down, 2=dragging or mouse up
        int RightLimit = 0; // 0 = Drag start before right bounding limit of all text, 1 = started after limit
        int Shift = 0; // Shift of selection during text change: -1 = shift sel 1 char back, 0 = no shift, 1 = shift sel 1 char formrd
        int LineTransfom = 0;  // applied on selected lines - 0=none, 1=tab forward , 2=tab back, 3=add comment, 4=remove comment, 5=python indent
        std::unordered_set<int> Blacklist = {};     // Blackist keys processed by UIKeyInputSS
        std::unordered_set<int> Whitelist = {};     // Whitelist keys processed by UIKeyInputSS
        
        TextMap(){};
        // TextMap(float inLineH, int inFontI, int inBoldI, int inItalicI){LineH = inLineH;FontI = inFontI;BoldI=inBoldI;ItalicI=inItalicI;};
        // TextMap(float inLineH, int inFontI, int inBoldI, int inItalicI, int inLanguage){LineH = inLineH;FontI = inFontI;BoldI=inBoldI;ItalicI=inItalicI;Language=inLanguage;};
        TextMap(float inLineH, array<int,4> inFont, float inScale){LineH = inLineH;Font = inFont;Scale=inScale;};
        TextMap(float inLineH, array<int,4> inFont, int inLanguage, float inScale){LineH = inLineH;Font = inFont;Language=inLanguage;Scale=inScale;};
        
        
        void DeleteSel(){ // marks selection for delete but does not change the selection (in case additional changes required after)
            for(int i=0;i<SList.size();i++){for(int j=SList[i].S;j<SList[i].E;j++){CList[j].Del = 1;}}Change = 1;
        }
        int GetLine(int inCIndex){ // given Clist index, return what line it is on
            for(int i=0;i<YRange.size();i++){if(inCIndex>=YRange[i][0] && inCIndex<=YRange[i][1]){return i;}}
            return (int)YRange.size()-1;
        }
        void SetBorder(float inXB, float inYB){XBorder=inXB;YBorder=inYB;}
        void LogSel(string inTxt){
            cout << inTxt << ", SEL: ";
            for(int i=0;i<SList.size();i++){cout << "{"<<SList[i].S << " - " << SList[i].E << "} ";}cout << "\n";
        }
        string GetLineString(int inIndex){
            int maxI = (int)YRange.size() - 1;
            if(inIndex > maxI){inIndex=maxI;}
            string lStr = "";int si = YRange[inIndex][0];int se = YRange[inIndex][1];
            for(int i=si;i<se;i++){lStr += CList[i].Char;}
            return lStr;
        }
    };

    struct UIContextMenu {
        string              Name;
        vector<string>      Options;
    };
    
    struct UIStatusMessage {
        string Message = "Ready";
        int FontColor = 0x000000;
        int BkgdColor = 0xF5F5F5;
        int FontIndex = 0;
        double Duration = -1;
        std::chrono::high_resolution_clock::time_point StartTime;
        // Constructors
        UIStatusMessage() = default;
        UIStatusMessage( string inM ) : Message( inM ) {};
        UIStatusMessage( string inM, double inD ) : Message( inM ), Duration(inD) {
            if (Duration > 0) { StartTime=std::chrono::high_resolution_clock::now(); }
        };
        UIStatusMessage( string inM, int inFC, int inBC, int inFI, double inD=-1 ) : 
            Message(inM), FontColor(inFC), BkgdColor(inBC), FontIndex(inFI), Duration(inD) {
            if (Duration > 0) { StartTime=std::chrono::high_resolution_clock::now(); }
        }
    };

    struct UISyntaxInfo {

        std::string     Name;
        int             Type;                   // See: syntax_types enum
        int             FontC       = 0x000000; // Font Color
        int             FontS       =  0;       // Font Style: 0=regular, 1=bold, 2=italic
        int             Background  = -1;       // Background Color: -1=none
        int             Border      = -1;       // Border Color: -1=none
        int             Underline   = -1;       // Underline Color: -1=none   

    };

    enum syntax_types {             
        syn_uncast                           = 0,        // 0 - 10 : types of tokens for syntax highlighting
        syn_op                               = 1,         
        syn_delim                            = 2,      
        syn_int                              = 3,        
        syn_float                            = 4,      
        syn_unquoted                         = 5,   
        syn_singlequote                      = 6,
        syn_doublequote                      = 7,
        syn_comment                          = 8,    
        syn_keyword                          = 9,    
        syn_type                             = 10,
        syn_bracket                          = 11,
        syn_funcname                         = 12,
        syn_sheetformula                     = 13,
        syn_sformulain                       = 14,
        syn_sformulaerr                      = 15,
        syn_custom                           = 16
    };

    struct UIEditorInfo {

        std::string     Name;
        int             Type;               // See: editor_types enum
        int             Fill        = -1;   // Fill Color: -1=none
        int             Border      = -1;   // Border Color: -1=none
        int             Underline   = -1;   // Underline Color: -1=none 

    };

    enum editor_types {                 
        ed_cursor_active                    = 0,  
        ed_cursor_inactive                  = 1,         
        ed_highlight_active                 = 2, 
        ed_highlight_inactive               = 3,       
        ed_instance_active                  = 4,       // secondary highlight of other instances of selected word
        ed_instance_inactive                = 5,
        ed_editor_box                       = 6,       // editor text box             
        ed_editor_row                       = 7,       // selected row within editor text box              
        ed_output_highlight                 = 8,       // cell output highlights
        ed_cell_label                       = 9,      // the numeric label on the notebook cell            
        ed_cell_edit                        = 10,      // notebook cell in edit mode 
        ed_cell_select                      = 11,      // notebook cell selected, but not in edit mode
        ed_cell_highlight                   = 12,      // notebook cell highlight, when multiple cells are selected
        ed_background                       = 13,       // overall background
        ed_term                             = 14       // search term color
    };

    // --------------------------------------------------------------------- //
    // Text Input Data Structures                                            //
    //                                                                       //
    // These are newer versions of TextFormat and TextMap, where input       //
    // events are managed internally.                                        //
    // --------------------------------------------------------------------- //

    struct UITextFormat {
        int BackClr = 0xFFFFFF;         // background color
        int HighlightClr = 0xCCE1F1;    // selection highlight color
        vec2 Align = {0,0};
        bool Bold = false;
        bool Italic = false;
        bool ULine = false;
        bool Wrap = false;
        struct FontData {
            int   Index = 0;
            float Scale = 1;
            int   Color = 0x000000;
            bool operator== (const FontData& o) const { return ( Index == o.Index && Scale == o.Scale && Color == o.Color ); }
            bool operator!= (const FontData& o) const { return !(*this==o); }
        };
        vector<FontData> Syntax = {{}}; // Syntax lookup for font info
    };

    struct UITextMap {

    public:

        enum ModeType : uint8_t {
            tm_input = 0,       // basic input box
            tm_floating_input,  // floating input box (auto-resizes to text)
            tm_editor,          // standard editor
            tm_code_editor,     // code editor
            tm_viewer           // text viewer
        };
        const uint8_t         Mode; // 0 = Text Box, 1 = Text Editor, 2 = Code Editor (future)
        bool ModeIsInput() { return Mode <= tm_floating_input; }

        enum ChangeType : uint8_t {
            tm_change_none = 0, // no change1 = insert text
            tm_change_insert,   // insert (and/or deleted) text
            tm_change_full,     // full replacement of text
            tm_focus_change     // focus changed
        };
    
        ImRect          ClickRect;      // Mouse clickable bounding box
        bool            Mute = false;   // Mutes mouse clicks
        bool            AutoFocus = true; // Automatically detect when to enter or exit focus.

        UIFormatScroller *      Scroller = nullptr;    // Defaults to UILibCTX->DefaultScroller if not specified
        highp_dvec2             ScrollPerc = vec2(0);
        bvec2                   ScrollDrag = bvec2(0);
        ivec2                   ScrollState = vec2(0); // State: 0 = none, 1 = clicked bar, 2 = currently dragging bar, 3 = clicked track, 4 = released from bar, 5 = hover over track or bar
        float                   ScrollSize = 10.0f; // Scroll bar size
        float                   ScrollIncr = 60.0f; // Size of shift on mouse wheel scroll

        std::function<bool(unsigned int&)> ValidateKey = [](unsigned int&){return true; }; // Callback function for validating/modifying keys (use this to whitelist or blacklist keys)
        
        UITextMap( uint8_t inMode = 0, const string & inText = "", const UITextFormat & inFormat = {}, const ImRect & inRect = {} );
        
        bool IsEmpty(); // Indicates whether the Text is empty

        void Set( const string & inText );                                                       // Builds the TextMap from inText. All existing text is replaced.
        void Set( const string & inText, const ImRect & inRect, const UITextFormat & inFormat ); // Builds the TextMap from inText and sets Rect and Format. All existing text is replaced.
        void RebuildTextMap(); // Rebuilds the entire TextMap based on the current CList. Call this function after modifying the CList.

        void UpdateRect( const ImRect & inRect ); // Update the draw Rect
        void UpdateFormat( const UITextFormat & inFormat ); // Update the Format
        
        void Draw( ImRect & inRect, const ImRect & inClickRect );

        void SetFocus(bool inFocus);
        bool Focused();
        int  GetChange();
        vec2 GetTextDim();

        string GetText();

        void SetScrollX( float pct );   // Set the X scroll percent
        void SetScrollY( float pct );   // Set the Y scroll percent

        void SetCaretFromPos( vec2 inPos ); // Removes all selections and sets a single caret at the character closest to the input position

    private:

        UITextFormat    Format;
        ImRect          TextRect;       // Computed bounding box for all text
        ImRect          Rect;           // Text draw bounding box        
        
        bool            Focus = false; // Inicates whether text is focused (has caret and can type)
        uint8_t         Change = tm_change_none; // CList change for current frame

        float LineH;    // line & character height
        float Width;    // max text width
        float Height;   // max text height
        float Width_;   // bounding text width (Width excluding the end display space)
        vec2  Offset = vec2(0); // draw offsets

        // Character Metadata
        struct CharData {
            char     c = 0; // char value (Note: whitespace used for display purposes, but are not part of the Text have this value as 0)
            float    x = 0; // char x position relative to the start of the line
            float    w = 0; // width of the char
            int      i = -1;// index in Text representation
            int      line = 0;  // line index
            int8_t   syntax = 0; // syntax lookup index
            CharData( char inC=0 ) : c(inC) {};
        };
        vector<CharData> CList;
        float CharW = 0;        // base char width
        
        // Line Metadata
        struct LineData {
            float    width = 0;  // bounding width of the line
            int      index = 0; // index of the first character in the line
            int      count = 0; // number of characters in the line
        };
        vector<LineData> LList;
        int              NbLines;
        int              LineStartI;
        int              LineEndI;

        // Selection Metadata
        struct SelectionData {
            int D = -1; // initial drag index
            int S = -1; // start index
            int E = -1; // end index
            bool RSide = true;  // true if dragging on right side, false if left drag
            inline static int maxI = 0; // max index allowed (this should be the CList.size()-1)
            SelectionData() = default;
            SelectionData( int inIndex ) { SetCaret(inIndex); };
            SelectionData( int inS, int inE ) { SetCaret(inS); UpdateSel(inE); };
            void SetCaret( int inIndex ) { ClipIndex(inIndex); D = S = E = inIndex; RSide = true; }
            void UpdateSel( int inIndex ){ ClipIndex(inIndex); if(D<inIndex){RSide=true;S=D;E=inIndex;}else{RSide=false;E=D;S=inIndex;} }
            int GetCaret() { return RSide ? E : S; }
            void ClipIndex( int & inIndex ) { if ( inIndex < 0 ) { inIndex = 0; } else if ( inIndex > maxI ) { inIndex = maxI; } }
            bool operator== (const SelectionData& o) const { return ( D == o.D && S == o.S && E == o.E && RSide == o.RSide ); }
            bool operator!= (const SelectionData& o) const { return !(*this==o); }
        };
        vector<SelectionData> SList;        

        // Mouse Metadata
        enum EventTypes : uint8_t {
            m_none = 0,
            m_drag,
            m_up
        };
        uint8_t LDrag = m_none; // left mouse button
        uint8_t MDrag = m_none; // middle mouse button
        vec2 DragPoint = {};   // drag start point global space

        // Time Points
        std::chrono::high_resolution_clock::time_point LastBlinkTime; // time of last caret blink
        std::chrono::high_resolution_clock::time_point LastShiftTime; // time of last drag to shift caret
        std::chrono::high_resolution_clock::time_point LastKeyTime;   // time of last key press
        
        // Key Input Metadata
        int LastKey = 0;
        bool HoldDown = false;
        enum TransformTypes : uint8_t {
            lt_none = 0,
            lt_tab_forward,
            lt_tab_back,
            lt_delete,
            lt_backspace
        };
        uint8_t LineTransform = lt_none; // transformations applied to to selected lines
        
        void SetCarets( const vector<int> & inCaretPos );
        vector<int> RemoveSelectionFromText(); // Removes selected regions from the Text variable and returns a list of new caret positions
        void InsertOrReplace( const string & inInsertS );
        void TabIndent( bool forward );
        void KeyInput();
        void MouseInput();
        void DrawCaret();
        void DrawSelection();
        void DrawScroll();

    #ifndef NDEBUG
        bool DebugOn = false;
        void DrawDebug();
    #endif

        float LinePosX( int inLineI );
        float LinePosY( int inLineI );
        float CharPosX( int inCharI );
        float CharPosY( int inCharI );
        float CharWidth( int inCharI );
        bool CharVisible( int inCharI );
        ImRect GetTextBoundingBox();
        int GetActiveLine( float yPos );
        int GetActiveChar( vec2 inPos );
        vector<int> GetSelectedLines();
        vector<int> GetTextRange( int inCListStart, int inCListEnd );
        void SyncOffsetX();
        void SyncOffsetY();
        void ShiftOffsetX( float inSize );
        void ShiftOffsetY( float inSize );
        void ShiftOffsetToChar( int inCharI );
        void ResetTime( std::chrono::high_resolution_clock::time_point & tp );

        template< typename T > static T AlignUp( T inSize, T inAlign ) { return (inSize + inAlign - 1) & ~(inAlign - 1); };
        template< typename T > static T AlignDown( T inSize, T inAlign ) { return (inSize) & ~(inAlign - 1); };
    };

    // --------------------------------------------------------------------- //
    // Formatting Structs                                                    //
    // --------------------------------------------------------------------- //

    struct UIFormatScroller {
        int     TrackBkgd = 0xF1F0F0;
        float   TrackAlpha = 1;
        int     TrackBorderColor = 0xC4C4C4;
        float   TrackBorderSize = 1;
        int     BarBkgd = 0xBAB7B8;     // default bar
        float   BarAlpha = 1;
        int     OverBarBkgd = -1;       // -1 means keep same as default bar
        float   OverBarAlpha = -1;
        int     DownBarBkgd = -1;
        float   DownBarAlpha = -1;
        float   BarRounding = 2;
    };

    struct UIDataTimeline {
        double  StartVal = 0;
        double  EndVal = 1;
        double  CurrVal = 0;
        double  BaseVal = 0;
        double  MajorDiv = 0.2;
        double  MinorDiv = 0.05;

        int     BkgdColor = 0xFFFFFF;
        float   BkgdAlpha = 1;
        int     LineColor = 0x000000;
        float   LineSize = 1;
        int     Font = 0;
        float   FontScale = 1;
        vec2    TextAlign = vec2(3, 0); // relative to the bottom of the MajorTick
        bool    HideLast = true;    // Hide last label

        float   MajorTickSize = 10;
        float   MinorTickSize = 5;

        int     MarkerColor = 0xFF0000;
        float   MarkerWidth = 5.0f;
        int     PreviewColor = 0xFF0000;
        float   PreviewAlpha = 0.25f;

        int     Status = 0;     // 0 = none, 1 = over, 2 = down, 3 = dragging, 4 = release

        std::function<string(double)> LabelFunc = [](double inVal) { return ""; };
        
    };

    struct UIDataCalendar {
        
        int         Year;   // standard year (i.e., 2023)
        int         Month;  // 1 to 12 where 1 = January
        int         Day;    // 1 to 31
        int         Hour;   // 0 to 23
        int         Minute; // 0 to 59
        int         Second; // 0 to 59
        int         ViewYear;
        int         ViewMonth;   // 1 to 12 where 1 = January
        int         ViewWeekday; // first weekday of the view month (0 to 6 where 0 = Sunday)
        int         ViewRow = 0; // first row of the view month (calendars always have 6 rows)

        int         HeaderFont = 0;
        float       HeaderFontScale = 1;
        int         HeaderFontColor = 0x000000;
        float       HeaderHeight = 26;

        bool        ShowTime = false;   // Indicate whether to show time options
        float       TimeHeight[2] = { 36, 25 }; // Height of the time sections { HMS combos section, ok/cancel section }
        int         LineColor = 0xCCCCCC;
        string      ComboID = "";
        bool        ComboOpen[3] = {0}; // Cached combo open state { Hour Combo, Minute Combo, AMPM Combo }

        int         NavColor = 0xB5B4B2;

        int         DateFont[3] = {0,0,0};                              // { default, selected, weekday labels } font index
        float       DateFontScale[3] = {1,1,1};                         // { default, selected, weekday labels } font scale
        int         DateFontColor[3] = {0x000000, 0xFFFFFF, 0x000000};  // { default, selected, weekday labels } font color
        int         DateSelectedColor = 0x2EAADC;         // background color of selected date
        int         ShadeColor[2] = {0x000000, 0xFFFFFF}; // { default, selected } mouse over shade color
        float       ShadeAlpha[2] = {0.08f};              // { default, selected } mouse over shade alpha

        enum CalendarStatus {
            status_none = 0,
            status_date_change,
            status_view_change,
            status_combo_opened,
            status_combo_closed,
            status_click_cancel,
            status_click_ok
        };
        int Status = status_none;

        inline static uint64_t calIndex = 0;

        UIDataCalendar();
        UIDataCalendar( int inYear, int inMon, int inDay, int inH = 0, int inM = 0 );  // inYear is standard year, inMon is 0-11, inDay is 1-31, inH is 0-23, inM is 0-59
        ~UIDataCalendar();

        void SetDate( int inYear, int inMon, int inDay, int inH = 0, int inM = 0, int inS = 0);
        void SetView( int inYear, int inMon );

        int DaysInMonth( int inMon, bool isLeapYear );
        string MonthAsString( int inMon, bool abbrev );
        bool IsLeapYear( int inYear );

        void SyncToCombo();
        void SyncFromCombo();
        void CloseComboBoxes();

        static void DatetimeFromS( int64_t inTime, int & outY, int & outM, int & outD, int & outH, int & outMin, int & outS );
        static int64_t DatetimeToS( int inY, int inM, int inD, int inH = 0, int inMin = 0, int inS = 0 );
        static int64_t DatetimeToD( int inY, int inM, int inD );
        
        static int Weekday( int inYear, int inMon, int inDay );

    };

    // --------------------------------------------------------------------- //
    // Form Data Structures                                                  //
    // --------------------------------------------------------------------- //

    // ----- Base Class -----

    // Breakdown of an UIFormItem:
    //
    //       _______________________________________
    //      |                                       |
    //      |                margin                 |
    //      |    _______________________________    |
    //      |   |                               |   |
    //      |   |            padding            |   |
    //      |   |    _______________________    |   |
    //      |   |   |                       |   |   |
    //      |   |   |                       |   |   |
    //      |   |   |      content area     |   |   |
    //      |   |   |                       |   |   |
    //      |   |   |_______________________|   |   |
    //      |   |                               |   |
    //      |   |_______________________________|   |
    //      |                                       |
    //      |_______________________________________|
    //
    // NOTES: 
    //  * Pos and Dim include the padding and content area rectangles
    //  * Pos.xy is the top left corner of the padding rectangle
    //  * Margins should be set by the PARENT UpdateDim(..)
    //  * Padding should be set by the ITEM UpdateDim(...)
    //  * AutoDim() returns dimensions for the content area of the ITEM
    //  * If an ITEM contains sub-ITEMs, then it needs to:
    //      * Set sub-ITEM Margins in its UpdateDim(...) 
    //      * Account for sub-ITEM margins in AutoDim()
    //

    enum UIAlignItemsTypes {
        align_start = 0,        //   |---    |
        align_end,              //   |    ---|
        align_center,           //   |  ---  |
        align_outer,            //   |-  -  -|
        align_even              //   | - - - |
    };

    enum UIDirectionTypes {
        v_forward = 0,
        h_forward = 1,
        v_reverse = 2,
        h_reverse = 3
    };

    enum uiform_types {
        uif_item = 0,
        uif_form,
        uif_spacer,
        uif_divider,
        uif_rectangle,
        uif_rectangle_df,
        uif_centered_shape,
        uif_grid,
        uif_header,
        uif_text,
        uif_textab,
        uif_textinput,
        uif_labeledtext,
        uif_labeledtextinput,
        uif_dateinput,
        uif_labeleddateinput,
        uif_paragraphinput,
        uif_image,
        uif_button,
        uif_labeledbutton,
        uif_selbutton,
        uif_labeledimage,
        uif_imagebutton,
        uif_labeledimagebutton,
        uif_labeledselimagebutton,
        uif_textbutton,
        uif_labeledtextbutton,
        uif_collapserbutton,
        uif_labeledcollapserbutton,
        uif_buttonrow,
        uif_selimagebutton,
        uif_seltextbutton,
        uif_selbuttonrow,
        uif_togglebuttons,
        uif_selbuttongrid,
        uif_slider,
        uif_labeledslider,
        uif_animationtimeline,
        uif_combobox,
        uif_labeledcombobox,
        uif_checkbox,
        uif_labeledcheckbox,
        uif_collapsercheckbox,
        uif_labeledcollapsercheckbox,
        uif_labeleditem,
        uif_custom,
        uif_custom_external
    };

    struct UIFormItem {

        int             ItemType = uif_item;
        string          ItemName = "";          // Name identifier

        // Rules set by the user
        string          X="", Y="", W="", H="";                                         // "10" pixels, "10%" of the PARENT content dimension
        string          MarginLeft="", MarginRight="", MarginTop="", MarginBottom="";   // "10" pixels, "10%" of the PARENT content dimension
        string          PadLeft="", PadRight="", PadTop="", PadBottom="";               // "10" pixels, "10%" of the CURRENT ITEM full dimension (content + padding)
        float           Expand  = 0;            // Ignore Expand is W or H is not blank. Otherwise, 0 = shrink to minimum, 1 = fully expanded
        bool            Visible = true;         // Indicates whether the item is visible in its space.
        bool            Collapse = false;       // Indicates whether the item is collapsed. If true, then the item takes up no space and is not visible.

        // Variables for drawing internally
        vec2            Pos = glm::vec2(0);     // X, Y -- these are LOCAL coordinates
        vec2            Dim = glm::vec2(0);     // W, H
        vec4            Padding = {0,0,0,0};    // inner space between edge and content
        vec4            Margin  = {0,0,0,0};    // outer space between different form items
        int             Index = -1;             // index in the Items vector of the UIForm object
        vec2            MinDim = {0,0};
        vec2            MaxDim = {FLT_MAX, FLT_MAX};

        struct BoxFormat {
            float       BorderSize  = -1;        // -1 means use the Default format
            int         BorderColor = -1;
            int         BkgdColor   = -1;
            float       BkgdAlpha   = -1;
            int         Font        = -1;
            int         FontColor   = -1;
            float       FontUnderline = -1;
            float       FontScale = 1;
            float       ShadowBlurDist = -1;
            float       ShadowAlpha = -1;
            int         ShadowRes = 15;
            int         ShadowColor = 0x000000;
            float       ShadowXO = 1;
            float       ShadowYO = 1;
        };

        UIFormItem( int inType ) : ItemType( inType ) {};
        virtual ~UIFormItem() = default;

        ImRect GetItemRect( float inX = 0, float inY = 0 );         // Gets rect for the ITEM
        ImRect GetContentRect( float inX = 0, float inY = 0 );      // Gets rect for the ITEM content area
        virtual bool IsVisible( ImRect inView, float inX = 0, float inY = 0  ); // Returns whether the item is visible within a specific view rectangle
        virtual void ResetDraw() {};                                // resets variables without drawing -- this is called when the item is not visible
        virtual void RequestUpdate() {};                            // request dimension update (mainly used by UIForm class)

        virtual ImRect GetFocusedRect( float inX = 0, float inY = 0);                            // Gets rect for the ITEM focused area (same content area if the item doesn't have a sub focus item)
        virtual void Draw( float inX, float inY, bool clickMute ) {}; // Pass in X and Y of the PARENT
        virtual void UpdateDim( float inW, float inH );             // Pass in W and H for the ITEM
        virtual void UpdatePadding();                               // Padding should be updated by the ITEM when UpdateDim(...) is called
        virtual vec2 & SetPos( float inX, float inY, float parentW, float parentH, float offsetX=0, float offsetY=0 );  // Pass in X and Y defaults for the ITEM. Pass in the content W and H of the PARENT. Offset X and Y if PARENT has padding.
        virtual vec4 & SetMargin( float parentW, float parentH );   // Pass in the content W and H of the PARENT.
        virtual vec2 GetAutoDim() { return MinDim; };               // Recommended dimensions of the FormItem. This is called when Expand = 0 and there is no W or H specified.
        
        virtual bool SetFocus( bool inFocus, int inDirection = 1, bool force = false ) { return false; };           // 1 = forward direction, -1 = backward direction
        virtual bool SetNextFocus( int inDirection, bool inWrap ) { return SetFocus(false); };  // 1 = forward direction, -1 = backward direction
        virtual bool GetFocus() { return false; };
        virtual int  GetFocusType() { return -1; };     // Returns -1 for item that doesn't allow focus, otherwise should return item type
        virtual bool RevertFocus() { return false; }    // Restores the last focus state
        virtual bool EscKeyFocus() { return SetFocus( false ); }
        virtual bool EnterKeyFocus() { return SetFocus( false ); };
        virtual bool ArrowKeyFocus( int inKey ) { return GetFocus(); };
        virtual bool ProcessKeyDown( int inKey ) { return false; } // Function to indicate whether the key down is handled by the parent UIForm or the focused item

        virtual bool GetOverlayState() { return false; }
        virtual vector<ImRect> GetOverlayRects() { return {}; }
        virtual void DrawOverlay( float parentX, float parentY ) {};

        virtual void SetCollapse( bool inCollapse ) { Collapse = inCollapse; };

        virtual bool ScrollChange( float inX, float inY, float inChange ) { return false; }

        // Helper Functions
        float DecodeValue( string inCode, float inVal );
        void  DecodeValueAdd( string inCode, float & inVal, float & inPct );
        float GetTotalWithPadding( string inCode1, string inCode2, float inVal );
        BoxFormat GetBoxFormat( BoxFormat & inFormat, BoxFormat & defaultFormat );

        
    };

    // ----- Non-Drawing Building Block Items -----

    struct UIFormCollapser {

        bool                    IsOpen = true;
        vector<UIFormItem*>     Items = {};
        bool &                  NeedUpdate;     // indicates when dimension update is needed (pass in the root form NeedUpdate bool)
        vector<bool>            Cache = {};
        bool                    ItemsCollapse = false;
        bool                    LastSetOpen = true;       

        UIFormCollapser( bool & inNeedUpdate ) : NeedUpdate( inNeedUpdate ) { ItemsCollapse = !IsOpen; LastSetOpen = IsOpen; };
        ~UIFormCollapser() = default;

        void        AddItem( UIFormItem * inItem );
        void        ToggleState();
        void        SetState( bool inState, bool force = false );
        void        SetItemsCollapse( bool inCollapse );
        void        SyncState();

    };

    // ----- Framing Items -----

    struct UIFormSpacer : UIFormItem {
        UIFormSpacer() : UIFormItem( uif_spacer ) {};
        ~UIFormSpacer() = default;
    };

    struct UIFormDivider : UIFormItem {
        
        int             Direction = 1;              // 0 = vertical, 1 = horizontal
        float           Thickness = 1;
        int             Color = 0x000000;

        UIFormDivider() : UIFormItem( uif_divider ) {};
        ~UIFormDivider() = default;

        void Draw( float inX, float inY, bool clickMute ) override final;
        vec2 GetAutoDim() override final;

    };

    // ----- Basic Items -----

    struct UIFormRectangle : UIFormItem {

        float       BorderSize  =   0;
        int         BorderColor =   0;
        int         BkgdColor   =  -1;
        float       BkgdAlpha   =   1;

        float       Rounding = 0;
        string      CornerBits = "0000";

        UIFormRectangle() : UIFormItem( uif_rectangle ) {};
        ~UIFormRectangle() = default;

        void Draw( float inX, float inY, bool clickMute ) override;

    };

    struct UIFormRectangleDynamicFill : UIFormRectangle {

        int &     FillColor;

        UIFormRectangleDynamicFill( int & inColor ) : UIFormRectangle(), FillColor(inColor) { ItemType = uif_rectangle_df; };
        ~UIFormRectangleDynamicFill() = default;

        void Draw( float inX, float inY, bool clickMute ) override final;
    };

    struct UIFormCenteredShape : UIFormItem {

        int         Color       =  0x9D9D9D;
        float       Size        =   0.3f; // size between 0 & 1 to fill the rect
        int         ShapeType   =   0; // 0=bullet, 1=triangle open, 2=triangle close
        bool &      Click;      // True on first frame of mouse down
        bool        Focused = false;

        UIFormCenteredShape(bool & inClick) : UIFormItem(uif_button), Click( inClick ) {};
        ~UIFormCenteredShape() = default;

        void Draw( float inX, float inY, bool clickMute ) override;

    };
    



    struct UIFormGrid : UIFormItem {

        uvec2                           GridSize;       // { nRows, nCols }

        vector<UIFormItem*>             Items;  // Item list is row major

        vec2            SpacerSize = glm::vec2(0);
        vec2            BlockSize = glm::vec2(0);
        vec2            ContentDim = glm::vec2(0); // Total dimensions of buttons (used to calculate scroll offset)

        int             FocusedItem = -1;
        int             LastFocusedItem = -1;
        int             OverlayItem = -1;

        UIFormGrid( int nRows, int nCols ) : UIFormItem(uif_grid), GridSize( uvec2(nRows,nCols) ) {};
        ~UIFormGrid();

        uvec2 GetRowCol( int inIndex );

        void Draw( float inX, float inY, bool clickMute ) override;
        void UpdateDim( float inW, float inH ) override final;
        vec2 GetAutoDim() override final;

        bool SetFocus( bool inFocus, int inDirection = 1, bool force = false ) override final;
        bool SetNextFocus( int inDirection, bool inWrap ) override final;
        bool GetFocus() override final;
        int  GetFocusType() override final;
        bool RevertFocus() override final;
        UIFormItem * GetFocusedItem();
        ImRect GetFocusedRect( float inX = 0, float inY = 0 ) override final;
        bool EnterKeyFocus() override final;

        bool GetOverlayState() override final;
        vector<ImRect> GetOverlayRects() override final;
        void DrawOverlay( float parentX, float parentY ) override final;

        void ResetDraw() override final;

        int AddItem( UIFormItem* inItem );  // Note: Items are deleted in the destructor unless ItemType is set to uif_custom_external.

    };

    // ----- Text Items -----

    struct UIFormHeader : UIFormItem {

        string          Text        = "";

        BoxFormat       Format;
        vec2            Alignment = {0,0};

        bool            FillItem = false;

        UIFormHeader( string inText = "" ) : UIFormItem(uif_header), Text( inText ) {
            Format.Font = 0; Format.FontColor = 0x000000;
        };
        ~UIFormHeader() = default;

        void Draw( float inX, float inY, bool clickMute ) override final;
        vec2 GetAutoDim() override final;

    };

    struct UIFormText : UIFormItem {

        string          Text        = "";
        
        BoxFormat       Format;
        vec2            Alignment = {0,0};
        bool            Multiline = false;

        UIFormText( string inText = "" ) : UIFormItem(uif_text), Text( inText ) {
            Format.Font = 0; Format.FontColor = 0x000000;
        };
        ~UIFormText() = default;

        void Draw( float inX, float inY, bool clickMute ) override final;
        vec2 GetAutoDim() override final;

    };

    struct UIFormTextAB : UIFormItem {

        string          Text        = "";
        
        BoxFormat       FormatTrue;
        BoxFormat       FormatFalse;
        vec2            Alignment = {0,0};
        bool            Multiline = false;

        bool &          Condition;

        UIFormTextAB( bool & inCondition, string inText = "" ) : UIFormItem(uif_textab), Condition( inCondition ), Text( inText ) {
            FormatTrue.Font = 0; FormatTrue.FontColor = 0x000000;
            FormatFalse.Font = 0; FormatFalse.FontColor = 0x000000;
        };
        ~UIFormTextAB() = default;

        void Draw( float inX, float inY, bool clickMute ) override final;
        vec2 GetAutoDim() override final;

    };

    struct UIFormTextInput : UIFormItem {

        BoxFormat       Default = { 1, 0xABABAB, 0xFFFFFF,  1,  0,  0 };
        BoxFormat       Focus   = { 1, 0x2BA9E0, -1,       -1, -1, -1 }; 

        string &        Text;
        vec2            Alignment = {0,0.5f};

        float           Rounding = 0;
        string          CornerBits = "0000";
    
        int             Mode = 1;     // 0= not focused, 1= input text, 2=input chart string  // The mode that gets registered to UILibCTX->InputBoxMode when this object grabs focus.
        std::function<bool()>       EnterFocus = [](){ return true; };
        std::function<bool(bool)>   ExitFocus  = []( bool override ){ return true; };
        std::function<bool(unsigned int&)>   ValidateKey = []( unsigned int & key ) { return true; };

        bool            Focused = false;
        bool            Mute = false;

        UIFormTextInput( string & inText ) : UIFormItem(uif_textinput), Text( inText ) {};
        ~UIFormTextInput();

        void Draw( float inX, float inY, bool clickMute ) override final;
        void UpdateDim( float inW, float inH ) override final;
        vec2 GetAutoDim() override final;

        bool SetFocus( bool inFocus, int inDirection = 1, bool force = false ) override final;
        bool GetFocus() override final;
        int  GetFocusType() override final { return ItemType; };
        bool RevertFocus() override final;

        bool GetOverlayState() override final;
        vector<ImRect> GetOverlayRects() override final;
        void DrawOverlay( float parentX, float parentY ) override final;

    };

    struct UIFormDateInput : UIFormItem {

        BoxFormat       Default = { 1, 0xABABAB, 0xFFFFFF,  1,  0,  0 };
        BoxFormat       Focus   = { 1, 0x2BA9E0, -1,       -1, -1, -1 };

        string &        Text;   // Datetime string
        UITextMap       TMap;
        UITextFormat    TFormat; // used internally (Default & Focus fonts override this)
        string          TextCache;
        
        // Use basic codes to indicate format:
        // m = numeric month, d = day, y = year, H = 24 hour, h = 12 hour, M = minute, A = AM/PM, a = am/pm
        // Repeat the letters to indicate how many digits. For months, 3 repeats is abbreviated text, and 4 is full text.
        // All other characters are left as is.
        // For example, January can be represented as:
        //   m    --> 1
        //   mm   --> 01
        //   mmm  --> Jan
        //   mmmm --> January
        // Example formats:
        //   m/d/yy         -->   4/7/23
        //   mm/dd/yyyy     -->   04/07/2023
        //   mmm d, yyyy    -->   Apr 7, 2023
        //   mmmm dd, yyyy hh:mm a   -->   April 07, 2023 04:22 pm
        string          TimeFormat = "m/d/yyyy";

        float           Rounding = 0;
        string          CornerBits = "0000";

        UIDataCalendar  Calendar; // Calendar data (specify)
        vec2            CalDim = { 200, 200 }; // Calendar widget outer size
        vec2            CalPad = { 2, 2 };  // Calendar XY white space padding
        ImRect          CalRect;  // [used internally] - Calendar draw rectangle. This is built off the CalDim.

        bool            Focused = false;
        bool            Mute = false;

        // Internal Variables (do not set these)

        UIFormDateInput( string & inText, bool inShowTime );
        ~UIFormDateInput();

        void Draw( float inX, float inY, bool clickMute ) override final;
        void UpdateDim( float inW, float inH ) override final;
        vec2 GetAutoDim() override final;

        bool SetFocus( bool inFocus, int inDirection = 1, bool force = false ) override final;
        bool SetNextFocus( int inDirection, bool inWrap ) override final;
        bool GetFocus() override final;
        int  GetFocusType() override final { return ItemType; };
        bool RevertFocus() override final;
        bool EscKeyFocus() override final;
        bool EnterKeyFocus() override final;
        bool ProcessKeyDown( int inKey ) override final;

        bool GetOverlayState() override final;
        vector<ImRect> GetOverlayRects() override final;
        void DrawOverlay( float parentX, float parentY ) override final;
        
        void UpdateInputText( const string  & inDateText );
        void UpdateTimeFormat( const string & inFormat );
        void UpdateDateTime( int inYear, int inMon, int inDay, int inH = 0, int inM = 0, int inS = 0 );
        void TimeDataToText();
        void TextToTimeData();
    };

    struct UIFormParagraphInput : UIFormItem {

        BoxFormat       Default = { 1, 0xABABAB, 0xFFFFFF,  1,  0,  0 };
        BoxFormat       Focus   = { 1, 0x2BA9E0, -1,       -1, -1, -1 };

        UITextMap       TMap;
        UITextFormat    TFormat; // used internally (Default & Focus fonts override this)
        
        float           Rounding = 0;
        string          CornerBits = "0000";

        bool            Focused = false;
        bool            Mute = false;

        // Internal Variables (do not set these)

        UIFormParagraphInput( const string & inText, bool inViewOnly, bool inWordWrap );
        ~UIFormParagraphInput();

        void Draw( float inX, float inY, bool clickMute ) override final;
        void UpdateDim( float inW, float inH ) override final;
        vec2 GetAutoDim() override final;

        bool SetFocus( bool inFocus, int inDirection = 1, bool force = false ) override final;
        bool GetFocus() override final;
        int  GetFocusType() override final { return ItemType; };
        bool RevertFocus() override final;
        bool EscKeyFocus() override final;
        bool EnterKeyFocus() override final;
        bool ProcessKeyDown( int inKey ) override final;

        bool ScrollChange( float inX, float inY, float inChange ) override final;

    };

    // ----- Image Items -----

    enum UIFormImageFit {
        imgfit_none = 0,    // no attempts at resizing
        imgfit_uniform,     // resized to fit within content area, maintaining aspect ratio
        imgfit_uniformfill, // resized to fill the content area, maintaining aspect ration
        imgfit_fill         // resized match the content area, not maintaining aspect ratio
    };

    struct UIFormImage : UIFormItem {

        string          Path;
        int             TextureId;
        vec2            ImageDim;
        vec2            PixelDim;
        ImRect          PixelUV;
        float           Scale = 1.0f;
        int             ImageFit = imgfit_none;          
        vec2            Alignment = { 0.5, 0.5 };

        UIFormImage( int TextureId, vec2 inImageDim, vec4 inPixelXYWH = vec4(0) );
        UIFormImage( string inPath, vec2 inImageDim, vec4 inPixelXYWH = vec4(0) );
        ~UIFormImage() = default;

        void UpdatePixels( const vec4 & inPixelXYWH );

        void Draw( float inX, float inY, bool clickMute ) override final;
        vec2 GetAutoDim() override final;
        ImRect GetImageRect( ImRect inContentRect );

    };

    // ----- Button Items -----

    struct UIFormButton : UIFormItem {

        BoxFormat       Default;
        BoxFormat       Over;
        BoxFormat       Down;
        BoxFormat       Focus; 
        
        std::function<void(ImRect)> DrawNullButton = []( ImRect inRect ){};     // Custom function to draw something when button is in a Null State (Default.BkgdColor = -1 and Default.BorderSize = -1)

        float           Rounding = 0;
        string          CornerBits = "1111";

        bool &          Click;      // True on first frame of mouse down
        bool            Focused = false;

        UIFormButton( bool & inClick ) : UIFormItem(uif_button), Click( inClick ) {
            Default.BorderSize  =  1;           // Default button is no color, with 1 px black border
            Default.BorderColor = 0x000000;
            Default.BkgdColor   = -1;
            Default.BkgdAlpha   =  1;
            Default.Font        = 0;
            Default.FontColor   = 0x000000;
            Default.FontUnderline = -1;
        };
        ~UIFormButton() = default;

        void Draw( float inX, float inY, bool clickMute ) override;
        int  GetFocusType() override final { return ItemType; };
        void ResetDraw() override;  // need to reset click

    };

    struct UIFormSelButton : UIFormButton {

        BoxFormat       Select;
        bool            Selected = false;

        UIFormSelButton( bool & inClick ) : 
            UIFormButton( inClick) { ItemType = uif_selbutton; };
        ~UIFormSelButton() = default;

        void Draw( float inX, float inY, bool clickMute ) override;

    };

    struct UIFormImageButton : UIFormButton { 

        UIFormImage     Image;
        int             ImageOrder = 0;     // 0 = Image below button draw, 1 = Image above button draw
        
        
        UIFormImageButton( bool & inClick, int inTextureId, vec2 inImageDim, vec4 inPixelXYWH = vec4(0) );
        UIFormImageButton( bool & inClick, string inPath, vec2 inImageDim, vec4 inPixelXYWH = vec4(0) );
        ~UIFormImageButton() = default;

        void Draw( float inX, float inY, bool clickMute ) override;
        void UpdateDim( float inW, float inH ) override final;
        vec2 GetAutoDim() override final;

    };

    struct UIFormSelImageButton : UIFormImageButton {

        BoxFormat       Select;
        bool            Selected = false;

        UIFormSelImageButton( bool & inClick, int inTextureId, vec2 inImageDim, vec4 inPixelXYWH = vec4(0) ) : 
            UIFormImageButton( inClick, inTextureId, inImageDim, inPixelXYWH ) { ItemType = uif_selimagebutton; };
        UIFormSelImageButton( bool & inClick, string inPath, vec2 inImageDim, vec4 inPixelXYWH = vec4(0) ) : 
            UIFormImageButton( inClick, inPath, inImageDim, inPixelXYWH ) { ItemType = uif_selimagebutton; };
        ~UIFormSelImageButton() = default;

        void Draw( float inX, float inY, bool clickMute ) override;

    };

    struct UIFormTextButton : UIFormButton {

        string          Text        = "";
        vec2            Alignment   = {0.5f, 0.5f};
        bool            Multiline   = false;

        UIFormTextButton( bool & inClick, string inText = "" ) : 
            UIFormButton(inClick), Text( inText ) { ItemType = uif_textbutton; Default.Font = 0; };
        ~UIFormTextButton() = default;

        void Draw( float inX, float inY, bool clickMute ) override;
        vec2 GetAutoDim() override final;

    };

    struct UIFormCollapserButton : UIFormButton, UIFormCollapser {

        enum CollapserType {        // odd -> left, even -> right
            caret_left = 0,
            caret_right,
            chevron_left,
            chevron_right,
            plusminus_left,
            plusminus_right
        };

        CollapserType       Type = caret_left;
        vec2                TriangleDim;
        bool                ButtonClick = false;

        UIFormCollapserButton( float inB, float inH, bool & inNeedUpdate ) : 
            UIFormButton(ButtonClick), UIFormCollapser(inNeedUpdate), TriangleDim(inB,inH) { 
                Default.BorderSize  = -1;       // Default collapser button is black with no border
                Default.BorderColor = 0x000000;
                Default.BkgdColor   = 0x000000;
                Default.BkgdAlpha   =  1;
                ItemType = uif_collapserbutton;
        };
        ~UIFormCollapserButton() = default;

        void Draw( float inX, float inY, bool clickMute ) override final;
        vec2 GetAutoDim() override final;
        void SetCollapse( bool inCollapse ) override final;

    };

    struct UIFormSelTextButton : UIFormTextButton {

        BoxFormat       Select;
        bool            Selected = false;

        UIFormSelTextButton( bool & inClick, string inText = "" ) : 
            UIFormTextButton( inClick, inText ) { ItemType = uif_seltextbutton; };
        ~UIFormSelTextButton() = default;

        void Draw( float inX, float inY, bool clickMute ) override;

    };

    struct UIFormButtonRow : UIFormItem {

        vector<bool*>                   ClickList;

        vector<UIFormButton*>   Buttons;
        
        float           SpacerSize = 0;
        vec2            ContentDim = glm::vec2(0); // Total dimensions of buttons (used to calculate scroll offset)
        float           ScrollTrackSize = 10;
        bool            ScrollDragging = false;
        float           ScrollPerc = 0;
        bool            AllowScroll = true;

        int             FocusedItem = -1;

        UIFormButtonRow() : UIFormItem(uif_buttonrow) {};
        ~UIFormButtonRow();

        void Draw( float inX, float inY, bool clickMute ) override;
        void UpdateDim( float inW, float inH ) override final;
        vec2 GetAutoDim() override final;

        ImRect GetFocusedRect( float inX = 0, float inY = 0 ) override final;
        bool ScrollChange( float inX, float inY, float inChange ) override final;

        UIFormImageButton* AddImageButton( int inTextureId, vec2 inImageDim, vec4 inPixelXYWH = vec4(0) );
        UIFormImageButton* AddImageButton( string inPath, vec2 inImageDim, vec4 inPixelXYWH = vec4(0) );
        UIFormTextButton* AddTextButton( string inText = "" );
        UIFormSelTextButton* AddSelTextButton( string inText = "" );

    };

    struct UIFormToggleButtons : UIFormItem {

        int &                           SelectedIndex;
        bool &                          Click;
        vector<string>                  ItemList = {};
        BoxFormat                       Format;
        float                           Round = 5;
        float                           Spacer = 12; // spacing on the left and right side of text

        UIFormToggleButtons( int & inButtonIndex, bool & inClick, vector<string> & inItemList);
        
        void Draw( float inX, float inY, bool clickMute ) override;
        void ResetDraw() override final;
        vec2 GetAutoDim() override final;

    };

    struct UIFormSelButtonRow : UIFormItem {

        int &                           SelectedIndex;

        vector<bool*>                   ClickList;
        vector<int>                     ButtonIndex;
        vector<int>                     ButtonType;     // 0 = Image, 1 = Text

        vector<UIFormSelImageButton*>   ImageButtons;
        vector<UIFormSelTextButton*>    TextButtons;

        float           SpacerSize = 0;
        vec2            ContentDim = glm::vec2(0); // Total dimensions of buttons (used to calculate scroll offset)
        float           ScrollTrackSize = 10;
        bool            ScrollDragging = false;
        float           ScrollPerc = 0;
        bool            AllowScroll = true;

        int             FocusedItem = -1;

        UIFormSelButtonRow( int & inButtonIndex ) : UIFormItem(uif_selbuttonrow), SelectedIndex( inButtonIndex ) {};
        ~UIFormSelButtonRow();

        void Draw( float inX, float inY, bool clickMute ) override;
        void UpdateDim( float inW, float inH ) override final;
        vec2 GetAutoDim() override final;

        ImRect GetFocusedRect( float inX = 0, float inY = 0 ) override final;
        void ResetDraw() override final;
        bool ScrollChange( float inX, float inY, float inChange ) override final;

        UIFormSelImageButton* AddSelImageButton( int inTextureId, vec2 inImageDim, vec4 inPixelXYWH = vec4(0) );
        UIFormSelImageButton* AddSelImageButton( string inPath, vec2 inImageDim, vec4 inPixelXYWH = vec4(0) );
        UIFormSelTextButton* AddSelTextButton( string inText = "" );

    };

    struct UIFormSelButtonGrid : UIFormItem {

        int &                           SelectedIndex;

        uvec2                           GridSize;       // { nRows, nCols }

        vector<bool*>                   ClickList;
        vector<int>                     ButtonIndex;
        vector<int>                     ButtonType;     // 0 = Image, 1 = Text

        vector<UIFormSelImageButton*>   ImageButtons;
        vector<UIFormSelTextButton*>    TextButtons;

        vec2            SpacerSize = glm::vec2(0);
        vec2            BlockSize = glm::vec2(0);
        vec2            ContentDim = glm::vec2(0); // Total dimensions of buttons (used to calculate scroll offset)

        int             FocusedItem = -1;

        UIFormSelButtonGrid( int & inButtonIndex, int nRows, int nCols ) : UIFormItem(uif_selbuttongrid), SelectedIndex( inButtonIndex ), GridSize( uvec2(nRows,nCols) ) {};
        ~UIFormSelButtonGrid();

        uvec2 GetRowCol( int inIndex );

        void Draw( float inX, float inY, bool clickMute ) override;
        void UpdateDim( float inW, float inH ) override final;
        vec2 GetAutoDim() override final;

        ImRect GetFocusedRect( float inX = 0, float inY = 0 ) override final;
        void ResetDraw() override final;

        UIFormSelImageButton* AddSelImageButton( int inTextureId, vec2 inImageDim, vec4 inPixelXYWH = vec4(0) );
        UIFormSelImageButton* AddSelImageButton( string inPath, vec2 inImageDim, vec4 inPixelXYWH = vec4(0) );
        UIFormSelTextButton* AddSelTextButton( string inText = "" );

    };

    // ----- Widget Items -----

    struct UIFormSlider : UIFormItem {

        float &             Value;
        bool &              Change;
        bool &              Mute;

        vec2                Bounds;
        int                 Direction = 1;     // 0 = vertical, 1 = horizontal

        UIFormSlider( float & inValue, bool & inMute, bool & inChange, vec2 inBounds ) : 
            Value( inValue), Mute( inMute ), Change( inChange ), Bounds( inBounds ), UIFormItem(uif_slider) {};
        ~UIFormSlider() = default;

        void Draw( float inX, float inY, bool clickMute ) override final;
        vec2 GetAutoDim() override final;

    };

    struct UIFormLabeledSlider : UIFormItem {

        UIFormSlider        Slider;
        UIFormTextButton    ValueLabel;
        bool                ValueClick = false;

        int                 Precision = 2;  // Default is 2 decimal places
        float               Spacer = 5;

        UIFormLabeledSlider( float & inValue, bool & inMute, bool & inChange, vec2 inBounds );
        ~UIFormLabeledSlider() = default;

        void Draw( float inX, float inY, bool clickMute ) override final;
        void UpdateDim( float inW, float inH ) override final;
        vec2 GetAutoDim() override final;

        float GetValueLabelWidth();

    };

    struct UIFormAnimationTimeline : UIFormItem {

        UIDataTimeline      Timeline;

        UIFormAnimationTimeline() : UIFormItem(uif_animationtimeline) {};
        ~UIFormAnimationTimeline() = default;

        void Draw( float inX, float inY, bool clickMute ) override final;
        vec2 GetAutoDim() override final;

        void ResetDraw() override final { Timeline.Status = 0; }

    };

    struct UIFormComboBox : UIFormItem {

        BoxFormat               Default  = { 1, 0xABABAB, 0xFFFFFF,  1 };
        BoxFormat               Focus    = { 1, 0x2BA9E0, -1, -1 };
        
        vector<string> &        ItemList;
        UIComboBox              ComboBox;

        float                   OpenCellH = 20;
        float                   OpenMaxH = 150;
        BoxFormat               OpenOver = { 0, 0, 0x2EAADC, 1 };

        int                     Font        = 0;
        int                     FontColor   = 0x000000;
        int                     FontColorOver = 0xFFFFFF;
        vec2                    Alignment = {0,0.5f};

        bool                    Focused = false;
        int                     LastOverIndex = -1;
        bool                    Mute = false;
        bool                    Clicked = false;

        UIFormComboBox( string inName, vector<string> & inItemList );
        ~UIFormComboBox() = default;

        void Draw( float inX, float inY, bool clickMute ) override final;
        vec2 GetAutoDim() override final;

        bool SetFocus( bool inFocus, int inDirection = 1, bool force = false ) override final;
        bool GetFocus() override final;
        int  GetFocusType() override final { return ItemType; };
        bool RevertFocus() override final;
        bool EnterKeyFocus() override final;
        bool ArrowKeyFocus( int inKey ) override final;
        void ScrollToOverIndex();
    
        bool GetOverlayState() override final;
        vector<ImRect> GetOverlayRects() override final;
        void DrawOverlay( float parentX, float parentY ) override final;
        void ResetDraw() override final;

    };

    struct UIFormCheckBox : UIFormItem {

        bool &                  IsChecked;
        
        BoxFormat               Default  = { 1, 0xABABAB, 0xFFFFFF,  1 };
        BoxFormat               Checked;
        BoxFormat               Focus    = { 1, 0x2BA9E0, -1, -1 };

        int                     CheckColor = 0x228BC5;

        bool                    Focused = false;

        UIFormCheckBox( bool & inCheck ) : UIFormItem(uif_checkbox), IsChecked( inCheck ) {};
        ~UIFormCheckBox() = default;

        void Draw( float inX, float inY, bool clickMute ) override;
        vec2 GetAutoDim() override final;

        bool SetFocus( bool inFocus, int inDirection = 1, bool force = false ) override final;
        bool GetFocus() override final;
        int  GetFocusType() override final { return ItemType; };
        bool RevertFocus() override final;
        bool EnterKeyFocus() override final;
        
    };

    struct UIFormCollapserCheckBox : UIFormCheckBox, UIFormCollapser {

        UIFormCollapserCheckBox( bool & inCheck, bool & inNeedUpdate );
        ~UIFormCollapserCheckBox() = default;

        void Draw( float inX, float inY, bool clickMute ) override final;
        void SetCollapse( bool inCollapse ) override final;

    };

    struct UIFormLabeledItem : UIFormItem {

        UIFormText          Label;
        UIFormItem *        Item = nullptr;

        int                 Direction  = h_forward;      // 0 = vertical, 1 = horizontal, 2 = vertical reverse, 3 = horizontal reverse

        UIFormLabeledItem( string inLabel, UIFormItem * inItem ) : Label( inLabel ), Item( inItem ), UIFormItem(uif_labeleditem) {};
        ~UIFormLabeledItem();

        void Draw( float inX, float inY, bool clickMute ) override final;
        void UpdateDim( float inW, float inH ) override final;
        vec2 GetAutoDim() override final;

        bool SetFocus( bool inFocus, int inDirection = 1, bool force = false ) override final;
        bool GetFocus() override final;
        int  GetFocusType() override final { return Item->GetFocusType(); };
        bool RevertFocus() override final { return Item->RevertFocus(); };

        ImRect GetFocusedRect( float inX = 0, float inY = 0 ) override final;

        bool EscKeyFocus() override final { return Item->EscKeyFocus(); }
        bool EnterKeyFocus() override final { return Item->EnterKeyFocus(); };
        bool ArrowKeyFocus( int inKey ) override final { return Item->ArrowKeyFocus( inKey ); };
    
        bool GetOverlayState() override final { return Item->GetOverlayState(); };
        vector<ImRect> GetOverlayRects() override final { return Item->GetOverlayRects(); };
        void DrawOverlay( float parentX, float parentY ) override final { Item->DrawOverlay( parentX, parentY); };

        void ResetDraw() override final;

    };

    struct UIFormLabeledText : UIFormLabeledItem {

        UIFormText          Text;

        UIFormLabeledText( string inText, string inLabel ) : Text( inText ), UIFormLabeledItem( inLabel, &Text ) { ItemType = uif_labeledtext; };
        ~UIFormLabeledText() = default;

    };

    struct UIFormLabeledTextInput : UIFormLabeledItem {

        UIFormTextInput     Input;

        UIFormLabeledTextInput( string & inText, string inLabel ) : Input( inText ), UIFormLabeledItem( inLabel, &Input ) { ItemType = uif_labeledtextinput; };
        ~UIFormLabeledTextInput() = default;

    };

    struct UIFormLabeledDateInput : UIFormLabeledItem {

        UIFormDateInput     Input;

        UIFormLabeledDateInput( string & inText, bool inShowTime, string inLabel ) : Input( inText, inShowTime ), UIFormLabeledItem( inLabel, &Input ) { ItemType = uif_labeleddateinput; };
        ~UIFormLabeledDateInput() = default;

    };

    struct UIFormLabeledButton : UIFormLabeledItem {

        UIFormButton        Button;

        UIFormLabeledButton( bool & inClick, string inLabel ) : Button( inClick ), UIFormLabeledItem( inLabel, &Button ) { ItemType = uif_labeledbutton; };
        ~UIFormLabeledButton() = default;

    };

    struct UIFormLabeledTextButton : UIFormLabeledItem {

        UIFormTextButton        Button;

        UIFormLabeledTextButton( bool & inClick, string inText, string inLabel ) : Button( inClick, inText ), UIFormLabeledItem( inLabel, &Button ) { ItemType = uif_labeledtextbutton; };
        ~UIFormLabeledTextButton() = default;

    };
    
    struct UIFormLabeledImage : UIFormLabeledItem {

        UIFormImage     Image;

        UIFormLabeledImage( int inTextureId, vec2 inImageDim, vec4 inPixelXYWH, string inLabel ) : 
            Image( inTextureId, inImageDim, inPixelXYWH ), UIFormLabeledItem( inLabel, &Image ) { ItemType = uif_labeledimage; };
        UIFormLabeledImage( string inPath, vec2 inImageDim, vec4 inPixelXYWH, string inLabel ) : 
            Image( inPath, inImageDim, inPixelXYWH ), UIFormLabeledItem( inLabel, &Image ) { ItemType = uif_labeledimage; };
        ~UIFormLabeledImage() = default;

    };

    struct UIFormLabeledImageButton : UIFormLabeledItem {

        UIFormImageButton     Button;

        UIFormLabeledImageButton( bool & inClick, int inTextureId, vec2 inImageDim, vec4 inPixelXYWH, string inLabel ) : 
            Button( inClick, inTextureId, inImageDim, inPixelXYWH ), UIFormLabeledItem( inLabel, &Button ) { ItemType = uif_labeledimagebutton; };
        UIFormLabeledImageButton( bool & inClick, string inPath, vec2 inImageDim, vec4 inPixelXYWH, string inLabel ) : 
            Button( inClick, inPath, inImageDim, inPixelXYWH ), UIFormLabeledItem( inLabel, &Button ) { ItemType = uif_labeledimagebutton; };
        ~UIFormLabeledImageButton() = default;

    };

    struct UIFormLabeledSelImageButton : UIFormLabeledItem {

        UIFormSelImageButton     Button;

        UIFormLabeledSelImageButton( bool & inClick, int inTextureId, vec2 inImageDim, vec4 inPixelXYWH, string inLabel ) : 
            Button( inClick, inTextureId, inImageDim, inPixelXYWH ), UIFormLabeledItem( inLabel, &Button ) { ItemType = uif_labeledselimagebutton; };
        UIFormLabeledSelImageButton( bool & inClick, string inPath, vec2 inImageDim, vec4 inPixelXYWH, string inLabel ) : 
            Button( inClick, inPath, inImageDim, inPixelXYWH ), UIFormLabeledItem( inLabel, &Button ) { ItemType = uif_labeledselimagebutton; };
        ~UIFormLabeledSelImageButton() = default;

    };

    struct UIFormLabeledCollapserButton : UIFormLabeledItem {

        UIFormCollapserButton     Collapser;

        UIFormLabeledCollapserButton( float inB, float inH, bool & inNeedUpdate, string inLabel ) : 
            Collapser( inB, inH, inNeedUpdate ), UIFormLabeledItem( inLabel, &Collapser ) { ItemType = uif_labeledcollapserbutton; };
        ~UIFormLabeledCollapserButton() = default;

        void SetCollapse( bool inCollapse ) override final { Collapse = inCollapse; Collapser.SetCollapse( inCollapse ); };
        
    };

    struct UIFormLabeledComboBox : UIFormLabeledItem {

        UIFormComboBox     Combo;

        UIFormLabeledComboBox( string inName, vector<string> & inItemList, string inLabel ) : 
            Combo( inName, inItemList ), UIFormLabeledItem( inLabel, &Combo ) { ItemType = uif_labeledcombobox; };
        ~UIFormLabeledComboBox() = default;

    };

    struct UIFormLabeledCheckBox : UIFormLabeledItem {

        UIFormCheckBox     Check;

        UIFormLabeledCheckBox( bool & inCheck, float inSize, string inLabel ) : 
            Check( inCheck ), UIFormLabeledItem( inLabel, &Check ) { ItemType = uif_labeledcheckbox;  Check.H = Check.W = (inSize<0) ? "" : to_string( inSize ); };
        ~UIFormLabeledCheckBox() = default;

    };

    struct UIFormLabeledCollapserCheckBox : UIFormLabeledItem {

        UIFormCollapserCheckBox     Check;

        UIFormLabeledCollapserCheckBox( bool & inCheck, bool & inNeedUpdate, float inSize, string inLabel ) : 
            Check( inCheck, inNeedUpdate ), UIFormLabeledItem( inLabel, &Check ) { 
            ItemType = uif_labeledcollapsercheckbox; Check.H = Check.W = (inSize<0) ? "" : to_string( inSize ); 
        };
        ~UIFormLabeledCollapserCheckBox() = default;

        void SetCollapse( bool inCollapse ) override final { Collapse = inCollapse; Check.SetCollapse( inCollapse ); };

    };

    
    // ----- The Form Class -----

    struct UIForm : UIFormItem {

        string                  Name;

        int                     Direction = v_forward;      // 0 = vertical, 1 = horizontal, 2 = vertical reverse, 3 = horizontal reverse
        int                     AlignItems = align_start;   // TO DO

        vec2                    ContentDim = glm::vec2(0); // Total dimensions of items in the form (used to calculate scroll offset)
        bool                    AllowScroll = true;
        float                   ScrollTrackSize = 10;
        bool                    ScrollDragging = false;
        float                   ScrollPerc = 0;
        int                     ScrollState = 0;

        float                   BorderSize = 0;
        int                     BorderColor = 0x000000;
        int                     BkgdColor = -1;
        float                   BkgdAlpha = 1;
        bool                    ClipBorder = true;
        
        vector<UIFormItem*>     Items = {};

        bool                    NeedUpdate = true;
        bool                    MouseHit = false;
        int                     FocusedItem = -1;
        int                     LastFocusedItem = -1;

        vector<ImRect>          OverlayRects = {};
        vector<int>             OverlayItems = {};
        int                     OverlayItem = -1;

        vector<int>             ScrollableItems = {};

        bool                    IsRoot = true;

        UIForm( string inName, int inDir, int inPPGIndex = -1 );
        ~UIForm();

        void Draw( float inX, float inY, bool clickMute ) override final;
        void UpdateDim( float inW, float inH ) override final;
        vec2 UpdateSubsetDim( int inStart, int inEnd, float inAvailable );
        vec2 GetAutoDim() override final;
        void ResetForm( string inName, int inDir, int inPPGIndex = -1 );
        void RequestUpdate() override final;
        void ResetDraw() override final;

        bool SetFocus( bool inFocus, int inDirection = 1, bool force = false ) override final;
        bool SetNextFocus( int inDirection, bool inWrap ) override final;
        bool GetFocus() override final;
        int  GetFocusType() override final;
        bool RevertFocus() override final;
        UIFormItem * GetFocusedItem();
        ImRect GetFocusedRect( float inX = 0, float inY = 0 ) override final;
        bool EscKeyFocus() override final;
        bool EnterKeyFocus() override final;
        bool ProcessKeyDown( int inKey ) override final;

        bool GetOverlayState() override final;
        vector<ImRect> GetOverlayRects() override final;
        void DrawOverlay( float parentX, float parentY ) override final;

        bool OverlayHit();
        void AddOverlayItem( int inIndex, ImRect inRect );
        void DrawOverlayItems( float inX, float inY );

        void ProcessKeyEvents();
        void ScrollToFocused();
        void ResizeScrollAdjust( const vec2 & oldContentDim );
        bool ScrollChange( float inX, float inY, float inChange ) override final;
        void CollapseAll( bool inState );

        void RemoveItem( int inIndex );
        void RemoveItems( int inStartIndex, int inEndIndex );
        
        // Use these to add items to the form. These return a pointer to the item so that
        // you can then set the properties.
        UIForm* AddForm( int inDir = 0 );
        UIForm* InsertForm( int inIndex, int inDir = 0 );

        // Framing
        UIFormSpacer* AddSpacer( string inVal );
        UIFormSpacer* AddSpacer( float inVal );
        UIFormSpacer* InsertSpacer( int inIndex, string inVal );
        UIFormSpacer* InsertSpacer( int inIndex, float inVal );
        UIFormDivider* AddDivider( float inThickness );

        // Basic
        UIFormRectangle* AddRectangle( string inW="", string inH="" );
        UIFormRectangleDynamicFill * AddRectangleDynamicFill( int & inColor, string inW="", string inH="" );
        UIFormCenteredShape* AddCenteredShape( bool & inClick, int inType, float inSize, string inW="", string inH="");
        UIFormGrid* AddGrid( int nRows, int nCols );

        // Text
        UIFormHeader* AddHeader( string inText );
        UIFormText* AddText( string inText );
        UIFormTextAB* AddTextAB( bool & inCondition, string inText );
        UIFormTextInput* AddTextInput( string & inText );
        UIFormLabeledText* AddLabeledText( string inText, string inLabel = "" );
        UIFormLabeledTextInput* AddLabeledTextInput( string & inText, string inLabel = "" );

        // Advanced Inputs
        UIFormDateInput * AddDateInput( string & inText, bool inShowTime = false );
        UIFormLabeledDateInput* AddLabeledDateInput( string & inText, string inLabel = "", bool inShowTime = false );
        UIFormParagraphInput * AddParagraphInput( const string & inText, bool inViewOnly, bool inWordWrap=true );

        // Image
        UIFormImage* AddImage( int inTextureId, vec2 inImageDim, vec4 inPixelXYWH = vec4(0) );
        UIFormImage* AddImage( string inPath, vec2 inImageDim, vec4 inPixelXYWH = vec4(0) );
        UIFormLabeledImage* AddLabeledImage( int inTextureId, vec2 inImageDim, vec4 inPixelXYWH = vec4(0), string inLabel = "" );
        UIFormLabeledImage* AddLabeledImage( string inPath, vec2 inImageDim, vec4 inPixelXYWH = vec4(0), string inLabel = "" );

        // Buttons
        UIFormButton* AddButton( bool & inClick );
        UIFormLabeledButton* AddLabeledButton( bool & inClick, string inLabel = "" );
        UIFormImageButton* AddImageButton( bool & inClick, int inTextureId, vec2 inImageDim, vec4 inPixelXYWH = vec4(0) );
        UIFormImageButton* AddImageButton( bool & inClick, string inPath, vec2 inImageDim, vec4 inPixelXYWH = vec4(0) );
        UIFormLabeledImageButton* AddLabeledImageButton( bool & inClick, int inTextureId, vec2 inImageDim, vec4 inPixelXYWH = vec4(0), string inLabel = "" );
        UIFormLabeledImageButton* AddLabeledImageButton( bool & inClick, string inPath, vec2 inImageDim, vec4 inPixelXYWH = vec4(0), string inLabel = "" );
        UIFormLabeledSelImageButton* AddLabeledSelImageButton( bool & inClick, int inTextureId, vec2 inImageDim, vec4 inPixelXYWH = vec4(0), string inLabel = "" );
        UIFormLabeledSelImageButton* AddLabeledSelImageButton( bool & inClick, string inPath, vec2 inImageDim, vec4 inPixelXYWH = vec4(0), string inLabel = "" );

        UIFormToggleButtons* AddToggleButtons( int & inButtonIndex, bool & inClick, vector<string> & inItemList );

        UIFormSelImageButton* AddSelImageButton( bool & inClick, int inTextureId, vec2 inImageDim, vec4 inPixelXYWH = vec4(0) );
        UIFormSelImageButton* AddSelImageButton( bool & inClick, string inPath, vec2 inImageDim, vec4 inPixelXYWH = vec4(0) );
        UIFormTextButton* AddTextButton( bool & inClick, string inText );
        UIFormLabeledTextButton* AddLabeledTextButton( bool & inClick, string inText, string inLabel = "" );
        UIFormCollapserButton* AddCollapserButton( float inB, float inH );
        UIFormCollapserButton* AddCollapserButton( float inB, float inH, bool & inNeedUpdate );
        UIFormLabeledCollapserButton* AddLabeledCollapserButton( float inB, float inH, string inLabel = "" );
        UIFormLabeledCollapserButton* AddLabeledCollapserButton( float inB, float inH, bool & inNeedUpdate, string inLabel = "" );
        UIFormSelTextButton* AddSelTextButton(  bool & inClick, string inText );
        UIFormButtonRow* AddButtonRow();
        UIFormSelButtonRow* AddSelButtonRow( int & inButtonIndex );
        UIFormSelButtonGrid* AddSelButtonGrid( int & inButtonIndex, int nRows, int nCols );

        // Widgets
        UIFormSlider* AddSlider( float & inValue, bool & inMute, bool & inChange, vec2 inBounds );
        UIFormLabeledSlider* AddLabeledSlider( float & inValue, bool & inMute, bool & inChange,  vec2 inBounds );
        UIFormAnimationTimeline* AddAnimationTimeline( double inStart = 0, double inEnd = 100, double inCurr = 0 );
        UIFormComboBox* AddComboBox( string inName, vector<string> & inItemList );
        UIFormLabeledComboBox* AddLabeledComboBox( string inName, vector<string> & inItemList, string inLabel = "" );
        UIFormCheckBox* AddCheckBox( bool & inCheck, float inSize = -1 );
        UIFormLabeledCheckBox* AddLabeledCheckBox( bool & inCheck, float inSize = -1, string inLabel = "" );
        UIFormCollapserCheckBox* AddCollapserCheckBox( bool & inCheck, bool & inNeedUpdate, float inSize = -1 );
        UIFormLabeledCollapserCheckBox* AddLabeledCollapserCheckBox( bool & inCheck, bool & inNeedUpdate, float inSize = -1, string inLabel = "" );

        // Custom
        int AddCustomItem( UIFormItem* CustomItem );
        UIFormLabeledItem* AddCustomLabeledItem( string inLabel, UIFormItem* CustomItem );

    };

    // --------------------------------------------------------------------- //
    // Input & Display Data Structures                                       //
    // --------------------------------------------------------------------- //

    struct InputStateData {
        
        struct ButtonData {
            bool IsPressed;
            bool Last;
            int Change;
        } Buttons[3];   
        
        struct KeyData {  // simplified keyboard state
            unsigned int CodePoint = 0; // native endian UTF-32
            int Key = 0;
            int Last = 0;
            int Change = 0;         // 0 = no change, 1 = key down, 2 = key up
            bool IsPressed = 0;
            vector<array<int,2>> Buffer;
            bool Reset = false;     // Indicates that the keyboard state should be reset on next update
        }Key;
        
        struct PositionData {
            double X;
            double Y;
            double LastX;
            double LastY;
            struct PositionDeltaData{
                double X;
                double Y;
            } Delta;
        } Position;
        
        string DropPath = ""; // filename of dropped path
        float ScrollY = 0;
        bool Shift = false;
        bool Alt = false;
        bool Ctrl = false;
        bool Supr = false; // Super key (windows key or command key on Mac)
        bool Caps = false;
        bool DoubleClick = false;

        bool Resize = false;  // window is resizing

        // Constructor
        InputStateData() {
            Buttons[0].IsPressed = false;
            Buttons[1].IsPressed = false;
            Buttons[2].IsPressed = false;
            Buttons[0].Last = false;
            Buttons[1].Last = false;
            Buttons[2].Last = false;
            Buttons[0].Change = 0;
            Buttons[1].Change = 0;
            Buttons[2].Change = 0;
            Position.X = 0;
            Position.Y = 0;
            Position.LastX = 0;
            Position.LastY = 0;
            Position.Delta.X = 0;
            Position.Delta.Y = 0;
        }
        
    };
    
    struct DisplayStateData {

        struct WindowSizeData {
            int W;
            int H;
        } WindowSize;

        
        struct FramebufferSizeData {
            int W;
            int H;
        } FramebufferSize;

        // Retina Diplays have different window and frame buffer resoution. Need to apply scale.
        // io.DisplaySize is the same as GLFW Framebuffer size
        struct ScaleFactorData {
            float X;
            float Y;
        } ScaleFactor;

        DisplayStateData() {
            WindowSize.W = 0;
            WindowSize.H = 0;
            FramebufferSize.W = 0;
            FramebufferSize.H = 0;
            ScaleFactor.X = 1.0f;
            ScaleFactor.Y = 1.0f;
        }

    };

    // --------------------------------------------------------------------- //
    // Context and IO structs                                                //
    //                                                                       //
    // Plan is to replace ImGuiIO with this struct...                        //
    // --------------------------------------------------------------------- //

    struct UILibIO {
        vector<string>          ImagePaths = {};
        int                     NumberOfResidentTextures = 0;
    };

    struct UILibContext {
        ImGuiContext*           ImGuiCTX;
        UILibIO                 IO;

        // GLFW-related
        UICursor                                Cursor;             // Current cursor
        vector<UICursor>                        CursorList;         // List of custom cursors

        // KeyBoard, State & Event Data
        int                                     LastKey = 0;        // last key press for typing
        std::chrono::high_resolution_clock::time_point LastKeyTime; // time of last key press.  Used to limit the nb character per second when holding down a key 
        int                                     HoldDown = 0;       // Is a key held down.  When the key is held down it repeat updates faster than a single key press

        std::chrono::high_resolution_clock::time_point LastBlinkTime; // time of last carat blink

        int                                     InputBoxMode = 0;   // 0 = none, 1 = typing in Input Box, Other numbers are for custom modes i.e, chart input
        ImRect                                  InputBoxRect = {};  // global screen cords of current input box
        string                                  InputText = "";     // text currently being entered in input box
        int                                     InputStartI = 0;    // Start index of text selection
        int                                     InputEndI = -1;     // End index of text selection
        int                                     InputDragging = 0;  // is the input box dragging a selection
        int                                     InputChange = 0;    // did the text change since the last draw - used to keep a solid caret (not blinking) on text entry
        bool                                    InputTextFocus = false; // Is the input box caret active
        int                                     InputTextColor = 0x000000;
        int                                     InputTextFont = 0;
        float                                   InputTextScale = 1;
        string *                                InputHandle = nullptr;  // Handle to the string object using 
        string                                  InputID = "";       // string identifier for the input
        std::function<bool()> *                 InputEnterFocus = nullptr;
        std::function<bool(bool)> *             InputExitFocus = nullptr;
        std::function<bool(unsigned int&)> *    InputValidateKey = nullptr;
        

        // Editor / Syntax Options
        string                  SchemeName;
        vector<UISyntaxInfo>    SyntaxOptions;
        vector<UIEditorInfo>    EditorOptions;

        // Font Index Defaults
        int                     DefaultFont = 0;
        int                     DefaultFontB = 0;
        int                     DefaultFontI = 0;
        int                     DefaultFontBI = 0;
        float                   DefaultFontS = 1;

        // Scroller Defaults
        UIFormatScroller        DefaultScroller;
        float                   DefaultScrollPixels = 10; // Default number of pixels to shift per unit of scroll
        float                   DefaultScrollUnits = 3;   // Default number of pixels to shift per mouse wheel scroll

        // UI Objects
        UIContextMenu           ContextMenu;
        UIStatusMessage         StatusMessage;
        UIStatusMessage         StatusMessage_;
        vector<string>          StatusMessageHistory;

        vector<UIComboBox>      ComboBoxList;
        int                     ComboMode = 0;       // 0 = closed, 1 = open combo list, 2 = open combo list in UIForm
        string                  ComboName = "";
        ImRect                  ComboRect = {};      // global screen cords of current combo list (below combo box top)
        vector<string>          ComboItems = {};     // list of combo items
        UIComboBox *            ComboHandle = nullptr; // handle to currently popped up combo box
        float                   ComboScrollPerc = 0;
        bool                    ComboDragging = false;
    };

    // --------------------------------------------------------------------- //
    // Extern pointers                                                       //
    // --------------------------------------------------------------------- //

    #ifndef UILibCTX
        extern UILibContext* UILibCTX;    // Current implicit InputStateData pointer
    #endif

    #ifndef UILibIS
        extern InputStateData* UILibIS;     // Current implicit InputStateData pointer
    #endif

    #ifndef UILibDS
        extern DisplayStateData* UILibDS;   // Current implicit DisplayState pointer
    #endif

    #ifndef UILibWindowHandle
        extern void* UILibWindowHandle;     // Current window handle pointer
    #endif


    // --------------------------------------------------------------------- //
    // Declarations                                                          //
    // --------------------------------------------------------------------- //
    

    // UILib Context and IO functions
    UILibContext*           UICreateContext();
    void                    UIDestroyContext(UILibContext* ctx = NULL);
    void                    UISetCurrentContext(UILibContext* ctx);
    UILibContext*           UIGetCurrentContext();
    void                    UIRender( bool clearFrame = true );
    void                    UIClearFrame(); // Clears frame variables 
    


    // UILib core event functions
    void                    UISetInputStatePointer( InputStateData* inMS );
    InputStateData*         UIGetInputState();
    void                    UIInputUpdate();    
    
    void                    UISetDisplayStatePointer( DisplayStateData* inDS );
    DisplayStateData*       UIGetDisplayState();
    vec2                    UIGetWindowSize();
    vec2                    UIGetFramebufSize();

    void                    UISetWindowHandlePointer( void* inWin );
    void*                   UIGetWindowHandle();
    void                    UISetWindowTitle( string inTitle );

    void                    UISetDefaultFonts( int inFont, int inFontB, int inFontI, int inFontBI );
    void                    UISetDefaultScroller( UIFormatScroller inScroller );

    void                    UIAddCustomCursor( string inName, string inPath, int inHotX, int inHotY );
    void                    UIRemoveCustomCursor( string inName );
    UICursor *              UIGetCursor( string inName );

    // UI Library TO DO / PLANNING:
    // We need to have a generalized system for window pane shifting (currently loosely called Global vs Local ).
    // In reality, some functions adds a XY shift based on the current active ImGui window pane. Sometimes, this is called GetGlobal__.
    // The applied shift moves from Window XYWH space to APP Global XYWH. An issue is if you call a global function twice it will double add this shift.

    // Design Concept:
    // All UI Library high level draw functions should be written with NO SHIFT.
    // Have a set of "SHIFTER" functions that apply the shift on a XYWH or ImRect or Point.
    // The feature is that you can easily see where pane to app shifting is occuring and don't double shift.
    //      UIShiftToGlobal( float inX, float inY, float inW, float inH )
    //      UIShiftToGlobal( ImRect inRect )
    //      UIShiftToGlobal( float inX, float inY )
    //      UIShiftToGlobal( vec2 inVec )
    void UIAddTriPrim( vec2 A, vec2 B, vec2 C, int clA, int clB, int clC, float aA, float aB, float aC );
    void UIAddTriPrim( vec2 A, vec2 B, vec2 C, int cl,float a);
    void UITriangulate( const vector<vec2> & inPnts, vector<vec2> & outPnts );
    void UITriangulate( const vector<highp_dvec2> & inPnts, vector<highp_dvec2> & outPnts, double inSclFactor, double inMarginError);

    // Primitive/Low Level Drawing Functions

    void                    UIAddQuadPrim( vec2 A, vec2 B, vec2 C, vec2 D, int clA, int clB, int clC, int clD, float alphaA, float alphaB, float alphaC, float alphaD );

    // Core Drawing Functions
    
    void                    UIAddTriangle(float inX, float inY, float inRadius, bool inOpen, unsigned int inC);
    void                    UIAddFlipTriangle(float inX, float inY, float inRadius, bool inOpen, unsigned int inC);
    void                    UIAddTriangleGlobal(float inX, float inY, float inRadius, bool inOpen, unsigned int inC);
    void                    UIAddTriangleGlobalAlpha(vec2 inCenter, float inB, float inH, char inDir, int inC, float inAlpha );
    void                    UIAddTriangleGlobal(vec2 inCenter, float inB, float inH, char inDir, int inC );

    void                    UIAddFilledPolyline(vector<vec2> inPnts, unsigned int inC );
    void                    UIAddFilledAlphaPoly(vector<vec2> inPnts, unsigned int inC , float inAlpha);
    void                    UIAddFilledAlphaPolyG(vector<vec2> inPnts, unsigned int inC , float inAlpha);
    void                    UIAddPolyline(vector<vec2> inPnts, unsigned int inC, float inThickness, bool inClosed );
    void                    UIAddPolylineG(vector<vec2> inPnts, unsigned int inC, float inThickness, bool inClosed );
    void                    UIAddRoundCornerRect(float inX, float inY, float inW, float inH, unsigned int inC, bool inFill, float inThickness, float inRounding, std::string cornerBits);
    void                    UIAddCircle(float inX, float inY, float inRadius, unsigned int inC, bool inFill, float inThickness, int nbSegments);
    void                    UIAddGlobalCircle(float inX, float inY, float inRadius, unsigned int inC, bool inFill, float inThickness, int nbSegments);
    void                    UIAddAlphaCircle(float inX, float inY, float inRadius, unsigned int inC, bool inFill, float inAlpha, float inThickness, int nbSegments);
    void                    UIAddGlobalCircleAlpha(float inX, float inY, float inRadius, unsigned int inC, bool inFill, float inAlpha, float inThickness, int nbSegments);
    void                    UIDrawArcAlpha(vec2 inCenter, float inRadius, double startAng, double endAng, unsigned int inC, bool inFill, float inAlpha = 1, float inThickness = 0, int nbSegments = 720 );
    void                    UIDrawGradientArcAlpha(vec2 inCenter, float inRadius, double startAng, double endAng, const vector<int> & inClrs, int inDir, float inAlpha = 1, int nbSegments = 360, int nbRDivs = 10 );
    void                    UIDrawGradientArcAlpha(vec2 inCenter, float inRadius, double startAng, double endAng, std::function<int(float)> clrFunc, int inDir, float inAlpha = 1, int nbSegments = 360, int nbRDivs = 10 );

    void                    UIDrawRadialGradientArcAlpha(vec2 inCenter, float inRadius, double startAng, double endAng, const vector<int> & inClrs, const vector<float> & inAlphas, int nbSegments = 360, int nbRDivs = 10 );
    
    void                    UIAddRect(float inX, float inY, float inW, float inH, unsigned int inC, bool inFill, float inThickness);
    void                    UIAddRectAlpha(float inX, float inY, float inW, float inH, unsigned int inC, bool inFill, float inThickness, float inAlpha);
    void                    UIAddRoundRect(float inX, float inY, float inW, float inH, unsigned int inC, bool inFill, float inThickness, float inRounding);
    void                    UIAddRoundRect(float inX, float inY, float inW, float inH, unsigned int inC, bool inFill, float inThickness, float inRounding, ImDrawCornerFlags corners);
    void                    UIAddAlphaRoundRect(float inX, float inY, float inW, float inH, unsigned int inC, float inAlpha, bool inFill, float inThickness, float inRounding);
    void                    UIAddAlphaRoundRect(float inX, float inY, float inW, float inH, unsigned int inC, float inAlpha, bool inFill, float inThickness, float inRounding, ImDrawCornerFlags corners);
    void                    UIAddGlobalAlphaRoundRect(float inX, float inY, float inW, float inH, unsigned int inC, float inAlpha, bool inFill, float inThickness, float inRounding, ImDrawCornerFlags corners);
    void                    UIAddLine(float inX1, float inY1, float inX2, float inY2, unsigned int inC1, float inThickness);
    void                    UIAddLineAlpha(float inX1, float inY1, float inX2, float inY2, unsigned int inC1, float inThickness, float inA);
    void                    UIDashedLine(vec2 &inP1, vec2 &inP2, float inDashLen, float inGapLen, float inOffset, unsigned int inC1, float inThickness);
    void                    UIAddGlobalLine(float inX1, float inY1, float inX2, float inY2, unsigned int inC1, float inThickness);
    void                    UIAddGlobalLineAngled(float inX1, float inY1, float inX2, float inY2, unsigned int inC1, float inThickness, vec2 inRPoint, float rad);
    void                    UIAddGlobalLineAlpha(float inX1, float inY1, float inX2, float inY2, unsigned int inC1, float inThickness, float inA);
    void                    UIAddGlobalLineAngledAlpha(float inX1, float inY1, float inX2, float inY2, unsigned int inC1, float inThickness, vec2 inRPoint, float rad, float inA);
    void                    UIAddGradientRect(float inX, float inY, float inW, float inH, unsigned int inC1, unsigned int inC2, bool inVertical);
    void                    UIAddGradientFade(float inX, float inY, float inW, float inH, unsigned int inC, bool inVertical);
    void                    UIAddBorderRect(float inX, float inY, float inW, float inH, unsigned int inFillC, unsigned int inBorderC, float inThickness);
    void                    UIPlaceRect(float inX, float inY, ImRect inRect, int inXType, int inYType, unsigned int inC);
    void                    UIDrawBB(ImRect inRect, unsigned int inC);
    void                    UIDrawAlphaBB(ImRect inRect, unsigned int inC, float inAlpha);
    void                    UIDrawAlphaBBWithCutout( ImRect inRect, unsigned int inC, float inAlpha, ImRect cutOutRect );
    void                    UIDrawAlphaBBRotated(ImRect inRect, unsigned int inC, float inAlpha, vec2 inRPoint, float inAng );
    void                    UIDrawMultiColorBB(ImRect inRect, std::vector<int> inC, std::vector<float> inAlpha); //{tl,tr,bl,br}
    
    void                    UIDrawBBLocalOutline(ImRect inRect, unsigned int inC, float inThickness);
    void                    UIDrawBBOutline(ImRect inRect, unsigned int inC, float inThickness);
    void                    UIDrawAlphaBBOutline(ImRect inRect, unsigned int inC, float inThickness, float inAlpha);
    void                    UIDashRect(ImRect inRect, unsigned int inC, float inThickness);
    void                    UIDashRectAni(ImRect inRect, unsigned int inC, float inThickness, float inDash, float inGap);

    void                    UIDrawRoundBB(ImRect inRect, unsigned int inC, float inRounding);
    void                    UIDrawAlphaRoundBB(ImRect inRect, unsigned int inC, float inAlpha, float inRounding);
    void                    UIDrawAlphaRoundBBRotated(ImRect inRect, unsigned int inC, float inAlpha, float inRounding, vec2 inRPoint, float rad);
    void                    UIDrawAlphaRoundCornerBB(ImRect inRect, unsigned int inC, float inAlpha, float inRounding, std::string cornerBits);
    void                    UIDrawRoundBBOutline(ImRect inRect, unsigned int inC, float inRounding, float inThickness);
    void                    UIDrawRoundBBOutlineRotated(ImRect inRect, unsigned int inC, float inRounding, float inThickness, vec2 inRPoint, float rad);
    void                    UIDrawRoundBBOutlineCorner(ImRect inRect, unsigned int inC, float inRounding, float inThickness, std::string cornerBits);
    void                    UIAddArrowLine(float inX1, float inY1, float inX2, float inY2, unsigned int inC1, float inThickness, float inW_Arrow, float inH_Arrow);

    void                    UIAddChevronLeft( const ImRect & inRect, unsigned int inC, float inAlpha, float inThickness);
    void                    UIAddChevronRight( const ImRect & inRect, unsigned int inC, float inAlpha, float inThickness);
    
    // ------------------------------------------ Inputs (UILibInput.cpp) ------------------------------------------
    
    string                  UIGetKeyInput(string inStr, int  &inInputIndex, int &inEndIndex, int &inChange, bool &inFocused);
    void                    UISelectBetweenDelim(string inStr, int &inInputIndex, int &inEndIndex);
    int                     UIGetKeyDirection();
    string                  UIGetClipboard();
    const char *            UIGetClipboardChars();
    void                    UISetClipboard(const string & inStr);
    int                     UIClipboardSize(bool isTextMap);
        
    bool                    UIInputTextInitFocus( string * inHandle, ImRect inRect, int inFont = 0, int inFontColor = 0, int inMode = 1, string inIdentifier = "", 
                                                    std::function<bool()> * inEnterFunc = nullptr, 
                                                    std::function<bool(bool)> * inExitFunc = nullptr,
                                                    std::function<bool(unsigned int&)> * inValidateFunc = nullptr );
    bool                    UIInputTextExitFocus( bool override = false );
    string *                UIInputTextGetFocusHandle();

    // ------------------------------------------ Text and Characters (UILibText.cpp) ------------------------------------------
    
    void                    UISetFont(int inIndex);
    void                    UIRenderChar(ImDrawList* draw_list, float size, ImVec2 pos, ImU32 col, ImWchar c);
    void                    UIRenderChar(ImDrawList* draw_list, float size, ImVec2 pos, ImU32 col, int inC, ImWchar c );
    void                    UIRenderChar(ImDrawList* draw_list, float size, ImVec2 pos, int inCI, ImWchar c, ImFont* font);
    void                    UIRenderChar( ImWchar c, ImVec2 pos, ImU32 col, ImFont * font );


    void                    UIAddTextFloat(float inX, float inY, float inW, float inH, float *inVal, vec2 inAlign);
    void                    UIAddTextString(float inX, float inY, float inW, float inH, string inText, int inC, vec2 inAlign);
    void                    UIAddGlobalText(float inX, float inY, float inW, float inH, string inText, int inC, vec2 inAlign);
    
    void                    UIAddTextWithFont(float inX, float inY, float inW, float inH, const string& inText, int inC, int inFontIndex, vec2 inAlign);
    void                    UIAddTextWithFont(float inX, float inY, float inW, float inH, const string& inText, int inC, int inFontIndex, vec2 inAlign, float inScale);

    void                    UIAddTextBB(ImRect inRect, const string& inText, int inC, vec2 inAlign);
    void                    UIAddTextBBAngle(ImRect inR, const string& inText, int inC, vec2 inAlign, vec2 inRPoint, float rad);//inRPoint is the rotation point, inAng is radians
    void                    UIAddTextWithFontBB(const ImRect& inRect, const string& inText, int inC, int inFontIndex, vec2 inAlign);
    void                    UIAddTextWithFontBB(const ImRect& inRect, const string& inText, int inC, int inFontIndex, vec2 inAlign, float inScale);
    void                    UIAddTextWithFontBBAngle(const ImRect& inR, const string& inText, int inC, int inFontIndex, vec2 inAlign, float inScale, vec2 inRPoint, float inAng); //inRPoint is the rotation point, inAng is radians
    void                    UIAddTextUnderline(const ImRect& inRect, const string& inText, int inC, int inFontIndex, vec2 inAlign, float inSize);
    void                    UIAddTextUnderline(const ImRect& inRect, const string& inText, int inC, int inFontIndex, vec2 inAlign, float inSize, float inScale);
    void                    UIAddTextUnderlineAngled( const ImRect& inRect, const string& inText, int inC, int inFontIndex, vec2 inAlign, float inSize, float inScale, vec2 inRPoint, float inAng );//inRPoint is the rotation point, inAng is radians
    
    void                    UIAddAlphaTextBB(ImRect inRect, const string& inText, int inC, float inAlpha, vec2 inAlign);
    void                    UIAddAlphaTextBBAngle(ImRect inR, const string& inText, int inC, float inAlpha, vec2 inAlign, vec2 inRPoint, float rad);//inRPoint is the rotation point, inAng is radians
    void                    UIAddAlphaTextWithFontBB(const ImRect& inRect, const string& inText, int inC, float inAlpha, int inFontIndex, vec2 inAlign);
    void                    UIAddAlphaTextWithFontBB(const ImRect& inRect, const string& inText, int inC, float inAlpha, int inFontIndex, vec2 inAlign, float inScale);
    void                    UIAddAlphaTextWithFontBBAngle(const ImRect& inR, const string& inText, int inC, float inAlpha, int inFontIndex, vec2 inAlign, float inScale, vec2 inRPoint, float inAng); //inRPoint is the rotation point, inAng is radians
    void                    UIAddAlphaTextUnderline(const ImRect& inRect, const string& inText, int inC, float inAlpha, int inFontIndex, vec2 inAlign, float inSize);
    void                    UIAddAlphaTextUnderline(const ImRect& inRect, const string& inText, int inC, float inAlpha, int inFontIndex, vec2 inAlign, float inSize, float inScale);
    void                    UIAddAlphaTextUnderlineAngled( const ImRect& inRect, const string& inText, int inC,float inAlpha,  int inFontIndex, vec2 inAlign, float inSize, float inScale, vec2 inRPoint, float inAng );//inRPoint is the rotation point, inAng is radians
    
    void                    UITextAligned(float inX, float inY, float inW, float inH, string inText, int inC, int inFontIndex, vec2 inAlign, bool inWordWrap, bool inULine, float inScale);
    void                    UITextAlignedG(float inX, float inY, float inW, float inH, string inText, int inC, int inFontIndex, vec2 inAlign, bool inWordWrap, bool inULine, float inScale);
    
    void                    UITextAlignedClr(float inX, float inY, float inW, float inH, string inText, vector<int> &ColorList, int inFontI, vec2 inAlign, bool inWordWrap, bool inULine, float inScale);
    void                    UITextAlignedClrG(float inX, float inY, float inW, float inH, string inText, vector<int> &ColorList, int inFontI, vec2 inAlign, bool inWordWrap, bool inULine, float inScale);
    
    void                    UIInputText(float inX, float inY, float inW, float inH, string inText, int inC, int inFontIndex, vec2 inAlign, float inYShift, int &inStartI, int &inEndI, int &inDragging, bool &inFocused, int &inChange, float inScale);
    void                    UIDrawTextInput();
    void                    UIDrawPasswordInput();
    
    void                    UIMultiLineWrapText(float inX, float inY, string inText, float inW);
    void                    UIMultiLineTextBB( const ImRect & inRect, const string & inText, int inC, float inAlpha, int inFontIndex, vec2 inAlign, bool inULine, float inScale );
    void                    UIMultiLineTextBBAngled( const ImRect & inRect, const string & inText, int inC, float inAlpha, int inFontIndex, vec2 inAlign, bool inULine, float inScale, vec2 inRPoint, float inAng );//inRPoint is the rotation point, inAng is radians
    void                    RecalibrateSel(TextMap &inTM);
    
    // -------------------------------------------- Editor / Syntax Settings ----------------------------------------------
    void                    UISetScheme( string inScheme = "Default" );

    // ---------------------------------------- Mult-Line Text Input: TextMap (TM) ----------------------------------------
    TextMap                 UIBuildLinTM(ImRect inR, string &inText, array<int,4> inFont, int inLanguage, vec2 inBorder, TextFormat inTF, float inScale);
    TextMap                 UIBuildWinTM(ImRect inR, string &inText, array<int,4> inFont, int inLanguage, vec2 inBorder, TextFormat inTF, float inScale);
    void                    UIDrawTM(TextMap &inTMap);
    void                    UIDrawEditorTM(TextMap &inTM);
    void                    UIDrawFormatTM(TextMap &inTM);
    void                    UIDrawSyntaxColorTM(TextMap &inTM);
    
    void                    UIDrawCaret(TextMap &inTM);
    void                    UIDrawSelection(TextMap &inTM);

    void                    UIMouseInputTM(TextMap &inTM);
    void                    UIKeyInputTM_ReadOnly(TextMap &inTM, int inNlType);
    void                    UIKeyInputTM(TextMap &inTMap, string &inText, int inNlType, string &inInsertS, string &inKeyOp);
    void                    UIKeyInputSS(TextMap &inTM, string &inText, int inNlType, string &inInsertS);
    string                  UITextFromTM(TextMap &inTMap, string &inInsertS, string &inText, int inNlType);
    
    void                    ParsePythonSyntax(vector<CharCell> &CL, string &inText, int inNlType);
    void                    CastKT(int &inCast, vector<int> &token, vector<CharCell> &CL, vector<string> &inKTList, string &inText);
    
    pair<int,int>           GetTextSelRange(TextMap &inTM, int inSIndex, int inEIndex);
    list<int>               GetSelectedLineStarts(TextMap &inTM);
    string                  GetPythonIndent(int inCharIndex, TextMap &inTM, int &inInsertI);
    void                    HighlightBrackets(TextMap &inTM);
    void                    UIHighlightOccurrences(TextMap &inTM);
    void                    UITermOccurrences(TextMap &inTM, string inTerm);
    string                  ConvertToWinNL(string inStr,int &inNbLines);
    string                  UIGetWordAtPosition(TextMap &inTM, vec2 inPos, int &outStart, int &outEnd, int&outYLine);
    string                  UIGetWordAtCaret(TextMap &inTM, int &outStart, int &outEnd, const vector<int> &inDlist);

    // ------------------------------------------ Text Metrics (UILibText.cpp) ------------------------------------------
    int                     UIGetNewlineType(string &inText);
    float                   UIGetParagraphHeight(const string & inText, float inW);
    int                     UIGetTextNbLines(string inText);
    float                   UIGetTextWidth(const string & inText, int inFontIndex);
    float                   UIGetTextWidth(const string & inText, int inFontIndex, float inScale);
    float                   UIGetTextHeight(const string & inText, int inFontIndex);
    float                   UIGetTextHeight(const string & inText, int inFontIndex, float inScale);
    vec2                    UIGetTextDim(const string & inText, int inFontIndex);
    vec2                    UIGetTextDim(const string & inText, int inFontIndex, float inScale);
    float                   UIGetFontHeight(int inFontIndex);
    float                   UIGetFontHeight(int inFontIndex, float inScale);
    void                    UIConformNewlines(string &inText, int inNlType);
    void                    LogText(string inText);
    void                    UIDebugTextMap(TextMap &inTMap);

    // Dynamic Widgets - Change Base On Event State
    void                    UIAddSliderH(float inX, float inY, float inW, float inH, float* inVal, float inMin, float inMax, bool &inMuteUI, bool &inChange);
    void                    UIAddSliderV(float inX, float inY, float inW, float inH, float* inVal, float inMin, float inMax, bool &inMuteUI, bool &inChange);

    void                    UIAddTimelineSlider(float inX, float inY, float inW, float inH, bool &inMuteUI, UIDataTimeline & inData );

    void                    UIAddCalendarWidget( const ImRect & inRect, UIDataCalendar & inData, bool Mute );

    void                    UIAddTabs(float inX, float inY, float inHeight, vector<string> inStrings, int* inIndex, float inPad, int inFontIndex);
    int                     UIToggleButtons(float inX, float inY, float inHeight, int inColor, float inRound, vector<string> inStrings, int* inIndex, float inPad, int inFontIndex);

    void                    UIAddScrollerH(float inX, float inY, float inW, float inH, float inContentW, float* inVal, bool* inDragging, const UIFormatScroller & inFormat, int* const inState = {}, bool inMute = false );
    void                    UIAddScrollerV(float inX, float inY, float inW, float inH, float inContentH, float* inVal, bool* inDragging, const UIFormatScroller & inFormat, int* const inState = {}, bool inMute = false );
    void                    UIAddScrollerH(float inX, float inY, float inW, float inH, float inContentW, double* inVal, bool* inDragging, const UIFormatScroller & inFormat, int* const inState = {}, bool inMute = false );
    void                    UIAddScrollerV(float inX, float inY, float inW, float inH, float inContentH, double* inVal, bool* inDragging, const UIFormatScroller & inFormat, int* const inState = {}, bool inMute = false );

    void                    UIColorSVPicker( float inX, float inY, float inW, float inH, float inHue, float &inSat, float &inVal, bool &inDragging, bool &inMuteUI, int &inChange);
    void                    UIColorHuePicker( float inX, float inY, float inW,  float inH, float &inHue, bool &inDragging, bool &inMuteUI, int &inChange);
    void                    UIColorHuePickerV( float inX, float inY, float inW,  float inH, float &inHue, bool &inDragging, bool &inMuteUI, int &inChange);
    void                    UICheckBox(float inX, float inY, float inWH, int &inChecked, int inClr = 0x228BC5 );

    // BB Manipulator Functions
    // These functions just update the ImRect and do not draw.
    // They are also all in Global space.
    void                    UITransformBB( ImRect & inRect, UITransformBBData & inTD, const ImRect & boundingRect, bool inMute, bool MuteSize = false, bool MuteDrag = false );

    // Buttons
    void                    UIAddButton(ImRect inRect, string inText, int inC, int inOverC, int inTextC, float inRounding, bool &inButtonDown, const bool & inMute = false);
    bool                    UIAddButtonRect(ImRect inR, string inText, vector<int> inColors, bool &inMuteUI);
    bool                    UIAddButtonRectClipped(ImRect inR, string inText, vector<int> inColors, bool &inMuteUI);
    bool                    UIAddButtonRectJustified(ImRect inR, string inText, vector<int> inColors, bool &inMuteUI, vec2 inAlign);
    bool                    UIAddButtonRoundRect(ImRect inR, string inText, vector<int> inColors, float inRounding, float inThickness, bool &inMuteUI);
    bool                    UIAddButtonRoundRectF(ImRect inR, string inText, vector<int> inColors, float inRounding, float inThickness, bool &inMuteUI, int inFontI);
    int                     UIAddCornerButton(const ImRect & inR, const string & inText, const vector<int> & inColors, float inRounding, float inThickness, bool &inMuteUI, int inFontI, const std::string & cornerBits);
    bool                    UIAddShadeButton(ImRect inRect, vector<int> inColors, vector<float> inShadeFactors, float inThickness, float inRounding  );
    bool                    UIAddCornerShadeButton(ImRect inRect, vector<int> inColors, vector<float> inShadeFactors, float inThickness, float inRounding, std::string cornerBits );

        
    // High Level UI Components
    void                    UICheckList(ImRect inRect, vector<string> &inTextList, vector<int> &inStateList, float &inScrollPerc, bool &inDragging);
    void                    UICheckListByCallback( ImRect inRect, std::function<string (int64_t)> inStrCallback, vector<int> &inStateList, float &inScrollPerc, bool &inDragging );
    
    void                    UIDrawComboBox( UIComboBox * inCombo, ImRect inRect, vector<string> &inItems );
    void                    UIDrawComboItems( UIComboBox * inCombo, ImRect inRect, vector<string> &inItems, float &inScrollPerc, bool &inDragging);
    UIComboBox *            UIAddComboBox( const string & inComboName, int inIndex, int inPPGIndex = -1 );
    UIComboBox *            UIAddComboBox( const string & inComboName, ImRect inRect, vector<string> &inItems, int inIndex = 0, int inPPGIndex = -1 );
    UIComboBox *            UIGetComboBox( const string & inComboName );
    void                    UIRemoveComboBox( const string & inComboName );
    void                    UIRemovePPGComboBoxes( int inPPGIndex );
    void                    UIUpdateComboBoxPPGIndex( vector<int> inNewIds );

    void                    UIComboInitFocus( int inMode, UIComboBox * inHandle, ImRect inRect = {}, vector<string> inList = {} );
    void                    UIComboExitFocus();
    UIComboBox *            UIComboGetFocusHandle();
    string                  UIComboGetFocusedName();


    // Loading UI
    void                    UILoadingIndicatorCircle( ImRect inRect, int inClr,  int inCount, float inPct, int nbSegments);
    void                    UILoadBar( const ImRect & inR, float inPerc, const string & inMessage, int inFontI, vector<int> inClr = { 0x000000, 0x286FFF, 0xD1DDEF}  );

    // Event State
    bool                    GetOverState(const ImRect& inBB);
    bool                    GetActiveState(bool inOver, ImGuiID inId);
    bool                    MouseDown();
    bool                    AnyMouseDown();
    bool                    MiddleMouseDown();
    bool                    RightMouseDown();
    bool                    DragStart();
    bool                    DoubleClicked();
    bool                    MiddleDragStart();
    bool                    RightDragStart();
    bool                    DragStartLR();
    bool                    MouseRelease();
    bool                    MouseDownOrRelease( int i = 0 );
    vec2                    GetMousePos();
    highp_dvec2             GetMousePosD();
    vec2                    GetMouseDelta();
    vec2                    GetLocalMousePos();
    vec2                    GetGlobalMousePos();
    int                     GetKeyDown();
    bool                    KeyIsPrintable( int key );
    float                   GetMouseScroll();
    bool                    AnyScroll();
    int                     GetUndoInput();
    
    // Drop Shadow
    void                    UIDropShadow(ImRect inBox, float inBlurDist, float inAlpha, int inRes1, int inRes2, unsigned int inC, float inXO, float inYO);
    void                    UIDropShadow(ImRect inBox, float inBlurDist, float inAlpha, int inRes, unsigned int inC, float inXO, float inYO);
    void                    UICircleGlobal(ImVec2 inP, float inRadius, unsigned int inC, bool inFill, float inThickness, int nbSegments);
    void                    UIAddGlobalPolyline(const vector<vec2> & inPnts, unsigned int inC, float inThickness, bool inClosed );
    void                    UIAddGlobalPolyline(const vector<highp_dvec2> & inPnts, unsigned int inC, float inThickness, bool inClosed );
    void                    UIAddGlobalPolylineAlpha( const vector<vec2> & inPnts, unsigned int inC, float inThickness, float inAlpha, bool inClosed );
    void                    UIGlobalFilledPLine(vector<vec2> inPnts, unsigned int inC );
    void                    UIGlobalFilledPLineA(vector<vec2> inPnts, unsigned int inC, float inA);

    // Polylines
    void                    UIDrawMiterPolyline( const vector<vec2>& inPnts, int inC, float inA, float inThickness, double angThresh = 0.261799 );

    // Images/Textures
    void                    UIImage(ImRect inBox, int inTextureId, vec2 uv0, vec2 uv1);
    void                    UIImage(ImRect inBox, int inTextureId, ImRect inUvBox);

    int                     UIAddImage(ImRect inBox, string inPath, ImRect inUvBox);
    int                     UIGetImageIndex( string inPath );
    void                    GetPngSize(const string & inPath, int & inW, int & inH);
    
    ImRect                  GetSprite(float inX, float inY, float inW, float inH);
    ImRect                  GetUV(ImRect fSprite, vec2 ImageDim);
    ImRect                  RectFromAspect(int XorY, ImRect inAspectRect, ImRect inBaseRect );
    void                    UISprite(int inTextureId, vec2 inPos, vector<float> inPixelXYWH, vec2 inImageDim, float inScale);

    // UI Objects
    void                    UISetContextMenu( string inName, const vector<string> & inOptions );
    UIContextMenu *         UIGetContextMenu();

    void                    UIResetStatusMessage();
    void                    UISetStatusMessage( string inM );
    void                    UISetFormattedStatusMessage( string inM, int inFCol, int inBCol, int inFInd );
    void                    UISetTimedStatusMessage( string inM, double inDur, string nextM="" );
    void                    UISetFormattedTimedStatusMessage( string inM, int inFCol, int inBCol, int inFInd, double inDur, string nextM="" );
    void                    UISetFormattedTimedStatusMessage( string inM, int inFCol, int inBCol, int inFInd, double inDur, string nextM, int nextFCol, int nextBCol, int nextFInd );
    UIStatusMessage *       UIGetStatusMessage();
    void                    UIAddToStatusMessageHistory( string inM );
    vector<string> *        UIGetStatusMessageHistory();

    
    // Helper Functions
    ImU32                   Cl(unsigned int hexValue);
    ImU32                   ClearCl(unsigned int hexValue);
    ImU32                   ClWithAlpha(unsigned int hexValue, float inAlpha);
    int                     UIGetBlendClr( const vector<int> & inClrs, float inPct );
    float                   UIGetBlendVal( const vector<float> & inVals, float inPct );

    ImRect                  UIGetUniformFitBB( const ImRect & inContainer, float inW, float inH );    // given aspect ratio get the BB that fits within content area
    ImRect                  FitBoxinBB(ImRect inBox, ImRect inContainer);
    void                    FitBBinBB( ImRect & inBB, const ImRect & boundingBB );
    ImRect                  RescaleToFitBox(const ImRect & inBox, const ImRect & inContainer );
    ImRect                  RescaleToFitBox(const ImRect & inBox, const ImRect & inContainer, float & outScl );
    bool                    BBIsSame( const ImRect & inA, const ImRect & inB );

    void                    UISetCursor(int inCType, string inCName = "");
    ImVec2                  Pos(float x, float y, ImVec2 winPos);
    float                   ClipFloat(float n, float lower, float upper);
    double                  ClipDouble(double n, double lower, double upper);
    ImRect                  GetGlobalBB(float inX, float inY, float inW, float inH);
    ImRect                  GetBB(float inX, float inY, float inW, float inH);
    ImRect                  GetBoundingBB(const ImRect & inR, vec2 inRPoint, float rad);
    ImRect                  GetArcBoundingBB( vec2 inCenter, float inRadius, double startAng, double endAng );
    ImRect                  BBToGlobal( const ImRect & inRect );
    ImRect                  BBToLocal( const ImRect & inRect );
    void                    CollateBB( ImRect & inRect );
    glm::vec2               VecToGlobal( const vec2 & inVec );
    glm::vec2               VecToLocal( const vec2 & inVec );
    ImRect                  GetGlobalFromP(float inX1, float inY1, float inX2, float inY2);
    ImRect                  GetCenteredBB(float inX, float inY, float inW, float inH);
    float                   YToGlobal(float inY);    
    ImRect                  VecBB(const vec2& inA, const vec2& inB);
    bool                    VecInBB(const vec2& inVec, const ImRect& inBB);
    bool                    VecInBBClip(const vec2& inVec, const ImRect& inBB);
    bool                    VecIsClipped(const vec2& inVec);
    bool                    VecInTriangle(const vec2& s, const vec2& a, const vec2& b, const vec2& c);
    bool                    VecInQuad(const vec2& s, const vec2& a, const vec2& b, const vec2& c, const vec2& d);
    bool                    VecInLine(const vec2& s, const vec2& a, const vec2& b, float inThickness);

    // Vector Math Helpers
    glm::vec2               UIRotate(const vec2& v, double radians );
    glm::vec2               UIRotate(const vec2& v, float cos_a, float sin_a);
    glm::vec2               UIRotate(const vec2& v, vec2 inRPoint, double radians );
    float                   UIGetVectorAngle( const vec2& v1, const vec2& v2 );
    double                  UIGetAngle( vec2 v );
    vec2                    UIGetNormal( vec2 v);
    double                  UICrossProduct(vec2 a, vec2 b);
    bool                    UILineSegIntersect( vec2 inP0, vec2 inP1, vec2 inP2, vec2 inP3, vec2 & pOut );
    
    // Logging Helper Functions
    void                    LogBB(ImRect inBB, bool inNL);
    void                    LogPos(string inTxt, vec2 inPos);
    string                  FloatToString( float inNum, int inPrecision = 2 );
    string                  VecToString( vec2 inVec );
    string                  VecToString( vec3 inVec );
    string                  VecToString( vec4 inVec );
    string                  RectToString( ImRect inRect );
    
    template<typename T> string VectorToString( const vector<T> & inVec, int inPrecision = 2 ) {
        std::ostringstream out; 
        out.precision(inPrecision);
        out << std::fixed;
        out << "( ";
        string dlm = "";
        for ( const auto & val : inVec ) {
            out << dlm << val;
            dlm = ", ";
        }
        out << " )";
        return out.str();
    };


    // Color

    int                     RGBtoInt(float inR, float inG, float inB);
    int                     RGBtoInt(vec3 inRGB);
    int                     RGBtoInt(vec4 inRGBA);
    void                    IntToHSV(unsigned int hexValue, float &inH, float &inS, float &inV);
    void                    IntToVec4(unsigned int color, vec4 &v);
    int                     HVStoInt(float &inH, float &inS, float &inV);
    string                  IntToHexColor(unsigned int hexValue);
    void                    HexToHSV(const string & inHexStr, float &inH, float &inS, float &inV);
    int                     HexToInt(const string & inHexStr);
    vec3                    HexToRGBVec(int hexValue);
    ColorSpace::Lab         ClToLab(unsigned int hexValue);
    int                     LabtoInt(ColorSpace::Lab inLab);

} // end namespace UILib

#endif //  UILIB_H