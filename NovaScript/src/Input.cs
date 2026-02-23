using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Http.Headers;
using System.Reflection.Metadata.Ecma335;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace NovaScript;

public ref struct InputState
{
    private nint _stateArrayPtr;

    public InputState(nint stateArrayPtr)
    {
        _stateArrayPtr = stateArrayPtr;
    }

    public bool IsPressed(byte index) => Marshal.ReadByte(_stateArrayPtr + (Marshal.SizeOf<nint>() * index)) != 0;
    public bool IsReleased(byte index) => !IsPressed(index);

    public bool this[byte index] => IsPressed(index);
}

public static class Input
{
    private static unsafe delegate* unmanaged<byte, byte> IsKeyDownImpl;
    private static unsafe delegate* unmanaged<byte, byte> IsKeyUpImpl;
    private static unsafe delegate* unmanaged<nint> GetKeyStateImpl;
    private static unsafe delegate* unmanaged<byte, byte> IsButtonDownImpl;
    private static unsafe delegate* unmanaged<byte, byte> IsButtonUpImpl;
    private static unsafe delegate* unmanaged<nint> GetButtonStateImpl;
    private static unsafe delegate* unmanaged<double> GetMouseXImpl;
    private static unsafe delegate* unmanaged<double> GetMouseYImpl;
    private static unsafe delegate* unmanaged<byte, byte> IsModifierActiveImpl;
    private static unsafe delegate* unmanaged<void> BeginTextInputImpl;
    private static unsafe delegate* unmanaged<nint> EndTextInputImpl;
    private static unsafe delegate* unmanaged<nint> GetTextInputImpl;
    private static unsafe delegate* unmanaged<ulong> GetTextInputCapacityImpl;
    private static unsafe delegate* unmanaged<ulong, void> SetTextInputBufferSizeImpl;
    private static unsafe delegate* unmanaged<byte> IsTextInputActiveImpl;
    
    public static bool IsKeyDown(byte key)
    {
        unsafe { return IsKeyDownImpl(key) != 0; }
    }

    public static bool IsKeyUp(byte key)
    {
        unsafe { return IsKeyUpImpl(key) != 0; }
    }

    public static InputState GetKeyState()
    {
        unsafe { return new(GetKeyStateImpl()); }
    }

    public static bool IsButtonDown(byte button)
    {
        unsafe { return IsButtonDownImpl(button) != 0; }
    }

    public static bool IsButtonUp(byte button)
    {
        unsafe { return IsButtonUpImpl(button) != 0; }
    }

    public static InputState GetButtonState()
    {
        unsafe { return new(GetButtonStateImpl()); }
    }

    public static double GetMouseX()
    {
        unsafe { return GetMouseXImpl(); }
    }

    public static double GetMouseY()
    {
        unsafe { return GetMouseYImpl(); }
    }

    public static (double, double) GetMousePos()
    {
        return (GetMouseX(), GetMouseY());
    }

    public static bool IsModifierActive(byte modifier)
    {
        unsafe { return IsModifierActiveImpl(modifier) != 0; }
    }

    public static void BeginTextInput()
    {
        unsafe { BeginTextInputImpl(); }
    }

    public static string EndTextInput()
    {
        unsafe { return Marshal.PtrToStringUTF8(EndTextInputImpl())!; }
    }

    public static string GetTextInput()
    {
        unsafe { return Marshal.PtrToStringUTF8(GetTextInputImpl())!; }
    }

    public static ulong GetTextInputCapacity()
    {
        unsafe { return GetTextInputCapacityImpl(); }
    }

    public static void SetTextInputBufferSize(ulong newSize)
    {
        unsafe { SetTextInputBufferSizeImpl(newSize); }
    }

    public static bool IsTextInputActive()
    {
        unsafe { return IsTextInputActiveImpl() != 0; }
    }
}
