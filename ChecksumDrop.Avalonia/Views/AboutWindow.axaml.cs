using global::Avalonia.Controls;
using ChecksumDrop.Avalonia.ViewModels;

namespace ChecksumDrop.Avalonia.Views;

public partial class SettingsWindow : Window
{
    public SettingsWindow()
    {
        InitializeComponent();
    }

    private void CloseButton_OnClick(object? sender, global::Avalonia.Interactivity.RoutedEventArgs e)
    {
        if (DataContext is SettingsWindowViewModel viewModel)
        {
            Close(viewModel.SelectedMethod);
        }
        else
        {
            Close();
        }
    }
}
