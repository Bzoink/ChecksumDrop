using System.Text.RegularExpressions;

namespace ChecksumDrop.Core.Services;

public static partial class HexUtility
{
    [GeneratedRegex(@"\A\b[0-9a-fA-F]+\b\Z")]
    private static partial Regex HexRegex();

    public static bool IsHexString(string test) => HexRegex().IsMatch(test);
}
