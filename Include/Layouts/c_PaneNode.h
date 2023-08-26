#ifndef C_PANENODE_H
#define C_PANENODE_H

#include <iostream>
#include <string>
#include <vector>
#include <array>
using namespace std;

namespace AppCore {
        
    class PaneNode {

    public:

        string Name = "";
        int Parent = -1;
        
        string WidthExp = "";   // width expression, options: number in pixels, * for all, 50% for half of avaiable width
        string HeightExp = "";  // height expression, options: number in pixels, * for all, 50% for half of avaiable height
        string Split = "";      // "x" for split in x, "y" for split in y
        string Mode = "";       // string of meta-data about window characteristics

        double Perc = -1;       // if the pane has W or H set by percentage filling space, this will hold the perc as a decimal ( ex: .25 for 25%)
        vector<int> Children = {};
        int X = 0;
        int Y = 0;
        int W = 0;
        int H = 0;
        bool Floating = false;
        
        string FX = "";         //  x pos for floating window (optional - if set becomes floating)
        string FY = "";         // y pos for floating window (optional - if set becomes floating)
        string FW = "";         // width for floating window (optional - if set becomes floating)
        string FH = "";         // height for floating window (optional - if set becomes floating)

        string ParentName = "";

        PaneNode(string inName, string inParentName, string inWidthExp, string inHeightExp, string inSplit, string inMode, string inFX, string inFY) {

            Name = inName;
            WidthExp = inWidthExp;
            HeightExp = inHeightExp;
            Split = inSplit;
            Mode = inMode;
            FX = inFX;
            FY = inFY;
            ParentName = inParentName;
            
            if(inFX!="" || inFY!=""){
                Floating = true;
                FW = inWidthExp;
                FH = inHeightExp;
            }

        }

        
    };

} // end namespace AppCore

#endif /* C_PANENODE_H */
