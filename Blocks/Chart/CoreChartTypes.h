#ifndef CORECHARTTYPES_H
#define CORECHARTTYPES_H

#include "ByteStreamMacros.h"
#include <vector>
#include <string>

// -------------------------------------------------------------------- //
// Common Formatting Options                                            //
//                                                                      //
// This is are nested in various fields of the .chart file and are      //
// structs relating the chart formatting.                               //
// -------------------------------------------------------------------- //

struct FillOptions {
    int                 Color = 0xFFFFFF;
    float               Alpha = 1;
    declBYTESTREAM
};

struct LineOptions {
    int                 Color = 0x000000;
    float               Size = 1;
    float               Alpha = 1;
    declBYTESTREAM
};

struct FontOptions {
    int                 Font = 0;   // font type
    float               Size = 20;  // font size
    int                 Color = 0x000000;
    bool                Bold = false;
    bool                Italic = false;
    bool                Underline = false;
    float               Alpha = 1.0f;    
    declBYTESTREAM
};

struct LayoutOptions {
    float RPadding = 50;
    float LPadding = 50;
    float TPadding = 50;
    float BPadding = 50;
    float Width = 1000;
    float Height = 500;
    std::vector<float>  ContentRect = {0,0,-1,-1};  // = { X, Y, W, H } corresponding to ContentBB which is defined by Width & Height
    std::vector<float>  Origin = {0,0};
    float               OriginScale = 1;
    std::vector<float>  PaneDim = {0,0};        // Last pane dimensions associated with the Origin & Origin Scale
    int                 ZoomPanMode = 0;        // Last zoom pan mode (corresponds with CDR_Base::zp_modifier_type )
    bool            HasChartBorder = false;       // Display the chart BB inside the Layout (not all chart types have this)
    LineOptions     ChartBorder = {};
    bool            HasChartFill = false;
    FillOptions     ChartFill = {};
    bool            HasBkgdFill = false;
    FillOptions     BkgdFill = {};
    declBYTESTREAM
};

// -------------------------------------------------------------------- //
// Global Chart Data                                                    //
//                                                                      //
// This is stored in the "Details" field of the .chart file and         //
// contains fields that are common across different types of charts.    //
// -------------------------------------------------------------------- //

struct ChartBase {
    std::string Type = "";        // Pie, VerticalBar, HorizontalBar, etc.
    std::string Title = "";
    bool ShowTitle = true;
    FontOptions TitleFont = {};
    std::vector<float> TitlePos = { 0.5f, 0.05f };
    LayoutOptions Layout = {};

    declBYTESTREAM
    
};

// -------------------------------------------------------------------- //
// Type Specific Chart Data                                             //
//                                                                      //
// This is stored in the "Data" field of the .chart file and contains   //
// contains fields relating the chart data content.                     //
// -------------------------------------------------------------------- //

struct ExampleData {
    std::vector<float> XValues = { 0, 100, 100, 0 };
    std::vector<float> YValues = { 0, 0, 100, 100 };
    
    declBYTESTREAM
};

// -------------------------------------------------------------------- //
// Type Specific Chart Formatting                                       //
//                                                                      //
// This is stored in the "Format" field of the .chart file and contains //
// fields relating the chart formatting.                                //
// -------------------------------------------------------------------- //

struct ExampleFormat {
    int Color = 0xFF0000;
    
    declBYTESTREAM
};

#endif /* CORECHARTTYPES_H */
