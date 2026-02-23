using System.Runtime.InteropServices;

namespace NovaHost;

[StructLayout(layoutKind: LayoutKind.Sequential)]
internal readonly struct AssemblyType
{
    public nint Name { get; init; }
    public nint FullName { get; init; }
    public nint AssemblyQualifiedName { get; init; }
    public int ID { get; init; }
}