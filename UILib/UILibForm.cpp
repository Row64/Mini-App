#ifndef UILIBFORM_CPP
#define UILIBFORM_CPP


#include "UILib.h"

#pragma warning(disable:4996)  //  cross-platform so avoid these Microsoft warnings

namespace UILib {

    // ============================================================================================ //
    //                                       HELPER FUNCTIONS                                       //
    // ============================================================================================ //

    float UIFormItem::DecodeValue( string inCode, float inVal ) {
        // std::cout << "UIFormItem: " << Index << "Type: " << ItemType << ", DecodeValue: " << inCode << std::endl;
        string str = inCode;
        if ( str.back() == '%' ) { 
            str.pop_back(); 
            return stof( str ) / 100 * inVal;
        } 
        return stof( str );
    }

    void UIFormItem::DecodeValueAdd( string inCode, float & inVal, float & inPct ) {
        // std::cout << "UIFormItem: " << Index << "Type: " << ItemType << ", DecodeValueAdd: " << inCode << std::endl;
        if ( inCode == "" ) { return; }
        string str = inCode; 
        if ( str.back() == '%' ) { str.pop_back(); inPct += stof( str ) / 100; } 
        else { inVal += stof(str); }
    }

    float UIFormItem::GetTotalWithPadding( string inCode1, string inCode2, float inVal ) {
        // std::cout << "UIFormItem: " << Index << "Type: " << ItemType << ", GetTotalWithPadding: " << inCode1 << ", " << inCode2 << std::endl;
        if ( inCode1 == "" && inCode2 == "" ) { return inVal; }
        float padPct = 0;
        float padVal = 0;
        if ( inCode1 != "" ) {
            string str1 = inCode1;
            if ( str1.back() == '%' ) { str1.pop_back();  padPct += stof( str1 ) / 100; } 
            else { padVal += stof(str1); }
        }
        if ( inCode2 != "" ) {
            string str2 = inCode2;
            if ( str2.back() == '%' ) { str2.pop_back(); padPct += stof( str2 ) / 100; } 
            else { padVal += stof( str2 ); }
        }
        float rmdPct = 1 - padPct;
        float totalVal = ( rmdPct <= 0 ) ? padVal : ( (inVal + padVal) / (1 - padPct) );
        return totalVal;
    }

    UIFormItem::BoxFormat UIFormItem::GetBoxFormat( UIFormItem::BoxFormat & inFormat, UIFormItem::BoxFormat & defaultFormat ) {
        UIFormItem::BoxFormat format;
        format.BorderSize  = ( inFormat.BorderSize  >= 0 ) ? inFormat.BorderSize  : defaultFormat.BorderSize;
        format.BorderColor = ( inFormat.BorderColor >= 0 ) ? inFormat.BorderColor : defaultFormat.BorderColor;
        format.BkgdColor   = ( inFormat.BkgdColor   >= 0 ) ? inFormat.BkgdColor   : defaultFormat.BkgdColor;
        format.BkgdAlpha   = ( inFormat.BkgdAlpha   >= 0 ) ? inFormat.BkgdAlpha   : defaultFormat.BkgdAlpha;
        if ( inFormat.Font >= 0 ) {
            format.Font = inFormat.Font;
            format.FontScale = inFormat.FontScale;
        }
        else {
            format.Font = defaultFormat.Font;
            format.FontScale = defaultFormat.FontScale;
        }
        format.FontColor   = ( inFormat.FontColor   >= 0 ) ? inFormat.FontColor   : defaultFormat.FontColor;
        format.FontUnderline = ( inFormat.FontUnderline >= 0 ) ? inFormat.FontUnderline : defaultFormat.FontUnderline;
        if ( inFormat.ShadowAlpha >= 0 ) {
            format.ShadowBlurDist = inFormat.ShadowBlurDist;
            format.ShadowAlpha = inFormat.ShadowAlpha;
            format.ShadowRes = inFormat.ShadowRes;
            format.ShadowColor = inFormat.ShadowColor;
            format.ShadowXO = inFormat.ShadowXO;
            format.ShadowYO = inFormat.ShadowYO;
        }
        else {
            format.ShadowBlurDist = defaultFormat.ShadowBlurDist;
            format.ShadowAlpha = defaultFormat.ShadowAlpha;
            format.ShadowRes = defaultFormat.ShadowRes;
            format.ShadowColor = defaultFormat.ShadowColor;
            format.ShadowXO = defaultFormat.ShadowXO;
            format.ShadowYO = defaultFormat.ShadowYO;
        }
        return format;
    }

    // ============================================================================================ //
    //                                      BASE FORM ITEM CLASS                                    //
    // ============================================================================================ //

    ImRect UIFormItem::GetItemRect( float inX, float inY ) {
        return GetBB( inX+Pos.x, inY+Pos.y, Dim.x, Dim.y );
    }

    ImRect UIFormItem::GetContentRect( float inX, float inY ) {
        return GetBB( inX+Pos.x+Padding[0], inY+Pos.y+Padding[2], Dim.x-Padding[0]-Padding[1], Dim.y-Padding[2]-Padding[3] );
    }

    bool UIFormItem::IsVisible( ImRect inView, float inX, float inY  ) {
        if ( !Visible ) { return false; }
        if ( Dim.x == 0 ) { return false; }
        if ( Dim.y == 0 ) { return false; }
        return inView.Overlaps( GetItemRect(inX, inY) );
    }

    ImRect UIFormItem::GetFocusedRect( float inX, float inY ) { 
        return GetContentRect( inX, inY ); 
    };
    
    void UIFormItem::UpdateDim( float inW, float inH ) { 
        Dim = {inW, inH}; 
        UpdatePadding();
    }

    void UIFormItem::UpdatePadding() {
        Padding[0] = ( PadLeft   != "" ) ? DecodeValue( PadLeft,   Dim.x ) : 0;
        Padding[1] = ( PadRight  != "" ) ? DecodeValue( PadRight,  Dim.x ) : 0;
        Padding[2] = ( PadTop    != "" ) ? DecodeValue( PadTop,    Dim.y ) : 0;
        Padding[3] = ( PadBottom != "" ) ? DecodeValue( PadBottom, Dim.y ) : 0;
    }

    vec2 & UIFormItem::SetPos( float inX, float inY, float parentW, float parentH, float offsetX, float offsetY ) {
        Pos.x = ( X != "" ) ? DecodeValue( X, parentW ) + offsetX : inX;
        Pos.y = ( Y != "" ) ? DecodeValue( Y, parentH ) + offsetY : inY;
        return Pos;
    }

    vec4 & UIFormItem::SetMargin( float parentW, float parentH ) {
        Margin[0] = ( MarginLeft   != "" ) ? DecodeValue( MarginLeft,   parentW ) : 0;
        Margin[1] = ( MarginRight  != "" ) ? DecodeValue( MarginRight,  parentW ) : 0;
        Margin[2] = ( MarginTop    != "" ) ? DecodeValue( MarginTop,    parentH ) : 0;
        Margin[3] = ( MarginBottom != "" ) ? DecodeValue( MarginBottom, parentH ) : 0;
        return Margin;
    }

    // ============================================================================================ //
    //                              NON-DRAWING BUILDING BLOCK ITEMS                                //
    // ============================================================================================ //

    // UIFormCollapser ------------------------------------------------------------------------------

    void UIFormCollapser::AddItem( UIFormItem * inItem ) {
        Cache.push_back( inItem->Collapse );
        inItem->SetCollapse( !IsOpen );
        Items.push_back(inItem);
    }

    void UIFormCollapser::ToggleState() {
        NeedUpdate = true;
        IsOpen = !IsOpen;
        ItemsCollapse = !IsOpen;
        if ( IsOpen ) {     // Opened State
            for ( int i = 0; i < Items.size(); i++ ) { 
                Items[i]->SetCollapse( Cache[i] );
            }
        }
        else {              // Closed State
            for ( int i = 0; i < Items.size(); i++ ) { 
                Cache[i] = Items[i]->Collapse;
                Items[i]->SetCollapse( true );
            }
        }
        LastSetOpen = IsOpen;
    }

    void UIFormCollapser::SetState( bool inState, bool force ) {
        if ( IsOpen == inState && !force ) { return; }
        NeedUpdate = true;
        IsOpen = inState;
        ItemsCollapse = !IsOpen;
        if ( IsOpen ) {     // Opened State
            for ( int i = 0; i < Items.size(); i++ ) { 
                Items[i]->SetCollapse( Cache[i] );
            }
        }
        else {              // Closed State
            for ( int i = 0; i < Items.size(); i++ ) { 
                Cache[i] = Items[i]->Collapse;
                Items[i]->SetCollapse( true );
            }
        }
        LastSetOpen = IsOpen;
    }

    void UIFormCollapser::SetItemsCollapse( bool inCollapse ) {
        // Sets the Items collapse state without changing the Collapser state.
        // This is used for nested collapsers. When the parent collapser closes, it tells
        // the children collapsers to hide their items.
        if ( inCollapse == ItemsCollapse ) { return; }
        NeedUpdate = true;
        ItemsCollapse = inCollapse;
        if ( inCollapse ) {    // Collapse items
            for ( int i = 0; i < Items.size(); i++ ) { 
                Cache[i] = Items[i]->Collapse;
                Items[i]->SetCollapse( true );
            }
        }
        else {                  // Uncollapse items
            for ( int i = 0; i < Items.size(); i++ ) { 
                Items[i]->SetCollapse( Cache[i] );
            }
        }
    }

    void UIFormCollapser::SyncState() {
        // If something else modifies the IsOpen bool, sync up the collapser state.
        if ( IsOpen != LastSetOpen ) { 
            SetState( IsOpen, true ); 
        }
    }

    // ============================================================================================ //
    //                                        FRAMING ITEMS                                         //
    // ============================================================================================ //

    // UIFormDivider ------------------------------------------------------------------------------

    void UIFormDivider::Draw( float inX, float inY, bool clickMute ) {
        ImRect rect = GetContentRect( inX, inY );
        if ( Direction == 1 ) {     // Vertical
            float centerY = ( rect.Min.y + rect.Max.y ) * 0.5f;
            UIAddLine( rect.Min.x, centerY, rect.Max.x, centerY, Color, Thickness );
        }
        else {                      // Horizontal
            float centerX = ( rect.Min.x + rect.Max.x ) * 0.5f;
            UIAddLine( centerX, rect.Min.y, centerX, rect.Max.y, Color, Thickness );
        }
    }

    vec2 UIFormDivider::GetAutoDim() {
        return ( Direction == 1 ) ? vec2(MinDim.x, Thickness) : vec2(Thickness, MinDim.y);
    }

    // ============================================================================================ //
    //                                         BASIC ITEMS                                          //
    // ============================================================================================ //

    // UIFormRectangle ------------------------------------------------------------------------------

    void UIFormRectangle::Draw( float inX, float inY, bool clickMute ) {
        ImRect rect = GetContentRect( inX, inY );
        if ( Rounding > 0 ) {
            if ( BkgdColor  >= 0 ) { if ( BkgdAlpha   >  0 ) { UIDrawAlphaRoundCornerBB(rect, BkgdColor, BkgdAlpha, Rounding, CornerBits); } }
            if ( BorderSize >  0 ) { if ( BorderColor >= 0 ) { UIDrawRoundBBOutlineCorner(rect, BorderColor, Rounding, BorderSize, CornerBits); } }
        }
        else {
            if ( BkgdColor  >= 0 ) { if ( BkgdAlpha   >  0 ) { UIDrawAlphaBB(rect, BkgdColor, BkgdAlpha); } }
            if ( BorderSize >  0 ) { if ( BorderColor >= 0 ) { UIDrawBBOutline(rect, BorderColor, BorderSize); } }
        }
    }

    // UIFormRectangleDynamicFill ------------------------------------------------------------------------------

    void UIFormRectangleDynamicFill::Draw( float inX, float inY, bool clickMute ) {
        BkgdColor = FillColor;
        UIFormRectangle::Draw( inX, inY, clickMute );
    }
    
    // UIFormCenteredShape ------------------------------------------------------------------------------

    void UIFormCenteredShape::Draw( float inX, float inY, bool clickMute ) {
        ImRect rect = GetContentRect( inX, inY );
        vec2 mPos = GetGlobalMousePos();
        Click = false;
        if( MouseDown() && DragStart()){
            if( !clickMute && VecInBBClip(mPos, rect)){ Click = true;} 
        }
        ImVec2 ct = rect.GetCenter();
        float rad = rect.GetWidth() * 0.5f * Size;
        if ( ShapeType == 0 ) {
            UIAddGlobalCircle(ct.x, ct.y, rad, Color, true, 0, 16);
        }
        else if ( ShapeType == 1 ) {
            UIAddTriangleGlobal(ct.x, ct.y, rad, true, Color);
        }
        else if ( ShapeType == 2) {
            UIAddTriangleGlobal(ct.x, ct.y, rad, false, Color);
        }
    
    }

    // UIFormGrid --------------------------------------------------------------------------
    
    UIFormGrid::~UIFormGrid() {
        for ( auto it : Items ) { if (it->ItemType != uif_custom_external) delete it; }
    }

    uvec2 UIFormGrid::GetRowCol( int inIndex ) {
        // index is row major
        return uvec2( inIndex / GridSize.y, inIndex % GridSize.y );
    }

    void UIFormGrid::Draw( float inX, float inY, bool clickMute ) {

        if ( OverlayItem >= 0 ) {
            if ( !Items[OverlayItem]->GetOverlayState() ) { OverlayItem = -1; }
        }

        ImRect viewRect = GetItemRect( inX, inY );
        ImRect contentRect = GetContentRect( inX, inY );

        float xPos = 0, yPos = 0;
        xPos += Padding[0];
        yPos += Padding[2];

        for ( int i = 0; i < Items.size(); i++ ) {

            uvec2 rc = GetRowCol( i );

            auto it = Items[i];

            ( i == FocusedItem ) ? it->SetFocus( true ) : it->SetFocus( false );

            // Set item position within content rectangle (ignores button Margins)
            it->Pos.x = xPos + ( BlockSize.x - it->Dim.x - it->Margin[0] - it->Margin[1] ) * 0.5f;
            it->Pos.y = yPos + ( BlockSize.y - it->Dim.y - it->Margin[2] - it->Margin[3] ) * 0.5f;
            it->Draw( viewRect.Min.x, viewRect.Min.y, clickMute );

            // Uncomment to draw block outline
            // ImRect currR = GetBB( viewRect.Min.x+it->Pos.x, viewRect.Min.y+it->Pos.y, BlockSize.x, BlockSize.y );
            // UIDrawBBOutline( currR, 0xFF0000, 3 );

            // Update the running X position
            xPos += BlockSize.x + SpacerSize.x;

            // Update the running X & Y position if last element in row
            if ( rc.y == GridSize.y-1 ) { 
                xPos = Padding[0];
                yPos += BlockSize.y + SpacerSize.y; 
            }

            if ( it->GetFocus() == true ) { FocusedItem = i; } else if ( FocusedItem == i ) { FocusedItem = -1; }
            if ( it->GetOverlayState() == true ) { OverlayItem = i; }

        }

        // Uncomment to draw grid outline
        // UIDrawBBOutline( viewRect, 0x00FF00, 3 );

    }

    void UIFormGrid::UpdateDim( float inW, float inH ) {
        if ( Dim == vec2( inW, inH ) ) { return; }
        UIFormItem::UpdateDim( inW, inH );

        float contentW = Dim.x - Padding[0] - Padding[1];
        float contentH = Dim.y - Padding[2] - Padding[3];
        
        // Set BlockSize from AutoDim
        ContentDim = GetAutoDim();
        ContentDim.x = std::max( contentW, std::min( contentW, ContentDim.x ) );
        ContentDim.y = std::max( contentH, std::min( contentH, ContentDim.y ) );
        ContentDim.x -= SpacerSize.x * (GridSize.y-1);
        ContentDim.y -= SpacerSize.y * (GridSize.x-1);
        BlockSize = { ContentDim.x / GridSize.y, ContentDim.y / GridSize.x };

        // Uncomment to Log GridSize and BlockSize
        // std::cout << "GridSize: " << VecToString(GridSize) << ", BlockSize: " << VecToString( BlockSize ) << std::endl;
      
        // Cycle through and update each individual item dimensions
        for ( int i = 0; i < Items.size(); i++ ) {

            uvec2 rc = GetRowCol( i );

            auto it = Items[i];

            float w = 0, h = 0;

            // Calculate known values. Set any unspecified widths and heights using AutoDim
            if ( it->W != "" ) { w = DecodeValue( it->W, BlockSize.x ); } 
            else {
                if (it->Expand <= 0 ) { w = it->GetAutoDim().x; }
                else { w = it->Expand * BlockSize.x; }
            }
            if ( it->H != "" ) { h = DecodeValue( it->H, BlockSize.y  ); } 
            else { 
                if (it->Expand <= 0 ) {  h = it->GetAutoDim().y; }
                else { h = it->Expand * BlockSize.y; }
            }

            // Check Min / Max sizes for buttons
            w = std::max( it->MinDim.x, std::min( it->MaxDim.x, w ) );
            h = std::max( it->MinDim.y, std::min( it->MaxDim.y, h ) );

            // Set button dimensions
            it->UpdateDim( w, h );
            
        }

    }

    vec2 UIFormGrid::GetAutoDim() {
        // Calculates total dimension of grid based on the largest button dimensions in the grid.
        
        vec2 maxDim = {0,0};
        vec2 maxPct = {0,0};

        for ( int i = 0; i < Items.size(); i++ ) {

            auto it = Items[i];

            vec2 cmDim = {0,0};
            vec2 cmPct = {0,0};
            DecodeValueAdd( it->MarginLeft,   cmDim.x, cmPct.x );
            DecodeValueAdd( it->MarginRight,  cmDim.x, cmPct.x );
            DecodeValueAdd( it->MarginTop,    cmDim.y, cmPct.y );
            DecodeValueAdd( it->MarginBottom, cmDim.y, cmPct.y );

            vec2 dim = it->GetAutoDim();
            dim.x = std::max( it->MinDim.x, std::min( it->MaxDim.x, dim.x ) );
            dim.y = std::max( it->MinDim.y, std::min( it->MaxDim.y, dim.y ) );

            vec2 citDim = {0,0};
            vec2 citPct = {0,0};

            if ( it->W != "" ) { DecodeValueAdd( it->W, citDim.x, citPct.x ); } else { citDim.x += dim.x; }
            if ( it->H != "" ) { DecodeValueAdd( it->H, citDim.y, citPct.y ); } else { citDim.y += dim.y; }

            maxDim.x = std::max( maxDim.x, citDim.x + cmDim.x );
            maxPct.x = std::max( maxPct.x, citPct.x + cmPct.x );
            maxDim.y = std::max( maxDim.y, citDim.y + cmDim.y );
            maxPct.y = std::max( maxPct.y, citPct.y + cmPct.y );

        }

        vec2 totalDim;
        totalDim.x = maxDim.x * GridSize.y + SpacerSize.x * (GridSize.y-1);
        totalDim.y = maxDim.y * GridSize.x + SpacerSize.y * (GridSize.x-1);
        totalDim.x = GetTotalWithPadding( PadLeft, PadRight, totalDim.x );
        totalDim.y = GetTotalWithPadding( PadTop, PadBottom, totalDim.y );

        return totalDim;

    }

    bool UIFormGrid::SetFocus( bool inFocus, int inDirection, bool force ) {
        LastFocusedItem = FocusedItem;
        if ( inFocus == false ) {
            if ( FocusedItem == -1 ) { return false; }
            else {
                auto it = Items[FocusedItem];
                it->SetFocus( inFocus, inDirection, force );
                FocusedItem = -1;
                return false;
            }
        }
        else {
            if ( FocusedItem == -1 ) {
                if ( inDirection == 1 ) {
                    for ( int i = 0; i < Items.size(); i++ ) {
                        auto it = Items[i];
                        if ( it->SetFocus( true, inDirection, force ) ) {
                            FocusedItem = i;
                            return true;
                        }
                    }
                } else {
                    for ( int i = (int) Items.size(); i <= 0; i-- ) {
                        auto it = Items[i];
                        if ( it->SetFocus( true, inDirection, force ) ) {
                            FocusedItem = i;
                            return true;
                        }
                    }
                }
                return false;
            } 
            else {
                auto it = Items[FocusedItem];
                return it->SetFocus( inFocus, inDirection, force );
            }

        }
    }

    
    bool UIFormGrid::GetFocus() {
        return ( FocusedItem > -1 ) ? true : false;
    }

    int UIFormGrid::GetFocusType() {
        if ( FocusedItem > -1 ) { 
            auto it = Items[FocusedItem];
            return it->GetFocusType(); 
        }
        return -1;
    }

    bool UIFormGrid::RevertFocus() {
        bool res = false;
        if ( FocusedItem > -1 ) {
            auto it = Items[FocusedItem];
            it->SetFocus( false, 1, true );
        }
        if ( LastFocusedItem > -1 ) { 
            auto it = Items[LastFocusedItem];
            res = it->RevertFocus();
        }
        std::swap( FocusedItem, LastFocusedItem );
        return res;
    }
    
    UIFormItem * UIFormGrid::GetFocusedItem() {
        if ( FocusedItem > -1 ) { 
            return Items[FocusedItem];
        }
        return nullptr;
    }

    bool UIFormGrid::EnterKeyFocus() {
        if ( FocusedItem > -1 ) { 
            auto it = Items[FocusedItem];
            return it->EnterKeyFocus(); 
        }
        return UIFormItem::EnterKeyFocus();
    }

    ImRect UIFormGrid::GetFocusedRect( float inX, float inY ) {
        if ( FocusedItem > -1 ) {
            auto it = Items[FocusedItem];
            return it->GetFocusedRect( inX+Pos.x, inY+Pos.y );
        }
        return UIFormItem::GetFocusedRect( inX, inY ); 
    }

    bool UIFormGrid::SetNextFocus( int inDirection, bool inWrap ) {
        // This is the same as the SetNextFocus function in UIForm
        // TO DO: Need to update this function to handle Collapsed and Non-Visible items.

        int startIndex = (inDirection == 1 ) ? 0 : (int) Items.size()-1 ;
        int currFocused = ( FocusedItem > -1 ) ? FocusedItem : startIndex ;
        int nextFocused;
        bool done = false;

        if ( FocusedItem == -1 ) {
            done = Items[currFocused]->SetFocus( true, inDirection );
            if ( done ) { FocusedItem = currFocused; return true;}
        }

        while ( !done ) {
            
            done = Items[currFocused]->SetNextFocus( inDirection, false );
            if ( done ) { 
                FocusedItem = currFocused; 
                return true;
            }
                        
            if ( inWrap ) {
                if ( inDirection == 1 ) { nextFocused = (currFocused >= Items.size()-1 ) ? 0 : currFocused+1; } 
                else { nextFocused = (currFocused <= 0 ) ? (int) Items.size()-1 : currFocused-1; }
                
            }
            else {
                if ( inDirection == 1 ) {
                    if ( currFocused >= Items.size()-1 ) { 
                        nextFocused = -1; 
                    } 
                    else { 
                        nextFocused = currFocused + 1; 
                    }
                } else { 
                    if ( currFocused <= 0 ) { nextFocused = -1; } 
                    else { nextFocused = currFocused - 1; }
                }
            }

            if ( nextFocused == -1 ) { 
                SetFocus(false); 
                return false; 
            }
            
            currFocused = nextFocused;  
            done = Items[currFocused]->SetFocus( true, inDirection );
            if ( done ) { 
                FocusedItem = currFocused; 
                return true; 
            }

        }

        return done;
    }

    bool UIFormGrid::GetOverlayState() {
        return ( OverlayItem >= 0 );
    }
    
    vector<ImRect> UIFormGrid::GetOverlayRects() {
        if ( OverlayItem >= 0 ) {
            return Items[OverlayItem]->GetOverlayRects();
        }
        return {};
    }

    void UIFormGrid::DrawOverlay( float parentX, float parentY ) {
        if ( OverlayItem >= 0 ) {
            Items[OverlayItem]->DrawOverlay( parentX, parentY );
        }
    }

    void UIFormGrid::ResetDraw() {
        for ( auto it : Items ) { it->ResetDraw(); }
    }

    int UIFormGrid::AddItem( UIFormItem* inItem ) {
        // Adds a custom UIFormItem to the grid.
        // Set inItem->ItemType = uif_custom_external if you want the Item to persist after the grid exits.
        // Otherwise, the grid will delete the Item on exit.
        inItem->Index = (int) Items.size();
        Items.push_back( inItem );
        return inItem->Index;
    }

    // ============================================================================================ //
    //                                          TEXT ITEMS                                          //
    // ============================================================================================ //

    // UIFormHeader -------------------------------------------------------------------------------

    void UIFormHeader::Draw( float inX, float inY, bool clickMute ) {
        ImRect rect = GetContentRect( inX, inY );
        ImRect fillrect = ( FillItem ) ? GetItemRect( inX, inY ) : rect;
        if ( Format.BkgdAlpha  > 0 ) { if ( Format.BkgdColor   >= 0 ) { UIDrawAlphaBB(fillrect, Format.BkgdColor, Format.BkgdAlpha); } }
        if ( Format.BorderSize > 0 ) { if ( Format.BorderColor >= 0 ) { UIDrawBBOutline(fillrect, Format.BorderColor, Format.BorderSize); } }
        UIAddTextWithFontBB(rect, Text, Format.FontColor, Format.Font, Alignment, Format.FontScale );
        if( Format.FontUnderline > 0 ) {
            UIAddTextUnderline( rect, Text, Format.FontColor, Format.Font, Alignment, Format.FontUnderline, Format.FontScale );
        }
    }

    vec2 UIFormHeader::GetAutoDim() {
        vec2 dim = UIGetTextDim( Text, Format.Font, Format.FontScale);
        dim.x = GetTotalWithPadding( PadLeft, PadRight, dim.x );
        dim.y = GetTotalWithPadding( PadTop, PadBottom, dim.y );
        return dim;
    }

    // UIFormText --------------------------------------------------------------------------------

    void UIFormText::Draw( float inX, float inY, bool clickMute ) {
        ImRect rect = GetContentRect( inX, inY );
        if ( Format.BkgdColor  >= 0 ) { if ( Format.BkgdAlpha   >  0 ) { UIDrawAlphaBB(rect, Format.BkgdColor, Format.BkgdAlpha); } }
        if ( Format.BorderSize >  0 ) { if ( Format.BorderColor >= 0 ) { UIDrawBBOutline(rect, Format.BorderColor, Format.BorderSize); } }
        if ( Multiline ) {
            UITextAlignedG(rect.Min.x, rect.Min.y, rect.GetWidth(), rect.GetHeight(), Text, Format.FontColor, Format.Font, Alignment, true, (Format.FontUnderline>0), Format.FontScale );
        } else {
            UIAddTextWithFontBB(rect, Text, Format.FontColor, Format.Font, Alignment, Format.FontScale );
            if( Format.FontUnderline > 0 ) {
                UIAddTextUnderline( rect, Text, Format.FontColor, Format.Font, Alignment, Format.FontUnderline, Format.FontScale );
            }
        }
    }

    vec2 UIFormText::GetAutoDim() {
        vec2 dim = UIGetTextDim( Text, Format.Font, Format.FontScale);
        // TO DO: To get multi-line to display correctly on first frame you need to call
        // inPPG.Form->UpdateDim() TWICE before inPPG.Form->Draw()
        if ( Multiline && Dim.x > 0 ) {
            dim.y *= std::ceil( dim.x / Dim.x );
        }
        dim.x = GetTotalWithPadding( PadLeft, PadRight, dim.x );
        dim.y = GetTotalWithPadding( PadTop, PadBottom, dim.y );
        return dim;
    }

    // UIFormTextAB --------------------------------------------------------------------------------

