#include <Nova/graphics/Window.hpp>
#include <Nova/debug/Profile.hpp>
#include <Nova/debug/Log.hpp>
#include <Nova/core/Input.hpp>
#include <Nova/events/Events.hpp>
#include <Nova/core/Application.hpp>
#include <Nova/platform/windows/Bitmap.hpp>
#include <Nova/platform/windows/WinRect.hpp>
#include <array>
#include <glad/wgl.h>
#include <Windowsx.h>
#include <shellapi.h>

#ifdef NV_DEBUG
constexpr auto cContextFlags = WGL_CONTEXT_DEBUG_BIT_ARB | WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
#else
constexpr auto cContextFlags = WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
#endif
constexpr auto cContextVersionMajor = 4;
constexpr auto cContextVersionMinor = 5;
constexpr auto cWindowClassName = "NovaWindow";

static void SendSetIcon(HWND window, const Nova::Icon &icon) noexcept
{
    SendMessage(
        window,
        WM_SETICON,
        ICON_BIG,
        reinterpret_cast<LPARAM>(icon.Get()));
    SendMessage(
        window,
        WM_SETICON,
        ICON_SMALL,
        reinterpret_cast<LPARAM>(icon.Get()));
}

static void SetIcon(HWND window, HICON icon) noexcept
{
    NV_PROFILE_FUNC;

    auto oldIconBig = reinterpret_cast<HICON>(
        SendMessage(
            window,
            WM_SETICON,
            ICON_BIG,
            reinterpret_cast<LPARAM>(icon)));
    auto oldIconSmall = reinterpret_cast<HICON>(
        SendMessage(
            window,
            WM_SETICON,
            ICON_SMALL,
            reinterpret_cast<LPARAM>(icon)));

    if (oldIconBig)
        DestroyIcon(oldIconBig);

    if (oldIconSmall && oldIconSmall != oldIconBig)
        DestroyIcon(oldIconSmall);
}

static void SetCustomCursor(HCURSOR cursor) noexcept
{
    NV_PROFILE_FUNC;

    auto oldCursor = SetCursor(cursor);
    if (oldCursor)
        DestroyCursor(oldCursor);
}

static HANDLE LoadImageFromFile(const std::filesystem::path &path, UINT imageType, HINSTANCE instance)
{
    NV_PROFILE_FUNC;

    HANDLE image{};
    if constexpr (std::is_same_v<std::filesystem::path::value_type, char>)
        image = LoadImageA(
            instance,
            reinterpret_cast<const char *>(path.c_str()),
            imageType,
            0,
            0,
            LR_LOADFROMFILE | LR_DEFAULTSIZE);
    else
        image = LoadImageW(
            instance,
            reinterpret_cast<const wchar_t *>(path.c_str()),
            imageType,
            0,
            0,
            LR_LOADFROMFILE | LR_DEFAULTSIZE);

    if (!image)
        throw std::runtime_error("Failed to load image file.");

    return image;
}

static void SelectPixelFormat(HDC deviceContext, std::span<const int> formatAttributes)
{
    NV_PROFILE_FUNC;

    auto formatIndex = 0;
    auto foundFormatsCount = 0u;
    const auto formatFound = wglChoosePixelFormatARB(
        deviceContext,
        formatAttributes.data(),
        nullptr,
        1,
        &formatIndex,
        &foundFormatsCount);
    if (!formatFound)
        throw std::runtime_error("Failed to find suitable pixel format.");

    SetPixelFormat(deviceContext, formatIndex, nullptr);
}

static void LoadWGL(HINSTANCE hInstance)
{
    NV_PROFILE_FUNC;

    constexpr auto wndClassName = "NovaDummyWindow";
    auto wndClass = Nova::WindowClass(
        WNDCLASSEXA{
            .cbSize = sizeof(WNDCLASSEXA),
            .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
            .lpfnWndProc = DefWindowProcA,
            .hInstance = hInstance,
            .lpszClassName = wndClassName,
        });
    auto windowHandle = Nova::WindowHandle(
        0,
        wndClass,
        "",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        500,
        500,
        hInstance);
    auto dc = Nova::DeviceContext(windowHandle);

    const PIXELFORMATDESCRIPTOR pfd{
        .nSize = sizeof(PIXELFORMATDESCRIPTOR),
        .nVersion = 1,
        .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        .iPixelType = PFD_TYPE_RGBA,
        .cColorBits = 24,
        .cAlphaBits = 0,
        .cDepthBits = 0,
        .cStencilBits = 0,
        .iLayerType = PFD_MAIN_PLANE,
    };
    const auto pfdIndex = ChoosePixelFormat(dc, &pfd);
    if (!pfdIndex)
        throw std::runtime_error("Failed to find suitable pixel format for dummy window.");

    if (!SetPixelFormat(dc, pfdIndex, &pfd))
        throw std::runtime_error("Failed to set pixel format for dummy window.");

    auto glContext = Nova::WGLContext(dc);
    glContext.MakeCurrent(dc);
    glContext.LoadModern(dc);
}

