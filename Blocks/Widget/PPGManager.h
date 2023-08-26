#ifndef PPGMANAGER_H
#define PPGMANAGER_H

#include "UILIb.h"
#include "ch_AppTimer.h"

#include <queue>

using namespace std;
using namespace UILib;

namespace Blocks {

    class PPGManager;

    class PropertyPage {

    public:

        string  Name = "";
        string  Name2 = "";
        string  BackLink = "";
        bool    Pinned = false;
        bool    Collapsed = false;
        bool    ShowActionBar = true; // Indicates whether to draw the top action bar
        bool    ShowDropShadow = true;
        vec2    Pos = {};       // Position in global space
        vec2    Dim = {};       // Width and Height
        vec2    MinDim = {};    // Minimum width and height allowed
        int     Index = -1;     // Index in the PPG vector
        int     RefIndex = -1;  // Reference index
        int     EvalCount = 0;  // Draw frame count

        bool    Mute = false;   // Mutes the mouse click (i.e., you haven't been clicked or a PPG overtop was clicked)

        bool    ExitFlag = false;  // Flag indicates PPG is going to be removed at the end of frame
        vector<int> Children = {}; // Sub-PPG indices
        
        string  Data = "";
        vector<int>    DataInts = {};
        vector<string>  DataList = {};  // used for storing lists of PPG display data, for example a pop-up of information of a row
        vector<float>  DataFloats = {};  // array of ints for storing data, often for heights in expando data PPGs
        vector<void*> DataPtr = {};  // Lets you store pointers to objects you may need. Use reinterpret_cast to retrieve it in the PPG. Objects persist after PPG is destroyed.

        UIForm * Form = nullptr;  // To prevent memory leak, make sure you call DeletePPGForm() for every MakeNewForm().

        std::function<void( PropertyPage & inPPG )> InitFunction = []( PropertyPage & inPPG ){ std::cout << "Got to PPG ( " << inPPG.Name + inPPG.Name2 << " ) InitFunction" << std::endl;};
        std::function<void( PropertyPage & inPPG )> ExitFunction = []( PropertyPage & inPPG ){ std::cout << "Got to PPG ( " << inPPG.Name + inPPG.Name2 << " ) ExitFunction" << std::endl;};
        std::function<void( PropertyPage & inPPG, float inW, float inH )> DrawFunction = []( PropertyPage & inPPG, float inW, float inH ){ std::cout << "Got to PPG ( " << inPPG.Name + inPPG.Name2 << " ) DrawFunction" << std::endl;};
        std::function<int( PropertyPage & inPPG, string inType, vector<string> inMessage )> UpdateFunction = []( PropertyPage & inPPG, string inType, vector<string> inMessage) { return 0; };
        
        PropertyPage(string inName, bool inPinned, vec2 inPos, vec2 inDim){
            Name = inName;
            Pinned = inPinned;
            Pos = inPos;
            Dim = inDim;
        }
        PropertyPage(string inName, bool inPinned, vec2 inDim) {
            Name = inName;
            Pinned = inPinned;
            Dim = inDim;
        };
        PropertyPage() = default;
        ~PropertyPage() = default;

        UIForm & MakeNewForm( int inDir );
        void DeletePPGForm();
        bool FormIsFocused();

        enum ModeTypes {
            mode_float = 0,
            mode_fill_pane,
            mode_top_dock,
            mode_right_dock,
            mode_bottom_dock,
            mode_left_dock,
            mode_top_auto_hide,
            mode_right_auto_hide,
            mode_bottom_auto_hide,
            mode_left_auto_hide
        };
        static bool IsOverlayMode( int inMode );
        static bool IsUnderlayMode( int inMode );

        string ModeRefPane = "";  // Reference PaneName for the PPG mode (if doesn't match any PaneName, then default to mode_float)
        void SetMode( int inMode );
        int  GetMode();
        void UpdateBaseDim();
        void UpdateAutoHide();

    private:

        friend PPGManager;

        // [Internal] Helper Variables and Functions
        enum StatusTypes {
            status_closed = 0,
            status_wait_opening,
            status_opening,
            status_opened,
            status_wait_closing,
            status_closing
        };
        vec2     BaseDim = {};    // [Internal] Based dim used for auto-hide
        AppTimer Timer;           // [Internal] Auto-hide timer
        double   TimerOffset = 0; // [Internal] Auto-hide timer offset
        int      Status = status_opened; // [Internal] Auto-hide status: 0 = closed, 1 = wait opening, 2 = opening, 3 = fully open, 4 = wait closing, 5 = closing'

