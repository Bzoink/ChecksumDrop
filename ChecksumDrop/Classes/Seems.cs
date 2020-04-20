using System;
using System.IO;
using System.Linq;
using System.Windows;

namespace ChecksumDrop.Classes
{
    static class Seems
    {
        public static string LikeADigest (string path)
        {
            string[] supported = new string[] { "sfv", "md5", "sha1", "sha256", "sha384", "sha512", "sha3-256", "sha3-384", "sha3-512" };
            string[] source = path.Split(new char[] { '.' }, StringSplitOptions.RemoveEmptyEntries);
            var intersect = source.Intersect(supported);

            if (File.Exists(path) && intersect.Count() == 1)
            {
                return intersect.First().Replace("-", string.Empty).ToUpper();
            }
            return "None";
        }
    }
}