    void UIFormTextAB::Draw( float inX, float inY, bool clickMute ) {
        ImRect rect = GetContentRect( inX, inY );
        UIFormItem::BoxFormat f = ( Condition ) ? FormatTrue : FormatFalse;
        if ( f.BkgdColor  >= 0 ) { if ( f.BkgdAlpha   >  0 ) { UIDrawAlphaBB(rect, f.BkgdColor, f.BkgdAlpha); } }
        if ( f.BorderSize >  0 ) { if ( f.BorderColor >= 0 ) { UIDrawBBOutline(rect, f.BorderColor, f.BorderSize); } }
        if ( Multiline ) {
            UITextAlignedG(rect.Min.x, rect.Min.y, rect.GetWidth(), rect.GetHeight(), Text, f.FontColor, f.Font, Alignment, true, (f.FontUnderline>0), f.FontScale );
        } else {
            UIAddTextWithFontBB(rect, Text, f.FontColor, f.Font, Alignment, f.FontScale );
            if( f.FontUnderline > 0 ) {
                UIAddTextUnderline( rect, Text, f.FontColor, f.Font, Alignment, f.FontUnderline, f.FontScale );
            }
        }        
    }

    vec2 UIFormTextAB::GetAutoDim() {
        UIFormItem::BoxFormat f = ( Condition ) ? FormatTrue : FormatFalse;
        vec2 dim = { UIGetTextWidth(Text, f.Font, f.FontScale), UIGetFontHeight(f.Font, f.FontScale) };
        dim.x = GetTotalWithPadding( PadLeft, PadRight, dim.x );
        dim.y = GetTotalWithPadding( PadTop, PadBottom, dim.y );
        return dim;
    }

    // UIFormTextInput --------------------------------------------------------------------------------

    UIFormTextInput::~UIFormTextInput() {
        if ( Focused ) {  
            if ( UIInputTextGetFocusHandle() == &Text  ) { 
                // std::cout << "UIFormTextInput destructor calling UIInputTextExitFocus()     global ptr: " << UIInputTextGetFocusHandle() << ", Text ptr: " << &Text << std::endl; 
                UIInputTextExitFocus( true );
            } 
        }
    }

    void UIFormTextInput::Draw( float inX, float inY, bool clickMute ) {
        if ( Dim.x == 0 ) { return; }
        if ( Dim.y == 0 ) { return; }

        // static uint32_t frame = 0;
        // std::cout << "--- Frame: " << (frame++) << std::endl;

        vec2 mPos = GetGlobalMousePos();
        ImRect rect = GetContentRect( inX, inY );
        UIFormItem::BoxFormat f = Default;

        if ( Rounding > 0 ) {
            if ( f.BkgdColor  >= 0 ) { if ( f.BkgdAlpha   >  0 ) { UIDrawAlphaRoundCornerBB(rect, f.BkgdColor, f.BkgdAlpha, Rounding, CornerBits); } }
            if ( f.BorderSize >  0 ) { if ( f.BorderColor >= 0 ) { UIDrawRoundBBOutlineCorner(rect, f.BorderColor, Rounding, f.BorderSize, CornerBits); } }
        }
        else {
            if ( f.BkgdColor  >= 0 ) { if ( f.BkgdAlpha   >  0 ) { UIDrawAlphaBB(rect, f.BkgdColor, f.BkgdAlpha); } }
            if ( f.BorderSize >  0 ) { if ( f.BorderColor >= 0 ) { UIDrawBBOutline(rect, f.BorderColor, f.BorderSize); } }
        }

        if ( Focused ) { 
            f = GetBoxFormat( Focus, Default );
            if ( UIInputTextGetFocusHandle() != &Text  ) {
                ImRect inputRect = GetGlobalBB(rect.Min.x+5, rect.Min.y, rect.GetWidth()-5, rect.GetHeight());
                UIInputTextInitFocus( &Text, inputRect, f.Font, f.FontColor, Mode, ItemName, &EnterFocus, &ExitFocus, &ValidateKey );
            }
            else {
                UILibCTX->InputBoxRect = GetGlobalBB(rect.Min.x+5, rect.Min.y, rect.GetWidth()-5, rect.GetHeight());
            }
            rect.Expand(1); 
            if ( Rounding > 0 ) {
                if ( f.BkgdColor  >= 0 ) { if ( f.BkgdAlpha   >  0 ) { UIDrawAlphaRoundCornerBB(rect, f.BkgdColor, f.BkgdAlpha, Rounding, CornerBits); } }
                if ( f.BorderSize >  0 ) { if ( f.BorderColor >= 0 ) { UIDrawRoundBBOutlineCorner(rect, f.BorderColor, Rounding, f.BorderSize, CornerBits); } }
            }
            else {
                if ( f.BkgdColor  >= 0 ) { if ( f.BkgdAlpha   >  0 ) { UIDrawAlphaBB(rect, f.BkgdColor, f.BkgdAlpha); } }
                if ( f.BorderSize >  0 ) { if ( f.BorderColor >= 0 ) { UIDrawBBOutline(rect, f.BorderColor, f.BorderSize); } }
            }
        }
        else {
            ImRect textRect = GetGlobalBB(rect.Min.x+5, rect.Min.y, rect.GetWidth()-5, rect.GetHeight());
            if(Mode == 2){
                std::string outputpwd = Text;
                for (int i = 0; i < outputpwd.length(); ++i) {
                    outputpwd[i] = '*';
                }
                UIAddTextWithFontBB(textRect, outputpwd, f.FontColor, f.Font, Alignment, f.FontScale );
            } else{
                UIAddTextWithFontBB(textRect, Text, f.FontColor, f.Font, Alignment, f.FontScale); 
            }
        }

        if( MouseDown() ) { 
            if( VecInBBClip(mPos, rect) ) {
                if( DragStart() && !clickMute && !Mute ){  // Clicking on the box enters text input mode
                    if ( !Focused ) {
                        SetFocus( true ); 
                        f = GetBoxFormat( Focus, Default );
                        ImRect inputRect = GetGlobalBB(rect.Min.x+5, rect.Min.y, rect.GetWidth()-5, rect.GetHeight());
                        if ( !UIInputTextInitFocus( &Text, inputRect, f.Font, f.FontColor, Mode, ItemName, &EnterFocus, &ExitFocus, &ValidateKey ) ) { Focused = false; }
                    }
                }    
            }
            else {                              // Clicking off the text rectangle 
                if ( DragStart() ) {            // Custom modes only exit text focus if click is not muted
                    if ( Mode > 1 && !clickMute && !Mute ) {
                        SetFocus(false);
                    }
                    else if ( Mode <= 1 ) {     // Default modes will exit text focus
                        SetFocus(false);
                    }
                    
                }
            }
        }
    }

    void UIFormTextInput::UpdateDim( float inW, float inH ) {
        UIFormItem::UpdateDim( inW, inH );
    }

    vec2 UIFormTextInput::GetAutoDim() {
        vec2 dim = UIGetTextDim( Text, Default.Font, Default.FontScale);
        dim.x = GetTotalWithPadding( PadLeft, PadRight, dim.x );
        dim.y = GetTotalWithPadding( PadTop, PadBottom, dim.y );
        return dim;
    }

    bool UIFormTextInput::SetFocus( bool inFocus, int inDirection, bool force ) {
        if ( Mute ) { return Focused; }
        if ( Focused == inFocus ) { return Focused; }
        if ( Focused ) {  
            if ( UIInputTextGetFocusHandle() == &Text  ) { 
                if ( !UIInputTextExitFocus( force ) ) {
                    return Focused;
                }
            } 
        }
        Focused = inFocus;
        return Focused;
    }
    
    bool UIFormTextInput::GetFocus() {
        return Focused;
    }

    bool UIFormTextInput::RevertFocus() {
        SetFocus( !Focused, 1, true );
        Focused = !Focused;
        return Focused;
    }

    bool UIFormTextInput::GetOverlayState() { return Focused; }

    vector<ImRect> UIFormTextInput::GetOverlayRects() {
        if ( !Focused ) { return {}; }
        return { UILibCTX->InputBoxRect };
    }

    void UIFormTextInput::DrawOverlay( float inX, float inY ) { 
        if( UILibCTX->InputBoxMode == 1 ) {
            UIDrawTextInput();
        }
        else if( UILibCTX->InputBoxMode == 2 ){
            UIDrawPasswordInput();
        }
    }

    // UIFormDateInput --------------------------------------------------------------------------------

    UIFormDateInput::UIFormDateInput( string & inText, bool inShowTime ) : 
        UIFormItem(uif_dateinput), Text( inText ), TMap( UITextMap::ModeType::tm_input ), Calendar() {
        Calendar.ShowTime = inShowTime;
        if ( Calendar.ShowTime ) { CalDim.y += Calendar.TimeHeight[0] + Calendar.TimeHeight[1]; } 
        TMap.AutoFocus = false;
        TextToTimeData();
        TMap.Set( Text );
        TextCache = Text;
    };

    UIFormDateInput::~UIFormDateInput() {
        if ( Focused ) { TMap.SetFocus(false); }
    }

    void UIFormDateInput::Draw( float inX, float inY, bool clickMute ) {
        if ( Dim.x == 0 ) { return; }
        if ( Dim.y == 0 ) { return; }

        vec2 mPos = GetGlobalMousePos();
        ImRect rect = GetContentRect( inX, inY );
        UIFormItem::BoxFormat f = Default;

        ImRect textRect = GetGlobalBB(rect.Min.x+5, rect.Min.y, rect.GetWidth()-5, rect.GetHeight());

        if ( Calendar.Status != UIDataCalendar::status_none ) {
            if ( !Calendar.ShowTime && Calendar.Status == UIDataCalendar::status_date_change ) { // Exit focus & close the calendar widget if last frame had a date change selection
                SetFocus(false);
            }
            else if ( Calendar.Status == UIDataCalendar::status_combo_closed ) {  // Time combo changed
                TimeDataToText();
            }
            else if ( Calendar.Status == UIDataCalendar::status_click_cancel ) {  // Clicked Cancel
                EscKeyFocus();
            }
            else if ( Calendar.Status == UIDataCalendar::status_click_ok ) {  // Clicked OK
                EnterKeyFocus();
            }
            Calendar.Status = UIDataCalendar::status_none; // Clear the status on the Calendar since it won't be drawn again this frame
        }
        
        if ( Focused ) { 
            f = GetBoxFormat( Focus, Default );
            rect.Expand(1);
            CalRect = GetBB( rect.Min.x, rect.Max.y + 2, CalDim.x, CalDim.y );
            FitBBinBB( CalRect, {0, 0, (float) UILibDS->FramebufferSize.W, (float) UILibDS->FramebufferSize.H } );
        }

        if ( Rounding > 0 ) {
            if ( f.BkgdColor  >= 0 ) { if ( f.BkgdAlpha   >  0 ) { UIDrawAlphaRoundCornerBB(rect, f.BkgdColor, f.BkgdAlpha, Rounding, CornerBits); } }
            if ( f.BorderSize >  0 ) { if ( f.BorderColor >= 0 ) { UIDrawRoundBBOutlineCorner(rect, f.BorderColor, Rounding, f.BorderSize, CornerBits); } }
        }
        else {
            if ( f.BkgdColor  >= 0 ) { if ( f.BkgdAlpha   >  0 ) { UIDrawAlphaBB(rect, f.BkgdColor, f.BkgdAlpha); } }
            if ( f.BorderSize >  0 ) { if ( f.BorderColor >= 0 ) { UIDrawBBOutline(rect, f.BorderColor, f.BorderSize); } }
        }

        TFormat.Align = { 0, 0.5f };
        TFormat.Syntax[0].Index = f.Font;
        TFormat.Syntax[0].Scale = f.FontScale;
        TFormat.Syntax[0].Color = f.FontColor;
        TMap.UpdateFormat( TFormat );

        TMap.Draw( textRect, rect );

        if( MouseDown() ) { 
            if( VecInBBClip(mPos, rect) ) {
                if( DragStart() && !clickMute && !Mute ){   // Clicking on the box enters text input mode
                    if ( !Focused ) {
                        SetFocus( true ); 
                        CalRect = GetBB( rect.Min.x, rect.Max.y + 2, CalDim.x, CalDim.y );
                        FitBBinBB( CalRect, {0, 0, (float) UILibDS->FramebufferSize.W, (float) UILibDS->FramebufferSize.H } );
                    }
                }    
            }
            else if ( DragStart() ) {
                const auto & overR = GetOverlayRects();
                bool overClicked = false;
                if ( Focused ) {
                    for ( const ImRect & oR : overR ) {
                        if ( VecInBB( mPos, oR ) ) { overClicked = true; break; }
                    }
                }
                if ( !overClicked ) { EscKeyFocus(); }  // Clicking outside overlay rects will exit focus (like the escape key)
            }
        }

        if ( TMap.Focused() ) {
            if ( TMap.GetChange() == UITextMap::ChangeType::tm_change_insert ) {
                Text = TMap.GetText();
                TextToTimeData();
            }
        }

    }

    void UIFormDateInput::UpdateDim( float inW, float inH ) {
        UIFormItem::UpdateDim( inW, inH );
    }

    vec2 UIFormDateInput::GetAutoDim() {
        vec2 dim = UIGetTextDim( Text, Default.Font, Default.FontScale);
        dim.x = GetTotalWithPadding( PadLeft, PadRight, dim.x );
        dim.y = GetTotalWithPadding( PadTop, PadBottom, dim.y );
        return dim;
    }

    bool UIFormDateInput::SetFocus( bool inFocus, int inDirection, bool force ) {
        if ( Mute ) { return Focused; }
        if ( Focused == inFocus ) { return Focused; }
        Focused = inFocus;
        TMap.SetFocus( Focused );
        if ( Focused ) { TextCache = Text; } // cache the text if entering focus
        else { Calendar.CloseComboBoxes(); } // close any calendar combos if exiting focus
        return Focused;
    }

    bool UIFormDateInput::SetNextFocus( int inDirection, bool inWrap ) {
        return EnterKeyFocus();
    }
    
    bool UIFormDateInput::GetFocus() {
        return Focused;
    }

    bool UIFormDateInput::RevertFocus() {
        return SetFocus( !Focused, 1, true );
    }

    bool UIFormDateInput::EscKeyFocus() {
        // Escape key exits focus and restore the original text & datetime
        if ( !Focused ) { return Focused; }
        SetFocus( false );
        Text = TextCache;
        TMap.Set( Text );
        TextToTimeData();
        return Focused;
    }

    bool UIFormDateInput::EnterKeyFocus() {
        // Enter key exits focus and updates the text map & datetime
        SetFocus( false );
        Text = TMap.GetText();
        TextToTimeData();
        TimeDataToText();
        return Focused;
    }    
    
    bool UIFormDateInput::ProcessKeyDown( int inKey ) {
        if ( !TMap.Focused() ) { return false; }
        if ( inKey == 256 ) {                       // Esc Key handled by parent form
            return false;
        }
        else if ( inKey == 257 || inKey == 335 ) {  // Enter Key handled by parent form
            return false;
        }
        else if ( inKey == 258 ) {                  // Tab key handled by parent form
            return false;
        }
        return true;    // All other keys handled by the TMap
    }

    bool UIFormDateInput::GetOverlayState() { return Focused; }

    vector<ImRect> UIFormDateInput::GetOverlayRects() {
        if ( !Focused ) { return {}; }
        vector<ImRect> res = { CalRect };
        if ( Calendar.ComboOpen[0] || Calendar.ComboOpen[1] || Calendar.ComboOpen[2] ) {
            res.push_back( UILibCTX->ComboRect ); 
        }
        return std::move( res );
    }

    void UIFormDateInput::DrawOverlay( float inX, float inY ) { 
        // Draw CalRect
        float dropAlpha = .18f;
        UIDropShadow(CalRect, 13, dropAlpha, 15, 15, 0x222222, 1, 1);
        UIDrawBB(CalRect, 0xFFFFFF);
        UIDrawBBOutline(CalRect, 0xCCCCCC, 1);
        CalRect.Expand( { CalPad.x*-1, CalPad.y*-1 } );
        UIAddCalendarWidget( CalRect, Calendar, Mute );
        if ( Calendar.Status == UIDataCalendar::status_date_change ) { // If date selection has changed, 
            TimeDataToText();         // then sync Text with date selection
        }
    }

    void UIFormDateInput::UpdateInputText( const string & inDateText ) {
        Text = inDateText;
        TextToTimeData();
        TMap.Set( Text );
        TextCache = Text;
    }

    void UIFormDateInput::UpdateTimeFormat( const string & inFormat ) {
        TimeFormat = inFormat;
        TimeDataToText();
    }

    void UIFormDateInput::UpdateDateTime( int inYear, int inMon, int inDay, int inH, int inM, int inS ) {
        Calendar.SetDate( inYear, inMon, inDay, inH, inM, inS );
        TimeDataToText();
    }

    void UIFormDateInput::TimeDataToText() {
        Text.clear(); Text.reserve( TimeFormat.size() + 10 );
        const char * c = TimeFormat.c_str();
        int count = 0;
        string tmp = "";
        while ( *c != 0 ) {
            
            if ( *c == 'm' ) {
                count = 1; c++; while ( *c == 'm' ) { count++; c++; }
                if      ( count == 1 ) { Text += std::to_string( Calendar.Month ); }
                else if ( count == 2 ) { if ( Calendar.Month < 10 ) { Text += " "; } Text += std::to_string( Calendar.Month ); }
                else if ( count == 3 ) { Text += Calendar.MonthAsString( Calendar.Month, true ); }
                else                   { Text += Calendar.MonthAsString( Calendar.Month, false ); }
            }
            else if ( *c == 'd' ) {
                count = 1; c++; while ( *c == 'd' ) { count++; c++; }
                if ( count > 1 ) {
                    count -= ( Calendar.Day < 10 ? 1 : 2 );
                    if ( count > 0 ) { Text += std::string(count,'0'); }
                }
                Text += std::to_string( Calendar.Day );
            }
            else if ( *c == 'y' ) {
                count = 1; c++; while ( *c == 'y' ) { count++; c++; }
                if ( count <= 2 ) { count = 2; tmp = std::to_string( Calendar.Year % 100  ); }
                else              { count = 4; tmp = std::to_string( Calendar.Year ); }
                if ( count > tmp.size() ) { Text += string( count-tmp.size(), '0' ); }
                Text += tmp;
            }
            else if ( *c == 'H' ) {
                count = 1; c++; while ( *c == 'H' ) { count++; c++; }
                tmp = std::to_string( Calendar.Hour );
                if ( count > tmp.size() ) { Text += string( count-tmp.size(), '0' ); }
            }
            else if ( *c == 'h' ) {
                count = 1; c++; while ( *c == 'h' ) { count++; c++; }
                tmp = std::to_string( Calendar.Hour == 0 ? 12 : ( Calendar.Hour > 12 ? Calendar.Hour - 12 : Calendar.Hour ) );
                if ( count > tmp.size() ) { Text += string( count-tmp.size(), '0' ); }
                Text += tmp;
            }
            else if ( *c == 'M' ) {
                count = 1; c++; while ( *c == 'M' ) { count++; c++; }
                tmp = std::to_string( Calendar.Minute );
                if ( count > tmp.size() ) { Text += string( count-tmp.size(), '0' ); }
                Text += tmp;
            }
            else if ( *c =='A' ) {
                c++; Text += ( Calendar.Hour < 12 ? "AM" : "PM" );
            }
            else if ( *c == 'a' ) {
                c++; Text += ( Calendar.Hour < 12 ? "am" : "pm" );
            }
            else {
                Text += *c; c++;
            }
        }
        TMap.Set(Text);
    }

    void UIFormDateInput::TextToTimeData() {
        if ( Text.empty() ) {
            TimeDataToText();
            return;
        }
        // Roughly parse the text to get the date
        const char * c = Text.c_str();
        int y = -1, m = -1, d = -1, H = -1, M = -1, A = -1;
        int count; 
        int num = -1;
        string tmp = "";
        int dlmCount = 0;
        while ( *c != 0 ) {
            if      ( *c == 'J' || *c == 'j' ) {    // January, June, or July
                c++; num = -1; if (m!=-1) { continue; } m = 0; 
                if ( *c == 'U' || *c == 'u' ) {
                    c++; m = 5; if ( *c == 'L' || *c == 'l' ) { m = 6; c++; }
                }
            }
            else if ( *c == 'F' || *c == 'f' ) { c++; num = -1; if (m!=-1) { continue; } m = 1; }  // February
            else if ( *c == 'M' || *c == 'm' ) {    // March or May
                c++; num = -1; if (m!=-1) { continue; } m = 2;
                if ( *c == 'A' || *c == 'a' ) {
                    c++; if ( *c == 'Y' || *c == 'y' ) { m = 4; c++; }
                }
                
            }
            else if ( *c == 'A' || *c == 'a' ) {    // April or August
                c++; num = -1;
                if (m!=-1) {
                    if ( H != -1 ) { // AM time
                        if ( *c == 'M' || *c == 'm' ) { A = 0; c++; }
                    }
                    continue;
                }
                m = 3;
                if ( *c == 'U' || *c == 'u' ) { m = 8; c++; }                
            }
            else if ( *c == 'S' || *c == 's' ) { c++; num = -1; if (m!=-1) { continue; } m = 8; }  // September
            else if ( *c == 'O' || *c == 'o' ) { c++; num = -1; if (m!=-1) { continue; } m = 9; } // October
            else if ( *c == 'N' || *c == 'n' ) { c++; num = -1; if (m!=-1) { continue; } m = 10; }  // November
            else if ( *c == 'D' || *c == 'd' ) { c++; num = -1; if (m!=-1) { continue; } m = 11; }  // December
            else if ( *c == 'P' || *c == 'p' ) { // PM time
                c++;
                if ( H != -1 ) {
                    if ( *c == 'M' || *c == 'm' ) { A = 1; c++; }
                }
            }
            else if ( *c >= '0' && *c <= '9' ) {
                count = 0; tmp.clear();
                while ( *c >= '0' && *c <= '9' ) { tmp += *c; count++; c++; }
                num = strtol( tmp.c_str(), NULL, 10 );
            }
            else if ( *c == '/' || *c == '-' ) {
                if      ( m == -1 ) { 
                    if ( num == -1 && dlmCount == 0 ) { // If no number, use the existing month
                        m = Calendar.Month;
                    }
                    else if ( num > 12 ) {  // If number is too large to be a month, then assume the it is a year
                        if ( y == -1 ) { 
                            if      ( num < 50 ) { y = 2000 + num; }
                            else if ( num < 100 ) { y = 1900 + num; }
                            else { y = num; }
                        }
                    }
                    else {                  // Otherwise, it is a month
                        m = std::max(1, num );
                    }
                }
                else if ( d == -1 ) { 
                    if ( num == -1 && dlmCount == 1 ) { // If no number, use the existing day
                        d = Calendar.Day;
                    }
                    else if ( num > 31 ) {  // If number is too large to be a day, then assume the it is a year
                        if ( y == -1 ) { 
                            if ( num < 50 ) { y = 2000 + num; }
                            else if ( num < 100 ) { y = 1900 + num; }
                            else { y = num; }
                        }
                    }
                    else {                  // Otherwise, it is a day.
                        d = num;
                    }
                }
                else if ( y == -1 ) { 
                    if ( num == -1 && dlmCount >=2 ) { y = Calendar.Year; }
                    else if ( num < 50 ) { y = 2000 + num; }
                    else if ( num < 100 ) { y = 1900 + num; }
                    else { y = num; }
                }
                c++; num = -1; dlmCount++;
            }
            else if ( *c == ':' ) {
                if ( H == -1 ) { 
                    if ( num == -1 ) { H = Calendar.Hour; }
                    else if ( num >= 0 && num <= 23 ) { H = num; }
                    else { H = 0; }
                }
                else if ( M == -1 ) { 
                    if ( num == -1 ) { M = Calendar.Minute; }
                    else if ( num >= 0 && num <= 59 ) { M = num; }
                    else { M = 0; }
                }
                c++; num = -1;
            }
            else if ( *c == ' ' || *c == ',' ) {
                if ( num != -1 ) {
                    if ( num > 31 ) { // If number is too large to be a day or month, then assume the it is a year.
                        if ( y == -1 ) { 
                            if ( num < 50 ) { y = 2000 + num; }
                            else if ( num < 100 ) { y = 1900 + num; }
                            else { y = num; }
                        }
                    }
                    else if ( m == -1 ) { 
                        if ( num > 12 ) {  // If number is too large to be a month, then assume the it is a day
                            if ( d == -1 ) { 
                                d = num;
                            }
                        }
                        else {                  // Otherwise, it is a month
                            m = std::max(1, num );
                        }
                    }
                    else if ( d == -1 ) { 
                        d = num;
                    }
                    else if ( y == -1 ) { 
                        if ( num < 50 ) { y = 2000 + num; }
                        else if ( num < 100 ) { y = 1900 + num; }
                        else { y = num; }
                    }
                    else if ( M == -1 ) { 
                        if ( num == -1 ) { M = Calendar.Minute; }
                        else if ( num >= 0 && num <= 59 ) { M = num; }
                        else { M = 0; }
                    }
                }
                c++; num = -1;
            }
            else {
                c++; num = -1;
            }
        }

        // Catch last num (if exists)
        if ( num != -1 ) {
            // If number is too large to be a month, then assume the it is a year.
            if ( num > 12 ) {
                if ( y == -1 ) { 
                    if ( num < 50 ) { y = 2000 + num; }
                    else if ( num < 100 ) { y = 1900 + num; }
                    else { y = num; }
                }
            }
            // Otherwise, it is a month (1-12)
            else if ( m == -1 ) {
                m = std::max(1, num );
            }
        }
        
        if ( y < 0 ) { y = Calendar.Year; }
        if ( m < 0 ) { m = Calendar.Month; } else if ( m < 1 ) { m = 1; } else if ( m > 12 ) { m = 12; }
        if ( Calendar.ShowTime ) {
            if ( H < 0 ) { H = Calendar.Hour; } else if ( H > 23 ) { H = 23; }
            if ( M < 0 ) { M = Calendar.Minute; } else if ( M > 59 ) { M = 59; }
            if      ( A == 0 ) { if ( H >  11 ) { H -= 12; } }
            else if ( A == 1 ) { if ( H <= 11 ) { H += 12; } }
        }
        else {
            H = 0; M = 0;
        }
        int nbDaysInMonth = Calendar.DaysInMonth( m, Calendar.IsLeapYear(y) );
        if ( d < 0 ) { d = Calendar.Day; } else if ( d < 1 ) { d = 1; } else if ( d > nbDaysInMonth ) { d = nbDaysInMonth; }
        Calendar.SetDate( y, m, d, H, M );

        // std::cout << "Parsed Date: " << m << "/" << d << "/" << y << " " << H << ":" << M << std::endl;
        // std::cout << "Set Date:    " << Calendar.Month << "/" << Calendar.Day << "/" << Calendar.Year << " " << Calendar.Hour << ":" << Calendar.Minute << std::endl;
        
    }

    // UIFormParagraphInput --------------------------------------------------------------------------------

    UIFormParagraphInput::UIFormParagraphInput( const string & inText, bool inViewOnly, bool inWordWrap ) : 
        UIFormItem(uif_paragraphinput), 
        TMap( inViewOnly ? UITextMap::ModeType::tm_viewer : UITextMap::ModeType::tm_editor, inText ) {
        TMap.AutoFocus = false;
        TFormat.Wrap = inWordWrap;
    };

    UIFormParagraphInput::~UIFormParagraphInput() {
        if ( Focused ) { TMap.SetFocus(false); }
    }