static constexpr DWORD GetStyleEx(Nova::WindowFlags flags) noexcept
{
    auto styleEx = 0l;
    if (Nova::Flag::IsSet(flags, Nova::WindowFlags::Transparent))
        Nova::Flag::Set(styleEx, WS_EX_TRANSPARENT);

    if (Nova::Flag::IsSet(flags, Nova::WindowFlags::AllowFileDrop))
        Nova::Flag::Set(styleEx, WS_EX_ACCEPTFILES);

    return styleEx;
}

static constexpr DWORD GetStyle(Nova::WindowFlags flags) noexcept
{
    auto style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_VISIBLE;
    if (Nova::Flag::IsSet(flags, Nova::WindowFlags::Resizable))
        Nova::Flag::Set(style, WS_THICKFRAME);

    if (Nova::Flag::IsSet(flags, Nova::WindowFlags::StartMaximized))
        Nova::Flag::Set(style, WS_MAXIMIZE);

    if (Nova::Flag::IsSet(flags, Nova::WindowFlags::StartMinimized))
        Nova::Flag::Set(style, WS_MINIMIZE);

    if (Nova::Flag::IsSet(flags, Nova::WindowFlags::Borderless))
    {
        Nova::Flag::Clear(style, WS_CAPTION);
        Nova::Flag::Clear(style, WS_BORDER);
    }

    return style;
}

static constexpr Nova::Key TranslateKey(WPARAM key, bool isExtended) noexcept
{
    using KeyDataType = std::underlying_type_t<Nova::Key>;

    switch (key)
    {
    case VK_BACK:
        return Nova::Key::Backspace;
    case VK_TAB:
        return Nova::Key::Tab;
    case VK_RETURN:
        return isExtended ? Nova::Key::KpEnter : Nova::Key::Enter;
    case VK_SHIFT:
        return isExtended ? Nova::Key::RightShift : Nova::Key::LeftShift;
    case VK_CONTROL:
        return isExtended ? Nova::Key::RightControl : Nova::Key::LeftControl;
    case VK_MENU:
        return isExtended ? Nova::Key::RightAlt : Nova::Key::LeftAlt;
    case VK_PAUSE:
        return Nova::Key::Pause;
    case VK_CAPITAL:
        return Nova::Key::CapsLock;
    case VK_ESCAPE:
        return Nova::Key::Escape;
    case VK_SPACE:
        return Nova::Key::Space;
    case VK_PRIOR:
        return Nova::Key::PageUp;
    case VK_NEXT:
        return Nova::Key::PageDown;
    case VK_END:
        return Nova::Key::End;
    case VK_HOME:
        return Nova::Key::Home;
    case VK_LEFT:
        return Nova::Key::Left;
    case VK_UP:
        return Nova::Key::Up;
    case VK_RIGHT:
        return Nova::Key::Right;
    case VK_DOWN:
        return Nova::Key::Down;
    case VK_SNAPSHOT:
        return Nova::Key::PrintScreen;
    case VK_INSERT:
        return Nova::Key::Insert;
    case VK_DELETE:
        return Nova::Key::Delete;
    case VK_LWIN:
        return Nova::Key::WorldLeft;
    case VK_RWIN:
        return Nova::Key::WorldRight;
    case VK_APPS:
        return Nova::Key::Menu;
    case VK_MULTIPLY:
        return Nova::Key::KpMultiply;
    case VK_ADD:
        return Nova::Key::KpAdd;
    case VK_SEPARATOR:
        return Nova::Key::KpDecimal;
    case VK_SUBTRACT:
        return Nova::Key::KpSubtract;
    case VK_DECIMAL:
        return Nova::Key::KpDecimal;
    case VK_DIVIDE:
        return Nova::Key::KpDivide;
    case VK_NUMLOCK:
        return Nova::Key::NumLock;
    case VK_SCROLL:
        return Nova::Key::ScrollLock;
    case VK_VOLUME_MUTE:
        return Nova::Key::VolumeMute;
    case VK_VOLUME_DOWN:
        return Nova::Key::VolumeDown;
    case VK_VOLUME_UP:
        return Nova::Key::VolumeUp;
    case VK_OEM_1:
        return Nova::Key::Semicolon;
    case VK_OEM_PLUS:
        return Nova::Key::Equal;
    case VK_OEM_COMMA:
        return Nova::Key::Comma;
    case VK_OEM_MINUS:
        return Nova::Key::Minus;
    case VK_OEM_PERIOD:
        return Nova::Key::Period;
    case VK_OEM_2:
        return Nova::Key::Slash;
    case VK_OEM_3:
        return Nova::Key::GraveAccent;
    case VK_OEM_4:
        return Nova::Key::LeftBracket;
    case VK_OEM_5:
        return Nova::Key::Backslash;
    case VK_OEM_6:
        return Nova::Key::RightBracket;
    case VK_OEM_7:
        return Nova::Key::Apostrophe;
    default:
        // numeric keys
        if (key >= 0x30 && key <= 0x39)
            return static_cast<Nova::Key>(static_cast<KeyDataType>(Nova::Key::Number0) + static_cast<KeyDataType>(key));

        // alpha keys
        if (key >= 0x41 && key <= 0x5A)
            return static_cast<Nova::Key>(static_cast<KeyDataType>(Nova::Key::A) + static_cast<KeyDataType>(key));

        // numpad keys
        if (key >= 0x60 && key <= 0x69)
            return static_cast<Nova::Key>(static_cast<KeyDataType>(Nova::Key::Kp0) + static_cast<KeyDataType>(key));

        // function keys
        if (key >= 0x70 && key <= 0x87)
            return static_cast<Nova::Key>(static_cast<KeyDataType>(Nova::Key::F1) + static_cast<KeyDataType>(key));
    }

    return Nova::Key::Unknown;
}

