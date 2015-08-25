//
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
//

#include <stdint.h>
#include <uchar.h>
#include <unicode/unorm2.h>
#include <unicode/ucal.h>
#include <unicode/smpdtfmt.h>
#include <unicode/dtfmtsym.h>
#include <unicode/dtptngen.h>
#include <unicode/locdspnm.h>
#include "locale.hpp"

/*
* These values should be kept in sync with System.Globalization.CalendarId
*/
enum CalendarId : int32_t
{
	UNINITIALIZED_VALUE = 0,
	GREGORIAN = 1,     // Gregorian (localized) calendar
	GREGORIAN_US = 2,     // Gregorian (U.S.) calendar
	JAPAN = 3,     // Japanese Emperor Era calendar
				   /* SSS_WARNINGS_OFF */
	TAIWAN = 4,     // Taiwan Era calendar /* SSS_WARNINGS_ON */ 
	KOREA = 5,     // Korean Tangun Era calendar
	HIJRI = 6,     // Hijri (Arabic Lunar) calendar
	THAI = 7,     // Thai calendar
	HEBREW = 8,     // Hebrew (Lunar) calendar
	GREGORIAN_ME_FRENCH = 9,     // Gregorian Middle East French calendar
	GREGORIAN_ARABIC = 10,     // Gregorian Arabic calendar
	GREGORIAN_XLIT_ENGLISH = 11,     // Gregorian Transliterated English calendar
	GREGORIAN_XLIT_FRENCH = 12,
	// Note that all calendars after this point are MANAGED ONLY for now.
	JULIAN = 13,
	JAPANESELUNISOLAR = 14,
	CHINESELUNISOLAR = 15,
	SAKA = 16,     // reserved to match Office but not implemented in our code
	LUNAR_ETO_CHN = 17,     // reserved to match Office but not implemented in our code
	LUNAR_ETO_KOR = 18,     // reserved to match Office but not implemented in our code
	LUNAR_ETO_ROKUYOU = 19,     // reserved to match Office but not implemented in our code
	KOREANLUNISOLAR = 20,
	TAIWANLUNISOLAR = 21,
	PERSIAN = 22,
	UMALQURA = 23,
	LAST_CALENDAR = 23      // Last calendar ID
};

/*
* These values should be kept in sync with System.Globalization.CalendarDataType
*/
enum CalenderDataType : int32_t
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
};

typedef bool(*EnumCalendarInfoCallback)(const UChar*, void*);

const char *GetCalendarName(CalendarId calendarId)
{
	switch (calendarId)
	{
	case JAPAN:
		return "japanese";
	case THAI:
		return "buddhist";
	case HEBREW:
		return "hebrew";
	case CHINESELUNISOLAR:
	case KOREANLUNISOLAR:
	case JAPANESELUNISOLAR:
	case TAIWANLUNISOLAR:
		return "chinese";
	case PERSIAN:
		return "persian";
	case HIJRI:
	case UMALQURA:
		return "islamic";
	case GREGORIAN:
	case GREGORIAN_US:
	case GREGORIAN_ARABIC:
	case GREGORIAN_ME_FRENCH:
	case GREGORIAN_XLIT_ENGLISH:
	case GREGORIAN_XLIT_FRENCH:
	case KOREA:
	case JULIAN:
	case LUNAR_ETO_CHN:
	case LUNAR_ETO_KOR:
	case LUNAR_ETO_ROKUYOU:
	case SAKA:
	case TAIWAN:
	default:
		return "gregorian";
	}
}

