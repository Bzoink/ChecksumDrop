namespace ChecksumDrop.Core.Models;

public sealed class AppSettings
{
    public string Method { get; set; } = ChecksumMethod.MD5.ToString();
}
