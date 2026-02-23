using System.Runtime.InteropServices;

namespace NovaScript;

public static partial class Window
{
    private static unsafe delegate* unmanaged<uint, uint, void> ResizeImpl;
    private static unsafe delegate* unmanaged<uint, uint, void> MoveImpl;
    private static unsafe delegate* unmanaged<nint, void> SetTitleImpl;
    private static unsafe delegate* unmanaged<void> CloseImpl;
    private static unsafe delegate* unmanaged<nint, void> SetIconImpl;
    private static unsafe delegate* unmanaged<byte, void> SetFullscreenImpl;
    private static unsafe delegate* unmanaged<void> MaximizeImpl;
    private static unsafe delegate* unmanaged<void> IconifyImpl;
    private static unsafe delegate* unmanaged<uint*, uint*, void> GetSizeImpl;
    private static unsafe delegate* unmanaged<uint*, uint*, void> GetFramebufferSizeImpl;
    private static unsafe delegate* unmanaged<uint*, uint*, void> GetPositionImpl;
    private static unsafe delegate* unmanaged<byte> IsVisibleImpl;

    public static void Resize(uint width, uint height)
    {
        unsafe
        {
            ResizeImpl(width, height);
        }
    }

    public static void Move(uint x, uint y)
    {
        unsafe
        {
            MoveImpl(x, y);
        }
    }

    public static void SetTitle(string title)
    {
        unsafe
        {
            fixed (char* ptr = title)
            {
                SetTitleImpl((nint)ptr);
            }
        }
    }

    public static void Close()
    {
        unsafe
        {
            CloseImpl();
        }
    }

    public static void SetIcon(string filepath)
    {
        unsafe
        {
            fixed (char* ptr = filepath)
            {
                SetIconImpl((nint)ptr);
            }
        }
    }

    public static void SetFullscreen(bool isEnabled)
    {
        unsafe
        {
            SetFullscreenImpl(isEnabled ? (byte)1 : (byte)0);
        }
    }

    public static void Maximize()
    {
        unsafe
        {
            MaximizeImpl();
        }
    }

    public static void Iconify()
    {
        unsafe
        {
            IconifyImpl();
        }
    }

    public static (uint width, uint height) GetSize()
    {
        uint width = 0;
        uint height = 0;

        unsafe
        {
            GetSizeImpl(&width, &height);
        }

        return (width, height);
    }

    public static (uint width, uint height) GetFramebufferSize()
    {
        uint width = 0;
        uint height = 0;

        unsafe
        {
            GetFramebufferSizeImpl(&width, &height);
        }

        return (width, height);
    }

    public static (uint x, uint y) GetPosition()
    {
        uint x = 0;
        uint y = 0;

        unsafe
        {
            GetPositionImpl(&x, &y);
        }

        return (x, y);
    }

    public static bool IsVisible()
    {
        unsafe
        {
            return IsVisibleImpl() != 0;
        }
    }
}
