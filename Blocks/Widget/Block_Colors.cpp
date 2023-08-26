#ifndef BLOCK_COLORS_CPP
#define BLOCK_COLORS_CPP

#include "c_PipelineManager.h"
// #include "gh_Primitives.h"
using namespace AppCore;

namespace Blocks {

    // ============================================================================================ //
    //                                      Bezier Functions                                        //
    // ============================================================================================ //

    vec3 GetBezierPoint(std::vector<vec3> controls, float t){
        if(t<0){t=0;}if(t>1){t=1;}
        float c = 1.0f - t;
        float bb0 = c*c*c;float bb1 = 3*t*c*c;float bb2 = 3*t*t*c;float bb3 = t*t*t;
        vec3 point = controls[0]*bb0 + controls[1]*bb1 + controls[2]*bb2 + controls[3]*bb3;
        return point;
    }

    std::vector<vec3> InverseBezierWithT(std::vector<vec3> &inControls, float t1, float t2){
        // Takes 4 bezier controls and returns a new set of controls that will 
        // go exactly through the control point when bezier interpolated
        // Good for interpolation that goes exactly through points
        //float t1 = .3f;float t2 = .7f;
        vec3 B;vec3 C;
        vec3 &A = inControls[0];vec3 &P1 = inControls[1];vec3 &P2 = inControls[2];vec3 &D = inControls[3];
        float t11 = (1 - t1);float t21 = (1 - t2);
        float b1 = float(3*t1*pow(t11,2));float b2 = float(3*t2*pow(t21,2));
        float e1 = (t11/(3*t1)) - (t21/(3*t2));float e2 = (t1/t11) - (t2/t21);
        float e3 = float(pow(t1,2)/(3*pow(t11,2)) - pow(t2,2)/(3*pow(t21,2)));
        C.x = (P1.x/b1 - P2.x/b2 - A.x*e1 -D.x*e3)/e2;
        C.y = (P1.y/b1 - P2.y/b2 - A.y*e1 -D.y*e3)/e2;
        C.z = (P1.z/b1 - P2.z/b2 - A.z*e1 -D.z*e3)/e2;
        B.x = float((P1.x -A.x*pow(t11,3) - 3*C.x*pow(t1,2)*t11 - D.x*pow(t1,3))/b1);
        B.y = float((P1.y -A.y*pow(t11,3) - 3*C.y*pow(t1,2)*t11 - D.y*pow(t1,3))/b1);
        B.z = float((P1.z -A.z*pow(t11,3) - 3*C.z*pow(t1,2)*t11 - D.z*pow(t1,3))/b1);
        return {A, B, C, D};

    }

    std::vector<vec3> BezierFitMinimized(std::vector<vec3> &inControls, float &outDistRatio, float &inAngleFactor){
        // Cycle through variants of InverseBezierWithT, and find a best fit
        // outDistRatio is the ratio of the cageDist / startEndDist
        int detail = 15;
        
        float t1 = .3f;  // .1 to .4
        float t2 = .7f;  // .6 to .9

        float bestT1 = .3f;
        float bestT1Dist = 100000;
        std::vector<vec3> cl;
        for(int i=0; i<detail;i++){
            float perc = (float)i/(detail-1.0f);
            float curT1 = .1f + perc * .3f;
            cl = InverseBezierWithT(inControls, curT1, t2);
            float cDist = length(cl[1]-cl[0]) + length(cl[2]-cl[1]) + length(cl[3]-cl[2]);
            if(cDist < bestT1Dist){
                bestT1Dist = cDist;
                bestT1 = curT1;
            }
        }
        float bestT2 = .6f;
        float bestT2Dist = 100000;
        for(int i=0; i<detail;i++){
            float perc = (float)i/(detail-1.0f);
            float curT2 = .6f + perc * .3f;
            cl = InverseBezierWithT(inControls, bestT1, curT2);
            float cDist = length(cl[1]-cl[0]) + length(cl[2]-cl[1]) + length(cl[3]-cl[2]);
            if(cDist < bestT2Dist){
                bestT2Dist = cDist;
                bestT2 = curT2;
            } 
        }        
        outDistRatio = bestT2Dist / length(inControls[0]-inControls[3]);
        return InverseBezierWithT(inControls, bestT1, bestT2);
    }

    // ============================================================================================ //
    //                                 ColorTemplateUI Functions                                    //
    // ============================================================================================ //

