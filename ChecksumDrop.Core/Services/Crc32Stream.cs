namespace ChecksumDrop.Core.Services;

public sealed class Crc32Stream(Stream stream) : Stream
{
    private readonly Stream _stream = stream;
    private static readonly uint[] Table = GenerateTable();
    private uint _readCrc = unchecked(0xFFFFFFFF);
    private uint _writeCrc = unchecked(0xFFFFFFFF);

    public override bool CanRead => _stream.CanRead;
    public override bool CanSeek => _stream.CanSeek;
    public override bool CanWrite => _stream.CanWrite;
    public override long Length => _stream.Length;

    public override long Position
    {
        get => _stream.Position;
        set => _stream.Position = value;
    }

    public uint ReadCrc => unchecked(_readCrc ^ 0xFFFFFFFF);
    public uint WriteCrc => unchecked(_writeCrc ^ 0xFFFFFFFF);

    public override void Flush() => _stream.Flush();
    public override long Seek(long offset, SeekOrigin origin) => _stream.Seek(offset, origin);
    public override void SetLength(long value) => _stream.SetLength(value);

    public override int Read(byte[] buffer, int offset, int count)
    {
        count = _stream.Read(buffer, offset, count);
        _readCrc = CalculateCrc(_readCrc, buffer, offset, count);
        return count;
    }

    public override void Write(byte[] buffer, int offset, int count)
    {
        _stream.Write(buffer, offset, count);
        _writeCrc = CalculateCrc(_writeCrc, buffer, offset, count);
    }

    public void ResetChecksum()
    {
        _readCrc = unchecked(0xFFFFFFFF);
        _writeCrc = unchecked(0xFFFFFFFF);
    }

    private static uint CalculateCrc(uint crc, byte[] buffer, int offset, int count)
    {
        unchecked
        {
            for (var i = offset; i < offset + count; i++)
            {
                crc = (crc >> 8) ^ Table[(crc ^ buffer[i]) & 0xFF];
            }
        }

        return crc;
    }

    private static uint[] GenerateTable()
    {
        unchecked
        {
            var table = new uint[256];
            const uint poly = 0xEDB88320;

            for (uint i = 0; i < table.Length; i++)
            {
                var crc = i;
                for (var j = 8; j > 0; j--)
                {
                    crc = (crc & 1) == 1 ? (crc >> 1) ^ poly : crc >> 1;
                }

                table[i] = crc;
            }

            return table;
        }
    }
}
