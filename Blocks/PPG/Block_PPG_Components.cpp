#ifndef BLOCK_PPG_COMPONENTS_CPP
#define BLOCK_PPG_COMPONENTS_CPP

#include "c_PipelineManager.h"
using namespace AppCore;

namespace Blocks {

    // LabeledColorChip --------------------------------------------------------------------------- //

    UIFormLabeledButton * LabeledColorChip::MakeColorChip( UIForm * form, int * inColor, string inLabel, string chipW, string chipH ) {
        Color = inColor;
        Chip = form->AddLabeledButton( Click, inLabel ); 
        Chip->Direction = h_reverse;
        Chip->Label.Alignment = { 0, 0.5 };
        Chip->Button.W = chipW; Chip->Button.H = chipH;
        Chip->Button.MarginRight = "10";
        Chip->Button.Default.BkgdColor = *Color;  
        Chip->Button.Default.BorderSize = -1.0f;  Chip->Button.Default.BorderColor = 0xB5B6B7;
        Chip->Button.Over.BorderSize = 2; Chip->Button.Over.BorderColor = 0xBCBCBC;
        Chip->Button.Down.BorderSize = 2; Chip->Button.Down.BorderColor = 0x7C7C7C;
        SyncColor();
        return Chip;
    }

    bool LabeledColorChip::SyncColor() {
        if ( !Color ) { return false; }
        bool HasChange = ( Chip->Button.Default.BkgdColor != *Color );
        Chip->Button.Default.BkgdColor = *Color;
        Chip->Button.Default.BorderSize = ( Chip->Button.Default.BkgdColor == 0xFFFFFF ) ? 1.0f : -1.0f;
        return HasChange;
    }

    bool LabeledColorChip::CheckEvents( PropertyPage &inPPG, string inName2, int inRefIndex ) {
        if ( Click ) {      // Open Select Color PPG on click
            vec2 mPos = GetGlobalMousePos();
            PropertyPage * subppg = OpenSubPPG(inPPG, "Select Color", {270,180}, inRefIndex, ADIO->Dim.x, ADIO->Dim.y, {-1, mPos.y-180*0.5} );
            subppg->DataPtr.push_back( Color );
            subppg->InitFunction = InitSelectColorPPG; 
            subppg->ExitFunction = ExitSelectColorPPG;
            subppg->DrawFunction = DrawSelectColorPPG;
            subppg->MinDim = {270,180};
            subppg->Name2 = ": " + ( inName2 == "" ? "" : inName2 + " " ) + Chip->Label.Text;
            return true;
        }
        return false;
    }

    // ColorChip --------------------------------------------------------------------------- //

    UIFormButton * ColorChip::MakeColorChip( UIForm * form, int * inColor, string chipW, string chipH ) {
        Color = inColor;
        Chip = form->AddButton( Click ); 
        Chip->W = chipW; Chip->H = chipH;
        Chip->Default.BkgdColor = *Color;  
        Chip->Default.BorderSize = -1.0f;  Chip->Default.BorderColor = 0xB5B6B7;
        Chip->Over.BorderSize = 2; Chip->Over.BorderColor = 0xBCBCBC;
        Chip->Down.BorderSize = 2; Chip->Down.BorderColor = 0x7C7C7C;
        SyncColor();
        return Chip;
    }

    bool ColorChip::SyncColor() {
        if ( !Color ) { return false; }
        bool HasChange = ( Chip->Default.BkgdColor != *Color );
        Chip->Default.BkgdColor = *Color;
        Chip->Default.BorderSize = ( Chip->Default.BkgdColor == 0xFFFFFF ) ? 1.0f : -1.0f;
        return HasChange;
    }

