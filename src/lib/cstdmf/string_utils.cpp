/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"
#include "string_utils.hpp"

#include "stdmf.hpp"
#include "smartpointer.hpp"

#include <stdarg.h>

//------------------------------------------------------------------------------
std::string bw_format( const char * format, ... )
{
	char buffer[ BUFSIZ << 2 ];

	va_list argPtr;
	va_start( argPtr, format );
	bw_vsnprintf( buffer, sizeof(buffer), format, argPtr );
	buffer[ sizeof( buffer ) - 1 ] = '\0';
	va_end( argPtr );

	return std::string( buffer );
}

#ifdef _WIN32
//------------------------------------------------------------------------------
bool bw_acptoutf8( const char * src, std::string& output )
{
	int len = MultiByteToWideChar( CP_ACP, 0, src, -1, NULL, 0 );
	if (len <= 0)
	{
		return false;
	}
	else
	{
		// MultiByteToWideChar will process the \0 at the end of the source,
		//  so len will contain that too, meaning that the output size WILL
		//  include the \0 at the end, which breaks string concatenation,
		//  since they will be put AFTER the \0. So, after processing the string
		//  we remove the \0 of the output.

		std::wstring temp;
		temp.resize( len );
		int res = MultiByteToWideChar( CP_ACP, 0, src, -1, &temp[0], len );
		temp.resize( temp.length() - 1 );
		return bw_wtoutf8( temp, output );
	}
}


//------------------------------------------------------------------------------
bool bw_utf8toacp( const char * src, std::string& output )
{
	int len = MultiByteToWideChar( CP_UTF8, 0, src, -1, NULL, 0 );
	if (len <= 0)
	{
		return false;
	}
	else
	{
		// MultiByteToWideChar will process the \0 at the end of the source,
		//  so len will contain that too, meaning that the output size WILL
		//  include the \0 at the end, which breaks string concatenation,
		//  since they will be put AFTER the \0. So, after processing the string
		//  we remove the \0 of the output.

		std::wstring temp;
		temp.resize( len );
		int res = MultiByteToWideChar( CP_UTF8, 0, src, -1, &temp[0], len );
		temp.resize( temp.length() - 1 );
		return bw_wtoacp( temp, output );
	}
}


//------------------------------------------------------------------------------
bool bw_acptow( const char * src, std::wstring& output )
{
	int len = MultiByteToWideChar( CP_ACP, 0, src, -1, NULL, 0 );
	if (len <= 0)
	{
		return false;
	}
	else
	{
		// MultiByteToWideChar will process the \0 at the end of the source,
		//  so len will contain that too, meaning that the output size WILL
		//  include the \0 at the end, which breaks string concatenation,
		//  since they will be put AFTER the \0. So, after processing the string
		//  we remove the \0 of the output.

		output.resize( len );
		int res = MultiByteToWideChar( CP_ACP, 0, src, -1, &output[0], len );
		output.resize( output.length() - 1 );
		return res != 0;
	}
}

//------------------------------------------------------------------------------
bool bw_wtoacp( const wchar_t * wsrc, std::string& output )
{
	int len = WideCharToMultiByte( CP_ACP, 0, wsrc, -1, NULL, 0, NULL, NULL );
	if (len <= 0)
	{
		return false;
	}
	else
	{
		output.resize( len );
		int res = WideCharToMultiByte( CP_ACP, 0, wsrc, -1, &output[0], len, NULL, NULL );
		output.resize( output.length() - 1 );
		return res != 0;
	}
}

//------------------------------------------------------------------------------
bool bw_utf8tow( const char * src, std::wstring& output )
{
	int len = MultiByteToWideChar( CP_UTF8, 0, src, -1, NULL, 0 );
	if (len <= 0)
	{
		return false;
	}
	else
	{
		output.resize( len );
		int res = MultiByteToWideChar( CP_UTF8, 0, src, -1, &output[0], len );
		output.resize( output.length() - 1 );
		return res != 0;
	}
}

//------------------------------------------------------------------------------
bool bw_wtoutf8( const wchar_t * wsrc, std::string& output )
{
	int len = WideCharToMultiByte( CP_UTF8, 0, wsrc, -1, NULL, 0, NULL, NULL );
	if (len <= 0)
	{
		return false;
	}
	else
	{
		output.resize( len );
		int res = WideCharToMultiByte( CP_UTF8, 0, wsrc, -1, &output[0], len, NULL, NULL );
		output.resize( output.length() - 1 );
		return res != 0;
	}
}

