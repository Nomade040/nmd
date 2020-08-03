#define NMD_GRAPHICS_IMPLEMENTATION
#define NMD_GRAPHICS_D3D11
#include "../nmd_graphics.h"

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CLOSE:
        DestroyWindow(hWnd);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int main()
{
    WNDCLASSEXW wcx;
    memset(&wcx, 0, sizeof(WNDCLASSEXW));
    wcx.cbSize = sizeof(WNDCLASSEXW);
    wcx.lpfnWndProc = WindowProc;
    wcx.lpszClassName = TEXT("D3D11");

    if (!RegisterClassExW(&wcx))
        return -1;

    HWND hWnd;
    if (!(hWnd = CreateWindowExW(0, TEXT("D3D11"), TEXT("D3D11"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, NULL, NULL)))
        return -1;

    ShowWindow(hWnd, SW_SHOW);

    ID3D11Device* pDevice;
    ID3D11DeviceContext* pDeviceContext;
    ID3D11RenderTargetView* pRenderTargetView;
    IDXGISwapChain* pSwapChain;
    
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    memset(&swapChainDesc, 0, sizeof(swapChainDesc));
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 1;
    swapChainDesc.OutputWindow = hWnd;
    swapChainDesc.Windowed = true;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_SINGLETHREADED, NULL, 0, D3D11_SDK_VERSION, &swapChainDesc, &pSwapChain, &pDevice, NULL, &pDeviceContext)))
        return -1;
    
    ID3D11Texture2D* pBackBuffer;
    if (FAILED(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer))))
        return -1;
    
    if (FAILED(pDevice->CreateRenderTargetView(pBackBuffer, 0, &pRenderTargetView)))
        return -1;
    
    pBackBuffer->Release();
    
    pDeviceContext->OMSetRenderTargets(1, &pRenderTargetView, NULL);

    nmd_d3d11_set_device_context(pDeviceContext);
    nmd_d3d11_resize(640, 480);

    MSG msg;
    while (true)
    {
        while (PeekMessageW(&msg, NULL, NULL, NULL, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
                return 0;
        }

        nmd_new_frame();

        nmd_add_rect_filled(50, 50, 200, 200, NMD_COLOR_AZURE, 0, 0);
        nmd_add_line(60, 60, 250, 60, NMD_COLOR_BLACK, 1.0f);
        nmd_add_line(60, 70, 250, 150, NMD_COLOR_BLACK, 1.0f);

        nmd_end_frame();
        
        FLOAT clear_color[4] = { 1.0f, 0.67f, 0.14f, 1.0f };
        pDeviceContext->ClearRenderTargetView(pRenderTargetView, (float*)&clear_color);
        nmd_d3d11_render();
        pSwapChain->Present(1, 0);
    }
}