    void UIFormParagraphInput::Draw( float inX, float inY, bool clickMute ) {
        if ( Dim.x == 0 ) { return; }
        if ( Dim.y == 0 ) { return; }

        vec2 mPos = GetGlobalMousePos();
        ImRect rect = GetContentRect( inX, inY );
        UIFormItem::BoxFormat f = Default;

        ImRect textRect = GetGlobalBB(rect.Min.x+5, rect.Min.y, rect.GetWidth()-5, rect.GetHeight());
        
        if ( Focused ) { 
            f = GetBoxFormat( Focus, Default );
            rect.Expand(1);
        }

        if ( Rounding > 0 ) {
            if ( f.BkgdColor  >= 0 ) { if ( f.BkgdAlpha   >  0 ) { UIDrawAlphaRoundCornerBB(rect, f.BkgdColor, f.BkgdAlpha, Rounding, CornerBits); } }
            if ( f.BorderSize >  0 ) { if ( f.BorderColor >= 0 ) { UIDrawRoundBBOutlineCorner(rect, f.BorderColor, Rounding, f.BorderSize, CornerBits); } }
        }
        else {
            if ( f.BkgdColor  >= 0 ) { if ( f.BkgdAlpha   >  0 ) { UIDrawAlphaBB(rect, f.BkgdColor, f.BkgdAlpha); } }
            if ( f.BorderSize >  0 ) { if ( f.BorderColor >= 0 ) { UIDrawBBOutline(rect, f.BorderColor, f.BorderSize); } }
        }

        TFormat.Align = { 0, 0 };
        TFormat.Syntax[0].Index = f.Font;
        TFormat.Syntax[0].Scale = f.FontScale;
        TFormat.Syntax[0].Color = f.FontColor;
        TMap.UpdateFormat( TFormat );

        TMap.Draw( textRect, rect );

        if( MouseDown() ) { 
            if( VecInBBClip(mPos, rect) ) {
                if( DragStart() && !clickMute && !Mute ){   // Clicking on the box enters text input mode
                    if ( !Focused ) {
                        SetFocus( true ); 
                    }
                }    
            }
            else if ( DragStart() ) {
                EscKeyFocus(); // Otherwise, exit focus (like the escape key)
            }
        }

    }

    void UIFormParagraphInput::UpdateDim( float inW, float inH ) {
        UIFormItem::UpdateDim( inW, inH );
    }

    vec2 UIFormParagraphInput::GetAutoDim() {
        vec2 dim = { 100, 20 };
        dim.x = GetTotalWithPadding( PadLeft, PadRight, dim.x );
        dim.y = GetTotalWithPadding( PadTop, PadBottom, dim.y );
        return dim;
    }

    bool UIFormParagraphInput::SetFocus( bool inFocus, int inDirection, bool force ) {
        if ( Mute ) { return Focused; }
        if ( Focused == inFocus ) { return Focused; }
        Focused = inFocus;
        TMap.SetFocus( Focused );
        return Focused;
    }
    
    bool UIFormParagraphInput::GetFocus() {
        return Focused;
    }

    bool UIFormParagraphInput::RevertFocus() {
        return SetFocus( !Focused, 1, true );
    }

    bool UIFormParagraphInput::EscKeyFocus() {
        // Escape key exits focus
        return SetFocus( false );
    }

    bool UIFormParagraphInput::EnterKeyFocus() {
        // Enter key exits focus in view mode, otherwise do nothing since it is a new line in editor mode.
        if ( TMap.Mode == UITextMap::ModeType::tm_viewer ) {
            return SetFocus( false );
        }
        return Focused;
    } 

    bool UIFormParagraphInput::ProcessKeyDown( int inKey ) {
        if ( !TMap.Focused() ) { return false; }
        if ( inKey == 256 ) {                       // Esc Key handled by parent form
            return false;
        }
        else if ( inKey == 257 || inKey == 335 ) {  // Enter Key handled by parent form only in Viewer mode
            if ( TMap.Mode == UITextMap::ModeType::tm_viewer ) { return false; }
        }
        else if ( inKey == 258 ) {                  // Tab key handled by parent form only in Viewer mode
            if ( TMap.Mode == UITextMap::ModeType::tm_viewer ) { return false; }
        }
        return true;    // All other keys handled by the TMap
    }

    bool UIFormParagraphInput::ScrollChange( float inX, float inY, float inChange ) {
        // Check whether the paragraph item gets the scroll change
        ImRect rect = GetContentRect( inX, inY );
        vec2 mPos = GetGlobalMousePos();
        if ( VecInBBClip( mPos, rect ) ) {
            UILibIS->ScrollY = inChange; // Set the mouse scroll back to UILib. Scroll will be handled later in TMap draw.
            return true;
        }
        return false;
    }

    // ============================================================================================ //
    //                                         IMAGE ITEMS                                          //
    // ============================================================================================ //

    // UIFormImage --------------------------------------------------------------------------------

    UIFormImage::UIFormImage( string inPath, vec2 inImageDim, vec4 inPixelXYWH ) : 
        UIFormItem(uif_image), Path( inPath ), TextureId( -1 ), ImageDim( inImageDim ) {
        UpdatePixels( inPixelXYWH );
    }

    UIFormImage::UIFormImage( int inTextureId, vec2 inImageDim, vec4 inPixelXYWH ) : 
        UIFormItem(uif_image), Path( "" ), TextureId( inTextureId ), ImageDim( inImageDim ) {
        UpdatePixels( inPixelXYWH );
    }

    void UIFormImage::UpdatePixels( const vec4 & inPixelXYWH ) {
        if ( inPixelXYWH == vec4(0) ) {
            ImRect PixelRect = GetBB( 0, 0, ImageDim.x, ImageDim.y );
            PixelDim = ImageDim;
            PixelUV = GetUV( PixelRect, ImageDim );
        }
        else {
            ImRect PixelRect = GetBB( inPixelXYWH[0], inPixelXYWH[1], inPixelXYWH[2], inPixelXYWH[3] );
            PixelDim = { inPixelXYWH[2], inPixelXYWH[3] };
            PixelUV = GetUV( PixelRect, ImageDim );
        }
    }

    void UIFormImage::Draw( float inX, float inY, bool clickMute ) {
        ImRect rect = GetContentRect( inX, inY );

        if ( ImageFit == imgfit_uniformfill ) {
            ImGui::PushClipRect(rect.Min, rect.Max, true);
        }

        ImRect imageRect = GetImageRect( rect );
        int texId = ( TextureId > -1 ) ? TextureId : UIGetImageIndex( Path );
        UIImage( imageRect, texId, PixelUV );
        
        if ( ImageFit == imgfit_uniformfill ) {
            ImGui::PopClipRect();
        }
    }
    
    vec2 UIFormImage::GetAutoDim() {
        vec2 dim = PixelDim * Scale;
        dim.x = GetTotalWithPadding( PadLeft, PadRight, dim.x );
        dim.y = GetTotalWithPadding( PadTop, PadBottom, dim.y );
        return dim;
    }

    ImRect UIFormImage::GetImageRect( ImRect inContentRect  ) {
        if ( ImageFit == imgfit_fill ) { return inContentRect; }

        float contentW = inContentRect.GetWidth();
        float contentH = inContentRect.GetHeight();

        float imageX = inContentRect.Min.x;
        float imageY = inContentRect.Min.y;
        float imageW = PixelDim.x * Scale;
        float imageH = PixelDim.y * Scale;

        if ( ImageFit == imgfit_uniform ) {
            float imageAspect = PixelDim.x / PixelDim.y;
            imageW = contentW;
            imageH = imageW / imageAspect;
            if ( imageH > contentH ) {
                imageH = contentH;
                imageW = imageH * imageAspect;
            }
        }
        else if ( ImageFit == imgfit_uniformfill ) {
            float imageAspect = PixelDim.x / PixelDim.y;
            imageW = contentW;
            imageH = imageW / imageAspect;
            if ( imageH < contentH ) {
                imageH = contentH;
                imageW = imageH * imageAspect;
            }
        }

        imageX += ( contentW - imageW ) * Alignment.x;
        imageY += ( contentH - imageH ) * Alignment.y;

        return GetBB( imageX, imageY, imageW, imageH );
    }

    // ============================================================================================ //
    //                                         BUTTON ITEMS                                         //
    // ============================================================================================ //

    // UIFormButton -------------------------------------------------------------------------------

    void UIFormButton::Draw( float inX, float inY, bool clickMute ) {
        if ( Dim.x == 0 ) { return; }
        if ( Dim.y == 0 ) { return; }

        vec2 mPos = GetGlobalMousePos();
        ImRect rect = GetContentRect( inX, inY );
        Click = false;
        UIFormItem::BoxFormat f = Default;

        if ( Default.BkgdColor < 0 && Default.BorderSize < 0 ) { DrawNullButton( rect ); }

        if( VecInBBClip(mPos, rect) && !clickMute ) {
            if( MouseDown() ) { f = GetBoxFormat( Down, Default ); if( DragStart() && !clickMute ){ Click = true; } }
            else{ f = GetBoxFormat( Over, Default ); }
        } 
        else if ( Focused ) { auto over = GetBoxFormat( Over, Default ); f = GetBoxFormat( Focus, over ); rect.Expand(1); }

        if (f.ShadowBlurDist > 0 && f.ShadowAlpha > 0){
            UIDropShadow(rect,f.ShadowBlurDist,f.ShadowAlpha,f.ShadowRes,f.ShadowColor,f.ShadowXO,f.ShadowYO);
        }

        if ( f.BkgdColor >= 0 ) { UIDrawAlphaRoundCornerBB(rect, f.BkgdColor, f.BkgdAlpha, Rounding, CornerBits); }
        if ( f.BorderSize > 0 ) { UIDrawRoundBBOutlineCorner(rect, f.BorderColor, Rounding, f.BorderSize, CornerBits); }

    }
    
    void UIFormButton::ResetDraw() {
        Click = false;
    }

    // UIFormButton -------------------------------------------------------------------------------

    void UIFormSelButton::Draw( float inX, float inY, bool clickMute ) {
        if ( Dim.x == 0 ) { return; }
        if ( Dim.y == 0 ) { return; }

        vec2 mPos = GetGlobalMousePos();
        ImRect rect = GetContentRect( inX, inY );
        Click = false;
        UIFormItem::BoxFormat f = Default;
        
        if ( Selected ) { 
            f = GetBoxFormat( Select, Default );
        }
        
        if( VecInBBClip(mPos, rect) && !clickMute ) {
            if( MouseDown() ) { f = GetBoxFormat( Down, f ); if( DragStart() && !clickMute ){ Click = true; Selected = !Selected; } }
            else{ f = GetBoxFormat( Over, f ); }
        } 
        else if ( Focused ) { auto over = GetBoxFormat( Over, f ); f = GetBoxFormat( Focus, over ); rect.Expand(1); }

        if (f.ShadowBlurDist > 0 && f.ShadowAlpha > 0){
            UIDropShadow(rect,f.ShadowBlurDist,f.ShadowAlpha,f.ShadowRes,f.ShadowColor,f.ShadowXO,f.ShadowYO);
        }

        if ( Rounding > 0 ) {
            if ( f.BkgdColor  >= 0 ) { if ( f.BkgdAlpha   >  0 ) { UIDrawAlphaRoundCornerBB(rect, f.BkgdColor, f.BkgdAlpha, Rounding, CornerBits); } }
            if ( f.BorderSize >  0 ) { if ( f.BorderColor >= 0 ) { UIDrawRoundBBOutlineCorner(rect, f.BorderColor, Rounding, f.BorderSize, CornerBits); } }
        }
        else {
            if ( f.BkgdColor  >= 0 ) { if ( f.BkgdAlpha   >  0 ) { UIDrawAlphaBB(rect, f.BkgdColor, f.BkgdAlpha); } }
            if ( f.BorderSize >  0 ) { if ( f.BorderColor >= 0 ) { UIDrawBBOutline(rect, f.BorderColor, f.BorderSize); } }
        }
        
    }
    
    // UIFormImageButton --------------------------------------------------------------------------

    UIFormImageButton::UIFormImageButton( bool & inClick, string inPath, vec2 inImageDim, vec4 inPixelXYWH ) : 
        UIFormButton( inClick ),
        Image( inPath, inImageDim, inPixelXYWH ) {
        ItemType = uif_imagebutton;
    }

    UIFormImageButton::UIFormImageButton( bool & inClick, int inTextureId, vec2 inImageDim, vec4 inPixelXYWH ) : 
        UIFormButton( inClick ),
        Image( inTextureId, inImageDim, inPixelXYWH ) {
        ItemType = uif_imagebutton;
    }

    void UIFormImageButton::Draw( float inX, float inY, bool clickMute ) {
        if ( Dim.x == 0 ) { return; }
        if ( Dim.y == 0 ) { return; }

        vec2 mPos = GetGlobalMousePos();
        ImRect rect = GetContentRect( inX, inY );
        Click = false;
        UIFormItem::BoxFormat f = Default;

        if ( ImageOrder == 0 ) {
            Image.Draw( rect.Min.x + Image.Margin[0], rect.Min.y + Image.Margin[2], clickMute );
        }
        
        if( VecInBBClip(mPos, rect) ) {
            if( MouseDown() ) { f = GetBoxFormat( Down, Default ); if( DragStart() && !clickMute ){ Click = true; } }
            else{ f = GetBoxFormat( Over, Default ); }
        } 
        else if ( Focused ) { auto over = GetBoxFormat( Over, Default ); f = GetBoxFormat( Focus, over ); rect.Expand(1); }

        if (f.ShadowBlurDist > 0 && f.ShadowAlpha > 0){
            UIDropShadow(rect,f.ShadowBlurDist,f.ShadowAlpha,f.ShadowRes,f.ShadowColor,f.ShadowXO,f.ShadowYO);
        }

        if ( Rounding > 0 ) {
            if ( f.BkgdColor >= 0 ) { if ( f.BkgdAlpha > 0 ) { UIDrawAlphaRoundCornerBB(rect, f.BkgdColor, f.BkgdAlpha, Rounding, CornerBits); } }
        }
        else {
            if ( f.BkgdColor >= 0 ) { if ( f.BkgdAlpha > 0 ) { UIDrawAlphaBB(rect, f.BkgdColor, f.BkgdAlpha); } }
        }
        
        if ( ImageOrder == 1 ) {
            Image.Draw( rect.Min.x + Image.Margin[0], rect.Min.y + Image.Margin[2], clickMute );
        }

        if ( Rounding > 0 ) {
            if ( f.BorderSize > 0 ) { if ( f.BorderColor >= 0 ) { UIDrawRoundBBOutlineCorner(rect, f.BorderColor, Rounding, f.BorderSize, CornerBits); } }
        }
        else {
            if ( f.BorderSize > 0 ) { if ( f.BorderColor >= 0 ) { UIDrawBBOutline(rect, f.BorderColor, f.BorderSize); } }
        }

        // UIDrawBBOutline( rect, 0x0000FF, 3 );

    }

    void UIFormImageButton::UpdateDim( float inW, float inH ) {
        UIFormItem::UpdateDim( inW, inH );
        float contentW = Dim.x-Padding[0]-Padding[1];
        float contentH = Dim.y-Padding[2]-Padding[3];
        vec4 & im = Image.SetMargin( contentW, contentH );
        Image.UpdateDim( contentW-im[0]-im[1], contentH-im[2]-im[3] );
    }

    vec2 UIFormImageButton::GetAutoDim() {

        vec2 mDim = {0,0};
        vec2 mPct = {0,0};
        DecodeValueAdd( Image.MarginLeft,   mDim.x, mPct.x );
        DecodeValueAdd( Image.MarginRight,  mDim.x, mPct.x );
        DecodeValueAdd( Image.MarginTop,    mDim.y, mPct.y );
        DecodeValueAdd( Image.MarginBottom, mDim.y, mPct.y );

        vec2 dim = Image.GetAutoDim();

        vec2 itDim = {0,0};
        vec2 itPct = {0,0};

        if ( Image.W != "" ) { DecodeValueAdd( Image.W, itDim.x, itPct.x ); } else { itDim.x += dim.x; }
        if ( Image.H != "" ) { DecodeValueAdd( Image.H, itDim.y, itPct.y ); } else { itDim.y += dim.y; }

        vec2 rmdPct = vec2(1) - itPct - mPct;
        vec2 totalDim;
        totalDim.x = ( rmdPct.x <= 0 ) ? ( itDim.x + mDim.x ) : ( ( itDim.x + mDim.x ) / rmdPct.x );
        totalDim.y = ( rmdPct.y <= 0 ) ? ( itDim.y + mDim.y ) : ( ( itDim.y + mDim.y ) / rmdPct.y );
        totalDim.x = GetTotalWithPadding( PadLeft, PadRight, totalDim.x );
        totalDim.y = GetTotalWithPadding( PadTop, PadBottom, totalDim.y );
        
        return totalDim;
    }

    // UIFormSelImageButton --------------------------------------------------------------------------

    void UIFormSelImageButton::Draw( float inX, float inY, bool clickMute ) {
        if ( Dim.x == 0 ) { return; }
        if ( Dim.y == 0 ) { return; }

        vec2 mPos = GetGlobalMousePos();
        ImRect rect = GetContentRect( inX, inY );
        Click = false;
        UIFormItem::BoxFormat f = Default;

        if ( ImageOrder == 0 ) { Image.Draw( rect.Min.x + Image.Margin[0], rect.Min.y + Image.Margin[2], clickMute ); }
        
        if ( Selected ) { 
            f = GetBoxFormat( Select, f );
        }

        if( VecInBBClip(mPos, rect) ) {
            if( MouseDown() ) { f = GetBoxFormat( Down, f ); if( DragStart() && !clickMute ){ Click = true; Selected = !Selected; } }
            else{ f = GetBoxFormat( Over, f ); }
        } 
        else if ( Focused ) { auto over = GetBoxFormat( Over, f ); f = GetBoxFormat( Focus, over ); rect.Expand(1); }

        if (f.ShadowBlurDist > 0 && f.ShadowAlpha > 0){
            UIDropShadow(rect,f.ShadowBlurDist,f.ShadowAlpha,f.ShadowRes,f.ShadowColor,f.ShadowXO,f.ShadowYO);
        }

        if ( Rounding > 0 ) {
            if ( f.BkgdColor >= 0 ) { if ( f.BkgdAlpha > 0 ) { UIDrawAlphaRoundCornerBB(rect, f.BkgdColor, f.BkgdAlpha, Rounding, CornerBits); } }
        }
        else {
            if ( f.BkgdColor >= 0 ) { if ( f.BkgdAlpha > 0 ) { UIDrawAlphaBB(rect, f.BkgdColor, f.BkgdAlpha); } }
        }
        
        if ( ImageOrder == 1 ) { Image.Draw( rect.Min.x + Image.Margin[0], rect.Min.y + Image.Margin[2], clickMute ); }

        if ( Rounding > 0 ) {
            if ( f.BorderSize > 0 ) { if ( f.BorderColor >= 0 ) { UIDrawRoundBBOutlineCorner(rect, f.BorderColor, Rounding, f.BorderSize, CornerBits); } }
        }
        else {
            if ( f.BorderSize > 0 ) { if ( f.BorderColor >= 0 ) { UIDrawBBOutline(rect, f.BorderColor, f.BorderSize); } }
        }

    }

    // UIFormTextButton ---------------------------------------------------------------------------
    
    void UIFormTextButton::Draw( float inX, float inY, bool clickMute ) {
        if ( Dim.x == 0 ) { return; }
        if ( Dim.y == 0 ) { return; }

        vec2 mPos = GetGlobalMousePos();
        ImRect view = GetItemRect( inX, inY );
        ImRect rect = GetContentRect( inX, inY );
        Click = false;
        UIFormItem::BoxFormat f = Default;

        if( VecInBBClip(mPos, rect) ) {
            if( MouseDown() ) { f = GetBoxFormat( Down, Default ); if( DragStart() && !clickMute ){ Click = true; } }
            else{ f = GetBoxFormat( Over, Default ); }
        } 
        else if ( Focused ) { auto over = GetBoxFormat( Over, Default ); f = GetBoxFormat( Focus, over ); rect.Expand(1); }

        if (f.ShadowBlurDist > 0 && f.ShadowAlpha > 0){
            UIDropShadow(rect,f.ShadowBlurDist,f.ShadowAlpha,f.ShadowRes,f.ShadowColor,f.ShadowXO,f.ShadowYO);
        }

        if ( Rounding > 0 ) {
            if ( f.BkgdColor  >= 0 ) { if ( f.BkgdAlpha   >  0 ) { UIDrawAlphaRoundCornerBB(view, f.BkgdColor, f.BkgdAlpha, Rounding, CornerBits); } }
            if ( f.BorderSize >  0 ) { if ( f.BorderColor >= 0 ) { UIDrawRoundBBOutlineCorner(view, f.BorderColor, Rounding, f.BorderSize, CornerBits); } }
        }
        else {
            if ( f.BkgdColor  >= 0 ) { if ( f.BkgdAlpha   >  0 ) { UIDrawAlphaBB(view, f.BkgdColor, f.BkgdAlpha); } }
            if ( f.BorderSize >  0 ) { if ( f.BorderColor >= 0 ) { UIDrawBBOutline(view, f.BorderColor, f.BorderSize); } }
        }

        if ( Text != "" ) {
            if ( Multiline ) {
                UITextAlignedG(rect.Min.x, rect.Min.y, rect.GetWidth(), rect.GetHeight(), Text, f.FontColor, f.Font, Alignment, true, (f.FontUnderline>0),f.FontScale );
            } else {
                UIAddTextWithFontBB(rect, Text, f.FontColor, f.Font, Alignment, f.FontScale );
                if( f.FontUnderline > 0 ) {
                    UIAddTextUnderline( rect, Text, f.FontColor, f.Font, Alignment, f.FontUnderline, f.FontScale );
                }
            }
        }
    }

    vec2 UIFormTextButton::GetAutoDim() {
        vec2 dim = UIGetTextDim( Text, Default.Font, Default.FontScale);
        float maxUSize = std::max( { Default.FontUnderline, Over.FontUnderline, Down.FontUnderline, Focus.FontUnderline } );
        if ( maxUSize > 0 ) { dim.y += maxUSize + 1.0f; }   // need extra space to for the underline, otherwise the it gets clipped
        dim.x = GetTotalWithPadding( PadLeft, PadRight, dim.x );
        dim.y = GetTotalWithPadding( PadTop, PadBottom, dim.y );
        return dim;
    }

    // UIFormCollapserButton ---------------------------------------------------------------------------
    
    void UIFormCollapserButton::Draw( float inX, float inY, bool clickMute ) {
        if ( Dim.x == 0 ) { return; }
        if ( Dim.y == 0 ) { return; }

        SyncState();

        vec2 mPos = GetGlobalMousePos();
        ImRect rect = GetContentRect( inX, inY );
        Click = false;
        UIFormItem::BoxFormat f = Default;
        
        if( VecInBBClip(mPos, rect) ) {
            if( MouseDown() ) { f = GetBoxFormat( Down, Default ); if( DragStart() && !clickMute ){ Click = true; ToggleState(); } }
            else{ f = GetBoxFormat( Over, Default ); }
        } 
        else if ( Focused ) { auto over = GetBoxFormat( Over, Default ); f = GetBoxFormat( Focus, over ); rect.Expand(1); }
        
        if ( f.BkgdColor >= 0 && f.BkgdAlpha > 0 ) {
            if ( Type == caret_left ) {
                UIAddTriangleGlobal( rect.GetCenter(), TriangleDim.x, TriangleDim.y, (IsOpen ? 'd' : 'r' ), f.BkgdColor );
            }
            if ( Type == caret_right ) {
                UIAddTriangleGlobal( rect.GetCenter(), TriangleDim.x, TriangleDim.y, (IsOpen ? 'd' : 'u' ), f.BkgdColor );
            }
        }

    }

    vec2 UIFormCollapserButton::GetAutoDim() {
        float maxBH = std::max( TriangleDim.x, TriangleDim.y );
        vec2 dim = vec2( maxBH, maxBH );
        dim.x = GetTotalWithPadding( PadLeft, PadRight, dim.x );
        dim.y = GetTotalWithPadding( PadTop, PadBottom, dim.y );
        return dim;
    }

    void UIFormCollapserButton::SetCollapse( bool inCollapse ) {
        Collapse =  inCollapse;
        SetItemsCollapse( Collapse ? true : !IsOpen );
    }

    // UIFormSelTextButton --------------------------------------------------------------------------

    void UIFormSelTextButton::Draw( float inX, float inY, bool clickMute ) {
        if ( Dim.x == 0 ) { return; }
        if ( Dim.y == 0 ) { return; }

        vec2 mPos = GetGlobalMousePos();
        ImRect rect = GetContentRect( inX, inY );
        Click = false;
        UIFormItem::BoxFormat f = Default;
        
        if ( Selected ) { 
            f = GetBoxFormat( Select, Default );
        }
        
        if( VecInBBClip(mPos, rect) ) {
            if( MouseDown() ) { f = GetBoxFormat( Down, f ); if( DragStart() && !clickMute ){ Click = true; Selected = !Selected; } }
            else{ f = GetBoxFormat( Over, f ); }
        } 
        else if ( Focused ) { auto over = GetBoxFormat( Over, f ); f = GetBoxFormat( Focus, over ); rect.Expand(1); }

        if (f.ShadowBlurDist > 0 && f.ShadowAlpha > 0){
            UIDropShadow(rect,f.ShadowBlurDist,f.ShadowAlpha,f.ShadowRes,f.ShadowColor,f.ShadowXO,f.ShadowYO);
        }

        if ( Rounding > 0 ) {
            if ( f.BkgdColor  >= 0 ) { if ( f.BkgdAlpha   >  0 ) { UIDrawAlphaRoundCornerBB(rect, f.BkgdColor, f.BkgdAlpha, Rounding, CornerBits); } }
            if ( f.BorderSize >  0 ) { if ( f.BorderColor >= 0 ) { UIDrawRoundBBOutlineCorner(rect, f.BorderColor, Rounding, f.BorderSize, CornerBits); } }
        }
        else {
            if ( f.BkgdColor  >= 0 ) { if ( f.BkgdAlpha   >  0 ) { UIDrawAlphaBB(rect, f.BkgdColor, f.BkgdAlpha); } }
            if ( f.BorderSize >  0 ) { if ( f.BorderColor >= 0 ) { UIDrawBBOutline(rect, f.BorderColor, f.BorderSize); } }
        }

        if ( Text != "" ) {
            ImRect rect = GetContentRect( inX, inY );
            if ( Multiline ) {
                UITextAlignedG(rect.Min.x, rect.Min.y, rect.GetWidth(), rect.GetHeight(), Text, f.FontColor, f.Font, Alignment, true, (f.FontUnderline>0),f.FontScale );
            } else {
                UIAddTextWithFontBB(rect, Text, f.FontColor, f.Font, Alignment );
                if( f.FontUnderline > 0 ) {
                    UIAddTextUnderline( rect, Text, f.FontColor, f.Font, Alignment, f.FontUnderline );
                }
            }
        }
    }

    // UIFormButtonRow --------------------------------------------------------------------------
    
    UIFormButtonRow::~UIFormButtonRow() {
        for ( auto b : Buttons ) { delete b; }
        for ( auto c : ClickList ) { delete c; }
    }

    void UIFormButtonRow::Draw( float inX, float inY, bool clickMute ) {

        ImRect viewRect = GetItemRect( inX, inY );
        ImRect contentRect = GetContentRect( inX, inY );

        float xPos = 0, yPos = 0;
        xPos += Padding[0];
        yPos += Padding[2];

        float totalW = ContentDim.x + Padding[0] + Padding[1];
        bool scrollOn = ( totalW > Dim.x + .001f ) && AllowScroll;
        float xShift = 0;

        if ( scrollOn ) {
            ImGui::PushClipRect(viewRect.Min, viewRect.Max, true);
            float offScreen = totalW - Dim.x;
            xShift = -ScrollPerc * offScreen;
        }

        for ( int i = 0; i < Buttons.size(); i++ ) {

            auto it = Buttons[i];

            // Set item position within content rectangle
            it->Pos.x = xPos + it->Margin[0];
            it->Pos.y = yPos + it->Margin[2] + ( contentRect.GetHeight() - it->Dim.y - it->Margin[2] - it->Margin[3] ) * 0.5f;
            it->Draw( viewRect.Min.x + xShift, viewRect.Min.y, clickMute );

            // Update the running X position
            xPos += it->Margin[0] + it->Dim.x + it->Margin[1] + SpacerSize;
            
        }

        if ( scrollOn ) { 
            UIAddScrollerH(viewRect.Min.x, viewRect.Max.y-ScrollTrackSize, Dim.x, ScrollTrackSize, totalW, &ScrollPerc, &ScrollDragging, UILibCTX->DefaultScroller);
            ImGui::PopClipRect(); 
        }

    }

