using global::Avalonia.Controls;
using ChecksumDrop.Avalonia.ViewModels;

namespace ChecksumDrop.Avalonia.Views;

public partial class AboutWindow : Window
{
    public AboutWindow()
    {
        InitializeComponent();
    }

    private void CloseButton_OnClick(object? sender, global::Avalonia.Interactivity.RoutedEventArgs e)
    {
        Close();
    }
}
