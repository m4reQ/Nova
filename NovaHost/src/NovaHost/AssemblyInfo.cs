using System.Runtime.InteropServices;

namespace NovaHost;

[StructLayout(layoutKind: LayoutKind.Sequential)]
internal readonly struct AssemblyInfo
{
    public nint Name { get; init; }
    public nint FullName { get; init; }
    public nint Filepath { get; init; }
    public int ID { get; init; }

    public static AssemblyInfo Invalid => new()
    {
        Name = 0,
        FullName = 0,
        Filepath = 0,
        ID = 0,
    };
}