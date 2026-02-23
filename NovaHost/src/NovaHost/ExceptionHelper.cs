using System;

using NovaHost.Interop;

namespace NovaHost;

internal static class ExceptionHelper
{
    internal static void ThrowIfNullOrEmpty(NativeString str, string? msg, string? argName)
    {
        if (str.IsNull)
            throw new ArgumentException(msg, argName);
    }
}
