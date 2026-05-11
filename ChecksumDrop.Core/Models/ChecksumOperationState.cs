namespace ChecksumDrop.Core.Models;

public sealed class ChecksumOperationState
{
    public string BasePath { get; set; } = string.Empty;
    public ChecksumMethod Method { get; set; } = ChecksumMethod.MD5;
    public int Digests { get; set; }
    public List<FileHash> FileHashes { get; } = new();

    public IReadOnlyList<FileHashOutput> ToOutputTable() => FileHashes
        .Select(currentFile => new FileHashOutput
        {
            Filename = currentFile.Filename,
            Method = currentFile.Method,
            Status = currentFile.Status switch
            {
                101 => "OK",
                404 => "File not found",
                909 => "Invalid hash",
                _ => "General error"
            }
        })
        .ToList();
}
