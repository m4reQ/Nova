using System;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace NovaHost.Interop;

internal readonly struct NativeType
{
    public nint Name { get; init; }
    public nint FullName { get; init; }
    public nint AssemblyQualifiedName { get; init; }
    public nint BaseType { get; init; }
    public nint ElementType { get; init; }
    public nint Methods { get; init; } = 0;
    public nint Properties { get; init; } = 0;
    public ulong Size { get; init; }
    public ulong ElementSize { get; init; } = 0;
    public int Attributes { get; init; }
    public NativeBool IsSZArray { get; init; }
    public NativeBool IsStatic { get; init; }

    public NativeType(Type type, bool retrieveMethods)
    {
        Name = Marshal.StringToHGlobalAnsi(type.Name);
        FullName = Marshal.StringToHGlobalAnsi(type.FullName);
        AssemblyQualifiedName = Marshal.StringToHGlobalAnsi(type.AssemblyQualifiedName);

        if (type.BaseType is not null && type.BaseType != typeof(object))
        {
            BaseType = Marshal.AllocHGlobal(Marshal.SizeOf<NativeType>());
            Marshal.StructureToPtr(new NativeType(type.BaseType, retrieveMethods), BaseType, false);
        }
        else
        {
            BaseType = 0;
        }

        if (type.HasElementType)
        {
            ElementType = Marshal.AllocHGlobal(Marshal.SizeOf<NativeType>());
            Marshal.StructureToPtr(new NativeType(type.GetElementType()!, retrieveMethods), ElementType, false);
        }
        else
        {
            ElementType = 0;
        }

        Size = (type.IsAbstract && type.IsSealed) 
            ? 0
            : (ulong)Marshal.SizeOf(type);
        ElementSize = type.HasElementType
            ? (ulong)Marshal.SizeOf(type.GetElementType()!)
            : 0;
        Attributes = (int)type.Attributes;
        IsSZArray = type.IsSZArray;
        IsStatic = type.IsAbstract && type.IsSealed;

        if (retrieveMethods)
        {
            // omit inherited methods (for now)
            var methods = type.GetMethods().Where(x => x.DeclaringType == type).ToArray();
            var methodsCount = methods.Length;

            Methods = Marshal.AllocHGlobal(methodsCount * Marshal.SizeOf<NativeMethodInfo>());
            for (int i = 0; i < methodsCount; i++)
                Marshal.StructureToPtr(
                    new NativeMethodInfo(methods[i]),
                    Methods + i * Marshal.SizeOf<NativeMethodInfo>(),
                    false);
        }
        else
        {
            Methods = 0;
        }
        
        // public nint Properties { get; init; } = 0;
    }
}
