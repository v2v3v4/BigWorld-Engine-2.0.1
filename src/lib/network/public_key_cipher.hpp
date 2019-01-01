/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PUBLIC_KEY_CIPHER_HPP
#define PUBLIC_KEY_CIPHER_HPP

#if !defined(MF_SERVER) // defined( WIN32 ) && !defined( _XBOX360 )
#define USE_OPENSSL
#endif

#ifdef USE_OPENSSL

#include "openssl/rsa.h"
#include "openssl/bio.h"
#include "openssl/pem.h"
#include <string>
#include <map>

#include "cstdmf/binary_stream.hpp"

namespace Mercury
{

/**
 *  This class provides a simple interface to public key (i.e. asymmetric)
 *  encryption.  This base class represents keys with only a public part.  It
 *  uses OpenSSL's RSA implementation for the heavy lifting.
 */
class PublicKeyCipher
{
public:
	PublicKeyCipher( bool isPrivate ) :
		pRSA_( NULL ),
		hasPrivate_( isPrivate )
	{}

	~PublicKeyCipher() { this->cleanup(); }

	bool setKey( const std::string & key );
	bool setKeyFromResource( const std::string & path );

	/// The signature of RSA encryption/decryption functions
	typedef int (*EncryptionFunctionPtr)( int flen, const unsigned char * from,
		unsigned char * to, RSA * pRSA, int padding );

	/// This method encrypts data with the public part of this key.
	int publicEncrypt( BinaryIStream & clearStream,
		BinaryOStream & cipherStream ) const
	{
		return this->encrypt( clearStream, cipherStream,
			&RSA_public_encrypt, RSA_PKCS1_OAEP_PADDING );
	}

	/// This method decrypts data with the public part of this key
	/// (i.e. verifies a digital signature).
	int publicDecrypt( BinaryIStream & cipherStream,
		BinaryOStream & clearStream ) const
	{
		return this->decrypt( cipherStream, clearStream,
			&RSA_public_decrypt, RSA_PKCS1_PADDING );
	}

	/// This method encrypts data with the private part of this key
	/// (i.e. digitally signs the data).
	int privateEncrypt( BinaryIStream & cipherStream,
		BinaryOStream & clearStream ) const
	{
		MF_ASSERT( hasPrivate_ );
		return this->encrypt( cipherStream, clearStream,
			&RSA_private_encrypt, RSA_PKCS1_PADDING );
	}

	/// This method decrypts data with the private part of this key.
	int privateDecrypt( BinaryIStream & cipherStream,
		BinaryOStream & clearStream ) const
	{
		MF_ASSERT( hasPrivate_ );
		return this->decrypt( cipherStream, clearStream,
			&RSA_private_decrypt, RSA_PKCS1_OAEP_PADDING );
	}

	/// This method returns true if this object is usable.
	bool isGood() const { return pRSA_ != NULL; }

	/// This method returns a base64 representation of this key's public part.
	const char * c_str() const { return readableKey_.c_str(); }

	/// This method returns a base64 representation of this key's public part.
	const std::string & str() const { return readableKey_; }

	/// This method returns the size of the key in bytes.
	int size() const { return pRSA_ ? RSA_size( pRSA_ ) : -1; }

	/// This method returns the size of the key in bits.
	int numBits() const { return this->size() * 8; }

	/// This method returns the type of this key for debugging purposes.
	const char * type() const { return hasPrivate_ ? "private" : "public"; }

	const char * err_str() const;

protected:
	void cleanup();
	void setReadableKey();

	int encrypt( BinaryIStream & clearStream, BinaryOStream & cipherStream,
		EncryptionFunctionPtr pFunc, int padding ) const;

	int decrypt( BinaryIStream & cipherStream, BinaryOStream & clearStream,
		EncryptionFunctionPtr pFunc, int padding ) const;

	/// The OpenSSL RSA object that does all the work.
	RSA * pRSA_;

	/// The base64 representation of this key's public part.
	std::string readableKey_;

	/// Whether or not this key has a private part.
	bool hasPrivate_;
};


} // namespace Mercury

#endif // USE_OPENSSL

#endif // PUBLIC_KEY_CIPHER_HPP
