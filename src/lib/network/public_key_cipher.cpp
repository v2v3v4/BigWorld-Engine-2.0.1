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

#include "public_key_cipher.hpp"

#include "openssl/err.h"
#include "openssl/rand.h"

#include "cstdmf/guard.hpp"
#include "resmgr/bwresource.hpp"


// See RSA_public_encrypt(3) for the origin of the magic 41.
// RSA_PKCS1_OAEP_PADDING padding mode is used.
#define	RSA_PADDING	41	

#if defined( USE_OPENSSL )

namespace Mercury
{

// -----------------------------------------------------------------------------
// Section: PublicKeyCipher
// -----------------------------------------------------------------------------

/**
 *  Sets the key to be used by this object.
 */
bool PublicKeyCipher::setKey( const std::string & key )
{
	this->cleanup();

	// Construct an in-memory BIO to the key data
	BIO *bio = BIO_new( BIO_s_mem() );
	BIO_puts( bio, key.c_str() );

	// Read the key into the RSA struct
	pRSA_ = hasPrivate_ ?
		PEM_read_bio_RSAPrivateKey( bio, NULL, NULL, NULL ) :
		PEM_read_bio_RSA_PUBKEY( bio, NULL, NULL, NULL );

	BIO_free( bio );

	if (pRSA_)
	{
		this->setReadableKey();
		return true;
	}
	else
	{
		return false;
	}
}


#if !defined( PLAYSTATION3 ) && !defined( _XBOX360 )
/**
 *  This method loads a key from a resource path.
 */
bool PublicKeyCipher::setKeyFromResource( const std::string & path )
{
	BW_GUARD;

	BWResource::WatchAccessFromCallingThreadHolder watchAccess( false );

	DataSectionPtr pSection = BWResource::openSection( path );

	if (pSection)
	{
		BinaryPtr pBinData = pSection->asBinary();
		std::string keyStr( pBinData->cdata(), pBinData->len() );

		if (this->setKey( keyStr ))
		{
			INFO_MSG( "PublicKeyCipher::setKeyFromResource: "
				"Loaded %d-bit %s key from %s\n",
				this->numBits(), this->type(), path.c_str() );

			return true;
		}
		else
		{
			ERROR_MSG( "PublicKeyCipher::setKeyFromResource: "
				"Failed to initialise RSA object: %s\n",
				this->err_str() );

			return false;
		}
	}
	else
	{
		ERROR_MSG( "PublicKeyCipher::setKeyFromResource: "
			"Couldn't load private key from non-existant file %s\n",
			path.c_str() );

		return false;
	}
}
#endif


/**
 *  This method encrypts a string using this key.  It returns the size of the
 *  encrypted stream, or -1 on error.
 */
int PublicKeyCipher::encrypt( BinaryIStream & clearStream,
	BinaryOStream & cipherStream, EncryptionFunctionPtr pEncryptionFunc,
	int padding ) const
{

	// Size of the modulus used in the RSA algorithm.
	int rsaModulusSize = RSA_size( pRSA_ );	

	// If the size of cleartext is equals to or greater than (the size 
	// of RSA modulus - RSA_PADDING), cleartext will be split into smaller 
	// parts.  These smaller parts are encrypted and written to 
	// cipherStream.  

	// Ensures each part to be encrypted is less than the size of the RSA 
	// modulus minus padding.  partSize must be less than 
	// (rsaModulusSize - RSA_PADDING).
	int partSize = rsaModulusSize - RSA_PADDING - 1;


	// Encrypt clearText.
	int encryptLen = 0;
	while (clearStream.remainingLength())
	{
		// If remaining cleartext to be encrypted is too large, 
		// we retrieve what can be encrypted in a block.
		int numBytesToRetrieve =
			std::min( partSize, clearStream.remainingLength() );

		unsigned char * clearText = 
			(unsigned char*)clearStream.retrieve( numBytesToRetrieve );

		unsigned char * cipherText =
			(unsigned char *)cipherStream.reserve( rsaModulusSize );

		int curEncryptLen = 
			(*pEncryptionFunc)( 
				numBytesToRetrieve, clearText, cipherText, pRSA_, 
				padding );

		MF_ASSERT( curEncryptLen == RSA_size( pRSA_ ) );

		encryptLen += curEncryptLen; 
	}


	return encryptLen;
}


/**
 *  This method decrypts data encrypted with this key.  It returns the length of
 *  the decrypted stream, or -1 on error.
 */
int PublicKeyCipher::decrypt( BinaryIStream & cipherStream,
	BinaryOStream & clearStream, EncryptionFunctionPtr pEncryptionFunc,
	int padding ) const
{

	// Size of the modulus used in the RSA algorithm.
	int rsaModulusSize = RSA_size( pRSA_ );	

	unsigned char * clearText = 
		(unsigned char*) clearStream.reserve( cipherStream.remainingLength() );

	// Decrypt the encrypted blocks and write cleartext to clearStream.
	int clearTextSize = 0;
	while (cipherStream.remainingLength())	
	{
		unsigned char * cipherText =
			(unsigned char *)cipherStream.retrieve( rsaModulusSize  );

		if (cipherStream.error())
		{
			ERROR_MSG( "PrivateKeyCipher::privateDecrypt: "
				"Not enough data on stream for encrypted data chunk\n" );

			return -1;
		}	

		int bytesDecrypted = 
			(*pEncryptionFunc)( RSA_size( pRSA_ ), cipherText, clearText,
				pRSA_, padding );	

		clearTextSize += bytesDecrypted;
		clearText += bytesDecrypted;
	}


	if (clearTextSize == 0)
	{
		return -1;
	}

	return  clearTextSize;
}


/**
 *  This method frees memory used by this object and makes it unusable.
 */
void PublicKeyCipher::cleanup()
{
	if (pRSA_)
	{
		RSA_free( pRSA_ );
		pRSA_ = NULL;
	}
}


/**
 *  This method writes the standard base64 encoded representation of the public
 *  key into readableKey_.
 */
void PublicKeyCipher::setReadableKey()
{
	BIO * bio = BIO_new( BIO_s_mem() );
	PEM_write_bio_RSA_PUBKEY( bio, pRSA_ );

	char buf[ 1024 ];
	while (BIO_gets( bio, buf, sizeof( buf ) - 1 ) > 0)
	{
		readableKey_.append( buf );
	}

	BIO_free( bio );
}


/**
 *  This method returns the most recent OpenSSL error message.
 */
const char * PublicKeyCipher::err_str() const
{
	static bool errorStringsLoaded = false;

	if (!errorStringsLoaded)
	{
		ERR_load_crypto_strings();

		errorStringsLoaded = true;
	}

	return ERR_error_string( ERR_get_error(), 0 );
}

} // namespace Mercury

#endif	// USE_OPENSSL

// public_key_cipher.cpp
