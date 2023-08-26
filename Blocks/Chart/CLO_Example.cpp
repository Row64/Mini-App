#ifndef CLO_EXAMPLE_CPP
#define CLO_EXAMPLE_CPP

#include "c_PipelineManager.h"
#include "CLO_Example.h"

using namespace AppCore;
using namespace std;


CLO_Example::CLO_Example( Blocks::ChartSConfig * inConfig, int inPaneIndex ) :
    CLO_Base( inConfig, inPaneIndex ) {
    CLOType = "CLO_Example";
}

void CLO_Example::Init() {
    if ( Config == nullptr ) { return; }
    if ( Loaded ) { Reset(); }

    // Load & initialize data needed for draw
    int NbValues = (int) std::min( CData.XValues.size(), CData.YValues.size() );
    for ( int i = 0; i < NbValues; i++ ) {
        Points.push_back( vec2( CData.XValues[i], CData.YValues[i] ) );
    }

    // Flag Loaded
    Loaded = true;

}

void CLO_Example::FullUpdate() {
    Init();
}

void CLO_Example::Reset() {
    CData = {};
    FData = {};
    Points.clear();
}

#endif /* CLO_EXAMPLE_CPP */
