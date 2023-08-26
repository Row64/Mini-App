#ifndef CDR_BASE_CPP
#define CDR_BASE_CPP

#include "c_PipelineManager.h"
#include "CDR_Base.h"

using namespace AppCore;
using namespace std;

CDR_Base::CDR_Base( ChartSConfig * inConfig, CLO_Base * inCLO, Display3D * inDisp, int inPaneIndex, float &inX, float &inY, float &inW, float &inH ) :
    Config( inConfig ),
    PaneIndex( inPaneIndex ),
    SheetName( inConfig->Name ),
    LoadObj( inCLO ),
    X(inX), Y(inY), Width(inW), Height(inH) {
};

void CDR_Base::Init() {
    ChartBase & DT = LoadObj->Details;
    if ( !SyncPanAndZoom() ) {
        ContentBB = GetBB( 0, 0, DT.Layout.Width, DT.Layout.Height );
        FrameContentBB();
    }
    LayoutBB = { ContentBB.Min + vec2(DT.Layout.LPadding, DT.Layout.TPadding), ContentBB.Max - vec2(DT.Layout.RPadding, DT.Layout.BPadding) };
    Initialized = true;
}

void CDR_Base::Draw() {
    if ( CDRType == "CDR_Base" ) { return; }
    if ( !LoadObj->IsLoaded() ) { 
        std::cout << "CLO object not loaded!" << std::endl;
        return; 
    }
    DrawStart();    
    DrawPane();
    DrawEnd();
}

void CDR_Base::DrawStart() {
    PaneSizeChange = (  Width != LastWidth || Height != LastHeight );
    PaneActive = true;
    IsOverTransformBB = false;
    LastFrameContentBBChange = ContentBBChange;
    ContentBBChange = false;
    LastFrameLayoutBBChange = LayoutBBChange;
    LayoutBBChange = false;
    LastFrameZoomPanChange = ZoomPanChange; // Note: ZoomPanChange and LastFrameZoomPanChange currently only used by Choropleth
    ZoomPanChange = false;
    OldContentBB = ContentBB;
    MouseActive = ADIO->FM.ClickOK() && VecInBB( GetGlobalMousePos(), GetBB( X, Y, Width, Height ) ) && !EventLocked;
    UpdateLayout();
    UpdateEvents();
}

void CDR_Base::DrawEnd() {
    if ( PaneSizeChange ) {
        if ( LastZoomPanModifier == zp_frame ) { FrameContentBB(); }
        else if ( LastZoomPanModifier == zp_center ) { CenterContentBB(); }
        else if ( LastZoomPanModifier == zp_frame_and_center ) { FrameContentBB(); CenterContentBB(); }
        else { AdjustOrigin( LastWidth, LastHeight, Width, Height ); }
    }
    UpdateExpando();
    UpdatePanAndZoom();
    LastWidth = Width;
    LastHeight = Height;
    if ( ContentBBChange ) { 
        auto & DT = LoadObj->Details;
        LayoutBB = { ContentBB.Min + vec2(DT.Layout.LPadding, DT.Layout.TPadding), ContentBB.Max - vec2(DT.Layout.RPadding, DT.Layout.BPadding) };
        SyncContentBBChange();
        SavePanAndZoom(); 
    }
}

void CDR_Base::UpdateEvents() {
    
    if( PaneActive && ADIO->FM.ClickOK() && !EventLocked && !ADIO->FM.HasInputFocus() ) {

        // Check Keyboard Inputs
        int curr_key = PMPTR->GetInputState().Key.Key;
        bool key_change = ( PMPTR->GetInputState().Key.Change == 1 );
        if(key_change){
            if ( curr_key == 'F' ) { FrameContentBB(); }
            else if ( curr_key == 'R' ) { ResetPanAndZoom(); }
            else if ( curr_key == 'C' ) { CenterContentBB(); }
        }
        
    }

}

