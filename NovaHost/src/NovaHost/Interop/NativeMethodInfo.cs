using System;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Security.AccessControl;

namespace NovaHost.Interop;

internal readonly struct NativeMethodInfo
{
    public nint Name { get; init; }
    public nint MethodPtr { get; init; }
    public int Attributes { get; init; }
    public ulong ArgsCount { get; init; }
    public NativeBool ReturnsValue { get; init; }

    public NativeMethodInfo(MethodInfo info)
    {
        Name = Marshal.StringToHGlobalAnsi(info.Name);
        ArgsCount = (ulong)info.GetParameters().Length;
        ReturnsValue = info.ReturnType != typeof(void);

        var paramsAndReturnType = info.GetParameters()
            .Select(x => x.ParameterType)
            .Append(info.ReturnType)
            .ToArray();
        var exprType = Expression.GetDelegateType(paramsAndReturnType);
        var delegateInstance = Delegate.CreateDelegate(exprType, info);

        //MethodPtr = info.MethodHandle.GetFunctionPointer();
        //GCHandle.Alloc(delegateInstance, GCHandleType.Pinned); // TODO Store those somewhere and dispose when unloading context

        //var methodType = Type.GetType($"{info.DeclaringType.FullName}+{info.Name}");
        //var delegateInstance = Delegate.CreateDelegate(methodType, info);
        //MethodPtr = Marshal.GetFunctionPointerForDelegate(delegateInstance);
    }
}
