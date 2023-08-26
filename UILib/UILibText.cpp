#ifndef UILIBTEXT_CPP
#define UILIBTEXT_CPP

#include "UILib.h"

#pragma warning(disable:4996)  //  cross-platform so avoid these Microsoft warnings

namespace UILib {

    void UISetFont(int inIndex){ 
        // Quick way to set font.  Much Better than PushFont/PopFont which causes assets if not paired properly
        GImGui->Font = GImGui->IO.Fonts->Fonts[inIndex];
    }
    
    // --------------------------------------------------------------------- //
    // SINGLE CHARACTER DRAW / MANIPULATE                                    //
    // --------------------------------------------------------------------- //

    void UIRenderChar(ImDrawList* draw_list, float size, ImVec2 pos, int inCI, ImWchar c, ImFont* font){
        // mechanical chars like space, newline ect should be done outside this function through setting positions
        ImU32 col = Cl(inCI);
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r'){return;}
        if (const ImFontGlyph* glyph = font->FindGlyph(c)){
            float scale = (size >= 0.0f) ? (size / font->FontSize) : 1.0f;
            pos.x = (float)(int)pos.x;
            pos.y = (float)(int)pos.y;
            draw_list->PrimReserve(6, 4);
            draw_list->PrimRectUV(ImVec2(pos.x + glyph->X0 * scale, pos.y + glyph->Y0 * scale), ImVec2(pos.x + glyph->X1 * scale, 
                pos.y + glyph->Y1 * scale), ImVec2(glyph->U0, glyph->V0), ImVec2(glyph->U1, glyph->V1), col);
        }
    }
    void UIRenderChar(ImDrawList* draw_list, float size, ImVec2 pos, int inCI, ImWchar c ){
        // mechanical chars like space, newline ect should be done outside this function through setting positions
        ImU32 col = Cl(inCI);
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r'){return;}
        ImFont* font = GImGui->Font;
        if (const ImFontGlyph* glyph = font->FindGlyph(c)){
            float scale = (size >= 0.0f) ? (size / font->FontSize) : 1.0f;
            pos.x = (float)(int)pos.x;
            pos.y = (float)(int)pos.y;
            draw_list->PrimReserve(6, 4);
            draw_list->PrimRectUV(ImVec2(pos.x + glyph->X0 * scale, pos.y + glyph->Y0 * scale), ImVec2(pos.x + glyph->X1 * scale, 
                pos.y + glyph->Y1 * scale), ImVec2(glyph->U0, glyph->V0), ImVec2(glyph->U1, glyph->V1), col);
        }

    }
    void UIRenderChar(ImDrawList* draw_list, float size, ImVec2 pos, ImU32 col, ImWchar c){
        
        // mechanical chars like space, newline ect should be done outside this function through setting positions
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r'){return;}
        
        ImFont* font = GImGui->Font;
        if (const ImFontGlyph* glyph = font->FindGlyph(c)){
            float scale = (size >= 0.0f) ? (size / font->FontSize) : 1.0f;
            pos.x = (float)(int)pos.x;
            pos.y = (float)(int)pos.y;
            draw_list->PrimReserve(6, 4);
            draw_list->PrimRectUV(ImVec2(pos.x + glyph->X0 * scale, pos.y + glyph->Y0 * scale), ImVec2(pos.x + glyph->X1 * scale, 
                pos.y + glyph->Y1 * scale), ImVec2(glyph->U0, glyph->V0), ImVec2(glyph->U1, glyph->V1), col);
        }

    }

    void UIRenderChar( ImWchar c, ImVec2 pos, ImU32 col, ImFont * font ) {
        // mechanical chars like space, newline ect should be done outside this function through setting positions
        assert( font );
        if ( c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\0' ) { return; }
        if (const ImFontGlyph* glyph = font->FindGlyph(c)) {
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            const float & scale = font->Scale;
            pos.x = (float)(int)pos.x;
            pos.y = (float)(int)pos.y;
            drawList->PrimReserve(6, 4);
            drawList->PrimRectUV(  
                ImVec2(pos.x + glyph->X0 * font->Scale, pos.y + glyph->Y0 * font->Scale), 
                ImVec2(pos.x + glyph->X1 * font->Scale, pos.y + glyph->Y1 * font->Scale), 
                ImVec2(glyph->U0, glyph->V0), 
                ImVec2(glyph->U1, glyph->V1), col
            );
        }
    }

// --------------------------------------------------------------------- //
// SINGLE LINE TEXT                                                      //
// --------------------------------------------------------------------- //

    void UIAddTextFloat(float inX, float inY, float inW, float inH, float *inVal, vec2 inAlign){
        
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImVec2 wPos = window->Pos;
        ImGuiDataType data_type = ImGuiDataType_Float;
        const char* format = ImGui::DataTypeGetInfo(data_type)->PrintFmt;
        char value_buf[5];
        const char* value_buf_end = value_buf + ImGui::DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, inVal, format);
        ImVec2 min = ImVec2(inX + wPos.x, inY + wPos.y);
        ImVec2 max = ImVec2(min.x + inW, min.y + inH);
        ImGui::RenderTextClipped(min, max, value_buf, value_buf_end, NULL, ImVec2(inAlign[0], inAlign[1]));
        
    }
    void UIAddTextString(float inX, float inY, float inW, float inH, string inText, int inC, vec2 inAlign){
        size_t text_size = inText.length();
        if (text_size == 0) { return; }
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImVec2 wPos = window->Pos;
        ImGuiDataType data_type = ImGuiDataType_Float;
        const char* format = ImGui::DataTypeGetInfo(data_type)->PrintFmt;
        const char* text_buf = inText.c_str();
        const char* value_text_begin = &g.TempBuffer[0];
        ImVec2 min = ImVec2(inX + wPos.x, inY + wPos.y);
        ImVec2 max = ImVec2(min.x + inW, min.y + inH);
        ImU32 Tclr = UILib::Cl( inC );
        PushStyleColor(ImGuiCol_Text, Tclr);
        // NOTE: ImGui::RenderTextClipped will hide anything after a '##' string, so use ImGui::RenderTextClippedEx instead.
        RenderTextClippedEx(window->DrawList, min, max, text_buf, text_buf + text_size, NULL, ImVec2((float)inAlign[0], inAlign[1])); 
        PopStyleColor(1);

    }
    void UIAddGlobalText(float inX, float inY, float inW, float inH, string inText, int inC, vec2 inAlign){
        size_t text_size = inText.length();
        if (text_size == 0) { return; }
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImGuiDataType data_type = ImGuiDataType_Float;
        const char* format = ImGui::DataTypeGetInfo(data_type)->PrintFmt;
        const char* text_buf = inText.c_str();
        const char* value_text_begin = &g.TempBuffer[0];
        ImVec2 min = ImVec2(inX, inY);
        ImVec2 max = ImVec2(min.x + inW, min.y + inH);
        ImU32 Tclr = UILib::Cl( inC );
        PushStyleColor(ImGuiCol_Text, Tclr);
        // NOTE: ImGui::RenderTextClipped will hide anything after a '##' string, so use ImGui::RenderTextClippedEx instead.
        RenderTextClippedEx(window->DrawList, min, max, text_buf, text_buf + text_size, NULL, ImVec2((float)inAlign[0], inAlign[1])); 
        PopStyleColor(1);

    }
    
    void UIAddTextWithFont(float inX, float inY, float inW, float inH, const string& inText, int inC, int inFontIndex, vec2 inAlign){

        ImGuiContext& g = *GImGui;
        PushFont(g.IO.Fonts->Fonts[inFontIndex]);
        UIAddTextString(inX, inY, inW, inH, inText, inC, inAlign);
        PopFont();
        
    }

    void UIAddTextWithFont(float inX, float inY, float inW, float inH, const string& inText, int inC, int inFontIndex, vec2 inAlign, float inScale){
        ImGuiContext& g = *GImGui;
        // https://github.com/ocornut/imgui/issues/1018
        // To set scale of ImFont*, change the font->Scale 
        // value and then call PushFont() on this font. After you call PopFont you want to restore the size.
        ImFont * font = g.IO.Fonts->Fonts[inFontIndex];
        font->Scale = std::max(inScale,0.00001f);
        PushFont(font);
        UIAddTextString(inX, inY, inW, inH, inText, inC, inAlign);
        PopFont();
        font->Scale = 1.0f; // reset font scale
    }

    void UIAddTextBB(ImRect inR, const string& inText, int inC, vec2 inAlign){
        size_t text_size = inText.length();
        if (text_size == 0) { return; }
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        const char* text_buf = inText.c_str();
        const char* value_text_begin = &g.TempBuffer[0];
        ImVec2 min = ImVec2(inR.Min.x, inR.Min.y);
        ImVec2 max = ImVec2(min.x + inR.GetWidth(), min.y + inR.GetHeight());
        ImU32 Tclr = UILib::Cl( inC );
        PushStyleColor(ImGuiCol_Text, Tclr);
        // NOTE: ImGui::RenderTextClipped will hide anything after a '##' string, so use ImGui::RenderTextClippedEx instead.
        RenderTextClippedEx(window->DrawList, min, max, text_buf, text_buf + text_size, NULL, ImVec2((float)inAlign[0], inAlign[1]));
        PopStyleColor(1);

    }

    void UIAddTextBBAngle(ImRect inR, const string& inText, int inC, vec2 inAlign, vec2 inRPoint, float rad){
        // Text rotates clockwise
        size_t text_size = inText.length();
        if (text_size == 0) { return; }
        int rotation_start_index = ImGui::GetWindowDrawList()->VtxBuffer.Size;
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        const char* text_buf = inText.c_str();
        const char* value_text_begin = &g.TempBuffer[0];
        ImU32 Tclr = UILib::Cl( inC );
        PushStyleColor(ImGuiCol_Text, Tclr);
        // NOTE: ImGui::RenderTextClipped will hide anything after a '##' string, so use ImGui::RenderTextClippedEx instead.
        RenderTextClippedEx(window->DrawList, inR.Min, inR.Max, text_buf, text_buf + text_size, NULL, ImVec2((float)inAlign[0], inAlign[1]));
        auto& buf = ImGui::GetWindowDrawList()->VtxBuffer;
        float s = sin(rad), c = cos(rad);
        for (int i = rotation_start_index; i < buf.Size; i++) {
            buf[i].pos = UIRotate( buf[i].pos - inRPoint, c, s ) + inRPoint;
        }
        PopStyleColor(1);

    }

    void UIAddTextWithFontBB(const ImRect& inR, const string& inText, int inC, int inFontIndex, vec2 inAlign){

        ImGuiContext& g = *GImGui;
        PushFont(g.IO.Fonts->Fonts[inFontIndex]);
        UIAddTextBB(inR, inText, inC, inAlign);
        PopFont();

    }

    void UIAddTextWithFontBBAngle(const ImRect& inR, const string& inText, int inC, int inFontIndex, vec2 inAlign, float inScale, vec2 inRPoint, float inAng){
        ImGuiContext& g = *GImGui;
        ImFont * font = g.IO.Fonts->Fonts[inFontIndex];
        font->Scale = std::max(inScale,0.00001f);
        PushFont(font);
        UIAddTextBBAngle(inR,inText,inC,inAlign,inRPoint,inAng);
        PopFont();
        font->Scale = 1.0f; // reset font scale
    }

    void UIAddTextWithFontBB(const ImRect& inR, const string& inText, int inC, int inFontIndex, vec2 inAlign, float inScale ){
        ImGuiContext& g = *GImGui;
        // https://github.com/ocornut/imgui/issues/1018
        // To set scale of ImFont*, change the font->Scale 
        // value and then call PushFont() on this font. After you call PopFont you want to restore the size.
        ImFont * font = g.IO.Fonts->Fonts[inFontIndex];
        font->Scale = std::max(inScale,0.00001f);
        PushFont(font);
        UIAddTextBB(inR, inText, inC, inAlign);
        PopFont();
        font->Scale = 1.0f; // reset font scale
    }

