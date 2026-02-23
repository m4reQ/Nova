using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using NovaHost.Interop;

namespace NovaHost;

internal static class AssemblyLoader
{
    static private readonly Dictionary<int, Assembly> AssembliesCache = [];

    [UnmanagedCallersOnly]
    private static unsafe nint GetAssemblyTypes(int assemblyID)
    {
        ManagedHost.LogTrace($"Getting types from assembly with ID {assemblyID}...");

        if (!AssembliesCache.TryGetValue(assemblyID, out Assembly? assembly))
        {
            ManagedHost.LogError($"Failed to find any cached assembly with for ID {assemblyID}.");
            return 0;
        }

        ManagedHost.LogTrace($"Found cached assembly \"{assembly.GetName().Name}\".");

        var types = assembly.GetExportedTypes()
            .Select(
                x => new AssemblyType
                {
                    Name = Marshal.StringToHGlobalAnsi(x.Name),
                    FullName = Marshal.StringToHGlobalAnsi(x.FullName),
                    AssemblyQualifiedName = Marshal.StringToHGlobalAnsi(x.AssemblyQualifiedName),
                    ID = x.GetHashCode(),
                })
            .ToArray();

        if (types.Length == 0)
        {
            ManagedHost.LogWarning("Didn't find any exported types for the given assembly.");
        }
        else
        {
            var suffix = types.Length > 1 ? "s" : "";
            ManagedHost.LogTrace($"Found {types.Length} exported type{suffix}.");
        }

        var bufferSize = Marshal.SizeOf<AssemblyType>() * types.Length;
        var buffer = Marshal.AllocHGlobal(bufferSize);
        fixed (AssemblyType* typesPtr = types)
        {
            Buffer.MemoryCopy(
                typesPtr,
                buffer.ToPointer(),
                bufferSize,
                bufferSize);
        }

        return buffer;
    }

    [UnmanagedCallersOnly]
    private static AssemblyInfo LoadAssemblyFromFilepath(NativeString filepath)
    {
        var managedFilepath = Path.GetFullPath(filepath.GetString(Encoding.Unicode));

        Assembly? assembly;
        try
        {
            assembly = Assembly.LoadFrom(managedFilepath);
            if (assembly is null)
            {
                return AssemblyInfo.Invalid;
            }
        }
        catch (Exception)
        {
            return AssemblyInfo.Invalid;
        }

        AssembliesCache.Add(assembly.GetHashCode(), assembly);

        return new AssemblyInfo
        {
            Name = Marshal.StringToHGlobalAnsi(assembly.GetName().Name),
            FullName = Marshal.StringToHGlobalAnsi(assembly.GetName().FullName),
            Filepath = Marshal.StringToHGlobalAnsi(managedFilepath),
            ID = assembly.GetHashCode(),
        };
    }
}
