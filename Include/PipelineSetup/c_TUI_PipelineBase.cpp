#ifndef C_TUI_PIPELINEBASE_CPP
#define C_TUI_PIPELINEBASE_CPP

#include "c_PipelineManager.h"

#include <d3dcompiler.h>
#ifdef _MSC_VER
#pragma comment(lib, "d3dcompiler") // Automatically link with d3dcompiler.lib as we are using D3DCompile() below.
#endif


namespace AppCore {

    TUI_PipelineBase::TUI_PipelineBase( PipelineManager &parent ) :
        Parent( parent ),
        AppData( parent.GetAppData() ),
        Context( UICreateContext() ) {
    }

    TUI_PipelineBase::~TUI_PipelineBase() {
        if ( Context ) {
            UIDestroyContext();
        }
    }

    // Public Methods

    void TUI_PipelineBase::Init( size_t resource_count ) {
        DXLoadResidentTextures();
        InitPanes();
        OnWindowSizeChanged_Post();
    }

    void TUI_PipelineBase::ReInit() {
        // ----- DX11 Initialization -----
        IDXGIDevice* pDXGIDevice = NULL;
        IDXGIAdapter* pDXGIAdapter = NULL;
        IDXGIFactory* pFactory = NULL;

        auto device = Parent.GetDXDevice();
        auto device_context = Parent.GetDXDeviceContext();
        auto target_view = Parent.GetDXRenderTargetView();

        if (device->QueryInterface(IID_PPV_ARGS(&pDXGIDevice)) == S_OK)
            if (pDXGIDevice->GetParent(IID_PPV_ARGS(&pDXGIAdapter)) == S_OK)
                if (pDXGIAdapter->GetParent(IID_PPV_ARGS(&pFactory)) == S_OK)
                {
                    g_pd3dDevice = device;
                    g_pd3dDeviceContext = device_context;
                    g_mainRenderTargetView = target_view;
                    g_pd3dDevice->AddRef();
                    g_pd3dDeviceContext->AddRef();
                }
        if (pDXGIDevice) pDXGIDevice->Release();
        if (pDXGIAdapter) pDXGIAdapter->Release();
        
        if (!DeviceObjValid) { DXCreateDeviceObjects(); }
    }
   
    void TUI_PipelineBase::Render( ID3D11DeviceContext* ctx, vector<TUI_PaneBase*> inPaneList  ) {
        if (!DeviceObjValid) { DXCreateDeviceObjects(); }
        DrawAllPanes( inPaneList );
        CompareTextures();
        DXDrawFrameData( ctx );
    }

    void TUI_PipelineBase::OnWindowSizeChanged_Pre(){
    }
    
    void TUI_PipelineBase::OnWindowSizeChanged_Post() {
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize.x = static_cast<float>(Parent.GetDisplayState().FramebufferSize.W);
        io.DisplaySize.y = static_cast<float>(Parent.GetDisplayState().FramebufferSize.H);
        // Window resizing in DX11 recreates the render target view, so we need to reassign
        // the pointer after DXBase::ResizeSwapChain() is called. Otherwise, we can get an error
        // in DXDrawFrameData() stating that g_mainRenderTargetView is corrupt or NULL.
        g_mainRenderTargetView = Parent.GetDXRenderTargetView();
    }

    void TUI_PipelineBase::InitPanes() {
        // You only need to set this ONCE somewhere in the initialization process
        UILib::UISetInputStatePointer( &Parent.GetInputState() );                               // This lets UILib internal functions to access mouse state with *UILibIS
        UILib::UISetDisplayStatePointer( &Parent.GetDisplayState() );                           // This lets UILib internal functions to access display state with *UILibDS

    }