//------------------------------------------------------------------------------
bool bw_utf8tow( const char * src, size_t srcSize, wchar_t * dst, size_t dstSize )
{
	if (dstSize)
	{
		int res = MultiByteToWideChar( CP_UTF8, 0, src, srcSize, dst, dstSize );

		if (res >= 0)
		{
			dst[ res ] = L'\0';

			return true;
		}
	}

	return false;
}


//------------------------------------------------------------------------------
bool bw_wtoutf8( const wchar_t * src, size_t srcSize, char * dst, size_t dstSize )
{
	if (dstSize)
	{
		int res = WideCharToMultiByte( CP_UTF8, 0, src, srcSize, dst, dstSize, NULL, NULL );

		if (res >= 0)
		{
			dst[ res ] = '\0';

			return true;
		}
	}

	return false;
}

#else // ! _WIN32


#include <iconv.h>
#include <errno.h>
#include <string.h>


class IConv;
typedef SmartPointer< IConv > IConvPtr;


/**
 *	Wrapper class around the glibc iconv() family of functions.
 *
 *	Ensures that iconv_t handles are released when objects of this class go out
 *	of scope.
 */
class IConv : public ReferenceCount
{
public:
	~IConv() { iconv_close( iConvDesc_ ); }

	/**
	 *	Wrapper around the iconv_open() function.
	 */
	static IConvPtr create( const char * toEncoding, const char * fromEncoding );


	/**
	 *	Wrapper around the iconv() function.
	 */
	size_t convert( const char ** inBytes, size_t * inBytesLeft,
			char ** outBytes, size_t * outBytesLeft )
	{
		// glibc iconv doesn't support const char ** type for inbytes parameter.
		return iconv( iConvDesc_, const_cast< char ** >( inBytes ), inBytesLeft, 
			outBytes, outBytesLeft );
	}
	
private:
	IConv( iconv_t iConvDesc ):
		ReferenceCount(),
		iConvDesc_( iConvDesc )
	{}

private:
	iconv_t iConvDesc_;
};

IConvPtr IConv::create( const char * toEncoding, const char * fromEncoding )
{
	iconv_t iconvDesc = iconv_open( toEncoding, fromEncoding );
	if (iconvDesc == iconv_t( -1 ))
	{
		return NULL;
	}
	return new IConv( iconvDesc );
}


/**
 *	Convert a UTF8 string to a STL wide character string.
 *
 *	@param src 		The source UTF8 string buffer.
 *	@param output 	The output string.
 *
 *	@return 		Whether the conversion succeeded or not.
 */
bool bw_utf8tow( const char * src, std::wstring& output )
{
	size_t bufSize = BUFSIZ;
	bool finished = false;
	bool success = false;

	IConvPtr pIConv = IConv::create( "WCHAR_T", "UTF-8" );
	MF_ASSERT( pIConv.get() != NULL );

	size_t srcLen = strlen( src ) + 1;
	char * buf = NULL;
	do
	{
		if (buf) delete [] buf;
		buf = new char[bufSize];

		const char * inBuf = src;
		size_t inBytesLeft = srcLen;
		char * outBuf = buf;
		size_t outBytesLeft = bufSize;

		size_t res = pIConv->convert( &inBuf, &inBytesLeft, 
			&outBuf, &outBytesLeft );

		if (res == size_t( -1 ))
		{
			if (errno != E2BIG)
			{
				// Invalid conversion.
				finished = true;
				success = false;
			}

			// Not enough buffer to convert into, resize.
			bufSize *= 2;
		}
		else
		{
			// We're done!
			finished = true;
			success = true;
		}

	} while (!finished);

	if (success)
	{
		wchar_t * wbuf = reinterpret_cast< wchar_t * >( buf );
		output.assign( wbuf, wcslen( wbuf ) );
	}

	if (buf) delete [] buf;

	return success;
}

/**
 *	Convert a wide character string to a UTF8 string.
 *	
 *	@param wsrc 		The source wide character string.
 *	@param output 		A string to be filled with the corresponding UTF-8 string.
 *
 *	@return 			Whether the conversion succeeded.
 */
