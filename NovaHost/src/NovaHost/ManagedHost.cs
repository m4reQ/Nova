using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using NovaHost.Interop;

namespace NovaHost;

internal enum LogLevel
{
    Trace = 0,
    Info = 1,
    Warning = 2,
    Error = 3,
}

internal static class ManagedHost
{
    private static unsafe delegate*<int, NativeString, void> _logCallback;
    private static unsafe delegate*<NativeString, bool, void> _errorCallback;

    [UnmanagedCallersOnly]
    internal static unsafe void Initialize(delegate*<int, NativeString, void> logCallback, delegate*<NativeString, bool, void> errorCallback)
    {
        _logCallback = logCallback;
        _errorCallback = errorCallback;

        LogMessage(LogLevel.Info, ".NET Managed host initialized.");
    }

    [Conditional("DEBUG")]
    internal static void LogMessage(LogLevel lvl, string message)
    {
        unsafe
        {
            _logCallback((int)lvl, new(message));
        }
    }

    internal static void HandleException(Exception exc, bool isFatal = false)
    {
        unsafe
        {
            _errorCallback(new(exc.ToString()), isFatal);
        }
    }
}