    void UIFormButtonRow::UpdateDim( float inW, float inH ) {
        if ( Dim == vec2( inW, inH ) ) { return; }
        UIFormItem::UpdateDim( inW, inH );

        float contentW = Dim.x - Padding[0] - Padding[1];
        float contentH = Dim.y - Padding[2] - Padding[3];
        ContentDim = {0, 0};

        float totalAvailable = contentW;

        for ( int i = 0; i < Buttons.size(); i++ ) {

            auto it = Buttons[i];

            float w = 0, h = 0;

            // Calculate known values. Set any unspecified widths and heights using AutoDim
            if ( it->W != "" ) { w = DecodeValue( it->W, contentW ); } else { w = it->GetAutoDim().x; }
            if ( it->H != "" ) { h = DecodeValue( it->H, contentH ); } else { h = it->GetAutoDim().y; }

            // Check Min / Max sizes for buttons
            w = std::max( it->MinDim.x, std::min( it->MaxDim.x, w ) );
            h = std::max( it->MinDim.y, std::min( it->MaxDim.y, h ) );

            // Subtract item width
            totalAvailable -= w;

            // Subtract item margin left / right
            totalAvailable -= ( it->Margin[0] + it->Margin[1] );

            // Update total ContentDim
            ContentDim.x += it->Margin[0] + w + it->Margin[1];
            ContentDim.y = std::max( ContentDim.y, it->Margin[2] + h + it->Margin[3] );

            // Set button dimensions
            it->UpdateDim( w, h );
            
        }

        //  Get spacer size and add any remainder to ContentDim
        SpacerSize    = ( totalAvailable > 0 ) ? totalAvailable / (Buttons.size()-1) : 0; 
        ContentDim.x += ( totalAvailable > 0 ) ? totalAvailable : 0;

    }

    vec2 UIFormButtonRow::GetAutoDim() {

        vec2 itDim = {0,0};
        vec2 itPct = {0,0};

        vec2 mDim = {0,0};
        vec2 mPct = {0,0};

        for ( int i = 0; i < Buttons.size(); i++ ) {

            auto it = Buttons[i];

            vec2 cmDim = {0,0};
            vec2 cmPct = {0,0};
            DecodeValueAdd( it->MarginLeft,   cmDim.x, cmPct.x );
            DecodeValueAdd( it->MarginRight,  cmDim.x, cmPct.x );
            DecodeValueAdd( it->MarginTop,    cmDim.y, cmPct.y );
            DecodeValueAdd( it->MarginBottom, cmDim.y, cmPct.y );

            mDim.x += cmDim.x;
            mPct.x += cmPct.x;
            mDim.y = std::max( mDim.y, cmDim.y );
            mPct.y = std::max( mPct.y, cmPct.y );

            vec2 dim = it->GetAutoDim();

            vec2 citDim = {0,0};
            vec2 citPct = {0,0};

            if ( it->W != "" ) { DecodeValueAdd( it->W, citDim.x, citPct.x ); } else { citDim.x += dim.x; }
            if ( it->H != "" ) { DecodeValueAdd( it->H, citDim.y, citPct.y ); } else { citDim.y += dim.y; }

            itDim.x += citDim.x;
            itPct.x += citPct.x;
            itDim.y = std::max( itDim.y, citDim.y );
            itPct.y = std::max( itPct.y, citPct.y );


        }

        vec2 rmdPct = vec2(1) - itPct - mPct;
        vec2 totalDim;
        totalDim.x = ( rmdPct.x <= 0 ) ? ( itDim.x + mDim.x ) : ( ( itDim.x + mDim.x ) / rmdPct.x );
        totalDim.y = ( rmdPct.y <= 0 ) ? ( itDim.y + mDim.y ) : ( ( itDim.y + mDim.y ) / rmdPct.y );
        totalDim.x = GetTotalWithPadding( PadLeft, PadRight, totalDim.x );
        totalDim.y = GetTotalWithPadding( PadTop, PadBottom, totalDim.y );

        return totalDim;

    }

    ImRect UIFormButtonRow::GetFocusedRect( float inX, float inY ) {
        if ( FocusedItem > -1 ) {
            auto it = Buttons[FocusedItem];
            return it->GetFocusedRect( inX+Pos.x, inY+Pos.y );
        }
        return UIFormItem::GetFocusedRect( inX, inY ); 
    }

    bool UIFormButtonRow::ScrollChange( float inX, float inY, float inChange ) {
        // Check whether the button row item gets the scroll change
        if ( !AllowScroll ) { return false; }
        if ( !UILibIS->Shift ) { return false; }    // SHIFT + Mouse Wheel for horizontal scrolling
        ImRect itemBB = GetItemRect( inX, inY );
        vec2 mPos = GetGlobalMousePos();
        if ( VecInBBClip( mPos, itemBB ) ) {
            float OffScreenWidth = ContentDim.x + Padding[0] + Padding[1] - Dim.x;
            float newPerc = ScrollPerc + inChange * (UILibCTX->DefaultScrollPixels/OffScreenWidth) * -UILibCTX->DefaultScrollUnits;
            if(newPerc > 1){newPerc = 1;}
            if(newPerc < 0){newPerc = 0;}
            ScrollPerc = newPerc;
            return true;
        }
        return false;
    }

    UIFormImageButton* UIFormButtonRow::AddImageButton( int inTextureId, vec2 inImageDim, vec4 inPixelXYWH  ) {
        int itemIndex = (int) Buttons.size();
        bool * Click = new bool( false );
        ClickList.push_back( Click );
        UIFormImageButton* item = new UIFormImageButton( *Click, inTextureId, inImageDim, inPixelXYWH );
        item->Index = itemIndex;
        Buttons.push_back( item );
        return item;
    }

    UIFormImageButton* UIFormButtonRow::AddImageButton( string inPath, vec2 inImageDim, vec4 inPixelXYWH  ) {
        int itemIndex = (int) Buttons.size();
        bool * Click = new bool( false );
        ClickList.push_back( Click );
        UIFormImageButton* item = new UIFormImageButton( *Click, inPath, inImageDim, inPixelXYWH );
        item->Index = itemIndex;
        Buttons.push_back( item );
        return item;
    }

    UIFormTextButton* UIFormButtonRow::AddTextButton( string inText ) {
        int itemIndex = (int) Buttons.size();
        bool * Click = new bool( false );
        ClickList.push_back( Click );
        UIFormTextButton * item = new UIFormTextButton( *Click, inText );
        item->Index = itemIndex;
        Buttons.push_back( item );
        return item;
    }

    UIFormSelTextButton* UIFormButtonRow::AddSelTextButton( string inText ) {
        int itemIndex = (int) Buttons.size();
        bool * Click = new bool( false );
        ClickList.push_back( Click );
        UIFormSelTextButton * item = new UIFormSelTextButton( *Click, inText );
        item->Index = itemIndex;
        Buttons.push_back( item );
        return item;
    }

    // UIFormToggleButtons -------------------------------------------------------------------------

    UIFormToggleButtons::UIFormToggleButtons( int & inButtonIndex, bool & inClick, vector<string> & inItemList) : 
        UIFormItem(uif_togglebuttons), SelectedIndex( inButtonIndex ), Click( inClick ), ItemList(inItemList) {
        Format.BorderSize  = 1.0f;
        Format.BorderColor = 0xABABAB;
        Format.BkgdColor   = 0xABABAB;
        Format.BkgdAlpha   = 1;
        Format.Font        = 0;
        Format.FontColor   = 0xABABAB;
        SelectedIndex = 0;
    };

    void UIFormToggleButtons::Draw( float inX, float inY, bool clickMute ) {

        Click = false;

        ImRect contentRect = GetContentRect( inX, inY );

        float xPos = contentRect.Min.x;
        float yPos = contentRect.Min.y;
        vec2 mPos = GetGlobalMousePos();

        float bHeight = contentRect.GetHeight();
        float pxShift = Format.BorderSize * 0.5f;

        for ( int i = 0; i < ItemList.size(); i++ ) {
            float bWidth = Spacer * 2.0f + UIGetTextWidth( ItemList[i], Format.Font, Format.FontScale ) + pxShift;
            ImRect bRect = GetBB(xPos, yPos, bWidth, bHeight);
            int fontClr = Format.FontColor;
            if(SelectedIndex == i){fontClr = 0xFFFFFF;}
            if(i==0){
                if(SelectedIndex == i){
                    UIDrawAlphaRoundCornerBB(bRect, Format.BkgdColor, Format.BkgdAlpha, Round, "0101");
                }
                UIDrawRoundBBOutlineCorner(bRect, Format.BorderColor, Round, Format.BorderSize, "0101");
            }
            else if(i==(int)ItemList.size()-1){
                if(SelectedIndex == i){
                    UIDrawAlphaRoundCornerBB(bRect, Format.BkgdColor, Format.BkgdAlpha, Round, "1010");
                }
                UIDrawRoundBBOutlineCorner(bRect, Format.BorderColor, Round, Format.BorderSize, "1010");
            }
            else{
                if(SelectedIndex == i){
                    UIDrawAlphaBB(bRect, Format.BkgdColor, Format.BkgdAlpha);
                }
                UIDrawBBOutline(bRect, Format.BorderColor, Format.BorderSize);
            }
            UIAddTextWithFontBB( bRect, ItemList[i], fontClr, Format.Font, vec2(0.5f, 0.45f) );
            xPos += bWidth - pxShift;

            if ( !clickMute && DragStart() && VecInBBClip(mPos,bRect) ){
                SelectedIndex = i;
                Click = true;
            }

        }


    }

    void UIFormToggleButtons::ResetDraw() {
        Click = false;
    }

    vec2 UIFormToggleButtons::GetAutoDim() {
        float totalW = 0;
        for ( int i = 0; i < ItemList.size(); i++ ) {
            totalW += Spacer * 2.0f + UIGetTextWidth( ItemList[i], Format.Font, Format.FontScale );
        }
        float totalH = UIGetTextHeight("T", Format.Font, Format.FontScale);
        return  { totalW, totalH };
    }

    UIFormToggleButtons* UIForm::AddToggleButtons( int & inButtonIndex, bool & inClick, vector<string> & inItemList ) {
        UIFormToggleButtons* item = new UIFormToggleButtons( inButtonIndex,  inClick, inItemList );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }

    // UIFormSelButtonRow --------------------------------------------------------------------------
    
    UIFormSelButtonRow::~UIFormSelButtonRow() {
        for ( auto b : ImageButtons ) { delete b; }
        for ( auto b : TextButtons ) { delete b; }
        for ( auto c : ClickList ) { delete c; }
    }

    void UIFormSelButtonRow::Draw( float inX, float inY, bool clickMute ) {

        ImRect viewRect = GetItemRect( inX, inY );
        ImRect contentRect = GetContentRect( inX, inY );

        float xPos = 0, yPos = 0;
        xPos += Padding[0];
        yPos += Padding[2];

        float totalW = ContentDim.x + Padding[0] + Padding[1];
        bool scrollOn = ( totalW > Dim.x + .001f ) && AllowScroll;
        float xShift = 0;

        if ( scrollOn ) {
            ImGui::PushClipRect(viewRect.Min, viewRect.Max, true);
            float offScreen = totalW - Dim.x;
            xShift = -ScrollPerc * offScreen;
        }

        for ( int i = 0; i < ButtonIndex.size(); i++ ) {

            if ( ButtonType[i] == 0 ) {     // ----- Image Button -----

                auto it = ImageButtons[ButtonIndex[i]];

                // Set selected button
                it->Selected = ( i == SelectedIndex ) ? true : false;

                // Set item position within content rectangle
                it->Pos.x = xPos + it->Margin[0];
                it->Pos.y = yPos + it->Margin[2] + ( contentRect.GetHeight() - it->Dim.y - it->Margin[2] - it->Margin[3] ) * 0.5f;
                it->Draw( viewRect.Min.x + xShift, viewRect.Min.y, clickMute );

                // Update selected button on click
                if ( it->Click ) { SelectedIndex = i; }

                // Update the running X position
                xPos += it->Margin[0] + it->Dim.x + it->Margin[1] + SpacerSize;

            }
            else {                          // ----- Text Button -----

                auto it = TextButtons[ButtonIndex[i]];

                // Set selected button
                it->Selected = ( i == SelectedIndex ) ? true : false;

                // Set item position within content rectangle
                it->Pos.x = xPos + it->Margin[0];
                it->Pos.y = yPos + it->Margin[2] + ( contentRect.GetHeight() - it->Dim.y - it->Margin[2] - it->Margin[3] ) * 0.5f;
                it->Draw( viewRect.Min.x + xShift, viewRect.Min.y, clickMute );

                // Update selected button on click
                if ( it->Click ) { SelectedIndex = i; }

                // Update the running X position
                xPos += it->Margin[0] + it->Dim.x + it->Margin[1] + SpacerSize;

            }
            
        }

        if ( scrollOn ) { 
            UIAddScrollerH(viewRect.Min.x, viewRect.Max.y-ScrollTrackSize, Dim.x, ScrollTrackSize, totalW, &ScrollPerc, &ScrollDragging, UILibCTX->DefaultScroller);
            ImGui::PopClipRect(); 
        }


    }

    void UIFormSelButtonRow::UpdateDim( float inW, float inH ) {
        if ( Dim == vec2( inW, inH ) ) { return; }
        UIFormItem::UpdateDim( inW, inH );

        float contentW = Dim.x - Padding[0] - Padding[1];
        float contentH = Dim.y - Padding[2] - Padding[3];
        ContentDim = {0, 0};

        float totalAvailable = contentW;

        for ( int i = 0; i < ButtonIndex.size(); i++ ) {

            if ( ButtonType[i] == 0 ) {     // ----- Image Button -----

                auto it = ImageButtons[ButtonIndex[i]];

                float w = 0, h = 0;

                // Calculate known values. Set any unspecified widths and heights using AutoDim
                if ( it->W != "" ) { w = DecodeValue( it->W, contentW ); } else { w = it->GetAutoDim().x; }
                if ( it->H != "" ) { h = DecodeValue( it->H, contentH ); } else { h = it->GetAutoDim().y; }

                // Check Min / Max sizes for buttons
                w = std::max( it->MinDim.x, std::min( it->MaxDim.x, w ) );
                h = std::max( it->MinDim.y, std::min( it->MaxDim.y, h ) );

                // Subtract item width
                totalAvailable -= w;

                // Subtract item margin left / right
                totalAvailable -= ( it->Margin[0] + it->Margin[1] );

                // Update total ContentDim
                ContentDim.x += it->Margin[0] + w + it->Margin[1];
                ContentDim.y = std::max( ContentDim.y, it->Margin[2] + h + it->Margin[3] );

                // Set button dimensions
                it->UpdateDim( w, h );

            }
            else {                          // ----- Text Button -----

                auto it = TextButtons[ButtonIndex[i]];

                float w = 0, h = 0;

                // Calculate known values. Set any unspecified widths and heights using AutoDim
                if ( it->W != "" ) { w = DecodeValue( it->W, contentW ); } else { w = it->GetAutoDim().x; }
                if ( it->H != "" ) { h = DecodeValue( it->H, contentH ); } else { h = it->GetAutoDim().y; }

                // Check Min / Max sizes for buttons
                w = std::max( it->MinDim.x, std::min( it->MaxDim.x, w ) );
                h = std::max( it->MinDim.y, std::min( it->MaxDim.y, h ) );

                // Subtract item width
                totalAvailable -= w;

                // Subtract item margin left / right
                totalAvailable -= ( it->Margin[0] + it->Margin[1] );

                // Update total ContentDim
                ContentDim.x += it->Margin[0] + w + it->Margin[1];
                ContentDim.y = std::max( ContentDim.y, it->Margin[2] + h + it->Margin[3] );

                // Set button dimensions
                it->UpdateDim( w, h );

            }
            
        }

        //  Get spacer size and add any remainder to ContentDim
        SpacerSize    = ( totalAvailable > 0 ) ? totalAvailable / (ButtonIndex.size()-1) : 0; 
        ContentDim.x += ( totalAvailable > 0 ) ? totalAvailable : 0;

    }

    vec2 UIFormSelButtonRow::GetAutoDim() {

        vec2 itDim = {0,0};
        vec2 itPct = {0,0};

        vec2 mDim = {0,0};
        vec2 mPct = {0,0};

        for ( int i = 0; i < ButtonIndex.size(); i++ ) {

            if ( ButtonType[i] == 0 ) {     // ----- Image Button -----

                auto it = ImageButtons[ButtonIndex[i]];

                vec2 cmDim = {0,0};
                vec2 cmPct = {0,0};
                DecodeValueAdd( it->MarginLeft,   cmDim.x, cmPct.x );
                DecodeValueAdd( it->MarginRight,  cmDim.x, cmPct.x );
                DecodeValueAdd( it->MarginTop,    cmDim.y, cmPct.y );
                DecodeValueAdd( it->MarginBottom, cmDim.y, cmPct.y );

                mDim.x += cmDim.x;
                mPct.x += cmPct.x;
                mDim.y = std::max( mDim.y, cmDim.y );
                mPct.y = std::max( mPct.y, cmPct.y );

                vec2 dim = it->GetAutoDim();

                vec2 citDim = {0,0};
                vec2 citPct = {0,0};

                if ( it->W != "" ) { DecodeValueAdd( it->W, citDim.x, citPct.x ); } else { citDim.x += dim.x; }
                if ( it->H != "" ) { DecodeValueAdd( it->H, citDim.y, citPct.y ); } else { citDim.y += dim.y; }

                itDim.x += citDim.x;
                itPct.x += citPct.x;
                itDim.y = std::max( itDim.y, citDim.y );
                itPct.y = std::max( itPct.y, citPct.y );

            }
            else {                          // ----- Text Button -----

                auto it = TextButtons[ButtonIndex[i]];

                vec2 cmDim = {0,0};
                vec2 cmPct = {0,0};
                DecodeValueAdd( it->MarginLeft,   cmDim.x, cmPct.x );
                DecodeValueAdd( it->MarginRight,  cmDim.x, cmPct.x );
                DecodeValueAdd( it->MarginTop,    cmDim.y, cmPct.y );
                DecodeValueAdd( it->MarginBottom, cmDim.y, cmPct.y );

                mDim.x += cmDim.x;
                mPct.x += cmPct.x;
                mDim.y = std::max( mDim.y, cmDim.y );
                mPct.y = std::max( mPct.y, cmPct.y );

                vec2 dim = it->GetAutoDim();

                vec2 citDim = {0,0};
                vec2 citPct = {0,0};

                if ( it->W != "" ) { DecodeValueAdd( it->W, citDim.x, citPct.x ); } else { citDim.x += dim.x; }
                if ( it->H != "" ) { DecodeValueAdd( it->H, citDim.y, citPct.y ); } else { citDim.y += dim.y; }

                itDim.x += citDim.x;
                itPct.x += citPct.x;
                itDim.y = std::max( itDim.y, citDim.y );
                itPct.y = std::max( itPct.y, citPct.y );

            }

        }

        vec2 rmdPct = vec2(1) - itPct - mPct;
        vec2 totalDim;
        totalDim.x = ( rmdPct.x <= 0 ) ? ( itDim.x + mDim.x ) : ( ( itDim.x + mDim.x ) / rmdPct.x );
        totalDim.y = ( rmdPct.y <= 0 ) ? ( itDim.y + mDim.y ) : ( ( itDim.y + mDim.y ) / rmdPct.y );
        totalDim.x = GetTotalWithPadding( PadLeft, PadRight, totalDim.x );
        totalDim.y = GetTotalWithPadding( PadTop, PadBottom, totalDim.y );

        return totalDim;

    }

    ImRect UIFormSelButtonRow::GetFocusedRect( float inX, float inY ) {
        if ( FocusedItem > -1 ) {
            if ( ButtonType[FocusedItem] == 0 ) {     // ----- Image Button -----
                auto it = ImageButtons[ButtonIndex[FocusedItem]];
                return it->GetFocusedRect( inX+Pos.x, inY+Pos.y );
            }
            else {                          // ----- Text Button -----
                auto it = TextButtons[ButtonIndex[FocusedItem]];
                return it->GetFocusedRect( inX+Pos.x, inY+Pos.y );
            }
        }
        return UIFormItem::GetFocusedRect( inX, inY ); 
    }

    void UIFormSelButtonRow::ResetDraw() {
        for ( auto b : ImageButtons ) { b->ResetDraw(); }
        for ( auto b : TextButtons  ) { b->ResetDraw(); }
    }

    bool UIFormSelButtonRow::ScrollChange( float inX, float inY, float inChange ) {
        // Check whether the button row item gets the scroll change
        if ( !AllowScroll ) { return false; }
        if ( !UILibIS->Shift ) { return false; }    // SHIFT + Mouse Wheel for horizontal scrolling
        ImRect itemBB = GetItemRect( inX, inY );
        vec2 mPos = GetGlobalMousePos();
        if ( VecInBBClip( mPos, itemBB ) ) {
            float OffScreenWidth = ContentDim.x + Padding[0] + Padding[1] - Dim.x;
            float newPerc = ScrollPerc + inChange * (UILibCTX->DefaultScrollPixels/OffScreenWidth) * -UILibCTX->DefaultScrollUnits;
            if(newPerc > 1){newPerc = 1;}
            if(newPerc < 0){newPerc = 0;}
            ScrollPerc = newPerc;
            return true;
        }
        return false;
    }

    UIFormSelImageButton* UIFormSelButtonRow::AddSelImageButton( int inTextureId, vec2 inImageDim, vec4 inPixelXYWH  ) {
        int itemIndex = (int) ButtonIndex.size();
        bool * Click = new bool( false );
        ClickList.push_back( Click );
        UIFormSelImageButton* item = new UIFormSelImageButton( *Click, inTextureId, inImageDim, inPixelXYWH );
        item->Index = itemIndex;
        ButtonIndex.push_back( (int) ImageButtons.size() );
        ButtonType.push_back( 0 );
        ImageButtons.push_back( item );
        return item;
    }

    UIFormSelImageButton* UIFormSelButtonRow::AddSelImageButton( string inPath, vec2 inImageDim, vec4 inPixelXYWH  ) {
        int itemIndex = (int) ButtonIndex.size();
        bool * Click = new bool( false );
        ClickList.push_back( Click );
        UIFormSelImageButton* item = new UIFormSelImageButton( *Click, inPath, inImageDim, inPixelXYWH );
        item->Index = itemIndex;
        ButtonIndex.push_back( (int) ImageButtons.size() );
        ButtonType.push_back( 0 );
        ImageButtons.push_back( item );
        return item;
    }

    UIFormSelTextButton* UIFormSelButtonRow::AddSelTextButton( string inText ) {
        int itemIndex = (int) ButtonIndex.size();
        bool * Click = new bool( false );
        ClickList.push_back( Click );
        UIFormSelTextButton * item = new UIFormSelTextButton( *Click, inText );
        item->Index = itemIndex;
        ButtonIndex.push_back( (int) TextButtons.size() );
        ButtonType.push_back( 1 );
        TextButtons.push_back( item );
        return item;
    }

    // UIFormSelButtonGrid --------------------------------------------------------------------------
    
    UIFormSelButtonGrid::~UIFormSelButtonGrid() {
        for ( auto b : ImageButtons ) { delete b; }
        for ( auto b : TextButtons ) { delete b; }
        for ( auto c : ClickList ) { delete c; }
    }

    uvec2 UIFormSelButtonGrid::GetRowCol( int inIndex ) {
        // index is row major
        return uvec2( inIndex / GridSize.y, inIndex % GridSize.y );
    }

    void UIFormSelButtonGrid::Draw( float inX, float inY, bool clickMute ) {

        ImRect viewRect = GetItemRect( inX, inY );
        ImRect contentRect = GetContentRect( inX, inY );

        float xPos = 0, yPos = 0;
        xPos += Padding[0];
        yPos += Padding[2];

        for ( int i = 0; i < ButtonIndex.size(); i++ ) {

            uvec2 rc = GetRowCol( i );

            if ( ButtonType[i] == 0 ) {     // ----- Image Button -----

                auto it = ImageButtons[ButtonIndex[i]];

                // Set selected button
                it->Selected = ( i == SelectedIndex ) ? true : false;

                // Set item position within content rectangle (ignores button Margins)
                it->Pos.x = xPos + ( BlockSize.x - it->Dim.x - it->Margin[0] - it->Margin[1] ) * 0.5f;
                it->Pos.y = yPos + ( BlockSize.y - it->Dim.y - it->Margin[2] - it->Margin[3] ) * 0.5f;
                it->Draw( viewRect.Min.x, viewRect.Min.y, clickMute );

                // Update selected button on click
                if ( it->Click ) { SelectedIndex = i; }

                // Update the running X position
                xPos += BlockSize.x;

                // Update the running X & Y position if last element in row
                if ( rc.y == GridSize.y-1 ) { 
                    xPos = Padding[0];
                    yPos += BlockSize.y; 
                }

            }
            else {                          // ----- Text Button -----

                auto it = TextButtons[ButtonIndex[i]];

                // Set selected button
                it->Selected = ( i == SelectedIndex ) ? true : false;

                // Set item position within content rectangle (ignores button Margins)
                it->Pos.x = xPos + ( BlockSize.x - it->Dim.x - it->Margin[0] - it->Margin[1] ) * 0.5f;
                it->Pos.y = yPos + ( BlockSize.y - it->Dim.y - it->Margin[2] - it->Margin[3] ) * 0.5f;
                it->Draw( viewRect.Min.x, viewRect.Min.y, clickMute );

                // Update selected button on click
                if ( it->Click ) { SelectedIndex = i; }

                // Update the running X position
                xPos += BlockSize.x;

                // Update the running X & Y position if last element in row
                if ( rc.y == GridSize.y-1 ) { 
                    xPos = Padding[0];
                    yPos += BlockSize.y; 
                }

            }
            
        }

    }

    void UIFormSelButtonGrid::UpdateDim( float inW, float inH ) {
        if ( Dim == vec2( inW, inH ) ) { return; }
        UIFormItem::UpdateDim( inW, inH );

        float contentW = Dim.x - Padding[0] - Padding[1];
        float contentH = Dim.y - Padding[2] - Padding[3];
        
        // Set BlockSize from AutoDim
        ContentDim = GetAutoDim();
        ContentDim.x = std::max( contentW, std::min( contentW, ContentDim.x ) );
        ContentDim.y = std::max( contentH, std::min( contentH, ContentDim.y ) );
        BlockSize = { ContentDim.x / GridSize.y, ContentDim.y / GridSize.x };
      
        // Cycle through and update each individual button dimensions
        for ( int i = 0; i < ButtonIndex.size(); i++ ) {

            uvec2 rc = GetRowCol( i );

            if ( ButtonType[i] == 0 ) {     // ----- Image Button -----

                auto it = ImageButtons[ButtonIndex[i]];

                float w = 0, h = 0;

                // Calculate known values. Set any unspecified widths and heights using AutoDim
                if ( it->W != "" ) { w = DecodeValue( it->W, contentW ); } 
                else {
                    if (it->Expand <= 0 ) { w = it->GetAutoDim().x; }
                    else { w = it->Expand * BlockSize.x; }
                }
                if ( it->H != "" ) { h = DecodeValue( it->H, contentH ); } 
                else { 
                   if (it->Expand <= 0 ) {  h = it->GetAutoDim().y; }
                   else { h = it->Expand * BlockSize.y; }
                }

                // Check Min / Max sizes for buttons
                w = std::max( it->MinDim.x, std::min( it->MaxDim.x, w ) );
                h = std::max( it->MinDim.y, std::min( it->MaxDim.y, h ) );

                // Set button dimensions
                it->UpdateDim( w, h );

            }
            else {                          // ----- Text Button -----

                auto it = TextButtons[ButtonIndex[i]];

                float w = 0, h = 0;

                // Calculate known values. Set any unspecified widths and heights using AutoDim
                if ( it->W != "" ) { w = DecodeValue( it->W, contentW ); } 
                else {
                    if (it->Expand <= 0 ) { w = it->GetAutoDim().x; }
                    else { w = it->Expand * BlockSize.x; }
                }
                if ( it->H != "" ) { h = DecodeValue( it->H, contentH ); } 
                else { 
                   if (it->Expand <= 0 ) {  h = it->GetAutoDim().y; }
                   else { h = it->Expand * BlockSize.y; }
                }

                // Check Min / Max sizes for buttons
                w = std::max( it->MinDim.x, std::min( it->MaxDim.x, w ) );
                h = std::max( it->MinDim.y, std::min( it->MaxDim.y, h ) );

                // Set button dimensions
                it->UpdateDim( w, h );

            }
            
        }

    }

