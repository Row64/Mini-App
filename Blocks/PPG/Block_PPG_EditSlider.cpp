#ifndef BLOCK_PPG_EDITSLIDER_CPP
#define BLOCK_PPG_EDITSLIDER_CPP

#include "c_PipelineManager.h"
using namespace AppCore;

namespace Blocks {

    // ============================================================================================ //
    //                                       EDIT SLIDER RANGE                                      //
    // ============================================================================================ //

    struct EditSliderPPGData {

        vector<string> ValuesLabels = {"Value: ", "Min: ", "Max: " };
        vector<string> ValuesText;
        vector<float*> Values;

        bool LastFocus[3] = {0,0,0};
        vector<bool*> PFocus;
        bool * PChange;
        int * PID;

        EditSliderPPGData( float * inValue, float * inMin, float * inMax, bool * inChange, int * inID ) :
            PChange( inChange ), PID( inID )  {
            PFocus.resize(3);
            ValuesText.push_back( STR_FromNum( *inValue ) ); Values.push_back( inValue );
            ValuesText.push_back( STR_FromNum( *inMin ) ); Values.push_back( inMin );
            ValuesText.push_back( STR_FromNum( *inMax ) ); Values.push_back( inMax );
        };
        ~EditSliderPPGData() = default;

    };

    void OpenEditSliderPPG( string inPPGName2, int inSourcePPGIndex, int inRefID, float inW, float inH, float * inValue, float * inMin, float * inMax, bool * inChange, int * inID ) {
        string ppgName = "Edit Slider";
        std::vector<int> ppgIds = GetPpgIndexes( ppgName );
        for ( int i : ppgIds ) {
            if ( ADIO->OpenPPGs[i].Name2 == inPPGName2  ) { // Check if PPG already exists
                ADIO->MM.RemovePPG( i );                    // Remove the existing PPG and remake it
            }
        }

        vec2 mPos = GetGlobalMousePos();

        PropertyPage * subppg = OpenSubPPG(ADIO->OpenPPGs[ inSourcePPGIndex ], ppgName, {200, 85}, 0, inW, inH, {-1, mPos.y-180*0.5} );

        subppg->DataPtr.push_back( (void *) new EditSliderPPGData( inValue, inMin, inMax, inChange, inID) );
        subppg->InitFunction = InitEditSliderPPG; 
        subppg->ExitFunction = ExitEditSliderPPG;
        subppg->DrawFunction = DrawEditSliderPPG;
        subppg->MinDim = {100,50};
        subppg->Name2 = inPPGName2;
        subppg->RefIndex = inRefID;
        subppg->Data = ADIO->OpenPPGs[ inSourcePPGIndex ].Name;    // store source PPG

    }

    void OpenEditSliderPPG(PropertyPage &inPPG, std::string inPPGName2, int inRefID, float inW, float inH, UIFormLabeledSlider * inSlider ) {
        string ppgName = "Edit Slider";
        std::vector<int> ppgIds = GetPpgIndexes( ppgName );
        for ( int i : ppgIds ) {
            if ( ADIO->OpenPPGs[i].Name2 == inPPGName2  ) { // Check if PPG already exists
                ADIO->MM.RemovePPG( i );                    // Remove the existing PPG and remake it
            }
        }

        vec2 mPos = GetGlobalMousePos();
        string sourcePPGName = inPPG.Name;
        PropertyPage * subppg = OpenSubPPG(inPPG, ppgName, {200, 85}, 0, inW, inH, {-1, mPos.y-180*0.5} );

        subppg->DataPtr.push_back( (void *) new EditSliderPPGData( &inSlider->Slider.Value, &inSlider->Slider.Bounds.x, &inSlider->Slider.Bounds.y, &inSlider->Slider.Change, nullptr ) );
        subppg->InitFunction = InitEditSliderPPG; 
        subppg->ExitFunction = ExitEditSliderPPG;
        subppg->DrawFunction = DrawEditSliderPPG;
        subppg->MinDim = {100,50};
        subppg->Name2 = inPPGName2;
        subppg->RefIndex = inRefID;
        subppg->Data = sourcePPGName;    // store source PPG
    }