extern "C" int32_t GetTwoDigitYearMax(CalendarId calendarId)
{
	switch (calendarId)
	{
	case JAPAN:
	case TAIWAN:
		return -1;
	default:
		break;
	}

	UnicodeString localeNameStart("@calendar=");
	UnicodeString calendarName(GetCalendarName(calendarId));
	UnicodeString localeName = localeNameStart.append(calendarName);

	Locale locale = GetLocale(localeName.getTerminatedBuffer());

	UErrorCode err = U_ZERO_ERROR;
	UnicodeString pattern("MMddYYYY");
	SimpleDateFormat format(pattern, locale, err);

	if (U_SUCCESS(err))
	{
		UDate twoDigitYearStart = format.get2DigitYearStart(err);
		if (U_SUCCESS(err))
		{
			Calendar* cal = Calendar::createInstance(locale, err);
			cal->setTime(twoDigitYearStart, err);

			// ICU returns the start of the 2 digit range, but the Calendar class needs it
			// to be the end (or "Max"), so add 99 years

			cal->add(UCAL_YEAR, 99, err);
			int32_t result = cal->get(UCAL_YEAR, err);
			delete cal;
			return result;
		}
	}

	return -1;
}

bool GetMonthDayPattern(Locale& locale, UChar* sMonthDay, int32_t stringCapacity)
{
	UErrorCode err = U_ZERO_ERROR;
	LocalPointer<DateTimePatternGenerator> generator(DateTimePatternGenerator::createInstance(locale, err));
	UnicodeString monthDayPattern = generator->getBestPattern(UnicodeString("MMMMd"), err);

	if (!U_SUCCESS(err))
		return false;

	monthDayPattern.extract(sMonthDay, stringCapacity, err);

	return U_SUCCESS(err);
}

bool GetNativeCalendarName(Locale& locale, CalendarId calendarId, UChar* nativeName, int32_t stringCapacity)
{
	LocalPointer<LocaleDisplayNames> displayNames(LocaleDisplayNames::createInstance(locale));

	UnicodeString calendarName;
	displayNames->keyValueDisplayName("calendar", GetCalendarName(calendarId), calendarName);

	UErrorCode err = U_ZERO_ERROR;
	calendarName.extract(nativeName, stringCapacity, err);

	return U_SUCCESS(err);
}

extern "C" bool GetCalendarInfo(UChar* localeName, CalendarId calendarId, CalenderDataType dataType, UChar* result, int32_t resultCapacity)
{
	Locale locale = GetLocale(localeName);
	if (locale.isBogus())
		return false;

	if (dataType == CalenderDataType::NativeName)
	{
		return GetNativeCalendarName(locale, calendarId, result, resultCapacity);
	}

	if (dataType == CalenderDataType::MonthDay)
	{
		return GetMonthDayPattern(locale, result, resultCapacity);
	}

	return false;
}

bool InvokeCallbackForDateTimePattern(Locale& locale, const char* patternSkeleton, EnumCalendarInfoCallback callback, void* context)
{
	UErrorCode err = U_ZERO_ERROR;
	LocalPointer<DateTimePatternGenerator> generator(DateTimePatternGenerator::createInstance(locale, err));
	UnicodeString pattern = generator->getBestPattern(UnicodeString(patternSkeleton), err);
	if (U_SUCCESS(err))
	{
		callback(pattern.getTerminatedBuffer(), context);
		return true;
	}

	return false;
}

bool EnumCalendarArray(const UnicodeString* srcArray, int32_t srcArrayCount, EnumCalendarInfoCallback callback, void* context)
{
	bool result = true;

	for (int i = 0; i < srcArrayCount; i++)
	{
		UnicodeString src = srcArray[i];
		result &= callback(src.getTerminatedBuffer(), context);
	}

	return result;
}

bool EnumWeekdays(
	DateFormatSymbols& dateFormatSymbols,
	DateFormatSymbols::DtContextType dtContext,
	DateFormatSymbols::DtWidthType dtWidth,
	EnumCalendarInfoCallback callback,
	void* context)
{
	int32_t daysCount;
	const UnicodeString* dayNames = dateFormatSymbols.getWeekdays(daysCount, dtContext, dtWidth);

	// ICU returns an empty string for the first/zeroth element in the weekdays array.
	// So skip the first element.
	dayNames++;

	return EnumCalendarArray(dayNames, daysCount, callback, context);
}

