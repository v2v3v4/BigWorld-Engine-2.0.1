/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/**
 * 	@file
 *
 *	This file provides the definition of the MemoryOStream class.
 *
 * 	@ingroup network
 */

#ifndef MEMORY_STREAM_HPP
#define MEMORY_STREAM_HPP

#include "cstdmf/binary_stream.hpp"
#include "cstdmf/debug.hpp"



/**
 *	This class is used to create a message that will later be placed on a
 *	bundle. It supports both the BinaryOStream and BinaryIStream
 *	interfaces, so it is possible to read and write data to and from
 *	this stream.
 *
 *	@ingroup network
 */
class MemoryOStream : public BinaryOStream, public BinaryIStream
{
public: // constructors and destructor
	inline MemoryOStream( int size = 64 );

	virtual ~MemoryOStream();

protected:
	MemoryOStream( const MemoryOStream & );
	MemoryOStream & operator=( const MemoryOStream & );

public: // overrides from BinaryOStream
	virtual void * reserve( int nBytes );
	virtual int size() const;

public: // overrides from BinaryIStream
	virtual const void * retrieve( int nBytes );
	virtual int remainingLength() const;
	virtual char peek();

public: // own methods
	bool shouldDelete() const;
	void shouldDelete( bool value );
	void * data();
	void reset();
	void rewind();


protected:
	char * pBegin_;
	char * pCurr_;
	char * pEnd_;
	char * pRead_;
	bool shouldDelete_;
};

/**
 *	This class is used to stream data off a memory block.
 */
class MemoryIStream : public BinaryIStream
{
public: // constructors and destructor
	MemoryIStream( const char * pData, int length ) :
		BinaryIStream(),
		pCurr_( pData ),
		pEnd_( pCurr_ + length )
	{}

	MemoryIStream( const void * pData, int length ) :
		pCurr_( reinterpret_cast< const char * >( pData ) ),
		pEnd_( pCurr_ + length )
	{}

	virtual ~MemoryIStream();


public: // overrides from BinaryIStream
	virtual const void * retrieve( int nBytes );

	virtual int remainingLength() const { return (int)(pEnd_ - pCurr_); }

	virtual char peek();

	virtual void finish()	{ pCurr_ = pEnd_; }

public: // new methods
	const void * data() { return pCurr_; }

protected:
	MemoryIStream() :
		pCurr_( NULL ),
		pEnd_( NULL )
	{}

	void init( const char * pData, int length )
	{
		pCurr_ = pData;
		pEnd_ = pCurr_ + length;
	}

private:
	const char * pCurr_;
	const char * pEnd_;
};


/**
 * TODO: to be documented.
 */
class MessageStream : public MemoryOStream
{
public: // constructors and destructor
	MessageStream( int size = 64 );

	virtual ~MessageStream() {}

public: // own methods
	bool addToStream( BinaryOStream & stream, uint8 messageID );
	bool getMessage( BinaryIStream & stream,
		int & messageID, int & length );
};

#include "memory_stream.ipp"

#endif // MEMORY_STREAM_HPP
