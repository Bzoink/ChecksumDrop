using ChecksumDrop.Classes;
using System.Collections.Generic;

namespace ChecksumDrop
{
    public static class FileList
    {
        public static int Digests { get; set; }
        public static string BasePath { get; set; }
        public static string Method { get; set; }
        public static long ExecutionTime { get; set; }
        public static List<FileHash> FileHash = new List<FileHash>();
        public static List<FileHashOutput> HashTableOutput()
        {
            List<FileHashOutput> listOuput = new List<FileHashOutput>();
            string status = string.Empty;
            foreach(var currentFile in FileHash)
            {
                switch (currentFile.Status)
                {
                    case 101:
                        status = "OK";
                        break;
                    case 404:
                        status = "File not found";
                        break;
                    case 909:
                        status = "Invalid hash";
                        break;
                    default:
                        status = "General error";
                        break;
                }
                listOuput.Add(new FileHashOutput { Filename = currentFile.Filename, Method = Hyphenator.Translate(currentFile.Method), Status = status });
            }
            return listOuput;
        }
        public static int HashLength()
        {
            var hashLength = 0;
            switch(Method)
            {
                case "SFV":
                case "CRC32":
                    hashLength = 8;
                    break;
                case "MD5":
                    hashLength = 32;
                    break;

                case "SHA1":
                    hashLength = 40;
                    break;
                case "SHA256":
                case "SHA3256":
                    hashLength = 64;
                    break;
                case "SHA384":
                case "SHA3384":
                    hashLength = 96;
                    break;
                case "SHA512":
                case "SHA3512":
                    hashLength = 128;
                    break;
            }
            return hashLength;
        }
    }

    public class FileHash
    {
        public string Filename { get; set; }
        public string Hash { get; set; }
        public string Method { get; set; }
        public int Status { get; set; }
    }

    public class FileHashOutput
    {
        public string Filename { get; set; }
        public string Status { get; set; }
        public string Method { get; set; }
    }
}