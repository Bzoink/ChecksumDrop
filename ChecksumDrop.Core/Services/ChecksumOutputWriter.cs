using ChecksumDrop.Core.Models;
using System.Text;

namespace ChecksumDrop.Core.Services;

public static class ChecksumOutputWriter
{
    public static string Generate(ChecksumOperationState state)
    {
        var build = new StringBuilder();
        build.AppendLine($"; Created with {AppMetadata.ProgramName} version {AppMetadata.Version} {DateTime.Now} using {state.Method.ToDigestToken()} hash");

        switch (state.Method)
        {
            case ChecksumMethod.CRC32:
                foreach (var currentFile in state.FileHashes)
                {
                    build.AppendLine($"{GetRelativeDigestPath(state.BasePath, currentFile.Filename)}  {currentFile.Hash}");
                }
                break;
            default:
                foreach (var currentFile in state.FileHashes)
                {
                    build.AppendLine($"{currentFile.Hash}  {GetRelativeDigestPath(state.BasePath, currentFile.Filename)}");
                }
                break;
        }

        return build.ToString();
    }

    private static string GetRelativeDigestPath(string basePath, string filePath)
    {
        if (string.IsNullOrWhiteSpace(basePath))
        {
            return filePath;
        }

        var normalizedBasePath = Path.GetFullPath(basePath);
        var normalizedFilePath = Path.GetFullPath(filePath);

        if (!normalizedFilePath.StartsWith(normalizedBasePath, StringComparison.OrdinalIgnoreCase))
        {
            return filePath;
        }

        var relativePath = Path.GetRelativePath(normalizedBasePath, normalizedFilePath);
        return relativePath.TrimStart(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);
    }
}