void CDR_Base::DrawTitle() {
    if ( !LoadObj->Details.ShowTitle ) { return; }

        // Draw Title based off of last ContentBB
    ChartBase & DT = LoadObj->Details;
    
    int   fontI;
    float fontS;
    float fontH;
    float fontW;
    
    // Update TitleBB if out of date
    if ( LoadObj->TitleUpdate || PaneSizeChange || TitleFontSize != DT.TitleFont.Size ) {

        GetFontIndAndScale( 0, (float) DT.TitleFont.Size, DT.TitleFont.Bold, DT.TitleFont.Italic, fontI, fontS );
        fontH = UIGetFontHeight( fontI, fontS );
        fontW = UIGetTextWidth( LoadObj->TitleText, fontI, fontS );
        
        float tX = DT.TitlePos[0] * ContentBB.GetWidth() - 0.5f*fontW + ContentBB.Min.x;
        float tY = DT.TitlePos[1] * ContentBB.GetHeight() - 0.5f*fontH + ContentBB.Min.y;

        TitleBB = GetBB( tX, tY, fontW, fontH );  // This is in content space

        TitleFontSize = DT.TitleFont.Size;
        LoadObj->TitleUpdate = false;

    }
    
    TitleBB = ContentToGlobalSpace( TitleBB );    // Shift from Local to Global Coordinates

    UITransformBB( TitleBB, TitleTD, ContentToGlobalSpace(ContentBB), !MouseActive || IsOverTransformBB, true );
    if ( !IsOverTransformBB ) { ADIO->FM.SetUITransformBBFocus( TitleTD.Type, CDRType + to_string(PaneIndex) + "TitleBB", true ); }

    using ts = UITransformBBData::TransformStatus;
    if ( TitleTD.Status == ts::ts_over || TitleTD.Status == ts::ts_down || TitleTD.Status == ts::ts_click ) {   // Highlight Legend box outline on mouse over or down
        ImRect highlightBB = TitleBB; highlightBB.Expand(1);
        UIDrawBBOutline( highlightBB, 0x2BA9E0, 1 );
        IsOverTransformBB = true;
    }
    else if ( TitleTD.Status == ts::ts_release ) {                                 // Save updated Title Pos & Dim on mouse release
        vec2 tCenter = GlobalToContentSpace( TitleBB ).GetCenter();
        DT.TitlePos = { ( tCenter.x - ContentBB.Min.x ) / LoadObj->Details.Layout.Width, ( tCenter.y - ContentBB.Min.y ) / LoadObj->Details.Layout.Height };  // Convert Pos to percentage
        LoadObj->SaveChart( false );    // Signal LoadObj to save later
    }
    
    GetFontIndAndScale( 0, ContentToGlobalSpace(DT.TitleFont.Size), DT.TitleFont.Bold, DT.TitleFont.Italic, fontI, fontS );
    UIAddTextWithFontBB( TitleBB, DT.Title, DT.TitleFont.Color, fontI, vec2(0.5), fontS );
    float fontU = std::round( ContentToGlobalSpace( DT.TitleFont.Size * ( DT.TitleFont.Bold ? 0.1f : 0.06f ) ) );
    if ( DT.TitleFont.Underline ) {
        UIAddAlphaTextUnderline( TitleBB, DT.Title, DT.TitleFont.Color, DT.TitleFont.Alpha, fontI, vec2(0.5f), fontU, fontS );
    }

    TitleBB = GlobalToContentSpace( TitleBB );      // Shift back to Content Space

    IsOverTransformBB |= ( TitleTD.Status == ts::ts_over );

}

