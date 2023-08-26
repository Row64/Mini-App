#ifndef BLOCK_PPG_COLORS_CPP
#define BLOCK_PPG_COLORS_CPP

// #include "gh_Primitives.h"
#include "c_PipelineManager.h"
using namespace AppCore;


namespace Blocks {

    string FormatStringToHex( string inStr ) {

        string hexStr = inStr;
        transform(hexStr.begin(), hexStr.end(), hexStr.begin(), ::toupper);

        // Remove 0x or # prefix
        if ( hexStr.find( "0X" ) == 0 ) { hexStr.erase(0,2); }
        else if ( hexStr.find( "#" ) == 0 ) { hexStr.erase(0,1); }

        // Fill in 0s at beginning or truncate to get 6 digits
        size_t nChar = hexStr.length();
        if ( nChar < 6 ) { hexStr.insert( nChar, 6-nChar, '0' ); } else if ( nChar > 6 ) { hexStr.resize(6); }
        
        // Check if digits are all valid hex
        bool isValid = true;
        for (char const &c: hexStr) { if ( !isxdigit(c) ) { isValid = false; break; } }
        
        return (isValid) ? hexStr : "INVALID";

    }

    // ============================================================================================ //
    //                                       SELECT COLOR PPG                                       //
    // ============================================================================================ //

    void InitSelectColorPPG(PropertyPage &inPPG) {
        
        int ftReg = BaseFont(), dftReg = GetCodeFont("");

        unsigned int * currentColor = reinterpret_cast< unsigned int*>( inPPG.DataPtr[0] );

        // If you need to modify the ChangeFunction, then SelectColorPPGData should be created when the PPG is made,
        // and pushed back after the current color pointer;
        // See Example: TButtonBar::OpenColorPPG()
        if ( inPPG.DataPtr.size() < 2 ) { inPPG.DataPtr.push_back( (void *) new SelectColorPPGData( *currentColor ) ); }
        SelectColorPPGData * sd = (SelectColorPPGData *) inPPG.DataPtr.back();

        UIForm & form = inPPG.MakeNewForm( v_reverse );     // Draw from bottom to top so that changes in HsvSelector propogate to the other form elements

        form.AddCustomItem( &sd->HsvSelector ); sd->HsvSelector.Expand = 1; sd->HsvSelector.MarginLeft = sd->HsvSelector.MarginRight = sd->HsvSelector.MarginBottom = "1";

        auto sf = form.AddForm( h_forward ); sf->H = "20";

        sf->AddSpacer(7);
        
        auto t1 = sf->AddText( "Color:" ); t1->Alignment = {0,0.5}; t1->Format.Font = ftReg;
        sd->ColorSquare = sf->AddRectangleDynamicFill( *(int*)currentColor, "16", "16" ); sd->ColorSquare->MarginLeft = "3"; sd->ColorSquare->MarginTop = sd->ColorSquare->MarginBottom = "2"; 
        sd->ColorSquare->BorderColor = 0xD3D2D1; sd->ColorSquare->BorderSize = 1;

        sf->AddSpacer(20);
        auto t2 = sf->AddLabeledTextInput( sd->ColorHex, "Code:" ); 
        t2->Label.Alignment = {0,0.5f}; t2->Label.Format.Font = ftReg;
        t2->Input.W = "55"; t2->Input.Alignment = {0,0.5f};
        t2->Input.MarginLeft = "3"; t2->Input.MarginTop = t2->Input.MarginBottom = "1";  
        t2->Input.Default.Font = ftReg; t2->Input.Default.BkgdColor = 0xFFFFFF; t2->Input.Default.BorderColor = 0xABABAB; 
        t2->Input.Focus.Font = dftReg;  t2->Input.Focus.BorderColor = 0x2BA9E0;
        sd->ColorInput = &t2->Input;     // save handle to text input
        sd->LastFocus = t2->Input.Focused;

        auto exps = sf->AddSpacer(""); exps->Expand = 1;
        
        auto b2 = sf->AddImageButton( sd->Picker, ADIO->ButtonBarIndex, ADIO->ButtonBarDim, {94, 37, 40, 40} ); b2->Image.Scale = 0.5f; b2->ImageOrder = 1;
        b2->MarginLeft="5"; b2->MarginRight = "5"; b2->MarginTop = b2->MarginBottom = "1"; 
        b2->Default.BorderSize = 0; b2->Default.BkgdColor = 0xFFFFFF;
        b2->Over.BkgdColor = 0xEDEDED; b2->Down.BkgdColor = 0xD9D9D9; 
        

    }