    vec2 UIFormSelButtonGrid::GetAutoDim() {
        // Calculates total dimension of grid based on the largest button dimensions in the grid.
        
        vec2 maxDim = {0,0};
        vec2 maxPct = {0,0};

        for ( int i = 0; i < ButtonIndex.size(); i++ ) {

            if ( ButtonType[i] == 0 ) {     // ----- Image Button -----

                auto it = ImageButtons[ButtonIndex[i]];

                vec2 cmDim = {0,0};
                vec2 cmPct = {0,0};
                DecodeValueAdd( it->MarginLeft,   cmDim.x, cmPct.x );
                DecodeValueAdd( it->MarginRight,  cmDim.x, cmPct.x );
                DecodeValueAdd( it->MarginTop,    cmDim.y, cmPct.y );
                DecodeValueAdd( it->MarginBottom, cmDim.y, cmPct.y );

                vec2 dim = it->GetAutoDim();
                dim.x = std::max( it->MinDim.x, std::min( it->MaxDim.x, dim.x ) );
                dim.y = std::max( it->MinDim.y, std::min( it->MaxDim.y, dim.y ) );

                vec2 citDim = {0,0};
                vec2 citPct = {0,0};

                if ( it->W != "" ) { DecodeValueAdd( it->W, citDim.x, citPct.x ); } else { citDim.x += dim.x; }
                if ( it->H != "" ) { DecodeValueAdd( it->H, citDim.y, citPct.y ); } else { citDim.y += dim.y; }

                maxDim.x = std::max( maxDim.x, citDim.x + cmDim.x );
                maxPct.x = std::max( maxPct.x, citPct.x + cmPct.x );
                maxDim.y = std::max( maxDim.y, citDim.y + cmDim.y );
                maxPct.y = std::max( maxPct.y, citPct.y + cmPct.y );

            }
            else {                          // ----- Text Button -----

                auto it = TextButtons[ButtonIndex[i]];

                vec2 cmDim = {0,0};
                vec2 cmPct = {0,0};
                DecodeValueAdd( it->MarginLeft,   cmDim.x, cmPct.x );
                DecodeValueAdd( it->MarginRight,  cmDim.x, cmPct.x );
                DecodeValueAdd( it->MarginTop,    cmDim.y, cmPct.y );
                DecodeValueAdd( it->MarginBottom, cmDim.y, cmPct.y );

                vec2 dim = it->GetAutoDim();
                dim.x = std::max( it->MinDim.x, std::min( it->MaxDim.x, dim.x ) );
                dim.y = std::max( it->MinDim.y, std::min( it->MaxDim.y, dim.y ) );

                vec2 citDim = {0,0};
                vec2 citPct = {0,0};

                if ( it->W != "" ) { DecodeValueAdd( it->W, citDim.x, citPct.x ); } else { citDim.x += dim.x; }
                if ( it->H != "" ) { DecodeValueAdd( it->H, citDim.y, citPct.y ); } else { citDim.y += dim.y; }

                maxDim.x = std::max( maxDim.x, citDim.x + cmDim.x );
                maxPct.x = std::max( maxPct.x, citPct.x + cmPct.x );
                maxDim.y = std::max( maxDim.y, citDim.y + cmDim.y );
                maxPct.y = std::max( maxPct.y, citPct.y + cmPct.y );

            }

        }

        vec2 totalDim;
        totalDim.x = maxDim.x * GridSize.y + SpacerSize.y * (GridSize.y-1);
        totalDim.y = maxDim.y * GridSize.x + SpacerSize.x * (GridSize.x-1);
        totalDim.x = GetTotalWithPadding( PadLeft, PadRight, totalDim.x );
        totalDim.y = GetTotalWithPadding( PadTop, PadBottom, totalDim.y );

        return totalDim;

    }

    ImRect UIFormSelButtonGrid::GetFocusedRect( float inX, float inY ) {
        if ( FocusedItem > -1 ) {
            if ( ButtonType[FocusedItem] == 0 ) {     // ----- Image Button -----
                auto it = ImageButtons[ButtonIndex[FocusedItem]];
                return it->GetFocusedRect( inX+Pos.x, inY+Pos.y );
            }
            else {                          // ----- Text Button -----
                auto it = TextButtons[ButtonIndex[FocusedItem]];
                return it->GetFocusedRect( inX+Pos.x, inY+Pos.y );
            }
        }
        return UIFormItem::GetFocusedRect( inX, inY ); 
    }

    void UIFormSelButtonGrid::ResetDraw() {
        for ( auto b : ImageButtons ) { b->ResetDraw(); }
        for ( auto b : TextButtons  ) { b->ResetDraw(); }
    }

    UIFormSelImageButton* UIFormSelButtonGrid::AddSelImageButton( int inTextureId, vec2 inImageDim, vec4 inPixelXYWH  ) {
        int itemIndex = (int) ButtonIndex.size();
        bool * Click = new bool( false );
        ClickList.push_back( Click );
        UIFormSelImageButton* item = new UIFormSelImageButton( *Click, inTextureId, inImageDim, inPixelXYWH );
        item->Index = itemIndex;
        ButtonIndex.push_back( (int) ImageButtons.size() );
        ButtonType.push_back( 0 );
        ImageButtons.push_back( item );
        return item;
    }

    UIFormSelImageButton* UIFormSelButtonGrid::AddSelImageButton( string inPath, vec2 inImageDim, vec4 inPixelXYWH  ) {
        int itemIndex = (int) ButtonIndex.size();
        bool * Click = new bool( false );
        ClickList.push_back( Click );
        UIFormSelImageButton* item = new UIFormSelImageButton( *Click, inPath, inImageDim, inPixelXYWH );
        item->Index = itemIndex;
        ButtonIndex.push_back( (int) ImageButtons.size() );
        ButtonType.push_back( 0 );
        ImageButtons.push_back( item );
        return item;
    }

    UIFormSelTextButton* UIFormSelButtonGrid::AddSelTextButton( string inText ) {
        int itemIndex = (int) ButtonIndex.size();
        bool * Click = new bool( false );
        ClickList.push_back( Click );
        UIFormSelTextButton * item = new UIFormSelTextButton( *Click, inText );
        item->Index = itemIndex;
        ButtonIndex.push_back( (int) TextButtons.size() );
        ButtonType.push_back( 1 );
        TextButtons.push_back( item );
        return item;
    }

    // ============================================================================================ //
    //                                        WIDGET ITEMS                                          //
    // ============================================================================================ //

    // UIFormSlider ------------------------------------------------------------------------------

    void UIFormSlider::Draw( float inX, float inY, bool clickMute ) {
        ImRect rect = GetContentRect( inX, inY );
        vec2 mPos = GetGlobalMousePos();
        bool sliderMute = ( MouseDown() ) ? clickMute || Mute : Mute;
        if ( Direction == 1 ) {
            UIAddSliderH(rect.Min.x, rect.Min.y, rect.GetWidth(), rect.GetHeight(), &Value, Bounds.x, Bounds.y, sliderMute, Change);
        }
        else {
            UIAddSliderV(rect.Min.x, rect.Min.y, rect.GetWidth(), rect.GetHeight(), &Value, Bounds.x, Bounds.y, sliderMute, Change);
        }
    }

    vec2 UIFormSlider::GetAutoDim() {
        vec2 dim = ( Direction == 0 ) ? vec2( 12, 0 ) : vec2( 0, 12 );
        dim.x = GetTotalWithPadding( PadLeft, PadRight, dim.x );
        dim.y = GetTotalWithPadding( PadTop, PadBottom, dim.y );
        return dim;
    }

    // UIFormLabeledSlider ------------------------------------------------------------------------------

    UIFormLabeledSlider::UIFormLabeledSlider( float & inValue, bool & inMute, bool & inChange, vec2 inBounds ) :
        Slider( inValue, inMute, inChange, inBounds), ValueLabel(ValueClick, FloatToString(inValue)), UIFormItem(uif_labeledslider) {
        ValueLabel.Default.BorderSize = 0;
        ValueLabel.Over.FontColor = 0x787878;
    }

    void UIFormLabeledSlider::Draw( float inX, float inY, bool clickMute ) {
        if ( Dim.x == 0 ) { return; }
        if ( Dim.y == 0 ) { return; }

        vec2 mPos = GetGlobalMousePos();
        
        ImRect viewRect = GetItemRect( inX, inY );
        ImRect contentRect = GetContentRect( inX, inY );

        ValueLabel.Text = FloatToString(Slider.Value);
        vector<UIFormItem*> Items = { &Slider, &ValueLabel };
        
        if ( Slider.Direction == 0 ) {         // ------------ vertical ------------

            float xPos = 0, yPos = 0;
            xPos += Padding[0];
            yPos += Padding[2];

            for ( auto it : Items ) { 

                // Set item position and draw if has non-zero dimensions
                vec2 & p = it->SetPos( xPos + it->Margin[0], yPos + it->Margin[2], contentRect.GetWidth(), contentRect.GetHeight(), Padding[0], Padding[2] );
                it->Draw( viewRect.Min.x, viewRect.Min.y, clickMute );

                // If the item position is not the same as the current running position, then set the
                // running position based off of the item position.
                if ( p.y != yPos + it->Margin[2] ) {    
                    yPos = std::max( yPos, p.y + it->Dim.y + it->Margin[3] );
                } else {
                    yPos += it->Margin[2] + it->Dim.y + it->Margin[3];
                }

                yPos += Spacer;

            }
        }
        else {                          // ------------ horizontal ------------

            float xPos = 0, yPos = 0;
            xPos += Padding[0];
            yPos += Padding[2];

            for ( auto it : Items ) { 

                // Set item position and draw if has non-zero dimensions
                vec2 & p = it->SetPos( xPos + it->Margin[0], yPos + it->Margin[2], contentRect.GetWidth(), contentRect.GetHeight(), Padding[0], Padding[2] );
                it->Draw( viewRect.Min.x, viewRect.Min.y, clickMute );

                // If the item position is not the same as the current running position, then set the
                // running position based off of the item position.
                if ( p.x != xPos + it->Margin[0] ) {    
                    xPos = std::max( xPos, p.x + it->Dim.x + it->Margin[1] );
                } else {
                    xPos += it->Margin[0] + it->Dim.x + it->Margin[1];
                }

                xPos += Spacer;

            }

        }
        
    }

    void UIFormLabeledSlider::UpdateDim( float inW, float inH ) {
        if ( Dim == vec2( inW, inH ) ) { return; }
        UIFormItem::UpdateDim( inW, inH );

        float contentW = Dim.x - Padding[0] - Padding[1];
        float contentH = Dim.y - Padding[2] - Padding[3];

        vector<UIFormItem*> Items = { &Slider, &ValueLabel };
        vector<vec2> itemDims( Items.size(), vec2(0) );

        float totalAvailable = ( Slider.Direction == 0 ) ? contentH : contentW;
        float totalExpand = 0;
        int numExpand = 0;

        totalAvailable -= (Items.size()-1)*Spacer;      // Subtract out spacers between labels and slider

        // Loop # 1 - Calculate known dimensions and sum of totals for Expand calculations in the next loop
        for ( int i = 0; i < Items.size(); i++ ) {

            auto it = Items[i];

            float w = 0, h = 0;

            // Calculate known values
            if ( it->W != "" ) { w = DecodeValue( it->W, contentW ); }
            if ( it->H != "" ) { h = DecodeValue( it->H, contentH ); }

            if ( Slider.Direction == 0 ) {     // ------------ vertical ------------
        
                // Set any unspecified widths and heights
                if ( it->H == "" ) {
                    if (it->Expand <= 0 ) { h = it->GetAutoDim().y; }
                    else { totalExpand += std::min(it->Expand, 1.0f); numExpand++; }
                }
                if ( it->W == "" ) { w = contentW - it->Margin[0] - it->Margin[1]; }

                // Subtract item height
                totalAvailable -= h;

                // Subtract item margin top / bottom, ignoring top margin if Y is explicitly placed
                totalAvailable -= ( it->Y != "" ) ? it->Margin[3] : ( it->Margin[2] + it->Margin[3] );

            } else {                    // ------------ horizontal ------------

                // Set any unspecified widths and heights
                if ( it->W == "" ) {
                    if( it->Expand <= 0 ) { 
                        w = ( i == (Items.size()-1) ) ? GetValueLabelWidth() : it->GetAutoDim().x;              // ValueLabel width based off of the widest possible number in the range
                    }
                    else { totalExpand += std::min(it->Expand, 1.0f); numExpand++; }
                }
                if ( it->H == "" ) { h = contentH - it->Margin[2] - it->Margin[3]; }

                // Subtract item width
                totalAvailable -= w;

                // Subtract item margin left / right, ignoring left margin if X is explicitly placed
                totalAvailable -= ( it->X != "" ) ? it->Margin[1] : ( it->Margin[0] + it->Margin[1] );
                
            }
            // std::cout << "UIFormLabeledSlider finished first loop " << i << ", dim: " << VecToString( {w, h} ) << std::endl;
            // Set known values so far
            itemDims[i] = { w, h };

        }

        // Loop # 2 - Fill in dimensions for Expand items and set final dimensions
        for ( int i = 0; i < Items.size(); i++ ) {

            auto it = Items[i];

            float w = itemDims[i].x, h = itemDims[i].y;

            if ( Slider.Direction == 0 ) {     // ------------ vertical ------------

                if ( it->H == "" ) {
                    if (it->Expand > 0 ) {
                        float expand = std::min(it->Expand, 1.0f);
                        h = ( totalAvailable > 0 ) ? std::min( expand/totalExpand, expand ) * totalAvailable : 0;   // If there is no more available space, don't draw
                    }
                }

                w = std::max( it->MinDim.x, std::min( it->MaxDim.x, w ) );
                h = std::max( it->MinDim.y, std::min( it->MaxDim.y, h ) );

            } else {                    // ------------ horizontal ------------

                if ( it->W == "" ) {
                    if (it->Expand > 0 ) {
                        float expand = std::min(it->Expand, 1.0f);
                        w = ( totalAvailable > 0 ) ? std::min( expand/totalExpand, expand ) * totalAvailable : 0;   // If there is no more available space, don't draw
                    }
                }

                w = std::max( it->MinDim.x, std::min( it->MaxDim.x, w ) );
                h = std::max( it->MinDim.y, std::min( it->MaxDim.y, h ) );

            }

            it->UpdateDim( w, h );      // Final update of dimensions

        }

    }

    vec2 UIFormLabeledSlider::GetAutoDim() {
        
        vector<UIFormItem*> Items = { &Slider, &ValueLabel };
        vector<vec2> mDim( Items.size(), vec2(0) );
        vector<vec2> mPct( Items.size(), vec2(0) );
        vector<vec2> itDim( Items.size(), vec2(0) );
        vector<vec2> itPct( Items.size(), vec2(0) );

        for ( int i = 0; i < Items.size(); i++ ) {
            auto it = Items[i];

            DecodeValueAdd( it->MarginLeft,   mDim[i].x, mPct[i].x );
            DecodeValueAdd( it->MarginRight,  mDim[i].x, mPct[i].x );
            DecodeValueAdd( it->MarginTop,    mDim[i].y, mPct[i].y );
            DecodeValueAdd( it->MarginBottom, mDim[i].y, mPct[i].y );

            vec2 dim = it->GetAutoDim();
            if ( i == (Items.size()-1) ) { dim.x = GetValueLabelWidth(); }       // ValueLabel width based off of the widest possible number in the range

            if ( it->W != "" ) { DecodeValueAdd( it->W, itDim[i].x, itPct[i].x ); } else { itDim[i].x += dim.x; }
            if ( it->H != "" ) { DecodeValueAdd( it->H, itDim[i].y, itPct[i].y ); } else { itDim[i].y += dim.y; }

        }
        
        // Combine
        vec2 mTotalDim = vec2(0), mTotalPct = vec2(0), itTotalDim = vec2(0), itTotalPct = vec2(0);
        if ( Slider.Direction == 0 ) {         // ------------ vertical ------------
            for ( int i = 0; i < Items.size(); i++ ) {
                mTotalDim.x = std::max( mTotalDim.x, mDim[i].x );
                mTotalPct.x = std::max( mTotalPct.x, mPct[i].x );
                mTotalDim.y += mDim[i].y;
                mTotalPct.y += mPct[i].y;

                itTotalDim.x = std::max( itTotalDim.x, itDim[i].x );
                itTotalPct.x = std::max( itTotalPct.x, itPct[i].x );
                itTotalDim.y += itDim[i].y;
                itTotalPct.y += itPct[i].y;
            }  
        }
        else {                          // ------------ horizontal ------------
            for ( int i = 0; i < Items.size(); i++ ) {
                mTotalDim.y = std::max( mTotalDim.y, mDim[i].y );
                mTotalPct.y = std::max( mTotalPct.y, mPct[i].y );
                mTotalDim.x += mDim[i].x;
                mTotalPct.x += mPct[i].x;

                itTotalDim.y = std::max( itTotalDim.y, itDim[i].y );
                itTotalPct.y = std::max( itTotalPct.y, itPct[i].y );
                itTotalDim.x += itDim[i].x;
                itTotalPct.x += itPct[i].x;
            }  
        }

        vec2 rmdPct = vec2(1) - itTotalPct - mTotalPct;
        vec2 totalDim;
        totalDim.x = ( rmdPct.x <= 0 ) ? ( itTotalDim.x + mTotalDim.x ) : ( ( itTotalDim.x + mTotalDim.x ) / rmdPct.x );
        totalDim.y = ( rmdPct.y <= 0 ) ? ( itTotalDim.y + mTotalDim.y ) : ( ( itTotalDim.y + mTotalDim.y ) / rmdPct.y );
        totalDim.x = GetTotalWithPadding( PadLeft, PadRight, totalDim.x );
        totalDim.y = GetTotalWithPadding( PadTop, PadBottom, totalDim.y );

        return totalDim;
    }

    float UIFormLabeledSlider::GetValueLabelWidth() {
        return std::max( UIGetTextWidth(FloatToString( Slider.Bounds[0], Precision ), ValueLabel.Default.Font), 
                        UIGetTextWidth(FloatToString( Slider.Bounds[1], Precision ), ValueLabel.Default.Font) );   
    }
    
    // UIFormAnimationTimeline --------------------------------------------------------------------------------

    void UIFormAnimationTimeline::Draw( float inX, float inY, bool clickMute ) {
        ImRect rect = GetContentRect( inX, inY );
        vec2 mPos = GetGlobalMousePos();
        bool tMute = clickMute;
        vec2 tdim = ( Timeline.HideLast ) ? vec2(0) : UIGetTextDim( Timeline.LabelFunc( Timeline.EndVal ), Timeline.Font, Timeline.FontScale);
        UIAddTimelineSlider( rect.Min.x, rect.Min.y, rect.GetWidth()-Timeline.TextAlign.x-tdim.x, rect.GetHeight(), tMute, Timeline );
    }

    vec2 UIFormAnimationTimeline::GetAutoDim() {
        vec2 tdim = ( Timeline.HideLast ) ? vec2(0) : UIGetTextDim( Timeline.LabelFunc( Timeline.EndVal ), Timeline.Font, Timeline.FontScale);
        vec2 dim = vec2( Timeline.MarkerWidth+Timeline.TextAlign.x+tdim.x, std::max( std::max( Timeline.MajorTickSize, Timeline.MinorTickSize ), tdim.y ) );
        dim.x = GetTotalWithPadding( PadLeft, PadRight, dim.x );
        dim.y = GetTotalWithPadding( PadTop, PadBottom, dim.y );
        return dim;
    }


    // UIFormComboBox --------------------------------------------------------------------------------

    UIFormComboBox::UIFormComboBox( string inName, vector<string> & inItemList ) : 
        UIFormItem(uif_combobox), ItemList( inItemList ) { 
        ComboBox.Name = inName;
        MinDim = { 20, 16 }; 
    };


    void UIFormComboBox::Draw( float inX, float inY, bool clickMute ) {
        if ( Dim.x == 0 ) { return; }
        if ( Dim.y == 0 ) { return; }

        if ( Collapse ) { SetFocus( false ); }

        vec2 mPos = GetGlobalMousePos();
        int keyD = GetKeyDown();
        int keyChange = UILibIS->Key.Change;
        Clicked = false;

        ImRect rect = GetContentRect( inX, inY );
        UIFormItem::BoxFormat f = Default;

        if( MouseDown() ) { 
            if( VecInBBClip(mPos, rect) ) {
                if( DragStart() && !clickMute){ // Clicking on the box opens the Item List
                    if ( SetFocus( true ) ) {
                        float cbHeight = std::min(ItemList.size() * OpenCellH, OpenMaxH);
                        ImRect openBB = GetBB(rect.Min.x, rect.Max.y-1, rect.GetWidth(), cbHeight);
                        if ( UILibDS && openBB.Max.y > UILibDS->WindowSize.H ) {    // Flip combo box if it goes off window
                            openBB.TranslateY( -(cbHeight + rect.GetHeight() ) );
                        }
                        UIComboInitFocus( 2, &ComboBox, openBB, ItemList );
                        LastOverIndex = -1;
                    }
                    Clicked = true;
                }
            }
            else if ( Focused ) { // Clicking off closes the Item List
                if( DragStart() /* && !clickMute */ ) {
                    float cbHeight = std::min(ItemList.size() * OpenCellH, OpenMaxH);
                    ImRect openBB = GetBB(rect.Min.x, rect.Max.y-1, rect.GetWidth(), cbHeight);
                    if ( UILibDS && openBB.Max.y > UILibDS->WindowSize.H ) {    // Flip combo box if it goes off window
                        openBB.TranslateY( -(cbHeight + rect.GetHeight() ) );
                    }
                    if( !VecInBB(mPos, openBB) ){ SetFocus( false ); }
                }
            }                   
        }

        if ( Focused ) { 
            f = GetBoxFormat( Focus, Default ); 
            if ( ComboBox.Open && UIComboGetFocusedName() != ComboBox.Name ) {
                SetFocus( true );                     
                float cbHeight = std::min(ItemList.size() * OpenCellH, OpenMaxH);
                ImRect openBB = GetBB(rect.Min.x, rect.Max.y-1, rect.GetWidth(), cbHeight);
                UIComboInitFocus( 2, &ComboBox, openBB, ItemList );
                LastOverIndex = -1;
            }
        }

        if ( f.BkgdColor >= 0 ) { UIDrawAlphaBB(rect, f.BkgdColor, f.BkgdAlpha); }
        if ( f.BorderSize > 0 ) { UIDrawBBOutline(rect, f.BorderColor, f.BorderSize); }

        if ( ComboBox.Index > -1 ) {
            ImRect textRect = GetGlobalBB(rect.Min.x+5, rect.Min.y+1, rect.GetWidth()-5, rect.GetHeight()-1);
            UIAddTextWithFontBB(textRect, ItemList[ComboBox.Index], FontColor, Font, Alignment );
        }

        if ( ComboBox.Open == 0 ){
            UIAddTriangleGlobal( rect.Min.x + rect.GetWidth()-12, rect.Min.y + rect.GetHeight() * 0.5f, 4.5, true, 0x898989);
        }

    }

    vec2 UIFormComboBox::GetAutoDim(){ return vec2( 100, UIGetFontHeight(Font)+4 ); }

    bool UIFormComboBox::SetFocus( bool inFocus, int inDirection, bool force ) {
        if ( Mute ) { return false; }
        if ( Focused == inFocus ) { return Focused; }
        if ( Focused ) { if ( UIComboGetFocusedName() == ComboBox.Name ) { UIComboExitFocus(); } }
        Focused = inFocus;
        return Focused; 
    }

    bool UIFormComboBox::GetFocus() { 
        return Focused;  
    }

    bool UIFormComboBox::RevertFocus() {
        SetFocus( !Focused, 1, true );
        Focused = !Focused;
        return Focused;
    }

    bool UIFormComboBox::EnterKeyFocus() { 
        SetFocus( true );         
        if ( ComboBox.Open ) {
            if ( LastOverIndex > -1 ) { ComboBox.Index = LastOverIndex; ComboBox.Open = false; }    // keep focus, but close the box
        } else {
            ComboBox.Open = true;
        }
        return Focused;
    }

    bool UIFormComboBox::ArrowKeyFocus( int inKey ) {
        // 262 = Arrow Right, 263 = Arrow Left, 264 = Arrow Down, 265 = Arrow Up
        if ( inKey != 264 && inKey != 265 ) { return Focused; }
        SetFocus( true ); 
        if ( ComboBox.Open ) {
            if ( LastOverIndex == -1 ) { LastOverIndex = 0; }
            else {
                if ( inKey == 264 ) { LastOverIndex = std::min( (int) ItemList.size()-1, LastOverIndex+1 ); }
                else { LastOverIndex = std::max( 0, LastOverIndex-1 ); }
            }
            ScrollToOverIndex();
        } else {
            if ( inKey == 264 ) { ComboBox.Index = std::min( (int) ItemList.size()-1, ComboBox.Index+1 ); }
            else { ComboBox.Index = std::max( 0, ComboBox.Index-1 ); }
        }
        
        return Focused;
    }

    bool UIFormComboBox::GetOverlayState() { return ComboBox.Open; }

    vector<ImRect> UIFormComboBox::GetOverlayRects() {
        if ( UILibCTX->ComboMode != 2 ) { return {}; }
        if ( UIComboGetFocusedName() != ComboBox.Name ) { return {}; } 
        return { UILibCTX->ComboRect };
    }

