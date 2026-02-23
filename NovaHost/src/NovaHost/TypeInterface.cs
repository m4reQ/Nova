using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using NovaHost.Interop;

namespace NovaHost;

internal struct InternalCallUpload
{
    public nint NamePtr { get; }
    public nint FuncPtr { get; }
}

internal class TypeInterface
{
    private static readonly ObjectCache<Type> TypeCache = new();
    private static readonly Dictionary<int, MethodInfo> MethodCache = new();
    private static readonly ObjectCache<PropertyInfo> PropertyCache = new();

    [UnmanagedCallersOnly]
    private static NativeString GetTypeName(int id)
    {
        return TypeCache.TryGetValue(id, out var type)
            ? new(type.Name)
            : NativeString.Empty;
    }

    [UnmanagedCallersOnly]
    private static NativeString GetTypeFullName(int id)
    {
        return TypeCache.TryGetValue(id, out var type)
            ? (type.FullName is null ? NativeString.Empty : new(type.FullName))
            : NativeString.Empty;
    }

    [UnmanagedCallersOnly]
    private static NativeString GetTypeAssemblyQualifiedName(int id)
    {
        return TypeCache.TryGetValue(id, out var type)
            ? (type.AssemblyQualifiedName is null ? NativeString.Empty : new(type.AssemblyQualifiedName))
            : NativeString.Empty;
    }

    [UnmanagedCallersOnly]
    private static unsafe NativeBool UploadInternalCalls(int id, InternalCallUpload* uploadData, int uploadsCount)
    {
        if (uploadData is null || uploadsCount == 0)
        {
            ManagedHost.LogMessage(LogLevel.Info, $"{nameof(UploadInternalCalls)}: {nameof(uploadData)} is null or {nameof(uploadsCount)} is 0. Skipping...");
            return true;
        }

        if (!TypeCache.TryGetValue(id, out var type))
        {
            ManagedHost.LogMessage(LogLevel.Error, $"{nameof(UploadInternalCalls)}: Type with id {id} was not found.");
            return false;
        }

        for (int i = 0; i < uploadsCount; i++)
        {
            var upload = uploadData[i];
            if (upload.NamePtr == 0 || upload.FuncPtr == 0)
            {
                ManagedHost.LogMessage(LogLevel.Warning, $"{nameof(UploadInternalCalls)}: Internal calls with null method name or function pointer are not allowed. Skipping...");
                continue;
            }

            var fieldName = Marshal.PtrToStringAnsi(upload.NamePtr)!;
            var field = type.GetField(fieldName, BindingFlags.Static | BindingFlags.Public | BindingFlags.NonPublic);
            if (field is null)
            {
                ManagedHost.LogMessage(LogLevel.Error, $"{nameof(UploadInternalCalls)}: Failed to find field {type.Name}.{fieldName} for internal call upload. Internal call not uploaded.");
                continue;
            }

            if (!field.FieldType.IsUnmanagedFunctionPointer)
            {
                ManagedHost.LogMessage(LogLevel.Error, $"{nameof(UploadInternalCalls)}: Field is not an unmanaged function pointer type. Internal call not uploaded.");
                continue;
            }

            try
            {
                field.SetValue(null, upload.FuncPtr);
            }
            catch (Exception ex)
            {
                ManagedHost.LogMessage(LogLevel.Error, $"{nameof(UploadInternalCalls)}: Internal error occurred while uploading internal call.");
                ManagedHost.HandleException(ex);
                return false;
            }

            ManagedHost.LogMessage(LogLevel.Trace, $"{nameof(UploadInternalCalls)}: Uploaded internal call \"{fieldName}\".");
        }

        return true;
    }

    [UnmanagedCallersOnly]
    private static nint GetTypeElementType(int id)
    {
        if (!TypeCache.TryGetValue(id, out var type))
            return -1;

        return type.HasElementType
            ? CacheType(type.GetElementType())
            : 0;
    }

    [UnmanagedCallersOnly]
    private static int GetTypeBaseType(int id)
    {
        if (!TypeCache.TryGetValue(id, out var type))
            return -1;

        return type.BaseType is not null
            ? CacheType(type.BaseType)
            : 0;
    }

    [UnmanagedCallersOnly]
    private static NativeBool InvokeStaticMethod(int methodHash, nint resultPtr, nint argsPtr)
    {
        if (!MethodCache.TryGetValue(methodHash, out var method))
        {
            LogMethodNotFound(methodHash);
            return false;
        }


        try
        {
            var result = method.Invoke(null, MarshalArgsArray(argsPtr, method.GetParameters()));
            if (result != null)
            {
                // TODO: Implement
                //MarshalObjectToPtr(result, resultPtr);
            }
        }
        catch (Exception ex)
        {
            ManagedHost.HandleException(ex);
        }

        return true;
    }

    [UnmanagedCallersOnly]
    internal static nint CreateInstance(int typeID, nint argsPtr)
    {
        if (!TypeCache.TryGetValue(typeID, out var type))
        {
            LogTypeNotFound(typeID);
            return 0;
        }
        // Activator.CreateInstance(type, )

        // TODO: Implement
        return 0;
    }

    internal static int CacheType(Type type)
    {
        if (!TypeCache.TryAdd(type, out var id))
        {
            ManagedHost.LogMessage(LogLevel.Warning, $"Type {type.Name} already in cache.");
            return id;
        }

        foreach (var method in type.GetMethods())
        {
            var methodHash = id ^ ComputeMethodHash(method);
            MethodCache.Add(methodHash, method);
        }

        return id;
    }

    internal static int ComputeMethodHash(MethodInfo info)
    {
        int hash = ComputeStringHash(info.Name);
        hash ^= ComputeTypeHash(info.ReturnType);
        var parameters = info.GetParameters();
        for (var i = parameters.Length; i >= 0; i--)
        {
            hash ^= ComputeTypeHash(parameters[i].ParameterType);
        }

        return hash;
    }