void CDR_Base::UpdateExpando() {

    if(MiminalDraw){return;}

    using ts = UITransformBBData::TransformStatus;

    // LayoutBB drag and resize
    ExpandoBB = ContentToGlobalSpace(LayoutBB);

    UITransformBB( ExpandoBB, LayoutTD, ContentToGlobalSpace(ContentBB), !MouseActive || IsOverTransformBB, false, false );
    if ( !IsOverTransformBB ) { ADIO->FM.SetUITransformBBFocus( LayoutTD.Type, CDRType + to_string(PaneIndex) + "ExpandoBB_Layout", false, false ); }

    if ( LayoutTD.Status == ts::ts_over && !IsOverTransformBB ) {   // Highlight box outline on mouse over
        ImRect highlightBB = ExpandoBB; highlightBB.Expand(1);
        UIDrawBBOutline( highlightBB, 0x2BA9E0, 1 );
        IsOverTransformBB = true;
    }
    else if ( LayoutTD.Status == ts::ts_down || LayoutTD.Status == ts::ts_click ) { // Highlight box outline on mouse down and update the ContentBB
        ImRect highlightBB = ExpandoBB; highlightBB.Expand(1);
        UIDrawBBOutline( highlightBB, 0x2BA9E0, 1 );
        LayoutBB = GlobalToContentSpace( ExpandoBB );
        LoadObj->Details.Layout.LPadding = LayoutBB.Min.x - ContentBB.Min.x;
        LoadObj->Details.Layout.TPadding = LayoutBB.Min.y - ContentBB.Min.y;
        LoadObj->Details.Layout.RPadding = ContentBB.Max.x - LayoutBB.Max.x;
        LoadObj->Details.Layout.BPadding = ContentBB.Max.y - LayoutBB.Max.y;
        LayoutBBChange = true;
        LoadObj->SaveChart( false );
    }
    else if ( LayoutTD.Status == ts::ts_release ) {    // Update the LayoutBB on mouse release
        LayoutBB = GlobalToContentSpace( ExpandoBB );
        LoadObj->Details.Layout.LPadding = LayoutBB.Min.x - ContentBB.Min.x;
        LoadObj->Details.Layout.TPadding = LayoutBB.Min.y - ContentBB.Min.y;
        LoadObj->Details.Layout.RPadding = ContentBB.Max.x - LayoutBB.Max.x;
        LoadObj->Details.Layout.BPadding = ContentBB.Max.y - LayoutBB.Max.y;
        LayoutBBChange = true;
        LoadObj->SaveChart( false );
    }

    // ContentBB resize
    ExpandoBB = ContentToGlobalSpace(ContentBB);

    UITransformBB( ExpandoBB, ContentTD, GetBB(X,Y,Width,Height), !MouseActive || IsOverTransformBB, false, true );
    if ( !IsOverTransformBB ) { ADIO->FM.SetUITransformBBFocus( ContentTD.Type, CDRType + to_string(PaneIndex) + "ExpandoBB_Content", false, true ); }

    if ( ContentTD.Status == ts::ts_over && !IsOverTransformBB) {   // Highlight box outline on mouse over
        ImRect highlightBB = ExpandoBB; highlightBB.Expand(1);
        UIDrawBBOutline( highlightBB, 0x2BA9E0, 1 );
        IsOverTransformBB = true;
    }
    else if ( ContentTD.Status == ts::ts_down || ContentTD.Status == ts::ts_click ) {   // Highlight box outline on mouse down and update the ContentBB
        ImRect highlightBB = ExpandoBB; highlightBB.Expand(1);
        UIDrawBBOutline( highlightBB, 0x2BA9E0, 1 );
        UpdateContentBB( GlobalToContentSpace( ExpandoBB ) );
        LastZoomPanModifier = LoadObj->Details.Layout.ZoomPanMode = zp_user;
    }
    else if ( ContentTD.Status == ts::ts_release ) {                // Update the ContentBB on mouse release
        ImRect OldBB = ContentBB;
        UpdateContentBB( GlobalToContentSpace( ExpandoBB ) );
        LastZoomPanModifier = LoadObj->Details.Layout.ZoomPanMode = zp_user;
    }

}

