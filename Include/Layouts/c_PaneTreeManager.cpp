#ifndef C_PANETREEMANAGER_CPP
#define C_PANETREEMANAGER_CPP

#include "c_PaneTreeManager.h"
#include "c_PipelineManager.h"

namespace AppCore {

    // ---------- PaneTreeManager Class -----------

    PaneTreeManager::PaneTreeManager( PipelineManager &parent ) :
        Parent( parent ),
        AppData( parent.GetAppData() ),
        TypeMap(),
        TUIMap(),
        TUIObjects(), 
        TUIPipeline( parent ),
        PTMap(),
        PTObjects() {
    }

    PaneTreeManager::~PaneTreeManager() {
        for ( int i = 0; i < TUIObjects.size(); ++i ) {
            TUIObjects[i]->~TUI_PaneBase();
        }
        for ( int i = 0; i < PTObjects.size(); ++i ) {
            PTObjects[i]->~PaneTree();
        }
    }

    // Public

    int PaneTreeManager::GetPaneType( string inName ) {
        auto iter = TypeMap.find( inName );
        if ( iter == TypeMap.end() ) {
            return -1;                                  // -1  means empty pane
        }
        return iter->second;
    }

    TUI_PaneBase* PaneTreeManager::GetTUI( string inName ) {
        auto iter = TUIMap.find( inName );
        if ( iter == TUIMap.end() ) {
            // std::cout << "TUI Object not found: " << inName << std::endl;
            // assert( iter != TUIMap.end() );
            return nullptr;
        }
        return TUIObjects[iter->second];
    }

    TUI_PipelineBase & PaneTreeManager::GetTUIPipeline() {
        return TUIPipeline;
    }

    void PaneTreeManager::InitPipelines( size_t resource_count ) {
        TUIPipeline.Init( resource_count );
    }

    void PaneTreeManager::AddPaneTree( string inPTName, bool inPoll, vector<vector<string>> inPTVec ) {
        auto iter = PTMap.find( inPTName );
        if ( iter != PTMap.end() ) {
            std::cout << "PTMap already contains " << iter->first << std::endl;
            assert( iter == PTMap.end() );
        } else {
            PaneTree *PT = new PaneTree( inPTName, inPoll );
            for ( auto & pt : inPTVec ) {
                PT->Add(pt[0], pt[1], pt[2], pt[3], pt[4], pt[5], pt[6], pt[7]);
            }
            PT->Process();
            PTObjects.push_back( PT );
            PTMap.insert( pair<string, int>( inPTName, (int) PTObjects.size() - 1) );
        }
    }

    PaneTree* PaneTreeManager::GetPaneTree( string inPTName ) {
        auto iter = PTMap.find( inPTName );
        if ( iter == PTMap.end() ) {
            std::cout << "PTMap Object not found: " << inPTName << std::endl;
            assert( iter != PTMap.end() );
        }
        return PTObjects[iter->second];
    }

    PaneTree* PaneTreeManager::GetPaneTree( int inIndex ) {
        return PTObjects[inIndex];
    }

    vector<PaneTree*> & PaneTreeManager::GetAllPaneTrees() {
        return PTObjects;
    }

    string PaneTreeManager::FilterByMap(string inString, map<string,string> *inPaneMap){
        auto it = inPaneMap->find(inString);
        if (it != inPaneMap->end()){ return it->second;}
        else {return inString;}   
    }

    vector<TUI_PaneBase*> PaneTreeManager::GetTUIVector( PaneTree *inPT ) {
        vector<TUI_PaneBase*> output;
        for( auto & node : inPT->Nodes ) {
            //string node_name = node.Name;
            string node_name = FilterByMap(node.Name, &PaneOverrideMap);
            if ( GetPaneType( node_name ) == 0 ) {
                output.push_back( GetTUI( node_name ) );
            }
        }
        return output;
    }

    vector<TUI_PaneBase*> PaneTreeManager::GetAllTUIVector() {
        return TUIObjects;
    }

    void PaneTreeManager::GetDimensions(PaneTree *inPT, array<int,2> *inWinSize) {

        inPT->Nodes[0].X = 0;
        inPT->Nodes[0].Y = 0;
        inPT->Nodes[0].W = inWinSize->data()[0];
        inPT->Nodes[0].H = inWinSize->data()[1];

        //cout << "=================================================\n";
        //cout << "   **** top pane: " << inPT->Nodes[0].Name <<  " to:["<< inPT->Nodes[0].X <<", "<< inPT->Nodes[0].Y <<", "<< inPT->Nodes[0].W <<","<<inPT->Nodes[0].H <<"]\n";

        DimensionCrawl(0, inPT);

    }

