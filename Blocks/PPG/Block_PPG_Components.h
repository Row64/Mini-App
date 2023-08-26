#ifndef BLOCK_PPG_COMPONENTS_H
#define BLOCK_PPG_COMPONENTS_H

#include "UILib.h"
using namespace UILib;

namespace Blocks {

    // Common components for building PPG forms

    struct LabeledColorChip {

        bool                    Click = false;
        UIFormLabeledButton *   Chip = nullptr;

        int *                   Color = nullptr;

        UIFormLabeledButton * MakeColorChip( UIForm * form, int * inColor, string chipLabel = "", string chipW = "40", string chipH = "22" );
        bool SyncColor();
        bool CheckEvents( PropertyPage &inPPG, string inName2 = "", int inRefIndex = 0 );
    };

    struct ColorChip {

        bool                    Click = false;
        UIFormButton *          Chip = nullptr;
        int *                   Color = nullptr;

        UIFormButton * MakeColorChip( UIForm * form, int * inColor, string chipW = "40", string chipH = "22" );
        bool SyncColor();
        bool CheckEvents( PropertyPage &inPPG, bool & hasChange, string inName2 = "", int inRefIndex = 0 );
    };

    struct LabeledSliderSet {

        bool                Mute = false;
        vector<string>      Names = {};
        vector<bool*>       Sliders = {};
        vector<UIFormLabeledSlider*> SliderPtrs = {};
        int                 ChangeIndex = -1;
        
        ~LabeledSliderSet() { for ( auto s : Sliders ) { if ( s ) delete s; } };
        
        UIFormLabeledSlider * MakeSlider( UIForm * inForm, string inName, float & inVal, vec2 inBounds, string inMargin = "" );
        bool CheckEvents( PropertyPage &inPPG, bool & hasChange, string inName2 = "", int inRefIndex = 0 );
        int CheckSlidersChange();

    };

    struct ChipWithSliders : LabeledColorChip, LabeledSliderSet {

        ChipWithSliders() : LabeledColorChip(), LabeledSliderSet() {};
        
        bool CheckAllEvents( PropertyPage &inPPG, bool & hasChange, string inName2 );

    };

    struct LabeledTextInput {

        bool *      Focus = nullptr;
        bool        LastFocus = false;
        UIFormLabeledTextInput * LInput;
        string      LastInput = "";

        std::function<bool(std::string &,std::string&)> TextFunc = [](std::string & last,std::string& curr) { return true;};

        UIFormLabeledTextInput * MakeInput( UIForm * inForm, string & inText, string inLabel, string inMargin = "" );
        bool CheckEvents( PropertyPage &inPPG, bool & hasChange );

    };

    struct HighlightColorData {

        string Name = "";
        bool Check = false;
        bool LastCheck = false;
        int * CheckAsInt = nullptr;

        LabeledSliderSet Sliders;

        bool ChipClick = false;
        int * Color = nullptr;
        UIFormButton * Chip = nullptr;

        struct SliderData {
            string name;
            float * val;
            vec2 bounds;
        };

        void UpdateChipFormat();
        void AddHighlightOptions( UIForm * form, int * inCheck, int * inColor, const vector<SliderData> & inData );
        bool CheckAllEvents( PropertyPage &inPPG, int & ChangeType, string inName2 );   // ChangeType: 0 = none, 1 = check, 2 = chip color, 3 = slider val

    };

    void DrawNullHighlightButton( ImRect inRect );

    struct BIUButtons {
        
        vector<UIFormSelTextButton*> Buttons;
        vector<bool*>                BIU ;

        UIFormButtonRow * MakeButtons( UIForm *inForm, vector<bool*> inBIU, string inMargin = "" );
        bool CheckEvents( PropertyPage &inPPG, bool & hasChange );

    }; 

    struct SingleSelImageButtonSet {

        vector<UIFormSelImageButton*> Buttons;
        vector<bool*> Clicks;
        int SelectedIndex = 0;

        SingleSelImageButtonSet() = default;
        ~SingleSelImageButtonSet() {
            for ( auto click : Clicks ) { if (click) delete click; }
        }
        
        UIFormSelImageButton* AddButton( UIForm *inForm, int inTextureId, vec2 inImageDim, vec4 inPixelXYWH = vec4(0) );
        UIFormSelImageButton* AddButton( UIForm *inForm, string inPath, vec2 inImageDim, vec4 inPixelXYWH = vec4(0) );
        bool CheckEvents( PropertyPage &inPPG, bool & hasChange );
        void ChangeSelectedIndex( int inIndex );

    };

    struct MultiSelImageButtonSet {

        vector<UIFormSelImageButton*> Buttons;
        vector<bool*> Clicks;
        int ClickIndex = 0;
        
        MultiSelImageButtonSet() = default;
        ~MultiSelImageButtonSet() {
            for ( auto click : Clicks ) { if (click) delete click; }
        }
        
        UIFormSelImageButton* AddButton( UIForm *inForm, int inTextureId, vec2 inImageDim, vec4 inPixelXYWH = vec4(0) );
        UIFormSelImageButton* AddButton( UIForm *inForm, string inPath, vec2 inImageDim, vec4 inPixelXYWH = vec4(0) );
        bool CheckEvents( PropertyPage &inPPG, bool & hasChange );

    };


    // ============================================================================================ //
    //                                     CHARTING COMPONENTS                                      //
    // ============================================================================================ //  

    struct TextLabelOptionsData {
        
        vector<string>      TextOptions = {};

        vector<UIComboBox*> Primary = {};
        vector<UIComboBox*> Secondary = {};
        vector<int>         LastPrimaryIndex = {};
        vector<int>         LastSecondaryIndex = {};
        bool                LastReverse = false;

        vector<string>      SepTypes = { "none", ", (comma)", "; (semicolon)", ". (period)", " (NewLine)", "  (space)" };
        UIComboBox *        SepPrimary = nullptr;
        UIComboBox *        SepSecondary = nullptr;
        int                 LastSepPrimaryIndex = 0;
        int                 LastSepSecondaryIndex = 0;

        vector<int*> PrimaryIndex = {};
        vector<int*> SecondaryIndex = {};
        int* PrimarySepIndex = nullptr;
        int* SecondarySepIndex = nullptr;
        bool * Reverse = nullptr;

        vector<string>      OrientationOptions = {};
        UIComboBox*         Orientation = nullptr;
        int                 LastOrientationIndex = 0;
        int*                OrientationIndex = nullptr;

        TextLabelOptionsData( const vector<string> & inOptions = {} ) : TextOptions(inOptions) {};

        void MakeOrientationCombo( UIForm *inForm, const string & inPrefix, const vector<string> & inOptions, int* oIndex );
        void MakeOptions( UIForm *inForm, const string & inPrefix, const vector<string> & inOptions, vector<int*> pIndex, int* pSep, vector<int*> sIndex, int* sSep, bool & reverse );
        bool CheckEvents( PropertyPage &inPPG, bool & hasChange );
        
    };


} // end namespace Blocks

#endif /* BLOCK_PPG_COMPONENTS_H */
