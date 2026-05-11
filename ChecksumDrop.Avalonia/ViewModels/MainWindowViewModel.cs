using Avalonia.Controls;
using Avalonia.Platform.Storage;
using ChecksumDrop.Avalonia.Views;
using ChecksumDrop.Core.Models;
using ChecksumDrop.Core.Services;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using System.Collections.ObjectModel;

namespace ChecksumDrop.Avalonia.ViewModels;

public partial class MainWindowViewModel : ViewModelBase
{
    private readonly JsonSettingsService _settingsService;
    private readonly AppSettings _settings;
    private readonly ChecksumProcessor _processor;
    private CancellationTokenSource? _cts;
    public ObservableCollection<ChecksumMethod> Methods { get; } = new(Enum.GetValues<ChecksumMethod>());

    [ObservableProperty]
    private string _mainText = "Drop files or folders here";

    [ObservableProperty]
    private string _subMainText = "to create a checksum file";

    [ObservableProperty]
    private string _subSubMainText = "Drop digest file to verify";

    [ObservableProperty]
    private string _statusText = string.Empty;

    [ObservableProperty]
    private int _progressValue;

    [ObservableProperty]
    private bool _isBusy;

    [ObservableProperty]
    private ChecksumMethod _selectedMethod = ChecksumMethod.MD5;

    public MainWindowViewModel(JsonSettingsService settingsService, AppSettings settings, ChecksumProcessor processor)
    {
        _settingsService = settingsService;
        _settings = settings;
        _processor = processor;

        if (ChecksumMethodExtensions.TryParseSetting(settings.Method, out var method))
        {
            _selectedMethod = method;
        }
    }

    partial void OnSelectedMethodChanged(ChecksumMethod value)
    {
        _settings.Method = value.ToString();
        _ = _settingsService.SaveAsync(_settings);
    }

    [RelayCommand]
    private async Task OpenSettingsAsync(Window owner)
    {
        var window = new SettingsWindow
        {
            DataContext = new SettingsWindowViewModel(SelectedMethod)
        };

        var result = await window.ShowDialog<ChecksumMethod?>(owner);
        if (result.HasValue)
        {
            SelectedMethod = result.Value;
        }
    }

    [RelayCommand]
    private void Cancel()
    {
        _cts?.Cancel();
        MainText = string.Empty;
        SubMainText = string.Empty;
        SubSubMainText = $"Cancelling {SelectedMethod.ToDigestToken()}";
        StatusText = "Waiting for file handle ...";
    }

    public async Task HandleDropAsync(Window owner, IEnumerable<IStorageItem> items)
    {
        if (IsBusy)
        {
            return;
        }

        var paths = items
            .Select(item => item.TryGetLocalPath())
            .Where(path => !string.IsNullOrWhiteSpace(path))
            .Cast<string>()
            .ToArray();

        if (paths.Length == 0)
        {
            return;
        }

        IsBusy = true;
        ProgressValue = 0;
        MainText = string.Empty;
        SubMainText = string.Empty;
        SubSubMainText = $"Creating {SelectedMethod.ToDigestToken()}";
        StatusText = string.Empty;

        _cts = new CancellationTokenSource();
        var progress = new Progress<ChecksumProgress>(update =>
        {
          if (update.MainText is not null)
            {
                MainText = update.MainText;
            }

            if (update.SubMainText is not null)
            {
                SubMainText = update.SubMainText;
            }

          if (update.SubSubMainText is not null)
            {
                SubSubMainText = update.SubSubMainText;
            }

          if (update.StatusText is not null)
            {
                StatusText = update.StatusText;
            }

            ProgressValue = update.Percent;
        });

        try
        {
            var result = await _processor.ProcessAsync(paths, SelectedMethod, progress, _cts.Token);
            MainText = result.MainText;
            SubMainText = result.SubMainText;
            SubSubMainText = result.SubSubMainText;
            StatusText = result.StatusText;
            ProgressValue = 100;

            if (result.IsValidation && result.ValidationSummary is not null)
            {
                var lastResults = result.FileHashes
                    .Select(file => new FileHashOutput { Filename = file.Filename, Method = file.Method, Status = file.Status switch { 101 => "OK", 404 => "File not found", 909 => "Invalid hash", _ => "General error" } })
                    .ToList();

                var resultsWindow = new ResultsWindow
                {
                    DataContext = new ResultsWindowViewModel(lastResults, result.ValidationSummary)
                };

                await resultsWindow.ShowDialog(owner);
            }
        }
        catch (OperationCanceledException)
        {
            ResetToIdle();
        }
        catch (Exception ex)
        {
            MainText = string.Empty;
            SubMainText = string.Empty;
            SubSubMainText = "Error";
            StatusText = ex.Message;
        }
        finally
        {
            IsBusy = false;
            _cts?.Dispose();
            _cts = null;
        }
    }

    [RelayCommand]
    private async Task PickFilesAsync(Window owner)
    {
        var topLevel = TopLevel.GetTopLevel(owner);
        if (topLevel?.StorageProvider is null)
        {
            return;
        }

        var files = await topLevel.StorageProvider.OpenFilePickerAsync(new FilePickerOpenOptions
        {
            AllowMultiple = true,
            Title = "Select files"
        });

        await HandleDropAsync(owner, files);
    }

    [RelayCommand]
    private async Task PickFolderAsync(Window owner)
    {
        var topLevel = TopLevel.GetTopLevel(owner);
        if (topLevel?.StorageProvider is null)
        {
            return;
        }

        var folders = await topLevel.StorageProvider.OpenFolderPickerAsync(new FolderPickerOpenOptions
        {
            AllowMultiple = false,
            Title = "Select folder"
        });

        await HandleDropAsync(owner, folders);
    }

    private void ResetToIdle()
    {
        MainText = "Drop files or folders here";
        SubMainText = "to create a checksum file";
        SubSubMainText = "Drop digest file to verify";
        StatusText = string.Empty;
        ProgressValue = 0;
    }
}
