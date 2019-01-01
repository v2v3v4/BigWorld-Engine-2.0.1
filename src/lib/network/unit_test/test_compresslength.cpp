/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include <stdio.h>
#include "cstdmf/memory_stream.hpp"
#include "network/interface_element.hpp"
#include "network/bundle.hpp"
#include "third_party/CppUnitLite2/src/CppUnitLite2.h"

const char testString[] = "Hello World, this is a test string";

static int checkValue( Mercury::InterfaceElement & ie, unsigned int in )
{
	/* Unfortunately one cannot use compresslength without really allocating 
	   a message of this size since it uses its own value to copy a chunk of 
	   the packet to where it came from. */
	Mercury::Bundle testBundle;
	testBundle.startMessage( ie );
	unsigned int stringLength = strlen( testString );

	unsigned int leftToAllocate = in;
	if (leftToAllocate >= stringLength)
	{
		leftToAllocate += testBundle.size();
		testBundle << testString;
		leftToAllocate -= testBundle.size();
	}

	// Allocate a message in small chunks
	while (leftToAllocate > 1000)
	{
		leftToAllocate -= 1000;
		testBundle.reserve( 1000 );
	}
	testBundle.reserve( leftToAllocate );

	testBundle.finalise();

	Mercury::Bundle::iterator iter = testBundle.begin();
	if(iter.msgID() != ie.id())
	{
		return 0;
	}

	// Check if the message length is correct
	unsigned int out = iter.unpack( ie ).length;
	if(in != out)
	{
		return 0;
	}

	// If we had enough space to stow it, check if the test string is intact
	if (out >= stringLength)
	{
		MemoryIStream mis = MemoryIStream( iter.data(), out );
		std::string stringOut;

		mis >> stringOut;

		if(strcmp( stringOut.c_str(), testString ))
		{
			mis.finish();
			return 0;
		}
		mis.finish();
	}
	return 1;
}


TEST( CompressLength )
{
	// Try each length parameter 
	for (int lengthParam = 1; lengthParam <= 4; lengthParam++)
	{
		Mercury::InterfaceElement ie( "TestElement", 1, 
									  Mercury::VARIABLE_LENGTH_MESSAGE,
									  lengthParam );

		// Try a zero length message
		checkValue( ie, 0x0 );

		// Poke around the edges with various awkward bits set
		const int nHeads = 4, nBodies = 5;
		const unsigned char heads [nHeads ] =       {0x01, 0xA0, 0xA1, 0xFF};
		const unsigned char bodies[nBodies] = {0x00, 0x01, 0xA0, 0xA1, 0xFF};
		
		for (int head = 0; head < nHeads ; head++)
		{
			checkValue( ie, heads[head] );
			for (int body = 0; body < nBodies; body++)
			{
				unsigned int testvalue;
				testvalue = heads[head];

				/* Testing around the 16mb mark is useful, but beyond that 
				   execution time will balloon to something insane */ 
				for (int length = 1; length < (heads[head] == 0x1 ? 4 : 3 ); 
					 length++)
				{
					testvalue = (testvalue << 8) | bodies[body];
					CHECK( checkValue( ie, testvalue ) );
				}
			}
		}
	}
}
