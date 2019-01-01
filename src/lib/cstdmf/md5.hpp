/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MD5_HPP
#define MD5_HPP

#define MD5 MD5Hash // to rename the MD5 function declaration in md5.h
#include "openssl/md5.h"
#undef MD5

#include <string>

class BinaryIStream;
class BinaryOStream;

/**
 *  This class implements the standard MD5 cryptographic hash function.
 */
class MD5
{
	public:
		/**
		 *  This struct manages the message digest associated with an MD5.
		 */
		struct Digest
		{
			unsigned char bytes[16];

			Digest() { }
			Digest( MD5 & md5 ) { *this = md5; }
			Digest & operator=( MD5 & md5 )
				{ md5.getDigest( *this ); return *this; }
			void clear();

			bool operator==( const Digest & other ) const;
			bool operator!=( const Digest & other ) const
				{ return !(*this == other); }

			bool operator<( const Digest & other ) const;

			std::string quote() const;
			bool unquote( const std::string & quotedDigest );

			bool isEmpty() const
			{
				for (size_t i = 0; i < sizeof( bytes ); ++i)
				{
					if (bytes[i] != '\0')
					{
						return false;
					}
				}

				return true;
			}
		};

		MD5();

		void append( const void * data, int numBytes );
		void getDigest( Digest & digest );

	private:
		MD5_CTX state_;
};

BinaryIStream& operator>>( BinaryIStream &is, MD5::Digest &d );
BinaryOStream& operator<<( BinaryOStream &is, const MD5::Digest &d );

#endif // MD5_HPP
