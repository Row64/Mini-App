#ifndef BLOCK_MODAL_POPUPS_H
#define BLOCK_MODAL_POPUPS_H


namespace Blocks {

   // ------------ Modal PopUps ------------
   // These are modal pop-ups which block all input from sheets,
   // and require hitting OK or Cancel on the dialog.

   void DrawModalPopUps(float inW, float inH);
   
   void DrawContextMenu(vector<string> inOptions, string inType, float inW, float inH);   
   void DrawColorPicker();


} // end namespace Blocks

#endif // BLOCK_MODAL_POPUPS_H