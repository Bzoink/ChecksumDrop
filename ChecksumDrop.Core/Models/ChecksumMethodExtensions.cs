using System.Globalization;

namespace ChecksumDrop.Core.Models;

public static class ChecksumMethodExtensions
{
    private static readonly Dictionary<string, ChecksumMethod> DigestTokens = new(StringComparer.OrdinalIgnoreCase)
    {
        ["sfv"] = ChecksumMethod.CRC32,
        ["md5"] = ChecksumMethod.MD5,
        ["sha1"] = ChecksumMethod.SHA1,
        ["sha256"] = ChecksumMethod.SHA256,
        ["sha384"] = ChecksumMethod.SHA384,
        ["sha512"] = ChecksumMethod.SHA512,
        ["sha3-256"] = ChecksumMethod.SHA3256,
        ["sha3-384"] = ChecksumMethod.SHA3384,
        ["sha3-512"] = ChecksumMethod.SHA3512,
        ["blake3"] = ChecksumMethod.BLAKE3,
        ["xxh64"] = ChecksumMethod.XXHASH64,
        ["xxh3"] = ChecksumMethod.XXHASH3
    };

    public static string ToDisplayName(this ChecksumMethod method) => method switch
    {
        ChecksumMethod.CRC32 => "CRC32",
        ChecksumMethod.MD5 => "MD5",
        ChecksumMethod.SHA1 => "SHA1",
        ChecksumMethod.SHA256 => "SHA256",
        ChecksumMethod.SHA384 => "SHA384",
        ChecksumMethod.SHA512 => "SHA512",
        ChecksumMethod.SHA3256 => "SHA3-256",
        ChecksumMethod.SHA3384 => "SHA3-384",
        ChecksumMethod.SHA3512 => "SHA3-512",
        ChecksumMethod.BLAKE3 => "BLAKE3",
        ChecksumMethod.XXHASH64 => "xxHash64",
        ChecksumMethod.XXHASH3 => "xxHash3",
        _ => method.ToString().ToUpperInvariant()
    };

    public static string ToDigestToken(this ChecksumMethod method) => method switch
    {
        ChecksumMethod.CRC32 => "sfv",
        ChecksumMethod.MD5 => "md5",
        ChecksumMethod.SHA1 => "sha1",
        ChecksumMethod.SHA256 => "sha256",
        ChecksumMethod.SHA384 => "sha384",
        ChecksumMethod.SHA512 => "sha512",
        ChecksumMethod.SHA3256 => "sha3-256",
        ChecksumMethod.SHA3384 => "sha3-384",
        ChecksumMethod.SHA3512 => "sha3-512",
        ChecksumMethod.BLAKE3 => "blake3",
        ChecksumMethod.XXHASH64 => "xxh64",
        ChecksumMethod.XXHASH3 => "xxh3",
        _ => method.ToString().ToLower(CultureInfo.InvariantCulture)
    };

    public static int GetHashLength(this ChecksumMethod method) => method switch
    {
        ChecksumMethod.CRC32 => 8,
        ChecksumMethod.MD5 => 32,
        ChecksumMethod.SHA1 => 40,
        ChecksumMethod.SHA256 or ChecksumMethod.SHA3256 or ChecksumMethod.BLAKE3 => 64,
        ChecksumMethod.SHA384 or ChecksumMethod.SHA3384 => 96,
        ChecksumMethod.SHA512 or ChecksumMethod.SHA3512 => 128,
        ChecksumMethod.XXHASH64 or ChecksumMethod.XXHASH3 => 16,
        _ => throw new ArgumentOutOfRangeException(nameof(method), method, null)
    };

    public static string GetOutputFileName(this ChecksumMethod method) => method == ChecksumMethod.CRC32
        ? "checksum.sfv"
        : $"checksum.{method.ToDigestToken()}.txt";

    public static bool TryParseDigestPath(string path, out ChecksumMethod method)
    {
        method = default;

        var fileName = Path.GetFileName(path);
        if (string.IsNullOrWhiteSpace(fileName))
        {
            return false;
        }

        var source = fileName
            .Split('.', StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries)
            .Distinct(StringComparer.OrdinalIgnoreCase)
            .ToArray();

        var matches = source
            .Where(token => DigestTokens.ContainsKey(token))
            .Select(token => DigestTokens[token])
            .Distinct()
            .ToArray();

        if (matches.Length != 1)
        {
            return false;
        }

        method = matches[0];
        return true;
    }

    public static bool TryParseSetting(string value, out ChecksumMethod method)
    {
        method = default;

        if (string.IsNullOrWhiteSpace(value))
        {
            return false;
        }

        return Enum.TryParse(value.Replace("-", string.Empty), ignoreCase: true, out method);
    }
}
