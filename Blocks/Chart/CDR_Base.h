#ifndef CDR_BASE_H
#define CDR_BASE_H

#pragma once

#include "CLO_Base.h"


namespace AppCore {
    // Dummy version of Display3D to simplify code portability with line up with Row64 Studio
    class Display3D {};
}

class CDR_Base {

public:
    
    CDR_Base( Blocks::ChartSConfig * inConfig, CLO_Base * inCLO, AppCore::Display3D * inDisp, int inPaneIndex, float &inX, float &inY, float &inW, float &inH );
    virtual ~CDR_Base() {};

    virtual void                Init();
    
    void                        Draw();

protected:

    Blocks::ChartSConfig *      Config = nullptr;
    std::string                 SheetName = "";         // Sheet name from Config->Name
    int                         PaneIndex = 0;
    std::string                 PaneName = "";
    std::string                 CDRType = "CDR_Base";

    float &                     X;
    float &                     Y;
    float &                     Width;
    float &                     Height;
    
    float                       LastWidth = -1;
    float                       LastHeight = -1;
    bool                        PaneSizeChange = false;
    bool                        PaneActive = false;

    CLO_Base *                  LoadObj;

    bool                        Initialized = false;

    bool                        MiminalDraw = false; // Absolute minimal drawing mode.  Ignores base layout box draw and mouse manip.

    void                        DrawStart();
    virtual void                DrawPane() {};
    void                        DrawEnd();
    void                        DrawWarning( bool show );
    
    void                        UpdateEvents();

    bool                        EventLocked = false;
    bool                        MouseActive = false;
    bool                        IsOverTransformBB = false;

    ImRect                      TitleBB;
    UITransformBBData           TitleTD;
    float                       TitleFontSize = -1.0f;
    void                        DrawTitle();
    
    // Transformable BB to allow user resize of Content and Layout BBs
    ImRect                      ExpandoBB = { FLT_MAX, FLT_MAX, FLT_MIN, FLT_MIN };
    void                        UpdateExpando();

    ImRect                      LayoutBB = { FLT_MAX, FLT_MAX, FLT_MIN, FLT_MIN };      // Layout in Content space as described in Details.Layout
    UITransformBBData           LayoutTD;
    bool                        LayoutBBChange = false;
    bool                        LastFrameLayoutBBChange = false;
    virtual void                UpdateLayout();

    glm::vec2                   PanOrigin = glm::vec2(0);
    glm::vec2                   PanMouse = glm::vec2(0);
    bool                        Panning = false;
    glm::vec2                   Origin = glm::vec2(0);
    float                       OriginScale = 1;
    ImRect                      ContentBB = { FLT_MAX, FLT_MAX, FLT_MIN, FLT_MIN };
    UITransformBBData           ContentTD;
    ImRect                      OldContentBB;
    bool                        ContentBBChange = false;
    bool                        LastFrameContentBBChange = false;
    bool                        ZoomPanChange = false;
    bool                        LastFrameZoomPanChange = false;
    bool                        StretchToFit = false;
    bool                        StretchToFitChange = false;
    void                        DrawContentBB();
    void                        FrameContentBB();
    void                        CenterContentBB();
    void                        ResetContentBB();
    void                        ExpandContentBB( const glm::vec2 & inPnt ); // inputs should be in the Content space (not the Pane space)
    void                        ExpandContentBB( const std::vector<glm::vec2> & inPnts );
    void                        ExpandContentBB( const ImRect & inRect );
    void                        UpdateContentBB( const ImRect & inRect );
    void                        SyncContentBBChange();
    bool                        ApplyPanning( bool start, bool pan );
    bool                        ApplyZoom();
    void                        ResetPanAndZoom();
    bool                        SyncPanAndZoom();
    void                        SavePanAndZoom();
    void                        UpdatePanAndZoom();
    void                        AdjustOrigin( float oldW, float oldH, float newW, float newH );

    enum zp_modifier_type {
        zp_none = 0,
        zp_frame,
        zp_center,
        zp_frame_and_center,
        zp_user
    };
    int                         LastZoomPanModifier = zp_none;  // tracks the last thing that modified zoom/pan variables
    
    // Note there are 3 types of view spaces:
    //      - Content Space (store your stuff in this space if you want zoomable & pannable)
    //      - Pane Space is in LOCAL coordinates relative to your current pane
    //      - App Space is in GLOBAL coordinates to the application window
    // When you go to draw, you must convert out of Content Space into Pane Space. 
    // This will mean you have the latest 2D camera zoom & pan settings.
    // The UILib allows you to call draw functions using LOCAL or GLOBAL coordinates, so
    // make sure you're calling the appropriate UILib function depending on your space.
    glm::vec2                   PaneToContentSpace( const glm::vec2 & inVec );
    ImRect                      PaneToContentSpace( const ImRect & inRect );
    glm::vec2                   ContentToPaneSpace( const glm::vec2 & inVec );
    ImRect                      ContentToPaneSpace( const ImRect & inRect );

    //Content To Global
    float                       ContentToGlobalSpace( float inVal );
    glm::vec2                   ContentToGlobalSpace( const glm::vec2 & inVec );
    ImRect                      ContentToGlobalSpace( const ImRect & inRect );
    float                       GlobalToContentSpace( float inVal );
    glm::vec2                   GlobalToContentSpace( const glm::vec2 & inVec);
    ImRect                      GlobalToContentSpace( const ImRect & inRect);
   
};

#endif /* CDR_BASE_H */