    void DrawSelectColorPPG(PropertyPage &inPPG, float inW, float inH) {

        float barH = 20;

        unsigned int * currentColor = reinterpret_cast< unsigned int*>( inPPG.DataPtr[0] );
        SelectColorPPGData * sd = (SelectColorPPGData *) inPPG.DataPtr.back();

        inPPG.Form->UpdateDim( inPPG.Dim.x, inPPG.Dim.y-barH);
        inPPG.Form->Draw( inPPG.Pos.x, inPPG.Pos.y+barH, inPPG.Mute );

        if ( inPPG.Form->GetFocus() ) { 
            ADIO->FM.SetFocus( ft_ppg_input,  "SelectColorPPG" );
        } else { 
            ADIO->FM.ClearFocus( ft_ppg_input,  "SelectColorPPG" );
        }

        // Update colors when text input is NOT FOCUSED
        if ( !(sd->ColorInput->Focused) ) {
            if ( sd->LastFocus ) {              // Update colors when EXITING FOCUS
                string hexStr = FormatStringToHex( sd->ColorHex );
                if ( hexStr == "INVALID" ) {    // Invalid hex reverts back to prior color
                    *currentColor = HexToInt(sd->LastHex);
                    sd->ColorHex = sd->LastHex;
                    transform(sd->ColorHex.begin(), sd->ColorHex.end(), sd->ColorHex.begin(), ::toupper);
                }
                else {                          // Valid hex updates color
                    *currentColor = HexToInt(hexStr);
                    sd->ColorHex = hexStr;
                    sd->HasChange = 2;
                }
            }
            else {
                sd->LastHex = sd->ColorHex;
                if ( sd->HsvSelector.DraggingHue || sd->HsvSelector.DraggingSV ) { sd->ColorHex = IntToHexColor( *currentColor ); sd->HasChange = 1; }   // mouse down dragging
                else if ( sd->HsvSelector.ChangeHue == 2 || sd->HsvSelector.ChangeSV == 2 ) { sd->HasChange = 3; }   // don't change color on mouse up, but pass to change function
            }
        }
        sd->LastFocus = sd->ColorInput->Focused;
        
        if ( sd->Picker ) {
            std::cout << "Clicked Picker Button" << inPPG.Name + inPPG.Name2 << std::endl;
            // You want to pass the top-most PPG instead of the current PPG, because if the current PPG is not on the top,
            // the first click will move it to the top. But the PPG reorder happens at the end of frame. Therefore,
            // the ColorPicker may have an outdated reference.
            ADIO->FM.SetFocus( ft_cursor_color_picker, "ColorPicker" );
            ADIO->ColorPickerSource = "PPG";
            ADIO->ColorPickerData = &ADIO->OpenPPGs.back();
        }

        if ( sd->HasChange > 0 ) {
            sd->ChangeFunction( sd->HasChange );     // Call change function if there is a change in color
            sd->HasChange = 0;
        }
        else {
            sd->NoChangeFunction();
        }
        

    }

    void ExitSelectColorPPG(PropertyPage &inPPG) {
        inPPG.DeletePPGForm();
        delete reinterpret_cast<SelectColorPPGData*>(inPPG.DataPtr.back());            // Delete the SelectColorPPGData
        inPPG.DataPtr.pop_back();
    }

    // ============================================================================================ //
    //                                       COLOR THEME PPGS                                       //
    // ============================================================================================ //

    struct ColorThemePPGData {

        ColorTemplateUI     ColorUI;
        string SmoothName = "Color Smooth";
        bool SmoothMute = false;
        bool SmoothChange = false;
        UIFormLabeledSlider* SmoothPtr = nullptr;

        vector<int>         ColorHighlight;

        bool IsFlip = false;

        ColorThemePPGData() {
            ColorUI.ItemType = uif_custom_external;
            PMPTR->SendAppMessage( "TUI", "GetColorTheme", {} );
            ColorUI.ChipColors = ADIO->CurrentTheme;
            ColorUI.ColorSmooth = ADIO->CurrentThemeSmooth;
            ColorHighlight = ADIO->CurrentHighlight;
        }
        ~ColorThemePPGData() = default;
    };

    void OpenColorThemePPG() {
        string ppgName = "Color Theme";
        int ppgId = GetPpgIndex( ppgName );
        if ( ppgId > -1 ) {
            ADIO->MM.MovePPGToTop(ppgId);    // signals to move this PPG to the top
            return;
        }

        vec2 mPos = GetGlobalMousePos();
         
        float ppx = mPos.x + 10;
        float ppy = mPos.y + 5;
        if(ppy < 61){ppy = 61;}
        
        float ppw = 270;
        float pph = 175;

        PropertyPage & newPPG = ADIO->MM.AddPPG( ppgName, true, {ppx,ppy}, {ppw,pph} );
        newPPG.MinDim = {200, 100};
        newPPG.InitFunction = InitColorThemePPG; 
        newPPG.ExitFunction = ExitColorThemePPG;
        newPPG.DrawFunction = DrawColorThemePPG;
        newPPG.UpdateFunction = UpdateColorThemePPG;

    }

