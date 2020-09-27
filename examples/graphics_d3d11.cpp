#define NMD_GRAPHICS_IMPLEMENTATION
#define NMD_GRAPHICS_D3D11
#include "../nmd_graphics.h"

IDXGISwapChain* g_swap_chain;
ID3D11RenderTargetView* g_render_target_view;
ID3D11Device* g_device;
ID3D11DeviceContext* g_device_context;

void create_render_target()
{
    ID3D11Texture2D* back_buffer;
    g_swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
    g_device->CreateRenderTargetView(back_buffer, NULL, &g_render_target_view);
    g_device_context->OMSetRenderTargets(1, &g_render_target_view, NULL);
    back_buffer->Release();
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    nmd_win32_wnd_proc(hWnd, uMsg, wParam, lParam);

    switch (uMsg)
    {
    case WM_SIZE:
        if (g_swap_chain && wParam != SIZE_MINIMIZED)
        {
            if (g_render_target_view)
                g_render_target_view->Release();
            g_swap_chain->ResizeBuffers(0, LOWORD(lParam), HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            create_render_target();

            nmd_d3d11_resize(LOWORD(lParam), HIWORD(lParam));
        }
        return 0;
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
        return 1;

    HWND hWnd;
    if (!(hWnd = CreateWindowExW(0, TEXT("D3D11"), TEXT("D3D11"), WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, NULL, NULL)))
        return 1;
    
    DXGI_SWAP_CHAIN_DESC swap_chain_desc;
    memset(&swap_chain_desc, 0, sizeof(swap_chain_desc));
    swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
    swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_desc.SampleDesc.Count = 1;
    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.BufferCount = 1;
    swap_chain_desc.OutputWindow = hWnd;
    swap_chain_desc.Windowed = true;
    swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_SINGLETHREADED, NULL, 0, D3D11_SDK_VERSION, &swap_chain_desc, &g_swap_chain, &g_device, NULL, &g_device_context)))
        return 1;
    
    create_render_target();

    nmd_d3d11_set_device_context(g_device_context);
    nmd_d3d11_resize(640, 480);

    bool checked = false;
    float f = 0;
    while (true)
    {
        MSG msg;
        while (PeekMessageW(&msg, NULL, NULL, NULL, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
                return msg.wParam;
        }

        nmd_new_frame();

        nmd_begin("Menu");
        if (nmd_button("Create message box"))
            MessageBoxA(0, "You clicked the button", "Button", 0);
        nmd_text("hello");
        nmd_checkbox("check box", &checked);
        nmd_slider_float("slider", &f, -10, 10);

        nmd_end();

        //nmd_add_rect_filled(50, 50, 200, 200, NMD_COLOR_AZURE, 0, 0);
        //nmd_add_line(60, 60, 250, 60, NMD_COLOR_BLACK, 1.0f);
        //nmd_add_line(60, 70, 250, 150, NMD_COLOR_BLACK, 1.0f);

        nmd_end_frame();
        
        FLOAT clear_color[4] = { 1.0f, 0.67f, 0.14f, 1.0f };
        g_device_context->ClearRenderTargetView(g_render_target_view, (float*)&clear_color);
        nmd_d3d11_render();
        g_swap_chain->Present(1, 0);
    }
}