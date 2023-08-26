#ifndef _BLOCK_PPG_TEMPLATE_H
#define _BLOCK_PPG_TEMPLATE_H

// How to use this template:
// 1. Duplicate this to a new .h and .cpp files with the name of your PPG
// 2. Replace "Test" with your PPG name (for example, OpenTestPPG become OpenMyStuffPPG)
// 3. Include the .h file in "Blocks.h"
// 4. Add your Draw___PPG() function in the "Block_PPG.cpp" DrawPPGs() function

#include "UILib.h"
using namespace UILib;

namespace Blocks {

    void OpenTestPPG( const std::string & inName, int inMode = 0 );
    void InitTestPPG(PropertyPage &inPPG);
    void DrawTestPPG(PropertyPage &inPPG, float inW, float inH);
    void ExitTestPPG(PropertyPage &inPPG);
    int  UpdateTestPPG(PropertyPage &inPPG, string inType, vector<string> inMessage);   // Optional function for handling PPG messages

} // end namespace Blocks

#endif /* _BLOCK_PPG_TEMPLATE_H */