    void UIFormComboBox::DrawOverlay( float inX, float inY ) { 
        if ( Dim.x == 0 ) { return; }
        if ( Dim.y == 0 ) { return; }

        if ( UILibCTX->ComboMode != 2 ) { return; }
        if ( UIComboGetFocusedName() != ComboBox.Name ) { return; }
        if ( !ComboBox.Open ) { return; }

        vec2 mPos = GetGlobalMousePos();

        ImRect viewRect = UILibCTX->ComboRect;
        float viewH = viewRect.GetHeight();
        float viewW = viewRect.GetWidth();

        UIFormItem::BoxFormat f = Default;
        if ( f.BkgdColor >= 0 ) { UIDrawAlphaBB(viewRect, f.BkgdColor, f.BkgdAlpha); }
        if ( f.BorderSize > 0 ) { UIDrawBBOutline(viewRect, f.BorderColor, f.BorderSize); }

        float xPos = viewRect.Min.x, yPos = viewRect.Min.y;

        float totalH = ( ItemList.size() ) * OpenCellH;
        bool scrollOn = ( totalH > viewH + 0.001f );
        float yShift = 0;
        float ScrollTrackSize = 10;
        
        int startRow = 0;
        int RowsPerBox = std::min( (int) ItemList.size()-1, (int) ( viewH / OpenCellH ) );
        float cellW = viewW;

        if ( scrollOn ) {
            ImGui::PushClipRect(viewRect.Min, viewRect.Max, true);
            float offScreen = totalH - viewH;
            yShift = -UILibCTX->ComboScrollPerc * offScreen;
            startRow = (int) floor( -yShift / OpenCellH );
            cellW -= ScrollTrackSize;
        }

        for ( int i = 0; i <= RowsPerBox; i++ ) { 
            int rowIndex = startRow + i;
            ImRect cell = GetBB( xPos, yPos + yShift + rowIndex*OpenCellH, cellW, OpenCellH );
            if( VecInBBClip(mPos, cell) ){
                if ( MouseDown() && !DragStart() ) {    // Don't highlight cells when you're dragging the scroll bar
                    ImRect textRect = GetBB(cell.Min.x+5, cell.Min.y+1, cellW-5, OpenCellH-1);
                    UIAddTextWithFontBB(textRect, ItemList[ rowIndex ], FontColor, Font, Alignment );
                }
                else {
                    // UIFormItem::BoxFormat overf = GetBoxFormat( OpenOver, Default ); 
                    // if ( overf.BkgdColor >= 0 ) { UIDrawAlphaBB(cell, overf.BkgdColor, overf.BkgdAlpha); }
                    // if ( overf.BorderSize > 0 ) { UIDrawBBOutline(cell, overf.BorderColor, overf.BorderSize); }
                    // ImRect textRect = GetBB(cell.Min.x+5, cell.Min.y+1, cellW-5, OpenCellH-1);
                    // UIAddTextWithFontBB(textRect, ItemList[ rowIndex ], FontColorOver, Font, Alignment );
                    LastOverIndex = rowIndex;
                }
                if( DragStart() ){
                    ComboBox.Index = rowIndex;
                    SetFocus( false );  // This will close the List, but keep the Focus on the combo box
                }
            }
            else{
                ImRect textRect = GetBB(cell.Min.x+5, cell.Min.y+1, cellW-5, OpenCellH-1);
                UIAddTextWithFontBB(textRect, ItemList[ rowIndex ], FontColor, Font, Alignment );
            }
            
        }

        if ( LastOverIndex > -1 ) {
            ImRect overCell =  GetBB( xPos, yPos + yShift + LastOverIndex*OpenCellH, cellW, OpenCellH );
            UIFormItem::BoxFormat overf = GetBoxFormat( OpenOver, Default ); 
            if ( overf.BkgdColor >= 0 ) { UIDrawAlphaBB(overCell, overf.BkgdColor, overf.BkgdAlpha); }
            if ( overf.BorderSize > 0 ) { UIDrawBBOutline(overCell, overf.BorderColor, overf.BorderSize); }
            ImRect textRect = GetBB(overCell.Min.x+5, overCell.Min.y+1, cellW-5, OpenCellH-1);
            UIAddTextWithFontBB(textRect, ItemList[ LastOverIndex ], FontColorOver, Font, Alignment );
        }

        if ( scrollOn ) { 
            UIAddScrollerV(viewRect.Max.x-ScrollTrackSize, viewRect.Min.y, ScrollTrackSize, viewH, totalH, &UILibCTX->ComboScrollPerc, &UILibCTX->ComboDragging, UILibCTX->DefaultScroller);
            ImGui::PopClipRect(); 
        }

    }

    void UIFormComboBox::ScrollToOverIndex() {
        if ( UILibCTX->ComboMode != 2 ) { return; }
        if ( UIComboGetFocusedName() != ComboBox.Name ) { return; }
        if ( !ComboBox.Open ) { return; }
        if ( LastOverIndex == -1 ) { return; }

        ImRect viewRect = UILibCTX->ComboRect;
        float viewH = viewRect.GetHeight();
        float viewW = viewRect.GetWidth();

        float xPos = viewRect.Min.x, yPos = viewRect.Min.y;
        
        float totalH = ( ItemList.size() ) * OpenCellH;
        bool scrollOn = ( totalH > viewH + 0.001f );
        if ( !scrollOn ) { return; }

        float offScreen = totalH - viewH;
        float yShift = -UILibCTX->ComboScrollPerc * offScreen;
        float cellW = viewW;

        ImRect overCell =  GetBB( xPos, yPos + yShift + LastOverIndex*OpenCellH, cellW, OpenCellH );

        float yDelta = 0;
        if ( overCell.Min.y < viewRect.Min.y ) { 
            yDelta = viewRect.Min.y - overCell.Min.y; 
            UILibCTX->ComboScrollPerc = ( -(yShift+yDelta) / offScreen );
        }
        else if ( overCell.Max.y > viewRect.Max.y ) {
            yDelta = overCell.Max.y - viewRect.Max.y; 
            UILibCTX->ComboScrollPerc = ( -(yShift-yDelta) / offScreen );
        }

    }

    void UIFormComboBox::ResetDraw() {
        ComboBox.Open = false;
        Clicked = false;
    };
    // UIFormLabeledItem --------------------------------------------------------------------------------

    UIFormLabeledItem::~UIFormLabeledItem() {
        if ( Item->ItemType == uif_custom ) { delete Item; }
    }

    void UIFormLabeledItem::Draw( float inX, float inY, bool clickMute ) {
        if ( Dim.x == 0 ) { return; }
        if ( Dim.y == 0 ) { return; }

        vec2 mPos = GetGlobalMousePos();
        
        ImRect viewRect = GetItemRect( inX, inY );
        ImRect contentRect = GetContentRect( inX, inY );

        vector<UIFormItem*> Items = { &Label, Item };
        
        if ( Direction % 2 == 0 ) {       // ------------ vertical ------------

            if (Direction == 2 ) Items = { Item, &Label };

            float xPos = 0, yPos = 0;
            xPos += Padding[0];
            yPos += Padding[2];

            for ( auto it : Items ) { 

                // Set item position and draw if has non-zero dimensions
                vec2 & p = it->SetPos( xPos + it->Margin[0], yPos + it->Margin[2], contentRect.GetWidth(), contentRect.GetHeight(), Padding[0], Padding[2] );
                it->Draw( viewRect.Min.x, viewRect.Min.y, clickMute );

                // If the item position is not the same as the current running position, then set the
                // running position based off of the item position.
                if ( p.y != yPos + it->Margin[2] ) {    
                    yPos = std::max( yPos, p.y + it->Dim.y + it->Margin[3] );
                } else {
                    yPos += it->Margin[2] + it->Dim.y + it->Margin[3];
                }

            }
        }
        else {                          // ------------ horizontal ------------

            if ( Direction == 3 ) Items = { Item, &Label };

            float xPos = 0, yPos = 0;
            xPos += Padding[0];
            yPos += Padding[2];

            for ( auto it : Items ) { 

                // Set item position and draw if has non-zero dimensions
                vec2 & p = it->SetPos( xPos + it->Margin[0], yPos + it->Margin[2], contentRect.GetWidth(), contentRect.GetHeight(), Padding[0], Padding[2] );
                it->Draw( viewRect.Min.x, viewRect.Min.y, clickMute );

                // If the item position is not the same as the current running position, then set the
                // running position based off of the item position.
                if ( p.x != xPos + it->Margin[0] ) {    
                    xPos = std::max( xPos, p.x + it->Dim.x + it->Margin[1] );
                } else {
                    xPos += it->Margin[0] + it->Dim.x + it->Margin[1];
                }

            }

        }

        // Uncomment to draw labeled item rect
        // UIDrawBBOutline( viewRect, 0x00FF00, 3 );
        
    }

    void UIFormLabeledItem::UpdateDim( float inW, float inH ) {
        if ( Dim == vec2( inW, inH ) ) { return; }
        UIFormItem::UpdateDim( inW, inH );

        float contentW = Dim.x - Padding[0] - Padding[1];
        float contentH = Dim.y - Padding[2] - Padding[3];

        vector<UIFormItem*> Items = { &Label, Item };
        if (Direction >= 2 ) { Items = { Item, &Label }; }
        
        vector<vec2> itemDims( 2, vec2(0) );

        float totalAvailable = ( Direction % 2 == 0 ) ? contentH : contentW;
        float totalExpand = 0;
        int numExpand = 0;

        // Loop # 1 - Calculate known dimensions and sum of totals for Expand calculations in the next loop
        for ( int i = 0; i < Items.size(); i++ ) {

            auto it = Items[i];
            vec4 & margin = it->SetMargin( contentW, contentH );

            float w = 0, h = 0;

            // Calculate known values
            if ( it->W != "" ) { w = DecodeValue( it->W, contentW ); }
            if ( it->H != "" ) { h = DecodeValue( it->H, contentH ); }

            if ( Direction % 2 == 0 ) {     // ------------ vertical ------------
        
                // Set any unspecified widths and heights
                if ( it->H == "" ) {
                    if (it->Expand <= 0 ) { h = it->GetAutoDim().y; }
                    else { totalExpand += std::min(it->Expand, 1.0f); numExpand++; }
                }
                if ( it->W == "" ) { w = contentW - it->Margin[0] - it->Margin[1]; }

                // Subtract item height
                totalAvailable -= h;

                // Subtract item margin top / bottom, ignoring top margin if Y is explicitly placed
                totalAvailable -= ( it->Y != "" ) ? it->Margin[3] : ( it->Margin[2] + it->Margin[3] );

            } else {                    // ------------ horizontal ------------

                // Set any unspecified widths and heights
                if ( it->W == "" ) {
                    if( it->Expand <= 0 ) { w = it->GetAutoDim().x; }
                    else { totalExpand += std::min(it->Expand, 1.0f); numExpand++; }
                }
                if ( it->H == "" ) { h = contentH - it->Margin[2] - it->Margin[3]; }

                // Subtract item width
                totalAvailable -= w;

                // Subtract item margin left / right, ignoring left margin if X is explicitly placed
                totalAvailable -= ( it->X != "" ) ? it->Margin[1] : ( it->Margin[0] + it->Margin[1] );

            }
            
            // Set known values so far
            itemDims[i] = { w, h };

        }

        // Loop # 2 - Fill in dimensions for Expand items and set final dimensions
        for ( int i = 0; i < Items.size(); i++ ) {

            auto it = Items[i];
            vec4 & margin = it->SetMargin( contentW, contentH );

            float w = itemDims[i].x, h = itemDims[i].y;

            if ( Direction % 2 == 0 ) { // ------------ vertical ------------

                if ( it->H == "" ) {
                    if (it->Expand > 0 ) {
                        float expand = std::min(it->Expand, 1.0f);
                        h = ( totalAvailable > 0 ) ? std::min( expand/totalExpand, expand ) * totalAvailable : 0;   // If there is no more available space, don't draw
                    }
                }

                w = std::max( it->MinDim.x, std::min( it->MaxDim.x, w ) );
                h = std::max( it->MinDim.y, std::min( it->MaxDim.y, h ) );

            } else {                    // ------------ horizontal ------------

                if ( it->W == "" ) {
                    if (it->Expand > 0 ) {
                        float expand = std::min(it->Expand, 1.0f);
                        w = ( totalAvailable > 0 ) ? std::min( expand/totalExpand, expand ) * totalAvailable : 0;   // If there is no more available space, don't draw
                    }
                }

                w = std::max( it->MinDim.x, std::min( it->MaxDim.x, w ) );
                h = std::max( it->MinDim.y, std::min( it->MaxDim.y, h ) );

            }

            it->UpdateDim( w, h );      // Final update of dimensions

        }

    }

    vec2 UIFormLabeledItem::GetAutoDim() {

        vector<UIFormItem*> Items = { &Label, Item };
        if (Direction >= 2 ) { Items = { Item, &Label }; }

        vector<vec2> mDim( Items.size(), vec2(0) );
        vector<vec2> mPct( Items.size(), vec2(0) );
        vector<vec2> itDim( Items.size(), vec2(0) );
        vector<vec2> itPct( Items.size(), vec2(0) );

        for ( int i = 0; i < Items.size(); i++ ) {
            auto it = Items[i];

            DecodeValueAdd( it->MarginLeft,   mDim[i].x, mPct[i].x );
            DecodeValueAdd( it->MarginRight,  mDim[i].x, mPct[i].x );
            DecodeValueAdd( it->MarginTop,    mDim[i].y, mPct[i].y );
            DecodeValueAdd( it->MarginBottom, mDim[i].y, mPct[i].y );

            vec2 dim = it->GetAutoDim();

            if ( it->W != "" ) { DecodeValueAdd( it->W, itDim[i].x, itPct[i].x ); } else { itDim[i].x += dim.x; }
            if ( it->H != "" ) { DecodeValueAdd( it->H, itDim[i].y, itPct[i].y ); } else { itDim[i].y += dim.y; }

        }
        
        // Combine
        vec2 mTotalDim = vec2(0), mTotalPct = vec2(0), itTotalDim = vec2(0), itTotalPct = vec2(0);
        if ( Direction % 2 == 0 ) {     // ------------ vertical ------------
            for ( int i = 0; i < Items.size(); i++ ) {
                auto it = Items[i];
                mTotalDim.x = std::max( mTotalDim.x, mDim[i].x );
                mTotalPct.x = std::max( mTotalPct.x, mPct[i].x );
                mTotalDim.y += mDim[i].y;
                mTotalPct.y += mPct[i].y;

                itTotalDim.x = std::max( itTotalDim.x, itDim[i].x );
                itTotalPct.x = std::max( itTotalPct.x, itPct[i].x );
                if ( it->Y != "" ) {
                    float yPos = DecodeValue( it->Y, Dim.y );    // relative to the last Content origin
                    if ( yPos < itTotalDim.y ) { itTotalDim.y = yPos; }
                }
                itTotalDim.y += itDim[i].y;
                itTotalPct.y += itPct[i].y;
            }  
        }
        else {                          // ------------ horizontal ------------
            for ( int i = 0; i < Items.size(); i++ ) {
                auto it = Items[i];
                mTotalDim.y = std::max( mTotalDim.y, mDim[i].y );
                mTotalPct.y = std::max( mTotalPct.y, mPct[i].y );
                mTotalDim.x += mDim[i].x;
                mTotalPct.x += mPct[i].x;

                itTotalDim.y = std::max( itTotalDim.y, itDim[i].y );
                itTotalPct.y = std::max( itTotalPct.y, itPct[i].y );
                if ( it->X != "" ) {
                    float xPos = DecodeValue( it->X, Dim.x );    // relative to the last Content origin
                    if ( xPos < itTotalDim.x ) { itTotalDim.x = xPos; }
                }
                itTotalDim.x += itDim[i].x;
                itTotalPct.x += itPct[i].x;
            }  
        }

        vec2 rmdPct = vec2(1) - itTotalPct - mTotalPct;
        vec2 totalDim;
        totalDim.x = ( rmdPct.x <= 0 ) ? ( itTotalDim.x + mTotalDim.x ) : ( ( itTotalDim.x + mTotalDim.x ) / rmdPct.x );
        totalDim.y = ( rmdPct.y <= 0 ) ? ( itTotalDim.y + mTotalDim.y ) : ( ( itTotalDim.y + mTotalDim.y ) / rmdPct.y );
        totalDim.x = GetTotalWithPadding( PadLeft, PadRight, totalDim.x );
        totalDim.y = GetTotalWithPadding( PadTop, PadBottom, totalDim.y );

        return totalDim;
    }

    bool UIFormLabeledItem::SetFocus( bool inFocus, int inDirection, bool force ) {
        return Item->SetFocus( inFocus, inDirection, force );
    }
    
    bool UIFormLabeledItem::GetFocus() {
        return Item->GetFocus();
    }

    ImRect UIFormLabeledItem::GetFocusedRect( float inX, float inY ) {
        return Item->GetFocusedRect( inX+Pos.x, inY+Pos.y );
    }

    void UIFormLabeledItem::ResetDraw() {
        Item->ResetDraw();
    }


    // UIFormCheckBox --------------------------------------------------------------------------------

    void UIFormCheckBox::Draw( float inX, float inY, bool clickMute ) {
        if ( Dim.x == 0 ) { return; }
        if ( Dim.y == 0 ) { return; }

        vec2 mPos = GetGlobalMousePos();
        int keyD = GetKeyDown();
        int keyChange = UILibIS->Key.Change;

        ImRect rect = GetContentRect( inX, inY );

        float dim = std::min( Dim.x, Dim.y );
        ImRect crect = GetBB( rect.Min.x, rect.Min.y, dim, dim );

        if( VecInBBClip(mPos, crect) ) {
            if( DragStart() && !clickMute){ IsChecked = !IsChecked; }
        }
        else {
            if ( DragStart() ) { SetFocus( false ); }
        }

        UIFormItem::BoxFormat f = Default;
        if ( IsChecked ) { f = GetBoxFormat( Checked, Default); }
        if ( Focused ) { f = GetBoxFormat( Focus, f ); crect.Expand(1); }

        if ( f.BkgdColor >= 0 ) { UIDrawAlphaBB(crect, f.BkgdColor, f.BkgdAlpha); }
        if ( f.BorderSize > 0 ) { UIDrawBBOutline(crect, f.BorderColor, f.BorderSize); }

        float thickness = std::max( dim / 5.0f, 1.0f );
        if(IsChecked){
            float sz = dim - thickness*0.5f - 2*f.BorderSize;
            vec2 pos = vec2(rect.Min.x, rect.Min.y) + vec2(thickness*0.25f + f.BorderSize, f.BorderSize);
            float third = sz / 3.0f;
            float bx = pos.x + third;
            float by = pos.y + sz - third*0.5f;
            ImU32 cClr = UILib::Cl( CheckColor );
            ImGuiContext& g = *GImGui;
            ImGuiWindow* window = g.CurrentWindow;
            window->DrawList->PathLineTo(ImVec2(bx - third, by - third));
            window->DrawList->PathLineTo(ImVec2(bx, by));
            window->DrawList->PathLineTo(ImVec2(bx + third*2, by - third*2));
            window->DrawList->PathStroke(cClr, false, thickness);
        }


    }

    vec2 UIFormCheckBox::GetAutoDim(){ return vec2( 15, 15 ); }

    bool UIFormCheckBox::SetFocus( bool inFocus, int inDirection, bool force ) {
        Focused = inFocus;
        return Focused; 
    }

    bool UIFormCheckBox::GetFocus() { 
        return Focused;  
    }

    bool UIFormCheckBox::RevertFocus() {
        SetFocus( !Focused, 1, true );
        Focused = !Focused;
        return Focused;
    }

    bool UIFormCheckBox::EnterKeyFocus() { 
        SetFocus( true );                     
        IsChecked = !IsChecked;
        return Focused;
    }

    // UIFormCollapserCheckBox --------------------------------------------------------------------------------

    UIFormCollapserCheckBox::UIFormCollapserCheckBox( bool & inCheck, bool & inNeedUpdate ) : 
            UIFormCheckBox(inCheck), UIFormCollapser(inNeedUpdate) { 
            ItemType = uif_collapsercheckbox;
            IsOpen = inCheck;
    };

    void UIFormCollapserCheckBox::Draw( float inX, float inY, bool clickMute ) {
        UIFormCheckBox::Draw( inX, inY, clickMute);
        SetState(IsChecked);
    }

    void UIFormCollapserCheckBox::SetCollapse( bool inCollapse ) {
        Collapse =  inCollapse;
        SetItemsCollapse( Collapse ? true : !IsOpen );
    }

    // ============================================================================================ //
    //                                          THE FORM                                            //
    // ============================================================================================ //

    UIForm::UIForm( string inName, int inDir, int inPPGIndex ) :
        UIFormItem(uif_form), 
        Name( inName ),
        Direction( inDir ) { 
        Index=inPPGIndex; 
    };
    
    UIForm::~UIForm() {
        for ( auto it : Items ) { if (it->ItemType != uif_custom_external) delete it; }
    }

    void UIForm::ResetForm( string inName, int inDir, int inPPGIndex ) {
        for ( auto it : Items ) { delete it; }

        Name = inName;
        Direction = inDir;
        Index = inPPGIndex;

        AlignItems = align_start;
        ContentDim = glm::vec2(0);
        ScrollTrackSize = 10;
        ScrollDragging = 0;
        ScrollPerc = 0;
        BorderSize = 0;
        BorderColor = 0x000000;
        BkgdColor = -1;
        BkgdAlpha = 1;
        Items = {};
        NeedUpdate = true;
        FocusedItem = -1;
        OverlayRects = {};
        OverlayItems = {};

    }

    void UIForm::RequestUpdate() { 
        NeedUpdate = true;
        for ( auto it : Items ) { it->RequestUpdate(); }
    };

    void UIForm::ResetDraw() {
        for ( int i = 0; i < Items.size(); i++ ) { Items[i]->ResetDraw(); }
    }

    void UIForm::Draw( float parentX, float parentY, bool clickMute ) { 
        // Currently assuming align_start placement of items
        
        if ( IsRoot ) { 
            ProcessKeyEvents(); 
            if ( !clickMute ) {
                float scrollChange = GetMouseScroll();                
                if ( scrollChange != 0 ) { 
                    ScrollChange( parentX, parentY, scrollChange ); 
                }
            }
            // Check and remove Overlay Items that are no longer valid.
            // The root form tracks all overlays from all items and sub-items.
            // This is used to check the mouse-hit.
            for ( int i = (int) OverlayItems.size()-1; i >= 0; i-- ) {
                if ( !Items[ OverlayItems[i] ]->GetOverlayState() ) {
                    OverlayItems.erase( OverlayItems.begin() + i );
                    OverlayRects.erase( OverlayRects.begin() + i );
                }
            }
        }
        else {
            // Check and reset the Overlay Item.
            // Sub-forms only track one overlay item.
            if ( OverlayItem >= 0 ) {
                if ( !Items[OverlayItem]->GetOverlayState() ) { OverlayItem = -1; }
            }
        }
        
        bool MuteClick = clickMute || OverlayHit();
        bool ScrollMute = MuteClick;

        auto currPos = Pos + vec2( parentX, parentY );

        float contentW = Dim.x - Padding[0] - Padding[1];
        float contentH = Dim.y - Padding[2] - Padding[3];
        
        ImRect viewRect = GetGlobalBB( currPos.x, currPos.y, Dim.x, Dim.y );
        ImRect clipRect = viewRect;
        if ( BkgdColor >= 0 ) { if ( BkgdAlpha > 0 ) { UIDrawAlphaBB( viewRect, BkgdColor, BkgdAlpha ); } }
        if ( BorderSize > 0 && ClipBorder ) { clipRect.Expand( { BorderSize, BorderSize } ); }

        MouseHit = !MuteClick && VecInBBClip( GetGlobalMousePos(), viewRect );

        if ( Direction == 0 ) {     // ------------ vertical ------------
            
            float xPos = 0, yPos = 0;
            xPos += Padding[0];
            yPos += Padding[2];

            float totalH = ContentDim.y + Padding[2] + Padding[3];
            bool scrollOn = ( totalH > Dim.y + 0.001f ) && AllowScroll;
            float yShift = 0;

            ImGui::PushClipRect(clipRect.Min, clipRect.Max, true);
            if ( scrollOn ) {
                float offScreen = totalH - Dim.y;
                yShift = -ScrollPerc * offScreen;
                ImRect scrollBB = GetBB( viewRect.Max.x-ScrollTrackSize, viewRect.Min.y, ScrollTrackSize, Dim.y );
                if ( GetOverState(scrollBB) && DragStart() ) { MuteClick = true; }
                else if ( ScrollState > 0 ) { MuteClick = true; }
            }

            for ( int i = 0; i < Items.size(); i++ ) { 
                
                auto it = Items[i];
                if ( it->Collapse ) { it->ResetDraw();continue; }

                ( i == FocusedItem ) ? it->SetFocus( true ) : it->SetFocus( false );

                // Set item position and draw if it is visible
                vec2 & p = it->SetPos( xPos + it->Margin[0], yPos + it->Margin[2], contentW, contentH, Padding[0], Padding[2] );
                if ( it->IsVisible( viewRect, currPos.x, currPos.y + yShift ) ) { it->Draw( currPos.x, currPos.y + yShift, MuteClick ); }
                else { it->ResetDraw(); }

                // If the item position is not the same as the current running position, then set the
                // running position based off of the item position.
                if ( p.y != yPos + it->Margin[2] ) {    
                    yPos = std::max( yPos, p.y + it->Dim.y + it->Margin[3] );
                } else {
                    yPos += it->Margin[2] + it->Dim.y + it->Margin[3];
                }

                if ( it->GetFocus() == true ) { FocusedItem = i; } else if ( FocusedItem == i ) { FocusedItem = -1; }
                if ( it->GetOverlayState() == true ) { 
                    if ( IsRoot ) { 
                        const auto overR = it->GetOverlayRects();
                        for ( const ImRect & oR : overR ) {
                            AddOverlayItem( i, oR );
                        }
                    }
                    else { OverlayItem = i; }
                }
                
            }

            if ( BorderSize > 0 ) { if ( BorderColor >= 0 ) { UIDrawBBOutline(viewRect, BorderColor, BorderSize); } }

            if ( scrollOn ) { 
                UIAddScrollerV(viewRect.Max.x-ScrollTrackSize, viewRect.Min.y, ScrollTrackSize, Dim.y, totalH, &ScrollPerc, &ScrollDragging, UILibCTX->DefaultScroller, &ScrollState, ScrollMute);
            }
            ImGui::PopClipRect(); 
            DrawOverlayItems( currPos.x, currPos.y + yShift );

        } else if ( Direction == 1 ) {                    // ------------ horizontal ------------

            float xPos = 0, yPos = 0;
            xPos += Padding[0];
            yPos += Padding[2];

            float totalW = ContentDim.x + Padding[0] + Padding[1];
            bool scrollOn = ( totalW > Dim.x + 0.001f ) && AllowScroll;
            float xShift = 0;

            ImGui::PushClipRect(clipRect.Min, clipRect.Max, true);
            if ( scrollOn ) {
                float offScreen = totalW - Dim.x;
                xShift = -ScrollPerc * offScreen;
                ImRect scrollBB = GetBB( viewRect.Min.x, viewRect.Max.y-ScrollTrackSize, Dim.x, ScrollTrackSize );
                if ( GetOverState(scrollBB) && DragStart() ) { MuteClick = true; }
                else if ( ScrollState > 0 ) { MuteClick = true; }
            }

            for ( int i = 0; i < Items.size(); i++ ) { 
                
                auto it = Items[i]; 
                if ( it->Collapse ) { it->ResetDraw();continue; }

                ( i == FocusedItem ) ? it->SetFocus( true ) : it->SetFocus( false );

                // Set item position and draw if it is visible
                vec2 & p = it->SetPos( xPos + it->Margin[0], yPos + it->Margin[2], contentW, contentH, Padding[0], Padding[2] );
                if ( it->IsVisible( viewRect, currPos.x + xShift, currPos.y ) ) { it->Draw( currPos.x + xShift, currPos.y, MuteClick ); }
                else { it->ResetDraw(); }

                // If the item position is not the same as the current running position, then set the
                // running position based off of the item position.
                if ( p.x != xPos + it->Margin[0] ) {    
                    xPos = std::max( xPos, p.x + it->Dim.x + it->Margin[1] );
                } else {
                    xPos += it->Margin[0] + it->Dim.x + it->Margin[1];
                }

                if ( it->GetFocus() == true ) { FocusedItem = i; } else if ( FocusedItem == i ) { FocusedItem = -1; }
                if ( it->GetOverlayState() == true ) { 
                    if ( IsRoot ) { 
                        const auto overR = it->GetOverlayRects();
                        for ( const ImRect & oR : overR ) {
                            AddOverlayItem( i, oR );
                        }
                    }
                    else { OverlayItem = i; }
                }

            }

            if ( BorderSize > 0 ) { if ( BorderColor >= 0 ) { UIDrawBBOutline(viewRect, BorderColor, BorderSize); } }

            if ( scrollOn ) { 
                UIAddScrollerH(viewRect.Min.x, viewRect.Max.y-ScrollTrackSize, Dim.x, ScrollTrackSize, totalW, &ScrollPerc, &ScrollDragging, UILibCTX->DefaultScroller, &ScrollState, ScrollMute);
            }
            ImGui::PopClipRect(); 
            DrawOverlayItems( currPos.x + xShift, currPos.y );

        } else if ( Direction == 2 ) {     // ------------ vertical reverse ------------

            float xPos = 0, yPos = 0;
            xPos += Padding[0];
            yPos += Padding[3];

            float totalH = ContentDim.y + Padding[2] + Padding[3];
            bool scrollOn = ( totalH > Dim.y + 0.001f ) && AllowScroll;
            float yShift = 0;

            ImGui::PushClipRect(clipRect.Min, clipRect.Max, true);
            if ( scrollOn ) {
                float offScreen = totalH - Dim.y;
                yShift = (1-ScrollPerc) * offScreen;
                ImRect scrollBB = GetBB( viewRect.Max.x-ScrollTrackSize, viewRect.Min.y, ScrollTrackSize, Dim.y );
                if ( GetOverState(scrollBB) && DragStart() ) { MuteClick = true; }
                else if ( ScrollState > 0 ) { MuteClick = true; }
            }

            for ( int i = 0; i < Items.size(); i++ ) { 
                
                auto it = Items[i];
                if ( it->Collapse ) {it->ResetDraw(); continue; }

                ( i == FocusedItem ) ? it->SetFocus( true ) : it->SetFocus( false );

                // Set item position and draw if it is visible
                vec2 & p = it->SetPos( xPos + it->Margin[0], yPos + it->Margin[3], contentW, contentH, Padding[0], Padding[3] );    // p.y is now the Y distance from the bottom of form
                float setY = p.y;
                p.y = contentH + Padding[2] + Padding[3] - p.y - it->Dim.y;                                                         // adjust the p.y so it is distance from top of form
                if ( it->IsVisible( viewRect, currPos.x, currPos.y  + yShift ) ) { it->Draw( currPos.x, currPos.y + yShift, MuteClick ); }
                else { it->ResetDraw(); }

                // If the item position is not the same as the current running position, then set the
                // running position based off of the item position.
                if ( setY != yPos + it->Margin[3] ) {    
                    yPos = std::max( yPos, setY + it->Dim.y + it->Margin[2] );
                } else {
                    yPos += it->Margin[2] + it->Dim.y + it->Margin[3];
                }

                if ( it->GetFocus() == true ) { FocusedItem = i; } else if ( FocusedItem == i ) { FocusedItem = -1; }
                if ( it->GetOverlayState() == true ) { 
                    if ( IsRoot ) { 
                        const auto overR = it->GetOverlayRects();
                        for ( const ImRect & oR : overR ) {
                            AddOverlayItem( i, oR );
                        }
                    }
                    else { OverlayItem = i; }
                }

            }

            if ( BorderSize > 0 ) { if ( BorderColor >= 0 ) { UIDrawBBOutline(viewRect, BorderColor, BorderSize); } }

            if ( scrollOn ) { 
                UIAddScrollerV(viewRect.Max.x-ScrollTrackSize, viewRect.Min.y, ScrollTrackSize, Dim.y, totalH, &ScrollPerc, &ScrollDragging, UILibCTX->DefaultScroller, &ScrollState, ScrollMute);
            }
            ImGui::PopClipRect(); 
            DrawOverlayItems( currPos.x, currPos.y + yShift );

        } else {                    // ------------ horizontal reverse ------------

            float xPos = 0, yPos = 0;
            xPos += Padding[1];
            yPos += Padding[2];

            float totalW = ContentDim.x + Padding[0] + Padding[1];
            bool scrollOn = ( totalW > Dim.x + 0.001f ) && AllowScroll;
            float xShift = 0;

            ImGui::PushClipRect(clipRect.Min, clipRect.Max, true);
            if ( scrollOn ) {
                float offScreen = totalW - Dim.x;
                xShift = (1-ScrollPerc) * offScreen;
                ImRect scrollBB = GetBB( viewRect.Min.x, viewRect.Max.y-ScrollTrackSize, Dim.x, ScrollTrackSize );
                if ( GetOverState(scrollBB) && DragStart() ) { MuteClick = true; }
                else if ( ScrollState > 0 ) { MuteClick = true; }
            }

            for ( int i = 0; i < Items.size(); i++ ) { 
                
                auto it = Items[i]; 
                if ( it->Collapse ) {it->ResetDraw(); continue; }

                ( i == FocusedItem ) ? it->SetFocus( true ) : it->SetFocus( false );

                // Set item position and draw if it is visible
                vec2 & p = it->SetPos( xPos + it->Margin[1], yPos + it->Margin[2], contentW, contentH, Padding[1], Padding[2] );    // p.x is now the X distance from the right of form
                float setX = p.x;
                p.x = contentW + Padding[0] + Padding[1] - p.x - it->Dim.x;                                                         // adjust the p.x so it is distance from left of form
                if ( it->IsVisible( viewRect, currPos.x + xShift, currPos.y ) ) { it->Draw( currPos.x + xShift, currPos.y, MuteClick ); }
                else { it->ResetDraw(); }

                // If the item position is not the same as the current running position, then set the
                // running position based off of the item position.
                if ( setX != xPos + it->Margin[0] ) {    
                    xPos = std::max( xPos, setX + it->Dim.x + it->Margin[0] );
                } else {
                    xPos += it->Margin[0] + it->Dim.x + it->Margin[1];
                }

                if ( it->GetFocus() == true ) { FocusedItem = i; } else if ( FocusedItem == i ) { FocusedItem = -1; }
                if ( it->GetOverlayState() == true ) { 
                    if ( IsRoot ) { 
                        const auto overR = it->GetOverlayRects();
                        for ( const ImRect & oR : overR ) {
                            AddOverlayItem( i, oR );
                        }
                    }
                    else { OverlayItem = i; }
                }

            }

            if ( BorderSize > 0 ) { if ( BorderColor >= 0 ) { UIDrawBBOutline(viewRect, BorderColor, BorderSize); } }

            if ( scrollOn ) { 
                UIAddScrollerH(viewRect.Min.x, viewRect.Max.y-ScrollTrackSize, Dim.x, ScrollTrackSize, totalW, &ScrollPerc, &ScrollDragging, UILibCTX->DefaultScroller, &ScrollState, ScrollMute);
            }
            ImGui::PopClipRect(); 
            DrawOverlayItems( currPos.x + xShift, currPos.y );

        }

    }

