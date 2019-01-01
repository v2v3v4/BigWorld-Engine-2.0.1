/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "log_string_interpolator.hpp"

#include "log_string_printer.hpp"
#include "log_string_writer.hpp"


DECLARE_DEBUG_COMPONENT( 0 );


namespace
{

// Corresponds to MESSAGE_LOGGER_VERSION from
// src/lib/network/logger_message_forwarder.hpp
const uint8 MESSAGE_LOGGER_VERSION_64BIT = 7;

}

LogStringInterpolator::LogStringInterpolator( const std::string &fmt ) :
	fmt_( fmt ), fileOffset_( 0xffffffff )
{
	handleFormatString( fmt_.c_str(), *this );
}


void LogStringInterpolator::onString( int start, int end )
{
	stringOffsets_.push_back( StringOffset( start, end ) );
	components_.push_back( 's' );
}


void LogStringInterpolator::onToken( char type, int cflags, int min,
	int max, int flags, uint8 base, int vflags )
{
	fmtData_.push_back( FormatData( type, cflags, base, min,
			max, flags, vflags ) );
	components_.push_back( 't' );
}


void LogStringInterpolator::write( FileStream &fs )
{
	if (fileOffset_ != 0xffffffff)
		return;

	fileOffset_ = fs.tell();
	fs << fmt_ << components_ << stringOffsets_ << fmtData_;
	fs.commit();
}


void LogStringInterpolator::read( FileStream &fs )
{
	fileOffset_ = fs.tell();
	fs >> fmt_ >> components_ >> stringOffsets_ >> fmtData_;
}


bool LogStringInterpolator::streamToString( BinaryIStream &is,
	std::string &str, uint8 version /*= MESSAGE_LOGGER_VERSION */ )
{
	static LogStringPrinter printer;
	printer.setResultString( str );

	return this->interpolate( printer, is, version );
}


bool LogStringInterpolator::streamToLog( LogStringWriter &writer, 
		BinaryIStream &is, uint8 version /*= MESSAGE_LOGGER_VERSION */ )
{
	if (!this->interpolate( writer, is, version ))
	{
		return false;
	}

	return writer.isGood();
}


template < class Handler >
bool LogStringInterpolator::interpolate( Handler &handler, 
		BinaryIStream &is, uint8 version )
{
	StringOffsetList::iterator sit = stringOffsets_.begin();
	std::vector< FormatData >::iterator fit = fmtData_.begin();

	for (unsigned i=0; i < components_.size(); i++)
	{
		if (components_[i] == 's')
		{
			StringOffset &so = *sit++;
			handler.onFmtStringSection( fmt_, so.start_, so.end_ );
		}
		else
		{
			FormatData &fd = *fit++;

			// Macro to terminate parsing on stream error
#			define CHECK_STREAM()											\
			if (is.error())													\
			{																\
				ERROR_MSG( "Stream too short for '%s'\n", fmt_.c_str() );	\
				handler.onError();											\
				return false;												\
			}

			if (fd.vflags_ & VARIABLE_MIN_WIDTH)
			{
				WidthType w; is >> w; CHECK_STREAM();
				handler.onMinWidth( w, fd );
			}
			if (fd.vflags_ & VARIABLE_MAX_WIDTH)
			{
				WidthType w; is >> w; CHECK_STREAM();
				handler.onMaxWidth( w, fd );
			}

			switch (fd.type_)
			{
			case 'd':
			{
				switch (fd.cflags_)
				{
				case DP_C_SHORT:
				{
					int16 val; is >> val; CHECK_STREAM();
					handler.onInt( val, fd );
					break;
				}

				case DP_C_LONG:
				{
					int64 finalVal;
					if (version < MESSAGE_LOGGER_VERSION_64BIT)
					{
						int32 val;
						is >> val; CHECK_STREAM();

						// We've just read from a 32-bit version client, but
						// write it out as if it were 64-bit version.
						finalVal = val;
					}
					else
					{
						is >> finalVal; CHECK_STREAM();
					}
					handler.onInt( (int64)finalVal, fd );
					break;
				}

				case DP_C_LLONG:
				{
					int64 val; is >> val; CHECK_STREAM();
					handler.onInt( val, fd );
					break;
				}

				default:
				{
					int32 val; is >> val; CHECK_STREAM();
					handler.onInt( val, fd );
					break;
				}
				}
				break;
			}

			case 'o':
			case 'u':
			case 'x':
			{
				switch (fd.cflags_)
				{
				case DP_C_SHORT:
				{
					uint16 val; is >> val; CHECK_STREAM();
					handler.onInt( val, fd );
					break;
				}

				case DP_C_LONG:
				{
					uint64 finalVal;

					if (version < MESSAGE_LOGGER_VERSION_64BIT)
					{
						uint32 val;
						is >> val; CHECK_STREAM();

						// We've just read from a 32-bit version client, but
						// write it out as if it were 64-bit version.
						finalVal = val;
					}
					else
					{
						is >> finalVal; CHECK_STREAM();
					}

					handler.onInt( finalVal, fd );
					break;
				}

				case DP_C_LLONG:
				{
					uint64 val; is >> val; CHECK_STREAM();
					handler.onInt( val, fd );
					break;
				}

				default:
				{
					uint32 val; is >> val; CHECK_STREAM();
					handler.onInt( val, fd );
					break;
				}
				}
				break;
			}

			case 'f':
			case 'e':
			case 'g':
			{
				switch (fd.cflags_)
				{
				case DP_C_LDOUBLE:
				{
					LDOUBLE val; is >> val; CHECK_STREAM();
					handler.onFloat( val, fd );
					break;
				}
				default:
				{
					double val; is >> val; CHECK_STREAM();
					handler.onFloat( val, fd );
					break;
				}
				}
				break;
			}

			case 's':
			{
				std::string val; is >> val; CHECK_STREAM();
				handler.onString( val.c_str(), fd );
				break;
			}

			case 'p':
			{
				int64 ptr; is >> ptr; CHECK_STREAM();
				handler.onPointer( ptr, fd );
				break;
			}

			case 'c':
			{
				char c; is >> c; CHECK_STREAM();
				handler.onChar( c, fd );
				break;
			}

			default:
				ERROR_MSG( "Unhandled format: '%c'\n", fd.type_ );
				handler.onError();
				return false;
			}
		}
	}

	handler.onParseComplete();

	return true;
}
