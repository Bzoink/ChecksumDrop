using ChecksumDrop.Classes;
using SHA3.Net;
using System;
using System.IO;
using System.Security.Cryptography;

namespace ChecksumDrop
{
    class Hash
    {
        public static string CreateMD5(string filename)
        {
            using (var md5 = MD5.Create())
            {
                using (var stream = File.OpenRead(filename))
                {
                    var hash = md5.ComputeHash(stream);
                    return BitConverter.ToString(hash).Replace("-", "").ToUpperInvariant();
                }
            }
        }
        public static string CreateSHA1(string filename)
        {
            using (var sha1 = SHA1.Create())
            {
                using (var stream = File.OpenRead(filename))
                {
                    var hash = sha1.ComputeHash(stream);
                    return BitConverter.ToString(hash).Replace("-", "").ToUpperInvariant();
                }
            }
        }
        public static string CreateSHA256(string filename)
        {
            using (var sha256 = SHA256.Create())
            {
                using (var stream = File.OpenRead(filename))
                {
                    var hash = sha256.ComputeHash(stream);
                    return BitConverter.ToString(hash).Replace("-", "").ToUpperInvariant();
                }
            }
        }
        public static string CreateSHA384(string filename)
        {
            using (var sha384 = SHA384.Create())
            {
                using (var stream = File.OpenRead(filename))
                {
                    var hash = sha384.ComputeHash(stream);
                    return BitConverter.ToString(hash).Replace("-", "").ToUpperInvariant();
                }
            }
        }
        public static string CreateSHA512(string filename)
        {
            using (var sha512 = SHA512.Create())
            {
                using (var stream = File.OpenRead(filename))
                {
                    var hash = sha512.ComputeHash(stream);
                    return BitConverter.ToString(hash).Replace("-", "").ToUpperInvariant();
                }
            }
        }
        public static string CreateSHA3_256(string filename)
        {
            using (var sha3_256 = Sha3.Sha3256())
            {
                using (var stream = File.OpenRead(filename))
                {
                    var hash = (stream.Length == 0) ? sha3_256.ComputeHash(new byte[0]) : sha3_256.ComputeHash(stream);
                    return BitConverter.ToString(hash).Replace("-", "").ToUpperInvariant();
                }
            }
        }
        public static string CreateSHA3_384(string filename)
        {
            using (var sha3_384 = Sha3.Sha3384())
            {
                using (var stream = File.OpenRead(filename))
                {
                    var hash = (stream.Length == 0) ? sha3_384.ComputeHash(new byte[0]) : sha3_384.ComputeHash(stream);
                    return BitConverter.ToString(hash).Replace("-", "").ToUpperInvariant();
                }
            }
        }
        public static string CreateSHA3_512(string filename)
        {
            using (var sha3_512 = Sha3.Sha3512())
            {
                using (var stream = File.OpenRead(filename))
                {
                    var hash = (stream.Length == 0) ? sha3_512.ComputeHash(new byte[0]) : sha3_512.ComputeHash(stream);
                    return BitConverter.ToString(hash).Replace("-", "").ToUpperInvariant();
                }
            }
        }
        public static string CreateCRC32(string filename)
        {
            byte[] buffer = new byte[4096];
            int length;

            FileStream file = new FileStream(filename, FileMode.Open);
            CrcStream stream = new CrcStream(file);

            while ((length = stream.Read(buffer, 0, buffer.Length)) != 0)
            {
                // Do whatever you need to do in here
            }

            return stream.ReadCrc.ToString("X8");
        }
        public static string GetHash(string filename)
        {
            string hash = string.Empty;

            switch (FileList.Method)
            {
                case "CRC32":
                case "SFV":
                    hash = Hash.CreateCRC32(filename);
                    break;
                case "MD5":
                    hash = Hash.CreateMD5(filename);
                    break;
                case "SHA1":
                    hash = Hash.CreateSHA1(filename);
                    break;
                case "SHA256":
                    hash = Hash.CreateSHA256(filename);
                    break;
                case "SHA384":
                    hash = Hash.CreateSHA384(filename);
                    break;
                case "SHA512":
                    hash = Hash.CreateSHA512(filename);
                    break;
                case "SHA3256":
                    hash = Hash.CreateSHA3_256(filename);
                    break;
                case "SHA3384":
                    hash = Hash.CreateSHA3_384(filename);
                    break;
                case "SHA3512":
                    hash = Hash.CreateSHA3_512(filename);
                    break;
            }
            return hash;
        }
    }
}
