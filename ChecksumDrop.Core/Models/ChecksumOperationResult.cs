namespace ChecksumDrop.Core.Models;

public sealed class ChecksumOperationResult
{
    public bool Cancelled { get; init; }
    public bool IsValidation { get; init; }
    public string? OutputFilePath { get; init; }
    public IReadOnlyList<FileHash> FileHashes { get; init; } = Array.Empty<FileHash>();
    public ValidationResultSummary? ValidationSummary { get; init; }
    public string MainText { get; init; } = string.Empty;
    public string SubMainText { get; init; } = string.Empty;
    public string SubSubMainText { get; init; } = string.Empty;
    public string StatusText { get; init; } = string.Empty;
}