static constexpr Nova::Button GetButtonFromMsg(UINT msg, WPARAM wParam) noexcept
{
    switch (msg)
    {
    case WM_LBUTTONDOWN:
        return Nova::Button::Left;
    case WM_RBUTTONDOWN:
        return Nova::Button::Right;
    case WM_MBUTTONDOWN:
        return Nova::Button::Middle;
    case WM_XBUTTONDOWN:
        return GET_XBUTTON_WPARAM(wParam) == XBUTTON1
                   ? Nova::Button::Extra1
                   : Nova::Button::Extra2;
    default:
        return Nova::Button::Unknown;
    }
}

static void HandleButtonDown(Nova::EventSystem &eventSystem, UINT msg, WPARAM wParam) noexcept
{
    NV_PROFILE_FUNC;

    const auto button = GetButtonFromMsg(msg, wParam);

    Nova::Input::UpdateButton_(button, true);
    eventSystem.InvokeEvent<Nova::MouseButtonDownEvent>(button);
}

static void HandleButtonUp(Nova::EventSystem &eventSystem, UINT msg, WPARAM wParam) noexcept
{
    NV_PROFILE_FUNC;

    const auto button = GetButtonFromMsg(msg, wParam);

    Nova::Input::UpdateButton_(button, false);
    eventSystem.InvokeEvent<Nova::MouseButtonUpEvent>(button);
}

static void HandleMouseMove(Nova::EventSystem &eventSystem, LPARAM lParam) noexcept
{
    NV_PROFILE_FUNC;

    const auto xPos = (double)GET_X_LPARAM(lParam);
    const auto yPos = (double)GET_Y_LPARAM(lParam);

    const auto xDelta = xPos - Nova::Input::GetMouseX();
    const auto yDelta = yPos - Nova::Input::GetMouseY();

    Nova::Input::UpdateMousePos_(xPos, yPos);
    eventSystem.InvokeEvent<Nova::MouseMoveEvent>(xPos, yPos, xDelta, yDelta);
}

static void HandleScroll(Nova::EventSystem &eventSystem, double vDelta, double hDelta) noexcept
{
    NV_PROFILE_FUNC;

    Nova::Input::UpdateMouseScroll_(vDelta, hDelta);
    eventSystem.InvokeEvent<Nova::MouseScrollEvent>(vDelta, hDelta);
}

static void HandleKeyUp(Nova::EventSystem &eventSystem, WPARAM wParam, bool isExtended) noexcept
{
    NV_PROFILE_FUNC;

    const auto key = TranslateKey(wParam, isExtended);
    eventSystem.InvokeEvent<Nova::KeyUpEvent>(key);
}

