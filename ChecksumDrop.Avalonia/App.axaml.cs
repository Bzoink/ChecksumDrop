using Avalonia;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Markup.Xaml;
using ChecksumDrop.Avalonia.ViewModels;
using ChecksumDrop.Avalonia.Views;
using ChecksumDrop.Core;
using ChecksumDrop.Core.Services;

namespace ChecksumDrop.Avalonia;

public partial class App : Application
{
    public override void Initialize()
    {
        AvaloniaXamlLoader.Load(this);
    }

    public override async void OnFrameworkInitializationCompleted()
    {
        if (ApplicationLifetime is IClassicDesktopStyleApplicationLifetime desktop)
        {
            var settingsService = new JsonSettingsService(AppMetadata.ProgramName);
            var settings = await settingsService.LoadAsync();
            var hashingService = new HashingService();
            var processor = new ChecksumProcessor(hashingService);

            desktop.MainWindow = new MainWindow
            {
                DataContext = new MainWindowViewModel(settingsService, settings, processor)
            };
        }

        base.OnFrameworkInitializationCompleted();
    }
}
