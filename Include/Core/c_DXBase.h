#ifndef C_DXBASE_H
#define C_DXBASE_H

#include "c_Window.h"

#if defined( _WIN32 )
#define R64_DX11_ENABLED
#endif

#ifdef R64_DX11_ENABLED
#include <DirectXMath.h> // For XMVector, XMFLOAT3, XMFLOAT4
#include <comdef.h>      // for _com_error
#include <d3d11.h>       // for D3D11 interface
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
// #include <dxgi1_6.h>     // for DXGI
// #include <wrl.h>         // for Microsoft::WRL::ComPtr
// using Microsoft::WRL::ComPtr;
using namespace DirectX;

static void DxTrace(const wchar_t *file, unsigned long line, HRESULT hr,const wchar_t *proc) {
  _com_error err(hr);
  std::cout << "file:" << file << "line:" << line << ", " << proc << " Error: " << (const char *)err.Description() << std::endl;
};

#define V_RETURN(op)                        \
if (FAILED(hr = (op))) {                    \
    assert(0);                              \
    DxTrace(__FILEW__, __LINE__, hr, L#op); \
    return false;                           \
}

#define V(op)                               \
if (FAILED(hr = (op))) {                    \
    assert(0);                              \
    DxTrace(__FILEW__, __LINE__, hr, L#op); \
}

#define SAFE_RELEASE(P) if (P) { P->Release(); P = NULL; }

#endif // R64_DX11_ENABLED

namespace AppCore {

    // ************************************************************ //
    // DXBase Class                                                 //
    // ************************************************************ //
    class DXBase {

    public:

        DXBase( bool & inCanRender, UILib::DisplayStateData & inDisplayState );
        virtual ~DXBase();

        bool                            InitDX( WindowParameters window );
        void                            SyncWindowSizeChange();
        std::string                     GetWindowsVersion();

    #ifdef R64_DX11_ENABLED

        // Accessing private handles
        
        ID3D11Device *                  GetDXDevice();
        ID3D11DeviceContext *           GetDXDeviceContext();
        ID3D11RenderTargetView*         GetDXRenderTargetView();

        bool                            CreateDXImage(unsigned char* image_data, int image_width, int image_height, ID3D11ShaderResourceView** out_srv);
        void                            CreateDXSampler(ID3D11SamplerState** out_ss);

    protected:

        IDXGIFactory*                   g_pFactory = NULL;
        IDXGIAdapter*                   g_pAdapter = NULL;
        ID3D11Device*                   g_pd3dDevice = NULL;
        ID3D11DeviceContext*            g_pd3dDeviceContext = NULL;
        IDXGISwapChain*                 g_pSwapChain = NULL;
        ID3D11RenderTargetView*         g_mainRenderTargetView = NULL;
        
    private:

        bool &                          DXCanRender;
        UILib::DisplayStateData &       DXDisplayState;
        
        void                            CreateRenderTarget();
        void                            CleanupRenderTarget();
        void                            ResizeSwapChain(int width, int height);

    #endif // R64_DX11_ENABLED
    };

} // end namespace AppCore


#endif /* C_DXBASE_H */
