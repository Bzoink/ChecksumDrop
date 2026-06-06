namespace ChecksumDrop.Core.Models;

public sealed class ValidationResultSummary
{
    public int TotalFiles { get; init; }
    public int TotalNotFound { get; init; }
    public int TotalInvalid { get; init; }
    public int TotalValid { get; init; }
    public int TotalDigests { get; init; }
    public string TotalResultsText => (TotalNotFound + TotalInvalid) == 0 ? "All files are validated" : "There were errors!";
    public string TotalFilesText => TotalDigests == 1 ? TotalFiles.ToString() : $"{TotalFiles} files in {TotalDigests} digests";
}
