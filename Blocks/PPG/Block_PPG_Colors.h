#ifndef BLOCK_PPG_COLORS_H
#define BLOCK_PPG_COLORS_H

#include "UILib.h"
using namespace UILib;

namespace Blocks {

    struct SelectColorPPGData{

        UIFormRectangle *   ColorSquare = nullptr;
        UIFormTextInput *   ColorInput = nullptr;

        HSVSelectorUI       HsvSelector;

        bool                LastFocus = 0;
        string              LastHex = "";
        string              ColorHex = "";

        bool                Preset = false;
        bool                Picker = false;

        int                 HasChange = 0;     // 0 = none, 1 = dragging color change, 2 = hex val change, 3 = mouse up from dragging (no color change)

        std::function<void(int)> ChangeFunction = [](int type){};
        std::function<void()> NoChangeFunction = [](){};

        SelectColorPPGData( unsigned int & inColor ) : HsvSelector( inColor ), ColorHex( IntToHexColor(inColor) ) {
            HsvSelector.ItemType = uif_custom_external;
            LastHex = ColorHex;
        };
        ~SelectColorPPGData() = default;
        
    };

    void InitSelectColorPPG(PropertyPage &inPPG);
    void DrawSelectColorPPG(PropertyPage &inPPG, float inW, float inH);
    void ExitSelectColorPPG(PropertyPage &inPPG);

    void OpenColorThemePPG();
    void InitColorThemePPG(PropertyPage &inPPG);
    void DrawColorThemePPG(PropertyPage &inPPG, float inW, float inH);
    void ExitColorThemePPG(PropertyPage &inPPG);
    int UpdateColorThemePPG(PropertyPage &inPPG, string inType, vector<string> inMessage);


    struct ThemeSelectionUI : UIFormItem {

        vector<int>         Colors = {};
        bool                Click = false;

        ThemeSelectionUI( const std::initializer_list<int> inClrs = {} );
        ~ThemeSelectionUI() = default;

        void Draw( float inX, float inY, bool clickMute ) override final;
        vec2 GetAutoDim() override final;
        void ResetDraw() override final;

    };

    void OpenThemePresetsPPG(PropertyPage &inPPG, const string & inSrc, const string & inName2, vector<int> & inClrs, vector<int> & inHClrs, vec2 inPos, int inRef = 0 );
    void InitThemePresetsPPG(PropertyPage &inPPG);
    void DrawThemePresetsPPG(PropertyPage &inPPG, float inW, float inH);
    void ExitThemePresetsPPG(PropertyPage &inPPG);
    int  UpdateThemePresetsPPG(PropertyPage &inPPG, string inType, vector<string> inMessage);

} // end namespace Blocks

#endif /* BLOCK_PPG_COLORS_H */