    void UIAddTextUnderline( const ImRect& inRect, const string& inText, int inC, int inFontIndex, vec2 inAlign, float inSize ) {
        vec2 textdim = UIGetTextDim( inText, inFontIndex );
        vec2 pos = { inRect.Min.x, inRect.Min.y };
        if (inAlign.x > 0.0f) { pos.x = std::max(pos.x, pos.x + (inRect.GetWidth()  - textdim.x) * inAlign.x); }
        if (inAlign.y > 0.0f) { pos.y = std::max(pos.y, pos.y + (inRect.GetHeight() - textdim.y) * inAlign.y); }
        UIAddGlobalLine( pos.x-1, pos.y+textdim.y, pos.x-1+textdim.x, pos.y+textdim.y, inC, inSize);        // x pos seems to be shifted by 1 pixel
    }

    void UIAddTextUnderline( const ImRect& inRect, const string& inText, int inC, int inFontIndex, vec2 inAlign, float inSize, float inScale ) {
        vec2 textdim = UIGetTextDim( inText, inFontIndex, inScale );
        vec2 pos = { inRect.Min.x, inRect.Min.y };
        if (inAlign.x > 0.0f) { pos.x = std::max(pos.x, pos.x + (inRect.GetWidth()  - textdim.x) * inAlign.x); }
        if (inAlign.y > 0.0f) { pos.y = std::max(pos.y, pos.y + (inRect.GetHeight() - textdim.y) * inAlign.y); }
        UIAddGlobalLine( pos.x-1, pos.y+textdim.y, pos.x-1+textdim.x, pos.y+textdim.y, inC, inSize);        // x pos seems to be shifted by 1 pixel
    }

    void UIAddTextUnderlineAngled( const ImRect& inRect, const string& inText, int inC, int inFontIndex, vec2 inAlign, float inSize, float inScale, vec2 inRPoint, float inAng ) {
        vec2 textdim = UIGetTextDim( inText, inFontIndex, inScale );
        vec2 pos = { inRect.Min.x, inRect.Min.y };
        if (inAlign.x > 0.0f) { pos.x = std::max(pos.x, pos.x + (inRect.GetWidth()  - textdim.x) * inAlign.x); }
        if (inAlign.y > 0.0f) { pos.y = std::max(pos.y, pos.y + (inRect.GetHeight() - textdim.y) * inAlign.y); }
        UIAddGlobalLineAngled( pos.x-1, pos.y+textdim.y, pos.x-1+textdim.x, pos.y+textdim.y, inC, inSize, inRPoint, inAng);        // x pos seems to be shifted by 1 pixel
    }

    void UIAddAlphaTextBB(ImRect inRect, const string& inText, int inC, float inAlpha, vec2 inAlign) {
        size_t text_size = inText.length();
        if (text_size == 0) { return; }
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        const char* text_buf = inText.c_str();
        const char* value_text_begin = &g.TempBuffer[0];
        ImVec2 min = ImVec2(inRect.Min.x, inRect.Min.y);
        ImVec2 max = ImVec2(min.x + inRect.GetWidth(), min.y + inRect.GetHeight());
        ImU32 Tclr = UILib::ClWithAlpha( inC, inAlpha );
        PushStyleColor(ImGuiCol_Text, Tclr);
        // NOTE: ImGui::RenderTextClipped will hide anything after a '##' string, so use ImGui::RenderTextClippedEx instead.
        RenderTextClippedEx(window->DrawList, min, max, text_buf, text_buf + text_size, NULL, ImVec2((float)inAlign[0], inAlign[1]));
        PopStyleColor(1);
    }

    void UIAddAlphaTextBBAngle(ImRect inR, const string& inText, int inC, float inAlpha, vec2 inAlign, vec2 inRPoint, float rad) {
        // Text rotates clockwise
        size_t text_size = inText.length();
        if (text_size == 0) { return; }
        int rotation_start_index = ImGui::GetWindowDrawList()->VtxBuffer.Size;
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        const char* text_buf = inText.c_str();
        const char* value_text_begin = &g.TempBuffer[0];
        ImU32 Tclr = UILib::ClWithAlpha( inC, inAlpha );
        PushStyleColor(ImGuiCol_Text, Tclr);
        // NOTE: ImGui::RenderTextClipped will hide anything after a '##' string, so use ImGui::RenderTextClippedEx instead.
        RenderTextClippedEx(window->DrawList, inR.Min, inR.Max, text_buf, text_buf + text_size, NULL, ImVec2((float)inAlign[0], inAlign[1]));
        auto& buf = ImGui::GetWindowDrawList()->VtxBuffer;
        float s = sin(rad), c = cos(rad);
        for (int i = rotation_start_index; i < buf.Size; i++) {
            buf[i].pos = UIRotate( buf[i].pos - inRPoint, c, s ) + inRPoint;
        }
        PopStyleColor(1);
    }    

    void UIAddAlphaTextWithFontBB(const ImRect& inRect, const string& inText, int inC, float inAlpha, int inFontIndex, vec2 inAlign) {
        ImGuiContext& g = *GImGui;
        PushFont(g.IO.Fonts->Fonts[inFontIndex]);
        UIAddAlphaTextBB(inRect, inText, inC, inAlpha, inAlign);
        PopFont();
    }

    void UIAddAlphaTextWithFontBB(const ImRect& inRect, const string& inText, int inC, float inAlpha, int inFontIndex, vec2 inAlign, float inScale) {
        ImGuiContext& g = *GImGui;
        // https://github.com/ocornut/imgui/issues/1018
        // To set scale of ImFont*, change the font->Scale 
        // value and then call PushFont() on this font. After you call PopFont you want to restore the size.
        ImFont * font = g.IO.Fonts->Fonts[inFontIndex];
        font->Scale = std::max(inScale,0.00001f);
        PushFont(font);
        UIAddAlphaTextBB(inRect, inText, inC, inAlpha, inAlign);
        PopFont();
        font->Scale = 1.0f; // reset font scale
    }

    void UIAddAlphaTextWithFontBBAngle(const ImRect& inR, const string& inText, int inC, float inAlpha, int inFontIndex, vec2 inAlign, float inScale, vec2 inRPoint, float inAng) {
        ImGuiContext& g = *GImGui;
        ImFont * font = g.IO.Fonts->Fonts[inFontIndex];
        font->Scale = std::max(inScale,0.00001f);
        PushFont(font);
        UIAddAlphaTextBBAngle(inR,inText,inC,inAlpha,inAlign,inRPoint,inAng);
        PopFont();
        font->Scale = 1.0f; // reset font scale
    }

    void UIAddAlphaTextUnderline(const ImRect& inRect, const string& inText, int inC, float inAlpha, int inFontIndex, vec2 inAlign, float inSize) {
        vec2 textdim = UIGetTextDim( inText, inFontIndex );
        vec2 pos = { inRect.Min.x, inRect.Min.y };
        if (inAlign.x > 0.0f) { pos.x = std::max(pos.x, pos.x + (inRect.GetWidth()  - textdim.x) * inAlign.x); }
        if (inAlign.y > 0.0f) { pos.y = std::max(pos.y, pos.y + (inRect.GetHeight() - textdim.y) * inAlign.y); }
        UIAddGlobalLineAlpha( pos.x-1, pos.y+textdim.y, pos.x-1+textdim.x, pos.y+textdim.y, inC, inSize, inAlpha);        // x pos seems to be shifted by 1 pixel
    }

    void UIAddAlphaTextUnderline(const ImRect& inRect, const string& inText, int inC, float inAlpha, int inFontIndex, vec2 inAlign, float inSize, float inScale) {
        vec2 textdim = UIGetTextDim( inText, inFontIndex, inScale );
        vec2 pos = { inRect.Min.x, inRect.Min.y };
        if (inAlign.x > 0.0f) { pos.x = std::max(pos.x, pos.x + (inRect.GetWidth()  - textdim.x) * inAlign.x); }
        if (inAlign.y > 0.0f) { pos.y = std::max(pos.y, pos.y + (inRect.GetHeight() - textdim.y) * inAlign.y); }
        UIAddGlobalLineAlpha( pos.x-1, pos.y+textdim.y, pos.x-1+textdim.x, pos.y+textdim.y, inC, inSize, inAlpha);        // x pos seems to be shifted by 1 pixel
    }

    void UIAddAlphaTextUnderlineAngled( const ImRect& inRect, const string& inText, int inC,float inAlpha,  int inFontIndex, vec2 inAlign, float inSize, float inScale, vec2 inRPoint, float inAng ) {
        vec2 textdim = UIGetTextDim( inText, inFontIndex, inScale );
        vec2 pos = { inRect.Min.x, inRect.Min.y };
        if (inAlign.x > 0.0f) { pos.x = std::max(pos.x, pos.x + (inRect.GetWidth()  - textdim.x) * inAlign.x); }
        if (inAlign.y > 0.0f) { pos.y = std::max(pos.y, pos.y + (inRect.GetHeight() - textdim.y) * inAlign.y); }
        UIAddGlobalLineAngled( pos.x-1, pos.y+textdim.y, pos.x-1+textdim.x, pos.y+textdim.y, inC, inSize, inRPoint, inAng);        // x pos seems to be shifted by 1 pixel
    }


    // --------------------------------------------------------------------- //
    // INPUT TEXT                                                            //
    // --------------------------------------------------------------------- //

    void UIInputText(float inX, float inY, float inW, float inH, string inText, int inC, int inFontIndex, vec2 inAlign, float inYShift, int &inStartI, int &inEndI, int &inDrag, bool &inFocused, int &inChange, float inScale){
        
        ImFont* font = GImGui->IO.Fonts->Fonts[inFontIndex];
        char* c = (char *)inText.data();

        float xPos = inX;vector<float> charXList = {inX};
        while( *c != 0){const float charWidth = font->GetCharAdvance((ImWchar)*c);xPos += charWidth;charXList.push_back(xPos);++c;}

        float textW = charXList[charXList.size()-1] - inX;

        ImRect textBB = GetGlobalBB(inX, inY, textW, inH);
        textBB.Expand(1);
        const bool overText = GetOverState(textBB);
        ImRect emptyBB = GetGlobalBB(inX+textW, inY, inW-textW, inH);
        const bool overEmpty = GetOverState(emptyBB);
        
        if( !MouseDown() ){
            inDrag = 0;if(inEndI == inStartI){inEndI = -1;}
            if(inEndI != -1){
                if(inStartI > inEndI){int sI = inStartI;inStartI = inEndI;inEndI = sI;}
            }
        }
        else if( inDrag == 1) {
            float lDist;float mx = GetLocalMousePos().x;float closeDist = 100000;
            for(int i=0;i<charXList.size();i++){
                lDist = abs(charXList[i]-mx);
                if(lDist < closeDist){closeDist = lDist;inEndI = i;}
            }
        }
        else if ( DragStart() ) {
             if( overEmpty ){
                inFocused=true; inDrag=1;
                if(UILibIS->Shift) { inEndI = (int)charXList.size()-1;  }
                else { inStartI = (int)charXList.size()-1; inEndI = -1; }
            }
            else if( overText ){
                inFocused=true; inDrag=1;
                float lDist;float mx = GetLocalMousePos().x;float closeDist = 100000;
                if(UILibIS->Shift) { // with shift down you reuse the old start point and define a new endpoint
                    for(int i=0;i<charXList.size();i++){
                        lDist = abs(charXList[i]-mx);
                        if(lDist < closeDist){closeDist = lDist;inEndI = i;}
                    }
                    
                }
                else{
                    for(int i=0;i<charXList.size();i++){
                        lDist = abs(charXList[i]-mx);
                        if(lDist < closeDist){closeDist = lDist;inStartI = i;}
                    }
                    inEndI = -1;
                }
            }
            else { inFocused=false; }
        }
        
        
        static auto lastTime = std::chrono::high_resolution_clock::now();
        if(inChange == 1){lastTime = std::chrono::high_resolution_clock::now();}

        if(inFocused){
            if(inEndI != -1){
                int sI = inStartI;
                int eI = inEndI;
                if(inStartI > inEndI){sI = inEndI;eI = inStartI;}
                float bx = charXList[sI];
                float bw = charXList[eI] - bx;
                UIAddRect(bx, inY, bw, inH, 0x6CCAFE, true, 1);
            }
            else if(inStartI != -1){
                
                auto currentTime = std::chrono::high_resolution_clock::now();
                float timeSpan = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
                int timeCounts = (int)floor(timeSpan / 0.65f);
                bool blink = false;
                if(inChange){blink = true;}
                else if(timeCounts % 2 == 0){blink = true;}
                if(inStartI > charXList.size()-1){inStartI = (int)charXList.size()-1;}
                if(blink){UIAddRect(charXList[inStartI]-.75f, inY, 1.5, inH, 0x228BC5, true, 1);}

            }
        }
        UIAddTextWithFont(inX, inY + inYShift, inW, inH, inText, inC, inFontIndex, inAlign, inScale);

    }
    