    bool ColorChip::CheckEvents( PropertyPage &inPPG, bool & hasChange, string inName2, int inRefIndex ) {
        if ( Click ) {      // Open Select Color PPG on click
            vec2 mPos = GetGlobalMousePos();
            PropertyPage * subppg = OpenSubPPG(inPPG, "Select Color", {270,180}, inRefIndex, ADIO->Dim.x, ADIO->Dim.y, {-1, mPos.y-180*0.5} );
            subppg->DataPtr.push_back( Color );
            subppg->InitFunction = InitSelectColorPPG; 
            subppg->ExitFunction = ExitSelectColorPPG;
            subppg->DrawFunction = DrawSelectColorPPG;
            subppg->MinDim = {270,180};
            subppg->Name2 = inName2.empty() ? "" : ": " + inName2;
            return true;
        }
        else if ( SyncColor() ) {
            hasChange = true;
            return true;
        }
        return false;
    }

    // LabeledSliderSet --------------------------------------------------------------------------- //
    
    UIFormLabeledSlider * LabeledSliderSet::MakeSlider( UIForm * inForm, string inName, float & inVal, vec2 inBounds, string inMargin ) {
        bool * sb = new bool(false);
        auto t = inForm->AddText( inName ); t->Format.Font = GetGeneralFont("tab"); t->Format.FontColor = 0x7AAECE; t->MarginBottom = "2";
        auto sl = inForm->AddLabeledSlider( inVal, Mute, *sb, inBounds ); 
        inForm->AddSpacer( 9 );
        if ( inMargin != "" ) {
            t->MarginLeft = t->MarginRight = inMargin;
            sl->MarginLeft = sl->MarginRight = inMargin;
        }
        Names.push_back( inName );
        Sliders.push_back( sb );
        SliderPtrs.push_back( sl );
        return sl;
    };

    bool LabeledSliderSet::CheckEvents( PropertyPage &inPPG, bool & hasChange, string inName2, int inRefIndex ) {
        // Returns true if there is an event. Sets hasChange to true if the event involves a data change.
        ChangeIndex = -1;
        for ( int i = 0; i < Names.size(); i++ ) {
            if ( *Sliders[i] ) { ChangeIndex = i; hasChange = true; return true; }
            else if ( SliderPtrs[i]->ValueClick ) {
                string ppgName2 = ": " + ( inName2 == "" ? "" : inName2 + " " ) + Names[i];
                OpenEditSliderPPG(inPPG, ppgName2, inRefIndex, ADIO->Dim.x, ADIO->Dim.y, SliderPtrs[i] );
                return true;
            }
        }
        return false;
    };

    int LabeledSliderSet::CheckSlidersChange() {
        for ( int i = 0; i < Sliders.size(); i++ ) {
            if ( *Sliders[i] ) { return i; }
        }
        return -1;
    }

    // ChipWithSliders --------------------------------------------------------------------------- //

    bool ChipWithSliders::CheckAllEvents( PropertyPage &inPPG, bool & hasChange, string inName2 ) {
            if ( LabeledColorChip::CheckEvents( inPPG, inName2, 0) ) { 
                return true; 
            } 
            SyncColor();
            return LabeledSliderSet::CheckEvents( inPPG, hasChange, inName2 );
        };

    // LabeledTextInput --------------------------------------------------------------------------- //

    UIFormLabeledTextInput * LabeledTextInput::MakeInput( UIForm * inForm, string & inText, string inLabel, string inMargin ) {
        int ftReg = BaseFont();
        LInput = inForm->AddLabeledTextInput( inText, inLabel ); 
        LInput->Label.H = "22"; LInput->Label.Format.Font = ftReg; LInput->Label.MarginRight = "5"; LInput->Label.Alignment = {0, 0.45f};
        LInput->Input.H = "22"; LInput->Label.Format.Font = ftReg; LInput->Input.Expand = 1;
        LInput->MarginLeft = LInput->MarginRight = inMargin;
        Focus = &LInput->Input.Focused;
        LastFocus = LInput->Input.Focused;
        LastInput = inText;
        return LInput;
    };

