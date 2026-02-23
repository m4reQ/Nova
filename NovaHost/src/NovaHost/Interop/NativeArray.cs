using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace NovaHost.Interop;

public struct NativeArrayEnumerator<T> : IEnumerator<T> where T : struct
{
    private IntPtr Data { get; init; }
    private int Index { get; set; } = 0;
    private int Length { get; init; }

    public NativeArrayEnumerator(IntPtr data, int length)
    {
        Data = data;
        Length = length;
    }

    public readonly T Current => Marshal.PtrToStructure<T>(Data + Marshal.SizeOf<T>() * Index);

    readonly object IEnumerator.Current => Current;

    public readonly void Dispose() { }

    public bool MoveNext()
    {
        Index++;
        return Index < Length;
    }

    public void Reset()
    {
        Index = 0;
    }
}

[StructLayout(LayoutKind.Sequential, Pack = 1)]
public struct NativeArray
{
    public nint Address;
    public ulong Length;
    public GCHandle? Handle;
}

public struct NativeArray<T> : IDisposable, IEnumerable<T> where T : struct
{
    public nint Data { get; init; }
    public long Length { get; init; }
    private NativeBool IsDisposed { get; set; } = false;
    private NativeBool ShouldDispose { get; init; }

    public static NativeArray<T> Empty => new(0, 0, false);

    public NativeArray(int length)
    {
        Length = length;
        Data = Marshal.AllocHGlobal(Marshal.SizeOf<T>() * length);
        ShouldDispose = true;
    }

    public NativeArray(T[] values)
        : this(values.Length)
    {
        var ptr = Data;
        foreach (var value in values)
        {
            Marshal.StructureToPtr(value, ptr, false);
            ptr += Marshal.SizeOf<T>();
        }
    }

    public NativeArray(IntPtr data, int length, bool shouldDispose)
    {
        Length = length;
        Data = data;
        ShouldDispose = shouldDispose;
    }

    public unsafe NativeArray(T* ptr, int length)
    {
        Length = length;
        Data = new(ptr);
        ShouldDispose = false;
    }

    public readonly ReadOnlySpan<T> AsReadonlySpan()
    {
        unsafe
        {
            // NOTE Unsafe conversion of int64_t to int32_t
            return new(Data.ToPointer(), (int)Length);
        }
    }

    public readonly Span<T> AsSpan()
    {
        unsafe
        {
            // NOTE Unsafe conversion of int64_t to int32_t
            return new(Data.ToPointer(), (int)Length);
        }
    }

    public readonly T[] ToArray()
    {
        var array = new T[Length];
        for (int i = 0; i < Length; i++)
            array[i] = this[i];

        return array;
    }

    public readonly T this[int i]
    {
        get => Marshal.PtrToStructure<T>(Data + Marshal.SizeOf<T>() * i);
        set => Marshal.StructureToPtr(value, Data + Marshal.SizeOf<T>() * i, false);
    }

    // IDisposable
    public readonly void Dispose()
    {
        if (!ShouldDispose || IsDisposed)
            return;

        Marshal.FreeHGlobal(Data);

        GC.SuppressFinalize(this);
    }

    // IEnumerable
    public readonly IEnumerator<T> GetEnumerator() => new NativeArrayEnumerator<T>(Data, (int)Length);

    readonly IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();
}