bool bw_wtoutf8( const wchar_t * wsrc, std::string& output )
{
	size_t bufSize = BUFSIZ;
	bool finished = false;
	bool success = false;

	IConvPtr pIConv = IConv::create( "UTF-8", "WCHAR_T" );
	size_t srcLen = (wcslen( wsrc ) + 1) * sizeof( wchar_t );
	char * buf = NULL;
	
	do
	{
		if (buf) delete [] buf;
		buf = new char[bufSize];
		const char *inBuf = reinterpret_cast< const char * >( wsrc );
		size_t inBytesLeft = srcLen;
		char *outBuf = buf;
		size_t outBytesLeft = bufSize;
		
		size_t res = pIConv->convert( &inBuf, &inBytesLeft, 
				&outBuf, &outBytesLeft );
		if (res == size_t( -1 ))
		{
			if (errno != E2BIG)
			{
				// Invalid conversion.
				finished = true;
				success = false;
			}

			// Not enough buffer to convert into, resize.
			bufSize *= 2;
		}
		else
		{
			// We're done!
			finished = true;
			success = true;
		}

	} while (!finished);

	if (success)
	{
		output.assign( buf, strlen( buf ) );
	}

	if (buf) delete [] buf;

	return success;
}


/**
 *	Convert a wide character string to the corresponding UTF-8 string.
 *
 *	@param src 		The source wide character string.
 *	@param srcSize 	The size of the wide character string, in bytes.
 *	@param dst 		The destination character buffer. This is filled with the
 *					UTF-8 string.
 *	@param dstSize 	The size of the destination character buffer, in bytes.
 */
bool bw_utf8tow( const char * src, size_t srcSize, wchar_t * wDst, 
		size_t dstSize )
{
	IConvPtr pIConv = IConv::create( "WCHAR_T", "UTF-8" );
	
	char * dst = reinterpret_cast< char * >( wDst );
	return pIConv->convert( &src, &srcSize, &dst, &dstSize ) != size_t( -1 );
}


/**
 *	Convert a wide character string to the corresponding UTF-8 string.
 *
 *	@param src 		The source wide character string.
 *	@param srcSize 	The size of the wide character string, in bytes.
 *	@param dst 		The destination character buffer. This is filled with the
 *					UTF-8 string.
 *	@param dstSize 	The size of the destination character buffer, in bytes.
 */
bool bw_wtoutf8( const wchar_t * wSrc, size_t srcSize, char * dst, 
		size_t dstSize )
{
	IConvPtr pIConv = IConv::create( "UTF-8", "WCHAR_T" );

	const char * src = reinterpret_cast< const char * >( wSrc );
	return pIConv->convert( &src, &srcSize, &dst, &dstSize ) != size_t( -1 );
}
#endif


//------------------------------------------------------------------------------
void bw_ansitow( const char * src, std::wstring & dst )
{
	size_t len = strlen( src );
	dst.resize( len );
	for ( size_t idx = 0 ; idx < len ; ++idx )
		dst[ idx ] = src[ idx ];
}


//------------------------------------------------------------------------------
int bw_MW_strcmp ( const wchar_t * lhs, const char * rhs )
{
	int ret = 0;

	while( ! (ret = (int)(*lhs - *rhs)) && *rhs)
	{
		++lhs, ++rhs;
	}

	if ( ret < 0 )
		ret = -1 ;
	else if ( ret > 0 )
		ret = 1 ;

	return( ret );
}

//------------------------------------------------------------------------------
int bw_MW_strcmp ( const char * lhs, const wchar_t * rhs )
{
	int ret = 0;

	while( ! (ret = (int)(*lhs - *rhs)) && *rhs)
	{
		++lhs, ++rhs;
	}

	if ( ret < 0 )
		ret = -1 ;
	else if ( ret > 0 )
		ret = 1 ;

	return( ret );
}

//-----------------------------------------------------------------------------
int bw_MW_stricmp ( const char * lhs, const wchar_t * rhs )
{
	int f, l;

	do
	{
		if ( ((f = (unsigned char)(*(lhs++))) >= 'A') && (f <= 'Z') )
			f -= 'A' - 'a';
		if ( ((l = (unsigned char)(*(rhs++))) >= 'A') && (l <= 'Z') )
			l -= 'A' - 'a';
	}
	while ( f && (f == l) );

	return(f - l);
}

//-----------------------------------------------------------------------------
int bw_MW_stricmp ( const wchar_t * lhs, const char * rhs )
{
	int f, l;

	do
	{
		if ( ((f = (unsigned char)(*(lhs++))) >= 'A') && (f <= 'Z') )
			f -= 'A' - 'a';
		if ( ((l = (unsigned char)(*(rhs++))) >= 'A') && (l <= 'Z') )
			l -= 'A' - 'a';
	}
	while ( f && (f == l) );

	return(f - l);
}



// string_utils.cpp