    bool LabeledTextInput::CheckEvents( PropertyPage &inPPG, bool & hasChange ) {
        if ( LastFocus != *Focus ) {
            if ( LastFocus ) {   // update when EXITING focus
                if  ( LInput->Input.Text != LastInput ) {
                    if ( TextFunc( LastInput, LInput->Input.Text ) ) {
                        LastInput = LInput->Input.Text;
                        hasChange = true; 
                    }
                    else {
                        LInput->Input.Text = LastInput;
                    }
                    LastFocus = *Focus;
                    return true;
                }
            }
            LastFocus = *Focus;
            return false;
        }
        else if ( *Focus ) {
            // std::cout << "Focused..." << std::endl;
            return true;        // the input still has the event
        }
        return false;
    };


    // HighlightColorData --------------------------------------------------------------------------- //

    void HighlightColorData::UpdateChipFormat() {
        if ( Check ) {
            Chip->Over.BorderSize = 4; Chip->Over.BorderColor = 0xBCBCBC;
            Chip->Down.BorderSize = 4; Chip->Down.BorderColor = 0x7C7C7C;
        } else {
            Chip->Over.BorderSize = -1; Chip->Over.BorderColor = -1;
            Chip->Down.BorderSize = -1; Chip->Down.BorderColor = -1;
        }
    }

    void HighlightColorData::AddHighlightOptions( UIForm * form, int * inCheck, int * inColor, const vector<HighlightColorData::SliderData> & inData ) {

        int hftBold = GetGeneralFont("headerS"), sftBold = GetGeneralFont("tab");

        float esp = 9;
        CheckAsInt = inCheck; 
        Check = ( *CheckAsInt == 1 );
        auto chk = form->AddLabeledCheckBox( Check, 22, Name ); 
        chk->Direction = h_reverse; chk->Label.Format.Font = hftBold; chk->Label.Alignment = {0, 0.5};
        chk->Check.MarginLeft = chk->Check.MarginTop = chk->Check.MarginBottom = "4"; chk->Check.MarginRight = "10";
        LastCheck = Check;

        form->AddSpacer( esp );

        auto sf = form->AddForm( h_forward ); //sf->H = "35";
        
        Color = inColor;
        Chip = sf->AddButton( ChipClick ); Chip->W = "60"; Chip->H = "35";
        Chip->Default.BorderSize = -1; Chip->Default.BkgdColor = *Color; Chip->Default.BorderColor = 0xB5B6B7;
        UpdateChipFormat();
        
        Chip->DrawNullButton = DrawNullHighlightButton;

        auto sf2 = sf->AddForm( v_forward ); sf2->Expand = 1; sf2->MarginLeft ="10";
        for ( int i = 0; i < inData.size(); i++ ) {
            Sliders.MakeSlider( sf2, inData[i].name, *inData[i].val, inData[i].bounds );
        }
        Sliders.Mute = !Check;

    }

    bool HighlightColorData::CheckAllEvents( PropertyPage &inPPG, int & ChangeType, string inName2 ) {
        // ChangeType: 0 = none, 1 = check, 2 = chip color, 3 = slider val

        ChangeType = 0;

        if ( Check != LastCheck ) {        // Clicked Check Box
            Sliders.Mute = !Check;
            LastCheck = Check;
            *CheckAsInt = Check ? 1 : 0;
            UpdateChipFormat();
            ChangeType = 1;
            return true;
        }
        else if ( Check ) {
            if ( ChipClick ) {             // Clicked Chip
                vec2 mPos = GetGlobalMousePos();
                PropertyPage * subppg = OpenSubPPG(inPPG, "Select Color", {270,180}, 0, ADIO->Dim.x, ADIO->Dim.y, {-1, mPos.y-180*0.5} );
                subppg->DataPtr.push_back( Color );
                subppg->InitFunction = InitSelectColorPPG; 
                subppg->ExitFunction = ExitSelectColorPPG;
                subppg->DrawFunction = DrawSelectColorPPG;
                subppg->MinDim = {270,180};
                subppg->Name2 = ": " + inName2;
                return true;
            }
            else if ( *Color != Chip->Default.BkgdColor ) {    // Chip color is changed
                Chip->Default.BkgdColor = *Color;
                Chip->Default.BorderSize = ( Chip->Default.BkgdColor == 0xFFFFFF ) ? 1.0f : -1.0f;
                ChangeType = 2;
                return true;
            }
            else {
                bool SliderChange = false;
                if ( Sliders.CheckEvents( inPPG, SliderChange) ) {
                    if ( SliderChange ) { ChangeType = 3; }
                    return true;
                }
            }
        }
        return false;
    }

