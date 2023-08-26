#ifndef C_TUI_PIPELINEBASE_H
#define C_TUI_PIPELINEBASE_H

#include "c_DXBase.h"
#include "ch_Texture.h"
#include "TUI_PaneBase.h"

using namespace std;

namespace AppCore {

    class PipelineManager;

    // ************************************************************ //
    // TUI_PipelineBase Class                                       //
    // ************************************************************ //
    class TUI_PipelineBase {

    public:

        TUI_PipelineBase( PipelineManager &parent );
        virtual ~TUI_PipelineBase();


        void                    Init( size_t resource_count );
        void                    ReInit();

        void                    Render( ID3D11DeviceContext* ctx, vector<TUI_PaneBase*> inPaneList  );

        void                    OnWindowSizeChanged_Pre();
        void                    OnWindowSizeChanged_Post();

        bool                    ForceTextureUpdate = false;

    protected:
        
        PipelineManager &       Parent;
        AppDataIO &             AppData;
        UILibContext*           Context;
       
        std::vector<ch_Texture>                     Textures;                     // DX11 pipeline also uses this
        uint32_t                                    MaxTextureCount = 100;        // This is updated by TUI_PipelineBase::Init(), based on device limits.
        const char*                                 TUIButtonBarTexturePath = "Data/BaseSprites_01.png";

        // Init methods
        void                    ResizeResources( size_t resource_count );
        void                    InitPanes();
       
        // Render methods
        void                    CompareTextures();
        void                    UpdateTextures();
        void                    DrawAllPanes( vector<TUI_PaneBase*> inPaneList );
      

        // ------ DX11 Variables & Methods -----

        ID3D11Device*                       g_pd3dDevice = NULL;
        ID3D11DeviceContext*                g_pd3dDeviceContext = NULL;
        ID3D11RenderTargetView*             g_mainRenderTargetView = NULL;
        ID3D11Buffer*                       g_pVB = NULL;
        ID3D11Buffer*                       g_pIB = NULL;
        ID3D10Blob*                         g_pVertexShaderBlob = NULL;
        ID3D11VertexShader*                 g_pVertexShader = NULL;
        ID3D11InputLayout*                  g_pInputLayout = NULL;
        ID3D11Buffer*                       g_pVertexConstantBuffer = NULL;
        ID3D10Blob*                         g_pPixelShaderBlob = NULL;
        ID3D11PixelShader*                  g_pPixelShader = NULL;
        vector<ID3D11SamplerState*>         g_pSamplers = {};
        vector<ID3D11ShaderResourceView*>   g_pTextureViews = {};
        ID3D11RasterizerState*              g_pRasterizerState = NULL;
        ID3D11BlendState*                   g_pBlendState = NULL;
        ID3D11DepthStencilState*            g_pDepthStencilState = NULL;
        int                                 g_VertexBufferSize = 5000, g_IndexBufferSize = 10000;

        bool                                DeviceObjValid = false;
        int                                 LastSRVSize = 0;
        int                                 LastSamplerSize = 0;

        // Init methods
        void                    DXCreateTexture();
        void                    DXLoadResidentTextures();
        void                    DXCreateDeviceObjects();
        void                    DXCreatePixelShader();

        // Render methods
        void                    DXDrawFrameData( ID3D11DeviceContext* ctx );
        void                    DXSetupRenderState( ImDrawData* draw_data, ID3D11DeviceContext* ctx );

        // Cleanup methods
        void                    DXInvalidateDeviceObjects();
        void                    DXInvalidateTexturesObjects();
        void                    DXInvalidateNonResidentTextures();
    

    };

} // end namespace AppCore

#endif // C_TUI_PIPELINEBASE_H