    void InitEditSliderPPG(PropertyPage &inPPG){

        float esp = 9;
        int ftReg = BaseFont(), ftBold = BaseFontB(), dftReg = GetCodeFont(""), sftBold = GetGeneralFont("tab");

        // Setup
        EditSliderPPGData * sd = (EditSliderPPGData *) inPPG.DataPtr.back();

        UIForm & form = inPPG.MakeNewForm( v_forward );
        form.PadLeft = "3%"; form.PadRight = "3%"; form.PadTop = "3%"; form.PadBottom = "3%";

        for ( int i = 0; i < sd->ValuesLabels.size(); i++ ) {
            auto t = form.AddLabeledTextInput( sd->ValuesText[i], sd->ValuesLabels[i] );
            t->Label.W = "50"; t->Label.Format.Font = sftBold; t->Label.Format.FontColor = 0x7AAECE; t->Label.Alignment = {0,0.5f}; 
            t->Input.Expand = 1; t->Input.Alignment = {0,0.5f};
            t->Input.MarginTop = t->Input.MarginBottom = "1";  
            t->Input.Default.Font = ftReg; t->Input.Default.BkgdColor = 0xFFFFFF; t->Input.Default.BorderColor = 0xABABAB; 
            t->Input.Focus.Font = dftReg;  t->Input.Focus.BorderColor = 0x2BA9E0;
            sd->LastFocus[i] = t->Input.Focused; sd->PFocus[i] = &t->Input.Focused;
        }

    }

    void DrawEditSliderPPG(PropertyPage &inPPG, float inW, float inH){

        float barH = 20;
        vec2 mPos = GetGlobalMousePos();
    
        EditSliderPPGData * sd = (EditSliderPPGData *) inPPG.DataPtr.back();

        inPPG.Form->UpdateDim( inPPG.Dim.x, inPPG.Dim.y-barH);
        inPPG.Form->Draw( inPPG.Pos.x, inPPG.Pos.y+barH, inPPG.Mute );

        if ( inPPG.Form->GetFocus() ) { 
            ADIO->FM.SetFocus( ft_ppg_input, "EditSliderPPG" );
        } else { 
            ADIO->FM.ClearFocus( ft_ppg_input,"EditSliderPPG" );
        }
        
        // Update numbers text input when NOT FOCUSED
        for ( int i = 0; i < sd->ValuesLabels.size(); i++ ) {
            if ( sd->LastFocus[i] != *sd->PFocus[i] ) {
                if ( sd->LastFocus[i] ) {   // update when EXITING focus
                    if ( sd->ValuesText[i] == "" ) { 
                        sd->ValuesText[i] = to_string( *sd->Values[i] ); 
                    }
                    else if ( STR_IsNum( sd->ValuesText[i] ) ) {
                        *sd->Values[i] = stof( sd->ValuesText[i] );
                        if ( i==0 ) { 
                            *sd->PChange = true; 
                            if ( sd->PID != nullptr ) { *sd->PID = inPPG.RefIndex; } 
                        }
                    }
                    else {
                        sd->ValuesText[i] = to_string( *sd->Values[i] ); 
                    }
                }
                STR_NumTrimZeros( sd->ValuesText[i] );
                sd->LastFocus[i] = *sd->PFocus[i];
                break;
            }
            else {
                sd->ValuesText[i] = STR_FromNum( *sd->Values[i] );
            }
        }
    }

    void ExitEditSliderPPG(PropertyPage &inPPG){
        inPPG.DeletePPGForm();
        delete reinterpret_cast<EditSliderPPGData*>(inPPG.DataPtr.back());            // Delete the EditSliderPPGData after the form
        inPPG.DataPtr.pop_back();
    }



} // end namespace Blocks

#endif /* BLOCK_PPG_EDITSLIDER_CPP */