    void DrawNullHighlightButton( ImRect inRect ) {
        int bClr = 0xB5B6B7;
        UIDrawBBOutline(inRect, bClr, 1); 
        UIAddLine( inRect.GetTL().x, inRect.GetTL().y, inRect.GetBR().x, inRect.GetBR().y, bClr, 1 ); 
        UIAddLine( inRect.GetTR().x, inRect.GetTR().y, inRect.GetBL().x, inRect.GetBL().y, bClr, 1 );
    }

    // BIUButtons --------------------------------------------------------------------------- //

    UIFormButtonRow * BIUButtons::MakeButtons( UIForm *inForm, vector<bool*> inBIU, string inMargin ) {
        BIU = inBIU;
        vector<string> brText = { "B", "I", "U" };
        vector<int> brFont = { BaseFontB(), BaseFontB()+1, BaseFont() };
        auto br = inForm->AddButtonRow(); br->H = "26", br->W = "90"; br->MarginLeft = br->MarginRight = inMargin;
        for ( int i = 0; i < brText.size(); i++ ) {
            auto b = br->AddSelTextButton( brText[i] ); b->W = b->H = "26"; b->Alignment = { 0.5f, 0.45f };
            b->Selected = *BIU[i];
            b->Default.Font = brFont[i]; b->Default.FontUnderline = (float) ( brText[i] == "U" ? 1 : -1 );
            b->Default.BkgdColor = 0xE5E5E5; b->Default.BorderSize = 0;
            b->Over.BkgdColor = 0xBFBFBF;
            b->Down.BkgdColor = 0xBFBFBF; b->Down.BorderColor = 0xAFAFAF; b->Down.BorderSize = 1;
            b->Select.BkgdColor = 0xE5E5E5; b->Select.BorderColor = 0xAFAFAF; b->Select.BorderSize = 1;
            Buttons.push_back( b );
        }
        return br;
    };

    bool BIUButtons::CheckEvents( PropertyPage &inPPG, bool & hasChange ) {
        for ( int i = 0; i < Buttons.size(); i++ ) {
            if ( Buttons[i]->Click ) {
                *BIU[i] = Buttons[i]->Selected; 
                hasChange = true;
                return true;
            }   
        }
        return false;
    };


    // SingleSelImageButtonSet --------------------------------------------------------------------------- //

    UIFormSelImageButton* SingleSelImageButtonSet::AddButton( UIForm *inForm, int inTextureId, vec2 inImageDim, vec4 inPixelXYWH ) {
        bool * click = new bool(false);
        auto b = inForm->AddSelImageButton( *click, inTextureId, inImageDim, inPixelXYWH ); 
        b->Image.Scale = 0.5f;
        b->Default.BorderSize=0; b->Rounding=2; b->ImageOrder=1;
        b->Default.BkgdColor = 0xE5E5E5;
        b->Over.BkgdColor = 0xC6C6C6;
        b->Down.BkgdColor = b->Select.BkgdColor = 0xB6D2E4;
        Buttons.push_back(b);
        Clicks.push_back(click);
        return b;
    }