static void HandleChar(WPARAM wParam) noexcept
{
    NV_PROFILE_FUNC;

    Nova::Input::AppendTextChar_(static_cast<wchar_t>(wParam));
}

static void HandleKeyDown(Nova::EventSystem &eventSystem, WPARAM wParam, bool isExtended, bool isRepeated) noexcept
{
    NV_PROFILE_FUNC;

    const auto key = TranslateKey(wParam, isExtended);
    eventSystem.InvokeEvent<Nova::KeyDownEvent>(key, isRepeated);
}

static void HandleDropFiles(Nova::EventSystem &eventSystem, HDROP drop) noexcept
{
    NV_PROFILE_FUNC;

    const auto pathsCount = DragQueryFileW(drop, static_cast<UINT>(-1), nullptr, 0);

    std::vector<std::filesystem::path> paths;
    paths.reserve(pathsCount);

    for (UINT i = 0; i < pathsCount; i++)
    {
        std::array<wchar_t, MAX_PATH> pathBuf;
        const auto pathLength = DragQueryFileW(drop, i, pathBuf.data(), pathBuf.size());

        paths.emplace_back(pathBuf.begin(), pathBuf.begin() + pathLength);
    }

    eventSystem.InvokeEvent<Nova::FileDropEvent>(std::move(paths));
}

LRESULT CALLBACK Nova::Window::WindowProc(HWND win, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
    NV_PROFILE_FUNC;

    auto *window = reinterpret_cast<Window *>(GetWindowLongPtrA(win, 0));
    return window->HandleMessage(win, msg, wParam, lParam);
}

LRESULT Nova::Window::HandleMessage(HWND win, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return FALSE;
    case WM_SIZE:
        eventSystem_.InvokeEvent<WindowResizeEvent>(LOWORD(lParam), HIWORD(lParam));
        return FALSE;
    case WM_MOVE:
        eventSystem_.InvokeEvent<WindowMoveEvent>(LOWORD(lParam), HIWORD(lParam));
        return FALSE;
    case WM_CLOSE:
        data_.shouldClose = true;
        eventSystem_.InvokeEvent<WindowCloseEvent>(0.0);
        return FALSE;
    case WM_KILLFOCUS:
        eventSystem_.InvokeEvent<WindowFocusEvent>(false);
        return FALSE;
    case WM_SETFOCUS:
        eventSystem_.InvokeEvent<WindowFocusEvent>(true);
        return FALSE;
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_XBUTTONDOWN:
        HandleButtonDown(eventSystem_, msg, wParam);
        return msg == WM_XBUTTONDOWN;
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_XBUTTONUP:
        HandleButtonUp(eventSystem_, msg, wParam);
        return msg == WM_XBUTTONUP;
    case WM_MOUSEMOVE:
        HandleMouseMove(eventSystem_, lParam);
        return FALSE;
    case WM_MOUSEWHEEL:
        HandleScroll(eventSystem_, GET_WHEEL_DELTA_WPARAM(wParam) / 120.0, 0.0);
        return TRUE;
    case WM_MOUSEHWHEEL:
        HandleScroll(eventSystem_, 0.0, GET_WHEEL_DELTA_WPARAM(wParam) / 120.0);
        return TRUE;
    case WM_KEYUP:
        HandleKeyUp(eventSystem_, wParam, (lParam & (1 << 24)) == lParam);
        return FALSE;
    case WM_KEYDOWN:
        HandleKeyDown(eventSystem_, wParam, (lParam & (1 << 24)) == lParam, (lParam & 0xFFFF) > 1);
        return FALSE;
    case WM_CHAR:
        HandleChar(wParam);
        return FALSE;
    case WM_DROPFILES:
        HandleDropFiles(eventSystem_, reinterpret_cast<HDROP>(wParam));
        return FALSE;
    default:
        return DefWindowProcA(win, msg, wParam, lParam);
    }
}

