#define NMD_GRAPHICS_IMPLEMENTATION
#define NMD_GRAPHICS_D3D9
#include "../nmd_graphics.hpp"

LRESULT CALLBACK WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(hWnd, Msg, wParam, lParam);
}

int main()
{
    // Register class.
    WNDCLASSEX wc = { sizeof(wc), NULL, WindowProc, 0, 0, NULL, NULL, NULL, NULL, NULL, TEXT("randomclassname"), NULL };
    if (!RegisterClassEx(&wc))
        return 0;

    // Calculate window dimensions for a 800x600 client rect.
    RECT r = { 0, 0, 800, 600 };
    if (!AdjustWindowRect(&r, WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_OVERLAPPEDWINDOW, FALSE))
        return 0;

    // Create window.
    HWND hWnd = CreateWindow(TEXT("randomclassname"), TEXT("D3D9 GUI"), WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left, r.bottom - r.top, NULL, NULL, NULL, NULL);
    if (!hWnd)
        return 0;

    // Show window.
    ShowWindow(hWnd, SW_SHOW);

    // Create an IDirect3D9 object.
    IDirect3D9* pD3D9 = Direct3DCreate9(D3D_SDK_VERSION);
    if (!pD3D9)
        return 0;

    // Create device.
    D3DPRESENT_PARAMETERS d3dpp = { 800, 600, D3DFMT_A8R8G8B8, 2, D3DMULTISAMPLE_8_SAMPLES, 0, D3DSWAPEFFECT_DISCARD, hWnd, TRUE, FALSE, D3DFMT_UNKNOWN, 0, D3DPRESENT_INTERVAL_DEFAULT };
    IDirect3DDevice9* pD3D9Device = NULL;
    if ((pD3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, NULL, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &pD3D9Device)) != D3D_OK)
        return 0;

    nmd::D3D9SetDevice(pD3D9Device);
	nmd::D3D9Resize(800, 600);

    while (true)
    {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }

        // Render vertex and index buffers.
        nmd::Begin();

        auto& drawList = nmd::GetContext().drawList;

        // uses trignometric functions to make a simple animation.
        static float i = 0.0f;
        i = fmodf(i + 0.05f, 6.283184f);
        const nmd::Vec2 circleEndPoint = nmd::Vec2(550 + 100.0f * cosf(i), 180 + 100.0f * sinf(i));

        // Add hexagon
        drawList.AddNgonFilled({ 250, 170 }, 45.0f, nmd::Color::AndroidGreen, 6);

        // Add blue ciclic pie circle.
        drawList.PathArcTo({ 550, 180 }, 100.0f, 0, i, 36, true);
        drawList.PathFillConvex(nmd::Color::LapisLazuli);

        // Add red border to the circle.
        drawList.PathArcTo({ 550, 180 }, 100.0f, 0, i, 24);
        drawList.PathStroke(nmd::Color::Lava, false, 2.0f);

        // Add lines
        drawList.AddLine({ 50, 50 }, circleEndPoint, nmd::Color::Corn);
        drawList.AddLine({ 50, 400 }, circleEndPoint, nmd::Color::Corn);

        // Add bezier curve
        drawList.AddBezierCurve({ 100, 500 }, circleEndPoint, circleEndPoint, { 800, 600 }, nmd::Color::Bronze);

        nmd::End();

        // Clear screen. Begin scene.
        pD3D9Device->Clear(0, 0, D3DCLEAR_TARGET, D3DCOLOR_XRGB(70, 70, 70), 0, 0);
        pD3D9Device->BeginScene();

        // Draw.
        nmd::D3D9Render();

        // End scene. Present.
        pD3D9Device->EndScene();
        pD3D9Device->Present(NULL, NULL, NULL, NULL);
    }
}