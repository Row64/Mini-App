#ifndef CLO_BASE_CPP
#define CLO_BASE_CPP

#include "c_PipelineManager.h"
#include "CLO_Base.h"

using namespace AppCore;
using namespace Blocks;

CLO_Base::CLO_Base( ChartSConfig * inConfig, int inPaneIndex ) :
    Config( inConfig ),
    PaneIndex( inPaneIndex ),
    SheetName( inConfig->Name ) {
};

CLO_Base::~CLO_Base() {}

void CLO_Base::Reset() {
    std::cout << "Got to " << CLOType << "::Reset for Pane " << PaneIndex << std::endl;
    Details = {};
    Loaded = false;
    TitleUpdate = true;
}


#endif /* CLO_BASE_CPP */