    UIFormSelImageButton* SingleSelImageButtonSet::AddButton( UIForm *inForm, string inPath, vec2 inImageDim, vec4 inPixelXYWH ) {
        bool * click = new bool(false);
        auto b = inForm->AddSelImageButton( *click, inPath, inImageDim, inPixelXYWH ); 
        b->Image.Scale = 0.5f;
        b->Default.BorderSize=0; b->Rounding=2; b->ImageOrder=1;
        b->Default.BkgdColor = 0xE5E5E5; // light gray
        b->Over.BkgdColor = 0xC6C6C6;    // gray
        b->Down.BkgdColor = b->Select.BkgdColor = 0xB6D2E4; // pale-blue down & selected color
        Buttons.push_back(b);
        Clicks.push_back(click);
        return b;
    }

    bool SingleSelImageButtonSet::CheckEvents( PropertyPage &inPPG, bool & hasChange ) {
        for ( int i = 0; i < Clicks.size(); i++ ) {
            if ( *Clicks[i] && SelectedIndex != i ) {
                if ( SelectedIndex >= 0 ) {
                    Buttons[SelectedIndex]->Selected = false;
                    Buttons[SelectedIndex]->Over.BkgdColor = 0xC6C6C6;  // gray
                }
                Buttons[i]->Over.BkgdColor = 0xA3BECC;  // blue-gray
                SelectedIndex = i;
                hasChange = true;
                return true;
            }
        }
        return false;
    }

    void SingleSelImageButtonSet::ChangeSelectedIndex( int inIndex ) {
        if ( inIndex < 0 || inIndex >= Buttons.size() ) { return; }
        if ( SelectedIndex >= 0 ) {
            Buttons[SelectedIndex]->Selected = false;
            Buttons[SelectedIndex]->Over.BkgdColor = 0xC6C6C6;  // gray
        }
        Buttons[inIndex]->Selected = true;
        Buttons[inIndex]->Over.BkgdColor = 0xA3BECC;  // blue-gray
        SelectedIndex = inIndex;
    }

    // MultiSelImageButtonSet --------------------------------------------------------------------------- //

    UIFormSelImageButton* MultiSelImageButtonSet::AddButton( UIForm *inForm, int inTextureId, vec2 inImageDim, vec4 inPixelXYWH ) {
        bool * click = new bool(false);
        auto b = inForm->AddSelImageButton( *click, inTextureId, inImageDim, inPixelXYWH ); 
        b->Image.Scale = 0.5f;
        b->Default.BorderSize=0; b->Rounding=2; b->ImageOrder=1;
        b->Default.BkgdColor = 0xE5E5E5;
        b->Over.BkgdColor = 0xC6C6C6;
        b->Down.BkgdColor = b->Select.BkgdColor = 0xB6D2E4;
        Buttons.push_back(b);
        Clicks.push_back(click);
        return b;
    }

    UIFormSelImageButton* MultiSelImageButtonSet::AddButton( UIForm *inForm, string inPath, vec2 inImageDim, vec4 inPixelXYWH ) {
        bool * click = new bool(false);
        auto b = inForm->AddSelImageButton( *click, inPath, inImageDim, inPixelXYWH ); 
        b->Image.Scale = 0.5f;
        b->Default.BorderSize=0; b->Rounding=2; b->ImageOrder=1;
        b->Default.BkgdColor = 0xE5E5E5; // light gray
        b->Over.BkgdColor = 0xC6C6C6;    // gray
        b->Down.BkgdColor = b->Select.BkgdColor = 0xB6D2E4; // pale-blue down & selected color
        Buttons.push_back(b);
        Clicks.push_back(click);
        return b;
    }

    bool MultiSelImageButtonSet::CheckEvents( PropertyPage &inPPG, bool & hasChange ) {
        bool res = false;
        ClickIndex = -1;
        for ( int i = 0; i < Clicks.size(); i++ ) {
            Buttons[i]->Over.BkgdColor = ( Buttons[i]->Selected ) ? 0xA3BECC : 0xC6C6C6;
            if ( *Clicks[i] ) {
                ClickIndex = i;
                hasChange = true;
                res = true;
            }            
        }
        return res;
    }


