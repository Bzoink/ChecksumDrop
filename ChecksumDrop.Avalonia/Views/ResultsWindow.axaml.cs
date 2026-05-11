using global::Avalonia.Controls;

namespace ChecksumDrop.Avalonia.Views;

public partial class ResultsWindow : Window
{
    public ResultsWindow()
    {
        InitializeComponent();
    }

    private void CloseButton_OnClick(object? sender, global::Avalonia.Interactivity.RoutedEventArgs e)
    {
        Close();
    }
}
