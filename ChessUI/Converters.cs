using System;
using System.Globalization; // Required for Converter
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;     // Required for Converter

namespace Chess
{
    // Boolean-to-visibility converter
    [ValueConversion(typeof(bool), typeof(Visibility))]
    public class BooleanToVisibilityConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            bool flag = false;
            if (value is bool b)
            {
                flag = b;
            }
            else if (value is bool?)
            {
                bool? nullableFlag = (bool?)value;
                flag = nullableFlag.HasValue ? nullableFlag.Value : false;
            }
            //Invert logic if parameter is set (e.g., parameter="invert")
            bool invert = parameter?.ToString().Equals("invert", StringComparison.OrdinalIgnoreCase) ?? false;
            if (invert) flag = !flag;

            return flag ? Visibility.Visible : Visibility.Collapsed;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotSupportedException();
        }
    }

    // Boolean-to-opacity converter
    [ValueConversion(typeof(bool), typeof(double))]
    public class BooleanToOpacityConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (value is bool booleanValue)
            {
                if (parameter?.ToString() == "invert")
                {
                    return booleanValue ? 0.5 : 1.0;
                }
                return booleanValue ? 1.0 : 0.5;
            }
            return DependencyProperty.UnsetValue;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
