#ifndef BLOCK_MODAL_POPUPS_CPP
#define BLOCK_MODAL_POPUPS_CPP

#include "c_PipelineManager.h"
using namespace AppCore;

#if defined( _WIN32 )
    typedef int(__stdcall *MYPROC)(HDC hd, int x, int y);
#endif

namespace Blocks {

    // --------------------------------------------------------------------
    using unique_void_ptr = std::unique_ptr<void, void(*)(void const*)>;
    template<typename T>
    auto unique_void(T * ptr) -> unique_void_ptr
    {   
        return unique_void_ptr(ptr, [](void const * data) {
            T const * p = static_cast<T const*>(data);            
            delete p;
        });
    };
    // --------------------------------------------------------------------

    static std::unique_ptr<UIForm> ModalForm;                                               // Static form used by modal pop-ups
    static unique_void_ptr ModalData = unique_void_ptr(nullptr, [](void const * data) {});  // Static data struct - Use unique_void(...) instead of std::make_unique().
    
   
    void DrawModalPopUps(float inW, float inH){

        if( !ADIO->FM.AnyClickOK( fwt_modal_ui ) ) { return; }

        if( ADIO->FM.HasFocus( ft_context_menu ) ){
            DrawContextMenu(UIGetContextMenu()->Options, UIGetContextMenu()->Name, inW, inH);
        }
        else if( ADIO->FM.HasFocus( ft_cursor_color_picker ) ) {
            DrawColorPicker();
        }
    
    }

    void DrawContextMenu(vector<string> inOptions, string inType, float inW, float inH){
        // *** NOTE *** 
        // Context Menu focus is cleared by FocusManager::PreRenderSetup at the start of frame.

        // Context Dimensions
        float cellH = 22;float spacerH = 10;float sideSpacer = 20;float mH = 0;float mW = 0;

        // First determine height and width
        for(string op:inOptions){
            if(op[0] == '-'){mH += spacerH;}else{mH += cellH;}
            float tWidth = UIGetTextWidth(op, 0);
            if(tWidth > mW){mW = tWidth;}
        }
        mW += sideSpacer * 2;
        
        // Figure out location based on mouse position and window boundaries
        static vec2 menuP;
        vec2 mPos = GetGlobalMousePos();

        if ( ADIO->FM.GetFocusFrameCount() == 0 ) { menuP = {mPos.x + 15, mPos.y}; }
        ImRect testBox = GetGlobalBB(menuP.x,menuP.y, mW, mH);
        ImRect container = GetGlobalBB(0,0,inW, inH);
        container.Expand(-30);
        ImRect fitRect = FitBoxinBB(testBox, container);
        menuP = {fitRect.Min.x, fitRect.Min.y};

        ImRect menuBB = ADIO->CurrentBB = GetGlobalBB(menuP.x,menuP.y, mW, mH);

        float dropAlpha = .16f;
        UIDropShadow(menuBB, 12, dropAlpha, 15, 15, 0x222222, 2, 2);
        UIDrawBB(menuBB, 0xFFFFFF);
        UIDrawBBOutline(menuBB, 0xAAAAAA, 1);
        float yPos = menuP.y;ImRect mBox;
        for(int i=0;i<inOptions.size();i++){
            if(inOptions[i][0] == '-'){
                UIAddLine(menuP.x, yPos+spacerH*.5f, menuP.x + mW, yPos+spacerH*.5f, 0xD3D3D3, 1);
                yPos += spacerH;
            }
            else{
                 mBox = GetGlobalBB(menuP.x+1, yPos, mW-2, cellH);
                 if( VecInBB(mPos, mBox) ){
                    UIDrawBB(mBox, 0XE6E8E7);
                    if(DragStart()){
                        ProcessContextClick(inType, inOptions[i]);
                        ADIO->FM.RevertToPane( ft_context_menu );
                        return;
                    }
                 }
                UIAddTextWithFont(menuP.x + sideSpacer, yPos, mW-sideSpacer, cellH, inOptions[i], 0x000000, 0, {0,.5} );
                yPos += cellH;
            }
        }

    }

    void DrawColorPicker(){

    #if defined( _WIN32 )

        vec2 mPos = GetGlobalMousePos();
        
        if( DragStart() && ADIO->FM.GetFocusFrameCount() > 0 ) {
            HINSTANCE _hGDI = LoadLibrary("gdi32.dll");
            MYPROC ProcAdd;
            ProcAdd = (MYPROC)GetProcAddress(_hGDI, "GetPixel");
            POINT _cursor;
            GetCursorPos(&_cursor);
            HDC _hdc = GetDC(NULL);
            if(_hdc){
                COLORREF _color = (*ProcAdd) (_hdc,  (int)_cursor.x, (int)_cursor.y);
                int rv = GetRValue(_color);
                int gv = GetGValue(_color);
                int bv = GetBValue(_color);
                int clr =  ((rv&0x0ff)<<16)|((gv&0x0ff)<<8)|(bv&0x0ff);
                string hexStr = IntToHexColor(clr);
                
                if ( ADIO->ColorPickerSource == "PPG" ) {
                    PropertyPage & inPPG = *reinterpret_cast<PropertyPage*>(ADIO->ColorPickerData);
                    if(inPPG.Name == "Select Color"){
                        // std::cout << "Color Picker from: " << inPPG.Name + inPPG.Name2 << std::endl;
                        unsigned int * currentColor = reinterpret_cast< unsigned int*>( inPPG.DataPtr[0] );
                        SelectColorPPGData * sd = (SelectColorPPGData *) inPPG.DataPtr[1];
                        *currentColor = clr;
                        sd->ColorHex = IntToHexColor( clr );
                        sd->HasChange = 2;
                    }
                }
                

            }
            FreeLibrary(_hGDI);
            ADIO->ColorPickerSource = "";
            ADIO->ColorPickerData = nullptr;
            ADIO->FM.RevertFocus( ft_cursor_color_picker );

        }
        
    #elif defined( __linux__ )

        // TO DO: Add LINUX-specific code here

    #endif

    }

} // end namespace Blocks

#endif // BLOCK_MODAL_POPUPS_CPP