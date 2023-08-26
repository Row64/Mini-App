#ifndef TUI_CHARTSHEET_CPP
#define TUI_CHARTSHEET_CPP

#define _USE_MATH_DEFINES
#include <math.h>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <utility>
#include <iostream>
#include <string>

#include "c_PipelineManager.h"

#include "CDR_Example.h"
using namespace Blocks;

namespace AppCore {
    
    // ============================================================================================ //
    //                                   CONSTRUCTOR / DESTRUCTOR                                   //
    // ============================================================================================ //

    ChartSheet::ChartSheet( PipelineManager &parent ) :
        TUI_PaneBase( parent ) {
        PaneType = "ChartSheet";
        Config = new ChartSConfig();
    }

    ChartSheet::~ChartSheet() {
        if ( Config ) { delete Config; }
    }

    // ============================================================================================ //
    //                              Override Functions from PaneBase                                //
    // ============================================================================================ //

    void ChartSheet::InitPane() {
    }

    void ChartSheet::DrawPane() {
        if ( !Linked ) { ConfigHS(); }
        StartPane();      
        if ( DrawObj ) {
            DrawObj->Draw();
        }
        FinishPane();
    }

    void ChartSheet::ConfigHS() {

        Config->Category = "Canvas";
        Config->Type = "Example";

        LoadObj = std::make_unique<CLO_Example>( Config, PaneIndex );
        DrawObj = std::make_unique<CDR_Example>( Config, LoadObj.get(), nullptr, PaneIndex, X, Y, Width, Height );

        LoadObj->Init(); DrawObj->Init();
        
        Linked = true;

    }

    int ChartSheet::ListenForMessage(string inType, vector<string> inM){

        // if ( inType == "UpdateColorTheme" ) {
        //     ProcessUpdateColorTheme( inM );
        //     return 2;
        // }

        return 0;
    }

    // ============================================================================================ //
    //                                   Event Syncing Functions                                    //
    // ============================================================================================ //

    // ============================================================================================ //
    //                            Listen for Message Receiver Functions                             //
    // ============================================================================================ //

   
    // ============================================================================================ //
    //                                  Receiver Function Helpers                                   //
    // ============================================================================================ //


} // end namespace AppCore

#endif // TUI_CHARTSHEET_CPP
