using System;
using System.Collections.Generic;

namespace ChecksumDrop.Models
{
    public class Validation
    {
        public List<Run> Runs = new List<Run>();
    }

    public class Run
    {
        public DateTime Start { get; set; }
        public DateTime Finish { get; set; }
        public List<FileHash> FileHash = new List<FileHash>();
    }
}