bool EnumMonths(
	DateFormatSymbols& dateFormatSymbols,
	DateFormatSymbols::DtContextType dtContext,
	DateFormatSymbols::DtWidthType dtWidth,
	EnumCalendarInfoCallback callback,
	void* context)
{
	int32_t monthsCount;
	const UnicodeString* monthNames = dateFormatSymbols.getMonths(monthsCount, dtContext, dtWidth);
	return EnumCalendarArray(monthNames, monthsCount, callback, context);
}

extern "C" bool EnumCalendarInfo(
	EnumCalendarInfoCallback callback,
	UChar* localeName,
	CalendarId calendarId,
	CalenderDataType dataType,
	void* context)
{
	UErrorCode err = U_ZERO_ERROR;

	Locale locale = GetLocale(localeName);
	if (locale.isBogus())
		return false;

	if (dataType == CalenderDataType::ShortDates)
	{
		return InvokeCallbackForDateTimePattern(locale, "Mdyyyy", callback, context);
	}

	if (dataType == CalenderDataType::LongDates)
	{
		// TODO: need to replace the "EEEE"s with "dddd"s for .net
		// Also, "LLLL"s to "MMMM"s
		return InvokeCallbackForDateTimePattern(locale, "eeeeMMMMddyyyy", callback, context);
	}

	if (dataType == CalenderDataType::YearMonths)
	{
		return InvokeCallbackForDateTimePattern(locale, "yyyyMMMM", callback, context);
	}

	DateFormatSymbols dateFormatSymbols(locale, GetCalendarName(calendarId), err);
	if (!U_SUCCESS(err))
		return false;

	if (dataType == CalenderDataType::DayNames)
	{
		return EnumWeekdays(
			dateFormatSymbols,
			DateFormatSymbols::STANDALONE,
			DateFormatSymbols::WIDE,
			callback,
			context);
	}

	if (dataType == CalenderDataType::AbbrevDayNames)
	{
		return EnumWeekdays(
			dateFormatSymbols,
			DateFormatSymbols::STANDALONE,
			DateFormatSymbols::ABBREVIATED,
			callback,
			context);
	}

	if (dataType == CalenderDataType::MonthNames)
	{
		return EnumMonths(
			dateFormatSymbols,
			DateFormatSymbols::STANDALONE,
			DateFormatSymbols::WIDE,
			callback,
			context);
	}

	if (dataType == CalenderDataType::AbbrevMonthNames)
	{
		return EnumMonths(
			dateFormatSymbols,
			DateFormatSymbols::STANDALONE,
			DateFormatSymbols::ABBREVIATED,
			callback,
			context);
	}

	if (dataType == CalenderDataType::SuperShortDayNames)
	{
		return EnumWeekdays(
			dateFormatSymbols,
			DateFormatSymbols::STANDALONE,
			DateFormatSymbols::SHORT,
			callback,
			context);
	}

	if (dataType == CalenderDataType::MonthGenitiveNames)
	{
		return EnumMonths(
			dateFormatSymbols,
			DateFormatSymbols::FORMAT,
			DateFormatSymbols::WIDE,
			callback,
			context);
	}

	if (dataType == CalenderDataType::AbbrevMonthGenitiveNames)
	{
		return EnumMonths(
			dateFormatSymbols,
			DateFormatSymbols::FORMAT,
			DateFormatSymbols::ABBREVIATED,
			callback,
			context);
	}

	if (dataType == CalenderDataType::EraNames)
	{
		// NOTE: On Windows, the EraName is "A.D." and AbbrevEraName is "AD".
		// But ICU/CLDR only supports "Anno Domini", "AD", and "A".
		// So returning getEras (i.e. "AD") for both EraNames and AbbrevEraNames.
		int32_t eraNameCount;
		const UnicodeString* eraNames = dateFormatSymbols.getEras(eraNameCount);
		return EnumCalendarArray(eraNames, eraNameCount, callback, context);
	}

	if (dataType == CalenderDataType::AbbrevEraNames)
	{
		int32_t eraNameCount;
		const UnicodeString* eraNames = dateFormatSymbols.getEras(eraNameCount);
		return EnumCalendarArray(eraNames, eraNameCount, callback, context);
	}

	return false;
}
