using System.Linq;
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
        e.DragEffects = e.Data.Contains(DataFormats.Files)
            ? DragDropEffects.Copy
            : DragDropEffects.None;
    }

    private async void DropZone_OnDrop(object? sender, DragEventArgs e)
    {
        if (DataContext is not MainWindowViewModel viewModel)
        {
            return;
        }

        var files = e.Data.GetFiles();
        if (files is null)
        {
            return;
        }

        var items = files.ToList();
        if (items.Count > 0)
        {
            e.Handled = true;
            await viewModel.HandleDropAsync(this, items);
        }
    }

    private async void AboutButton_OnClick(object? sender, global::Avalonia.Interactivity.RoutedEventArgs e)
    {
        if (DataContext is MainWindowViewModel viewModel)
        {
            await viewModel.OpenAboutCommand.ExecuteAsync(this);
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
