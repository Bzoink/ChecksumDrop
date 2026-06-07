using ChecksumDrop.Core;

namespace ChecksumDrop.Avalonia.ViewModels;

public partial class AboutWindowViewModel : ViewModelBase
{
    public string AboutLine1 => $"{AppMetadata.ProgramName} Version {AppMetadata.Version}";
    public string AboutLine2 => $"©{AppMetadata.PublicationYear} {AppMetadata.Author} ({AppMetadata.Company})";
}