    // TextLabelOptionsData --------------------------------------------------------------------------- //

    void TextLabelOptionsData::MakeOrientationCombo( UIForm *inForm, const string & inPrefix, const vector<string> & inOptions, int* oIndex ) {
        if ( !oIndex ) { return; }
        float esp = 9;
        string labelW = "125", comboW = "120", margin = "15";
        OrientationOptions = inOptions;
        OrientationIndex = oIndex;
        auto cb = inForm->AddLabeledComboBox( inPrefix + "_Orientation", OrientationOptions, "Orientation" );
        cb->MarginLeft = cb->MarginRight = margin;
        cb->Direction = h_forward; cb->H = "20"; cb->Label.Alignment = { 0, 0.5 }; 
        cb->Label.W = labelW; cb->Combo.W = comboW;
        if ( *oIndex < 0 ) { *oIndex = 0; } else if ( *oIndex >= (int) inOptions.size() ) { *oIndex = (int) inOptions.size() - 1; }
        LastOrientationIndex = cb->Combo.ComboBox.Index = *oIndex;
        Orientation = &cb->Combo.ComboBox;
        inForm->AddSpacer( esp );
    }

    void TextLabelOptionsData::MakeOptions( UIForm *inForm, const string & inPrefix, const vector<string> & inOptions, vector<int*> pIndex, int* pSep, vector<int*> sIndex, int* sSep, bool & reverse ) {

        // Store a handle to the actual indices
        PrimaryIndex = pIndex;
        SecondaryIndex = sIndex;
        PrimarySepIndex = pSep;
        SecondarySepIndex = sSep;
        Reverse = &reverse;
        TextOptions = inOptions;

        float esp = 9;
        string labelW = "125", comboW = "120", margin = "15";

        bool addSuffix = ( pIndex.size() > 1 );
        Primary.resize( pIndex.size() );
        LastPrimaryIndex.resize( pIndex.size() );
        for ( int i = 0; i < pIndex.size(); i++ ) {
            auto cb = inForm->AddLabeledComboBox( inPrefix + "_Primary" + to_string(i), TextOptions, "Primary Text" + ( addSuffix ? " " + to_string(i+1) : "" ) );
            cb->MarginLeft = cb->MarginRight = margin;
            cb->Direction = h_forward; cb->H = "20"; cb->Label.Alignment = { 0, 0.5 }; 
            cb->Label.W = labelW; cb->Combo.W = comboW;
            if ( *pIndex[i] < 0 ) { *pIndex[i] = 0; } else if ( *pIndex[i] >= (int) inOptions.size() ) { *pIndex[i] = (int) inOptions.size() - 1; }
            LastPrimaryIndex[i] = cb->Combo.ComboBox.Index = *pIndex[i];
            Primary[i] = &cb->Combo.ComboBox;
            inForm->AddSpacer( esp );
        }

        addSuffix = ( sIndex.size() > 1 );
        Secondary.resize( sIndex.size() );
        LastSecondaryIndex.resize( sIndex.size() );
        for ( int i = 0; i < sIndex.size(); i++ ) {
            auto cb = inForm->AddLabeledComboBox( inPrefix + "_Secondary" + to_string(i), TextOptions, "Secondary Text" + ( addSuffix ? " " + to_string(i+1) : "" ) );
            cb->MarginLeft = cb->MarginRight = margin;
            cb->Direction = h_forward; cb->H = "20"; cb->Label.Alignment = { 0, 0.5 }; 
            cb->Label.W = labelW; cb->Combo.W = comboW;
            if ( *sIndex[i] < 0 ) { *sIndex[i] = 0; } else if ( *sIndex[i] >= (int) inOptions.size() ) { *sIndex[i] = (int) inOptions.size() - 1; }
            LastSecondaryIndex[i] = cb->Combo.ComboBox.Index = *sIndex[i];
            Secondary[i] = &cb->Combo.ComboBox;
            inForm->AddSpacer( esp );
        }
        
        LastReverse = reverse;
        auto chk = inForm->AddLabeledCheckBox( reverse, 16, "Reverse Primary and Secondary" ); 
        chk->Direction = h_reverse; chk->Check.MarginRight = "10"; chk->MarginLeft = chk->MarginRight = margin;
        inForm->AddSpacer(esp);

        auto d = inForm->AddDivider(1); d->Color = 0xCDCDCD; d->MarginLeft = d->MarginRight = margin;
        inForm->AddSpacer( esp );

        auto cb = inForm->AddLabeledComboBox( inPrefix + "_PrimarySep", SepTypes, "Primary Separator" ); cb->MarginLeft = cb->MarginRight = margin;
        cb->Direction = h_forward; cb->H = "20"; cb->Label.Alignment = { 0, 0.5 }; cb->Label.W = labelW; cb->Combo.W = comboW;
        LastSepPrimaryIndex = cb->Combo.ComboBox.Index = *pSep;
        SepPrimary = &cb->Combo.ComboBox;
        inForm->AddSpacer( esp );

        cb = inForm->AddLabeledComboBox( inPrefix + "_SecondSep", SepTypes, "Secondary Separator" ); cb->MarginLeft = cb->MarginRight = margin;
        cb->Direction = h_forward; cb->H = "20"; cb->Label.Alignment = { 0, 0.5 }; cb->Label.W = labelW; cb->Combo.W = comboW;
        LastSepSecondaryIndex = cb->Combo.ComboBox.Index = *sSep;
        SepSecondary = &cb->Combo.ComboBox;
        inForm->AddSpacer( esp );

    }