    void InitColorThemePPG(PropertyPage &inPPG) {

        float esp = 5;
        string margin = "15";
        int sftBold = GetGeneralFont("tab");

        // Setup
        inPPG.DataPtr.push_back( (void *) new ColorThemePPGData() );
        ColorThemePPGData * cd = (ColorThemePPGData *) inPPG.DataPtr.back();

        UIForm & form = inPPG.MakeNewForm( v_forward );
        form.PadLeft = "3%"; form.PadRight = "3%"; form.PadTop = "3%"; form.PadBottom = "3%";
        form.AllowScroll = false;

        // Color Palette
        cd->ColorUI.TextButtons = { "Presets", "Flip", "Blend" }; cd->ColorUI.Expand = 1; cd->ColorUI.Spacer="5"; 
        form.AddCustomItem( &cd->ColorUI );
        form.AddSpacer( esp );

        // Color Smooth Slider
        auto st = form.AddText( cd->SmoothName ); st->Format.Font = sftBold; st->Format.FontColor = 0x7AAECE;
        auto sl = form.AddLabeledSlider( cd->ColorUI.ColorSmooth, cd->SmoothMute, cd->SmoothChange, {0,1} ); sl->ValueLabel.Alignment = {1, 0.5};
        cd->SmoothPtr = sl;
    }

    void DrawColorThemePPG(PropertyPage &inPPG, float inW, float inH) {

        float barH = 20;
        vec2 mPos = GetGlobalMousePos();

        ColorThemePPGData * cd = (ColorThemePPGData *) inPPG.DataPtr.back();

        inPPG.Form->UpdateDim( inPPG.Dim.x, inPPG.Dim.y-barH);
        inPPG.Form->Draw( inPPG.Pos.x, inPPG.Pos.y+barH, inPPG.Mute );

        bool HasChange = false;
        bool HighlightChange = false;

        // Color theme buttons
        for ( int i = 0; i < 4; i++ ) {
            if ( cd->ColorUI.ClickList[i] ) { 
                PropertyPage * subppg = OpenSubPPG(inPPG, "Select Color", {270,180}, i, inW, inH, {-1, mPos.y-180*0.5} );
                subppg->DataPtr.push_back(&cd->ColorUI.ChipColors[i]);
                subppg->InitFunction = InitSelectColorPPG; 
                subppg->ExitFunction = ExitSelectColorPPG;
                subppg->DrawFunction = DrawSelectColorPPG;
                subppg->MinDim = {270,180};
                subppg->Name2 = ": Color Theme Chip " + to_string(i);
                cd->ColorUI.ClickList[i] = false;
                break;
            } 
        }

        if( cd->ColorUI.ClickedText == "Presets" ) {
            OpenThemePresetsPPG( inPPG, "ColorThemePPG", ": Color Theme", cd->ColorUI.ChipColors, cd->ColorHighlight, {-1, mPos.y-162*0.5} );
        }
        else if( cd->ColorUI.ClickedText == "Flip" ) {
            cd->IsFlip = !cd->IsFlip;
            vector<int> colors = cd->ColorUI.ChipColors;
            int endI = (int) cd->ColorUI.ChipColors.size()-1;
            for ( int i = 0; i <= endI; i++ ) {
                cd->ColorUI.ChipColors[i] = colors[endI-i];
            }
        }
        else if( cd->ColorUI.ClickedText == "Blend" ) {
            AutoBlendColorTheme( cd->ColorUI.ChipColors );
            HasChange = true;
            ADIO->CurrentTheme = cd->ColorUI.ChipColors;
        }

        if ( !HasChange ) {
            if ( ADIO->CurrentHighlight != cd->ColorHighlight ) {
                HasChange = true; HighlightChange = true;
                ADIO->CurrentHighlight = cd->ColorHighlight;
            }
            if ( ADIO->CurrentTheme != cd->ColorUI.ChipColors ) {
                HasChange = true;
                ADIO->CurrentTheme = cd->ColorUI.ChipColors;
            }
            else if ( cd->SmoothChange ) {
                HasChange = true;
                ADIO->CurrentThemeSmooth = cd->ColorUI.ColorSmooth;
            }
            else if ( cd->SmoothPtr->ValueClick ) { 
                UIFormLabeledSlider * ls = cd->SmoothPtr;
                string name2 = ": " + cd->SmoothName;
                OpenEditSliderPPG( name2, inPPG.Index, 0, inW, inH, &ls->Slider.Value, &ls->Slider.Bounds.x, &ls->Slider.Bounds.y, &cd->SmoothChange );
            }
        }

        if ( HasChange ) {
            PMPTR->SendAppMessage( "TUI", "UpdateColorTheme", {"ColorThemePPG", HighlightChange ? "1" : "0"} );     // Chart & Geo PPG color theme updates sent by TUI_ChartSheet as "UpdateChart"
        }

    }

    void ExitColorThemePPG(PropertyPage &inPPG) {
        inPPG.DeletePPGForm();
        delete reinterpret_cast<ColorThemePPGData*>(inPPG.DataPtr.back());            // Delete the ColorThemePPGData after the form
        inPPG.DataPtr.pop_back();

    }

    int UpdateColorThemePPG(PropertyPage &inPPG, string inType, vector<string> inM) {
        if ( inM.size() < 1 ) { return 0; }
        if (inM[0] == "ColorThemePPG" ) { return 0; }

        if ( inType == "Close" ) {
            for ( int i : inPPG.Children ) { ADIO->MM.RemovePPG( i ); }
            return 1;
        }
        
        if ( inType == "UpdateColorTheme" ) {
            ColorThemePPGData * cd = (ColorThemePPGData *) inPPG.DataPtr.back();
            cd->ColorUI.ChipColors = ADIO->CurrentTheme;
            cd->ColorUI.ColorSmooth = ADIO->CurrentThemeSmooth;
            cd->ColorHighlight = ADIO->CurrentHighlight;
            return 1;
        }
        return 0;
    }