    void UIDrawTextInput() {

        // Exit if the input has already been processed this frame.
        // This prevents aabbcc typing errors due to UIDrawTextInput
        // being called twice in a frame.
        if ( UILibCTX->InputChange != 0 ) { return; }

        UILibCTX->InputText = UIGetKeyInput(UILibCTX->InputText, UILibCTX->InputStartI, UILibCTX->InputEndI, UILibCTX->InputChange, UILibCTX->InputTextFocus);
        if(UILibIS->DoubleClick){
            UISelectBetweenDelim(UILibCTX->InputText, UILibCTX->InputStartI, UILibCTX->InputEndI);
            UILibIS->DoubleClick = false;
        }
        float iX = UILibCTX->InputBoxRect.Min.x;
        float iY = UILibCTX->InputBoxRect.Min.y;
        float iW = UILibCTX->InputBoxRect.GetWidth();
        float iH = UILibCTX->InputBoxRect.GetHeight();
        UIInputText( iX, iY, iW, iH, UILibCTX->InputText, UILibCTX->InputTextColor, UILibCTX->InputTextFont, {0, .5}, -1, UILibCTX->InputStartI, UILibCTX->InputEndI, UILibCTX->InputDragging, UILibCTX->InputTextFocus,  UILibCTX->InputChange, UILibCTX->InputTextScale);
        
    }

    void UIDrawPasswordInput() {

        // Exit if the input has already been processed this frame.
        // This prevents aabbcc typing errors due to UIDrawPasswordInput
        // being called twice in a frame.
        if ( UILibCTX->InputChange != 0 ) { return; }

        UILibCTX->InputText = UIGetKeyInput(UILibCTX->InputText, UILibCTX->InputStartI, UILibCTX->InputEndI, UILibCTX->InputChange, UILibCTX->InputTextFocus);
        if(UILibIS->DoubleClick){
            UISelectBetweenDelim(UILibCTX->InputText, UILibCTX->InputStartI, UILibCTX->InputEndI);
            UILibIS->DoubleClick = false;
        }
        std::string output = UILibCTX->InputText;
        for (int i = 0; i < output.length(); ++i) {
            output[i] = '*';
        }
        float iX = UILibCTX->InputBoxRect.Min.x;
        float iY = UILibCTX->InputBoxRect.Min.y;
        float iW = UILibCTX->InputBoxRect.GetWidth();
        float iH = UILibCTX->InputBoxRect.GetHeight();
        UIInputText( iX, iY, iW, iH, output, UILibCTX->InputTextColor, UILibCTX->InputTextFont, {0, .5}, -1, UILibCTX->InputStartI, UILibCTX->InputEndI, UILibCTX->InputDragging, UILibCTX->InputTextFocus,  UILibCTX->InputChange, UILibCTX->InputTextScale);
        
    }
    
// --------------------------------------------------------------------- //
// MULTI LINE TEXT                                                       //
// --------------------------------------------------------------------- //

    void UIMultiLineTextBB( const ImRect & inRect, const string & inText, int inC, float inAlpha, int inFontIndex, vec2 inAlign, bool inULine, float inScale ) {
        if ( inText.empty() ) { return; }

        ImFont* font = GImGui->IO.Fonts->Fonts[inFontIndex];
        font->Scale = std::max( inScale, 0.00001f);

        float LineH = font->FontSize*font->Scale;
        vector<float> LineWidths = {};  // line widths
        float currW = 0;                // current width

        // Scan Pass
        const char * c = inText.c_str();
        while ( *c != 0 ) {
            if ( *c == '\n' ) {
                LineWidths.push_back(currW);
                currW = 0;
            }
            else if( *c == '\r' ){
                if( c[1] == '\n' ) { c++; }
                LineWidths.push_back(currW);
                currW = 0;
            }                
            else {
                currW += font->GetCharAdvance((ImWchar)(*c))*font->Scale;
            }
            c++;
        }
        if ( currW > 0 ) {
            LineWidths.push_back(currW);
        }

        if ( LineWidths.empty() ) { return; } // nothing to draw

        // Draw Pass
        float rectW = inRect.GetWidth();
        float xPos = inRect.Min.x + ( rectW - LineWidths[0] )*inAlign.x;
        float yPos = inRect.Min.y + ( inRect.GetHeight() - LineWidths.size()*LineH )*inAlign.y;
        ImU32 clr = UILib::ClWithAlpha( inC, inAlpha );
        
        c = inText.c_str();
        int lineI = 0;
        float lineY = 0;
        const float lineT = LineH*0.059f;

        while ( *c != 0 ) {
            if ( *c == '\n' ) { 
                lineI++; if ( lineI >= LineWidths.size() ) { break; }
                xPos = inRect.Min.x + ( rectW - LineWidths[lineI] )*inAlign.x;
                yPos += LineH;
            }
            else if( *c == '\r' ){
                if( c[1] == '\n' ) { c++; }
                lineI++; if ( lineI >= LineWidths.size() ) { break; }
                xPos = inRect.Min.x + ( rectW - LineWidths[lineI] )*inAlign.x;
                yPos += LineH;
            }                
            else {
                currW = font->GetCharAdvance((ImWchar)(*c))*font->Scale;
                UIRenderChar( (ImWchar)(*c), { xPos, yPos }, clr, font );
                if ( inULine ) {
                    lineY = yPos + (LineH * .94f);
                    UIAddGlobalLineAlpha(xPos, lineY, xPos+currW, lineY, clr, lineT, inAlpha);
                }
                xPos += currW;
            }
            c++;
        }    
        font->Scale = 1.0f; // reset font scale

    }

    void UIMultiLineTextBBAngled( const ImRect & inRect, const string & inText, int inC, float inAlpha, int inFontIndex, vec2 inAlign, bool inULine, float inScale, vec2 inRPoint, float rad ) {
        // Draws Multilined rotated clockwise
        if ( inText.empty() ) { return; }
        int rotation_start_index = ImGui::GetWindowDrawList()->VtxBuffer.Size;
        UIMultiLineTextBB( inRect, inText, inC, inAlpha, inFontIndex, inAlign, inULine, inScale );
        auto& buf = ImGui::GetWindowDrawList()->VtxBuffer;
        float s = sin(rad), c = cos(rad);
        for (int i = rotation_start_index; i < buf.Size; i++) {
            buf[i].pos = UIRotate( buf[i].pos - inRPoint, c, s ) + inRPoint;
        }
    }
    
