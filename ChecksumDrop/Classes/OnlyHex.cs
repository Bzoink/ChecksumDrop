namespace ChecksumDrop.Classes
{
    static class OnlyHex
    {
        public static bool InString(string test)
        {
            return System.Text.RegularExpressions.Regex.IsMatch(test, @"\A\b[0-9a-fA-F]+\b\Z");
        }
    }
}
