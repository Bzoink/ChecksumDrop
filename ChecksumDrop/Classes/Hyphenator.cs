namespace ChecksumDrop.Classes
{
    public static class Hyphenator
    {
        public static string Translate(string digest)
        {
            string method;
            if(digest == "SFV")
            {
                method = "crc32";
            }
            else if (digest.Length == 7 && digest.Substring(0, 4).ToUpper() == "SHA3")
            {
                method = digest.Insert(4, "-").ToLower();
            }
            else
            {
                method = digest.ToLower();
            }
            return method;
        }
    }
}