    // ============================================================================================ //
    //                                 ThemeSelectionUI Functions                                   //
    // ============================================================================================ //

    ThemeSelectionUI::ThemeSelectionUI( const std::initializer_list<int> inClrs ) : 
        UIFormItem(uif_custom_external), Colors(inClrs) { 
            MinDim = { 150, 16 };
            Colors.resize(6, -1);       // 4 theme colors, 2 highlights
            Expand = 1;
        };

    void ThemeSelectionUI::Draw( float inX, float inY, bool clickMute ) {
        if ( Dim.x == 0 ) { return; }
        if ( Dim.y == 0 ) { return; }

        if ( Colors.size() < 6 ) { Colors.resize(6, -1); }

        vec2 mPos = GetGlobalMousePos();
        ImRect rect = GetContentRect( inX, inY );

        Click = false;

        float hsp = 7.0f;      // highlight spacer
        float esp = 2.0f;      // space between chips
        float chipW = ( rect.GetWidth() - hsp - esp*4 ) / Colors.size();
        float chipH = rect.GetHeight();

        ImRect chipBB = GetBB( rect.Min.x, rect.Min.y, chipW, chipH );
        for ( int i = 0; i < Colors.size(); i++ ) {
            if ( Colors[i] == 0xFFFFFF ) { UIDrawBBOutline(chipBB, 0xB5B6B7, 1); }
            else if ( Colors[i] >= 0 ) { UIDrawBB(chipBB, Colors[i] ); }
            chipBB.TranslateX( ( i == 3 ) ? chipW + hsp : chipW + esp );            
        }

        if(VecInBBClip(mPos, rect)){
            rect.Expand(2);
            UIDrawBBOutline(rect, 0xBCBCBC, 2);
            if(DragStart() && !clickMute ) { Click = true; }
        }        
    

      
    }

    vec2 ThemeSelectionUI::GetAutoDim() {
        return MinDim;
    }

    void ThemeSelectionUI::ResetDraw() {
        Click = false;
    };

    // ============================================================================================ //
    //                                      THEME PRESETS PPG                                       //
    // ============================================================================================ //
    
    struct ThemePresetsPPGData {

        vector<string> TabList = { "Basic", "Biz", "Pop", "Data" };
        int CurrentTab = 0;
        int LastTab = 0;
        bool IsTabSwitch = false;

        vector<ThemeSelectionUI> Themes = {};

        UIFormGrid * Grid = nullptr;

        uvec2 GetGridSize() {
            uvec2 RowCol = { 0, 2 };   // { # rows, # cols }
            RowCol.x = (int) Themes.size() / RowCol.y; 
            if ( RowCol.x * RowCol.y < (glm::u32) Themes.size() ) { RowCol.x += 1; }
            return RowCol;
        };

