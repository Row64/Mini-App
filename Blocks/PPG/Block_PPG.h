#ifndef BLOCK_PPG_H
#define BLOCK_PPG_H

#include "UILib.h"
using namespace UILib;

namespace Blocks {

   // ------------ PPGs (Non-Modal) ------------
   // These are non-modal pop-ups that accept dynamic changes to sheets,
   // while moving and changing the pop-up.
   
   void DrawPPGs(float inW, float inH);
   void DrawBasePPG(PropertyPage & inPpg, int inI, bool inDrawName);
   PropertyPage * OpenSubPPG(PropertyPage &inPPG, string inName, vec2 inDim, int inRefIndex, float inW, float inH, vec2 inPos = {-1,-1} );  // inPos = reference position in global coords
   
} // end namespace Blocks

#endif // BLOCK_PPG_H