void CDR_Base::UpdateLayout() {
    ChartBase & DT = LoadObj->Details;
    if(LoadObj->UpdateBB){      // Update from LayoutBB sliders
        ImRect OldBB = ContentBB;
        UpdateContentBB( GetBB( ContentBB.Min.x, ContentBB.Min.y, DT.Layout.Width, DT.Layout.Height ) );
        if ( ContentBB.Min.x + DT.Layout.LPadding > ContentBB.Max.x ) { DT.Layout.LPadding = ContentBB.GetWidth(); }
        if ( ContentBB.Min.x + DT.Layout.RPadding > ContentBB.Max.x ) { DT.Layout.RPadding = ContentBB.GetWidth(); }
        if ( ContentBB.Min.y + DT.Layout.TPadding > ContentBB.Max.y ) { DT.Layout.TPadding = ContentBB.GetHeight(); }
        if ( ContentBB.Min.y + DT.Layout.BPadding > ContentBB.Max.y ) { DT.Layout.BPadding = ContentBB.GetHeight(); }
        LayoutBB = { ContentBB.Min + vec2(DT.Layout.LPadding, DT.Layout.TPadding), ContentBB.Max - vec2(DT.Layout.RPadding, DT.Layout.BPadding) };
        LoadObj->UpdateBB = false;
    }
}

void CDR_Base::DrawContentBB(){
    if (   ContentBB.Min.x == FLT_MAX || ContentBB.Min.y == FLT_MAX 
        || ContentBB.Max.x == FLT_MIN || ContentBB.Max.y == FLT_MIN ) { return; }
    if ( LoadObj->Details.Layout.HasBkgdFill ) {
        UIDrawAlphaBB( ContentToGlobalSpace( ContentBB ), LoadObj->Details.Layout.BkgdFill.Color, LoadObj->Details.Layout.BkgdFill.Alpha );
    }
    UIDashRect( ContentToPaneSpace( ContentBB ), 0xCCCCCC, 1 );
}

void CDR_Base::ResetContentBB() {
    ContentBB = { FLT_MAX, FLT_MAX, FLT_MIN, FLT_MIN };
    LoadObj->SaveChart( false );    // signal to save later
}

void CDR_Base::FrameContentBB(){

    ImRect DrawRect = ContentToPaneSpace(ContentBB);
    float goalAspect = Width/Height;
    float aspect = DrawRect.GetWidth()/DrawRect.GetHeight();
    float aspectChange = goalAspect/aspect; 
    vec2 centerShift = {};

    float scl = 1;
    if(aspect <= goalAspect){
        scl = Height / DrawRect.GetHeight();
        float targetCWidth = ContentBB.GetHeight() * goalAspect;
        centerShift.x = (targetCWidth - ContentBB.GetWidth()) * 0.5f;
    }
    else{
        scl = Width / DrawRect.GetWidth();
        float targetCHeight = ContentBB.GetWidth() / goalAspect;
        centerShift.y = (targetCHeight - ContentBB.GetHeight()) * 0.5f;
    }
    OriginScale *=scl;
    vec2 contentP = ContentBB.Min - centerShift;

    Origin = (contentP * -1) * OriginScale;

    ZoomPanChange = true;
    LastZoomPanModifier = zp_frame;
    SavePanAndZoom();

}

void CDR_Base::CenterContentBB(){
    ImRect cBB = ContentToPaneSpace(ContentBB);
    vec2 shift = vec2( Width, Height ) * 0.5f - cBB.GetCenter();
    Origin += shift;
    ZoomPanChange = true;
    LastZoomPanModifier = ( LastZoomPanModifier == zp_frame ) ? zp_frame_and_center : zp_center;
    SavePanAndZoom();
}

void CDR_Base::ExpandContentBB( const glm::vec2 & p ) {
    if ( p.x < ContentBB.Min.x ) { ContentBB.Min.x = p.x; ContentBBChange = true; }
    if ( p.x > ContentBB.Max.x ) { ContentBB.Max.x = p.x; ContentBBChange = true; }
    if ( p.y < ContentBB.Min.y ) { ContentBB.Min.y = p.y; ContentBBChange = true; }
    if ( p.y > ContentBB.Max.y ) { ContentBB.Max.y = p.y; ContentBBChange = true; }
}

