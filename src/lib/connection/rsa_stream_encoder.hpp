/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef RSA_STREAM_ENCODER_HPP
#define RSA_STREAM_ENCODER_HPP

#ifdef USE_OPENSSL

#include "stream_encoder.hpp"

#include "network/public_key_cipher.hpp"

class RSAStreamEncoder : public StreamEncoder
{

public:
	RSAStreamEncoder( bool keyIsPrivate ):
		StreamEncoder(),
		key_( keyIsPrivate )
	{
	}


	virtual ~RSAStreamEncoder()
	{}


	/**
	 * 	Initialise the key from the given string.
	 */
	bool initFromKeyString( const std::string & keyString )
	{
		return key_.setKey( keyString );
	}


	/**
	 *	Initialise the key from the given path, which refers to a file
	 *	containing the key contents.
	 */
	bool initFromKeyPath( const std::string & keyPath )
	{
		return key_.setKeyFromResource( keyPath );
	}


	/**
	 * 	Override from LogOnParamsEncoder.
	 */
	virtual bool encrypt( BinaryIStream & clearText, 
			BinaryOStream & cipherText ) const
	{
		return (key_.publicEncrypt( clearText, cipherText ) != -1);
	}


	/**
	 *	Override from LogOnParamsEncoder.
	 */
	virtual bool decrypt( BinaryIStream & cipherText,
			BinaryOStream & clearText ) const
	{
		return (key_.privateDecrypt( cipherText, clearText ) != -1);
	}


private:
	Mercury::PublicKeyCipher 	key_;
};

#endif // USE_OPENSSL

#endif // RSA_STREAM_ENCODER_HPP
