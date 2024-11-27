#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "config.h"
#include "util.h"
#include "vulkan.h"
#include "win32.h"

static const char classname[] = "Main Window";

HWND hwnd;
int quitting;
int minimised;

static LRESULT CALLBACK
WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_DESTROY:
	/* Request to quit */
	quitting = 1;
	PostQuitMessage(0);
	return 0;
    case WM_SIZE:
	switch(wParam) {
	case SIZE_MINIMIZED:
	    minimised = 1;
	    break;
	case SIZE_RESTORED:
	    minimised = 0;
	    break;
	}
	return 0;
    }

    /* If we don't handle the message, use the default handler */
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void
onmessage(MSG *msg)
{
    TranslateMessage(msg);
    DispatchMessage(msg);
}

int APIENTRY
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    UNUSED(hPrevInstance);
    UNUSED(lpCmdLine);
    MSG msg;
    BOOL bRet;
    int running;
    WNDCLASS wc = {
	.style         = 0,
	.lpfnWndProc   = WindowProc,
	.cbClsExtra    = 0,
	.cbWndExtra    = 0,
	.hInstance     = hInstance,
	.hIcon         = NULL,
	.hCursor       = NULL,
	.hbrBackground = NULL,
	.lpszMenuName  = NULL,
	.lpszClassName = classname
    };

    RegisterClass(&wc);
    hwnd = CreateWindowEx(0, classname, appname,
	    (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX),
	    CW_USEDEFAULT, CW_USEDEFAULT, appwidth, appheight, NULL, NULL,
	    hInstance, NULL);
    if (hwnd == NULL)
	terminate("Failed to create window.\n");

    vk_initialise();

    minimised = (nShowCmd == SW_SHOWMINIMIZED);
    ShowWindow(hwnd, nShowCmd);

    /* Main loop */
    quitting = 0;
    running = 1;
    do {
	/* Once we've requested to quit, stop rendering and wait for WM_QUIT */
	if (quitting) {
	    while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
		if (bRet == -1 )
		    terminate("Windows message error.\n");
		else
		    onmessage(&msg);

	    running = 0;
	/* If the window has been minimised wait for WM_SIZE or WM_DESTROY */
	} else if (minimised) {
	    while (minimised && !quitting &&
		    (bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
		if (bRet == -1 )
		    terminate("Windows message error.\n");
		else
		    onmessage(&msg);

	    /* WM_QUIT may have already been processed */
	    if (msg.message == WM_QUIT)
		running = 0;
	} else {
	    vk_drawframe();

	    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		onmessage(&msg);
	}
    } while (running);

    /* Terminate Vulkan, once the logical device is finished */
    vk_devicewait();
    vk_terminate();

    /* Return nExitCode value from PostQuitMessage() */
    return msg.wParam;
}