        void SyncTheme() {
            string Tab = TabList[CurrentTab];
            if ( Tab == "Biz" ) {
                Themes = {
                    ThemeSelectionUI( { 0x0370C2,0x013661,0xA8C3DF,0xA6A6A6,    -1      ,-1         } ),    ThemeSelectionUI( { 0xF07069,0xF7B573,0xA5D2E6,0x5DA3C6,    -1      ,-1         } ),
                    ThemeSelectionUI( { 0x25535B,0x7DD9D7,0x5B9EA8,0x98DEBA,    -1      ,-1         } ),    ThemeSelectionUI( { 0xF6984F,0xD6D6D6,0x99C5E6,0x3A92D3,    -1      ,-1         } ),
                    ThemeSelectionUI( { 0x31397D,0x0383A8,0x02A38B,0x7FBC6F,    -1      ,-1         } ),    ThemeSelectionUI( { 0x0F1E37,0x8C786E,0xF5554B,0x194B5A,    -1      ,-1         } ),
                    ThemeSelectionUI( { 0x0C8566,0x7DBE66,0xB6DA66,0xEDF666,    -1      ,-1         } ),    ThemeSelectionUI( { 0x005DA0,0xE52A44,0x96ABA5,0x504C51,    -1      ,-1         } ),
                    ThemeSelectionUI( { 0x0E672D,0xBFC8BA,0xF17060,0x9C0824,    -1      ,-1         } ),    ThemeSelectionUI( { 0xA6192E,0xEB9B7B,0x8BBDD2,0x1F5EA6,    -1      ,-1         } ),
                    ThemeSelectionUI( { 0x900D25,0xF4AA88,0xC8C8C8,0x343434,    -1      ,-1         } ),    ThemeSelectionUI( { 0xFFCDB9,0x6EB4BE,0x006E82,0xE1E6E6,    -1      ,-1         } ),
                    ThemeSelectionUI( { 0x282D32,0xF54B4B,0xF0D2BE,0x5F504B,    -1      ,-1         } ),    ThemeSelectionUI( { 0xEC6925,0xF07F3D,0xED9F71,0xDEDEDE,    0xFAE3D1,-1         } ),
                    ThemeSelectionUI( { 0x23303C,0x565658,0x969698,0xE0E1E3,    0x2242E5,0x37A5F4   } ),    ThemeSelectionUI( { 0x181616,0x5D5C5C,0xB8B3B3,0xDEDEDE,    0xCA0A07,-1         } ),
                    ThemeSelectionUI( { 0x141414,0x0C202F,0x96989A,0xDEDEDE,    0x16ABF4,-1         } ),    ThemeSelectionUI( { 0x071D2C,0x1A628D,0x37A5F4,0xDEDEDE,    0x3D43E3,0xE59DCC   } ),
                    ThemeSelectionUI( { 0x313030,0xF58A4B,0xF9B87A,0xDEDEDE,    0x5CA5B6,-1         } ),    ThemeSelectionUI( { 0x313030,0x5CA5B6,0x37A5F4,0xDEDEDE,    0xF58A4B,0xF9B87A   } ),
                    ThemeSelectionUI( { 0x2D2D2D,0xBDBEC1,0xD7D8D8,0xDEDEDE,    0x33B881,0x77CA5B   } ),    ThemeSelectionUI( { 0x565658,0x969698,0xBDBEC1,0xDEDEDE,    0xF14E41,-1         } ),
                    ThemeSelectionUI( { 0x4B4B4D,0x969698,0xAEAEAF,0xDEDEDE,    0xF9D217,0xC7B256   } ),    ThemeSelectionUI( { 0x3A4B69,0x1A628D,0x5ECEDB,0xDEDEDE,    0xBEC82E,0x299066   } ),
                    ThemeSelectionUI( { 0x999DA0,0xB4B9BC,0xDBDDE1,0xDEDEDE,    0x2171DF,0x73A5E9   } ),    ThemeSelectionUI( { 0x828C88,0xBDBEC1,0xDEDFDD,0xDEDEDE,    0x27805E,0x93CA72   } ),
                    ThemeSelectionUI( { 0x0A090A,0x787777,0xCCCCCC,0xDEDEDE,    0x55701E,0x9ECF48   } ),    ThemeSelectionUI( { 0x323132,0x4F4F4F,0xAEAEAF,0xB5B6AF,    0xB1DA96,0x75AF4A   } ),
                    ThemeSelectionUI( { 0x8D35E3,0x96969D,0xDBDDE1,0xDEDEDE,    0xE3A31E,0x0041F0   } ),    ThemeSelectionUI( { 0x8D35E3,0x96969D,0xDBDDE1,0xDEDEDE,    0x55701E,0x9ECF48   } ),
                    ThemeSelectionUI( { 0x0A0C0D,0x013FA7,0x0BABE7,0xCDECF6,    0xF8E412,-1         } ),    ThemeSelectionUI( { 0x0842A6,0x44A7E2,0xCDECF6,0x0BABE7,    0xF2B73F,0x29B658   } ),
                    ThemeSelectionUI( { 0x141819,0x494E51,0xD1D4D2,0xDEDEDE,    0x8AEBFA,0x4CB0EC   } ),    ThemeSelectionUI( { 0x0C0C0E,0x3F589E,0x4592CC,0xDEDEDE,    0x4EAAA3,0x743FBF   } ),
                    ThemeSelectionUI( { 0x2DBB76,0x4CB195,0xB3D3C7,0xDEDEDE,    0x1B7B57,-1         } ),    ThemeSelectionUI( { 0x3483BA,0x1FADB2,0x969698,0xDEDEDE,    0x3CAE63,-1         } ) 
                };
            }
            else if ( Tab == "Pop" ) {
                Themes = {
                    ThemeSelectionUI( { 0x4372B0,0x9CC4DA,0xF8DC8D,0xD02E26,    -1      ,-1         } ),    ThemeSelectionUI( { 0xFF388C,0x9C007B,0x0059D6,0x00359C,    -1      ,-1         } ),
                    ThemeSelectionUI( { 0x4A8D20,0x9CD066,0xF5DBE8,0xBF1A7B,    -1      ,-1         } ),    ThemeSelectionUI( { 0x7D75D7,0xD975CC,0xB6E7B6,0x77D3CD,    -1      ,-1         } ),
                    ThemeSelectionUI( { 0xEF1A30,0xFAC3CA,0x71DCE2,0x0196AA,    -1      ,-1         } ),    ThemeSelectionUI( { 0x03FCFF,0x58A7FF,0xA758FF,0xF906FF,    -1      ,-1         } ),
                    ThemeSelectionUI( { 0xCFEEAC,0x5EAEAA,0x396A7E,0xDF5474,    -1      ,-1         } ),    ThemeSelectionUI( { 0x22FFCE,0x54B6DE,0xC663F7,0x856BDE,    -1      ,-1         } ),
                    ThemeSelectionUI( { 0x3BBFA7,0x014035,0xF2B035,0xF37126,    -1      ,-1         } ),    ThemeSelectionUI( { 0x9F86CC,0xE290E4,0xB4D782,0x3C8BAA,    -1      ,-1         } ),
                    ThemeSelectionUI( { 0x88500A,0xEEE1BC,0xC2E5DF,0x01635B,    -1      ,-1         } ),    ThemeSelectionUI( { 0x0000B2,0x0068FF,0x31FFC5,0xBDFF39,    -1      ,-1         } ),
                    ThemeSelectionUI( { 0x522683,0x9489BD,0xF8DBB3,0xAE5405,    -1      ,-1         } ),    ThemeSelectionUI( { 0x7A07FE,0x1E8FF3,0x3CE9D5,0x96FCA7,    -1      ,-1         } ),
                    ThemeSelectionUI( { 0x9E0421,0xEF233C,0xDCDEDE,0x7C7580,    -1      ,-1         } ),    ThemeSelectionUI( { 0x0888EE,0x01CC66,0xA1EC86,0xFDFE98,    -1      ,-1         } ),
                    ThemeSelectionUI( { 0x00293C,0x1E656D,0xF1F3CE,0xF62A00,    -1      ,-1         } ),    ThemeSelectionUI( { 0xA3D2D6,0x80BBC3,0xB4A7C4,0x927B94,    -1      ,-1         } ),
                    ThemeSelectionUI( { 0xAF4425,0x662E1C,0xC9A66B,0xEBDCB2,    -1      ,-1         } ),    ThemeSelectionUI( { 0xF4CDCB,0xECB2AC,0xF0A881,0xE28F6D,    -1      ,-1         } ),
                    ThemeSelectionUI( { 0x231942,0x9F86C0,0xE0B1CB,0xF1DAC4,    -1      ,-1         } ),    ThemeSelectionUI( { 0xF7EB9C,0xE6C988,0xB3C7AF,0x3F6C67,    -1      ,-1         } ),
                    ThemeSelectionUI( { 0xFF6262,0x6D80EB,0x4592CC,0xA5EAB4,    -1      ,-1         } ),    ThemeSelectionUI( { 0xD92C13,0xFBB486,0xF9E9A0,0xF7BA00,    -1      ,-1         } ),
                    ThemeSelectionUI( { 0xEF1A30,0x517FFF,0x0073FF,0x0196AA,    -1      ,-1         } ),    ThemeSelectionUI( { 0xF7EAA7,0xF6AF4D,0xE0211D,0xA70024,    -1      ,-1         } ),
                    ThemeSelectionUI( { 0x4372B0,0xBCFF6D,0xFCEF75,0xD02E26,    -1      ,-1         } ),    ThemeSelectionUI( { 0xEF233C,0xF55D33,0xFB8625,0xFFA901,    -1      ,-1         } )
                };
            }
            else if ( Tab == "Data" ) {
                Themes = {
                    ThemeSelectionUI( { 0x9BDCD7,0x3AA07D,0x62DF93,0x0076BE,    -1      ,-1         } ),    ThemeSelectionUI( { 0xED192F,0x3BBEA7,0xC2E5DF,0x0076BE     -1      ,-1         } ),
                    ThemeSelectionUI( { 0x48029F,0xAD2692,0xF38649,0xF7E125,    -1      ,-1         } ),    ThemeSelectionUI( { 0xD64815,0x9C2B15,0x948684,0x845D5A,    -1      ,-1         } ),
                    ThemeSelectionUI( { 0x180F3F,0x762181,0xF9795C,0xFCECAE,    -1      ,-1         } ),    ThemeSelectionUI( { 0x737DA5,0x9CBACE,0xDEB077,0x8C726B,    -1      ,-1         } ),
                    ThemeSelectionUI( { 0x1C4F7A,0x388A69,0xABB35B,0xD4B093,    -1      ,-1         } ),    ThemeSelectionUI( { 0x8092AD,0xCDDAE3,0xC3B495,0x9E8262,    -1      ,-1         } ),
                    ThemeSelectionUI( { 0x007B03,0x00084F,0x006D9E,0x96CADC,    -1      ,-1         } ),    ThemeSelectionUI( { 0x91AFC3,0x326987,0x283232,0x783728,    -1      ,-1         } ),
                    ThemeSelectionUI( { 0x6CC1BE,0xB6E7B6,0xD0EDF6,0x7ED8D7,    -1      ,-1         } ),    ThemeSelectionUI( { 0xA59691,0x736E78,0xF0D7C8,0xEB9687,    -1      ,-1         } ),
                    ThemeSelectionUI( { 0x2EAFF5,0x3384B8,0x50CD69,0xB3E39E,    -1      ,-1         } ),    ThemeSelectionUI( { 0xF69035,0xD9D5C9,0x77ACD3,0x2B5C8A,    -1      ,-1         } ),
                    ThemeSelectionUI( { 0xFF9998,0xD9241E,0x93DF83,0x289F27,    -1      ,-1         } ),    ThemeSelectionUI( { 0x9BCCA9,0xE3E5A1,0xF7BE74,0x953734,    -1      ,-1         } )
                }; 
            }
            else {  // "Basic"
                Themes = {
                    ThemeSelectionUI( { 0x41719C,0x5694CB,0x8DB5DB,0xC4D5EB,    -1      ,-1       } ),      ThemeSelectionUI( { 0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,    -1      ,-1,      } ),
                    ThemeSelectionUI( { 0xBC8C00,0xF3B700,0xF9CC5E,0xFFE2BC,    -1      ,-1       } ),      ThemeSelectionUI( { 0xD8D8D8,0xD8D8D8,0xD8D8D8,0xD8D8D8,    -1      ,-1,      } ),
                    ThemeSelectionUI( { 0xAE5A21,0xE2772E,0xECA176,0xF6CCBE,    -1      ,-1       } ),      ThemeSelectionUI( { 0xC6C6C6,0xC6C6C6,0xC6C6C6,0xC6C6C6,    -1      ,-1,      } ),
                    ThemeSelectionUI( { 0x507E32,0x6BA543,0x9AC082,0xC9DBC1,    -1      ,-1       } ),      ThemeSelectionUI( { 0xB1B1B1,0xB1B1B1,0xB1B1B1,0xB1B1B1,    -1      ,-1,      } ),
                    ThemeSelectionUI( { 0x264C12,0x67A34D,0xB0D0A3,0xDADADA,    -1      ,-1       } ),      ThemeSelectionUI( { 0x9D9D9D,0x9D9D9D,0x9D9D9D,0x9D9D9D,    -1      ,-1,      } ),
                    ThemeSelectionUI( { 0x1B617B,0x2085A0,0x46B1D3,0xAFDEED,    0xDEDEDE,-1       } ),      ThemeSelectionUI( { 0x8C8C8C,0x8C8C8C,0x8C8C8C,0x8C8C8C,    -1      ,-1,      } ),
                    ThemeSelectionUI( { 0x196079,0x1C9F91,0x29DDAB,0xAEFFE2,    -1      ,-1       } ),      ThemeSelectionUI( { 0x787878,0x787878,0x787878,0x787878,    -1      ,-1       } ),
                    ThemeSelectionUI( { 0x003B46,0x07575B,0x66A5AD,0xC4DFE6,    -1      ,-1       } ),      ThemeSelectionUI( { 0x000000,0x000000,0x000000,0x000000,    -1      ,-1       } ),
                    ThemeSelectionUI( { 0x03045E,0x0077B6,0x48CAE4,0xADE8F4,    -1      ,-1       } ),      ThemeSelectionUI( { 0x69CF9C,0x69CF9C,0x69CF9C,0x69CF9C,    -1      ,-1       } ),
                    ThemeSelectionUI( { 0x2A78AE,0x408CC1,0x57A1D3,0x6EB6E6,    -1      ,-1       } ),      ThemeSelectionUI( { 0x2171DF,0x2171DF,0x2171DF,0x2171DF,    -1      ,-1       } ),
                    ThemeSelectionUI( { 0x0080AD,0x3A9DB0,0x56BDB3,0x6CDEB5,    -1      ,-1       } ),      ThemeSelectionUI( { 0xED7D31,0xED7D31,0xED7D31,0xED7D31,    -1      ,-1       } ),
                    ThemeSelectionUI( { 0x061B42,0x225EA8,0x7FCDBB,0xFFFFD9,    -1      ,-1       } ),      ThemeSelectionUI( { 0xEF3227,0xEF3227,0xEF3227,0xEF3227,    -1      ,-1       } ),
                    ThemeSelectionUI( { 0x212529,0x6C757D,0xADB5BD,0xE9ECEF,    -1      ,-1       } ),      ThemeSelectionUI( { 0x0800F7,0x5800A7,0xA60059,0xF0000F,    -1      ,-1       } ),
                    ThemeSelectionUI( { 0x6A040F,0xD00000,0xF48C06,0xFFBA08,    -1      ,-1       } ),      ThemeSelectionUI( { 0xF70700,0xA05F00,0x56A800,0x15E900,    -1      ,-1       } ),
                    ThemeSelectionUI( { 0xEBA6A4,0xE6706C,0xC92A1D,0xA52116,    -1      ,-1       } ),      ThemeSelectionUI( { 0xE190AE,0xC02E57,0x7F1D3A,0x2E0815,    -1      ,-1       } )
                };
            }
        };