    void ColorTemplateUI::Draw( float inX, float inY, bool clickMute ) {
        if ( Dim.x == 0 ) { return; }
        if ( Dim.y == 0 ) { return; }

        vec2 mPos = GetGlobalMousePos();
        ImRect rect = GetContentRect( inX, inY );

        ClickList = {0,0,0,0};

        float xPos = rect.Min.x;
        float yPos = rect.Min.y;

        float contentH = rect.GetHeight() + ( ( TextButtons.size() > 0 ) ? -30 : 0 );
        float contentW = rect.GetWidth();
        float chipH = DecodeValue(ChipH, contentH);
        float gradH = DecodeValue(GradientH, contentH);
        float palH = DecodeValue(PaletteH, contentH);
        float bsp = DecodeValue(Spacer, contentH);
        
        bool allWhite = true;
        vec2 boxDim = { (contentW-3*bsp) / 4 , chipH };      // color box dimensions
        for(int i = 0; i < 4; i++){
            ImRect chipBB = GetGlobalBB(xPos, yPos, boxDim.x, boxDim.y);
            ImRect overBB = chipBB;
            overBB.Expand(4);
            if(VecInBBClip(mPos, overBB)){
                UIDrawBBOutline(overBB, 0xBCBCBC, 2);
                if(DragStart() && !clickMute ) { ClickList[i] = true; }
            }
            UIDrawBB(chipBB, ChipColors[i]);
            if ( ChipColors[i] != 0xFFFFFF ) { allWhite = false; }
            else { UIDrawBBOutline(chipBB, 0xB5B6B7, 1 ); }
            xPos += boxDim.x + bsp;
        }
        yPos += chipH + bsp;

        vector<int> clrSteps = GetColorStepsFromTheme( ChipColors, ColorSmooth, NbColorSteps ).first;

        float segTW = contentW;
        float segW = segTW / (NbColorSteps-1);
        xPos = rect.Min.x;
        for(int i = 0; i < ( NbColorSteps-1 ); i++){
            UIAddGradientRect(xPos, yPos, segW, gradH, clrSteps[i], clrSteps[i+1], false);
            xPos += segW;
        }
        if ( allWhite ) { UIDrawBBOutline( GetBB( rect.Min.x, yPos, segTW, gradH ), 0xB5B6B7, 1 ); }
        yPos += gradH + bsp;

        segW = segTW/NbColorSteps;
        xPos = rect.Min.x;
        for(int i = 0; i < NbColorSteps; i++){
            UIAddRect(xPos, yPos, segW, palH, clrSteps[i], true, 0);
            xPos += segW;
        }
        if ( allWhite ) { UIDrawBBOutline( GetBB( rect.Min.x, yPos, segTW, gradH ), 0xB5B6B7, 1 ); }
        yPos += palH + 5;

        if ( TextButtons.size() > 0 ) {
            float textW = DecodeValue(TextButtonW, contentW);
            float textH = 25;
            float spaceW = ( contentW - textW*TextButtons.size() ) / ( TextButtons.size() - 1 );
            if ( spaceW < 0 ) {  spaceW = 0; textW = contentW / TextButtons.size(); }
            ClickedText = "";
            xPos = rect.Min.x;
            for ( int i = 0; i < TextButtons.size(); i++ ) {
                ImRect bRect = GetGlobalBB(xPos, yPos, textW, textH);
                if(UIAddButtonRectClipped(bRect, TextButtons[i], {0xFFFFFF,0xEDEDED,0xD9D9D9}, clickMute) ) { ClickedText = TextButtons[i]; }
                xPos += textW + spaceW;
            }
        } 

    }

    vec2 ColorTemplateUI::GetAutoDim() {
        vec2 dim = MinDim;
        if ( TextButtons.size() > 0 ) { dim.y += 30; }
        return dim;
    }

    void ColorTemplateUI::ResetDraw() {
        ClickList = {0,0,0,0};
    };

    // ============================================================================================ //
    //                                    ColorChipUI Functions                                     //
    // ============================================================================================ //

