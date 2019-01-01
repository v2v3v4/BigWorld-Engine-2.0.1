/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DATE_TIME_UTILS_HPP
#define DATE_TIME_UTILS_HPP


/**
 *	This class helps in formating time and dates.
 */
class DateTimeUtils
{
public:
	struct DateTime
	{
		uint16 year;
		uint8 month;
		uint8 day;
		uint8 hour;
		uint8 minute;
		uint8 second;
	};

	static void format( std::string & retFormattedTime,
						time_t time, bool fullDateFormat = false );

	static bool parse( DateTime & retDateTime,
						const std::string & formattedTime );
};

#endif // DATE_TIME_UTILS_HPP
