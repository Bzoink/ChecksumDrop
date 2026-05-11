using ChecksumDrop.Core.Models;

namespace ChecksumDrop.Core.Services;

public sealed class ChecksumProcessor(HashingService hashingService)
{
    private readonly HashingService _hashingService = hashingService;

    public Task<ChecksumOperationResult> ProcessAsync(
        IReadOnlyList<string> droppedPaths,
        ChecksumMethod configuredMethod,
        IProgress<ChecksumProgress>? progress,
        CancellationToken cancellationToken)
    {
        return Task.Run(() => Process(droppedPaths, configuredMethod, progress, cancellationToken), cancellationToken);
    }

    private ChecksumOperationResult Process(
        IReadOnlyList<string> droppedPaths,
        ChecksumMethod configuredMethod,
        IProgress<ChecksumProgress>? progress,
        CancellationToken cancellationToken)
    {
        var state = new ChecksumOperationState { Method = configuredMethod };
        var fileCollection = new List<string>();
        var shortestPath = new string('a', 4096);
        var allFilesAreDigest = true;

        foreach (var path in droppedPaths)
        {
            cancellationToken.ThrowIfCancellationRequested();

            var directoryName = Path.GetDirectoryName(path) ?? string.Empty;
            if (directoryName.Length > 0 && directoryName.Length < shortestPath.Length)
            {
                shortestPath = directoryName;
            }

            if (Directory.Exists(path))
            {
                var rangeCollection = Directory.GetFiles(path, "*.*", SearchOption.AllDirectories);
                fileCollection.AddRange(rangeCollection);
            }
            else
            {
                fileCollection.Add(path);
            }
        }

        state.BasePath = shortestPath == new string('a', 4096) ? string.Empty : shortestPath;

        foreach (var path in droppedPaths)
        {
            if (!ChecksumMethodExtensions.TryParseDigestPath(path, out _))
            {
                allFilesAreDigest = false;
                break;
            }
        }

        return allFilesAreDigest
            ? ValidateDigests(fileCollection, state, progress, cancellationToken)
            : CreateDigests(fileCollection, state, progress, cancellationToken);
    }

    private ChecksumOperationResult ValidateDigests(
        IReadOnlyList<string> digestFiles,
        ChecksumOperationState state,
        IProgress<ChecksumProgress>? progress,
        CancellationToken cancellationToken)
    {
        state.Digests = 0;

        foreach (var currentFile in digestFiles)
        {
            cancellationToken.ThrowIfCancellationRequested();
            state.Digests++;

            if (!ChecksumMethodExtensions.TryParseDigestPath(currentFile, out var method))
            {
                continue;
            }

            state.Method = method;
            var done = 0;
            var totalNumberOfItems = Math.Max(1, LineCounter.CountLines(currentFile));

            progress?.Report(new ChecksumProgress
            {
                MainText = string.Empty,
                SubMainText = string.Empty,
                SubSubMainText = $"Checking {method.ToDisplayName()} hash",
                StatusText = $"Analyzing files in {Path.GetFileName(currentFile)}",
                Percent = 0
            });

            using var fileHandle = new StreamReader(currentFile);
            string? line;
            while ((line = fileHandle.ReadLine()) is not null)
            {
                cancellationToken.ThrowIfCancellationRequested();
                progress?.Report(new ChecksumProgress { Percent = (int)Math.Round(((double)done / totalNumberOfItems) * 100) });

                if (line.Length > method.GetHashLength())
                {
                    string filename;
                    string hash;

                    if (method == ChecksumMethod.CRC32)
                    {
                        filename = line.Substring(0, line.Length - method.GetHashLength()).Trim();
                        hash = line.Substring(line.Length - method.GetHashLength());
                    }
                    else
                    {
                        filename = line.Substring(method.GetHashLength() + 1).Trim();
                        hash = line.Substring(0, method.GetHashLength());
                    }

                    if (HexUtility.IsHexString(hash))
                    {
                      var filePath = Path.Combine(state.BasePath, NormalizeDigestPath(filename));
                        if (File.Exists(filePath))
                        {
                            var currentHash = _hashingService.ComputeHash(filePath, method);
                            state.FileHashes.Add(new FileHash
                            {
                                Filename = filename,
                                Hash = hash,
                                Status = currentHash == hash ? 101 : 909,
                                Method = method.ToDisplayName()
                            });
                        }
                        else
                        {
                            state.FileHashes.Add(new FileHash
                            {
                                Filename = filename,
                                Hash = hash,
                                Status = 404,
                                Method = method.ToDisplayName()
                            });
                        }
                    }
                }

                done++;
            }
        }

        var summary = new ValidationResultSummary
        {
            TotalFiles = state.FileHashes.Count,
            TotalNotFound = state.FileHashes.Count(t => t.Status == 404),
            TotalValid = state.FileHashes.Count(t => t.Status == 101),
            TotalInvalid = state.FileHashes.Count(t => t.Status == 909),
            TotalDigests = state.Digests
        };

        return new ChecksumOperationResult
        {
            IsValidation = true,
            FileHashes = state.FileHashes,
            ValidationSummary = summary,
            MainText = string.Empty,
            SubMainText = string.Empty,
            SubSubMainText = "Done validation!",
            StatusText = string.Empty
        };
    }

    private ChecksumOperationResult CreateDigests(
        IReadOnlyList<string> fileCollection,
        ChecksumOperationState state,
        IProgress<ChecksumProgress>? progress,
        CancellationToken cancellationToken)
    {
        state.Digests = 1;
        var done = 0;
        var totalNumberOfItems = Math.Max(1, fileCollection.Count);

        foreach (var currentFile in fileCollection)
        {
            cancellationToken.ThrowIfCancellationRequested();
            progress?.Report(new ChecksumProgress { Percent = (int)Math.Round(((double)done / totalNumberOfItems) * 100) });

            state.FileHashes.Add(new FileHash
            {
                Filename = currentFile,
                Hash = _hashingService.ComputeHash(currentFile, state.Method),
                Method = state.Method.ToDisplayName()
            });

            done++;
        }

        var outputFileName = state.Method.GetOutputFileName();
        var outputFilePath = Path.Combine(state.BasePath, outputFileName);
        File.WriteAllText(outputFilePath, ChecksumOutputWriter.Generate(state));

        return new ChecksumOperationResult
        {
            IsValidation = false,
            OutputFilePath = outputFilePath,
            FileHashes = state.FileHashes,
            MainText = string.Empty,
            SubMainText = string.Empty,
            SubSubMainText = "Wrote checksum file",
            StatusText = $"{outputFileName} of {fileCollection.Count} files using {state.Method.ToDisplayName()}"
        };
    }

    private static string NormalizeDigestPath(string relativePath)
    {
        return relativePath
            .Replace('\\', Path.DirectorySeparatorChar)
            .Replace('/', Path.DirectorySeparatorChar)
            .TrimStart(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);
    }
}
