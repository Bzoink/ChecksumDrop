using global::Avalonia.Controls;
using global::Avalonia.Input;
using ChecksumDrop.Avalonia.ViewModels;

namespace ChecksumDrop.Avalonia.Views;

public partial class MainWindow : Window
{
    public MainWindow()
    {
        InitializeComponent();
        DragDrop.SetAllowDrop(DropZone, true);
        DropZone.AddHandler(DragDrop.DragOverEvent, DropZone_OnDragOver);
        DropZone.AddHandler(DragDrop.DropEvent, DropZone_OnDrop);
    }

    private void DropZone_OnDragOver(object? sender, DragEventArgs e)
    {
        e.DragEffects = DragDropEffects.Copy;
    }

    private async void DropZone_OnDrop(object? sender, DragEventArgs e)
    {
        if (DataContext is MainWindowViewModel viewModel)
        {
            var items = e.DataTransfer.TryGetFiles();
            if (items is not null)
            {
                await viewModel.HandleDropAsync(this, items);
            }
        }
    }

    private async void SettingsButton_OnClick(object? sender, global::Avalonia.Interactivity.RoutedEventArgs e)
    {
        if (DataContext is MainWindowViewModel viewModel)
        {
            await viewModel.OpenSettingsCommand.ExecuteAsync(this);
        }
    }

    private async void ChooseFilesButton_OnClick(object? sender, global::Avalonia.Interactivity.RoutedEventArgs e)
    {
        if (DataContext is MainWindowViewModel viewModel)
        {
            await viewModel.PickFilesCommand.ExecuteAsync(this);
        }
    }

    private async void ChooseFolderButton_OnClick(object? sender, global::Avalonia.Interactivity.RoutedEventArgs e)
    {
        if (DataContext is MainWindowViewModel viewModel)
        {
            await viewModel.PickFolderCommand.ExecuteAsync(this);
        }
    }
}
