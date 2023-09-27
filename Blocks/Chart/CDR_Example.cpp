#ifndef CDR_EXAMPLE_CPP
#define CDR_EXAMPLE_CPP

#include "c_PipelineManager.h"
#include "CDR_Example.h"

using namespace AppCore;

CDR_Example::CDR_Example( ChartSConfig * inConfig, CLO_Base * inCLO, Display3D * inDisp, int inPaneIndex, float &inX, float &inY, float &inW, float &inH ) :
    CDR_Base( inConfig, inCLO, inDisp, inPaneIndex, inX, inY, inW, inH ) {
    CDRType = "CDR_Example";
    Example = reinterpret_cast<CLO_Example*>(inCLO->GetSelf());
}

void CDR_Example::DrawPane() {
    DrawContentBB();
    DrawExample();
    DrawTitle();
}

void CDR_Example::DrawExample() {
    
    vec2 p0, p1;

    // Draw Axis
    int lineClr = 0xCDCDCD;
    vec2 center = LayoutBB.GetCenter();

    p0 = ContentToGlobalSpace( { center.x, LayoutBB.Min.y } );
    p1 = ContentToGlobalSpace( { center.x, LayoutBB.Max.y } );
    UIAddGlobalLine(p0.x, p0.y, p1.x, p1.y, lineClr, 1);

    p0 = ContentToGlobalSpace( { LayoutBB.Min.x, center.y } );
    p1 = ContentToGlobalSpace( { LayoutBB.Max.x, center.y } );
    UIAddGlobalLine(p0.x, p0.y, p1.x, p1.y, lineClr, 1);

    for ( int i = 0; i < Example->Points.size(); i++ ) {
        p0 = ContentToGlobalSpace( { center.x + Example->Points[i].x, center.y - Example->Points[i].y } ); // Y is inverted
        UIAddGlobalCircle( p0.x, p0.y, 5, Example->FData.Color, true, 1, 8);     
    }

    // CTRL-Right click OUTSIDE the ContentBB to open a context menu
    ImRect PaneBB = GetGlobalBB( 0, 0, Width, Height );
    vec2 mPos = GetGlobalMousePos();
    if ( RightDragStart() ) {
        ImRect cBB = ContentToGlobalSpace( ContentBB );
        bool openMenu = false;
        if ( ADIO->FM.ClickOK() && VecInBB( mPos, PaneBB ) && !VecInBB( mPos, cBB ) ) { openMenu = true; }
        int ppgI = GetPpgIndex( "Underlay PPG" );
        if ( ppgI != -1 && ADIO->MM.MouseHit( ppgI ) ) { openMenu = true; }       
        if ( openMenu ) {
            ADIO->FM.SetFocus( ft_context_menu, "Example_RightClick" );
            UISetContextMenu( "Example_RightClick", {"Add Floating PPG", "Open Underlay PPG", "Close Underlay PPG", "Open Overlay PPG", "Close Overlay PPG",
                            "----","Connect Server", "Test Server","Test HTTP",
                            "----","Open File Dialog", "Save File Dialog"
                            } );
        }
    }

}

#endif /* CDR_EXAMPLE_CPP */
