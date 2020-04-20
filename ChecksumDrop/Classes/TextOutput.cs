using System;
using System.IO;
using System.Text;

namespace ChecksumDrop.Classes
{
    public static class TextOutput
    {
        public static string Generate()
        {
            StringBuilder build = new StringBuilder();
            build.AppendLine(string.Format("; Created with {0} version {1}.{2} {3} using {4} hash", Properties.Resources.ProgramName, Properties.Resources.MajorVersion, Properties.Resources.MinorVersion, DateTime.Now, Hyphenator.Translate(FileList.Method).ToLower()));
            switch (FileList.Method)
            {
                case "CRC32":
                    foreach(var currentFile in FileList.FileHash)
                    {
                        build.AppendLine(string.Format("{0}  {1}", currentFile.Filename.Replace(FileList.BasePath, "").Trim('\\'), currentFile.Hash));
                    }
                    break;
                default:
                    foreach (var currentFile in FileList.FileHash)
                    {
                        build.AppendLine(string.Format("{0}  {1}", currentFile.Hash, currentFile.Filename.Replace(FileList.BasePath, "").Trim('\\')));
                    }
                    break;
            }
            return build.ToString();
        }
    }
}
