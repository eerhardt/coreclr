// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

using System;
using System.Globalization;
using System.Runtime.InteropServices;
using System.Text;

internal static partial class Interop
{
    internal delegate bool EnumCalendarInfoCallback(
        [MarshalAs(UnmanagedType.LPWStr)] string calendarString,
        IntPtr context);

    internal static partial class GlobalizationInterop
    {
        [DllImport(Libraries.GlobalizationInterop, CharSet = CharSet.Unicode)]
        internal static extern ushort GetTwoDigitYearMax(ushort calendarId);

        [DllImport(Libraries.GlobalizationInterop, CharSet = CharSet.Unicode)]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool GetCalendarInfo(string localeName, CalendarId calendarId, CalenderDataType calendarDataType, StringBuilder result, int resultCapacity);

        [DllImport(Libraries.GlobalizationInterop, CharSet = CharSet.Unicode)]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool EnumCalendarInfo(EnumCalendarInfoCallback callback, string localeName, CalendarId calendarId, CalenderDataType calendarDataType, IntPtr context);
    }
}