    internal static int ComputeStringHash(ReadOnlySpan<char> str)
    {
        int hash = 5381;
        foreach (char c in str)
            hash = (byte)c + 33 * hash;

        return hash;
    }

    internal static int ComputeTypeHash(Type type)
    {
        if (type == typeof(sbyte))
            return 3;
        if (type == typeof(byte))
            return 7;
        if (type == typeof(short))
            return 11;
        if (type == typeof(ushort))
            return 13;
        if (type == typeof(int))
            return 17;
        if (type == typeof(uint))
            return 19;
        if (type == typeof(long))
            return 23;
        if (type == typeof(ulong))
            return 29;
        if (type == typeof(bool))
            return 31;
        if (type == typeof(float))
            return 37;
        if (type == typeof(double))
            return 41;
        if (type == typeof(string))
            return 47;

        return 0;
    }

    internal static object?[] MarshalArgsArray(nint argsPtr, ParameterInfo[] argTypes)
    {
        var args = new object?[argTypes.Length];

        for (int i = 0; i < argTypes.Length; i++)
        {
            var argPtr = Marshal.ReadIntPtr(argsPtr + Marshal.SizeOf<nint>() * i);
            args[i] = MarshalObjectFromPtr(argPtr, argTypes[i].ParameterType);
        }

        return args;
    }

    internal static object? MarshalObjectFromPtr(nint ptr, Type type)
    {
        if (type == typeof(string))
            return Marshal.PtrToStringAnsi(ptr);
        if (type == typeof(bool))
            return Marshal.ReadByte(ptr) != 0;
        if (type == typeof(byte))
            return Marshal.ReadByte(ptr);
        if (type == typeof(sbyte))
            return (sbyte)Marshal.ReadByte(ptr);
        if (type == typeof(short))
            return Marshal.ReadInt16(ptr);
        if (type == typeof(ushort))
            return (ushort)Marshal.ReadInt16(ptr);
        if (type == typeof(int))
            return Marshal.ReadInt32(ptr);
        if (type == typeof(uint))
            return (uint)Marshal.ReadInt32(ptr);
        if (type == typeof(long))
            return Marshal.ReadInt64(ptr);
        if (type == typeof(ulong))
            return (ulong)Marshal.ReadInt64(ptr);

        return Marshal.PtrToStructure(ptr, type);
    }

    internal static void MarshalArraytoPtr<T>(T[] array, nint outPtr)
    {
        var type = typeof(T);
        var elementType = type.GetElementType()!;

        if (elementType.IsEnum)
            elementType = elementType.GetEnumUnderlyingType()!;

        NativeArray storage;
        if (type.IsSZArray && elementType.IsValueType)
        {
            var handle = GCHandle.Alloc(array, GCHandleType.Pinned);
            var address = handle.AddrOfPinnedObject();
            storage = new NativeArray { Handle = handle, Address = address, Length = (ulong)array.Length };
        }
        else
        {
            var elementSize = Marshal.SizeOf(elementType);
            var address = Marshal.AllocHGlobal(elementSize * array.Length);

            for (int i = 0; i < array.Length; i++)
            {
                Marshal.StructureToPtr(array[i]!, address + (elementSize * i), false);
            }

            storage = new NativeArray { Handle = null, Address = address, Length = (ulong)array.Length };
        }

        Marshal.StructureToPtr(storage, outPtr, false);
    }

    internal static void MarshalObjectToPtr(object obj, Type objType, nint outPtr)
    {
        if (objType.IsEnum)
            objType = objType.GetEnumUnderlyingType();

        if (objType.IsSZArray)
        {
            var handle = GCHandle.Alloc(obj, GCHandleType.Pinned);
            var address = handle.AddrOfPinnedObject();
            Marshal.WriteIntPtr(outPtr, address);

            handle.Free();
        }

        if (objType == typeof(string))
            Marshal.WriteIntPtr(outPtr, Marshal.StringToHGlobalAnsi(Unsafe.As<string>(obj)));
        else if (objType == typeof(bool))
            Marshal.StructureToPtr(new NativeBool((bool)obj), outPtr, false);
        else if (objType == typeof(byte))
            Marshal.WriteByte(outPtr, (byte)obj);
        else if (objType == typeof(sbyte))
            Marshal.WriteByte(outPtr, (byte)(sbyte)obj);
        else if (objType == typeof(short))
            Marshal.WriteInt16(outPtr, (short)obj);
        else if (objType == typeof(ushort))
            Marshal.WriteInt16(outPtr, (short)(ushort)obj);
        else if (objType == typeof(int))
            Marshal.WriteInt32(outPtr, (int)obj);
        else if (objType == typeof(uint))
            Marshal.WriteInt32(outPtr, (int)(uint)obj);
        else if (objType == typeof(long))
            Marshal.WriteInt64(outPtr, (long)obj);
        else if (objType == typeof(ulong))
            Marshal.WriteInt64(outPtr, (long)(ulong)obj);
        else
            throw new Exception("Unsupported object type for unmarshalling.");
    }

    private static void LogTypeNotFound(int id)
    {
        ManagedHost.LogMessage(LogLevel.Error, $"Couldn't find type with id {id}.");
    }

    private static void LogMethodNotFound(int id)
    {
        ManagedHost.LogMessage(LogLevel.Error, $"Couldn't find method with id {id}.");
    }

    private static void LogFailedToInvokeMethod(MethodInfo method, string cause)
    {
        ManagedHost.LogMessage(LogLevel.Error, $"Failed to invoke method {method.DeclaringType.Name}.{method.Name}: {cause}.");
    }
}
