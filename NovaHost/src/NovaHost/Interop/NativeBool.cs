namespace NovaHost.Interop;

// c# booleans are not blittable so we need special type that is of the same size but is blittable
public readonly struct NativeBool
{
    private byte Value { get; init; }

    NativeBool(bool value)
    {
        Value = value ? (byte)1 : (byte)0;
    }

    public static implicit operator bool(NativeBool x) => x.Value != 0;
    public static implicit operator NativeBool(bool x) => new(x);

    public static NativeBool True => new(true);
    public static NativeBool False => new(false);


}
