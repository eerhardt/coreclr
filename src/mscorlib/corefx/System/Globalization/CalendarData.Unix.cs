// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

namespace System.Globalization
{
    internal enum CalenderDataType
    {
        Uninitialized = 0,
        NativeName = 1,
        MonthDay = 2,
        ShortDates = 3,
        LongDates = 4,
        YearMonths = 5,
        DayNames = 6,
        AbbrevDayNames = 7,
        MonthNames = 8,
        AbbrevMonthNames = 9,
        SuperShortDayNames = 10,
        MonthGenitiveNames = 11,
        AbbrevMonthGenitiveNames = 12,
        EraNames = 13,
        AbbrevEraNames = 14,
    }

    internal partial class CalendarData
    {
        private bool LoadCalendarDataFromSystem(String localeName, CalendarId calendarId)
        {
            bool result = true;
            result &= GetCalendarInfo(localeName, calendarId, CalenderDataType.NativeName, out this.sNativeName);
            result &= GetCalendarInfo(localeName, calendarId, CalenderDataType.MonthDay, out this.sMonthDay);

            result &= EnumCalendarInfo(localeName, calendarId, CalenderDataType.ShortDates, out this.saShortDates);
            result &= EnumCalendarInfo(localeName, calendarId, CalenderDataType.LongDates, out this.saLongDates);
            result &= EnumCalendarInfo(localeName, calendarId, CalenderDataType.YearMonths, out this.saYearMonths);
            result &= EnumCalendarInfo(localeName, calendarId, CalenderDataType.DayNames, out this.saDayNames);
            result &= EnumCalendarInfo(localeName, calendarId, CalenderDataType.AbbrevDayNames, out this.saAbbrevDayNames);
            result &= EnumCalendarInfo(localeName, calendarId, CalenderDataType.MonthNames, out this.saMonthNames);
            result &= EnumCalendarInfo(localeName, calendarId, CalenderDataType.AbbrevMonthNames, out this.saAbbrevMonthNames);
            result &= EnumCalendarInfo(localeName, calendarId, CalenderDataType.SuperShortDayNames, out this.saSuperShortDayNames);
            result &= EnumCalendarInfo(localeName, calendarId, CalenderDataType.MonthGenitiveNames, out this.saMonthGenitiveNames);
            result &= EnumCalendarInfo(localeName, calendarId, CalenderDataType.AbbrevMonthGenitiveNames, out this.saAbbrevMonthGenitiveNames);
            result &= EnumCalendarInfo(localeName, calendarId, CalenderDataType.EraNames, out this.saEraNames);
            result &= EnumCalendarInfo(localeName, calendarId, CalenderDataType.AbbrevEraNames, out this.saAbbrevEraNames);

            return result;
        }

        // Get native two digit year max
        internal static int GetTwoDigitYearMax(CalendarId calendarId)
        {
            return Interop.GlobalizationInterop.GetTwoDigitYearMax((ushort)calendarId);
        }

        // Call native side to figure out which calendars are allowed
        internal static int GetCalendars(string localeName, bool useUserOverride, CalendarId[] calendars)
        {
            return 0;
        }

        private static bool SystemSupportsTaiwaneseCalendar()
        {
            return false;
        }

        // PAL Layer ends here

        private static bool GetCalendarInfo(string localeName, CalendarId calendarId, CalenderDataType dataType, out string calendarString)
        {
            const int stringSize = 80;
            StringBuilder stringBuilder = StringBuilderCache.Acquire(stringSize);

            bool result = Interop.GlobalizationInterop.GetCalendarInfo(localeName, calendarId, CalenderDataType.MonthDay, stringBuilder, stringSize);
            calendarString = StringBuilderCache.GetStringAndRelease(stringBuilder);

            return result;
        }

        private static bool EnumCalendarInfo(string localeName, CalendarId calendarId, CalenderDataType dataType, out string[] calendarData)
        {
            List<string> calendarDataList = new List<string>();
            GCHandle context = GCHandle.Alloc(calendarDataList);
            try
            {
                bool result = Interop.GlobalizationInterop.EnumCalendarInfo(EnumCalendarInfoCallback, localeName, calendarId, dataType, (IntPtr)context);
                calendarData = calendarDataList.ToArray();
                return result;
            }
            finally
            {
                context.Free();
            }
        }

        private static bool EnumCalendarInfoCallback(string calendarString, IntPtr context)
        {
            List<string> calendarDataList = (List<string>)((GCHandle)context).Target;
            calendarDataList.Add(calendarString);
            return true;
        }
    }
}