    void TUI_PipelineBase::CompareTextures() {
        // Check whether we need to update Textures

        if(ForceTextureUpdate){
            cout << "*************** FORCE TEXTURE UPDATE *******************" << "\n";
            ForceTextureUpdate = false;
            UpdateTextures();
            return;
        }

        // If ImGui changed textures since the last frame, update the Textures list now.
        // - The ImGui list (IO.ImagePaths) is cleared at end of each frame.
        // - Each ImGui pane drawn could add images to the list.
        // - Since the ImGui list could "grow" for each pane back up to the size of the last frame, we don't want to call update if
        //   it is just growing back to what it was in the last frame.
        // - The Textures list persists across frames and is changed only if ImGui added new textures or changed the texture list
        //   since the last frame.
        // - For steady state (simple redraw of app with no changes), the ImGui texture list will be the same for each frame and we
        //   won't have to update the textures.
        // - If ImGui added a new texture, the first path is taken for the update
        // - If ImGui changed a texture, the else path detects that and does the update
        // - If ImGui removed a texture, other than the last texture in the list, then the "changed" texture path is taken for the
        //   update since the other textures would shift down to take the place of the removed texture.
        // - If ImGui removed the last texture, then this case won't really be detected here and the texture update won't get
        //   called.  This is probably pretty harmless since it would just leave an unused texture in the Textures list.  This
        //   problem would "fix itself" if ImGui makes a change in the texture list that would cause an update.
        if ( Context->IO.ImagePaths.size() > Textures.size() ) {
            UpdateTextures();
            return;
        } else {
            for ( int i = 0; i < Context->IO.ImagePaths.size(); i++ ) {               // Otherwise, compare each ch_Texture and update if there are any differences
                std::string tPath = ( ( (fs::path) Textures[i].path ).make_preferred() ).string();
                if ( Context->IO.ImagePaths[i] != tPath ) {
                    UpdateTextures();
                    return;
                }
            }
        }
    }

    // This function and CreateTexture() should probably be optimized for the case when the only change to the texture list is
    // adding new textures.
    void TUI_PipelineBase::UpdateTextures() {
        size_t oldSize = Textures.size();
        Textures.clear();
        for ( std::string uPath : Context->IO.ImagePaths ) {
            Textures.push_back( { uPath } );
        }
        DXCreateTexture();
        // The pixel shader must be recreated when the number of textures changes.
        if ( Textures.size() != oldSize ) { DXCreatePixelShader(); }
    }

    void TUI_PipelineBase::DrawAllPanes( vector<TUI_PaneBase*> inPaneList ) {
        Parent.CanRender = false;  // blocks call to RenderPipelines() if in the middle of drawing panes
        ImGui::NewFrame();
        if ( inPaneList.size() > 0 ) {
            for ( int i = 0; i < inPaneList.size(); ++i ) {
                inPaneList[i]->DrawPane();
            }
        }
        Parent.CanRender = true;
    }


    // ============================================================================================ //
    //                                         DX11 Methods                                         //
    // ============================================================================================ //

    struct VERTEX_CONSTANT_BUFFER {
        float   mvp[4][4];
    };

    void TUI_PipelineBase::DXCreateTexture() {

        DXInvalidateNonResidentTextures();

        for ( auto &tex : Textures ) {
            if ( g_pTextureViews.size() >= MaxTextureCount ) {  // Avoid going past device limits
                break;
            }
            int width = 0, height = 0, data_size = 0;
            CharBuffer texdata( tex.GetImageData(4, &width, &height, nullptr, &data_size), data_size);
            g_pTextureViews.push_back( NULL );
            if ( !Parent.CreateDXImage( (unsigned char * ) texdata.Content, width, height, &g_pTextureViews.back() ) ) {
                std::cout << "Failed to CreateDXImage     path: " << tex.path << std::endl;
            }
            g_pSamplers.push_back( NULL );
            Parent.CreateDXSampler( &g_pSamplers.back() );
        }

    }

