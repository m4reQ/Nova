using System;
using System.Runtime.InteropServices;
using NovaHost.Interop;

namespace NovaHost;

internal static class GarbageCollector
{
    [UnmanagedCallersOnly]
    private static void CollectGarbage(int generation, GCCollectionMode mode, NativeBool blocking, NativeBool compacting)
    {
        try
        {
            if (generation < 0)
                GC.Collect();
            else
                GC.Collect(generation, mode, blocking, compacting);
        }
        catch (Exception exc)
        {
            ManagedHost.HandleException(exc);
        }
    }

    [UnmanagedCallersOnly]
    private static void WaitForPendingFinalizers()
    {
        try
        {
            GC.WaitForPendingFinalizers();
        }
        catch (Exception exc)
        {
            ManagedHost.HandleException(exc);
        }
    }
}