    void UIForm::UpdateDim( float inW, float inH ) {
        if ( Dim != vec2( inW, inH ) ) {
            UIFormItem::UpdateDim( inW, inH );
            NeedUpdate = true;
        }
        if ( !NeedUpdate ) { return; }
        if ( Dim.x == 0 ) { return; }
        if ( Dim.y == 0 ) { return; }

        // std::cout << "************ Updating Dimensions: Form " << Index << ", New Dim: " << VecToString( {inW, inH } ) << " ************" << std::endl;

        vec2 oldContentDim = ContentDim;

        float contentW = Dim.x - Padding[0] - Padding[1];
        float contentH = Dim.y - Padding[2] - Padding[3];
        ContentDim = {0, 0};

        int start = 0;
        float lastPos = 0;

        if ( Direction % 2 == 0 ) {     // ------------ vertical ------------

            for ( int i = 0; i < Items.size(); i++ ) {

                auto it = Items[i];
                if ( it->Collapse ) { continue; }

                vec4 & margin = it->SetMargin( contentW, contentH );

                // Check if the Y position is known. If so, fit everything up to the known position.
                if ( it->Y != "" ) { 
                    float yPos = DecodeValue( it->Y, contentH );    // relative to Content origin
                    yPos -= Margin[2];                              // offset by top margin
                    vec2 sDim = UpdateSubsetDim( start, i, yPos-lastPos );
                    ContentDim.x = std::max( ContentDim.x, sDim.x );
                    ContentDim.y = std::max( lastPos + sDim.y, std::max( ContentDim.y, yPos ) );
                    start = i;
                    lastPos = yPos;
                }
            }
            if ( start < Items.size() ) {
                vec2 sDim = UpdateSubsetDim( start, (int) Items.size(), contentH-lastPos );
                ContentDim.x = std::max( ContentDim.x, sDim.x );
                ContentDim.y = std::max( lastPos + sDim.y, std::max( ContentDim.y, contentH ) );
            }

        } else {                    // ------------ horizontal ------------

            for ( int i = 0; i < Items.size(); i++ ) {

                auto it = Items[i];
                if ( it->Collapse ) { continue; }

                vec4 & margin = it->SetMargin( contentW, contentH );

                // Check if the X position is known. If so, fit everything up to the known position.
                if ( it->X != "" ) { 
                    float xPos = DecodeValue( it->X, contentW );    // relative to Content origin
                    xPos -= Margin[0];                              // offset by left margin
                    vec2 sDim = UpdateSubsetDim( start, i, xPos-lastPos );
                    ContentDim.x = std::max( lastPos + sDim.x, std::max( ContentDim.x, xPos ) );
                    ContentDim.y = std::max( ContentDim.y, sDim.y );
                    start = i;
                    lastPos = xPos;
                }
            }
            if ( start < Items.size() ) {
                vec2 sDim = UpdateSubsetDim( start, (int) Items.size() , contentW-lastPos );
                ContentDim.x = std::max( lastPos + sDim.x, std::max( ContentDim.x, contentW ) );
                ContentDim.y = std::max( ContentDim.y, sDim.y );
            }

        }

        // std::cout << "Total ContentDim: " << ContentDim.x << ", " << ContentDim.y << std::endl;
        // std::cout << "************ Finished Update Dim ************" << std::endl;

        // All Item positions and dimensions are up to date, so turn off flag
        NeedUpdate = false;
        ResizeScrollAdjust( oldContentDim );
    }

    vec2 UIForm::UpdateSubsetDim( int inStart, int inEnd, float inAvailable ) {
        // std::cout << "  ----- Subset: " << inStart << " to " << inEnd << ",  Available: " << inAvailable << " -----" << std::endl;

        float contentW = Dim.x - Padding[0] - Padding[1];
        float contentH = Dim.y - Padding[2] - Padding[3];

        float totalAvailable = inAvailable;
        float totalExpand = 0;
        int numExpand = 0;

        vec2 subsetDim = {0, 0};
        vector<vec2> itemDims( inEnd-inStart, vec2(0) );

        // Loop # 1 - Calculate known dimensions and sum of totals for Expand calculations in the next loop
        for ( int i = inStart; i < inEnd; i++ ) {

            auto it = Items[i];
            if ( it->Collapse ) { continue; }

            float w = 0, h = 0;

            // Calculate known values
            if ( it->W != "" ) { w = DecodeValue( it->W, contentW ); }
            if ( it->H != "" ) { h = DecodeValue( it->H, contentH ); }

            if ( Direction % 2 == 0 ) {     // ------------ vertical ------------
        
                // Set any unspecified widths and heights
                if ( it->H == "" ) {
                    if (it->Expand <= 0 ) { h = it->GetAutoDim().y; }
                    else { totalExpand += std::min(it->Expand, 1.0f); numExpand++; }
                }
                if ( it->W == "" ) { w = contentW - it->Margin[0] - it->Margin[1]; }

                // Subtract item height
                totalAvailable -= h;

                // Subtract item margin top / bottom, ignoring top margin if Y is explicitly placed
                totalAvailable -= ( it->Y != "" ) ? it->Margin[3] : ( it->Margin[2] + it->Margin[3] );

            } else {                    // ------------ horizontal ------------

                // Set any unspecified widths and heights
                if ( it->W == "" ) {
                    if( it->Expand <= 0 ) { w = it->GetAutoDim().x; }
                    else { totalExpand += std::min(it->Expand, 1.0f); numExpand++; }
                }
                if ( it->H == "" ) { h = contentH - it->Margin[2] - it->Margin[3]; }

                // Subtract item width
                totalAvailable -= w;

                // Subtract item margin left / right, ignoring left margin if X is explicitly placed
                totalAvailable -= ( it->X != "" ) ? it->Margin[1] : ( it->Margin[0] + it->Margin[1] );

            }
            
            // Set known values so far
            itemDims[i-inStart] = { w, h };

        }

        // Loop # 2 - Fill in dimensions for Expand items and set final dimensions
        for ( int i = inStart; i < inEnd; i++ ) {

            auto it = Items[i];
            if ( it->Collapse ) { continue; }

            float w = itemDims[i-inStart].x, h = itemDims[i-inStart].y;

            if ( Direction % 2 == 0 ) {     // ------------ vertical ------------

                if ( it->H == "" ) {
                    if (it->Expand > 0 ) {
                        float expand = std::min(it->Expand, 1.0f);
                        h = ( totalAvailable > 0 ) ? std::min( expand/totalExpand, expand ) * totalAvailable : 0;   // If there is no more available space, don't draw
                    }
                }

                w = std::max( it->MinDim.x, std::min( it->MaxDim.x, w ) );
                h = std::max( it->MinDim.y, std::min( it->MaxDim.y, h ) );

                float xTotal = ( it->X != "" ) ? ( DecodeValue( it->X, contentW ) + w + it->Margin[1] ) : ( it->Margin[0] + w + it->Margin[1] );
                subsetDim.x = std::max( subsetDim.x, xTotal );
                
                float yTotal = ( it->Y != "" ) ? ( h + it->Margin[3] ) : ( it->Margin[2] + h + it->Margin[3] );
                subsetDim.y += yTotal;     // don't add top margin of Y to subsetDim if Y explictly set

            } else {                    // ------------ horizontal ------------

                if ( it->W == "" ) {
                    if (it->Expand > 0 ) {
                        float expand = std::min(it->Expand, 1.0f);
                        w = ( totalAvailable > 0 ) ? std::min( expand/totalExpand, expand ) * totalAvailable : 0;   // If there is no more available space, don't draw
                    }
                }

                w = std::max( it->MinDim.x, std::min( it->MaxDim.x, w ) );
                h = std::max( it->MinDim.y, std::min( it->MaxDim.y, h ) );

                float xTotal = ( it->Y != "" ) ? ( w + it->Margin[1] ) : ( it->Margin[0] + w + it->Margin[1] );
                subsetDim.x += xTotal;     // don't add left margin of X to subsetDim if X explictly set

                float yTotal = ( it->Y != "" ) ? ( DecodeValue( it->Y, contentH ) + h + it->Margin[3] ) : ( it->Margin[2] + h + it->Margin[3] );
                subsetDim.y = std::max( subsetDim.y, yTotal );

            }

            it->UpdateDim( w, h );      // Final update of dimensions
            // std::cout << "    Item: " << it->Index << ", Final Dim: " << w << ", " << h << std::endl;

        }

        //  Add any remainder to subsetDim
        if ( Direction % 2 == 0 ) {     // ------------ vertical ------------
            subsetDim.y += std::max( 0.0f, inAvailable - subsetDim.y );
        } else {                    // ------------ horizontal ------------
            subsetDim.x += std::max( 0.0f, inAvailable - subsetDim.x );
        }

        // *** TO DO: Will need use the remainder for align_outer or align_even to include the spacing between items.

        // std::cout << "  Subset Summary: " << std::endl;
        // std::cout << "    contentW: " << contentW << ", contentH: " << contentH << std::endl;
        // std::cout << "    totalAvailable: " << totalAvailable << std::endl;
        // std::cout << "    totalExpand: " << totalExpand << std::endl;
        // std::cout << "    numExpand: " << numExpand << std::endl;
        // std::cout << "    subsetDim: " << subsetDim.x << ", " << subsetDim.y << std::endl;


        // Returns the aggregate dimension of the subset including margin.
        return subsetDim;

    }

    vec2 UIForm::GetAutoDim() {

        // std::cout << "    UIForm::GetAutoDim    Index: " << Index << std::endl;

        vec2 itDim = {0,0};
        vec2 itPct = {0,0};

        vec2 mDim = {0,0};
        vec2 mPct = {0,0};

        if ( Direction % 2 == 0 ) {     // ------------ vertical ------------

            for ( int i = 0; i < Items.size(); i++ ) {

                auto it = Items[i];
                if ( it->Collapse ) { continue; }
                
                vec2 cmDim = {0,0};
                vec2 cmPct = {0,0};
                DecodeValueAdd( it->MarginLeft,   cmDim.x, cmPct.x );
                DecodeValueAdd( it->MarginRight,  cmDim.x, cmPct.x );
                DecodeValueAdd( it->MarginTop,    cmDim.y, cmPct.y );
                DecodeValueAdd( it->MarginBottom, cmDim.y, cmPct.y );

                mDim.x = std::max( mDim.x, cmDim.x );
                mPct.x = std::max( mPct.x, cmPct.x );
                mDim.y += cmDim.y;
                mPct.y += cmPct.y;

                vec2 dim = it->GetAutoDim();

                vec2 citDim = {0,0};
                vec2 citPct = {0,0};

                if ( it->W != "" ) { DecodeValueAdd( it->W, citDim.x, citPct.x ); } else { citDim.x += dim.x; }
                if ( it->H != "" ) { DecodeValueAdd( it->H, citDim.y, citPct.y ); } else { citDim.y += dim.y; }

                itDim.x = std::max( itDim.x, citDim.x );
                itPct.x = std::max( itPct.x, citPct.x );
                itDim.y += citDim.y;
                itPct.y += citPct.y;

                // std::cout << "      Item: " << it->Index << "  mDim: " << VecToString(mDim) << ", mPct: " << VecToString(mPct) << ", itDim: " << VecToString(itDim) << std::endl;
                            
            }
        
        } else {                    // ------------ horizontal ------------

            for ( int i = 0; i < Items.size(); i++ ) {

                auto it = Items[i];
                if ( it->Collapse ) { continue; }
                
                vec2 cmDim = {0,0};
                vec2 cmPct = {0,0};
                DecodeValueAdd( it->MarginLeft, cmDim.x, cmPct.x );
                DecodeValueAdd( it->MarginRight, cmDim.x, cmPct.x );
                DecodeValueAdd( it->MarginTop, cmDim.y, cmPct.y );
                DecodeValueAdd( it->MarginBottom, cmDim.y, cmPct.y );

                mDim.x += cmDim.x;
                mPct.x += cmPct.x;
                mDim.y = std::max( mDim.y, cmDim.y );
                mPct.y = std::max( mPct.y, cmPct.y );
                
                vec2 dim = it->GetAutoDim();

                vec2 citDim = {0,0};
                vec2 citPct = {0,0};

                if ( it->W != "" ) { DecodeValueAdd( it->W, citDim.x, citPct.x ); } else { citDim.x += dim.x; }
                if ( it->H != "" ) { DecodeValueAdd( it->H, citDim.y, citPct.y ); } else { citDim.y += dim.y; }

                itDim.x += citDim.x;
                itPct.x += citPct.x;
                itDim.y = std::max( itDim.y, citDim.y );
                itPct.y = std::max( itPct.y, citPct.y );
                
                // std::cout << "      Item: " << it->Index << "  mDim: " << VecToString(mDim) << ", mPct: " << VecToString(mPct) << ", itDim: " << VecToString(itDim) << std::endl;
            
            }

        }

        vec2 rmdPct = vec2(1) - itPct - mPct;
        vec2 totalDim;
        totalDim.x = ( rmdPct.x <= 0 ) ? ( itDim.x + mDim.x ) : ( ( itDim.x + mDim.x ) / rmdPct.x );
        totalDim.y = ( rmdPct.y <= 0 ) ? ( itDim.y + mDim.y ) : ( ( itDim.y + mDim.y ) / rmdPct.y );
        totalDim.x = GetTotalWithPadding( PadLeft, PadRight, totalDim.x );
        totalDim.y = GetTotalWithPadding( PadTop, PadBottom, totalDim.y );

        // std::cout << "    Total Auto Dim: " << VecToString( totalDim ) << std::endl;

        return totalDim;

    }

    bool UIForm::SetFocus( bool inFocus, int inDirection, bool force ) {
        LastFocusedItem = FocusedItem;
        if ( inFocus == false ) {
            if ( FocusedItem == -1 ) { return false; }
            else {
                auto it = Items[FocusedItem];
                it->SetFocus( inFocus, inDirection, force );
                FocusedItem = -1;
                return false;
            }
        }
        else {
            if ( FocusedItem == -1 ) {
                if ( inDirection == 1 ) {
                    for ( int i = 0; i < Items.size(); i++ ) {
                        auto it = Items[i];
                        if ( it->SetFocus( true, inDirection, force ) ) {
                            FocusedItem = i;
                            return true;
                        }
                    }
                } else {
                    for ( int i = (int) Items.size(); i <= 0; i-- ) {
                        auto it = Items[i];
                        if ( it->SetFocus( true, inDirection, force ) ) {
                            FocusedItem = i;
                            return true;
                        }
                    }
                }
                return false;
            } 
            else {
                auto it = Items[FocusedItem];
                return it->SetFocus( inFocus, inDirection, force );
            }

        }
    }
    
    bool UIForm::GetFocus() {
        return ( FocusedItem > -1 ) ? true : false;
    }

    int UIForm::GetFocusType() {
        if ( FocusedItem > -1 ) { 
            auto it = Items[FocusedItem];
            return it->GetFocusType(); 
        }
        return -1;
    }

    bool UIForm::RevertFocus() {
        bool res = false;
        if ( FocusedItem > -1 ) {
            auto it = Items[FocusedItem];
            it->SetFocus( false, 1, true );
        }
        if ( LastFocusedItem > -1 ) { 
            auto it = Items[LastFocusedItem];
            res = it->RevertFocus();
        }
        std::swap( FocusedItem, LastFocusedItem );
        return res;
    }

    UIFormItem * UIForm::GetFocusedItem() {
        if ( FocusedItem > -1 ) { 
            return Items[FocusedItem];
        }
        return nullptr;
    }

    ImRect UIForm::GetFocusedRect( float inX, float inY ) {
        if ( FocusedItem > -1 ) { 
            auto it = Items[FocusedItem];
            return it->GetFocusedRect( inX+Pos.x, inY+Pos.y ); 
        }
        return UIFormItem::GetFocusedRect( inX, inY );       
    }

    bool UIForm::EscKeyFocus() {
        if ( FocusedItem > -1 ) { 
            auto it = Items[FocusedItem];
            return it->EscKeyFocus(); 
        }
        return UIFormItem::EscKeyFocus();
    }

    bool UIForm::EnterKeyFocus() {
        if ( FocusedItem > -1 ) { 
            auto it = Items[FocusedItem];
            return it->EnterKeyFocus(); 
        }
        return UIFormItem::EnterKeyFocus();
    }

    bool UIForm::ProcessKeyDown( int inKey ) {
        if ( FocusedItem > -1 ) { 
            auto it = Items[FocusedItem];
            return it->ProcessKeyDown( inKey ); 
        }
        return UIFormItem::ProcessKeyDown( inKey );
    }

    bool UIForm::GetOverlayState() {
        return ( OverlayItem >= 0 );
    }
    
    vector<ImRect> UIForm::GetOverlayRects() {
        if ( OverlayItem >= 0 ) {
            return Items[OverlayItem]->GetOverlayRects();
        }
        return {};
    }

    void UIForm::DrawOverlay( float parentX, float parentY ) {
        if ( OverlayItem >= 0 ) {
            Items[OverlayItem]->DrawOverlay( parentX, parentY );
        }
    }

    bool UIForm::OverlayHit() {
        if ( IsRoot ) {
            vec2 mPos = GetGlobalMousePos();
            for ( int i = 0; i < OverlayRects.size(); i++ ) {
                if ( OverlayRects[i].Contains( {mPos.x, mPos.y} ) ) {
                    return true;
                }
            }
            return false;
        }
        else if ( OverlayItem >= 0 ) {
            const vec2 mPos = GetGlobalMousePos();
            const auto overR = Items[OverlayItem]->GetOverlayRects();
            for ( const ImRect & oR : overR ) {
                if ( VecInBB( mPos, oR ) ) { return true; }
            }
            return false;
        }
        return false;
    }

    void UIForm::AddOverlayItem( int inIndex, ImRect inRect ) {
        for ( int i = 0; i < OverlayItems.size(); i++ ) {
            if ( OverlayItems[i] == inIndex ) {
                OverlayRects[i] = inRect;
                return;
            }
        }
        OverlayItems.push_back( inIndex );
        OverlayRects.push_back( inRect );
    }

    void UIForm::DrawOverlayItems( float inX, float inY ) {
        vector<int> eraseList = {};
        for ( int i = 0; i < OverlayItems.size(); i++ ) {
            auto it = Items[ OverlayItems[i] ];
            it->DrawOverlay( inX, inY );
            if ( !it->GetFocus() ) { 
                eraseList.push_back(i);
            }
        }
        for ( int i = (int) eraseList.size()-1; i>= 0; i-- ) {
            OverlayItems.erase( OverlayItems.begin() + eraseList[i] );
            OverlayRects.erase( OverlayRects.begin() + eraseList[i] );
        }
    }

    void UIForm::ProcessKeyEvents() {

        int keyD = GetKeyDown();
        int keyChange = UILibIS->Key.Change;
        bool shiftKey = UILibIS->Shift;

        if ( keyChange == 1 ) {
            if ( GetFocus() ) {
                auto it = Items[FocusedItem];
                if ( it->ProcessKeyDown( keyD ) ) { return; } // Kick out if the focused item handles the key down
            }
            if ( keyD == 256 ) {              // Esc Key -- exits current focus, otherwise does nothing
                if ( GetFocus() ) {
                    auto it = Items[FocusedItem];
                    SetFocus( it->EscKeyFocus() );
                }
            } else if ( keyD == 257 || keyD == 335 ) {  // Enter Key -- processes enter key when in focus
                if ( GetFocus() ) {
                    auto it = Items[FocusedItem];
                    SetFocus( it->EnterKeyFocus() );
                }
            } else if ( keyD == 258 ) {       // Tab key navigates between focused items
                if ( GetFocus() ) {
                    // std::cout << "** Got to here TAB KEY NEXT FOCUS, Form: " << Index << " ****" << std::endl;
                    SetNextFocus( ( shiftKey ) ? -1 : 1, true );
                } else {
                    SetFocus( true );
                }
                ScrollToFocused();
            } else if ( keyD >= 262 && keyD <= 265 ) {          // 262 = Arrow Right, 263 = Arrow Left, 264 = Arrow Down, 265 = Arrow Up
                if ( GetFocus() ) {
                    auto it = Items[FocusedItem];
                    SetFocus( it->ArrowKeyFocus( keyD ) );
                }
            }
            if ( GetFocus() ) {
                UILibIS->Key.Reset = true;      // Signal keyboard reset on next frame update
            }
        }

    }

    bool UIForm::ScrollChange( float inX, float inY, float inChange ) {
        // Check whether scrollable sub-items gets the scroll change
        float xShift = 0, yShift = 0;
        if ( Direction % 2 == 0 ) { yShift = -ScrollPerc * (ContentDim.y + Padding[2] + Padding[3] - Dim.y); }
        else { xShift = -ScrollPerc * (ContentDim.x + Padding[0] + Padding[1] - Dim.x); }
        for ( int i = 0; i < ScrollableItems.size(); i++ ) { 
            if ( Items[ ScrollableItems[i] ]->ScrollChange( inX+xShift, inY+yShift, inChange ) ) { return true; }
        }
        // Check whether the current form item gets the scroll change
        if ( !AllowScroll ) { return false; }
        ImRect itemBB = GetItemRect( inX, inY );
        vec2 mPos = GetGlobalMousePos();
        if ( VecInBBClip( mPos, itemBB ) ) {
            if ( Direction % 2 == 0 ) {     // ------------ vertical ------------
                if ( !UILibIS->Shift ) {    // Mouse Wheel ONLY for vertical scrolling
                    float OffScreenHeight = ContentDim.y + Padding[2] + Padding[3] - Dim.y;
                    float newPerc = ScrollPerc + inChange * (UILibCTX->DefaultScrollPixels/OffScreenHeight) * -UILibCTX->DefaultScrollUnits;
                    if(newPerc > 1){newPerc = 1;}
                    if(newPerc < 0){newPerc = 0;}
                    ScrollPerc = newPerc;
                }

            } else {                        // ------------ horizontal ------------
                if ( UILibIS->Shift ) {     // SHIFT + Mouse Wheel for horizontal scrolling
                    float OffScreenWidth = ContentDim.x + Padding[0] + Padding[1] - Dim.x;
                    float newPerc = ScrollPerc + inChange * (UILibCTX->DefaultScrollPixels/OffScreenWidth) * -UILibCTX->DefaultScrollUnits;
                    if(newPerc > 1){newPerc = 1;}
                    if(newPerc < 0){newPerc = 0;}
                    ScrollPerc = newPerc;
                }
            }
            return true;
        }
        return false;
    }