    void TUI_PipelineBase::DXLoadResidentTextures() {
        DXInvalidateTexturesObjects();
        UILibCTX->IO.NumberOfResidentTextures = 0;
        {
            // Load Font Texture at position 0
            // Build Font texture atlas
            ImGuiIO& io = ImGui::GetIO();
            unsigned char* pixels;
            int width, height;
            io.Fonts->GetTexDataAsRGBA32( &pixels, &width, &height );

            g_pTextureViews.push_back( NULL );
            Parent.CreateDXImage( pixels, width, height, &g_pTextureViews.back() );
            g_pSamplers.push_back( NULL );
            Parent.CreateDXSampler( &g_pSamplers.back() );

            UILibCTX->IO.NumberOfResidentTextures += 1;
        }
        {
            // Load ButtonBar Texture at position 1
            fs::path tPath = ADIO->RootPath / fs::path( TUIButtonBarTexturePath );
            ch_Texture tex;
            tex.path = tPath.string();
            int width = 0, height = 0, data_size = 0;
            CharBuffer texdata( tex.GetImageData( 4, &width, &height, nullptr, &data_size ), data_size );

            // Font Texture if position 0
            g_pTextureViews.push_back( NULL );
            Parent.CreateDXImage( (unsigned char*)texdata.Content, width, height, &g_pTextureViews.back() );
            g_pSamplers.push_back( NULL );
            Parent.CreateDXSampler( &g_pSamplers.back() );

            ADIO->ButtonBarIndex = 1;
            UILibCTX->IO.NumberOfResidentTextures += 1;
        }
    }

