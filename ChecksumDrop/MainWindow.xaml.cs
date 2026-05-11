using ChecksumDrop.Classes;
using ChecksumDrop.Models;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Input;
using System.Windows.Interop;

namespace ChecksumDrop
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        BackgroundWorker worker;
        [DllImport("user32.dll")]
        private static extern IntPtr GetSystemMenu(IntPtr hWnd, bool bRevert);

        [DllImport("user32.dll")]
        private static extern bool EnableMenuItem(IntPtr hMenu, uint uIDEnableItem, uint uEnable);
        
        private const uint SC_CLOSE = 0xF060;
        private const uint MF_GRAYED = 0x00000001;
        private const uint MF_ENABLED = 0x00000000;

        public MainWindow()
        {
            InitializeComponent();
        }

        private void CancelButton_Click(object sender, RoutedEventArgs e)
        {
            MainLabel.Content = string.Empty;
            SubMainLabel.Content = string.Empty;
            SubSubMainLabel.Content = string.Format("Cancelling {0}", Hyphenator.Translate(FileList.Method));
            StatusText.Content = "Waiting for file handle ...";
            worker.CancelAsync();
        }

        private void OpenSettingsWinow(object sender, MouseButtonEventArgs e)
        {
            if (SettingsWindow.Handle == null)
            {
                IntPtr hSystemMenu = GetSystemMenu(new WindowInteropHelper(this).Handle, false);
                EnableMenuItem(hSystemMenu, SC_CLOSE, MF_GRAYED);

                SettingsWindow.Handle = new Settings();
                SettingsWindow.Handle.Topmost = true;
                SettingsWindow.Handle.Closing += (s, e) =>
                {
                    SettingsWindow.Handle = null;
                    EnableMenuItem(hSystemMenu, SC_CLOSE, MF_ENABLED);
                };

                SettingsWindow.Handle.Left = this.Left + (this.Width - SettingsWindow.Handle.Width) / 2;
                SettingsWindow.Handle.Top = this.Top + (this.Height - SettingsWindow.Handle.Height) / 2;
                SettingsWindow.Handle.Show();
            }
            else
            {
                SettingsWindow.Handle.Close();
            }
        }

        private void FileDropHandler(object sender, DragEventArgs e)
        {
            string[] instance = (string[])e.Data.GetData(DataFormats.FileDrop, false);
            List<string> fileCollection = new List<string>();
            FileList.Method = Properties.User.Default.Method;
            string thisHash = string.Empty;
            FileList.FileHash.Clear();
            FileList.Digests = 0;
            var shortestPath = new String('a', 4096);
            var numberOfFiles = 0;
            var numberOfDirectories = 0;
            double totalNumberOfItems = 0;
            int done = 0;
            bool allFilesAreDigest = true;

            MainLabel.Content = string.Empty;
            SubMainLabel.Content = string.Empty;
            SubSubMainLabel.Content = string.Format("Creating {0}", Hyphenator.Translate(FileList.Method));
            StatusText.Content = string.Empty;

            for (var i = 0; i < instance.Length; i++)
            {
                if (Path.GetDirectoryName(instance[i]).Length < shortestPath.Length)
                {
                    shortestPath = Path.GetDirectoryName(instance[i]);
                }
                if (Directory.Exists(instance[i]))
                {
                    var rangeCollection = Directory.GetFiles(instance[i], "*.*", SearchOption.AllDirectories);
                    fileCollection.AddRange(rangeCollection);
                    numberOfFiles += rangeCollection.Length;
                    numberOfDirectories++;
                }
                else
                {
                    fileCollection.Add(instance[i]);
                    numberOfFiles++;
                }
            }

            FileList.BasePath = shortestPath;
            Cancel.Visibility = Visibility.Visible;
            ProgressBar.Visibility = Visibility.Visible;
            ProgressBar.Value = 0;

            for (var i = 0; i < instance.Length; i++)
            {
                if (Seems.LikeADigest(instance[i]) == "None")
                {
                    allFilesAreDigest = false;
                    break;
                }
            }

            // All files are digest files, so validate them
            if (allFilesAreDigest)
            {
                worker = new BackgroundWorker();
                worker.DoWork += HashLoop_Validate;
                worker.ProgressChanged += ProgressReport;
                worker.RunWorkerCompleted += EndRunReport_Validate;
                worker.WorkerReportsProgress = true;
                worker.WorkerSupportsCancellation = true;
                worker.RunWorkerAsync();
            }
            else
            {
                worker = new BackgroundWorker();
                worker.DoWork += HashLoop_Create;
                worker.ProgressChanged += ProgressReport;
                worker.RunWorkerCompleted += EndRunReport_Create;
                worker.WorkerReportsProgress = true;
                worker.WorkerSupportsCancellation = true;
                worker.RunWorkerAsync();
            }

            void HashLoop_Validate(object sender, DoWorkEventArgs e)
            {
                BackgroundWorker worker = sender as BackgroundWorker;
                string line, hash, filename;
                List<string> invalidHex = new List<string>();

                foreach (var currentFile in fileCollection)
                {
                    FileList.Digests++;
                    FileList.Method = Seems.LikeADigest(currentFile);
                    done = 0;
                    totalNumberOfItems = ReadAllLines.CountLines(currentFile);
                    this.Dispatcher.Invoke(() =>
                    {
                        MainLabel.Content = string.Empty;
                        SubMainLabel.Content = string.Empty;
                        SubSubMainLabel.Content = string.Format("Checking {0} hash", Hyphenator.Translate(FileList.Method));
                        StatusText.Content = string.Format("Analyzing files in {0}", Path.GetFileName(currentFile));
                    });

                    StreamReader fileHandle = new StreamReader(currentFile);
                    while ((line = fileHandle.ReadLine()) != null)
                    {
                        if (worker.CancellationPending)
                        {
                            e.Cancel = true;
                            break;
                        }
                        worker.ReportProgress(done);
                        if (line.Length > FileList.HashLength())
                        {
                            if(FileList.Method == "SFV")
                            {
                                filename = line.Substring(0, line.Length - FileList.HashLength()).Trim();
                                hash = line.Substring(line.Length - FileList.HashLength());
                            }
                            else
                            {
                                filename = line.Substring(FileList.HashLength() + 1).Trim();
                                hash = line.Substring(0, FileList.HashLength());
                            }

                            if (OnlyHex.InString(hash))
                            {
                                var filePath = string.Format("{0}\\{1}", FileList.BasePath, filename);
                                // File exists
                                if (File.Exists(filePath))
                                {
                                    var thisHash = Hash.GetHash(filePath);
                                    // The hash in the textfile is the same as the hashed path
                                    if (thisHash == hash)
                                    {
                                        // Valid
                                        FileList.FileHash.Add(new FileHash { Filename = filename, Hash = hash, Status = 101, Method = FileList.Method });
                                    }
                                    else
                                    {
                                        // Invalid
                                        FileList.FileHash.Add(new FileHash { Filename = filename, Hash = hash, Status = 909, Method = FileList.Method });
                                    }
                                }
                                else
                                {
                                    // Filepath not found
                                    FileList.FileHash.Add(new FileHash { Filename = filename, Hash = hash, Status = 404, Method = FileList.Method });
                                }
                            }
                            else
                            {
                                // The hash in the textfile is not the same as the hashed path
                                // The line does not contain a hash, file value pair
                            }
                        }
                        done++;
                    }
                    fileHandle.Close();
                }
            }

            void EndRunReport_Validate(object sender, RunWorkerCompletedEventArgs e)
            {
                Cancel.Visibility = Visibility.Hidden;
                ProgressBar.Visibility = Visibility.Hidden;

                if (e.Cancelled)
                {
                    this.Dispatcher.Invoke(() =>
                    {
                        MainLabel.Content = "Drop files or folders here";
                        SubMainLabel.Content = "to create a checksum file";
                        SubSubMainLabel.Content = "Drop digest file to verify";
                        StatusText.Content = string.Empty;
                    });
                }
                else
                {
                    this.Dispatcher.Invoke(() =>
                    {
                        MainLabel.Content = string.Empty;
                        SubMainLabel.Content = string.Empty;
                        SubSubMainLabel.Content = "Done validation!";
                        StatusText.Content = string.Empty;
                    });
                    int totalFiles = FileList.FileHash.Count();
                    int totalFileNotFound = FileList.FileHash.Where(t => t.Status == 404).Count();
                    int totaValidHash = FileList.FileHash.Where(t => t.Status == 101).Count();
                    int totalInvalidHash = FileList.FileHash.Where(t => t.Status == 909).Count();
                    var resultsWindow = new Results();
                    resultsWindow.Results01.ItemsSource = FileList.HashTableOutput();
                    resultsWindow.ResultsTotalResults.Content = ((totalFileNotFound + totalInvalidHash) == 0) ? "All files are validated" : "There were errors!";
                    resultsWindow.ResultsTotalFiles.Content = (FileList.Digests == 1) ? totalFiles.ToString() : string.Format("{0} files in {1} digests", totalFiles, FileList.Digests);
                    resultsWindow.ResultsNotFound.Content = totalFileNotFound.ToString();
                    resultsWindow.ResultsNotValid.Content = totalInvalidHash.ToString();
                    resultsWindow.Show();
                }
            }

            void HashLoop_Create(object sender, DoWorkEventArgs e)
            {
                BackgroundWorker worker = sender as BackgroundWorker;
                totalNumberOfItems = fileCollection.Count;
                // FileList.FileHash.Clear();
                FileList.Digests = 1;
                foreach (var currentFile in fileCollection)
                {
                    if (worker.CancellationPending)
                    {
                        e.Cancel = true;
                        break;
                    }
                    worker.ReportProgress(done);
                    FileList.FileHash.Add(new FileHash { Filename = currentFile, Hash = Hash.GetHash(currentFile) });
                    done++;
                }
            }
            void EndRunReport_Create(object sender, RunWorkerCompletedEventArgs e)
            {
                Cancel.Visibility = Visibility.Hidden;
                ProgressBar.Visibility = Visibility.Hidden;

                if (e.Cancelled)
                {
                    MainLabel.Content = "Drop files or folders here";
                    SubMainLabel.Content = "to create a checksum file";
                    SubSubMainLabel.Content = "Drop digest file to verify";

                    StatusText.Content = string.Empty;
                }
                else
                {
                    string baseFilename = (FileList.Method == "CRC32") ? "checksum.sfv" : string.Format("checksum.{0}.txt", Hyphenator.Translate(FileList.Method));
                    string filename = string.Format("{0}\\{1}", FileList.BasePath, baseFilename);

                    try
                    {
                        TextWriter textWriter = new StreamWriter(filename);
                        textWriter.Write(TextOutput.Generate());
                        textWriter.Close();
                        MainLabel.Content = string.Empty;
                        SubMainLabel.Content = string.Empty;
                        SubSubMainLabel.Content = "Wrote checksum file";
                        StatusText.Content = string.Format("{0} of {1} files using {2}", baseFilename, fileCollection.Count, Hyphenator.Translate(FileList.Method));
                    }
                    catch (Exception)
                    {
                        MainLabel.Content = string.Empty;
                        SubMainLabel.Content = string.Empty;
                        SubSubMainLabel.Content = "Error";
                        StatusText.Content = string.Format("Could not save {0}", baseFilename);
                    }
                }
            }
            void ProgressReport (object sender, ProgressChangedEventArgs e)
            {
                var percent = (int)Math.Round(((double)e.ProgressPercentage / totalNumberOfItems) * 100);
                ProgressBar.Value = percent;
            }
        }

        private void Window_DragEnter(object sender, DragEventArgs e)
        {
            //
        }
    }
}