    void PaneTreeManager::UpdateModes( PaneTree *inPT ) {
         for( auto & node : inPT->Nodes ) {
            //string node_name = node.Name;
            string node_name = FilterByMap(node.Name, &PaneOverrideMap);
            if ( GetPaneType( node_name ) == 0 ) {
                GetTUI( node_name )->UpdateMode( node.Mode);
            }
        }
    }

    void PaneTreeManager::UpdateDimensions( PaneTree *inPT ) {
        for( auto & node : inPT->Nodes ){
            //string node_name = node.Name;
            string node_name = FilterByMap(node.Name, &PaneOverrideMap);
            //cout << "node.Name & filter: " << node.Name << ", " << node_name << "\n";
            if ( GetPaneType( node_name ) == 0 ){
                GetTUI( node_name )->UpdateDimensions( (float) node.X, (float) node.Y, (float) node.W, (float) node.H );
            }
        }
    }

    void PaneTreeManager::DimensionCrawl(int inIndex, PaneTree *inPT){

        PaneNode *f = &inPT->Nodes[inIndex];  // f for frame.  Each PT Tree node is a frame in the app window
        //cout << "*** DimensionCrawl ***: " << f->Name << " ["<<  f->W <<","<< f->H <<"]\n";

        string exp = "";            // length expression on current axis (x or y)
        int fixedSum = 0;           // sum of fixed with dimensions
        int nbExpanding = 0;        // expando frames have a "*" and expand to fill available space
        int percTotal = 0;          // is the total perc of the dimension account for using the % symbol
        int nodeLen;                // length of the current node on the axis it is split on
        if(f->Split == "x"){nodeLen = f->W;}
        else{ nodeLen = f->H;}
        int n = (int)size(f->Children);

        for(int i=0; i<n;i++){
            PaneNode *childF = &inPT->Nodes[ f->Children[i] ];
            f->Split == "x"? exp = childF->WidthExp:exp = childF->HeightExp;
            if(childF->Floating){
                // floating window
                CrawlFloatingPane(f, childF);
                //DimensionCrawl(f->Children[i], inPT);
            }
            else if(exp == "*"){nbExpanding++;}
            else if (exp.find("%") != std::string::npos){
                string expCopy = exp;
                exp.erase(std::remove(exp.begin(), exp.end(), '%'), exp.end());
                double perc = 0;

                if(IsNum(exp)){ childF->Perc = stod(exp) * .01f; }
                else{ cout << "Window Layout, includes invalid percentage string: " << expCopy << "\n"; }

                f->Split == "x" ? fixedSum += (int)round((double)f->W * childF->Perc) : fixedSum += (int)round((double)f->H * childF->Perc);
            }
            else{
                if(IsNum(exp)){ fixedSum += stoi(exp); }
                else{ cout << "Window Layout, includes invalid characters: " << exp << "\n"; }
            }
        }
        int expansionSize = 0;
        if(nbExpanding > 0){ expansionSize =  (int) ( (float)(nodeLen - fixedSum)  /  (float) nbExpanding );}
        int newX = f->X; int newY = f->Y;
        int newW = f->W; int newH = f->H;
        f->Split == "x"? newW = 0: newH = 0;
        for(int i=0; i<n;i++){
            int carryForward = 0;
            PaneNode *childF = &inPT->Nodes[ f->Children[i] ];
            f->Split == "x"? exp = childF->WidthExp:exp = childF->HeightExp;
            if(childF->Floating){}
            else if(exp == "*"){
                f->Split == "x"? newW = expansionSize: newH = expansionSize;
                carryForward = expansionSize;
            }
            else if(childF->Perc != -1 ){
                f->Split == "x"? newW = (int)round((double)f->W * childF->Perc): newH = (int)round((double)f->H * childF->Perc);
                if(f->Split == "x"){
                    newW = (int)round((double)f->W * childF->Perc);
                    carryForward = newW;
                }
                else{
                    newH = (int)round((double)f->H * childF->Perc);
                    carryForward = newH;
                } 
            }
            else {
                carryForward = stoi(exp);
                f->Split == "x"? newW = carryForward : newH = carryForward;
            }
            if(!childF->Floating){
                childF->X = newX; childF->Y = newY;
                childF->W = newW; childF->H = newH;
                //cout << "    set node: " << childF->Name <<  " to:["<< newX <<", "<< newY <<", "<< newW <<","<<newH <<"]\n";
            }
            
            DimensionCrawl(f->Children[i], inPT);
            f->Split == "x" ? newX += carryForward : newY += carryForward;

        }   
    }