    bool TextLabelOptionsData::CheckEvents( PropertyPage &inPPG, bool & hasChange ) {
        for ( int i = 0; i < Primary.size(); i++ ) {
            if ( LastPrimaryIndex[i] != Primary[i]->Index ) {
                LastPrimaryIndex[i] = Primary[i]->Index;
                if ( i < PrimaryIndex.size() ) { *PrimaryIndex[i] = Primary[i]->Index; }
                hasChange = true; return true;
            }
        }
        for ( int i = 0; i < Secondary.size(); i++ ) {
            if ( LastSecondaryIndex[i] != Secondary[i]->Index ) {
                LastSecondaryIndex[i] = Secondary[i]->Index;
                if ( i < SecondaryIndex.size() ) { *SecondaryIndex[i] = Secondary[i]->Index; }
                hasChange = true; return true;
            }
        }
        if ( LastSepPrimaryIndex != SepPrimary->Index ) {
            LastSepPrimaryIndex = SepPrimary->Index;
            if ( PrimarySepIndex ) { *PrimarySepIndex = SepPrimary->Index; }
            hasChange = true; return true;
        }
        else if ( LastSepSecondaryIndex != SepSecondary->Index ) {
            LastSepSecondaryIndex = SepSecondary->Index;
            if ( SecondarySepIndex ) { *SecondarySepIndex = SepSecondary->Index; }
            hasChange = true; return true;
        }
        else if ( Reverse ) {
            if ( LastReverse != *Reverse ) {
                LastReverse = *Reverse;
                hasChange = true; return true;
            }
        }
        if ( Orientation ) {
            if ( LastOrientationIndex != Orientation->Index ) {
                LastOrientationIndex = Orientation->Index;
                if ( OrientationIndex ) { *OrientationIndex = Orientation->Index; }
                hasChange = true; return true;
            }
        }
        return false;
    }

} // end namespace Blocks


#endif /* BLOCK_PPG_COMPONENTS_CPP */