        ThemePresetsPPGData() {
            SyncTheme();
        };

    };

    void OpenThemePresetsPPG(PropertyPage &inPPG, const string & inSrc, const string & inName2, vector<int> & inClrs, vector<int> & inHClrs, vec2 inPos, int inRef) {
        auto subppg = OpenSubPPG( inPPG, "Color Presets", {400,390}, inRef, ADIO->Dim.x, ADIO->Dim.y, inPos );
        subppg->Data = inSrc;
        subppg->Name2 = inName2;
        subppg->DataPtr.push_back( &inClrs );
        subppg->DataPtr.push_back(&inHClrs);
        subppg->DataPtr.push_back( (void *) new ThemePresetsPPGData() );    // Make the ThemePresetsPPGData here instead of in the init function
        subppg->InitFunction = InitThemePresetsPPG;
        subppg->ExitFunction = ExitThemePresetsPPG;
        subppg->DrawFunction = DrawThemePresetsPPG;
        subppg->UpdateFunction = UpdateThemePresetsPPG;
    };

    void InitThemePresetsPPG(PropertyPage &inPPG) {

        int ftReg = BaseFont(), ftBold = BaseFontB(), sftBold = GetGeneralFont("tab"), dftReg = GetCodeFont("");

        ThemePresetsPPGData * data = (ThemePresetsPPGData *) inPPG.DataPtr.back();

        UIForm & form = inPPG.MakeNewForm( v_forward );form.AllowScroll = false;
        form.PadTop = "0"; form.PadBottom = "10"; 
        
        // Tab Switcher
        auto sf = form.AddForm(h_forward); sf->BkgdColor = 0xB7B7B7;
        
        auto br = sf->AddSelButtonRow( data->CurrentTab ); 
        br->H = "20"; br->MarginBottom = "0"; br->MarginTop = "4";
        for ( int i = 0; i < data->TabList.size(); i++ ) {
            auto b = br->AddSelTextButton( data->TabList[i] );
            b->CornerBits = "0011";
            b->W = "75"; b->H = "100%"; b->Rounding = 5;
            b->Default.BorderSize=0; b->Default.BkgdAlpha=0; b->Default.Font=sftBold; b->Default.FontColor=0xFFFFFF;  
            b->Select.BkgdColor=0xFFFFFF; b->Select.BkgdAlpha=1; b->Select.FontColor=0x656565;
            b->PadLeft = "4";
        }
        
        // Color Chips Grid
        auto gf = form.AddForm( v_forward ); gf->AllowScroll = true; gf->Expand = 1;
        gf->PadLeft = gf->PadRight = gf->PadTop = gf->PadBottom = "6";
        
        auto gridDim = data->GetGridSize();
        data->Grid = gf->AddGrid( gridDim.x , gridDim.y ); data->Grid->Expand = 1;
        data->Grid->SpacerSize = { 24.0f, 2.0f };
        for (int i = 0; i < data->Themes.size(); i++ ) {
            data->Grid->AddItem( &data->Themes[i] );
        }
        data->Grid->MinDim = data->Grid->GetAutoDim();
    }

