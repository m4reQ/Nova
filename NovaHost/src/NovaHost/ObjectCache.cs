using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Diagnostics;

namespace NovaHost;

internal class ObjectCache<T>
{
    private readonly Dictionary<int, T> CacheDict = [];

    public T GetValue(int hashcode)
    {
        return CacheDict[hashcode];
    }

    public bool TryAdd(T obj, out int hashcode)
    {
        hashcode = obj!.GetHashCode();
        return CacheDict.TryAdd(hashcode, obj);
    }

    public bool TryGetValue(int hashcode, [NotNullWhen(true)] out T? obj)
    {
        if (CacheDict.TryGetValue(hashcode, out obj))
        {
            Debug.Assert(obj is not null);
            return true;
        }

        return false;
    }

    public void Clear()
    {
        CacheDict.Clear();
    }
}
