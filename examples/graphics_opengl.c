#define NMD_GRAPHICS_IMPLEMENTATION
#define NMD_GRAPHICS_OPENGL
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "../nmd_graphics.h"

#ifdef _WIN32
#pragma comment(lib, "opengl32.lib")
#endif

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
    GLFWwindow* window;
    if (!glfwInit())
        return -1;

    window = glfwCreateWindow(640, 480, "OpenGL", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glewInit();

    nmd_opengl_resize(640, 480);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        nmd_new_frame();

        nmd_add_rect_filled(50, 50, 200, 200, NMD_COLOR_AZURE, 0, 0);
        nmd_add_line(60, 60, 250, 60, NMD_COLOR_BLACK, 1.0f);
        nmd_add_line(60, 70, 250, 150, NMD_COLOR_BLACK, 1.0f);

        nmd_end_frame();

        glClearColor(1.0f, 0.67f, 0.14f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        nmd_opengl_render();

        glfwSwapBuffers(window);
    }
}