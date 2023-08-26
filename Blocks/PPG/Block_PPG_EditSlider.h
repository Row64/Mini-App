#ifndef BLOCK_PPG_EDITSLIDER_H
#define BLOCK_PPG_EDITSLIDER_H

#include "UILib.h"
using namespace UILib;

namespace Blocks {

    void OpenEditSliderPPG(std::string inPPGName2, int inSourcePPGIndex, int inRefID, float inW, float inH, float * inValue, float * inMin, float * inMax, bool * inChange, int * inID = nullptr);
    void OpenEditSliderPPG(PropertyPage &inPPG, std::string inPPGName2, int inRefID, float inW, float inH, UIFormLabeledSlider * inSlider );
    void InitEditSliderPPG(PropertyPage &inPPG);
    void DrawEditSliderPPG(PropertyPage &inPPG, float inW, float inH);
    void ExitEditSliderPPG(PropertyPage &inPPG);
    int  UpdateEditPPG(PropertyPage &inPPG, string inType, vector<string> inMessage);

} // end namespace Blocks

#endif /* BLOCK_PPG_EDITSLIDER_H */
