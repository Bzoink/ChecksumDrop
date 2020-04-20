using System.Windows;
using System.Windows.Controls;

namespace ChecksumDrop
{
    /// <summary>
    /// Interaction logic for Settings.xaml
    /// </summary>
    public partial class Settings : Window
    {
        public Settings()
        {
            InitializeComponent();

            About1.Content = string.Format("{0} Version {1}.{2}",
                Properties.Resources.ProgramName,
                Properties.Resources.MajorVersion,
                Properties.Resources.MinorVersion);

            About2.Content = string.Format("©{0} {1} ({2})",
                Properties.Resources.PublicationYear,
                Properties.Resources.Author,
                Properties.Resources.Company);

            switch(Properties.User.Default.Method)
            {
                case "CRC32":
                    this.CRC32.IsChecked = true;
                    break;
                case "MD5":
                    this.MD5.IsChecked = true;
                    break;
                case "SHA1":
                    this.SHA1.IsChecked = true;
                    break;
                case "SHA256":
                    this.SHA256.IsChecked = true;
                    break;
                case "SHA384":
                    this.SHA384.IsChecked = true;
                    break;
                case "SHA512":
                    this.SHA512.IsChecked = true;
                    break;
                case "SHA3256":
                    this.SHA3256.IsChecked = true;
                    break;
                case "SHA3384":
                    this.SHA3384.IsChecked = true;
                    break;
                case "SHA3512":
                    this.SHA3512.IsChecked = true;
                    break;
            }
        }
        private void method_Checked(object sender, RoutedEventArgs e)
        {
            RadioButton radioButton = sender as RadioButton;
            Properties.User.Default.Method = radioButton.Name;
            Properties.User.Default.Save();
            Properties.User.Default.Upgrade();
        }
    }
}
