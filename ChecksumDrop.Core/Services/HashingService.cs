using ChecksumDrop.Core.Models;
using SHA3.Net;
using System.Security.Cryptography;

namespace ChecksumDrop.Core.Services;

public sealed class HashingService
{
    public string ComputeHash(string filename, ChecksumMethod method)
    {
        return method switch
        {
            ChecksumMethod.CRC32 => CreateCrc32(filename),
            ChecksumMethod.MD5 => Compute(filename, MD5.Create),
            ChecksumMethod.SHA1 => Compute(filename, SHA1.Create),
            ChecksumMethod.SHA256 => Compute(filename, SHA256.Create),
            ChecksumMethod.SHA384 => Compute(filename, SHA384.Create),
            ChecksumMethod.SHA512 => Compute(filename, SHA512.Create),
            ChecksumMethod.SHA3256 => ComputeSha3(filename, Sha3.Sha3256),
            ChecksumMethod.SHA3384 => ComputeSha3(filename, Sha3.Sha3384),
            ChecksumMethod.SHA3512 => ComputeSha3(filename, Sha3.Sha3512),
            _ => throw new ArgumentOutOfRangeException(nameof(method), method, null)
        };
    }

    private static string Compute(string filename, Func<HashAlgorithm> factory)
    {
        using var algorithm = factory();
        using var stream = File.OpenRead(filename);
        return ToHex(algorithm.ComputeHash(stream));
    }

    private static string ComputeSha3(string filename, Func<HashAlgorithm> factory)
    {
        using var algorithm = factory();
        using var stream = File.OpenRead(filename);
        var hash = stream.Length == 0 ? algorithm.ComputeHash(Array.Empty<byte>()) : algorithm.ComputeHash(stream);
        return ToHex(hash);
    }

    private static string CreateCrc32(string filename)
    {
        var buffer = new byte[4096];
        using var file = new FileStream(filename, FileMode.Open, FileAccess.Read, FileShare.Read);
        using var stream = new Crc32Stream(file);

        while (stream.Read(buffer, 0, buffer.Length) != 0)
        {
        }

        return stream.ReadCrc.ToString("X8");
    }

    private static string ToHex(byte[] hash) => BitConverter.ToString(hash).Replace("-", string.Empty).ToUpperInvariant();
}