Nova::Window::Window(
    const WindowSettings &settings,
    const StartupData &startupData,
    EventSystem &eventSystem)
    : eventSystem_(eventSystem)
{
    NV_PROFILE_FUNC;

    LoadWGL(startupData.exeInstance);

    data_.instance = startupData.exeInstance;

    data_.wndClass = WindowClass(
        WNDCLASSEXA{
            .cbSize = sizeof(WNDCLASSEXA),
            .style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC | CS_DBLCLKS,
            .lpfnWndProc = WindowProc,
            .cbWndExtra = sizeof(WindowData),
            .lpszClassName = cWindowClassName,
        });

    data_.handle = WindowHandle(
        GetStyleEx(settings.Flags),
        data_.wndClass,
        settings.Title,
        GetStyle(settings.Flags),
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        settings.Width,
        settings.Height,
        startupData.exeInstance);

    data_.deviceContext = DeviceContext(data_.handle);

    const auto formatAttribs = std::array<int, 17>({
        WGL_DRAW_TO_WINDOW_ARB,
        GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB,
        GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,
        GL_TRUE,
        WGL_PIXEL_TYPE_ARB,
        WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB,
        24,
        WGL_ALPHA_BITS_ARB,
        8,
        WGL_DEPTH_BITS_ARB,
        24,
        WGL_STENCIL_BITS_ARB,
        8,
        0,
    });
    SelectPixelFormat(data_.deviceContext, formatAttribs);

    const auto contextAttribs = std::array<int, 9>({
        WGL_CONTEXT_MAJOR_VERSION_ARB,
        cContextVersionMajor,
        WGL_CONTEXT_MINOR_VERSION_ARB,
        cContextVersionMinor,
        WGL_CONTEXT_FLAGS_ARB,
        cContextFlags,
        WGL_CONTEXT_PROFILE_MASK_ARB,
        WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0,
    });
    data_.wglContext = WGLContext(data_.deviceContext, contextAttribs);
    data_.wglContext.MakeCurrent(data_.deviceContext);

    SetWindowLongPtrA(data_.handle, 0, reinterpret_cast<LONG_PTR>(this));
}

void *Nova::Window::GetNativeHandle() noexcept
{
    return data_.handle.Get();
}

const void *Nova::Window::GetNativeHandle() const noexcept
{
    return data_.handle.Get();
}

int Nova::Window::GetWidth() const noexcept
{
    return WinRect::Window(data_.handle).GetWidth();
}

int Nova::Window::GetHeight() const noexcept
{
    return WinRect::Window(data_.handle).GetHeight();
}

std::pair<int, int> Nova::Window::GetSize() const noexcept
{
    return WinRect::Window(data_.handle).GetSize();
}

float Nova::Window::GetAspectRatio() const noexcept
{
    const auto [w, h] = GetSize();
    return w / static_cast<float>(h);
}

int Nova::Window::GetClientWidth() const noexcept
{
    return WinRect::Client(data_.handle).GetWidth();
}

int Nova::Window::GetClientHeight() const noexcept
{
    return WinRect::Client(data_.handle).GetHeight();
}

std::pair<int, int> Nova::Window::GetClientSize() const noexcept
{
    return WinRect::Client(data_.handle).GetSize();
}

float Nova::Window::GetClientAspectRatio() const noexcept
{
    const auto [w, h] = GetClientSize();
    return w / static_cast<float>(h);
}

int Nova::Window::GetX() const noexcept
{
    return WinRect::Window(data_.handle).GetX();
}

int Nova::Window::GetY() const noexcept
{
    return WinRect::Window(data_.handle).GetY();
}

std::pair<int, int> Nova::Window::GetPosition() const noexcept
{
    return WinRect::Window(data_.handle).GetPosition();
}

std::string Nova::Window::GetTitle() const noexcept
{
    const auto length = GetWindowTextLengthA(data_.handle);
    if (length == 0)
        return {};

    std::string title(length, '\0');
    GetWindowTextA(data_.handle, title.data(), length);

    return title;
}

bool Nova::Window::ShouldClose() const noexcept
{
    return data_.shouldClose;
}

Nova::FullscreenMode Nova::Window::GetFullscreenMode() const noexcept
{
    return data_.fsMode;
}

bool Nova::Window::IsFullscreen() const noexcept
{
    return GetFullscreenMode() != FullscreenMode::Windowed;
}

bool Nova::Window::IsWindowed() const noexcept
{
    return !IsFullscreen();
}

bool Nova::Window::IsVisible() const noexcept
{
    return !IsIconic(data_.handle);
}

void Nova::Window::Resize(int width, int height) noexcept
{
    NV_PROFILE_FUNC;

    SetWindowPos(
        data_.handle,
        nullptr,
        0,
        0,
        width,
        height,
        SWP_NOREPOSITION | SWP_NOZORDER);
}

void Nova::Window::Move(int x, int y) noexcept
{
    NV_PROFILE_FUNC;

    SetWindowPos(
        data_.handle,
        nullptr,
        x,
        y,
        0,
        0,
        SWP_NOSIZE | SWP_NOZORDER);
}