    void PaneTreeManager::CrawlFloatingPane(PaneNode* inP, PaneNode * inC){

        // For floating UI, the x y z are in the space of it's parent
        // if the box won't fit within the window it is shifted to fit

        if(inC->FW  == "*"){inC->W = inP->W;}
        else if (inC->FW.find("%") != std::string::npos){inC->W = LengthFromPerc(inC->FW, inP->W);}
        else{inC->W = StringToInt(inC->FW);}
        
        if(inC->FH  == "*"){inC->H = inP->H;}
        else if (inC->FH.find("%") != std::string::npos){inC->H = LengthFromPerc(inC->FH, inP->H);}
        else{inC->H = StringToInt(inC->FH);}

        int xMax =  inP->W - inC->W;
        if(inC->FX  == "*"){inC->X = xMax;}
        else if (inC->FX.find("%") != std::string::npos){inC->X = LengthFromPerc(inC->FX, xMax);}
        else if (inC->FX.find("-") != std::string::npos){inC->X = ReverseNegative(inC->FX, xMax);}
        else{ inC->X = StringToInt(inC->FX);}
        if(inC->X > xMax){inC->X = xMax;}
        inC->X += inP->X;

        int yMax =  inP->H - inC->H;
        if(inC->FY  == "*"){inC->Y = yMax;}
        else if (inC->FY.find("%") != std::string::npos){inC->Y = LengthFromPerc(inC->FY, yMax);}
        else if (inC->FY.find("-") != std::string::npos){inC->Y = ReverseNegative(inC->FY, yMax);}
        else{ inC->Y = StringToInt(inC->FY);}
        if(inC->Y > yMax){inC->Y = yMax;}
        inC->Y += inP->Y;
        
        //cout << "    floating: " << inC->Name <<  " to:["<< inC->X <<", "<< inC->Y <<", "<< inC->W <<","<<inC->H <<"]\n";

    }
    bool PaneTreeManager::IsNum(const std::string& s){
        std::istringstream iss(s);
        float f;
        iss >> noskipws >> f; // noskipws considers leading whitespace invalid
        return iss.eof() && !iss.fail(); 
    }
    int PaneTreeManager::LengthFromPerc(string inPercExp, int inLength){
        double perc = 0;
        string numOnly = inPercExp;
        numOnly.erase(std::remove(numOnly.begin(), numOnly.end(), '%'), numOnly.end());
        bool NumValid = IsNum(numOnly);
        if(!NumValid){cout << "Assert: Window Layout, includes invalid percentage string: " << inPercExp << "\n";}
        assert(NumValid);
            perc = stod(numOnly) * .01f;
            int pNum =  (int)round((double)inLength * perc);
            return pNum;

        return 0;
    }

    int PaneTreeManager::StringToInt(string inStr){
        if(IsNum(inStr)){ return stoi(inStr); }
        cout << "Window Layout, includes invalid characters: " << inStr << "\n";
        return 0;
    }

    int PaneTreeManager::ReverseNegative(string inCord, int inRightSideX){
        // if there is a negative number on a xy coordinate then set it's value from the rigth edge moving inward
        string numOnly = inCord;
        numOnly.erase(std::remove(numOnly.begin(), numOnly.end(), '-'), numOnly.end());
        bool NumValid = IsNum(numOnly);
        if(!NumValid){cout << "Assert: Window Layout, includes invalid percentage string: " << inCord << "\n";}
        assert(NumValid);
            int intCord = stoi(numOnly);
            int reverseNum =  (int)inRightSideX - intCord;
            return reverseNum;
        return 0;
    }

    ChartSheet * PaneTreeManager::GetChartSheet( const string & inPaneName ) {
        for ( auto * tui : TUIObjects ) {
            if ( tui->PaneType == "ChartSheet" && tui->PaneName == inPaneName ) {
                return reinterpret_cast<ChartSheet*>( tui->GetSelf() );
            }
        }
        return nullptr;
    }

} // end namespace AppCore

#endif // C_PANETREEMANAGER_CPP