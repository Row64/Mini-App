#ifndef FONTMANAGER_CPP
#define FONTMANAGER_CPP

#include "c_PipelineManager.h"
using namespace AppCore;

#include "Font_CodeBold.h"
#include "Font_CodeBoldItalic.h"
#include "Font_CodeItalic.h"
#include "Font_CodeRegular.h"
#include "Font_OsBold.h"
#include "Font_OsBoldItalic.h"
#include "Font_OsItalic.h"
#include "Font_OsRegular.h"

namespace Blocks {
    
    void InitFonts() {
        
        ImGuiIO& io = ImGui::GetIO();

        // --------------------------------------------------------------------------------------------
        // ---------------------------------- LOAD FONTS FROM .H CODE ---------------------------------
        // --------------------------------------------------------------------------------------------

        // Based on process in: https://github.com/ocornut/imgui/blob/master/docs/FONTS.md
        // in section: "Using Font Data Embedded In Source Code"
        
        //============== 0 Font = DEFAULT FONT (widely used by UILib) ========================
        io.Fonts->AddFontFromMemoryCompressedTTF(OsRegular_compressed_data, (int)OsRegular_compressed_size, 17);

        ///============== BASE FONT SIZES (OpenSans) ========================

        //  ======== scale 0 - 2 below -- 0
        io.Fonts->AddFontFromMemoryCompressedTTF(OsRegular_compressed_data, (int)OsRegular_compressed_size, 10 );     
        io.Fonts->AddFontFromMemoryCompressedTTF(OsBold_compressed_data, (int)OsBold_compressed_size, 10 );   
        io.Fonts->AddFontFromMemoryCompressedTTF(OsItalic_compressed_data, (int)OsItalic_compressed_size, 10 ); 
        io.Fonts->AddFontFromMemoryCompressedTTF(OsBoldItalic_compressed_data, (int)OsBoldItalic_compressed_size, 10 );
        
        //  ======== scale 1 - 1 below -- 4
        io.Fonts->AddFontFromMemoryCompressedTTF(OsRegular_compressed_data, (int)OsRegular_compressed_size, 15 );     
        io.Fonts->AddFontFromMemoryCompressedTTF(OsBold_compressed_data, (int)OsBold_compressed_size, 15 );   
        io.Fonts->AddFontFromMemoryCompressedTTF(OsItalic_compressed_data, (int)OsItalic_compressed_size, 15 ); 
        io.Fonts->AddFontFromMemoryCompressedTTF(OsBoldItalic_compressed_data, (int)OsBoldItalic_compressed_size, 15 );

        //  ======== scale 2 - default -- 8
        io.Fonts->AddFontFromMemoryCompressedTTF(OsRegular_compressed_data, (int)OsRegular_compressed_size, 17 );     
        io.Fonts->AddFontFromMemoryCompressedTTF(OsBold_compressed_data, (int)OsBold_compressed_size, 17 );   
        io.Fonts->AddFontFromMemoryCompressedTTF(OsItalic_compressed_data, (int)OsItalic_compressed_size, 17 ); 
        io.Fonts->AddFontFromMemoryCompressedTTF(OsBoldItalic_compressed_data, (int)OsBoldItalic_compressed_size, 17 );

        //  ======== scale 3 - 1 above  -- 12
        io.Fonts->AddFontFromMemoryCompressedTTF(OsRegular_compressed_data, (int)OsRegular_compressed_size, 20 );     
        io.Fonts->AddFontFromMemoryCompressedTTF(OsBold_compressed_data, (int)OsBold_compressed_size, 20 );   
        io.Fonts->AddFontFromMemoryCompressedTTF(OsItalic_compressed_data, (int)OsItalic_compressed_size, 20 ); 
        io.Fonts->AddFontFromMemoryCompressedTTF(OsBoldItalic_compressed_data, (int)OsBoldItalic_compressed_size, 20 );

        //  ======== scale 4 - 2 above  -- 16
        io.Fonts->AddFontFromMemoryCompressedTTF(OsRegular_compressed_data, (int)OsRegular_compressed_size, 24 );     
        io.Fonts->AddFontFromMemoryCompressedTTF(OsBold_compressed_data, (int)OsBold_compressed_size, 24 );   
        io.Fonts->AddFontFromMemoryCompressedTTF(OsItalic_compressed_data, (int)OsItalic_compressed_size, 24 ); 
        io.Fonts->AddFontFromMemoryCompressedTTF(OsBoldItalic_compressed_data, (int)OsBoldItalic_compressed_size, 24 );

        //  ======== scale 5 - 3 above  -- 20
        io.Fonts->AddFontFromMemoryCompressedTTF(OsRegular_compressed_data, (int)OsRegular_compressed_size, 30 );     
        io.Fonts->AddFontFromMemoryCompressedTTF(OsBold_compressed_data, (int)OsBold_compressed_size, 30 );   
        io.Fonts->AddFontFromMemoryCompressedTTF(OsItalic_compressed_data, (int)OsItalic_compressed_size, 30 ); 
        io.Fonts->AddFontFromMemoryCompressedTTF(OsBoldItalic_compressed_data, (int)OsBoldItalic_compressed_size, 30 );

        //  ======== scale 6 - 4 above -- 24
        io.Fonts->AddFontFromMemoryCompressedTTF(OsRegular_compressed_data, (int)OsRegular_compressed_size, 40 );     
        io.Fonts->AddFontFromMemoryCompressedTTF(OsBold_compressed_data, (int)OsBold_compressed_size, 40 );   
        io.Fonts->AddFontFromMemoryCompressedTTF(OsItalic_compressed_data, (int)OsItalic_compressed_size, 40 ); 
        io.Fonts->AddFontFromMemoryCompressedTTF(OsBoldItalic_compressed_data, (int)OsBoldItalic_compressed_size, 40 );
        
        //  ======== scale 7 - 5 above -- 28
        io.Fonts->AddFontFromMemoryCompressedTTF(OsRegular_compressed_data, (int)OsRegular_compressed_size, 60 );     
        io.Fonts->AddFontFromMemoryCompressedTTF(OsBold_compressed_data, (int)OsBold_compressed_size, 60 );   
        io.Fonts->AddFontFromMemoryCompressedTTF(OsItalic_compressed_data, (int)OsItalic_compressed_size, 60 ); 
        io.Fonts->AddFontFromMemoryCompressedTTF(OsBoldItalic_compressed_data, (int)OsBoldItalic_compressed_size, 60 );
        ///============== CODE FONT SIZES (Incoloata) ========================

               //  ======== scale 0 - 2 below -- 32
        io.Fonts->AddFontFromMemoryCompressedTTF(CodeRegular_compressed_data, (int)CodeRegular_compressed_size, 10 );     
        io.Fonts->AddFontFromMemoryCompressedTTF(CodeBold_compressed_data, (int)CodeBold_compressed_size, 10 );   
        io.Fonts->AddFontFromMemoryCompressedTTF(CodeItalic_compressed_data, (int)CodeItalic_compressed_size, 10 ); 
        io.Fonts->AddFontFromMemoryCompressedTTF(CodeBoldItalic_compressed_data, (int)CodeBoldItalic_compressed_size, 10 );


        //  ======== scale 1 - 1 below - 36
        io.Fonts->AddFontFromMemoryCompressedTTF(CodeRegular_compressed_data, (int)CodeRegular_compressed_size, 15 );     
        io.Fonts->AddFontFromMemoryCompressedTTF(CodeBold_compressed_data, (int)CodeBold_compressed_size, 15 );   
        io.Fonts->AddFontFromMemoryCompressedTTF(CodeItalic_compressed_data, (int)CodeItalic_compressed_size, 15 ); 
        io.Fonts->AddFontFromMemoryCompressedTTF(CodeBoldItalic_compressed_data, (int)CodeBoldItalic_compressed_size, 15 );

        //  ======== scale 2 - default -- 40
        io.Fonts->AddFontFromMemoryCompressedTTF(CodeRegular_compressed_data, (int)CodeRegular_compressed_size, 17 );     
        io.Fonts->AddFontFromMemoryCompressedTTF(CodeBold_compressed_data, (int)CodeBold_compressed_size, 17 );   
        io.Fonts->AddFontFromMemoryCompressedTTF(CodeItalic_compressed_data, (int)CodeItalic_compressed_size, 17 ); 
        io.Fonts->AddFontFromMemoryCompressedTTF(CodeBoldItalic_compressed_data, (int)CodeBoldItalic_compressed_size, 17 );

        //  ======== scale 3 - 1 above  -- 44
        io.Fonts->AddFontFromMemoryCompressedTTF(CodeRegular_compressed_data, (int)CodeRegular_compressed_size, 20 );     
        io.Fonts->AddFontFromMemoryCompressedTTF(CodeBold_compressed_data, (int)CodeBold_compressed_size, 20 );   
        io.Fonts->AddFontFromMemoryCompressedTTF(CodeItalic_compressed_data, (int)CodeItalic_compressed_size, 20 ); 
        io.Fonts->AddFontFromMemoryCompressedTTF(CodeBoldItalic_compressed_data, (int)CodeBoldItalic_compressed_size, 20 );

        //  ======== scale 4 - 2 above  -- 48
        io.Fonts->AddFontFromMemoryCompressedTTF(CodeRegular_compressed_data, (int)CodeRegular_compressed_size, 24 );     
        io.Fonts->AddFontFromMemoryCompressedTTF(CodeBold_compressed_data, (int)CodeBold_compressed_size, 24 );   
        io.Fonts->AddFontFromMemoryCompressedTTF(CodeItalic_compressed_data, (int)CodeItalic_compressed_size, 24 ); 
        io.Fonts->AddFontFromMemoryCompressedTTF(CodeBoldItalic_compressed_data, (int)CodeBoldItalic_compressed_size, 24 );

        //  ======== scale 5 - 3 above  -- 52
        io.Fonts->AddFontFromMemoryCompressedTTF(CodeRegular_compressed_data, (int)CodeRegular_compressed_size, 30 );     
        io.Fonts->AddFontFromMemoryCompressedTTF(CodeBold_compressed_data, (int)CodeBold_compressed_size, 30 );   
        io.Fonts->AddFontFromMemoryCompressedTTF(CodeItalic_compressed_data, (int)CodeItalic_compressed_size, 30 ); 
        io.Fonts->AddFontFromMemoryCompressedTTF(CodeBoldItalic_compressed_data, (int)CodeBoldItalic_compressed_size, 30 );

        //  ======== scale 6 - 4 above -- 56
        io.Fonts->AddFontFromMemoryCompressedTTF(CodeRegular_compressed_data, (int)CodeRegular_compressed_size, 40 );     
        io.Fonts->AddFontFromMemoryCompressedTTF(CodeBold_compressed_data, (int)CodeBold_compressed_size, 40 );   
        io.Fonts->AddFontFromMemoryCompressedTTF(CodeItalic_compressed_data, (int)CodeItalic_compressed_size, 40 ); 
        io.Fonts->AddFontFromMemoryCompressedTTF(CodeBoldItalic_compressed_data, (int)CodeBoldItalic_compressed_size, 40 );
        
        //  ======== scale 7 - 5 above -- 60
        io.Fonts->AddFontFromMemoryCompressedTTF(CodeRegular_compressed_data, (int)CodeRegular_compressed_size, 60 );     
        io.Fonts->AddFontFromMemoryCompressedTTF(CodeBold_compressed_data, (int)CodeBold_compressed_size, 60 );   
        io.Fonts->AddFontFromMemoryCompressedTTF(CodeItalic_compressed_data, (int)CodeItalic_compressed_size, 60 ); 
        io.Fonts->AddFontFromMemoryCompressedTTF(CodeBoldItalic_compressed_data, (int)CodeBoldItalic_compressed_size, 60 );

        //=========================== GENERAL FONTS ===========================

        io.Fonts->AddFontFromMemoryCompressedTTF(OsRegular_compressed_data, (int)OsRegular_compressed_size, 18 );     
        io.Fonts->AddFontFromMemoryCompressedTTF(OsBold_compressed_data, (int)OsBold_compressed_size, 22 );   
        io.Fonts->AddFontFromMemoryCompressedTTF(OsBold_compressed_data, (int)OsBold_compressed_size, 34 ); 
        io.Fonts->AddFontFromMemoryCompressedTTF(OsBold_compressed_data, (int)OsBold_compressed_size, 17.5 );
        
        
        /* --------------------------------------------------------------------------------------------
        // ---------------------------- UNCOMMENT TO LOAD FONTS FROM FONT FILES ----------------------------
        // --------------------------------------------------------------------------------------------

        // Load fonts needed
        fs::path font_path = AppData.RootPath / "Data/OpenSans-Regular.ttf";
        fs::path semi_path = AppData.RootPath / "Data/open_sans_semibold.ttf";
        fs::path italic_path = AppData.RootPath / "Data/OpenSans-Italic.ttf";
        fs::path italicB_path = AppData.RootPath / "Data/OpenSans-BoldItalic.ttf";

        fs::path tech = AppData.RootPath / "Data/InconsolataGo-Regular.ttf";
        fs::path techB = AppData.RootPath / "Data/InconsolataGo-Bold.ttf";
        fs::path techI = AppData.RootPath / "Data/InconsolatalGo-Italic.otf";
        fs::path techBI = AppData.RootPath / "Data/InconsolataLGo-Bold-Italic.otf";
        
        //============== 0 Font = DEFAULT FONT (widely used by UILib) ========================
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(font_path), 17 ); 

        ///============== BASE FONT SIZES (OpenSans) ========================

        //  ======== scale 0 - 2 below -- 0
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(font_path), 10 );     
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(semi_path), 10 );   
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(italic_path), 10 ); 
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(italicB_path), 10 );
        
        //  ======== scale 1 - 1 below -- 4
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(font_path), 15 );   
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(semi_path), 15 );   
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(italic_path), 15 ); 
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(italicB_path), 15 );

        //  ======== scale 2 - default -- 8
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(font_path), 17 );    // size 16
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(semi_path), 17 );    // bold 16
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(italic_path), 17 );    // italic 16
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(italicB_path), 17 );    // italic bold 16

        //  ======== scale 3 - 1 above  -- 12
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(font_path), 20 );   
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(semi_path), 20 );   
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(italic_path), 20 ); 
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(italicB_path), 20 );

        //  ======== scale 4 - 2 above  -- 16
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(font_path), 24 );   
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(semi_path), 24 );   
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(italic_path), 24 ); 
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(italicB_path), 24 );

        //  ======== scale 45 - 3 above  -- 20
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(font_path), 30 );   
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(semi_path), 30 );   
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(italic_path), 30 ); 
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(italicB_path), 30 );

        //  ======== scale 6 - 4 above -- 24
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(font_path), 40 );   
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(semi_path), 40 );   
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(italic_path), 40 ); 
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(italicB_path), 40 );
        
        //  ======== scale 7 - 5 above -- 28
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(font_path), 60 );   
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(semi_path), 60 );   
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(italic_path), 60 ); 
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(italicB_path), 60 );

        ///============== CODE FONT SIZES (Incoloata) ========================

        //  ======== scale 0 - 2 below -- 32
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(tech), 10 );   
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(techB), 10 );   
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(techI), 10 ); 
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(techBI), 10 );

        //  ======== scale 1 - 1 below - 36
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(tech), 15 );   
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(techB), 15 );   
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(techI), 15 ); 
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(techBI), 15 );

        //  ======== scale 2 - BASE - 40
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(tech), 17 );   
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(techB), 17 );   
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(techI), 17 ); 
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(techBI), 17 );

        //  ======== scale 3 - 1 above - 44
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(tech), 20 );   
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(techB), 20 );   
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(techI), 20 ); 
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(techBI), 20 );

        //  ======== scale 4 - 2 above - 48
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(tech), 24 );   
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(techB), 24 );   
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(techI), 24 ); 
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(techBI), 24 );

        //  ======== scale 45 - 3 above - 52
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(tech), 30 );   
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(techB), 30 );   
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(techI), 30 ); 
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(techBI), 30 );

        //  ======== scale 6 - 4 above - 56
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(tech), 40 );
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(techB), 40 );
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(techI), 40 );
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(techBI), 40 );
        
        //  ======== scale 7 - 5 above - 60
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(tech), 60 );   
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(techB), 60 );   
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(techI), 60 ); 
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(techBI), 60 );

        //=========================== GENERAL FONTS ===========================
        
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(font_path), 18 );    // for formulas
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(semi_path), 22 );    // for popUp headers
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(semi_path), 34 );    // for diagnositcs headers
        io.Fonts->AddFontFromFileTTF(CONVERT_PATH(semi_path), 17.5 );    // for top tabs
        
        */


        UISetDefaultFonts( 9, 10, 11, 12 );

    }

