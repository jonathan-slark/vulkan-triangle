#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>
#include "vulkan_instance.h"

#define UNUSED(x) (void) (x)

#define CLASSNAME L"Vulkan Test Window Class"
#define TITLE     L"Vulkan Test"
#define WIDTH     800
#define HEIGHT    600

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    UNUSED(hPrevInstance);
    UNUSED(pCmdLine);

    // Register the window class 
    WNDCLASS wc = {};
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASSNAME;
    RegisterClass(&wc);

    // Create the window 
    HWND hwnd = CreateWindowEx(
	    0,         // Optional window styles 
	    CLASSNAME, // Window class 
	    TITLE,     // Window text 

	    // Window style 
	    (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX),

	    // Size and position 
	    CW_USEDEFAULT, CW_USEDEFAULT, WIDTH, HEIGHT,

	    NULL,       // Parent window 
	    NULL,       // Menu 
	    hInstance,  // Instance handle 
	    NULL        // Additional application data 
	    );
    if (hwnd == NULL)
    {
	return 0;
    }

    // Initialise Vulkan
    initVulkan();

    // Show the window
    ShowWindow(hwnd, nCmdShow);

    // Run the message loop 
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
	TranslateMessage(&msg);
	DispatchMessage(&msg);
    }

    // Terminate Vulkan
    termVulkan();

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
	case WM_DESTROY:
	    PostQuitMessage(0);
	    return 0;

	case WM_PAINT:
	    PAINTSTRUCT ps;
	    HDC hdc = BeginPaint(hwnd, &ps);
	    FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));
	    EndPaint(hwnd, &ps);
	    return 0;

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
