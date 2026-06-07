using System;
using System.Globalization;
using Avalonia.Data.Converters;
using ChecksumDrop.Core.Models;

namespace ChecksumDrop.Avalonia.Converters;

public class EnumDisplayNameConverter : IValueConverter
{
    public object? Convert(object? value, Type targetType, object? parameter, CultureInfo culture)
    {
        if (value is ChecksumMethod method)
        {
            return method.ToDisplayName();
        }
        return value?.ToString();
    }

    public object? ConvertBack(object? value, Type targetType, object? parameter, CultureInfo culture)
    {
        throw new NotImplementedException();
    }
}
