#define NMD_GRAPHICS_IMPLEMENTATION
#define NMD_GRAPHICS_D3D9
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
    wcx.lpszClassName = TEXT("D3D9");

    if (!RegisterClassExW(&wcx))
        return -1;

    HWND hWnd;
    if (!(hWnd = CreateWindowExW(0, TEXT("D3D9"), TEXT("D3D9"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, NULL, NULL)))
        return -1;

    ShowWindow(hWnd, SW_SHOW);

    LPDIRECT3D9 d3d9;
    LPDIRECT3DDEVICE9 d3dDevice;
    if ((d3d9 = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
        return -1;
    
    D3DPRESENT_PARAMETERS d3dpp;
    memset(&d3dpp, 0, sizeof(D3DPRESENT_PARAMETERS));
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    if (d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &d3dDevice) < 0)
        return false;
    d3dDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
    nmd_get_context()->drawList.fillAntiAliasing = false;
    
    nmd_d3d9_set_device(d3dDevice);
    nmd_d3d9_resize(640, 480);

    nmd_atlas a;
    nmd_bake_font("C:/Windows/Fonts/arial.ttf", &a, 32.0f);

    a.font = nmd_d3d9_create_texture(a.pixels32, a.width, a.height);

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

        nmd_add_text(&a, 20, 20, "Nomade", 0, NMD_COLOR_RED);

        nmd_end_frame();

        d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_RGBA(255, 170, 40, 255), 0, 0);
        d3dDevice->BeginScene();
        nmd_d3d9_render();
        d3dDevice->EndScene();
        HRESULT result = d3dDevice->Present(NULL, NULL, NULL, NULL);
    }
}