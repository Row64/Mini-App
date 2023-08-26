#ifndef BLOCK_COLORS_H
#define BLOCK_COLORS_H

#include "UILib.h"
using namespace UILib;

namespace Blocks {

    vec3 GetBezierPoint(std::vector<vec3> controls, float t);
    std::vector<vec3> InverseBezierWithT(std::vector<vec3> &inControls, float t1, float t2);
    std::vector<vec3> BezierFitMinimized(std::vector<vec3> &inControls, float &inDist, float &inAngleFactor);

    // Custom UIFormItem
    struct ColorTemplateUI : UIFormItem {

        vector<bool>        ClickList = {0,0,0,0};
        
        vector<int>         ChipColors = {0x061B42, 0x225EA8, 0x7FCDBB, 0xFFFFD9};
        string              ChipH = "45%";
        string              GradientH = "25%";
        string              PaletteH = "20%";
        string              Spacer = "5%";

        vector<string>      TextButtons;
        string              TextButtonW = "100";
        string              ClickedText;
        
        int                 NbColorSteps = 12;
        float               ColorSmooth = 0.1f;

        ColorTemplateUI() : UIFormItem(uif_custom) { MinDim = {0, 100}; };
        ~ColorTemplateUI() = default;

        void Draw( float inX, float inY, bool clickMute ) override final;
        vec2 GetAutoDim() override final;
        void ResetDraw() override final;

    };

    struct ColorChipUI : UIFormItem {

        vector<bool>        ClickList = {};
        vector<int>         ChipColors = {};
        vector<string>      ChipText = {};
        string              Spacer = "5%";
        int                 Font = 0;
        int                 FontColor = 0x000000;
        int                 TextOrder = 0;  // 0 = before, 1 = after
        vec2                TextAlignment = {0,0};

        ColorChipUI() : UIFormItem(uif_custom) {};
        ~ColorChipUI() = default;

        void Draw( float inX, float inY, bool clickMute ) override final;
        vec2 GetAutoDim() override final;
        void ResetDraw() override final;

    };

    struct HSVSelectorUI : UIFormItem {

        unsigned int &     Color;
        float              Hue = 0;
        float              Sat = 0;
        float              Val = 0;

        bool               DraggingSV = false;
        int                ChangeSV = 0;
        
        bool               DraggingHue = false;
        int                ChangeHue = 0;

        string             HueBarH = "20";

        HSVSelectorUI( unsigned int & inColor ) : UIFormItem(uif_custom), Color(inColor) {};
        ~HSVSelectorUI() = default;

        void Draw( float inX, float inY, bool clickMute ) override final;
        
    };

    vec3 GetLabPnts( int inColor );
    float GetPercInRange(float inVal, float inMin, float inMax);
    vector<vec3> SmoothBezierKnots(float inKnotDist, vector<vec3> inControls1, vector<vec3> inControls2);

    vector<vec3> ThemeToLabPnts( const vector<int> & inTheme );
    vector<vec3> ThemetoLinearClrs( const vector<int> & inTheme );
    vec3 GetLinearRGB( const vector<vec3> & inRGBList, float inPct );
    int GetLinearClr( const vector<int> & inTheme, float inPct );

    pair<vector<int>, vector<float>> GetColorStepsFromTheme( vector<int> inTheme, float inBlend, int inNbSteps );         // returns: { {color steps}, {perc} }
    vector<int> GetColorsFromThemeWeights( vector<int> inTheme, float inBlend, vector<float> & inWeights );
    int GetColorFromTheme( vector<int> inTheme, float inBlend, float inPct );
    void AutoBlendColorTheme( vector<int> & inTheme );
    void FlipColorTheme( vector<int> & inTheme );

    class ColorThemeBlender {

    private:

        vector<int> Theme;
        float Blend;
        vector<vec3> LabPnts;
        vector<vec3> SmoothLabPnts;
        vector<vec3> LinColors;
        bool Reverse = false;   // Treat as if the theme is inverted
        
    public:

        ColorThemeBlender( const vector<int> & inTheme, float inBlend = 1 );
        ColorThemeBlender() = default;
        
        void UpdateTheme( const vector<int> & inTheme, float inBlend = 0.1f);
        void BlendColorWithTheme( int inColor, float inPct );
        void ClampSaturationValue( float sMin, float sMax, float sMult, float vMin, float vMax, float vMult );
        int GetColorFromWeight( float inWeight );
        void AutoBlend();
        void SetReverse( bool inReverse );

        void DrawGradientBB( const ImRect & inBB, int inDir, int nbDiv = 1, float startWeight = 0.0f, float endWeight = 1.0f, float startAlpha = 1.0f, float endAlpha = 1.0f, bool hasAA = false );
        
    };
    

} // end namespace Blocks


#endif /* BLOCK_COLORS_H */