    int GetFontInd(int inType, int inEmphasis, int inScale){
        
        // Type: 0 = sans-serif, 1 = code font
        // Emphasis: 0=regular, 1=bold, 2=italic, 3=bold italic
        // inScale: 0-1: small, 2: default, 3-7: large
        int fontI = 1;if(inType==1){fontI=32;}
        fontI += inScale * 4 + inEmphasis;
        return fontI;
    }

    void GetFontIndAndScale( int inType, float inSize, int inEmphasis, int & outIndex, float & outScale ){
        // Type: 0 = sans-serif, 1 = code font
        // Emphasis: 0=regular, 1=bold, 2=italic, 3=bold italic
        outIndex = 1;
        if ( inType == 1 ) { outIndex += 32; }
        if      ( inSize <= 10 ) { outIndex += 0 * 4 + inEmphasis; outScale = inSize / 10; }
        else if ( inSize <= 15 ) { outIndex += 1 * 4 + inEmphasis; outScale = inSize / 15; }
        else if ( inSize <= 17 ) { outIndex += 2 * 4 + inEmphasis; outScale = inSize / 17; }
        else if ( inSize <= 20 ) { outIndex += 3 * 4 + inEmphasis; outScale = inSize / 20; }
        else if ( inSize <= 24 ) { outIndex += 4 * 4 + inEmphasis; outScale = inSize / 24; }
        else if ( inSize <= 30 ) { outIndex += 5 * 4 + inEmphasis; outScale = inSize / 30; }
        else if ( inSize <= 40 ) { outIndex += 6 * 4 + inEmphasis; outScale = inSize / 40; }
        else if ( inSize <= 60 ) { outIndex += 7 * 4 + inEmphasis; outScale = inSize / 60; }
        else                     { outIndex += 7 * 4 + inEmphasis; outScale = inSize / 60; }
    }