    void UIForm::CollapseAll( bool inCollapse ) {
        for ( auto it : Items ) {
            if ( it->ItemType == uif_collapserbutton ) {
                UIFormCollapserButton * cb = reinterpret_cast<UIFormCollapserButton*>(it);
                cb->SetState( !inCollapse );
            }
            else if ( it->ItemType == uif_labeledcollapserbutton ) {
                UIFormLabeledCollapserButton * cb = reinterpret_cast<UIFormLabeledCollapserButton*>(it);
                cb->Collapser.SetState( !inCollapse );
            }
            else if ( it->ItemType == uif_form ) {
                UIForm * f = reinterpret_cast<UIForm*>(it);
                f->CollapseAll( inCollapse );
            }
        }
    }

    void UIForm::ResizeScrollAdjust( const vec2 & oldContentDim ) {
        if ( ScrollPerc == 0 ) { return; }
        if ( oldContentDim == ContentDim ) { return; }
        if ( oldContentDim.x == 0 || oldContentDim.y == 0 ) { return; }
        if ( ContentDim.x == 0 || ContentDim.y == 0 ) { return; }
        if ( Direction % 2 == 0 ) {     // ------------ vertical ------------
            float OffScreenHeightOld = oldContentDim.y + Padding[2] + Padding[3] - Dim.y;
            float OffScreenHeight = ContentDim.y + Padding[2] + Padding[3] - Dim.y;
            float newPerc = ScrollPerc * OffScreenHeightOld / OffScreenHeight;
            if(newPerc > 1){newPerc = 1;}
            if(newPerc < 0){newPerc = 0;}
            if ( OffScreenHeightOld > OffScreenHeight && 1 - newPerc < .00001 ) { ContentDim.y = oldContentDim.y; }
            else { ScrollPerc = newPerc; }

        } else {                        // ------------ horizontal ------------
            float OffScreenWidthOld = oldContentDim.x + Padding[0] + Padding[1] - Dim.x;
            float OffScreenWidth = ContentDim.x + Padding[0] + Padding[1] - Dim.x;
            float newPerc = ScrollPerc * OffScreenWidthOld / OffScreenWidth;
            if(newPerc > 1){newPerc = 1;}
            if(newPerc < 0){newPerc = 0;}
            if ( OffScreenWidthOld > OffScreenWidth && 1 - newPerc < .00001 ) { ContentDim.x = oldContentDim.x; }
            else { ScrollPerc = newPerc; }
        }
    }

    void UIForm::ScrollToFocused() {
        if ( FocusedItem == -1 ) { return; }
        // std::cout << "****---- Got to here 0 ----****" << std::endl;

        // Uses the most recently drawn X,Y,W,H
        ImRect formRect = GetContentRect();

        if ( Direction == 0 ) {     // ------------ vertical ------------
            
            float totalH = ContentDim.y + Padding[2] + Padding[3];
            bool scrollOn = ( totalH > Dim.y + 0.001f );
            if ( !scrollOn ) { return; }

            float offScreen = totalH - Dim.y;
            float yShift = -ScrollPerc * offScreen;

            ImRect focusRect = Items[FocusedItem]->GetFocusedRect( Pos.x, Pos.y + yShift );

            float yDelta = 0;
            if ( focusRect.Max.y < formRect.Min.y ) { 
                yDelta = formRect.Min.y - focusRect.Min.y; 
                ScrollPerc = ( -(yShift+yDelta) / offScreen );
            }
            else if ( focusRect.Min.y > formRect.Max.y ) {
                yDelta = focusRect.Max.y - formRect.Max.y; 
                ScrollPerc = ( -(yShift-yDelta) / offScreen );
            }
            
            
        } else if ( Direction == 1 ) {                    // ------------ horizontal ------------

            float totalW = ContentDim.x + Padding[0] + Padding[1];
            bool scrollOn = ( totalW > Dim.x + 0.001f );
            if ( !scrollOn ) { return; }

            float offScreen = totalW - Dim.x;
            float xShift = -ScrollPerc * offScreen;

            ImRect focusRect = Items[FocusedItem]->GetFocusedRect( Pos.x + xShift, Pos.y );

            float xDelta = 0;
            if ( focusRect.Max.x < formRect.Min.x ) { 
                xDelta = formRect.Min.x - focusRect.Min.x; 
                ScrollPerc = ( -(xShift+xDelta) / offScreen );
            }
            else if ( focusRect.Min.x > formRect.Max.x ) {
                xDelta = focusRect.Max.x - formRect.Max.x; 
                ScrollPerc = ( -(xShift-xDelta) / offScreen );
            }

        } else if ( Direction == 2 ) {     // ------------ vertical reverse ------------

            float totalH = ContentDim.y + Padding[2] + Padding[3];
            bool scrollOn = ( totalH > Dim.y + 0.001f );
            if ( !scrollOn ) { return; }

            float offScreen = totalH - Dim.y;
            float yShift = (1-ScrollPerc) * offScreen;

            ImRect focusRect = Items[FocusedItem]->GetFocusedRect( Pos.x, Pos.y + yShift );

            float yDelta = 0;
            if ( focusRect.Max.y < formRect.Min.y ) { 
                yDelta = formRect.Min.y - focusRect.Min.y; 
                ScrollPerc = 1 - ( (yShift+yDelta) / offScreen );
            }
            else if ( focusRect.Min.y > formRect.Max.y ) {
                yDelta = focusRect.Max.y - formRect.Max.y; 
                ScrollPerc = 1 - ( (yShift-yDelta) / offScreen );
            }


        } else {                    // ------------ horizontal reverse ------------

            float totalW = ContentDim.x + Padding[0] + Padding[1];
            bool scrollOn = ( totalW > Dim.x + 0.001f );
            if ( !scrollOn ) { return; }

            float offScreen = totalW - Dim.x;
            float xShift = (1-ScrollPerc) * offScreen;

            ImRect focusRect = Items[FocusedItem]->GetFocusedRect( Pos.x + xShift, Pos.y );

            float xDelta = 0;
            if ( focusRect.Max.x < formRect.Min.x ) { 
                xDelta = formRect.Min.x - focusRect.Min.x; 
                ScrollPerc = 1 - ( (xShift+xDelta) / offScreen );
            }
            else if ( focusRect.Min.x > formRect.Max.x ) {
                xDelta = focusRect.Max.x - formRect.Max.x; 
                ScrollPerc = 1 - ( (xShift-xDelta) / offScreen );
            }

        }

    }

    bool UIForm::SetNextFocus( int inDirection, bool inWrap ) {
        // TO DO: Need to update this function to handle Collapsed and Non-Visible items.

        // std::cout << "Got to SetNextFocus, Form: " << Index << std::endl;

        int startIndex = (inDirection == 1 ) ? 0 : (int) Items.size()-1 ;
        int currFocused = ( FocusedItem > -1 ) ? FocusedItem : startIndex ;
        int nextFocused;
        bool done = false;

        if ( FocusedItem == -1 ) {
            done = Items[currFocused]->SetFocus( true, inDirection );
            if ( done ) { FocusedItem = currFocused; return true;}
        }

        while ( !done ) {
            
            // std::cout << "  Looping Form: " << Index  << ", currFocused: " << currFocused << std::endl;
            done = Items[currFocused]->SetNextFocus( inDirection, false );
            if ( done ) { 
                FocusedItem = currFocused; 
                // std::cout << "  Form " << Index << ", keeping current focus" << std::endl; 
                return true;
            }
            
            // std::cout << "  Looping Form: " << Index  << ",   Got to 1" << std::endl;
            
            if ( inWrap ) {
                // std::cout << "  Looping Form: " << Index  << ",   Got to 2" << std::endl;
                if ( inDirection == 1 ) { nextFocused = (currFocused >= Items.size()-1 ) ? 0 : currFocused+1; } 
                else { nextFocused = (currFocused <= 0 ) ? (int) Items.size()-1 : currFocused-1; }
                
            }
            else {
                // std::cout << "  Looping Form: " << Index  << ",   Got to 3" << std::endl;
                if ( inDirection == 1 ) {
                    // std::cout << "  Looping Form: " << Index  << ",   Got to 3A" << std::endl;
                    if ( currFocused >= Items.size()-1 ) { 
                        // std::cout << "  Looping Form: " << Index  << ",   Got to 3A - 1" << std::endl; 
                        nextFocused = -1; 
                    } 
                    else { 
                        // std::cout << "  Looping Form: " << Index  << ",   Got to 3A - 2" << std::endl; 
                        nextFocused = currFocused + 1; 
                    }
                } else { 
                    // std::cout << "  Looping Form: " << Index  << ",   Got to 3B" << std::endl;
                    if ( currFocused <= 0 ) { nextFocused = -1; } 
                    else { nextFocused = currFocused - 1; }
                }
            }

            if ( nextFocused == -1 ) { 
                SetFocus(false); 
                // std::cout<< "  Form: " << Index << ", exit form focus" << std::endl; 
                return false; 
            }
            
            currFocused = nextFocused;  
            done = Items[currFocused]->SetFocus( true, inDirection );
            if ( done ) { 
                FocusedItem = currFocused; 
                // std::cout << "  Form " << Index << ", Got focus on " << currFocused << std::endl; 
                return true; 
            }

        }

        return done;
    }

    // ------------ Removing Form Items ------------

    void UIForm::RemoveItem( int inIndex ) {
        if ( inIndex > Items.size() ) { return; }
        if ( inIndex < 0 ) { return; }
        int formIndex = inIndex;
        for ( int i = formIndex + 1; i < Items.size(); i++ ) {
            auto it = Items[i];
            it->Index--;
        }
        if (Items[formIndex]->ItemType != uif_custom_external) delete Items[formIndex];
        Items.erase(Items.begin() + formIndex );
        for ( int i = (int)ScrollableItems.size()-1; i >= 0; i-- ) {
            if ( ScrollableItems[i] == formIndex ) {
                ScrollableItems.erase( ScrollableItems.begin() + i );
            }
            else if ( ScrollableItems[i] > formIndex ) {
                ScrollableItems[i]--;
            }
        }
        NeedUpdate = true;
    }

    void UIForm::RemoveItems( int inStartI, int inEndI ) {
        if ( inStartI > Items.size() || inEndI > Items.size() ) { return; }
        if ( inStartI < 0 || inEndI < 0 ) { return; }
        if ( inEndI < inStartI ) { return;}
        int nbItems = inEndI - inStartI;
        for ( int i = inEndI; i < Items.size(); i++ ) {
            auto it = Items[i];
            it->Index -= nbItems;
        }
        for ( int i = inStartI; i < inEndI; i++ ) {
            std::cout << "    Form deleting item: " << i << std::endl;
            if (Items[i]->ItemType != uif_custom_external) delete Items[i];
        }
        Items.erase(Items.begin() + inStartI, Items.begin() + inEndI );
        for ( int i = (int)ScrollableItems.size()-1; i >= 0; i-- ) {
            if ( ScrollableItems[i] >= inStartI && ScrollableItems[i] < inEndI ) {
                ScrollableItems.erase( ScrollableItems.begin() + i );
            }
            else if ( ScrollableItems[i] > inEndI ) {
                ScrollableItems[i] -= nbItems;
            }
        }
        NeedUpdate = true;
    }
    

    // ------------ Adding Form Items ------------

    UIForm* UIForm::AddForm( int inDir ) {
        UIForm * item = new UIForm( "", inDir, (int) Items.size() );
        item->IsRoot = false;
        item->AllowScroll = false;
        ScrollableItems.push_back( (int) Items.size() );
        Items.push_back( item );
        return item;
    }

    UIForm* UIForm::InsertForm( int inIndex, int inDir ) {
        int formIndex = inIndex;
        if ( inIndex > Items.size() ) { formIndex = (int) Items.size(); }
        else if ( inIndex < 0 ) { formIndex = 0; }
        for ( auto & index : ScrollableItems ) { if ( index >= formIndex ) { index++; } }
        UIForm * item = new UIForm( "", inDir, formIndex );
        item->IsRoot = false;
        Items.insert(Items.begin() + formIndex, item );
        for ( int i = formIndex + 1; i < Items.size(); i++ ) {
            auto it = Items[i];
            it->Index++;
        }
        ScrollableItems.push_back( formIndex );
        return item;
    }

    UIFormSpacer* UIForm::AddSpacer( string inVal ) {
        UIFormSpacer* item = new UIFormSpacer();
        item->Index = (int) Items.size();
        if ( inVal == "" ) { item->Expand = 1; }
        if ( Direction % 2 == 0 ) { item->H = inVal; } else { item->W = inVal; }
        Items.push_back( item );
        return item;
    }

    UIFormSpacer* UIForm::AddSpacer( float inVal ) {
        UIFormSpacer* item = new UIFormSpacer();
        item->Index = (int) Items.size();
        if ( Direction % 2 == 0 ) { item->H = to_string(inVal); } else { item->W = to_string(inVal); }
        Items.push_back( item );
        return item;
    } 

    UIFormSpacer* UIForm::InsertSpacer( int inIndex, string inVal ) {
        int formIndex = inIndex;
        if ( inIndex > Items.size() ) { formIndex = (int) Items.size(); }
        else if ( inIndex < 0 ) { formIndex = 0; }
        UIFormSpacer* item = new UIFormSpacer();
        item->Index = formIndex;
        if ( inVal == "" ) { item->Expand = 1; }
        if ( Direction % 2 == 0 ) { item->H = inVal; } else { item->W = inVal; }
        Items.insert(Items.begin() + inIndex, item );
        for ( int i = formIndex + 1; i < Items.size(); i++ ) {
            auto it = Items[i];
            it->Index++;
        }
        for ( auto & index : ScrollableItems ) { if ( index >= formIndex ) { index++; } }
        return item;
    }

    UIFormSpacer* UIForm::InsertSpacer( int inIndex, float inVal ) {
        int formIndex = inIndex;
        if ( inIndex > Items.size() ) { formIndex = (int) Items.size(); }
        else if ( inIndex < 0 ) { formIndex = 0; }
        UIFormSpacer* item = new UIFormSpacer();
        item->Index = formIndex;
        if ( Direction % 2 == 0 ) { item->H = to_string(inVal); } else { item->W = to_string(inVal); }
        Items.insert(Items.begin() + inIndex, item );
        for ( int i = formIndex + 1; i < Items.size(); i++ ) {
            auto it = Items[i];
            it->Index++;
        }
        for ( auto & index : ScrollableItems ) { if ( index >= formIndex ) { index++; } }
        return item;
    }   

    UIFormDivider* UIForm::AddDivider( float inThickness ) {
        UIFormDivider* item = new UIFormDivider();
        item->Index = (int) Items.size();
        item->Thickness = inThickness;
        if ( Direction % 2 == 0 ) { item->Direction = 1; } else { item->Direction = 0; }
        Items.push_back( item );
        return item;
    }

    UIFormRectangle* UIForm::AddRectangle( string inW, string inH ) {
        UIFormRectangle* item = new UIFormRectangle();
        item->Index = (int) Items.size();
        item->W = inW; item->H = inH;
        Items.push_back( item );
        return item;
    }

    UIFormRectangleDynamicFill * UIForm::AddRectangleDynamicFill( int & inColor, string inW, string inH ) {
        UIFormRectangleDynamicFill* item = new UIFormRectangleDynamicFill( inColor );
        item->Index = (int) Items.size();
        item->W = inW; item->H = inH;
        Items.push_back( item );
        return item;
    }

    UIFormCenteredShape * UIForm::AddCenteredShape( bool & inClick, int inType, float inSize, string inW, string inH ) {
        UIFormCenteredShape* item = new UIFormCenteredShape(inClick);
        item->Click = inClick;
        item->Index = (int) Items.size();
        item->W = inW; item->H = inH;
        item->ShapeType=inType;item->Size=inSize;
        Items.push_back( item );
        return item;
    }

    UIFormGrid* UIForm::AddGrid( int nRows, int nCols ) {
        UIFormGrid* item = new UIFormGrid( nRows, nCols );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }

    UIFormHeader* UIForm::AddHeader( string inText ) {
        UIFormHeader* item = new UIFormHeader( inText );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }

    UIFormText* UIForm::AddText( string inText ) {
        UIFormText* item = new UIFormText( inText );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }

    UIFormTextAB* UIForm::AddTextAB( bool & inCondition, string inText ) {
        UIFormTextAB* item = new UIFormTextAB( inCondition, inText );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }

    UIFormTextInput* UIForm::AddTextInput( string & inText ) {
        UIFormTextInput* item = new UIFormTextInput( inText );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }

    UIFormLabeledText* UIForm::AddLabeledText( string inText, string inLabel ) {
        UIFormLabeledText* item = new UIFormLabeledText( inText, inLabel );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }

    UIFormLabeledTextInput* UIForm::AddLabeledTextInput( string & inText, string inLabel ) {
        UIFormLabeledTextInput* item = new UIFormLabeledTextInput( inText, inLabel );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }

    UIFormDateInput * UIForm::AddDateInput( string & inText, bool inShowTime ) {
        UIFormDateInput* item = new UIFormDateInput( inText, inShowTime );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }

    UIFormLabeledDateInput* UIForm::AddLabeledDateInput( string & inText, string inLabel, bool inShowTime ) {
        UIFormLabeledDateInput* item = new UIFormLabeledDateInput( inText, inShowTime, inLabel );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }

    UIFormParagraphInput * UIForm::AddParagraphInput( const string & inText, bool inViewOnly, bool inWordWrap ) {
        UIFormParagraphInput* item = new UIFormParagraphInput( inText, inViewOnly, inWordWrap );
        item->Index = (int) Items.size();
        ScrollableItems.push_back( (int) Items.size() );
        Items.push_back( item );
        return item;
    }

    UIFormImage* UIForm::AddImage( int inTextureId, vec2 inImageDim, vec4 inPixelXYWH ) {
        UIFormImage* item = new UIFormImage( inTextureId, inImageDim, inPixelXYWH );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }

    UIFormImage* UIForm::AddImage( string inPath, vec2 inImageDim, vec4 inPixelXYWH ) {
        UIFormImage* item = new UIFormImage( inPath, inImageDim, inPixelXYWH );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }

    UIFormButton* UIForm::AddButton( bool & inClick ) {
        UIFormButton* item = new UIFormButton( inClick );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }

    UIFormLabeledButton* UIForm::AddLabeledButton( bool & inClick, string inLabel ) {
        UIFormLabeledButton* item = new UIFormLabeledButton( inClick, inLabel );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }

    UIFormImageButton* UIForm::AddImageButton( bool & inClick, int inTextureId, vec2 inImageDim, vec4 inPixelXYWH  ) {
        UIFormImageButton* item = new UIFormImageButton( inClick, inTextureId, inImageDim, inPixelXYWH );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }

    UIFormImageButton* UIForm::AddImageButton( bool & inClick, string inPath, vec2 inImageDim, vec4 inPixelXYWH  ) {
        UIFormImageButton* item = new UIFormImageButton( inClick, inPath, inImageDim, inPixelXYWH );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }

    UIFormLabeledImage* UIForm::AddLabeledImage(int inTextureId, vec2 inImageDim, vec4 inPixelXYWH, string inLabel ) {
        UIFormLabeledImage* item = new UIFormLabeledImage( inTextureId, inImageDim, inPixelXYWH, inLabel );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }

    UIFormLabeledImage* UIForm::AddLabeledImage(string inPath, vec2 inImageDim, vec4 inPixelXYWH, string inLabel ) {
        UIFormLabeledImage* item = new UIFormLabeledImage( inPath, inImageDim, inPixelXYWH, inLabel );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }

    UIFormLabeledImageButton* UIForm::AddLabeledImageButton( bool & inClick, int inTextureId, vec2 inImageDim, vec4 inPixelXYWH, string inLabel ) {
        UIFormLabeledImageButton* item = new UIFormLabeledImageButton( inClick, inTextureId, inImageDim, inPixelXYWH, inLabel );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }

    UIFormLabeledImageButton* UIForm::AddLabeledImageButton( bool & inClick, string inPath, vec2 inImageDim, vec4 inPixelXYWH, string inLabel ) {
        UIFormLabeledImageButton* item = new UIFormLabeledImageButton( inClick, inPath, inImageDim, inPixelXYWH, inLabel );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }

    UIFormLabeledSelImageButton* UIForm::AddLabeledSelImageButton( bool & inClick, int inTextureId, vec2 inImageDim, vec4 inPixelXYWH, string inLabel ) {
        UIFormLabeledSelImageButton* item = new UIFormLabeledSelImageButton( inClick, inTextureId, inImageDim, inPixelXYWH, inLabel );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }

    UIFormLabeledSelImageButton* UIForm::AddLabeledSelImageButton( bool & inClick, string inPath, vec2 inImageDim, vec4 inPixelXYWH, string inLabel ) {
        UIFormLabeledSelImageButton* item = new UIFormLabeledSelImageButton( inClick, inPath, inImageDim, inPixelXYWH, inLabel );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }


    UIFormSelImageButton* UIForm::AddSelImageButton( bool & inClick, int inTextureId, vec2 inImageDim, vec4 inPixelXYWH  ) {
        UIFormSelImageButton* item = new UIFormSelImageButton( inClick, inTextureId, inImageDim, inPixelXYWH );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }

    UIFormSelImageButton* UIForm::AddSelImageButton( bool & inClick, string inPath, vec2 inImageDim, vec4 inPixelXYWH  ) {
        UIFormSelImageButton* item = new UIFormSelImageButton( inClick, inPath, inImageDim, inPixelXYWH );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }

    UIFormTextButton* UIForm::AddTextButton( bool & inClick, string inText ) {
        UIFormTextButton * item = new UIFormTextButton( inClick, inText );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }

    UIFormLabeledTextButton* UIForm::AddLabeledTextButton( bool & inClick, string inText, string inLabel ) {
        UIFormLabeledTextButton* item = new UIFormLabeledTextButton( inClick, inText, inLabel );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }

    UIFormCollapserButton* UIForm::AddCollapserButton( float inB, float inH ) {
        UIFormCollapserButton * item = new UIFormCollapserButton( inB, inH, NeedUpdate );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }

    UIFormCollapserButton* UIForm::AddCollapserButton( float inB, float inH, bool & inNeedUpdate ) {
        UIFormCollapserButton * item = new UIFormCollapserButton( inB, inH, inNeedUpdate );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }

    UIFormLabeledCollapserButton* UIForm::AddLabeledCollapserButton( float inB, float inH, string inLabel ) {
        UIFormLabeledCollapserButton* item = new UIFormLabeledCollapserButton( inB, inH, NeedUpdate, inLabel );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }

    UIFormLabeledCollapserButton* UIForm::AddLabeledCollapserButton( float inB, float inH, bool & inNeedUpdate, string inLabel ) {
        UIFormLabeledCollapserButton* item = new UIFormLabeledCollapserButton( inB, inH, inNeedUpdate, inLabel );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }

    UIFormSelTextButton* UIForm::AddSelTextButton(  bool & inClick, string inText ) {
        UIFormSelTextButton * item = new UIFormSelTextButton( inClick, inText );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }

    UIFormButtonRow* UIForm::AddButtonRow() {
        UIFormButtonRow* item = new UIFormButtonRow();
        item->Index = (int) Items.size();
        ScrollableItems.push_back( (int) Items.size() );
        Items.push_back( item );
        return item;
    }

    UIFormSelButtonRow* UIForm::AddSelButtonRow( int & inButtonIndex  ) {
        UIFormSelButtonRow* item = new UIFormSelButtonRow( inButtonIndex );
        item->Index = (int) Items.size();
        ScrollableItems.push_back( (int) Items.size() );
        Items.push_back( item );
        return item;
    }

    UIFormSelButtonGrid* UIForm::AddSelButtonGrid( int & inButtonIndex, int nRows, int nCols ) {
        UIFormSelButtonGrid* item = new UIFormSelButtonGrid( inButtonIndex, nRows, nCols );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }

    UIFormSlider* UIForm::AddSlider( float & inValue, bool & inMute, bool & inChange, vec2 inBounds ) {
        UIFormSlider* item = new UIFormSlider( inValue, inMute, inChange, inBounds );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }

    UIFormLabeledSlider* UIForm::AddLabeledSlider( float & inValue, bool & inMute, bool & inChange,  vec2 inBounds ) {
        UIFormLabeledSlider* item = new UIFormLabeledSlider( inValue, inMute, inChange, inBounds );
        item->Index = (int) Items.size();
        item->Slider.Expand = 1; 
        item->ValueLabel.Alignment = {0.5f,0.5f};
        Items.push_back( item );
        return item;
    }

    UIFormAnimationTimeline* UIForm::AddAnimationTimeline( double inStart, double inEnd, double inCurr ) {
        UIFormAnimationTimeline* item = new UIFormAnimationTimeline();
        item->Timeline.StartVal = inStart;
        item->Timeline.EndVal = inEnd;
        item->Timeline.CurrVal = inCurr;
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }

    UIFormComboBox* UIForm::AddComboBox( string inName, vector<string> & inItemList ) {
        static uint32_t comboIndex = 0;
        if ( comboIndex == UINT32_MAX ) { comboIndex = 0; }
        string comboName = ( inName == "" ) ? "UIFormCombo_" + to_string(comboIndex++) : inName;
        // std::cout << "Add Combo: " << comboName << std::endl;
        UIFormComboBox* item = new UIFormComboBox( comboName, inItemList );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }

    UIFormLabeledComboBox* UIForm::AddLabeledComboBox( string inName, vector<string> & inItemList, string inLabel ) {
        UIFormLabeledComboBox* item = new UIFormLabeledComboBox( inName, inItemList, inLabel );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }

    UIFormCheckBox* UIForm::AddCheckBox( bool & inCheck, float inSize ) {
        UIFormCheckBox* item = new UIFormCheckBox( inCheck );
        item->Index = (int) Items.size();
        if ( inSize > -1 ) {
            if ( Direction % 2 == 0 ) { // ------------ vertical ------------
                item->H = to_string( inSize );
            } else {                      // ------------ horizontal ------------
                item->W = to_string( inSize );
            }
        }
        Items.push_back( item );
        return item;
    }

    UIFormLabeledCheckBox* UIForm::AddLabeledCheckBox( bool & inCheck, float inSize, string inLabel ) {
        UIFormLabeledCheckBox* item = new UIFormLabeledCheckBox( inCheck, inSize, inLabel );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }

    UIFormCollapserCheckBox* UIForm::AddCollapserCheckBox( bool & inCheck, bool & inNeedUpdate, float inSize ) {
        UIFormCollapserCheckBox* item = new UIFormCollapserCheckBox( inCheck, inNeedUpdate );
        item->Index = (int) Items.size();
        if ( inSize > -1 ) {
            if ( Direction % 2 == 0 ) { // ------------ vertical ------------
                item->H = to_string( inSize );
            } else {                      // ------------ horizontal ------------
                item->W = to_string( inSize );
            }
        }
        Items.push_back( item );
        return item;
    }

    UIFormLabeledCollapserCheckBox* UIForm::AddLabeledCollapserCheckBox( bool & inCheck, bool & inNeedUpdate, float inSize, string inLabel ) {
        UIFormLabeledCollapserCheckBox* item = new UIFormLabeledCollapserCheckBox( inCheck, inNeedUpdate, inSize, inLabel );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }


    int UIForm::AddCustomItem( UIFormItem* CustomItem ) {
        // Adds a custom UIFormItem to the form.
        // Set CustomItem->ItemType = uif_custom if you want UIForm to delete the CustomItem on exit.
        // Otherwise, set Custom->ItemType = uif_custom_external if you want the CustomItem to persist after the form exits.
        CustomItem->Index = (int) Items.size();
        Items.push_back( CustomItem );
        return CustomItem->Index;
    }

    UIFormLabeledItem* UIForm::AddCustomLabeledItem( string inLabel, UIFormItem* CustomItem ) {
        // Adds a custom labeled UIFormItem to the form.
        // Set CustomItem->ItemType = uif_custom if you want UIForm to delete the CustomItem on exit.
        // Otherwise, set Custom->ItemType = uif_custom_external if you want the CustomItem to persist after the form exits.
        CustomItem->Index = 1;
        UIFormLabeledItem* item = new UIFormLabeledItem( inLabel, CustomItem );
        item->Index = (int) Items.size();
        Items.push_back( item );
        return item;
    }


    

} // end namespace UILib

#endif /* UILIBFORM_CPP */
