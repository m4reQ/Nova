using System.Reflection;
using System.Runtime.InteropServices;

namespace NovaHost.Interop;

internal readonly struct NativeAssembly
{
    // TODO Remove unnecessary properties and reintroduce methods to retrieve them from `AssemblyLoader`
    private nint Name { get; init; } = 0;
    private nint FullName { get; init; } = 0;
    private nint Filepath { get; init; } = 0;
    private long TypesCount { get; init; } = 0;
    private nint Types { get; init; } = 0;
    private NativeBool IsLoaded { get; init; }

    public static NativeAssembly Failed()
    {
        return new NativeAssembly { IsLoaded = false };
    }

    public NativeAssembly(Assembly assembly)
    {
        var _name = assembly.GetName();

        Name = Marshal.StringToHGlobalAnsi(_name.Name);
        FullName = Marshal.StringToHGlobalAnsi(_name.FullName);
        Filepath = Marshal.StringToHGlobalUni(assembly.Location);

        var types = assembly.GetTypes();

        TypesCount = types.Length;
        Types = Marshal.AllocHGlobal(types.Length * Marshal.SizeOf<NativeType>());

        var current = Types;
        foreach (var type in types)
        {
            Marshal.StructureToPtr(new NativeType(type, false), current, false);
            current += Marshal.SizeOf<NativeType>();
        }

        IsLoaded = true;
    }
}
