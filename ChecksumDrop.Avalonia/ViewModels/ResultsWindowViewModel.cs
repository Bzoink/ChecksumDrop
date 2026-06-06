using ChecksumDrop.Core.Models;
using System.Collections.ObjectModel;

namespace ChecksumDrop.Avalonia.ViewModels;

public sealed class ResultsWindowViewModel : ViewModelBase
{
    public ObservableCollection<ResultItemViewModel> Items { get; }
    public string TotalResults { get; }
    public string TotalFiles { get; }
    public string NotFound { get; }
    public string NotValid { get; }

    public ResultsWindowViewModel(IReadOnlyList<FileHashOutput> items, ValidationResultSummary summary)
    {
        Items = new ObservableCollection<ResultItemViewModel>(items.Select(item => new ResultItemViewModel
        {
            Filename = item.Filename,
            Method = item.Method,
            Status = item.Status
        }));

        TotalResults = summary.TotalResultsText;
        TotalFiles = summary.TotalFilesText;
        NotFound = summary.TotalNotFound.ToString();
        NotValid = summary.TotalInvalid.ToString();
    }
}
