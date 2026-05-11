namespace ChecksumDrop.Core.Models;

public sealed class FileHash
{
    public string Filename { get; set; } = string.Empty;
    public string Hash { get; set; } = string.Empty;
    public string Method { get; set; } = string.Empty;
    public int Status { get; set; }
}
