/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef STREAM_ENCODER_HPP
#define STREAM_ENCODER_HPP

class BinaryIStream;
class BinaryOStream;

/**
 *	Interface for encrypting a stream.
 */
class StreamEncoder
{
public:
	virtual ~StreamEncoder() {}

	/**
	 *	Encode the given cleartext input stream into the ciphertext output
	 *	stream.
	 *
	 *	@param clearText	The input stream.
	 *	@param cipherText 	The output stream.
	 *
	 *	@return	true on success, false otherwise.
	 */
	virtual bool encrypt( BinaryIStream & clearText, 
			BinaryOStream & cipherText ) const = 0;

	/**
	 *	Decode the given ciphertext input stream into the output stream as
	 *	cleartext.
	 *
	 *	@param cipherText	The input stream.
	 *	@param clearText 	The output stream.
	 *
	 *	@return true on success, false otherwise.
	 */
	virtual bool decrypt( BinaryIStream & cipherText, 
			BinaryOStream & clearText ) const = 0;
};

#endif // STREAM_ENCODER_HPP
