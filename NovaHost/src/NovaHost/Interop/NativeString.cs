using System;
using System.Runtime.InteropServices;

namespace NovaHost.Interop;

public enum Encoding
{
    Auto = 0,
    Ansi = 1,
    Unicode = 2,
}

public struct NativeString : IDisposable, IEquatable<NativeString>
{
    public IntPtr Data { get; private set; }
    private NativeBool ShouldDispose { get; set; }
    private NativeBool IsDisposed { get; set; } = false;

    public NativeString(IntPtr data)
    {
        Data = data;
        ShouldDispose = false;
    }

    public NativeString(string? data, Encoding encoding = Encoding.Ansi)
    {
        Data = StringToHGlobal(data, encoding);
        ShouldDispose = true;
    }

    public readonly string? GetString(Encoding encoding = Encoding.Ansi)
    {
        return PtrToString(Data, encoding);
    }

    public readonly void Dispose()
    {
        if (!ShouldDispose || IsDisposed)
            return;

        Marshal.FreeHGlobal(Data);
        GC.SuppressFinalize(this);
    }

    public readonly bool Equals(NativeString other)
    {
        if (Data == other.Data)
            return true;

        unsafe
        {
            char* s1 = (char*)Data.ToPointer();
            char* s2 = (char*)other.Data.ToPointer();

            while (*s1 != 0 && (*s1 == *s2))
            {
                s1++;
                s2++;
            }

            return *s1 == *s2;
        }
    }

    public readonly bool Equals(ReadOnlySpan<char> other)
    {
        unsafe
        {
            int i = 0;
            foreach (var a in other)
            {
                var b = ((char*)Data.ToPointer())[i];
                if (b == '\0' || a != b)
                    return false;

                i++;
            }
        }

        return true;
    }

    // From: https://stackoverflow.com/a/7666668
    public override readonly int GetHashCode()
    {
        int result = 0x55555555;
        unsafe
        {
            char* str = (char*)Data.ToPointer();

            while (*str != '\0')
            {
                result ^= *str++;
                result = (result << 5) | (result >> (32 - 5));
            }
        }

        return result;
    }

    public readonly bool IsNull => Data == IntPtr.Zero;

    public static NativeString Empty => new(IntPtr.Zero);

    private static IntPtr StringToHGlobal(string data, Encoding encoding)
    {
        return encoding switch
        {
            Encoding.Ansi => Marshal.StringToHGlobalAnsi(data),
            Encoding.Auto => Marshal.StringToHGlobalAuto(data),
            Encoding.Unicode => Marshal.StringToHGlobalUni(data),
            _ => throw new Exception($"Invalid encoding value: {encoding}."),
        };
    }

    private static string? PtrToString(IntPtr data, Encoding encoding)
    {
        return encoding switch
        {
            Encoding.Ansi => Marshal.PtrToStringAnsi(data),
            Encoding.Auto => Marshal.PtrToStringAuto(data),
            Encoding.Unicode => Marshal.PtrToStringUni(data),
            _ => throw new Exception($"Invalid encoding value: {encoding}."),
        };
    }
}
