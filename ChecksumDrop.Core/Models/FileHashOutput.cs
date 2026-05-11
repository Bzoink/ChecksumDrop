namespace ChecksumDrop.Core.Models;

public sealed class FileHashOutput
{
    public string Filename { get; set; } = string.Empty;
    public string Status { get; set; } = string.Empty;
    public string Method { get; set; } = string.Empty;
}