    pair<int,int> GetTextSelRange(TextMap &inTM, int inSIndex, int inEIndex){ // returns range in the space of line numbers
        int syLine = -1; int eyLine = -1;
        for(int i=0;i<inTM.YRange.size();i++){
            if(inSIndex >= inTM.YRange[i][0] && inSIndex <= inTM.YRange[i][1]){syLine = i;}
            if(inEIndex >= inTM.YRange[i][0] && inEIndex <= inTM.YRange[i][1]){eyLine = i;}
            if(syLine != -1 && eyLine != -1){break;}
        }
        pair<int,int> range = pair(syLine, eyLine);
        return range;

    }
    list<int> GetSelectedLineStarts(TextMap &inTM){
        list<int> ltList;
        for(int i=0;i<inTM.SList.size();i++){
            pair<int,int> sRange = GetTextSelRange(inTM, inTM.SList[i].S, inTM.SList[i].E);
            for(int j=sRange.first;j<=sRange.second;j++){
                ltList.push_back ( inTM.YRange[j][0] );
            }
        }
        ltList.unique();
        return ltList;
    }
    string GetPythonIndent(int inCharIndex, TextMap &inTM, int &inInsertI){
        int yIndex = -1;vector<CharCell> &CL = inTM.CList;
        for(int i=0;i<inTM.YRange.size();i++){
            if(inCharIndex >= inTM.YRange[i][0] && inCharIndex <= inTM.YRange[i][1]){yIndex=i;inInsertI=inCharIndex;break;}
        }
        string front="";string back="";int flipC = 0; // break the line into front and back
        if(inTM.YRange[yIndex][0]==inTM.YRange[yIndex][1]){return "";}
        for(int i=inTM.YRange[yIndex][0];i<inCharIndex;i++){
            if(flipC==0){if(!(CL[i].Char==32||CL[i].Char=='\t')){flipC++;back+=CL[i].Char;}else{front+=CL[i].Char;}}
            else{back+=CL[i].Char;}
        }
        if(back.size()==0){return front;}
        // need to get the end char not including space - look for ':' & '('
        unsigned char endChar;size_t i=back.size();while(i-->0){if( !(back[i]==' '||back[i]=='\t')){endChar = back[i]; break;}}
        int nbIndent = 0;
        if(endChar=='('){nbIndent=1;}
        else if(endChar==':'){
            string t = "";// get the first token 
            for(size_t i=0;i<back.size();i++){if(back[i]==' '||back[i]=='\t'||back[i]=='('||back[i]=='{'){break;}t+=back[i];}
            if(t=="class"||t=="for"||t=="while"||t=="def"||t=="elif"||t=="else"||t=="except"||t=="finally"||t=="if"||t=="try"||t=="with"){nbIndent=1;}
        }
        for(i=0;i<nbIndent;i++){front+="\t";}
        return front;
    }
    void HighlightBrackets(TextMap &inTM){
        // syntax highlights a TextMap to highlight brackets
        // don't highlight any selections that are not single carets
        vector<CharCell> &CL = inTM.CList;
        if(CL.size()==0){return;}
        for(int i=0;i<inTM.SList.size();i++){
            if(inTM.SList[i].D != inTM.SList[i].R){continue;}
            int ci = inTM.SList[i].D;
            int forI = -1;int foc = 0 ;int forC;
            int backI = -1;int boc = 0;int backC;
            vector<int> depthC = {0,0,0}; // depth for '(', '[', '{'
            int maxJ = (int)CL.size()-1;
            if(ci > maxJ){ci = maxJ;}
            for(int j=ci;j-->0;){ // crawl back - counting boc:  back opposite count
                unsigned char c = CL[j].Char;
                if(c=='('){if(depthC[0]==0){backI=j;backC=c;break;}depthC[0]--;}
                if(c==')'){depthC[0]++;boc++;}
                if(c=='['){if(depthC[1]==0){backI=j;backC=c;break;}depthC[1]--;}
                if(c==']'){depthC[1]++;boc++;}
                if(c=='{'){if(depthC[2]==0){backI=j;backC=c;break;}depthC[2]--;}
                if(c=='}'){depthC[2]++;boc++;}
            }
            for(int j=ci;j<CL.size();j++){ // crawl back - counting foc:  forward opposite count
                unsigned char c = CL[j].Char;
                if(c==')'){if(depthC[0]==0){forI=j;forC=c;break;}depthC[0]--;}
                if(c=='('){depthC[0]++;foc++;}
                if(c==']'){if(depthC[1]==0){forI=j;forC=c;break;}depthC[1]--;}
                if(c=='['){depthC[1]++;foc++;}
                if(c=='}'){if(depthC[2]==0){forI=j;forC=c;break;}depthC[2]--;}
                if(c=='{'){depthC[2]++;foc++;}
            }
            if(boc <= foc){  // backwards crawl is closest bracket - crawl forward for match
                if(backI==-1){return;}
                CL[backI].Syntax = 11; // bracket
                int matchC = ')';if(backC=='['){matchC=']';}else if(backC=='{'){matchC='}';}
                int depth = 0;
                for(int j=ci;j<CL.size();j++){
                    unsigned char c = CL[j].Char;
                    if(c==backC){depth--;}
                    if(c==matchC){if(depth == 0){CL[j].Syntax = 11; return;}else{depth++;}}
                }
            }
            else{ // forwards crawl is closest bracket - crawl backwards for match
                if(forI==-1){return;}
                CL[forI].Syntax = 11; // bracket
                int matchC = '(';if(forC==']'){matchC='[';}else if(forC=='}'){matchC='{';}
                int depth = 0;
                for(int j=ci;j-->0;){
                        unsigned char c = CL[j].Char;
                        if(c==forC){depth--;}
                        if(c==matchC){if(depth == 0){CL[j].Syntax = 11; return;}else{depth++;}}
                }
            }
        }
    }
    string UITextFromTM(TextMap &inTM, string &inInsertS, string &inText, int inNlType){

        // cout << "GOT TO UITextFromTM ---> inInsertS: " << inInsertS << "\n";

        int ins = (int)inInsertS.size();
        if(inTM.CList.size() == 0){
            if(inText==""){
                if(ins > 0){inTM.SList = {{ ins,ins,ins,ins }};} // paste in blank line
                else{inTM.SList = {{ 1,1,1,1 }};} // character typed in blank
            }
            if(inInsertS==""){return inText;}
            return inInsertS;
        }

        string newText = ""; // int insertShift = (int)inInsertS.size();
        string newLine = "\r\n";if(inNlType== 0){newLine="\n";}
        vector<int> selMap = {}; // make mapping from the old index into the new index.   Given a index in old TM, what is it in the new TM?
        int count = 0; // index in new mapping
        map<int,string> multiInsert = {};

        if(inTM.LineTransfom > 0){  //  0=none, 1=tab forward , 2=tab back, 3=flip comment, 4=python indent
            list<int> ltList = GetSelectedLineStarts(inTM);
            if(inTM.LineTransfom==1){inInsertS = '\t';for (auto lt : ltList){inTM.CList[lt].Op = 1;}}
            else if(inTM.LineTransfom==2){
                for(int i:ltList){  // to tab back, example removal scenarios: 3 SP & 1 \t, 2 SP & \t, 1 SP & \t, \t
                    vector<CharCell> &CL = inTM.CList;int maxI = (int)CL.size()-1;int delL = 0;
                    if(i<maxI-4){if(CL[i].Char==32 && CL[i+1].Char==32 && CL[i+2].Char==32){if(CL[i+3].Char==9||CL[i+3].Char==32){delL=4;goto setDelete;}}}
                    if(i<maxI-3){if(CL[i].Char==32 && CL[i+1].Char==32){if(CL[i+2].Char==9||CL[i+2].Char==32){delL=3;goto setDelete;}}}
                    if(i<maxI-2){if(CL[i].Char==32){if(CL[i+1].Char==9||CL[i+1].Char==32){delL=2;goto setDelete;}}}
                    if(CL[i].Char==9||CL[i].Char==32){delL=1;}
                    setDelete:
                    for(int j=0;j<delL;j++){inTM.CList[i+j].Del = 1;}
                }
            }
            else if(inTM.LineTransfom==3){ // add or remove python comments on each line
                vector<CharCell> &CL = inTM.CList;
                list<int> firstCs = {};  // a list of the first characters that are not space or tab
                for(int i:ltList){int j=i;while(true){if(!(CL[j].Char==32||CL[j].Char=='\t')){break;}j++;}firstCs.push_back(j);}
                bool addOp = false;
                for(int i:firstCs){ if(CL[i].Char!='#'){addOp = true;break;}}
                if(addOp){for(int i:firstCs){ CL[i].Op = 1;}inInsertS = "# ";}
                else{
                    int maxI = (int)CL.size()-1;
                    for(int i:firstCs){CL[i].Del = 1;if(i<maxI-2){if( CL[i+1].Char==32){CL[i+1].Del = 1;}}}  
                }
            }
            else if(inTM.LineTransfom==5){  // python indenting
                vector<CharCell> &CL = inTM.CList;
                for(int i=0;i<inTM.SList.size();i++){
                        int insertC;string indentStr = GetPythonIndent(inTM.SList[i].R, inTM, insertC);
                        multiInsert.insert( {insertC, indentStr} );
                }
            }
        }

        int insertL = (int)inInsertS.size();
        for(int i=0;i<inTM.CList.size();i++){
            
            selMap.push_back(count);
            if(inTM.CList[i].Op == 1){ // insert string before
                //cout <<"----------- insert string before\n";
                newText += inInsertS;//count++;
                count+= insertL;
                if(inTM.LineTransfom==5){ // python indents
                        auto it = multiInsert.find(i);
                        if (it != multiInsert.end()) {newText += it->second;inTM.Shift += (int)it->second.length();}
                }
                if(inTM.CList[i].Del == 1){} // Delete
                else if(inTM.CList[i].Type==1){newText += newLine;count++;}   // Type 1=newline
                else if(inTM.CList[i].Type==2){} // EOL
                else{newText += inTM.CList[i].Char;count++;}
            }
            else if(inTM.CList[i].Op == 2){// insert string after
                //cout <<"----------- insert string after\n";
                if(inTM.CList[i].Del == 1){} // Delete
                else if(inTM.CList[i].Type == 1){newText += newLine;count++;} // Type 1=newline
                else{newText += inTM.CList[i].Char;count++;}
                newText += inInsertS;//count++;
                count+= insertL;
                //selMap[selMap.size()-1] = count; // this doesn't help
            }
            else if(inTM.CList[i].Op == 3){// close bracket after
                //cout <<"----------- insert string after\n";
                if(inTM.CList[i].Del == 1){} // Delete
                else if(inTM.CList[i].Type == 1){newText += newLine;count++;} // Type 1=newline
                else{newText += inTM.CList[i].Char;count++;}
                if(inInsertS=="("){newText += ")";}
                else if(inInsertS=="["){newText += "]";}
                else if(inInsertS=="{"){newText += "}";}
                count+= insertL;
            }
            else{ // place character no insert
                if(inTM.CList[i].Del == 1){} // Delete
                else if(inTM.CList[i].Type == 2){break;}
                else if(inTM.CList[i].Type == 1){newText += newLine;count++;} 
                else{newText += inTM.CList[i].Char;count++;}
            }
        }
        
        // log sel map
        // for(int i=0;i<selMap.size();i++){cout<<selMap[i]<<" ";}cout << "\n";

        if(inTM.CList.size()==0){inTM.SList={};} // clear selection with empty text
        int newS;
        if(newText.size()==inInsertS.size()){ // special case for all characters text selected and replaced by a paste
            inTM.SList = {{ insertL,insertL,insertL,insertL }};
        }
        else if(inTM.LineTransfom == 0 || inTM.LineTransfom == 5){
            for(int i=0;i<inTM.SList.size();i++){
                if(inTM.SList[i].S < 0){inTM.SList[i].SetCaret(0);continue;}
                if(inTM.SList[i].S>(int)selMap.size()-1){inTM.SList[i].S=(int)selMap.size()-1;}
                if(inTM.SList[i].E < 0){inTM.SList[i].SetCaret(0);continue;}
                if(inTM.SList[i].E>(int)selMap.size()-1){inTM.SList[i].E=(int)selMap.size()-1;}

                newS = selMap[inTM.SList[i].E]; // used to be selMap[inTM.SList[i].S]; but with .E paste works better
                if(inTM.SList[i].Op==1){
                    newS--;
                    inTM.SList[i].Op=0;
                }
                if(ins == 0 || inTM.SList[i].S==inTM.SList[i].E){newS +=inTM.Shift;}

                inTM.SList[i].SetCaret(newS);
            }
        }
        else{
            for(int i=0;i<inTM.SList.size();i++){
                int newE;
                newS = selMap[inTM.SList[i].S];
                newE = selMap[inTM.SList[i].E];
                inTM.SList[i].S = newS;inTM.SList[i].E = newE;
                if(inTM.SList[i].RSide){inTM.SList[i].D = newS;inTM.SList[i].R = newE;}
                else{inTM.SList[i].R = newS;inTM.SList[i].D = newE;}
            }
        }
        
        //RecalibrateSel(inTM);
        
        return newText;
        
    }
    void RecalibrateSel(TextMap &inTM){
        
        // ---- for now not using this it doesn't really seem to help ----
        // it's a kind of guard code but doesn't seem to be very useful...

        // recalibrate the text selection so that it fits within the current TMap
        // this is important to prevent being out of range after trasnformations like paste
        
        //int maxI = (int)inTM.CList.size() -1;
        int maxI = (int)inTM.CList.size();

        for(int i=0;i<inTM.SList.size();i++){
            if(inTM.SList[i].S < 0){inTM.SList[i].S=0;}
            if(inTM.SList[i].D < 0){inTM.SList[i].D=0;}
            if(inTM.SList[i].E > maxI){inTM.SList[i].E=maxI;}
            if(inTM.SList[i].R > maxI){inTM.SList[i].R=maxI;}
        }
    }
    void UITermOccurrences(TextMap &inTM, string inTerm){
        
        // for now this is case-insensitive but would be good to add options for case-sensitive and wildcards
        // highlights all matches to a term in inTM.TList
        // If you want to add multiple terms add multiple times
        float xShift = inTM.Rect.Min.x; float yShift = inTM.Rect.Min.y;  // shift caused by scrolling
        vector<CharCell>&CL=inTM.CList;
        // make both the text and the term lower case
        string content = "";string term = "";char c;
        for(int i=0;i<CL.size();i++){c=CL[i].Char;if(c>64&&c<91){content+=(char)(c+32);}else{content+=c;}} 
        for(int i=0;i<inTerm.size();i++){if(inTerm[i]>64&&inTerm[i]<91){term+=(char)(inTerm[i]+32);}else{term+=inTerm[i];}}

        vector<int> matches={};size_t len=term.size();int j;
        for(int i=0;i<(int)content.size();i++){
            if(content[i]==term[0]){for(j=1;j<len;j++){if(content[i+j]!=term[j]){goto endScan;}}matches.push_back(i);}endScan:;
        }
        if(matches.size()>0){
            int si=matches[0];int ei=si+(int)inTerm.size();
            float rectW=0;for(int i=si;i<ei;i++){rectW += CL[i].w;}
            int borderC = UILibCTX->EditorOptions[ed_term].Fill;
            for(int i=0;i<matches.size();i++){
                int mi =matches[i] ;
                ImRect bRect = GetBB(CL[mi].x+xShift, CL[mi].y+yShift, rectW, inTM.LineH);
                UIDrawAlphaBB(bRect, borderC, .25f);
            }
        }
    }
    void UIHighlightOccurrences(TextMap &inTM){
        // highlight selected text occurances
        float xShift = inTM.Rect.Min.x;  // shift caused by scrolling
        float yShift = inTM.Rect.Min.y;  // shift caused by scrolling
        if(inTM.SList.size()!=1){return;}
        if(inTM.SList[0].S==inTM.SList[0].E){return;}
        vector<CharCell>&CL=inTM.CList;string selW = "";
        vector<int> LC={};int c;
        for(int i=0;i<CL.size();i++){c=CL[i].Char;if(c>64&&c<91){LC.push_back(c+32);}else{LC.push_back(c);}} // make lower case
        int si = inTM.SList[0].S;int ei = inTM.SList[0].E;
        if(ei>(int)LC.size()-1){ei=(int)LC.size()-1;}
        for(int i=si;i<ei;i++){if(CL[i].Type==1){return;}selW+=LC[i];}
        
        if(selW.size() == 1 && selW[0] == ' '){return;}
        vector<int> matches={};size_t len=selW.size();int j;
        for(int i=0;i<LC.size();i++){
            if(LC[i]==selW[0]&&i!=si){for(j=1;j<len;j++){if(LC[i+j]!=selW[j]){goto endScan;}}matches.push_back(i);}endScan:;
        }
        if(matches.size()>0){
            float rectW=0;for(int i=si;i<ei;i++){rectW += CL[i].w;}
            int borderC = UILibCTX->EditorOptions[ed_instance_active].Fill;
            for(int i=0;i<matches.size();i++){
                int mi =matches[i] ;
                ImRect bRect = GetBB(CL[mi].x+xShift, CL[mi].y+yShift, rectW, inTM.LineH);
                UIDrawBBOutline(bRect, borderC, 1);
            }
        }
    }
    