    void ColorChipUI::Draw( float inX, float inY, bool clickMute ) {
        if ( Dim.x == 0 ) { return; }
        if ( Dim.y == 0 ) { return; }

        vec2 mPos = GetGlobalMousePos();
        ImRect rect = GetContentRect( inX, inY );

        int nChips = (int) ChipColors.size();
        ClickList = vector<bool>( nChips, 0 );

        float contentH = rect.GetHeight();
        float contentW = rect.GetWidth();
        float bsp = DecodeValue(Spacer, contentW);

        float textH = 0;
        float chipH = contentH;
        if ( ChipText.size() > 0 ) {
            textH = UIGetFontHeight(Font) + 4;
            chipH -= textH;
        }
        float textOffset = ( TextOrder == 0 ) ? textH : 0;
        
        float xPos = rect.Min.x;
        float yPos = rect.Min.y;

        vec2 boxDim = { (contentW-(nChips-1)*bsp) / ChipColors.size() , chipH };      // color box dimensions
        vector<vec3> labPnts = {};
        for(int i = 0; i < ChipColors.size(); i++){
            if ( TextOrder == 0 ) {
                if ( ChipText.size() > i ) { 
                    ImRect textBB = GetGlobalBB(xPos, yPos, boxDim.x, textH);
                    UIAddTextWithFontBB(textBB, ChipText[i], FontColor, Font, TextAlignment );
                }
            }
            labPnts.push_back( GetLabPnts( ChipColors[i] ) );
            ImRect chipBB = GetGlobalBB(xPos, yPos+textOffset, boxDim.x, boxDim.y);
            ImRect overBB = chipBB;
            overBB.Expand(4);
            if(VecInBBClip(mPos, overBB)){
                UIDrawBBOutline(overBB, 0xBCBCBC, 2);
                if(DragStart() && !clickMute ) { ClickList[i] = true; }
            }
            UIDrawBB(chipBB, ChipColors[i]);
            if ( TextOrder == 1 ) {
                if ( ChipText.size() > i ) { 
                    ImRect textBB = GetGlobalBB(xPos, yPos+boxDim.y, boxDim.x, textH);
                    UIAddTextWithFontBB(textBB, ChipText[i], FontColor, Font, TextAlignment );
                }
            }
            xPos += boxDim.x + bsp;
        }

    }

    vec2 ColorChipUI::GetAutoDim() {
        vec2 dim = MinDim;
        if ( ChipText.size() > 0 ) { dim.y += UIGetFontHeight(Font) + 4; }
        return dim;
    }

    void ColorChipUI::ResetDraw() {
        ClickList = vector<bool>( ChipColors.size(), 0 );
    }

    // ============================================================================================ //
    //                                    HSVSelectorUI Functions                                     //
    // ============================================================================================ //

    void HSVSelectorUI::Draw( float inX, float inY, bool clickMute ) {
        if ( Dim.x == 0 ) { return; }
        if ( Dim.y == 0 ) { return; }

        ImRect rect = GetContentRect( inX, inY );

        float contentW = rect.GetWidth();
        float contentH = rect.GetHeight();
        float hbH = DecodeValue(HueBarH, contentH);

        float newHue;
        IntToHSV(Color, newHue, Sat, Val);
        if ( Sat > 0 ) { Hue = newHue; }
        
        UIColorSVPicker(rect.Min.x, rect.Min.y, contentW, contentH - hbH, Hue, Sat, Val, DraggingSV, clickMute, ChangeSV);
        UIColorHuePickerV(rect.Min.x, rect.Min.y + contentH - hbH, contentW, hbH, Hue, DraggingHue, clickMute, ChangeHue);

        if ( DraggingSV || DraggingHue ) {
            ADIO->FM.SetFocus( ft_drag_color, "PPG" );
            Color = HVStoInt(Hue, Sat, Val);
        }
        else {
            ADIO->FM.ClearFocus( ft_drag_color, "PPG" );
        }

        float xPos = rect.Min.x + Sat * contentW;
        float yPos = rect.Min.y + (1-Val) * (contentH - hbH);
        int clr = 0x000000;
        if ( Val < 0.5f ) { clr = 0xFFFFFF; }
        UIAddCircle( xPos, yPos, 5, clr, false, 2, 20 ); 
       
    }

    // ============================================================================================ //
    //                                       Color Functions                                        //
    // ============================================================================================ //

    vec3 GetLabPnts( int inColor ) {
        ColorSpace::Lab lp = ClToLab( inColor );
        return { lp.l, lp.a, lp.b };
    }

