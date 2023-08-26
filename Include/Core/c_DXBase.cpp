#ifndef C_DXBASE_CPP
#define C_DXBASE_CPP

#include "c_DXBase.h"

#ifdef R64_DX11_ENABLED
#ifdef _DEBUG
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif
#endif // R64_DX11_ENABLED

namespace AppCore {

#ifdef R64_DX11_ENABLED

    // ------- DXBase ------- //

    DXBase::DXBase( bool & inCanRender, UILib::DisplayStateData & inDisplayState ) :
        DXCanRender( inCanRender ),
        DXDisplayState(inDisplayState) {}

    bool DXBase::InitDX( WindowParameters window ) {
        #if !defined( _WIN32 )
        return false;
        #endif
        
        DXCanRender = false;
        HRESULT hr;

        // Setup swap chain
        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = 2;
        sd.BufferDesc.Width = 0;
        sd.BufferDesc.Height = 0;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = glfwGetWin32Window( window.WindowPtr );
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        
        UINT createDeviceFlags = 0;
        
    #ifdef _DEBUG
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif

        V_RETURN( CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&g_pFactory) ) )

        UINT i = 0; 
        IDXGIAdapter * pAdapter;
        SIZE_T maxVRAM = 0;
        INT selectedAdapter = -1;
        while(g_pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND) 
        { 
            DXGI_ADAPTER_DESC ad;
            ZeroMemory(&ad, sizeof(DXGI_ADAPTER_DESC));
            pAdapter->GetDesc( &ad );
            if ( ad.DedicatedVideoMemory > maxVRAM ) {
                maxVRAM = ad.DedicatedVideoMemory;
                selectedAdapter = i;
            }
            wprintf( L"Found DX11 Adaptor %i: %s, Video Memory: %llu, System Memory: %llu, Shared System Memory: %llu\n", i, ad.Description, ad.DedicatedVideoMemory, ad.DedicatedSystemMemory, ad.SharedSystemMemory );
            ++i; 
        }

        if ( selectedAdapter > -1 ) {
            V_RETURN( g_pFactory->EnumAdapters((UINT)selectedAdapter, &g_pAdapter) )
            g_pAdapter->AddRef();

            DXGI_ADAPTER_DESC ad;
            ZeroMemory(&ad, sizeof(DXGI_ADAPTER_DESC));
            g_pAdapter->GetDesc( &ad );
            wprintf( L"Selected DX11 Adaptor: %s\n", ad.Description );

            wstring ws(ad.Description);
            // ADIO->SelectedGPUDesc = std::string( ws.begin(), ws.end() );
            ADIO->SelectedGPUDesc = "";
            for ( const auto & wc : ws ) {
                ADIO->SelectedGPUDesc += (char) wc;
            }
            ADIO->SelectedGPUVRam = (double) ad.DedicatedVideoMemory / (1024 * 1024 * 1024);
        }

        // AppTimer Timer;
        D3D_FEATURE_LEVEL featureLevel;
        const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
        V_RETURN( D3D11CreateDeviceAndSwapChain(g_pAdapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) );
        // std::cout << ">> TIMER (InitDX): " << Timer.elapse_millisecond() << std::endl;
        CreateRenderTarget();

        DXCanRender = true;
        return true;

    }

    void DXBase::ResizeSwapChain(int width, int height) {

        DXCanRender = false;

        DXGI_SWAP_CHAIN_DESC sd;
        g_pSwapChain->GetDesc(&sd);
                
        CleanupRenderTarget();
        g_pSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, sd.Flags);
        CreateRenderTarget();

        DXCanRender = true;

    }

    void DXBase::SyncWindowSizeChange() {
        assert(g_pd3dDevice && "Device must be available!");
        ResizeSwapChain( DXDisplayState.WindowSize.W, DXDisplayState.WindowSize.H );      
    }

    

    ID3D11Device * DXBase::GetDXDevice() {
        return g_pd3dDevice;
    }

    ID3D11DeviceContext * DXBase::GetDXDeviceContext() {
        return g_pd3dDeviceContext;
    }

    ID3D11RenderTargetView* DXBase::GetDXRenderTargetView() {
        return g_mainRenderTargetView;
    }

    void DXBase::CreateRenderTarget() {
        // std::cout << "DXBase::CreateRenderTarget    START" << std::endl;
        ID3D11Texture2D* pBackBuffer;
        g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        if ( FAILED(g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView)) ) {
            std::cout << "Failed to CreateRenderTargetView" << std::endl;
        }
        pBackBuffer->Release();
        // std::cout << "DXBase::CreateRenderTarget    END" << std::endl;
    }

    void DXBase::CleanupRenderTarget() {
        if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
    }

    bool DXBase::CreateDXImage(unsigned char* image_data, int image_width, int image_height, ID3D11ShaderResourceView** out_srv) {
        // image_data is a raw RGBA buffer
 
        if (image_data == NULL)
            return false;

        // Create texture
        D3D11_TEXTURE2D_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Width = image_width;
        desc.Height = image_height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = 0;

        ID3D11Texture2D *pTexture = NULL;
        D3D11_SUBRESOURCE_DATA subResource;
        subResource.pSysMem = image_data;
        subResource.SysMemPitch = desc.Width * 4;
        subResource.SysMemSlicePitch = 0;
        g_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);

        // Create texture view
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        ZeroMemory(&srvDesc, sizeof(srvDesc));
        srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = desc.MipLevels;
        srvDesc.Texture2D.MostDetailedMip = 0;
        g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, out_srv);
        pTexture->Release();

        return true;
    }

    void DXBase::CreateDXSampler(ID3D11SamplerState** out_ss) {
        // Create texture sampler
        D3D11_SAMPLER_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.MipLODBias = 0.f;
        desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        desc.MinLOD = 0.f;
        desc.MaxLOD = 0.f;
        g_pd3dDevice->CreateSamplerState(&desc, out_ss);
    }

    DXBase::~DXBase() {
        CleanupRenderTarget();
        SAFE_RELEASE(g_pSwapChain)
        SAFE_RELEASE(g_pd3dDeviceContext)
        SAFE_RELEASE(g_pd3dDevice)
        SAFE_RELEASE(g_pFactory)
        SAFE_RELEASE(g_pAdapter)

    }

#else // R64_DX11_ENABLED

    DXBase::DXBase( bool & inCanRender, UILib::DisplayStateData & inDisplayState ) {}
    DXBase::~DXBase() {}

    bool DXBase::InitDX( WindowParameters window ) { return false; }

    void DXBase::SyncWindowSizeChange() {}

#endif // R64_DX11_ENABLED

    std::string DXBase::GetWindowsVersion() {
    #if defined( _WIN32 )
    try {
        // https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getversionexa
        // https://docs.microsoft.com/en-us/windows/win32/sysinfo/targeting-your-application-at-windows-8-1
        _OSVERSIONINFOA   info;
        ZeroMemory(&info, sizeof(_OSVERSIONINFOA));
        info.dwOSVersionInfoSize = sizeof(_OSVERSIONINFOA);
        GetVersionEx((LPOSVERSIONINFOA )&info);
        // printf( "Windows Version: %i, %i, %i, %i, %s\n", info.dwMajorVersion, info.dwMinorVersion, info.dwBuildNumber, info.dwPlatformId, info.szCSDVersion );
        std::string version = "Windows " + to_string(info.dwMajorVersion) + "." + to_string(info.dwMinorVersion) + ", Build: " + to_string(info.dwBuildNumber);
        return version;
    } catch (...) { return ""; }
    #else
        return "";
    #endif
    }

} // end namespace AppCore

#endif /* C_DXBASE_CPP */
