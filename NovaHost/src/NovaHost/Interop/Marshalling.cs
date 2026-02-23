using System;
using System.Reflection;
using System.Runtime.InteropServices;

namespace NovaHost.Interop;

internal static class Marshalling
{
    internal static void MarshalObject(object? obj, Type type, nint outPtr)
    {
        // where type switch
        if (type.IsPointer)
        {
            // TODO Can this just be done with Marshal.WriteIntPtr?
            // unsafe
            // {
            //     Marshal.WriteIntPtr(outPtr, obj is null ? 0 : (nint)Pointer.Unbox(obj));
            // }

            if (obj is null)
            {
                Marshal.WriteIntPtr(outPtr, 0);
            }
            else
            {
                unsafe
                {
                    void* objPtr = Pointer.Unbox(obj);
                    Buffer.MemoryCopy(&objPtr, outPtr.ToPointer(), nint.Size, nint.Size);
                }
            }
        }
        else if (type == typeof(char))
        {
            Marshal.WriteInt16(outPtr, (char)obj);
        }
        else if (type == typeof(byte) || type == typeof(sbyte))
        {
            Marshal.WriteByte(outPtr, (byte)obj);
        }
        else if (type == typeof(short) || type == typeof(ushort))
        {
            Marshal.WriteInt16(outPtr, (short)obj);
        }
        else if (type == typeof(int) || type == typeof(int))
        {
            Marshal.WriteInt32(outPtr, (int)obj);
        }
        else if (type == typeof(long))
        {
            Marshal.WriteInt64(outPtr, (long)obj);
        }
        else if (type == typeof(bool))
        {
            Marshal.StructureToPtr((NativeBool)obj, outPtr, false);
        }
        else if (type == typeof(string))
        {
            // FIXME This will leak string contents which is non-bueno (or not if user calls NvDotnetStringFree i guess)
            var strPtr = Marshal.StringToHGlobalAuto((string)obj);
            Marshal.WriteIntPtr(outPtr, strPtr);
        }
        else
        {
            var size = Marshal.SizeOf(type.IsEnum ? Enum.GetUnderlyingType(type) : type);
            var handle = GCHandle.Alloc(obj, GCHandleType.Pinned);

            unsafe
            {
                Buffer.MemoryCopy(handle.AddrOfPinnedObject().ToPointer(), outPtr.ToPointer(), size, size);
            }

            handle.Free();
        }

        // TODO Add fast path for primitive types ???
        // TODO Array marshalling
    }

    internal static bool TryMarshalArgumentsPtr(nint arguments, Type[] types, out object?[]? result)
    {
        if (arguments == 0)
        {
            result = null;
            return true;
        }

        result = new object?[types.Length];
        for (int i = 0; i < types.Length; i++)
        {
            if (!TryMarshalPtr(arguments, types[i], out result[i]))
            {
                return false;
            }

            arguments += Marshal.SizeOf<nint>();
        }

        return true;
    }

    internal static bool TryMarshalPtr(nint ptr, Type type, out object? result)
    {
        if (ptr == 0)
        {
            result = null;
            return true;
        }

        if (type.IsPointer || type == typeof(nint))
        {
            result = ptr;
            return true;
        }

        if (type == typeof(bool))
        {
            try
            {
                result = Marshal.PtrToStructure<byte>(ptr) != 0;
                return true;
            }
            catch (Exception exc)
            {
                ManagedHost.HandleException(exc);

                result = null;
                return false;
            }
        }

        if (type == typeof(string))
        {
            result = Marshal.PtrToStringAnsi(Marshal.ReadIntPtr(ptr));
            return true;
        }

        if (type.IsClass)
        {
            try
            {
                var handle = GCHandle.FromIntPtr(Marshal.ReadIntPtr(ptr));

                result = handle.Target;
                return true;
            }
            catch (Exception exc)
            {
                ManagedHost.HandleException(exc);

                result = null;
                return false;
            }
        }

        try
        {
            result = Marshal.PtrToStructure(ptr, type);
            return true;
        }
        catch (Exception exc)
        {
            ManagedHost.HandleException(exc);

            result = false;
            return false;
        }
    }
}
