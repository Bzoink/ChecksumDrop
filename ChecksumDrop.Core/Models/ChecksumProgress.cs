namespace ChecksumDrop.Core.Models;

public sealed class ChecksumProgress
{
   public string? MainText { get; init; }
    public string? SubMainText { get; init; }
    public string? SubSubMainText { get; init; }
    public string? StatusText { get; init; }
    public int Percent { get; init; }
}
