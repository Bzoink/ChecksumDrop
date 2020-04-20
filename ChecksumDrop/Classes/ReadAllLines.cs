using System.IO;

namespace ChecksumDrop.Classes
{
    class ReadAllLines
    {
        private const char CR = '\r';
        private const char LF = '\n';
        private const char NULL = (char)0;

        public static long CountLines(string filePath)
        {
            var lineCount = 0L;

            var byteBuffer = new byte[1024 * 1024];
            var prevChar = NULL;
            var pendingTermination = false;

            int bytesRead;

            var file = new FileInfo(filePath);
            var stream = file.OpenRead();

            while ((bytesRead = stream.Read(byteBuffer, 0, byteBuffer.Length)) > 0)
            {
                for (var i = 0; i < bytesRead; i++)
                {
                    var currentChar = (char)byteBuffer[i];
                    switch (currentChar)
                    {
                        case NULL:
                        case LF when prevChar == CR:
                            continue;
                        case CR:
                        case LF when prevChar != CR:
                            lineCount++;
                            pendingTermination = false;
                            break;
                        default:
                            if (!pendingTermination)
                            {
                                pendingTermination = true;
                            }
                            break;
                    }
                    prevChar = currentChar;
                }
            }

            if (pendingTermination) { lineCount++; }
            return lineCount;
        }
    }
}

