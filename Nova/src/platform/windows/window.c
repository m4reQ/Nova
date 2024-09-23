#include <Nova/graphics/window.h>
#include <stdio.h>

#define MAX_ERROR_MSG_BUF_SIZE 1024 * 64
#define WINDOW_CLASS_NAME L"NovaWindow"
#define WIN_HANDLE s_Data.window
#define ASSERT_INITIALIZED NV_ASSERT(s_IsInitialized, "Window module is not initialized")

static bool s_IsInitialized;
static NvWindowData s_Data;

static LRESULT CALLBACK WindowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CLOSE:
        s_Data.shouldClose = true;
        return 0;
    default:
        return DefWindowProcW(window, msg, wParam, lParam);
    }
}

static const char *GetWin32ErrorString(void)
{
    static char buffer[MAX_ERROR_MSG_BUF_SIZE];

    DWORD bytesWritten = FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        GetLastError(),
        LANG_NEUTRAL,
        buffer,
        MAX_ERROR_MSG_BUF_SIZE,
        NULL);
    if (bytesWritten == 0)
        return "";

    return buffer;
}

static bool RegisterWindowClass(void)
{
    WNDCLASSW class = {
        .lpszClassName = WINDOW_CLASS_NAME,
        .hIcon = NULL,
        .hInstance = GetModuleHandleW(NULL),
        .lpfnWndProc = WindowProc,
    };

    return RegisterClassW(&class);
}

NvWindowResize(uint32_t width, uint32_t height)
{
    ASSERT_INITIALIZED;
    SetWindowPos(WIN_HANDLE, NULL, 0, 0, (int)width, (int)height, SWP_NOMOVE);
}

NvWindowMove(uint32_t x, uint32_t y)
{
    ASSERT_INITIALIZED;
    SetWindowPos(WIN_HANDLE, NULL, (int)x, (int)y, 0, 0, SWP_NOSIZE);
}

NvWindowSetTitle(const wchar_t *title)
{
    ASSERT_INITIALIZED;
    SetWindowTextW(WIN_HANDLE, title);
}

NvWindowClose(void)
{
    ASSERT_INITIALIZED;
    CloseWindow(WIN_HANDLE);
}

bool _NvWindowInitialize(const NvWindowSettings *settings, const NvStartupData *startupData)
{
    NV_ASSERT(settings != NULL, "window settings cannot be NULL");

    if (!RegisterWindowClass())
    {
        fprintf(stderr, "Failed to register window class: %s.\n", GetWin32ErrorString());
        return false;
    }

    HWND window = CreateWindowW(
        WINDOW_CLASS_NAME,
        settings->title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        settings->width,
        settings->height,
        NULL,
        NULL,
        GetModuleHandleW(NULL),
        NULL);
    if (!window)
    {
        fprintf(stderr, "Failed to create window: %s.\n", GetWin32ErrorString());
        return false;
    }

    s_Data = (NvWindowData){
        .window = window,
        .shouldClose = false,
    };

    ShowWindow(WIN_HANDLE, SW_SHOW);

    s_IsInitialized = true;
    return true;
}

void _NvWindowShutdown(void)
{
    if (s_IsInitialized)
        DestroyWindow(s_Data.window);

    s_IsInitialized = false;
}

void _NvWindowUpdate(void)
{
    ASSERT_INITIALIZED;

    MSG msg;
    while (PeekMessageW(&msg, WIN_HANDLE, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

const NvWindowData *_NvWindowGetPlatformData(void)
{
    ASSERT_INITIALIZED;
    return &s_Data;
}

bool _NvWindowShouldClose(void)
{
    return s_Data.shouldClose;
}
