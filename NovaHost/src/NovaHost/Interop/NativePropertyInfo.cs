using System.Reflection;
using System.Runtime.InteropServices;

namespace NovaHost.Interop;

internal readonly struct NativePropertyInfo
{
    nint Name { get; init; }
    int ID { get; init; }
    int TypeID { get; init; }
    Accessibility Accessibility { get; init; }

    public NativePropertyInfo(PropertyInfo info)
    {
        Name = Marshal.StringToHGlobalAnsi(info.Name);
        ID = info.GetHashCode();
        TypeID = info.PropertyType.GetHashCode();
        Accessibility = GetPropertyAccessibility(info);
    }

    public NativePropertyInfo(PropertyInfo info, int id)
    {
        Name = Marshal.StringToHGlobalAnsi(info.Name);
        ID = id;
        TypeID = info.PropertyType.GetHashCode();
        Accessibility = GetPropertyAccessibility(info);
    }

    private static Accessibility GetPropertyAccessibility(PropertyInfo info)
    {
        var attributes = info.GetGetMethod().Attributes;

        if (attributes.HasFlag(MethodAttributes.Private))
            return Accessibility.Private;

        if (attributes.HasFlag(MethodAttributes.Public))
            return Accessibility.Public;

        if (attributes.HasFlag(MethodAttributes.Family))
            return Accessibility.Protected;

        if (attributes.HasFlag(MethodAttributes.Assembly))
            return Accessibility.Internal;

        return Accessibility.Unknown;
    }
}
