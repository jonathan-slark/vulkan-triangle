/* Based on:
 * https://learn.microsoft.com/en-us/windows/win32/learnwin32/your-first-windows-program
 * TODO:
 * Check message loop, it blocks until a message is sent. Currently using WM_PAINT.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "config.h"
#include "util.h"
#include "vulkan.h"
#include "win32.h"

static const wchar_t classname[] = L"Main Window";

HWND hwnd;
int frame = 0;

static LRESULT CALLBACK
WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_DESTROY:
	/*  Request to quit */
	PostQuitMessage(0);
	return 0;
    case WM_PAINT:
	fprintf(stderr, "Frame #%i\n", frame++);
	vk_drawframe();
    }

    /*  If we don't handle the message, use the default handler */
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI
wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    UNUSED(hPrevInstance);
    UNUSED(pCmdLine);
    WNDCLASS wc = { 0 };
    MSG msg     = { 0 };
    wchar_t *title;
    int len;

    /* Convert char string to wchar_t string */
    len = strlen(appname);
    title = (wchar_t *) malloc((len + 1) * sizeof(wchar_t));
    mbstowcs(title, appname, len + 1);

    /*  Register the window class  */
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = classname;
    RegisterClass(&wc);

    /*  Create the window  */
    hwnd = CreateWindowEx(
	    0,         /*  Optional window styles  */
	    classname, /*  Window class  */
	    title,     /*  Window text  */

	    /*  Window style  */
	    (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX),

	    /*  Size and position  */
	    CW_USEDEFAULT, CW_USEDEFAULT, appwidth, appheight,

	    NULL,       /*  Parent window  */
	    NULL,       /*  Menu  */
	    hInstance,  /*  Instance handle  */
	    NULL        /*  Additional application data  */
	    );
    free(title);
    assert(hwnd != NULL);

    /*  Initialise Vulkan */
    vk_initialise();

    /*  Show the window */
    ShowWindow(hwnd, nCmdShow);

    /*  Run the message loop  */
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
	TranslateMessage(&msg);
	DispatchMessage(&msg);
    }

    /*  Terminate Vulkan, once the logical device is finished */
    vk_devicewait();
    vk_terminate();

    return 0;
}