void CDR_Base::ExpandContentBB( const std::vector<glm::vec2> & inPnts ) {
    for ( const auto & p : inPnts ) {
        if ( p.x < ContentBB.Min.x ) { ContentBB.Min.x = p.x; ContentBBChange = true; }
        if ( p.x > ContentBB.Max.x ) { ContentBB.Max.x = p.x; ContentBBChange = true; }
        if ( p.y < ContentBB.Min.y ) { ContentBB.Min.y = p.y; ContentBBChange = true; }
        if ( p.y > ContentBB.Max.y ) { ContentBB.Max.y = p.y; ContentBBChange = true; }
    }
}

void CDR_Base::ExpandContentBB( const ImRect & inRect ) {
    ContentBB.Add( inRect ); ContentBBChange = true;
}

void CDR_Base::UpdateContentBB( const ImRect & inRect ) {
    ContentBB = inRect; ContentBBChange = true;
}

void CDR_Base::SyncContentBBChange() {

    vec2 relPct, newPnt;

    // Updates TitleBB maintaining relative location
    relPct = { LoadObj->Details.TitlePos[0], LoadObj->Details.TitlePos[1] };
    newPnt = { relPct.x * ContentBB.GetWidth() + ContentBB.Min.x, relPct.y * ContentBB.GetHeight() + ContentBB.Min.y };
    TitleBB.Translate( newPnt - TitleBB.GetCenter() );
    FitBBinBB( TitleBB, ContentBB );
}

bool CDR_Base::ApplyPanning( bool start, bool pan ) {
    // Adjust for Pan and returns true if there is any activity
    glm::vec2 mPos = GetLocalMousePos();
    if ( VecInBB( mPos, GetBB(0,0,Width,Height) ) && start && ADIO->FM.ClickOK() && !EventLocked ) {
        PanOrigin  = Origin;
        PanMouse = mPos;
        Panning = true;
        return true;
    }
    else if ( Panning ) {
        if ( pan ) {
            Origin = PanOrigin + mPos - PanMouse;
        }
        else {
            Origin = PanOrigin + mPos - PanMouse;
            Panning = false;
        }
        return true;
    }
    return false;
}

bool CDR_Base::ApplyZoom() {
    // Adjust for Zoom and returns true if there is any activity
    if ( !IsOverTransformBB && !ADIO->FM.HasMFocus(ft_chartsheet, PaneName) ) { return false; };
    float mScroll = GetMouseScroll();
    glm::vec2 mPos = GetLocalMousePos();
    float ScaleChange = 0, ScaleFactor = 1;
    vec2 A = mPos - Origin;             // zoom point in current origin space
    vec2 OriginDelta = vec2(0);
    if ( mScroll != 0 ) {
        ScaleChange = mScroll * 0.07f;          // current scale change
        ScaleFactor = 1 + ScaleChange;          // current scale factor
        OriginDelta = A - A * ScaleFactor;      // current origin shift
        OriginScale *= ScaleFactor;             // cumulative origin scale
    }
    Origin += OriginDelta;                      // cumulative origin shift
    return ( mScroll != 0 );
}

void CDR_Base::ResetPanAndZoom() {
    PanOrigin = glm::vec2(0);
    PanMouse = glm::vec2(0);
    Panning = false;
    Origin = glm::vec2(0);
    OriginScale = 1;
    ZoomPanChange = true;
    LastZoomPanModifier = zp_none;
    CenterContentBB();
}

bool CDR_Base::SyncPanAndZoom() {
    // Syncs up Pan and Zoom variables from Details
    auto & ir = LoadObj->Details.Layout.ContentRect;
    if ( ir[2] < 0 || ir[3] < 0 ) { return false; }     // Exit if ContentBB has never been initialized
    ContentBB = GetBB( ir[0], ir[1], ir[2], ir[3] );
    Origin = { LoadObj->Details.Layout.Origin[0], LoadObj->Details.Layout.Origin[1] };
    OriginScale = LoadObj->Details.Layout.OriginScale;
    if ( isnan(Origin.x) || isnan(Origin.y) || isnan(OriginScale) ) {
        Origin = glm::vec2(0); OriginScale = 1;
        SavePanAndZoom();
    }
    ZoomPanChange = true;
    LastZoomPanModifier = LoadObj->Details.Layout.ZoomPanMode;
    if ( LastZoomPanModifier == zp_frame ) { FrameContentBB(); }
    else if ( LastZoomPanModifier == zp_center ) { CenterContentBB(); }
    else if ( LastZoomPanModifier == zp_frame_and_center ) { FrameContentBB(); CenterContentBB(); }
    else { AdjustOrigin( LoadObj->Details.Layout.PaneDim[0], LoadObj->Details.Layout.PaneDim[1], Width, Height ); }
    return true;
}

