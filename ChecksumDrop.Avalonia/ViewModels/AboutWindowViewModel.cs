using ChecksumDrop.Core;
using ChecksumDrop.Core.Models;
using CommunityToolkit.Mvvm.ComponentModel;
using System.Collections.ObjectModel;

namespace ChecksumDrop.Avalonia.ViewModels;

public partial class SettingsWindowViewModel : ViewModelBase
{
    public ObservableCollection<ChecksumMethod> Methods { get; } = new(Enum.GetValues<ChecksumMethod>());

    [ObservableProperty]
    private ChecksumMethod _selectedMethod;

    public string AboutLine1 => $"{AppMetadata.ProgramName} Version {AppMetadata.Version}";
    public string AboutLine2 => $"©{AppMetadata.PublicationYear} {AppMetadata.Author} ({AppMetadata.Company})";

    public SettingsWindowViewModel(ChecksumMethod selectedMethod)
    {
        _selectedMethod = selectedMethod;
    }
}