    void UIDrawTM(TextMap &inTM){

        // Plain editor text box
        // render text map - both selection and characters

        if(inTM.CList.size()!=0){

            float xShift = inTM.Rect.Min.x;  // shift caused by scrolling
            float yShift = inTM.Rect.Min.y;  // shift caused by scrolling
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            
            UIDrawSelection(inTM);

            ImFont* font=GImGui->IO.Fonts->Fonts[ inTM.Font[0] ];
            font->Scale = std::max(inTM.Scale,0.00001f);

            // draw characters
            for(int i=0;i<inTM.CList.size()-1;i++){ // step through one character at a time
                UIRenderChar(drawList, inTM.LineH, {inTM.CList[i].x+xShift, inTM.CList[i].y+yShift}, UILibCTX->SyntaxOptions[syn_uncast].FontC, inTM.CList[i].Char, font );
            }

            font->Scale = 1.0f; // reset font scale

        }

        if(inTM.Focus==0){ return;}
        UIDrawCaret(inTM);

    }

    void UIDrawSyntaxColorTM(TextMap &inTM){

        // Draws the TextMap where the color is set by the .Clist[i].Syntax read as a per character color
        // other formating (outside of color) is set by the .Format parameter
        
        float xShift = inTM.Rect.Min.x;  // shift caused by scrolling
        float yShift = inTM.Rect.Min.y;  // shift caused by scrolling
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        int maxCl = (int)inTM.CList.size() -1;
        if(inTM.CList.size()!=0){
            
            UIDrawSelection(inTM);

            int fontS = 0;  // font state: 0=regular, 1=bold, 2=italic
            
            int fIndex = inTM.Font[0];  // [default, bold, italic, bold italic]
            if(inTM.Format.Bold){if(inTM.Format.Italic){fIndex = inTM.Font[3];}else{fIndex = inTM.Font[1];}}
            else if(inTM.Format.Italic){fIndex = inTM.Font[2];}
            ImFont* font=GImGui->IO.Fonts->Fonts[fIndex];
            
            for(int i=0;i<inTM.CList.size()-1;i++){ // step through one character at a time
                int clr =inTM.CList[i].Syntax;
                UIRenderChar(drawList, inTM.LineH, {inTM.CList[i].x+xShift, inTM.CList[i].y+yShift}, clr, inTM.CList[i].Char, font);
                if(inTM.Format.ULine){
                    float lineY = inTM.CList[i].y + (inTM.LineH * .94f) + yShift;
                    float lineX = inTM.CList[i].x+xShift;
                    UIAddGlobalLine(lineX, lineY, lineX+inTM.CList[i].w, lineY, clr, inTM.LineH*0.059f);
                }
            }
        }

        if(inTM.Focus==0){ return;}
        UIDrawCaret(inTM);

    }
    void UIDrawFormatTM(TextMap &inTM){
        
        // Draws the TextMap using TextMap.Format parameters
        // Used for standard simple text drawing of blocks of text with alignment, bold/italic, single color for entire block, ect
        
        if(inTM.CList.size()!=0){

            float xShift = inTM.Rect.Min.x;  // shift caused by scrolling
            float yShift = inTM.Rect.Min.y;  // shift caused by scrolling
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            
            UIDrawSelection(inTM);

            int fontS = 0;  // font state: 0=regular, 1=bold, 2=italic
            int clr = inTM.Format.TextColor;
            int fIndex = inTM.Font[0];  // [default, bold, italic, bold italic]
            if(inTM.Format.Bold){if(inTM.Format.Italic){fIndex = inTM.Font[3];}else{fIndex = inTM.Font[1];}}
            else if(inTM.Format.Italic){fIndex = inTM.Font[2];}
            ImFont* font=GImGui->IO.Fonts->Fonts[fIndex];

            for(int i=0;i<inTM.CList.size()-1;i++){ // step through one character at a time
                UIRenderChar(drawList, inTM.LineH, {inTM.CList[i].x+xShift, inTM.CList[i].y+yShift}, clr, inTM.CList[i].Char, font);
                if(inTM.Format.ULine){
                    float lineY = inTM.CList[i].y + (inTM.LineH * .94f) + yShift;
                    float lineX = inTM.CList[i].x+xShift;
                    UIAddGlobalLine(lineX, lineY, lineX+inTM.CList[i].w, lineY, clr, inTM.LineH*0.059f);
                }
            }
        }

        if(inTM.Focus==0){ return;}
        UIDrawCaret(inTM);
        
    }
    void UIDrawSelection(TextMap &inTM){ // Draw Selection in TextMap

        // draw selection rectangles
        float xShift = inTM.Rect.Min.x;  // shift caused by scrolling
        float yShift = inTM.Rect.Min.y;  // shift caused by scrolling
        CharCell *ch;int maxCl = (int)inTM.CList.size() -1;
        for(int i=0;i<inTM.SList.size();i++){
            if(inTM.SList[i].S != inTM.SList[i].E){
                if(inTM.SList[i].E > maxCl){inTM.SList[i].E = maxCl;}
                for(int j=inTM.SList[i].S;j<=inTM.SList[i].E-1;j++){
                    ch = &inTM.CList[j];
                    ImRect charBB = GetBB(ch->x+xShift, ch->y+yShift, ch->w, inTM.LineH);
                    if(inTM.Focus==0){
                        UIDrawAlphaBB(charBB, UILibCTX->EditorOptions[ed_highlight_inactive].Fill, 1.0f);
                    }
                    else{
                        UIDrawAlphaBB(charBB, UILibCTX->EditorOptions[ed_highlight_active].Fill, 1.0f);
                    }
                }
            }
        }

    }
    void UIDrawCaret(TextMap &inTM){ // Draw Blinking Caret in TextMap

        // draw selection caret
        float xShift = inTM.Rect.Min.x;  // shift caused by scrolling
        float yShift = inTM.Rect.Min.y;  // shift caused by scrolling
        CharCell *ch;int maxCl = (int)inTM.CList.size() -1;
        float caretW = inTM.CharW * 0.2f;ImRect caretBB;
        //static auto lastTime = std::chrono::high_resolution_clock::now();
        
        if(inTM.Change == 1 || inTM.SelChange==1 || inTM.LDrag>0 || inTM.MDrag>0){UILibCTX->LastBlinkTime = std::chrono::high_resolution_clock::now();}
        inTM.SelChange = 0; 

        if(inTM.SList.size() > 0){
            auto currentTime = std::chrono::high_resolution_clock::now();
            float timeSpan = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - UILibCTX->LastBlinkTime).count();
            int timeCounts = (int)floor(timeSpan / 0.6f);
            bool blink = false;
            if(inTM.Change){blink = true;}
            else if(timeCounts % 2 == 0){blink = true;}
            if(blink){
                if(inTM.CList.size()==0){
                    float x0 = inTM.XBorder+xShift;
                    if(inTM.Format.HAlign > .8){x0 = xShift + inTM.Rect.GetWidth() - inTM.XBorder;}
                    else if(inTM.Format.HAlign > .2){x0 = xShift + inTM.Rect.GetWidth() * 0.5f;}
                    float y0 = inTM.Rect.Min.y + inTM.YBorder;
                    if(inTM.Format.VAlign > .8){y0 = inTM.Rect.Max.y - inTM.LineH;}
                    else if(inTM.Format.VAlign > .2){y0 = inTM.Rect.Min.y + inTM.Rect.GetHeight() * .5f - inTM.LineH * .5f;}
                    caretBB = GetBB(x0, y0, caretW, inTM.LineH);
                    UIDrawBB(caretBB,UILibCTX->EditorOptions[ed_cursor_active].Fill);
                }
                else{
                    int maxCL = (int)inTM.CList.size()-1;
                    for(int i=0;i<inTM.SList.size();i++){
                        if(inTM.SList[i].S<0 || inTM.SList[i].E<0){ 
                            inTM.SList[i].SetCaret(0); 
                        }
                        else if(inTM.SList[i].R == inTM.SList[i].S){
                            if(inTM.SList[i].S > maxCL){inTM.SList[i].S = maxCL;}
                            ch = &inTM.CList[inTM.SList[i].S];
                            caretBB = GetBB(ch->x-1.5f+xShift, ch->y+yShift, caretW, inTM.LineH);
                        }
                        else{
                            if(inTM.SList[i].E>(int)inTM.CList.size()-1){inTM.SList[i].E=(int)inTM.CList.size()-1;}
                            ch = &inTM.CList[inTM.SList[i].E];
                            if(inTM.SList[i].RSide){  caretBB = GetBB(ch->x-1.5f +xShift, ch->y+yShift, caretW, inTM.LineH);} // on right side
                            else{  caretBB = GetBB(ch->x-1.5f +ch->w +xShift, ch->y+yShift, caretW, inTM.LineH);} // on left side
                        }
                        UIDrawBB(caretBB, UILibCTX->EditorOptions[ed_cursor_active].Fill );
                    }
                }
            }
        }

    }
    void UIDrawEditorTM(TextMap &inTM){
        
        float xShift = inTM.Rect.Min.x;  // shift caused by scrolling
        float yShift = inTM.Rect.Min.y;  // shift caused by scrolling
        // Draw Text Editor Box with Python Syntax colors
        ImDrawList* drawList = ImGui::GetWindowDrawList();CharCell *ch;int maxJ;
        if(inTM.CList.size()!=0){
            // draw selection rectangles
            for(int i=0;i<inTM.SList.size();i++){
                if(inTM.SList[i].S != inTM.SList[i].E){
                    maxJ = inTM.SList[i].E-1;if(maxJ>(int)inTM.CList.size()-1){maxJ=(int)inTM.CList.size()-1;}
                    for(int j=inTM.SList[i].S;j<=maxJ;j++){
                        ch = &inTM.CList[j];
                        ImRect charBB = GetBB(ch->x+xShift, ch->y+yShift, ch->w, inTM.LineH);
                        if(inTM.Focus==0){
                            UIDrawAlphaBB(charBB, UILibCTX->EditorOptions[ed_highlight_inactive].Fill, 1.0f);
                        }
                        else{
                            UIDrawAlphaBB(charBB, UILibCTX->EditorOptions[ed_highlight_active].Fill, 1.0f);
                        }
                    }
                }
            }
            int clr;int oldFontS = 0;
            UISetFont(inTM.Font[0]);
            int fontS = 0;  // font state: 0=regular, 1=bold, 2=italic
            for(int i=0;i<inTM.CList.size()-1;i++){ // step through one word at a time
                clr = UILibCTX->SyntaxOptions[ inTM.CList[i].Syntax ].FontC;
                fontS = UILibCTX->SyntaxOptions[ inTM.CList[i].Syntax ].FontS;
                if(fontS!=oldFontS){UISetFont(inTM.Font[fontS]);}
                
                //cout << "      -inTM.CList["<<i<<"].x: " << inTM.CList[i].x << "\n";
                UIRenderChar(drawList, inTM.LineH, {inTM.CList[i].x+xShift, inTM.CList[i].y+yShift}, clr, inTM.CList[i].Char);
                oldFontS = fontS;
                if(inTM.CList[i].Syntax==11){
                    int uLineC = UILibCTX->SyntaxOptions[ inTM.CList[i].Syntax ].Underline;
                    if(uLineC != -1){
                        float lineY = inTM.CList[i].y + inTM.LineH + yShift;
                        UIAddGlobalLine(inTM.CList[i].x+xShift, lineY, inTM.CList[i].x+inTM.CList[i].w, lineY, uLineC, 1);
                    }
                    int borderC = UILibCTX->SyntaxOptions[ inTM.CList[i].Syntax ].Border;
                    if(borderC != -1){
                        ImRect bRect = GetGlobalBB(inTM.CList[i].x+xShift, inTM.CList[i].y-inTM.LineH+yShift, inTM.CList[i].w, inTM.LineH);
                        UIDrawBBOutline(bRect, borderC, 1);
                    }
                }
            }
        }
        
        if(inTM.Focus==0){ return;}
        // draw selection caret
        float caretW = inTM.CharW * 0.2f;ImRect caretBB;
        // static auto lastTime = std::chrono::high_resolution_clock::now();
        //UILibCTX->LastBlinkTime = std::chrono::high_resolution_clock::now();
        
        if(inTM.Change == 1 || inTM.SelChange==1 || inTM.LDrag>0 || inTM.MDrag>0){UILibCTX->LastBlinkTime = std::chrono::high_resolution_clock::now();}
        inTM.SelChange = 0; 
        //cout <<"  inTM.SList.size(): " << inTM.SList.size() << "\n";
        if(inTM.SList.size() > 0){
            auto currentTime = std::chrono::high_resolution_clock::now();
            float timeSpan = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - UILibCTX->LastBlinkTime).count();
            //cout << "       timeSpan: " << timeSpan << "\n";
            int timeCounts = (int)floor(timeSpan / 0.6f);
            bool blink = false;
            if(inTM.Change){blink = true;}
            else if(timeCounts % 2 == 0){blink = true;}
            if(blink){
                if(inTM.CList.size()==0){
                    float x0 = inTM.XBorder+xShift;
                    if(inTM.Format.HAlign > .8){x0 = xShift + inTM.Rect.GetWidth() - inTM.XBorder;}
                    else if(inTM.Format.HAlign > .2){x0 = xShift + inTM.Rect.GetWidth() * 0.5f;}
                    float y0 = inTM.Rect.Min.y + inTM.YBorder;
                    if(inTM.Format.VAlign > .8){y0 = inTM.Rect.Max.y - inTM.LineH;}
                    else if(inTM.Format.VAlign > .2){y0 = inTM.Rect.Min.y + inTM.Rect.GetHeight() * .5f - inTM.LineH * .5f;}
                    caretBB = GetBB(x0, y0, caretW, inTM.LineH);
                    UIDrawBB(caretBB,UILibCTX->EditorOptions[ed_cursor_active].Fill);
                }
                else{
                    int maxCL = (int)inTM.CList.size()-1;
                    for(int i=0;i<inTM.SList.size();i++){
                        if(inTM.SList[i].S<0 || inTM.SList[i].E<0){ 
                            inTM.SList[i].SetCaret(0); 
                        }
                        else if(inTM.SList[i].R == inTM.SList[i].S){
                            if(inTM.SList[i].S > maxCL){inTM.SList[i].S = maxCL;}
                            ch = &inTM.CList[inTM.SList[i].S];
                            caretBB = GetBB(ch->x-1.5f+xShift, ch->y+yShift, caretW, inTM.LineH);
                        }
                        else{
                            if(inTM.SList[i].E>(int)inTM.CList.size()-1){inTM.SList[i].E=(int)inTM.CList.size()-1;}
                            ch = &inTM.CList[inTM.SList[i].E];
                            if(inTM.SList[i].RSide){  caretBB = GetBB(ch->x-1.5f +xShift, ch->y+yShift, caretW, inTM.LineH);} // on right side
                            else{  caretBB = GetBB(ch->x-1.5f +ch->w +xShift, ch->y+yShift, caretW, inTM.LineH);} // on left side
                        }
                        UIDrawBB(caretBB, UILibCTX->EditorOptions[ed_cursor_active].Fill );
                    }
                }
            }
        }
    }
    void UITextAlignedClr(float inX, float inY, float inW, float inH, string inText, vector<int> &ColorList, int inFontI, vec2 inAlign, bool inWordWrap, bool inULine, float inScale){
        ImVec2 wPos=GImGui->CurrentWindow->Pos;
        UITextAlignedClrG(inX + wPos.x, inY + wPos.y, inW, inH, inText, ColorList, inFontI, inAlign, inWordWrap, inULine, inScale);
    }
    void UITextAlignedClrG(float inX, float inY, float inW, float inH, string inText, vector<int> &ColorList, int inFontI, vec2 inAlign, bool inWordWrap, bool inULine, float inScale){
        
        // draws with a 0 pixel border so shink dimensions in to get a border
        // =============================== BUILD VECTOR OF LINES ===============================
        ImDrawList* drawList=ImGui::GetWindowDrawList();
        char* c=(char *)inText.data();
        string word="";float wordW=0;int line=0; // current line in tLines;

        ImFont* font=GImGui->IO.Fonts->Fonts[inFontI];
        font->Scale = std::max(inScale,0.00001f);
        vector<float> charLens={};float x=0;int xI=0;  // x char index count
        float charW=0;float lineH=font->FontSize;
        vector<string> tLines={""}; // text lines, accumulates words
        vector<float> lengths={}; // list of text line lengths
        vector<int> endTypes={0}; // 0=started by /nl, 1=started by wordwrap

        int align = 0;if(inAlign[0]>.8){align=2;}else if(inAlign[0]>.2){align=1;}
        float space =font->GetCharAdvance((ImWchar)*c) ;
        
        while( *c != 0){
            if(*c == '\n'){
                for(int i=0;i<word.size();i++){x += charLens[i];}
                tLines[line]+=word;
                tLines.push_back("");lengths.push_back(x);line++;endTypes.push_back(0);
                x=0;charLens={};wordW=0;word="";++c;
            }
            if(*c==' ' && inWordWrap){
                if(x+wordW - space> inW){ // wrap around, can't fit word
                    
                    charW=font->GetCharAdvance((ImWchar)*c);
                    tLines.push_back("");lengths.push_back(x);endTypes.push_back(1);
                    line++;x=0;
                    
                    for(int i=0;i<word.size();i++){x += charLens[i];}
                    tLines[line] += word;
                    charLens={};wordW=0;word="";
                    continue;
                } 
                else{
                    for(int i=0;i<word.size();i++){x += charLens[i];}
                    tLines[line] += word;charLens={};wordW=0;word="";
                }
            }
            word+=*c;
            charW=font->GetCharAdvance((ImWchar)*c);
            charLens.push_back(charW);wordW += charW;++c;
            
            if(*c == 0){break;}
            //if(c[1] == 0){break;}

        }
        
        if(x+wordW>inW && inWordWrap){tLines.push_back("");lengths.push_back(x);line++;x=0;}
        tLines[line]+=word;for(int i=0;i<word.size();i++){x+=charLens[i];}
        lengths.push_back(x);

        int inC = 0x000000;int count=0;int clrMax = (int)ColorList.size()-1;
        // =============================== ALIGN AND RENDER TEXT ===============================
        float step;float yShift=inY;float y;
        int maxI=(int)tLines.size();
        if(tLines[maxI-1]==""||(int)tLines[maxI-1][0]==0){maxI--;}
        if(inAlign[1]>.8){ float textH= maxI * lineH;yShift += inH - textH;} // vertical bottom aligned
        else if(inAlign[1]>.2){float textH= maxI * lineH;yShift+=(inH-textH)*0.5f;} // vertical center aligned
        if(inAlign[0] > .8){  // right aligned
            for(int i=0;i<maxI;i++){
                x=inX+inW-lengths[i];y=lineH*(float)i+yShift;
                for(int j=0;j<tLines[i].size();j++){
                    step=font->GetCharAdvance((ImWchar)tLines[i][j]);
                    UIRenderChar(drawList,lineH,{x,y}, ColorList[count],tLines[i][j],font);count++;if(count>clrMax){count=clrMax;}
                    x+=step;
                }
                if(inULine){
                    float lineY = y + (lineH * .94f);
                    UIAddGlobalLine(inX+inW-lengths[i], lineY, x, lineY, inC, lineH*0.059f);
                }
            }
        }
        else if(inAlign[0]>.2){ // center aligned
            float halfSpace =space * .5f;
            for(int i=0;i<maxI;i++){
                x=inX+(inW-lengths[i])*0.5f;
                if(i!=0 && inWordWrap && endTypes[i]==1){x -= halfSpace;}
                y=lineH*(float)i+yShift;
                for(int j=0;j<tLines[i].size();j++){
                    step=font->GetCharAdvance((ImWchar)tLines[i][j]);
                    UIRenderChar(drawList,lineH,{x,y}, ColorList[count],tLines[i][j],font);count++;if(count>clrMax){count=clrMax;}
                    x+=step;
                }
                if(inULine){
                    float lineY = y + (lineH * .94f);
                    UIAddGlobalLine(inX+(inW-lengths[i])*0.5f, lineY, x, lineY, inC, lineH*0.059f);
                }
            }
        }
        else{ // left aligned
            for(int i=0;i<maxI;i++){
                x=inX;y=lineH*(float)i+yShift;
                if(i!=0 && inWordWrap && endTypes[i]==1){x -= space;}
                for(int j=0;j<tLines[i].size();j++){
                    if(tLines[i][j]==0){break;}
                    step=font->GetCharAdvance((ImWchar)tLines[i][j]);
                    UIRenderChar(drawList,lineH,{x,y}, ColorList[count],tLines[i][j],font);count++;if(count>clrMax){count=clrMax;}
                    x+=step;
                }
                if(inULine){
                    float lineY = y + (lineH * .94f);
                    UIAddGlobalLine(inX, lineY, x, lineY, inC, lineH*0.059f);
                }
            }
        }
        font->Scale = 1.0f; // reset font scale
    }
    void UITextAligned(float inX, float inY, float inW, float inH, string inText, int inC, int inFontI, vec2 inAlign, bool inWordWrap, bool inULine, float inScale){
        ImVec2 wPos=GImGui->CurrentWindow->Pos;
        UITextAlignedG(inX + wPos.x, inY + wPos.y, inW, inH, inText, inC, inFontI, inAlign, inWordWrap, inULine, inScale);
    }

    void UITextAlignedG(float inX, float inY, float inW, float inH, string inText, int inC, int inFontI, vec2 inAlign, bool inWordWrap, bool inULine, float inScale){
        
        // draws with a 0 pixel border so shink dimensions in to get a border
        // =============================== BUILD VECTOR OF LINES ===============================
        ImDrawList* drawList=ImGui::GetWindowDrawList();
        char* c=(char *)inText.data();
        string word="";float wordW=0;int line=0; // current line in tLines;

        ImFont* font=GImGui->IO.Fonts->Fonts[inFontI];
        font->Scale = std::max(inScale,0.00001f);
        vector<float> charLens={};float x=0;int xI=0;  // x char index count
        float charW=0;float lineH=font->FontSize;
        vector<string> tLines={""}; // text lines, accumulates words
        vector<float> lengths={}; // list of text line lengths
        vector<int> endTypes={0}; // 0=started by /nl, 1=started by wordwrap

        int align = 0;if(inAlign[0]>.8){align=2;}else if(inAlign[0]>.2){align=1;}
        float space =font->GetCharAdvance((ImWchar)*c) ;
        
        //cout << "ALIGN: ";
        while( *c != 0){
            if(*c == '\n'){
                if(x+wordW>inW && inWordWrap){ // wrap around, can't fit word
                    charW=font->GetCharAdvance((ImWchar)*c);
                    tLines.push_back("");lengths.push_back(x);
                    endTypes.push_back(1);
                    line++;x=0;
                } 
                for(int i=0;i<word.size();i++){x += charLens[i];}
                tLines[line]+=word;
                tLines.push_back("");lengths.push_back(x);line++;
                endTypes.push_back(0);
                x=0;charLens={};wordW=0;word="";++c;
            }
            if(*c==' ' && inWordWrap){
                if(x+wordW - space> inW){ // wrap around, can't fit word
                    
                    charW=font->GetCharAdvance((ImWchar)*c);
                    tLines.push_back("");lengths.push_back(x);
                    endTypes.push_back(1);
                    line++;x=0;
                    
                    for(int i=0;i<word.size();i++){x += charLens[i];}
                    tLines[line] += word;
                    charLens={};wordW=0;word="";
                    continue;
                } 
                else{
                    for(int i=0;i<word.size();i++){x += charLens[i];}
                    tLines[line] += word;charLens={};wordW=0;word="";
                }
            }
            word+=*c;
            charW=font->GetCharAdvance((ImWchar)*c);
            charLens.push_back(charW);wordW += charW;++c;
            if(*c == 0){break;}
        }

        if(x+wordW>inW && inWordWrap){tLines.push_back("");lengths.push_back(x);endTypes.push_back(1);line++;x=0;}
        tLines[line]+=word;for(int i=0;i<word.size();i++){x+=charLens[i];}
        lengths.push_back(x);
        // =============================== ALIGN AND RENDER TEXT ===============================
        float step;float yShift=inY;float y;
        int maxI=(int)tLines.size();
        if(tLines[maxI-1]==""||(int)tLines[maxI-1][0]==0){maxI--;}
        if(inAlign[1]>.8){ float textH= maxI * lineH;yShift += inH - textH;} // vertical bottom aligned
        else if(inAlign[1]>.2){float textH= maxI * lineH;yShift+=(inH-textH)*0.5f;} // vertical center aligned
        if(inAlign[0] > .8){  // right aligned
            for(int i=0;i<maxI;i++){
                x=inX+inW-lengths[i];y=lineH*(float)i+yShift;
                for(int j=0;j<tLines[i].size();j++){
                    step=font->GetCharAdvance((ImWchar)tLines[i][j]);
                    UIRenderChar(drawList,lineH,{x,y},inC,tLines[i][j],font);
                    x+=step;
                }
                if(inULine){
                    float lineY = y + (lineH * .94f);
                    UIAddGlobalLine(inX+inW-lengths[i], lineY, x, lineY, inC, lineH*0.059f);
                }

            }
        }
        else if(inAlign[0]>.2){ // center aligned
            float halfSpace =space * .5f;
            for(int i=0;i<maxI;i++){
                x=inX+(inW-lengths[i])*0.5f;
                if(i!=0 && inWordWrap && endTypes[i]==1){x -= halfSpace;}
                y=lineH*(float)i+yShift;
                for(int j=0;j<tLines[i].size();j++){
                    step=font->GetCharAdvance((ImWchar)tLines[i][j]);
                    UIRenderChar(drawList,lineH,{x,y},inC,tLines[i][j],font);
                    x+=step;
                }
                if(inULine){
                    float lineY = y + (lineH * .94f);
                    UIAddGlobalLine(inX+(inW-lengths[i])*0.5f, lineY, x, lineY, inC, lineH*0.059f);
                }
            }
        }
        else{ // left aligned
            for(int i=0;i<maxI;i++){
                x=inX;y=lineH*(float)i+yShift;
                if(i!=0 && inWordWrap && endTypes[i]==1){x-=space*0.5f;}
                for(int j=0;j<tLines[i].size();j++){
                    if(tLines[i][j]==0){break;}
                    step=font->GetCharAdvance((ImWchar)tLines[i][j]);
                    UIRenderChar(drawList,lineH,{x,y},inC,tLines[i][j],font);
                    x+=step;
                }
                if(inULine){
                    float lineY = y + (lineH * .94f);
                    UIAddGlobalLine(inX, lineY, x, lineY, inC, lineH*0.059f);
                }
            }
        }
        font->Scale = 1.0f; // reset font scale
    }
    TextMap UIBuildLinTM (ImRect inR, string &inText, array<int,4> inFont, int inLanguage, vec2 inBorder, TextFormat inTF, float inScale){
        
        // Linux version of get text map.  Uses \n for newline.  Assumes inText has been conformed.
        // also good by tools like data input UI that want to minimize characters (ie \n not \r\n)
        
        ImFont* font=GImGui->IO.Fonts->Fonts[inFont[0]];
        font->Scale = std::max(inScale,0.00001f);
        float lineH=font->FontSize;
        TextMap tMap=TextMap(lineH,inFont,inLanguage, inScale);
        tMap.CharW=font->GetCharAdvance('a');float baseW=tMap.CharW;float border=7;

        tMap.Format=inTF;

        tMap.Rect=inR;ImRect charBB;tMap.Width=0;float charW=0;
        string word="";float wordW=0;int line=0; // current line in tLines;
        vector<float> charLens={};float x=0;int xI=0;  // x char index count
        float inW=tMap.Rect.GetWidth();
        
        vector<string> tLines={""}; // text lines,accumulates words
        vector<float> lengths={}; // list of text line lengths
        unsigned char* c=(unsigned char *)inText.data();
        
        if(tMap.Format.Wrap==1){
            while( *c != 0){
                if(*c==' '){
                    if(x+wordW>inW){ // wrap around,can't fit word
                        tLines[line] += *c;
                        charW=font->GetCharAdvance((ImWchar)*c);
                        tLines.push_back("");lengths.push_back(x);
                        line++;x=0;
                        int clStart = 0;
                        if(word[0] ==' '){
                            word = word.substr(1);
                            clStart = 1;
                        }
                        for(int i=clStart;i<charLens.size();i++){x += charLens[i];}
                        tLines[line] += word;charLens={};wordW=0;word="";
                        continue;
                    } 
                    for(int i=0;i<charLens.size();i++){x += charLens[i];}
                    tLines[line] += word;charLens={};wordW=0;word="";
                }

                word+=*c;charW=font->GetCharAdvance((ImWchar)*c);
                if(*c=='\n'){
                    for(int i=0;i<charLens.size();i++){x += charLens[i];}
                    tLines[line]+=word;
                    tLines.push_back("");lengths.push_back(x);line++;
                    x=0;charLens={};wordW=0;word="";
                }
                else{wordW += charW;charLens.push_back(charW);}
                ++c;
            }
        }
        else{
            while( *c != 0){
                word+=*c;charW=font->GetCharAdvance((ImWchar)*c);
                if(*c=='\n'){
                    for(int i=0;i<charLens.size();i++){x += charLens[i];}
                    tLines[line]+=word;
                    tLines.push_back("");lengths.push_back(x);line++;
                    x=0;charLens={};wordW=0;word="";
                }
                else{wordW += charW;charLens.push_back(charW);}
                ++c;
            }
        }
        

        if(x+wordW>inW && tMap.Format.Wrap){
            tLines.push_back("");lengths.push_back(x);line++;x=0;
        }
        tLines[line]+=word;for(int i=0;i<word.size();i++){x+=charLens[i];}
        lengths.push_back(x);

        // =============================== ALIGN AND RENDER TEXT ===============================
        float y=0;float yShift=0;array<int,2> yRange={};yRange[0]=0;bool nl=false;bool skipEnd = false;
        int count=0;  // count in space of orginal text
        int clCount=0;  // tMap.CList count
        tMap.NbLines=(int)tLines.size();if(tLines[tMap.NbLines-1]==""){tMap.NbLines--;};int charType = 0 ;
        tMap.Height=tMap.NbLines * tMap.LineH;tMap.Width=0;tMap.MinX=FLT_MAX;
        if(inTF.VAlign>.8){ float textH= tMap.NbLines * lineH;yShift=tMap.Rect.GetHeight() - textH;} // vertical bottom aligned
        else if(inTF.VAlign>.2){float textH= tMap.NbLines * lineH;yShift=(tMap.Rect.GetHeight()-textH)*0.5f;} // vertical center aligned
        if(inTF.HAlign>.8){ // right aligned
            for(int i=0;i<tMap.NbLines;i++){
                x=inW-lengths[i];y=lineH*(float)i+yShift;tMap.MinX=std::min(tMap.MinX,x);nl=false;
                for(int j=0;j<tLines[i].size();j++){
                    if(tLines[i][j]=='\n'){charW=baseW;charType=1;nl=true;}
                    else{charW=font->GetCharAdvance((ImWchar)tLines[i][j]);charType=0;}
                    tMap.CList.push_back( {count,clCount,(unsigned char)tLines[i][j],x,y,charW,charType}  );
                    clCount++;x+=charW;count++;
                }
                if(tMap.Format.Wrap==1 && i==(int)tLines.size()-1){yRange[1]=clCount;skipEnd = true;}
                else if(nl||tMap.Format.Wrap){yRange[1]=clCount-1;}
                else{yRange[1]=clCount;}
                tMap.YRange.push_back(yRange);if(!skipEnd){yRange[0]=clCount;}
                tMap.Width=std::max(tMap.Width,lengths[i]);
            }
        }
        else if(inTF.HAlign>.2){ // center aligned
            for(int i=0;i<tMap.NbLines;i++){
                x=(inW-lengths[i])*0.5f;y=lineH*(float)i+yShift;tMap.MinX=std::min(tMap.MinX,x);nl=false;
                for(int j=0;j<tLines[i].size();j++){
                    if(tLines[i][j]=='\n'){charW=baseW;charType=1;nl=true;}
                    else{charW=font->GetCharAdvance((ImWchar)tLines[i][j]);charType=0;}
                    tMap.CList.push_back( {count,clCount,(unsigned char)tLines[i][j],x,y,charW,charType}  );
                    clCount++;x+=charW;count++;
                }
                if(tMap.Format.Wrap==1 && i==(int)tLines.size()-1){yRange[1]=clCount;skipEnd = true;}
                else if(nl||tMap.Format.Wrap){yRange[1]=clCount-1;}
                else{yRange[1]=clCount;}
                tMap.YRange.push_back(yRange);if(!skipEnd){yRange[0]=clCount;}
                tMap.Width=std::max(tMap.Width,lengths[i]);
            }
        }
        else{ // left aligned
            for(int i=0;i<tMap.NbLines;i++){
                x=0;y=lineH*(float)i+yShift;tMap.MinX=std::min(tMap.MinX,x);nl=false;
                for(int j=0;j<tLines[i].size();j++){
                    if(tLines[i][j]=='\n'){charW=baseW;charType=1;nl=true;}
                    else{charW=font->GetCharAdvance((ImWchar)tLines[i][j]);charType=0;}
                    tMap.CList.push_back( {count,clCount,(unsigned char)tLines[i][j],x,y,charW,charType}  );
                    clCount++;x+=charW;count++;
                }
                
                if(tMap.Format.Wrap==1 && i==(int)tLines.size()-1){yRange[1]=clCount;skipEnd = true;}
                else if(nl||tMap.Format.Wrap==1){yRange[1]=clCount-1;}
                else{yRange[1]=clCount;}
                tMap.YRange.push_back(yRange);if(!skipEnd){yRange[0]=clCount;}
                tMap.Width=std::max(tMap.Width,lengths[i]);
            }
        }

        if(!skipEnd){yRange[1]=clCount;tMap.YRange.push_back(yRange);}
        if(tMap.CList.size()==0){} // leave empty CList blank
        else if(charType==1){ x = 0;y = y+lineH;tMap.CList.push_back({count,clCount,0,x,y,baseW,2});}  // ends on newline, EOL is on newline
        else{ tMap.CList.push_back({count,clCount,0,x,y,baseW,2});} // ends on regular character, EOL is end of current line

        //cout << "YRange: ";for(auto yr:tMap.YRange){cout << "{"<< yr[0] << ","<<yr[1] <<"}, ";}cout <<"\n";

        if(tMap.CList.size()>0){tMap.MinY = tMap.CList[0].y;};
        font->Scale = 1.0f; // reset font scale
        return tMap;
        
    }
    TextMap UIBuildWinTM (ImRect inR, string &inText, array<int,4> inFont, int inLanguage, vec2 inBorder, TextFormat inTF, float inScale){
        
        // Windows version of get text map.  Uses \r\n for newline.  Assumes inText has been conformed
        ImFont* font = GImGui->IO.Fonts->Fonts[ inFont[0] ];
        font->Scale = std::max(inScale,0.00001f);
        vector<float> charLens = {};

        int xI = 0;  // x char index count
        int yI = 0; // y line count

        float lineH = font->FontSize;
        
        TextMap tMap = TextMap(lineH, inFont, inLanguage,inScale);
        tMap.SetBorder(inBorder.x, inBorder.y);
        tMap.CharW = font->GetCharAdvance('a');float charW = tMap.CharW;float border = 7;
        //float x0 = inR.Min.x + tMap.XBorder; float y0 = tMap.YBorder;  // don't put the Rect.Min.y in the y value.  That way you can offset scroller without build
        
        float x0 = tMap.XBorder;
        float y0 = tMap.YBorder;

        float x=x0;float y=y0;bool nl = false; // new line
        int count = 0;  // count in space of orginal text
        int clCount = 0;  // tMap.CList count
        tMap.Rect = inR;array<int,2> yRange = {};yRange[0] = 0;ImRect charBB;tMap.Width = 0;
        
        unsigned char* c = (unsigned char *)inText.data();
        while( *c != 0){
            if(*c == '\r'){
                yRange[1] = clCount;
                tMap.YRange.push_back(yRange);
                yRange[0] = clCount+1;
                ++c;count++;nl=true;charW=tMap.CharW;
                tMap.CList.push_back( {count, clCount, *c, x, y, charW, 1}  );
                clCount++;
            }
            else{
                nl=false;
                charW = font->GetCharAdvance((ImWchar)*c);
                tMap.CList.push_back( {count, clCount, *c, x, y, charW, 0}  );
                clCount++;
            }
            ++c;xI++;x+=charW;count++;
            if(nl){tMap.Width = std::max(tMap.Width, x);xI=0;yI++;x=x0;y+=lineH;}
            if(*c == 0){ // final box
                tMap.Width = std::max(tMap.Width, x);
                tMap.CList.push_back( {count, clCount, *c, x, y, charW, 2}  );
                yRange[1] = clCount;
                tMap.YRange.push_back( yRange );
                yI++;
            }
        }

        if(yI < 1){yI=1;}
        tMap.NbLines = yI;
        tMap.Height = tMap.NbLines * tMap.LineH;
        if(tMap.CList.size()>0){tMap.MinY = tMap.CList[0].y;};

         // =============================== ALIGNMENT PASS ===============================
         // TODO add alignment pass, not needed for now, typical use is Linux style text with alignment
         
        font->Scale = 1.0f; // reset font scale
        return tMap;

    }

    void UIDebugTextMap(TextMap &inTM){
        
        ImRect charBB;
        float xShift = inTM.Rect.Min.x; float yShift = inTM.Rect.Min.y; // shifts caused by scrolling
        // draw characters
        for(int i=0;i<inTM.CList.size();i++){

            float charW = inTM.CList[i].w;
            charBB = GetBB(inTM.CList[i].x+xShift, inTM.CList[i].y+yShift, charW, inTM.LineH);
            if( inTM.CList[i].Type == 0){  // 0=char, 1=newline, 2=endtext)
                UIDrawAlphaBB(charBB, 0x00AAFF, .03f);
                UIDrawBBOutline(charBB, 0x00AAFF, 2);
                UIAddTextWithFontBB(charBB, to_string(i), 0x000000, UILibCTX->DefaultFontB, {0.13, 0.05},UILibCTX->DefaultFontS );
            } 
            else if( inTM.CList[i].Type == 1){  // 0=char, 1=newline, 2=endtext)
                UIDrawAlphaBB(charBB, 0x0000FF, .2f);
                UIDrawBBOutline(charBB, 0x0000FF, 2);
                UIAddTextWithFontBB(charBB, to_string(i), 0x000000, UILibCTX->DefaultFontB, {0.13, 0.05}, UILibCTX->DefaultFontS );
            } 
            else if( inTM.CList[i].Type == 2){  // 0=char, 1=newline, 2=endtext)
                UIDrawAlphaBB(charBB, 0x0000FF, .5f);
                UIDrawBBOutline(charBB, 0x0000FF, 2);
                UIAddTextWithFontBB(charBB, to_string(i), 0x000000, UILibCTX->DefaultFontB, {0.13, 0.05}, UILibCTX->DefaultFontS );
            } 
        }

    }
    
    void UIMultiLineWrapText(float inX, float inY, string inText, float inW){
        
        float border = 7;
        float bWidth = inW - border * 2.0f;
        float lineSpacer = 20;
        
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImVec2 wPos = window->Pos;
        ImVec2 p1 = ImVec2(inX + wPos.x, inY + wPos.y);

        int fIndex = 0;
        ImFont* font = GImGui->IO.Fonts->Fonts[fIndex];
        float mWidth = 0;
        char* c = (char *)inText.data();
        string word = "";
        float wordW = 0;
        float x = border;
        float y = border;

        vector<float> charLens = {};
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        
        float cell_size = font->FontSize;
        ImGuiStyle& style = ImGui::GetStyle();
        float cell_spacing = style.ItemSpacing.y;
        float charW = 0;
        ImU32 clr = Cl( 0x000000 );

        while( *c != 0){
            if(*c==' '){
                if(x+wordW > bWidth){
                        // add words here
                        y += lineSpacer;
                        x = border; // + wordW;
                        
                }
                else{
                        //x += wordW;
                }

                //float startX = x + inX;
                // fill letters as a typewritter would
                for(int i=0;i<word.size();i++){
                        UIRenderChar(drawList, cell_size, {x+p1.x, y+p1.y}, clr, word[i] );
                        x += charLens[i];
                }
                charLens = {};
                wordW = 0;
                word = "";
            }
            
            word+=*c;
            charW = font->GetCharAdvance((ImWchar)*c);
            charLens.push_back(charW);
            wordW += charW;
            
            ++c;

        }

    }