void CDR_Base::SavePanAndZoom() {
    LoadObj->Details.Layout.ContentRect = { ContentBB.Min.x, ContentBB.Min.y, ContentBB.GetWidth(), ContentBB.GetHeight() };
    LoadObj->Details.Layout.Origin = { Origin.x, Origin.y };
    LoadObj->Details.Layout.OriginScale = OriginScale;
    LoadObj->Details.Layout.Width = LoadObj->Details.Layout.ContentRect[2];
    LoadObj->Details.Layout.Height = LoadObj->Details.Layout.ContentRect[3];
    LoadObj->Details.Layout.PaneDim = { Width, Height };
    LoadObj->Details.Layout.ZoomPanMode = LastZoomPanModifier;
    LoadObj->SaveChart( false );     // signal to save later
}

void CDR_Base::UpdatePanAndZoom() {
    // Adjust for Pan (using right click) and Zoom (with scroller)
    if( ApplyPanning( RightDragStart(), RightMouseDown() ) || ApplyZoom() ) {
        ZoomPanChange = true;
        LastZoomPanModifier = zp_user;
        SavePanAndZoom();
    }
}

void CDR_Base::AdjustOrigin( float oldW, float oldH, float newW, float newH ) {
    if ( oldW <= 0.0001f || oldH <= 0.0001f || newW <= 0.0001f || newH <= 0.0001f ) { return; }
    if ( oldW == newW && oldH == newH ) { return; }
    float fitScale = 1.0f;
    ImRect oldRect = GetBB(0,0,oldW,oldH);
    ImRect newRect = GetBB(0,0,newW,newH);
    ImRect fitRect = RescaleToFitBox( oldRect, newRect, fitScale );   
    vec2 shift = fitRect.GetCenter() - oldRect.GetCenter();
    Origin.x += shift.x;
    Origin.y += shift.y;
    ZoomPanChange = true;
    SavePanAndZoom();
}

glm::vec2 CDR_Base::PaneToContentSpace( const glm::vec2 & inVec ) {
    return ( inVec - Origin ) * ( 1 / OriginScale );
}

ImRect CDR_Base::PaneToContentSpace( const ImRect & inRect ) {
    return { PaneToContentSpace(inRect.Min), PaneToContentSpace(inRect.Max) };
}

glm::vec2 CDR_Base::ContentToPaneSpace( const glm::vec2 & inVec ) {
    return inVec * ( OriginScale ) + Origin;
}

ImRect CDR_Base::ContentToPaneSpace( const ImRect & inRect ) {
    return { ContentToPaneSpace(inRect.Min), ContentToPaneSpace(inRect.Max) };
}

float CDR_Base::ContentToGlobalSpace( float inVal ) {
    return inVal * OriginScale;
}

glm::vec2 CDR_Base::ContentToGlobalSpace( const glm::vec2 & inVec ){
    return VecToGlobal( ContentToPaneSpace(inVec) );
}

ImRect CDR_Base::ContentToGlobalSpace( const ImRect & inRect ) {
    return BBToGlobal( ContentToPaneSpace(inRect) );
}

ImRect CDR_Base::GlobalToContentSpace( const ImRect & inRect){
    return PaneToContentSpace( BBToLocal(inRect) );
}

glm::vec2 CDR_Base::GlobalToContentSpace( const glm::vec2 & inVec){
    return PaneToContentSpace( VecToLocal(inVec) );
}

float CDR_Base::GlobalToContentSpace( float inVal ){
    return inVal / OriginScale;
}

#endif /* CDR_BASE_CPP */