    float GetPercInRange(float inVal, float inMin, float inMax){
        if(inVal < inMin){inVal = inMin;};
        if(inVal > inMax){inVal = inMax;};
        float perc = (inVal - inMin)/(inMax - inMin);
        return perc;
    }

    vector<vec3> SmoothBezierKnots(float inCageRatio, vector<vec3> inControls1, vector<vec3> inControls2){
        // if the lab points are to close to each other apply 2 levels of smoothing to prevent knots
        // level 1 is blending from a bezier through controls to between controls
        // level 2 is smoothing the control points themselves to remove cusps and intersections
        float blendPerc = GetPercInRange(inCageRatio, 1, 2.5);
        float cuspFactor = GetPercInRange(inCageRatio, 2, 8);
        vec3 B = inControls1[1];vec3 C = inControls1[2];
        vec BsTarget = mix(inControls1[0], C, .5f);
        vec CsTarget = mix(inControls1[1], B, .5f);
        B = cuspFactor * BsTarget + (1-cuspFactor) * B;
        C = cuspFactor * CsTarget + (1-cuspFactor) * C;
        vec3 newB = blendPerc * B + (1-blendPerc) * inControls2[1];
        vec3 newC = blendPerc * C + (1-blendPerc) * inControls2[2];
        vector<vec3> blendControls = {inControls1[0],newB,newC,inControls1[3]};
        return blendControls;
    }

    vector<vec3> ThemeToLabPnts( const vector<int> & inTheme ) {
        vector<vec3> labPnts = {};
        for(int i=0;i<inTheme.size();i++){
            labPnts.push_back( GetLabPnts( inTheme[i] ) );
        }
        return labPnts;
    }

    vector<vec3> ThemetoLinearClrs( const vector<int> & inTheme ) {
        vector<vec3> clrs = {};
        for(int i=0;i<inTheme.size();i++){
            clrs.push_back( HexToRGBVec(inTheme[i]) );
        }
        return clrs;
    }

    vec3 GetLinearRGB( const vector<vec3> & inRGBList, float inPct ) {
        vec3 linRGB = inRGBList[0];
        float rangeMin = 0;
        float rangeSize = 1.0f / (inRGBList.size()-1);
        for ( int i = 0; i < inRGBList.size()-1; i++ ) {
            if ( inPct > rangeMin && inPct <= rangeMin + rangeSize ) {
                return glm::mix( inRGBList[i], inRGBList[i+1], (inPct-rangeMin)/rangeSize );
            }
            rangeMin += rangeSize;
        }
        return linRGB;
    }

    int GetLinearClr( const vector<int> & inTheme, float inPct ) {
        return RGBtoInt( GetLinearRGB( ThemetoLinearClrs( inTheme ), inPct ) );
    }

    pair<vector<int>, vector<float>> GetColorStepsFromTheme( vector<int> inTheme, float inBlend, int inNbSteps ) {
        vector<vec3> labPnts = ThemeToLabPnts( inTheme );
        float bezierDist; float angleFactor;
        vector<vec3> newLabPnts = BezierFitMinimized(labPnts, bezierDist, angleFactor);
        newLabPnts = SmoothBezierKnots(bezierDist, labPnts, newLabPnts);
        vector<vec3> linColors = ThemetoLinearClrs( inTheme );
        vector<int> clrSteps = {};
        vector<float> percs = {};
        for(int i=0;i<inNbSteps;i++){
            float perc = ( inNbSteps > 1) ? ((float)i/(inNbSteps-1)) : 0.5f;      // If only 1 step, then set it to middle value of color theme
            vec3 bezP = GetBezierPoint(newLabPnts, perc);
            ColorSpace::Lab labP = {bezP.x,bezP.y,bezP.z};
            int labCl = LabtoInt(labP);
            vec3 labRGB = HexToRGBVec( labCl );
            vec3 linRGB = GetLinearRGB( linColors, perc );
            clrSteps.push_back( RGBtoInt( glm::mix(linRGB, labRGB, inBlend) ) );
            percs.push_back( perc );
        }
        return std::move( pair<vector<int>, vector<float>>(clrSteps, percs) );
    }

