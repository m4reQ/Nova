#include <Nova/graphics/window.h>
#include <stdio.h>

#define MAX_ERROR_MSG_BUF_SIZE 1024 * 64
#define WINDOW_CLASS_NAME L"NovaWindow"
#define WIN_HANDLE s_Data.window
#define ASSERT_INITIALIZED NV_ASSERT(s_IsInitialized, "Window module is not initialized")

static bool s_IsInitialized;
static NvWindowData s_Data;

static int GetRectWidth(const RECT *rect)
{
    return rect->right - rect->left;
}

static int GetRectHeight(const RECT *rect)
{
    return rect->bottom - rect->top;
}

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

static HICON LoadIconFromFilepath(const wchar_t *filepath)
{
    HICON icon = (HICON)LoadImageW(NULL, filepath, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
    if (!icon)
    {
        fprintf(stderr, "Failed to load icon from file \"%ls\": %s.\n", filepath, GetWin32ErrorString());
        return NULL;
    }

    return icon;
}

static bool RegisterWindowClass(HICON icon, HINSTANCE instance)
{
    WNDCLASSW class = {
        .lpszClassName = WINDOW_CLASS_NAME,
        .hIcon = icon,
        .hInstance = instance,
        .lpfnWndProc = WindowProc,
    };

    return RegisterClassW(&class);
}

static DWORD GetWindowStyle(NvWindowFlags flags)
{
    if (IS_FLAG_SET(flags, NV_WND_FLAGS_FULLSCREEN))
        return WS_POPUP;

    DWORD style = IS_FLAG_SET(flags, NV_WND_FLAGS_BORDERLESS) ? WS_OVERLAPPED : WS_OVERLAPPEDWINDOW;
    if (!IS_FLAG_SET(flags, NV_WND_FLAGS_RESIZABLE))
        style &= ~WS_THICKFRAME;

    return style;
}

static DWORD GetWindowExStyle(NvWindowFlags flags)
{
    DWORD style = IS_FLAG_SET(flags, NV_WND_FLAGS_ALLOW_FILE_DROP) ? WS_EX_ACCEPTFILES : 0;
    if (IS_FLAG_SET(flags, NV_WND_FLAGS_TRANSPARENT))
        style |= WS_EX_TRANSPARENT;

    return style;
}

void NvWindowResize(uint32_t width, uint32_t height)
{
    ASSERT_INITIALIZED;
    SetWindowPos(WIN_HANDLE, NULL, 0, 0, (int)width, (int)height, SWP_NOMOVE);
}

void NvWindowMove(uint32_t x, uint32_t y)
{
    ASSERT_INITIALIZED;
    SetWindowPos(WIN_HANDLE, NULL, (int)x, (int)y, 0, 0, SWP_NOSIZE);
}

void NvWindowSetTitle(const wchar_t *title)
{
    ASSERT_INITIALIZED;
    SetWindowTextW(WIN_HANDLE, title);
}

void NvWindowClose(void)
{
    ASSERT_INITIALIZED;
    CloseWindow(WIN_HANDLE);
}

void NvWindowSetIcon(const wchar_t *filepath)
{
    ASSERT_INITIALIZED;

    if (s_Data.icon)
        DestroyIcon(s_Data.icon);

    HICON icon = LoadIconFromFilepath(filepath);
    if (!icon)
    {
        fprintf(stderr, "Failed to set window icon.\n");
        return;
    }

    SendMessage(WIN_HANDLE, WM_SETICON, ICON_SMALL, (LPARAM)icon);
    SendMessage(WIN_HANDLE, WM_SETICON, ICON_BIG, (LPARAM)icon);

    s_Data.icon = icon;
}

void NvWindowSetFullscreen(bool isEnabled)
{
    ASSERT_INITIALIZED;

    if (isEnabled && !s_Data.isFullscreen)
    {
        GetWindowPlacement(WIN_HANDLE, &s_Data.savedPlacement);

        MONITORINFO info = {
            .cbSize = sizeof(MONITORINFO),
        };
        GetMonitorInfoW(MonitorFromWindow(WIN_HANDLE, MONITOR_DEFAULTTOPRIMARY), &info);

        SetWindowLongW(WIN_HANDLE, GWL_STYLE, s_Data.originalStyle & ~WS_OVERLAPPEDWINDOW);
        SetWindowPos(
            WIN_HANDLE,
            HWND_TOP,
            info.rcMonitor.left,
            info.rcMonitor.top,
            GetRectWidth(&info.rcMonitor),
            GetRectHeight(&info.rcMonitor),
            SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
    else if (!isEnabled && s_Data.isFullscreen)
    {
        SetWindowLongW(WIN_HANDLE, GWL_STYLE, s_Data.originalStyle);
        SetWindowPlacement(WIN_HANDLE, &s_Data.savedPlacement);
        SetWindowPos(
            WIN_HANDLE,
            NULL,
            0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }

    s_Data.isFullscreen = isEnabled;
}

void NvWindowMaximize(void)
{
    ASSERT_INITIALIZED;
    ShowWindow(WIN_HANDLE, SW_MAXIMIZE);
}

void NvWindowIconfiy(void)
{
    ASSERT_INITIALIZED;
    ShowWindow(WIN_HANDLE, SW_MINIMIZE);
}

void NvWindowGetSize(uint32_t *width, uint32_t *height)
{
    ASSERT_INITIALIZED;

    RECT rect;
    GetWindowRect(WIN_HANDLE, &rect);

    if (width)
        *width = (uint32_t)GetRectWidth(&rect);

    if (height)
        *height = (uint32_t)GetRectHeight(&rect);
}

void NvWindowGetFramebufferSize(uint32_t *width, uint32_t *height)
{
    ASSERT_INITIALIZED;

    RECT rect;
    GetClientRect(WIN_HANDLE, &rect);

    if (width)
        *width = (uint32_t)GetRectWidth(&rect);

    if (height)
        *height = (uint32_t)GetRectHeight(&rect);
}

void NvWindowGetPosition(uint32_t *x, uint32_t *y)
{
    ASSERT_INITIALIZED;

    RECT rect;
    GetWindowRect(WIN_HANDLE, &rect);

    if (x)
        *x = (uint32_t)rect.left;

    if (y)
        *y = (uint32_t)rect.top;
}

bool NvWindowIsVisible(void)
{
    ASSERT_INITIALIZED;
    return s_Data.isVisible;
}

bool _NvWindowInitialize(const NvWindowSettings *settings)
{
    NV_ASSERT(settings != NULL, "window settings cannot be NULL");

    HINSTANCE instance = GetModuleHandleW(NULL);
    HICON icon = settings->iconFilepath ? LoadIconFromFilepath(settings->iconFilepath) : NULL;
    if (!RegisterWindowClass(icon, instance))
    {
        fprintf(stderr, "Failed to register window class: %s.\n", GetWin32ErrorString());
        return false;
    }

    DWORD style = GetWindowStyle(settings->windowFlags);
    DWORD exStyle = GetWindowExStyle(settings->windowFlags);
    HWND window = CreateWindowExW(
        exStyle,
        WINDOW_CLASS_NAME,
        settings->title,
        style | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        settings->width,
        settings->height,
        NULL,
        NULL,
        instance,
        NULL);
    if (!window)
    {
        fprintf(stderr, "Failed to create window: %s.\n", GetWin32ErrorString());
        return false;
    }

    s_Data = (NvWindowData){
        .instance = instance,
        .window = window,
        .icon = icon,
        .originalStyle = style,
        .shouldClose = false,
        .isVisible = true,
        .isFullscreen = false,
    };

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