    void DrawThemePresetsPPG(PropertyPage &inPPG, float inW, float inH) {

        float barH = 20;

        ThemePresetsPPGData * data = (ThemePresetsPPGData *) inPPG.DataPtr.back();

        // Draw the PPG
        inPPG.Form->UpdateDim( inPPG.Dim.x, inPPG.Dim.y-barH);
        inPPG.Form->Draw( inPPG.Pos.x, inPPG.Pos.y+barH, inPPG.Mute );

        if ( inPPG.Form->GetFocus() ) { 
            ADIO->FM.SetFocus( ft_ppg_input,  "ThemePresetsPPG" );
        } else { 
            ADIO->FM.ClearFocus( ft_ppg_input,  "ThemePresetsPPG" );
        }

        // Check for click
        for ( auto & thm : data->Themes ) {
            if ( thm.Click ) {
                // Update Color Theme Colors
                vector<int> & ThemeColors = ( inPPG.DataPtr.size() == 0 || inPPG.DataPtr[0] == nullptr ) ? ADIO->CurrentTheme : *( ( vector<int> *) inPPG.DataPtr[0] );
                ThemeColors.resize(4,0);
                for ( int i = 0; i < 4; i++ ) { ThemeColors[i] = thm.Colors[i]; }
                // Update Highlight Colors
                vector<int> & HighlightColors = ( inPPG.DataPtr.size() < 2 || inPPG.DataPtr[1] == nullptr ) ? ADIO->CurrentHighlight : *( ( vector<int> *) inPPG.DataPtr[1] );
                HighlightColors.resize(2,0);
                for(int i = 0; i < 2; i++ ) { HighlightColors[i] = thm.Colors[i+4]; }
                break;
            }
        }

        // This remakes the PPG from a tab change - must be at the end of this function
        if ( data->LastTab != data->CurrentTab ) {
            data->LastTab = data->CurrentTab;
            data->IsTabSwitch = true;
            ExitThemePresetsPPG( inPPG );
            data->SyncTheme();
            InitThemePresetsPPG( inPPG );
            data->IsTabSwitch = false;
        }

    }

    void ExitThemePresetsPPG(PropertyPage &inPPG) {
        inPPG.DeletePPGForm();
        ThemePresetsPPGData * data = (ThemePresetsPPGData *) inPPG.DataPtr.back();
        if ( data->IsTabSwitch ) { return; }
        delete reinterpret_cast<ThemePresetsPPGData*>(inPPG.DataPtr.back());            // Delete the ThemePresetsPPGData after the form
        inPPG.DataPtr.pop_back();
    }

    int UpdateThemePresetsPPG(PropertyPage &inPPG, string inType, vector<string> inMessage) {
        return 0;
    }

} // end namespace Blocks

#endif /* BLOCK_PPG_COLORS_CPP */
