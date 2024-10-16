using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Loader;
using NovaHost.Interop;

namespace NovaHost;

internal struct AssemblyInfo
{
    NativeString Name { get; set; }
    NativeString FullName { get; set; }

}

internal static class AssemblyLoader
{
    private static readonly Dictionary<NativeString, AssemblyLoadContext> LoadContexts = new()
    {
        { new("default"), AssemblyLoadContext.Default },
    };
    private static readonly Dictionary<int, Assembly> Assemblies = new();

    [UnmanagedCallersOnly]
    private static int LoadAssemblyFromFilepath(NativeString filepath, NativeString contextName)
    {
        try
        {
            ThrowIfNullOrEmpty(filepath, "Filepath cannot be null.", nameof(filepath));
            ThrowIfNullOrEmpty(contextName, "Context name cannot be null.", nameof(contextName));

            if (!LoadContexts.TryGetValue(contextName, out var ctx))
            {
                ctx = CreateLoadContext(contextName);
            }

            // TODO remove allocation by using custom mapped file with constructor accepting IntPtr
            var filepathStr = Path.GetFullPath(filepath.GetString(Encoding.Unicode)!);

            var assembly = ctx.LoadFromAssemblyPath(filepathStr);
            var assemblyId = assembly.GetHashCode();

            Assemblies.Add(assemblyId, assembly);

            ManagedHost.LogMessage(LogLevel.Info, $"Loaded assembly from filepath {filepathStr}. Assembly id: {assemblyId}");

            return assemblyId;
        }
        catch (Exception exc)
        {
            ManagedHost.HandleException(exc);
            return -1;
        }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static void UnloadContext(NativeString contextName)
    {
        try
        {
            ThrowIfNullOrEmpty(contextName, "Context name cannot be null.", nameof(contextName));

            if (!LoadContexts.TryGetValue(contextName, out var ctx))
            {
                ManagedHost.LogMessage(LogLevel.Error, $"Couldn't find context named {contextName.GetString(Encoding.Ansi)}.");
                return;
            }

            ctx.Unload();

            ManagedHost.LogMessage(LogLevel.Info, $"Unloaded assembly load context named {contextName.GetString()}");
        }
        catch (Exception exc)
        {
            ManagedHost.HandleException(exc);
        }
    }

    [UnmanagedCallersOnly]
    private static int GetAssemblyID(NativeString assemblyName, NativeString contextName)
    {
        try
        {
            ThrowIfNullOrEmpty(assemblyName, "Assembly name cannot be null.", nameof(assemblyName));
            ThrowIfNullOrEmpty(contextName, "Context name cannot be null.", nameof(contextName));

            if (!LoadContexts.TryGetValue(contextName, out var ctx))
                throw new Exception($"Context named {contextName.GetString()} not found.");

            var assembly = ctx.Assemblies.First(x => contextName.Equals(x.GetName().Name.AsSpan()));
            return assembly.GetHashCode();
        }
        catch (Exception exc)
        {
            ManagedHost.HandleException(exc);
            return -1;
        }
    }

    [UnmanagedCallersOnly]
    private static NativeString GetAssemblyName(int assemblyId)
    {
        try
        {
            if (!Assemblies.TryGetValue(assemblyId, out var assembly))
                throw new Exception($"Assembly with id {assemblyId} not found.");

            return new(assembly.GetName().Name);
        }
        catch (Exception exc)
        {
            ManagedHost.HandleException(exc);
            return NativeString.Empty;
        }
    }

    [UnmanagedCallersOnly]
    private static NativeString GetAssemblyFullName(int assemblyId)
    {
        try
        {
            if (!Assemblies.TryGetValue(assemblyId, out var assembly))
                throw new Exception($"Assembly with id {assemblyId} not found.");

            return new(assembly.GetName().FullName);
        }
        catch (Exception exc)
        {
            ManagedHost.HandleException(exc);
            return NativeString.Empty;
        }
    }

    [UnmanagedCallersOnly]
    private static void ReloadContext(NativeString contextName)
    {
        try
        {
            ThrowIfNullOrEmpty(contextName, "Context name cannot be null.", nameof(contextName));

            if (!LoadContexts.TryGetValue(contextName, out var ctx))
            {
                ManagedHost.LogMessage(LogLevel.Error, $"Couldn't find load context named {contextName.GetString(Encoding.Ansi)}");
                return;
            }

            if (!ctx.IsCollectible)
                throw new Exception("Context is not collectible.");

            var assemblyPaths = ctx.Assemblies
                .Select(x => x.Location)
                .ToArray();
            var assemblyRefs = ctx.Assemblies
                .Select(x => new WeakReference(x))
                .ToArray();

            ctx.Unload();

            foreach (var assemblyRef in assemblyRefs)
            {
                Assemblies.Remove(assemblyRef.Target.GetHashCode());

                while (assemblyRef.IsAlive) { }

                GC.Collect();
                GC.WaitForPendingFinalizers();
            }

            foreach (var assemblyPath in assemblyPaths)
            {
                var assembly = ctx.LoadFromAssemblyPath(assemblyPath);
                Assemblies.Add(assembly.GetHashCode(), assembly);
            }
        }
        catch (Exception exc)
        {
            ManagedHost.HandleException(exc);
        }
    }

    private static AssemblyLoadContext CreateLoadContext(string name)
    {
        return new(name, true);
    }

    private static AssemblyLoadContext CreateLoadContext(NativeString name)
    {
        return new(name.GetString(Encoding.Ansi), true);
    }

    private static void ThrowIfNullOrEmpty(NativeString str, string? msg, string? argName)
    {
        if (str.IsNull)
            throw new ArgumentException(msg, argName);
    }
}