    vector<int> GetColorsFromThemeWeights( vector<int> inTheme, float inBlend, vector<float> & inWeights ) {
        vector<vec3> labPnts = ThemeToLabPnts( inTheme );
        float bezierDist; float angleFactor;
        vector<vec3> newLabPnts = BezierFitMinimized(labPnts, bezierDist, angleFactor);
        newLabPnts = SmoothBezierKnots(bezierDist, labPnts, newLabPnts);
        vector<vec3> linColors = ThemetoLinearClrs( inTheme );
        vector<int> clrSteps = {};
        for(int i=0;i<inWeights.size();i++){
            if ( inWeights[i] < 0 ) { clrSteps.push_back(0); }
            else {
                vec3 bezP = GetBezierPoint(newLabPnts, inWeights[i]);
                ColorSpace::Lab labP = {bezP.x,bezP.y,bezP.z};
                int labCl = LabtoInt(labP);
                vec3 labRGB = HexToRGBVec( labCl );
                vec3 linRGB = GetLinearRGB( linColors, inWeights[i] );
                clrSteps.push_back( RGBtoInt( glm::mix(linRGB, labRGB, inBlend) ) );
            }
        }
        return std::move( clrSteps );
    }

    int GetColorFromTheme( vector<int> inTheme, float inBlend, float inPct ) {

        // Lab Colors
        vector<vec3> labPnts = ThemeToLabPnts( inTheme );
        float bezierDist; float angleFactor;
        vector<vec3> newLabPnts = BezierFitMinimized(labPnts, bezierDist, angleFactor);
        newLabPnts = SmoothBezierKnots(bezierDist, labPnts, newLabPnts);
        vec3 bezP = GetBezierPoint(newLabPnts, inPct);
        ColorSpace::Lab labP = {bezP.x,bezP.y,bezP.z};
        int labCl = LabtoInt(labP);
        vec3 labRGB = HexToRGBVec( labCl );

        // Linear Colors
        vector<vec3> linColors = ThemetoLinearClrs( inTheme );
        vec3 linRGB = GetLinearRGB( linColors, inPct );

        return RGBtoInt( glm::mix(linRGB, labRGB, inBlend) );
    }

    void AutoBlendColorTheme( vector<int> & inTheme ) {
        ColorSpace::Lab lp0 = ClToLab( inTheme[0] );
        ColorSpace::Lab lp3 = ClToLab( inTheme[3] );
        vec3 v0 = {lp0.l, lp0.a, lp0.b };
        vec3 v3 = {lp3.l, lp3.a, lp3.b };
        vec3 v1 = mix(v0,v3,.33f);
        vec3 v2 = mix(v0,v3,.67f);
        ColorSpace::Lab labP1 = {v1.x,v1.y,v1.z};
        ColorSpace::Lab labP2 = {v2.x,v2.y,v2.z};
        inTheme[1] = LabtoInt(labP1);
        inTheme[2] = LabtoInt(labP2);
    }

    void FlipColorTheme( vector<int> & inTheme ) {
        std::reverse( inTheme.begin(), inTheme.end() );
    }

    ColorThemeBlender::ColorThemeBlender( const vector<int> & inTheme, float inBlend ) {             
        UpdateTheme( inTheme, inBlend );
    }

    void ColorThemeBlender::UpdateTheme( const vector<int> & inTheme, float inBlend ) {
        if ( Theme == inTheme && Blend == inBlend ) { return; }
        Theme = inTheme; Blend = inBlend;
        LabPnts = ThemeToLabPnts( inTheme );
        float bezierDist; float angleFactor;
        SmoothLabPnts = SmoothBezierKnots(bezierDist, LabPnts, BezierFitMinimized(LabPnts, bezierDist, angleFactor));
        LinColors = ThemetoLinearClrs( inTheme );
        Reverse = false;
    }

    void ColorThemeBlender::BlendColorWithTheme( int inColor, float inPct ) {
        vector<int> newTheme = {};
        for ( auto & color : Theme ) {
            newTheme.push_back( UIGetBlendClr( {color, inColor}, inPct ) );
        }
        UpdateTheme( newTheme, Blend );
    }

    void ColorThemeBlender::ClampSaturationValue( float sMin, float sMax, float sMult, float vMin, float vMax, float vMult ) {
        vector<int> newTheme = {};
        float h, s, v;
        for ( auto & color : Theme ) {
            IntToHSV(color,h,s,v);
            s *= sMult;
            v *= vMult;
            if ( s < sMin ) { s = sMin; } else if ( s > sMax ) { s = sMax; }
            if ( v < vMin ) { v = vMin; } else if ( v > vMax ) { v = vMax; }
            newTheme.push_back( HVStoInt(h,s,v) );
        }
        UpdateTheme( newTheme, Blend );
    }

