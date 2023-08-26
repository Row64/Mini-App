#ifndef C_PANETREE_H
#define C_PANETREE_H

#include "c_PaneNode.h"

#include <map> 
using namespace std;

namespace AppCore {

    class PaneTree {

    public:
        
        string Name;
        vector<PaneNode> Nodes;
        map<string,int> Lookup;
        int count = 0;
        bool Poll = false;
        
        PaneTree() : Name() {}
        PaneTree( string inName, bool inPoll ) : Name( inName ), Poll( inPoll ) {}
        ~PaneTree() {}
        
        //            ==Name=============ParentName======WidthExp=====    HeightExp==             =Split======    =Mode=  ===FloatX=   =FloatY=== ==SUI/TUI Type   ===ObjectHandle
        void Add(string inName, string inParentName, string inWidthExp, string inHeightExp, string inSplit, string inMode, string fX, string fY) {

            int nSize = (int)size(Nodes);
            //if(inIndex < nSize){cout << "ERROR: PaneTree.Add - Layout Description Error, Index: " << inIndex << " is overwriting another pane" <<"\n";return;}
            //if(inIndex > nSize){cout << "ERROR: PaneTree.Add - Layout Description Error, Index: " << inIndex << " is skipping indicies, larger than it should be" <<"\n";return;}
            Nodes.push_back( PaneNode(inName, inParentName, inWidthExp, inHeightExp, inSplit, inMode, fX, fY) );
            count++;
        }

        void Process() {

            int nbNodes = (int)size(Nodes);
            for(int i=0;i<nbNodes;i++){
                Lookup.insert( pair<string, int>(Nodes[i].Name, i) ); 
            }

            // Fill parents
            for(int i=0;i<nbNodes;i++){
                
                auto iter = Lookup.find(Nodes[i].ParentName);
                if (iter == Lookup.end()) {
                    if(i==0){
                        //cout << "Setting Pane: " << Nodes[i].Name << " to root node" << "\n";
                    }
                    else{
                        //cout << "Did not find Parent Node for: " << Nodes[i].ParentName << ", setting to root node " << "\n";
                    }
                    if( i > 0){
                        Nodes[i].Parent = 0;
                    }
                }
                else {
                    int parentIndex = iter->second;
                    //cout << "map " << i << " to parent: " << parentIndex << "\n";
                    Nodes[i].Parent = parentIndex;
                } 
                

            }
            // Next fill Children
            for(int i=0;i<nbNodes;i++){
                int parent = Nodes[i].Parent;
                if(parent != -1){
                    Nodes[parent].Children.push_back(i);
                }  

            }
    
        }
        
    };

} // end namespace AppCore

#endif /* C_PANETREE_H */