        int     Mode = mode_float; // PPG draw modes for preset UI behavior
        
    };

    int             GetPpgIndex(string inStr);
    vector<int>     GetPpgIndexes(string inStr);

    class PPGManager{
        
        public:

            bool                CloseUnpinned = false;      // if true, then delete all unpinned PPG in post-frame cleanup
            float               DragArea = 10;              // PPG resize drag area ( radius for corners and distance for edges )
            vector<ImRect>      PPGRects = {};              // List of PPG rects built once at start of frame

            PPGManager();   
            ~PPGManager();

            void                PreRenderSetup();
            void                PostRenderSetup();

            bool                MouseHit(int inPPGIndex);   // Returns true if mouse pos IS in the specified PPG
            bool                AvoidPPG();                 // Returns true if mouse pos IS NOT in any PPG
            bool                IsTop( int inPPGIndex );    // Returns true if specified PPG is on the top
            bool                IsTopNextFrame( int inPPGIndex );    // Returns true if specified PPG is on the top next frame
            
            PropertyPage &      AddPPG( PropertyPage & inPPG );
            PropertyPage &      AddPPG( string inName, bool inPinned, vec2 inPos, vec2 inDim );

            void                RemovePPG( int inPPGIndex );        // Requests for PPG to be removed in post-frame cleanup
            void                RemoveAllPPGs();                    // Requests for all PPG to be removed in post-frame cleanup
            void                RemoveFloatingPPGs();               // Requests for all floating PPGs to be removed in post-frame cleanup
            void                RemoveExistingPPGs();               // Requests for all existing PPGs (at time function is called) to be removed in post-frame cleanup
            void                MovePPGToTop( int inPPGIndex );     // Requests for PPG to be moved to the top in post-frame cleanup
            void                MovePPGToBottom( int inPPGIndex );  // Requests for PPG to be moved to the bottom in post-frame cleanup

            bool                DragFromPPG();          // Returns true if mouse down drag started from PPG
            int                 DragFromPPGIndex();     // Returns the index of the PPG where mouse down drag started from

            void                SetActivePPG( int inPPGIndex = -1, string inAction = "" );
            pair<int, string>   GetActivePPG();
            vec2                GetActivePPGOffset();

            void                CloseUnPinnedPPGs();

            void                SendPPGMessage( string inType, vector<string> inMessage );
            void                QueuePPGMessage( string inType, vector<string> inMessage );
            void                ProcessQueuedMessages();


    private:

            bool                Render = false;

            int                 PPGHit = -1;            // Index of PPG that intersect with mouse pos.
            vector<int>         PPGTop = {};            // Index of PPGs to be moved to the top in post-frame cleanup.
            vector<int>         PPGBot = {};            // Index of PPGs to be moved to the bottom in post-frame cleanup.
            vector<int>         DeleteList = {};        // Indices of PPGs to be deleted in post-frame cleanup.
            vector<int>         OpenList = {};          // Indices of PPGs to be kept open when closing unpinned.  Typically this is the PPG just opened.

            vector<int>         NextDeleteList = {};    // Indices of PPGs to be deleted in the next frame post-frame cleanup -- used when RemovePPG(..) is called prior to frame start.
            bool                DeleteAllPPGs = false;
            bool                DeleteAllFloatingPPGs = false;

            int                 DragFromPPGId = -1;     // Index of PPG where mouse down drag started from ( -1 = not dragging from any PPG )

            int                 ActivePPG = -1;         // Index of PPG currently being manipulated ( this is cleared when PPG is removed or on mouse release )
            string              ActivePPGAction = "";   // Action of the PPG currently being manipulated
            vec2                ActivePPGOffset = {};   // Offset of mouse pos relative to PPG origin when made active

            std::queue<std::pair<string, vector<string>>> MsgQueue = {};

            void                BuildHitRects();
            
            void                CloseDeleteListPPGs();
            void                CloseAllPPGs();

            void                SortTopAndBottom();
            void                ReorderPPGs();          // Moves the PPGHit to the top

            bool                InOpenListList( int inPPGIndex );
            bool                InDeleteList( int inPPGIndex );

            void                RemapLists( vector<int> inMap );
            void                RemapChildren( vector<int> inMap );

        private:

    };

}


#endif /* PPGMANAGER_H */
