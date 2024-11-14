#ifndef UNICODE
#define UNICODE
#endif 

#include <assert.h>
#include <windows.h>
#include "config.h"
#include "util.h"
#include "vulkan_instance.h"
#include "win32.h"

#define CLASSNAME L"" APPNAME " Window Class"
#define TITLE     L"" APPNAME

HWND hwnd;

HWND getHwnd() {
    return hwnd;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
	case WM_DESTROY:
	    // Request to quit
	    PostQuitMessage(0);
	    return 0;

	/*
	case WM_PAINT:
	    PAINTSTRUCT ps;
	    HDC hdc = BeginPaint(hwnd, &ps);
	    FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));
	    EndPaint(hwnd, &ps);
	    return 0;
	*/
    }

    // If we don't handle the message, use the default handler
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

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
    hwnd = CreateWindowEx(
	    0,         // Optional window styles 
	    CLASSNAME, // Window class 
	    TITLE,     // Window text 

	    // Window style 
	    (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX),

	    // Size and position 
	    CW_USEDEFAULT, CW_USEDEFAULT, APPWIDTH, APPHEIGHT,

	    NULL,       // Parent window 
	    NULL,       // Menu 
	    hInstance,  // Instance handle 
	    NULL        // Additional application data 
	    );
    assert(hwnd != NULL);

    // Initialise Vulkan
    initVulkan();

    // Show the window
    ShowWindow(hwnd, nCmdShow);

    // Run the message loop 
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
	TranslateMessage(&msg);
	DispatchMessage(&msg);
    }

    // Terminate Vulkan
    termVulkan();

    return 0;
}