    int ColorThemeBlender::GetColorFromWeight( float inWeight ) {
        if ( Theme.empty() ) { return 0; }
        if ( inWeight < 0 ) { inWeight = 0; } else if ( inWeight > 1 ) { inWeight = 1; }
        if ( Reverse ) { inWeight = 1 - inWeight; }
        vec3 bezP = GetBezierPoint(SmoothLabPnts, inWeight);
        return RGBtoInt( glm::mix( GetLinearRGB( LinColors, inWeight ), HexToRGBVec( LabtoInt( { bezP.x, bezP.y, bezP.z } ) ), Blend) );
    }

    void ColorThemeBlender::AutoBlend() {
        AutoBlendColorTheme( Theme );
        UpdateTheme( Theme, Blend );
    }

    void ColorThemeBlender::SetReverse( bool inReverse ) {
        Reverse = inReverse;
    }

    void ColorThemeBlender::DrawGradientBB( const ImRect & inBB, int inDir, int nbDiv, float startWeight, float endWeight, float startAlpha, float endAlpha, bool hasAA ) {
        // inDir: 0 = Horizontal, 1 = Vertical
        // Vertical bar draws from TOP to BOTTOM.
        // Horizontal bar draw from LEFT to RIGHT
        assert( inDir == 0 || inDir == 1 );
        float step = ( inBB.Max[inDir] - inBB.Min[inDir] ) / nbDiv;     // position step
        vec2 v0 = inBB.Min;
        vec2 v1 = inBB.Max;
        float pstep = ( endWeight - startWeight ) / nbDiv ;             // percentage step
        int c0 = GetColorFromWeight( startWeight );
        int c1;
        float astep = ( startAlpha == endAlpha ) ? 0 : ( endAlpha - startAlpha ) / nbDiv;   // alpha step
        float a0 = startAlpha;
        float a1;
        const float aaSize = 1.0f;
        if ( hasAA ) {
            int othDir = ( inDir == 1 ? 0 : 1 );
            v0[othDir] += aaSize*0.5f;
            v1[othDir] -= aaSize*0.5f;
        }

        for ( int i = 0; i < nbDiv; i++ ) {
            v1[inDir] = inBB.Min[inDir] + (i+1) * step;
            c1 = GetColorFromWeight( startWeight + (i+1) * pstep );
            a1 = startAlpha + (i+1) * astep;
            ( inDir == 0 ) ? UIAddQuadPrim(  { v0.x, v0.y }, { v1.x, v0.y },{ v1.x, v1.y }, { v0.x, v1.y }, c0, c1, c1, c0, a0, a1, a1, a0 )
                           : UIAddQuadPrim(  { v0.x, v0.y }, { v1.x, v0.y },{ v1.x, v1.y }, { v0.x, v1.y }, c0, c0, c1, c1, a0, a0, a1, a1 );
            if ( hasAA ) {
                if ( inDir == 0 ) {
                    UIAddQuadPrim( { v0.x, v0.y-aaSize }, { v1.x, v0.y-aaSize },{ v1.x, v0.y }, { v0.x, v0.y }, c0, c1, c1, c0, 0, 0, a1, a0 );     // top AA
                    UIAddQuadPrim( { v0.x, v1.y }, { v1.x, v1.y },{ v1.x, v1.y+aaSize }, { v0.x, v1.y+aaSize }, c0, c1, c1, c0, a0, a1, 0, 0 );     // bottom AA
                }
                else {
                    UIAddQuadPrim(  { v0.x-aaSize, v0.y }, { v0.x, v0.y },{ v0.x, v1.y }, { v0.x-aaSize, v1.y }, c0, c0, c1, c1, 0, a0, a1, 0 );    // left AA
                    UIAddQuadPrim(  { v1.x, v0.y }, { v1.x+aaSize, v0.y },{ v1.x+aaSize, v1.y }, { v1.x, v1.y }, c0, c0, c1, c1, a0, 0, 0, a1 );    // right AA
                }
            }
            v0[inDir] = v1[inDir];
            c0 = c1;
            a0 = a1;
        }

    }


} // end namespace Blocks

#endif /* BLOCK_COLORS_CPP */