void Nova::Window::Close() noexcept
{
    NV_PROFILE_FUNC;

    CloseWindow(data_.handle);
}

void Nova::Window::SetTitle(const std::string_view title) noexcept
{
    NV_PROFILE_FUNC;

    SetWindowTextA(data_.handle, title.data());
}

void Nova::Window::SetTitle(const std::wstring_view title) noexcept
{
    NV_PROFILE_FUNC;

    SetWindowTextW(data_.handle, title.data());
}

void Nova::Window::Maximize() noexcept
{
    NV_PROFILE_FUNC;

    ShowWindow(data_.handle, SW_MAXIMIZE);
}

void Nova::Window::Minimize() noexcept
{
    NV_PROFILE_FUNC;

    ShowWindow(data_.handle, SW_MINIMIZE);
}

void Nova::Window::SwapBuffers() noexcept
{
    NV_PROFILE_FUNC;

    ::SwapBuffers(data_.deviceContext);
}

void Nova::Window::ProcessEvents()
{
    NV_PROFILE_FUNC;

    MSG msg{};
    BOOL pmResult{};
    while ((pmResult = PeekMessageA(&msg, data_.handle, 0, 0, PM_REMOVE)) != 0)
    {
        if (pmResult == -1)
            throw std::runtime_error("An error ocurred during window message handling.");

        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
}

void Nova::Window::SetIcon(const std::filesystem::path &filepath)
{
    NV_PROFILE_FUNC;

    data_.userIcon = Icon(filepath, data_.instance);
    SendSetIcon(data_.handle, data_.userIcon.value());
}

void Nova::Window::SetIcon(int width, int height, std::span<const std::byte> data)
{
    NV_PROFILE_FUNC;

    data_.userIcon = Icon(width, height, data);
    SendSetIcon(data_.handle, data_.userIcon.value());
}

void Nova::Window::SetCursor(const std::filesystem::path &filepath)
{
    NV_PROFILE_FUNC;

    data_.userCursor = Cursor(filepath, data_.instance);
    ::SetCursor(data_.userCursor.value());
}

void Nova::Window::SetCursorCaptured(bool captured) noexcept
{
    auto rect = WinRect::Window(data_.handle);
    ClipCursor(captured ? &rect.rect : nullptr);
}

void Nova::Window::SetFullscreen(FullscreenMode mode) noexcept
{
    NV_PROFILE_FUNC;

    if (mode == data_.fsMode)
        return;

    if (data_.fsMode == FullscreenMode::Windowed)
    {
        data_.savedMaximizedState = IsZoomed(data_.handle);
        if (data_.savedMaximizedState)
            ShowWindow(data_.handle, SW_RESTORE);

        data_.savedStyle = GetWindowLong(data_.handle, GWL_STYLE);
        data_.savedStyleEx = GetWindowLong(data_.handle, GWL_EXSTYLE);

        data_.savedRect = WinRect::Window(data_.handle);
    }

    data_.fsMode = mode;

    if (data_.fsMode == FullscreenMode::Fullscreen)
    {
        SetWindowLong(
            data_.handle,
            GWL_STYLE,
            data_.savedStyle & ~(WS_CAPTION | WS_THICKFRAME));
        SetWindowLong(
            data_.handle,
            GWL_STYLE,
            data_.savedStyleEx & ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));

        MONITORINFO monitorInfo{
            .cbSize = sizeof(MONITORINFO),
        };
        GetMonitorInfo(MonitorFromWindow(data_.handle, MONITOR_DEFAULTTONEAREST), &monitorInfo);
        SetWindowPos(
            data_.handle,
            nullptr,
            monitorInfo.rcMonitor.left,
            monitorInfo.rcMonitor.top,
            monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
            monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
            SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    }
    else if (data_.fsMode == FullscreenMode::Windowed)
    {
        SetWindowLong(data_.handle, GWL_STYLE, data_.savedStyle);
        SetWindowLong(data_.handle, GWL_STYLE, data_.savedStyleEx);

        SetWindowPos(
            data_.handle,
            nullptr,
            data_.savedRect.GetY(),
            data_.savedRect.GetX(),
            data_.savedRect.GetWidth(),
            data_.savedRect.GetHeight(),
            SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

        if (data_.savedMaximizedState)
            ShowWindow(data_.handle, SW_MAXIMIZE);
    }
    else
    {
        // TODO Implement fullscreen borderless
    }
}