    void TUI_PipelineBase::DXDrawFrameData( ID3D11DeviceContext* ctx ) {
        
        UIRender();     // Calls ImGui::Render() and clears the image paths list

        ImDrawData* draw_data = ImGui::GetDrawData();

        if( draw_data->TotalVtxCount == 0 ) {
            return;
        }
        if ( !g_mainRenderTargetView ) {
            std::cout << "No render target view!" << std::endl;
            return;
        }

        ImVec4 clear_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);                  // std::cout << "TUI_PipelineBase::DXDrawFrameData     AAA 1" << std::endl;
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, (float*)&clear_color);

        // Avoid rendering when minimized
        if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f) {
            return;
        }

        // Create and grow vertex/index buffers if needed
        if (!g_pVB || g_VertexBufferSize < draw_data->TotalVtxCount)
        {
            if (g_pVB) { g_pVB->Release(); g_pVB = NULL; }
            g_VertexBufferSize = draw_data->TotalVtxCount + 5000;
            D3D11_BUFFER_DESC desc;
            memset(&desc, 0, sizeof(D3D11_BUFFER_DESC));
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.ByteWidth = g_VertexBufferSize * sizeof(ImDrawVert);
            desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            desc.MiscFlags = 0;
            if (g_pd3dDevice->CreateBuffer(&desc, NULL, &g_pVB) < 0) {
                return;
            }
        }

        if (!g_pIB || g_IndexBufferSize < draw_data->TotalIdxCount)
        {
            if (g_pIB) { g_pIB->Release(); g_pIB = NULL; }
            g_IndexBufferSize = draw_data->TotalIdxCount + 10000;
            D3D11_BUFFER_DESC desc;
            memset(&desc, 0, sizeof(D3D11_BUFFER_DESC));
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.ByteWidth = g_IndexBufferSize * sizeof(ImDrawIdx);
            desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            if (g_pd3dDevice->CreateBuffer(&desc, NULL, &g_pIB) < 0) {
                return;
            }
        }

        // Upload vertex/index data into a single contiguous GPU buffer
        D3D11_MAPPED_SUBRESOURCE vtx_resource, idx_resource;
        if (ctx->Map(g_pVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &vtx_resource) != S_OK) {
            return;
        }
        if (ctx->Map(g_pIB, 0, D3D11_MAP_WRITE_DISCARD, 0, &idx_resource) != S_OK) {
            return;
        }
        ImDrawVert* vtx_dst = (ImDrawVert*)vtx_resource.pData;
        ImDrawIdx* idx_dst = (ImDrawIdx*)idx_resource.pData;
        for (int n = 0; n < draw_data->CmdListsCount; n++)
        {
            const ImDrawList* cmd_list = draw_data->CmdLists[n];
            memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
            memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
            vtx_dst += cmd_list->VtxBuffer.Size;
            idx_dst += cmd_list->IdxBuffer.Size;
        }
        ctx->Unmap(g_pVB, 0);
        ctx->Unmap(g_pIB, 0);

        // Setup orthographic projection matrix into our constant buffer
        // Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
        {
            D3D11_MAPPED_SUBRESOURCE mapped_resource;
            if (ctx->Map(g_pVertexConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource) != S_OK) {
                return;
            }
            VERTEX_CONSTANT_BUFFER* constant_buffer = (VERTEX_CONSTANT_BUFFER*)mapped_resource.pData;
            float L = draw_data->DisplayPos.x;
            float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
            float T = draw_data->DisplayPos.y;
            float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
            float mvp[4][4] =
            {
                { 2.0f/(R-L),   0.0f,           0.0f,       0.0f },
                { 0.0f,         2.0f/(T-B),     0.0f,       0.0f },
                { 0.0f,         0.0f,           0.5f,       0.0f },
                { (R+L)/(L-R),  (T+B)/(B-T),    0.5f,       1.0f },
            };
            memcpy(&constant_buffer->mvp, mvp, sizeof(mvp));
            ctx->Unmap(g_pVertexConstantBuffer, 0);
        }

        // Backup DX state that will be modified to restore it afterwards (unfortunately this is very ugly looking and verbose. Close your eyes!)
        // NOTE: AppCore namespace include enum datatypes that conflict with UINT and FLOAT, so we need to remove the namespace here.
        struct BACKUP_DX11_STATE
        {
            ::UINT                      ScissorRectsCount, ViewportsCount;
            D3D11_RECT                  ScissorRects[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
            D3D11_VIEWPORT              Viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
            ID3D11RasterizerState*      RS;
            ID3D11BlendState*           BlendState;
            ::FLOAT                     BlendFactor[4];
            ::UINT                      SampleMask;
            ::UINT                      StencilRef;
            ID3D11DepthStencilState*    DepthStencilState;
            vector<ID3D11ShaderResourceView*>   PSShaderResource;
            vector<ID3D11SamplerState*>         PSSampler;
            ID3D11PixelShader*          PS;
            ID3D11VertexShader*         VS;
            ID3D11GeometryShader*       GS;
            ::UINT                      PSInstancesCount, VSInstancesCount, GSInstancesCount;
            ID3D11ClassInstance         *PSInstances[256], *VSInstances[256], *GSInstances[256];   // 256 is max according to PSSetShader documentation
            D3D11_PRIMITIVE_TOPOLOGY    PrimitiveTopology;
            ID3D11Buffer*               IndexBuffer, *VertexBuffer, *VSConstantBuffer;
            ::UINT                      IndexBufferOffset, VertexBufferStride, VertexBufferOffset;
            DXGI_FORMAT                 IndexBufferFormat;
            ID3D11InputLayout*          InputLayout;
        };
        
        BACKUP_DX11_STATE old = {};
        old.ScissorRectsCount = old.ViewportsCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
        ctx->RSGetScissorRects(&old.ScissorRectsCount, old.ScissorRects);
        ctx->RSGetViewports(&old.ViewportsCount, old.Viewports);
        ctx->RSGetState(&old.RS);
        ctx->OMGetBlendState(&old.BlendState, old.BlendFactor, &old.SampleMask);
        ctx->OMGetDepthStencilState(&old.DepthStencilState, &old.StencilRef);

        if ( LastSRVSize > 0 ) {
            old.PSShaderResource.resize( LastSRVSize, NULL );
            ctx->PSGetShaderResources(0, LastSRVSize, &old.PSShaderResource[0]);
        }
        if ( LastSamplerSize > 0 ) {
            old.PSSampler.resize( LastSamplerSize, NULL );
            ctx->PSGetSamplers(0, LastSamplerSize, &old.PSSampler[0]);
        }

        old.PSInstancesCount = old.VSInstancesCount = old.GSInstancesCount = 256;
        ctx->PSGetShader(&old.PS, old.PSInstances, &old.PSInstancesCount);
        ctx->VSGetShader(&old.VS, old.VSInstances, &old.VSInstancesCount);
        ctx->VSGetConstantBuffers(0, 1, &old.VSConstantBuffer);
        ctx->GSGetShader(&old.GS, old.GSInstances, &old.GSInstancesCount);

        ctx->IAGetPrimitiveTopology(&old.PrimitiveTopology);
        ctx->IAGetIndexBuffer(&old.IndexBuffer, &old.IndexBufferFormat, &old.IndexBufferOffset);
        ctx->IAGetVertexBuffers(0, 1, &old.VertexBuffer, &old.VertexBufferStride, &old.VertexBufferOffset);
        ctx->IAGetInputLayout(&old.InputLayout);

        // Setup desired DX state
        DXSetupRenderState(draw_data, ctx);

        // Bind Texture Resources
        if ( !g_pTextureViews.empty() ) {
            ctx->PSSetShaderResources(0, (int) g_pTextureViews.size(), &g_pTextureViews[0]);
            LastSRVSize = (int) g_pTextureViews.size();
        }
        if ( !g_pSamplers.empty() ) {
            ctx->PSSetSamplers( 0, (int) g_pSamplers.size(), &g_pSamplers[0] );
            LastSamplerSize = (int) g_pSamplers.size();
        }

        // Render command lists
        // (Because we merged all buffers into a single one, we maintain our own offset into them)
        int global_idx_offset = 0;
        int global_vtx_offset = 0;
        ImVec2 clip_off = draw_data->DisplayPos;
        for (int n = 0; n < draw_data->CmdListsCount; n++)
        {
            const ImDrawList* cmd_list = draw_data->CmdLists[n];
            for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
            {
                const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
                if (pcmd->UserCallback != NULL)
                {
                    // User callback, registered via ImDrawList::AddCallback()
                    // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                    if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                        DXSetupRenderState(draw_data, ctx);
                    else
                        pcmd->UserCallback(cmd_list, pcmd);
                }
                else
                {
                    // Apply scissor/clipping rectangle
                    const D3D11_RECT r = { (LONG)(pcmd->ClipRect.x - clip_off.x), (LONG)(pcmd->ClipRect.y - clip_off.y), (LONG)(pcmd->ClipRect.z - clip_off.x), (LONG)(pcmd->ClipRect.w - clip_off.y) };
                    ctx->RSSetScissorRects(1, &r);
                    // Draw indexed
                    ctx->DrawIndexed(pcmd->ElemCount, pcmd->IdxOffset + global_idx_offset, pcmd->VtxOffset + global_vtx_offset);
                }
            }
            global_idx_offset += cmd_list->IdxBuffer.Size;
            global_vtx_offset += cmd_list->VtxBuffer.Size;
        }

        // Restore modified DX state
        ctx->RSSetScissorRects(old.ScissorRectsCount, old.ScissorRects);
        ctx->RSSetViewports(old.ViewportsCount, old.Viewports);
        ctx->RSSetState(old.RS);                                                                            SAFE_RELEASE(old.RS)
        ctx->OMSetBlendState(old.BlendState, old.BlendFactor, old.SampleMask);                              SAFE_RELEASE(old.BlendState)
        ctx->OMSetDepthStencilState(old.DepthStencilState, old.StencilRef);                                 SAFE_RELEASE(old.DepthStencilState)
        if ( old.PSShaderResource.size() > 0 ) {
            ctx->PSSetShaderResources(0, (int) old.PSShaderResource.size(), &old.PSShaderResource[0]);      for ( auto p : old.PSShaderResource ) { SAFE_RELEASE(p); }
        }
        if ( old.PSSampler.size() > 0 ) {
            ctx->PSSetSamplers(0, (int) old.PSSampler.size(), &old.PSSampler[0]);                           for ( auto p : old.PSSampler ) { SAFE_RELEASE(p); }
        }
        ctx->PSSetShader(old.PS, old.PSInstances, old.PSInstancesCount);                                    SAFE_RELEASE(old.PS) for (::UINT i = 0; i < old.PSInstancesCount; i++) { SAFE_RELEASE(old.PSInstances[i]) }
        ctx->VSSetShader(old.VS, old.VSInstances, old.VSInstancesCount);                                    SAFE_RELEASE(old.VS)
        ctx->VSSetConstantBuffers(0, 1, &old.VSConstantBuffer);                                             SAFE_RELEASE(old.VSConstantBuffer)
        ctx->GSSetShader(old.GS, old.GSInstances, old.GSInstancesCount);                                    SAFE_RELEASE(old.GS) for (::UINT i = 0; i < old.VSInstancesCount; i++) { SAFE_RELEASE(old.VSInstances[i]) }
        ctx->IASetPrimitiveTopology(old.PrimitiveTopology);
        ctx->IASetIndexBuffer(old.IndexBuffer, old.IndexBufferFormat, old.IndexBufferOffset);               SAFE_RELEASE(old.IndexBuffer)
        ctx->IASetVertexBuffers(0, 1, &old.VertexBuffer, &old.VertexBufferStride, &old.VertexBufferOffset); SAFE_RELEASE(old.VertexBuffer)
        ctx->IASetInputLayout(old.InputLayout);                                                             SAFE_RELEASE(old.InputLayout)

    }


    void TUI_PipelineBase::DXSetupRenderState(  ImDrawData* draw_data, ID3D11DeviceContext* ctx  ) {

        // Setup viewport
        D3D11_VIEWPORT vp;
        memset(&vp, 0, sizeof(D3D11_VIEWPORT));
        vp.Width = draw_data->DisplaySize.x;
        vp.Height = draw_data->DisplaySize.y;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = vp.TopLeftY = 0;
        ctx->RSSetViewports(1, &vp);

        // Setup shader and vertex buffers
        unsigned int stride = sizeof(ImDrawVert);
        unsigned int offset = 0;
        ctx->IASetInputLayout(g_pInputLayout);
        ctx->IASetVertexBuffers(0, 1, &g_pVB, &stride, &offset);
        ctx->IASetIndexBuffer(g_pIB, sizeof(ImDrawIdx) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
        ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        ctx->VSSetShader(g_pVertexShader, NULL, 0);
        ctx->VSSetConstantBuffers(0, 1, &g_pVertexConstantBuffer);
        ctx->PSSetShader(g_pPixelShader, NULL, 0);
        ctx->PSSetSamplers(0, (int) g_pSamplers.size(), &g_pSamplers[0]);
        ctx->GSSetShader(NULL, NULL, 0);
        ctx->HSSetShader(NULL, NULL, 0); // In theory we should backup and restore this as well.. very infrequently used..
        ctx->DSSetShader(NULL, NULL, 0); // In theory we should backup and restore this as well.. very infrequently used..
        ctx->CSSetShader(NULL, NULL, 0); // In theory we should backup and restore this as well.. very infrequently used..

        // Setup blend state
        const float blend_factor[4] = { 0.f, 0.f, 0.f, 0.f };
        ctx->OMSetBlendState(g_pBlendState, blend_factor, 0xffffffff);
        ctx->OMSetDepthStencilState(g_pDepthStencilState, 0);
        ctx->RSSetState(g_pRasterizerState);
    }

    void TUI_PipelineBase::DXCreatePixelShader() {

        if (g_pPixelShader) { g_pPixelShader->Release(); g_pPixelShader = NULL; }
        if (g_pPixelShaderBlob) { g_pPixelShaderBlob->Release(); g_pPixelShaderBlob = NULL; }
    
        string nbSamp = to_string( std::max(g_pSamplers.size(), (size_t) 1) );
        string nbTex = to_string( std::max(g_pTextureViews.size(), (size_t) 1) );
        string pixelShaderDECL = 
            "sampler samplers[" + nbSamp + "] : register(s0);\n"
            "Texture2D textures[" + nbTex + "] : register(t0);\n";

        string pixelShaderIFS = "";
        string currID = "";
        for ( int i = 1; i < g_pTextureViews.size(); i++ ) {
            currID = to_string(i);
            pixelShaderIFS += "else if ( input.texID == " + currID + " ) { out_col = textures[" + currID + "].Sample(samplers[" + currID + "], input.uv); }\n";
        }

        string pixelShader =  
        "struct PS_INPUT {\n"
            "float4 pos : SV_POSITION;\n"
            "float4 col : COLOR0;\n"
            "float2 uv  : TEXCOORD0;\n"
            "int texID  : TEXINDEX0;\n"
        "};\n"
        + pixelShaderDECL +
        "float4 main(PS_INPUT input) : SV_Target {\n"
            "float4 out_col;\n"
            "if ( input.texID == 0 ) { out_col = textures[0].Sample(samplers[0], input.uv); }\n"
            + pixelShaderIFS +
            "else { out_col = input.col * textures[0].Sample(samplers[0], input.uv); }\n"
            "return out_col;\n"
        "}";

        // std::cout << "pixelShader: \n" << pixelShader << std::endl;
        
        D3DCompile(pixelShader.c_str(), pixelShader.length(), NULL, NULL, NULL, "main", "ps_4_0", 0, 0, &g_pPixelShaderBlob, NULL);
        if (g_pPixelShaderBlob == NULL) { // NB: Pass ID3D10Blob* pErrorBlob to D3DCompile() to get error showing in (const char*)pErrorBlob->GetBufferPointer(). Make sure to Release() the blob!
            std::cout << "    Failed to Create Pixel Shader Blob" << std::endl;
            return;
        }
        if (g_pd3dDevice->CreatePixelShader((DWORD*)g_pPixelShaderBlob->GetBufferPointer(), g_pPixelShaderBlob->GetBufferSize(), NULL, &g_pPixelShader) != S_OK) {
            std::cout << "    Failed to Create Pixel Shader" << std::endl;
            return;
        }

    }

    void TUI_PipelineBase::DXCreateDeviceObjects() {
        if (!g_pd3dDevice) return;
        DXInvalidateDeviceObjects();

        // By using D3DCompile() from <d3dcompiler.h> / d3dcompiler.lib, we introduce a dependency to a given version of d3dcompiler_XX.dll (see D3DCOMPILER_DLL_A)
        // If you would like to use this DX11 sample code but remove this dependency you can:
        //  1) compile once, save the compiled shader blobs into a file or source code and pass them to CreateVertexShader()/CreatePixelShader() [preferred solution]
        //  2) use code to detect any version of the DLL and grab a pointer to D3DCompile from the DLL.
        // See https://github.com/ocornut/imgui/pull/638 for sources and details.

        // Create the vertex shader
        {
            static const char* vertexShader =
                "cbuffer vertexBuffer : register(b0) \
                {\
                float4x4 ProjectionMatrix; \
                };\
                struct VS_INPUT\
                {\
                float2 pos : POSITION;\
                float4 col : COLOR0;\
                float2 uv  : TEXCOORD0;\
                int texID  : TEXINDEX0;\
                };\
                \
                struct PS_INPUT\
                {\
                float4 pos : SV_POSITION;\
                float4 col : COLOR0;\
                float2 uv  : TEXCOORD0;\
                int texID  : TEXINDEX0;\
                };\
                \
                PS_INPUT main(VS_INPUT input)\
                {\
                PS_INPUT output;\
                output.pos = mul( ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));\
                output.col = input.col;\
                output.uv  = input.uv;\
                output.texID = input.texID;\
                return output;\
                }";

            D3DCompile(vertexShader, strlen(vertexShader), NULL, NULL, NULL, "main", "vs_4_0", 0, 0, &g_pVertexShaderBlob, NULL);
            if (g_pVertexShaderBlob == NULL) {// NB: Pass ID3D10Blob* pErrorBlob to D3DCompile() to get error showing in (const char*)pErrorBlob->GetBufferPointer(). Make sure to Release() the blob!
                std::cout << "    Failed to Create Vertex Shader Blob" << std::endl;
                return;
            }
            if (g_pd3dDevice->CreateVertexShader((DWORD*)g_pVertexShaderBlob->GetBufferPointer(), g_pVertexShaderBlob->GetBufferSize(), NULL, &g_pVertexShader) != S_OK) {
                std::cout << "    Failed to Create Vertex Shader Blob" << std::endl;
                return;
            }

            // Create the input layout
            D3D11_INPUT_ELEMENT_DESC local_layout[] =
            {
                { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,   0, (size_t)(&((ImDrawVert*)0)->pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,   0, (size_t)(&((ImDrawVert*)0)->uv),  D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, (size_t)(&((ImDrawVert*)0)->col), D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "TEXINDEX", 0, DXGI_FORMAT_R32_SINT,       0, (size_t)(&((ImDrawVert*)0)->texID), D3D11_INPUT_PER_VERTEX_DATA, 0 },
            };
            if (g_pd3dDevice->CreateInputLayout(local_layout, 4, g_pVertexShaderBlob->GetBufferPointer(), g_pVertexShaderBlob->GetBufferSize(), &g_pInputLayout) != S_OK) {
                std::cout << "    Failed to Create Input Layout" << std::endl;
                return;
            }

            // Create the constant buffer
            {
                D3D11_BUFFER_DESC desc;
                desc.ByteWidth = sizeof(VERTEX_CONSTANT_BUFFER);
                desc.Usage = D3D11_USAGE_DYNAMIC;
                desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
                desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
                desc.MiscFlags = 0;
                g_pd3dDevice->CreateBuffer(&desc, NULL, &g_pVertexConstantBuffer);
            }
        }

        // Create the blending setup
        {
            D3D11_BLEND_DESC desc;
            ZeroMemory(&desc, sizeof(desc));
            desc.AlphaToCoverageEnable = false;
            desc.RenderTarget[0].BlendEnable = true;
            desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
            desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
            desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
            desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
            desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
            desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
            desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
            g_pd3dDevice->CreateBlendState(&desc, &g_pBlendState);
        }

        // Create the rasterizer state
        {
            D3D11_RASTERIZER_DESC desc;
            ZeroMemory(&desc, sizeof(desc));
            desc.FillMode = D3D11_FILL_SOLID;
            desc.CullMode = D3D11_CULL_NONE;
            desc.ScissorEnable = true;
            desc.DepthClipEnable = true;
            g_pd3dDevice->CreateRasterizerState(&desc, &g_pRasterizerState);
        }

        // Create depth-stencil State
        {
            D3D11_DEPTH_STENCIL_DESC desc;
            ZeroMemory(&desc, sizeof(desc));
            desc.DepthEnable = false;
            desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
            desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
            desc.StencilEnable = false;
            desc.FrontFace.StencilFailOp = desc.FrontFace.StencilDepthFailOp = desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
            desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
            desc.BackFace = desc.FrontFace;
            g_pd3dDevice->CreateDepthStencilState(&desc, &g_pDepthStencilState);
        }

        DXCreateTexture();
        DXCreatePixelShader();

        DeviceObjValid = true;

    }

    void TUI_PipelineBase::DXInvalidateDeviceObjects() {
        if (!g_pd3dDevice) return;
        if ( !DeviceObjValid ) { return; }
        DXInvalidateTexturesObjects();
        SAFE_RELEASE(g_pIB);
        SAFE_RELEASE(g_pVB);
        SAFE_RELEASE(g_pBlendState);
        SAFE_RELEASE(g_pDepthStencilState);
        SAFE_RELEASE(g_pRasterizerState);
        SAFE_RELEASE(g_pPixelShader);
        SAFE_RELEASE(g_pPixelShaderBlob);
        SAFE_RELEASE(g_pVertexConstantBuffer);
        SAFE_RELEASE(g_pInputLayout);
        SAFE_RELEASE(g_pVertexShader);
        SAFE_RELEASE(g_pVertexShaderBlob);
        DeviceObjValid = false;
    }

    void TUI_PipelineBase::DXInvalidateTexturesObjects() {
        for ( auto p : g_pSamplers ) { SAFE_RELEASE(p); } g_pSamplers.clear();
        for ( auto p : g_pTextureViews ) { SAFE_RELEASE(p); } g_pTextureViews.clear();
    }

    void TUI_PipelineBase::DXInvalidateNonResidentTextures() {
        for ( size_t i = UILibCTX->IO.NumberOfResidentTextures; i < g_pSamplers.size(); ++i ) {
            SAFE_RELEASE( g_pSamplers[i] );
        }
        for ( size_t i = UILibCTX->IO.NumberOfResidentTextures; i < g_pTextureViews.size(); ++i ) {
            SAFE_RELEASE( g_pTextureViews[i] );
        }
        g_pSamplers.resize( UILibCTX->IO.NumberOfResidentTextures );
        g_pTextureViews.resize( UILibCTX->IO.NumberOfResidentTextures );
    }

} // end namespace AppCore

#endif // C_TUI_PIPELINEBASE_CPP