    void GetFontIndAndScale( int inType, float inSize, bool inB, bool inI, int & outIndex, float & outScale ) {
        if ( inB && inI ) { return GetFontIndAndScale( inType, inSize, ftem_bolditalic, outIndex, outScale ); }
        else if ( inB ) { return GetFontIndAndScale( inType, inSize, ftem_bold, outIndex, outScale ); }
        else if ( inI ) { return GetFontIndAndScale( inType, inSize, ftem_italic, outIndex, outScale ); }
        return GetFontIndAndScale( inType, inSize, ftem_none, outIndex, outScale );
    }

    float ConvertStandardFontSize( float ghSize ) {
        // Converts from standard font sizes to ImGui font size
        float ghScale = 1.333333f; // scale factor so that the 'gh' height is a the number of pixels specified by inSize (currently only for OpenSans)
        return ghSize*ghScale;
    }

    int GetCodeFont(string inType){
        if(inType==""){return 41;}
        if(inType=="B"){return 42;}
        return 41;
    }
    int BaseFont(){return 9;}
    int BaseFontB(){return 10;}
    int GetGeneralFont(string inName){
        if(inName == "formula"){return 65;}
        else if(inName == "headerS"){return 66;} // for popup headers
        else if(inName == "headerL"){return 67;}
        else if(inName == "tab"){return 68;}
        else if(inName == "bigTab"){return 13;}
        else if(inName == "default"){return 9;}
        else if(inName == "code"){return 41;}
        else if(inName == "tabLabel"){return 14;}
        return 0;
    }

} // end namespace Blocks

#endif // FONTMANAGER_CPP