// --------------------------------------------------------------------- //
// TEXT METRICS & MEASURING HELPERS                                      //
// --------------------------------------------------------------------- //
    
    void UIConformNewlines(string &inText, int inNlType){
        // make all LF and CR match inNLType
        // newline type: 0 = "\n" (linux & Mac), 1 = "\r" (old Mac), 2 = "\r\n" (Windows)
        string newText = "";
        if(inNlType == 2){
            for(int i=0;i<inText.size();i++){
                if(inText[i]=='\n'){newText += "\r\n";}
                else if(inText[i]=='\r'){if(inText[i+1]=='\n'){newText += "\r\n";i++;}else{newText += "\r\n";}}
                else{newText += inText[i];}
            }
        }
        else if(inNlType == 0){
            for(int i=0;i<inText.size();i++){
                if(inText[i]=='\n'){newText += "\n";}
                else if(inText[i]=='\r'){if(inText[i+1]=='\n'){newText += "\n";i++;}else{newText += "\n";}}
                else{newText += inText[i];}
            }
        }
        inText = newText;
    }
    void LogText(string inText){
        for(int i=0;i<inText.size();i++){
            if(inText[i]=='\n'){cout<<"\\n";}
            else if(inText[i]=='\r'){cout<<"\\r";}
            else{cout << inText[i];}
        }
        cout << "\n";
    }
    int UIGetNewlineType(string &inText){
        // Determine what type of new-line the text bufffer uses.
        // -1 = not determined, 0 = "\n" (linux & Mac), 1 = "\r" (old Mac), 2= "\r\n" (Windows)
        int nlCount=0;int A[]={0,0,0};int nlType=0;
        vector<string> nlLookup={"\\n","\\r","\\r\\n"};
        for(int i=0;i<inText.size();i++){
            if(inText[i]=='\n'){A[0]++;nlCount++;}
            else if(inText[i]=='\r'){
                if(inText[i+1]=='\n'){A[2]++;nlCount++;i++;}
                else{A[1]++;nlCount++;}
            }
        }
        if(nlCount==0){return -1;}
        nlType=(int)distance(A,max_element(A,A+3));
        return nlType;
    }
    float UIGetFontHeight(int inFontIndex){

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImFont* font = GImGui->IO.Fonts->Fonts[inFontIndex];
        float cellH = font->FontSize;
        return cellH;

    }
    float UIGetFontHeight(int inFontIndex, float inScale){
        return UIGetFontHeight( inFontIndex ) * inScale;
    }
    float UIGetTextWidth(const string & inText, int inFontIndex){
        
        ImFont* font = GImGui->IO.Fonts->Fonts[inFontIndex];
        float charW = 0;
        char* c = (char *)inText.data();
        while( *c != 0){
            charW += font->GetCharAdvance((ImWchar)*c);
            ++c;
        }
        return charW;

    }
    float UIGetTextWidth(const string & inText, int inFontIndex, float inScale){
        return UIGetTextWidth( inText, inFontIndex ) * inScale;
    }
    int UIGetTextNbLines(string inText){

        // gets height for UIMultiLineText, counting newline characters
        int count = 1;
        char* c = (char *)inText.data();
        while( *c != 0){
            if(*c == '\n'){count++;} // \n newline
            else if(*c == '\r'){
                if(c[1] == '\n'){++c;count++;} // \r\n newline
                else{count++;}  // \r newline
            }
            ++c;
        }
        
        return count;
    }


    float UIGetTextHeight(const string & inText, int inFontIndex){
        // gets height for UIMultiLineText, counting newline characters
        
        float lineH = UIGetFontHeight(inFontIndex);
        int count = 1;
        char* c = (char *)inText.data();
        while( *c != 0){
            if(*c == '\n'){count++;} // \n newline
            else if(*c == '\r'){
                if(c[1] == '\n'){++c;count++;} // \r\n newline
                else{count++;}  // \r newline
            }
            ++c;
        }
        
        return count * lineH;
    }
    float UIGetTextHeight(const string & inText, int inFontIndex, float inScale){
        return UIGetTextHeight( inText, inFontIndex ) * inScale;
    }
    float UIGetParagraphHeight( const string & inText, float inW){
        
        // gets height for UIMultiLineWrapText, cycling text fitting & wrapping and counting newlines

        float border = 7;
        float bWidth = inW - border * 2.0f;
        float lineSpacer = 20;

        int fIndex = 0;
        ImFont* font = GImGui->IO.Fonts->Fonts[fIndex];
        float mWidth = 0;
        const char* c = inText.c_str();
        string word = "";
        float wordW = 0;
        float x = border;
        float y = border;

        while( *c != 0){
            if(*c==' '){
                if(x+wordW > bWidth){
                        y += lineSpacer;
                        x = border + wordW;
                }
                else{
                        x += wordW;
                }
                wordW = 0;
                word = "";
            }
            
            word+=*c;
            wordW += font->GetCharAdvance((ImWchar)*c);
            ++c;

        }
        y += lineSpacer;
        return y + border;
    }

    vec2 UIGetTextDim(const string & inText, int inFontIndex) {

        ImFont* font = GImGui->IO.Fonts->Fonts[inFontIndex];
        float charW = 0, lineW = 0;
        float lineH = UIGetFontHeight(inFontIndex);
        int count = 1;
        char* c = (char *)inText.data();
        
        while( *c != 0){
            if(*c == '\n') { count++; lineW = std::max( lineW, charW ); charW = 0; }                // \n newline
            else if(*c == '\r') {
                if(c[1] == '\n') {                                                                  // \r\n newline
                    charW += font->GetCharAdvance((ImWchar)*c); c++; count++; lineW = std::max( lineW, charW ); charW = 0; 
                }     
                else { count++; lineW = std::max( lineW, charW ); charW = 0; }                      // \r newline
            }
            charW += font->GetCharAdvance((ImWchar)*c); c++; 
        }
        lineW = std::max( lineW, charW );

        return { lineW, count * lineH };
    }

    vec2 UIGetTextDim(const string & inText, int inFontIndex, float inScale) {
        return UIGetTextDim( inText, inFontIndex ) * inScale;
    }

    string ConvertToWinNL(string inStr,int &inNbLines){

        // convert from Linux /n text to Windows /r/n
        char* c=(char *)inStr.data();
        string outStr="";inNbLines=0;
        vector<string> vec={};string startC;string endC;string str="";
        while(*c != 0){
            if(*c=='\n'){outStr+='\r';inNbLines++;}
            outStr+=*c;++c;
        }
        return outStr;
    }
    

} // end namespace UILib

#endif /* UILIBTEXT_CPP */
