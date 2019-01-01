/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef INTERFACE_MINDER_HPP
#define INTERFACE_MINDER_HPP

#include "interface_element.hpp"
#include "misc.hpp"

#include "cstdmf/stdmf.hpp"

namespace Mercury
{

class NetworkInterface;

/**
 * 	The InterfaceMinder class manages a set of interface elements. It provides
 * 	an iterator for iterating over this set.
 *
 * 	@ingroup mercury
 */
class InterfaceMinder
{
public:
	InterfaceMinder( const char * name );

	InterfaceElement & add( const char * name, int8 lengthStyle,
			int lengthParam, InputMessageHandler * pHandler = NULL );

	InputMessageHandler * handler( int index );
	void handler( int index, InputMessageHandler * pHandler );
	const InterfaceElement & interfaceElement( uint8 id ) const;

	void registerWithInterface( NetworkInterface & networkInterface );
	Reason registerWithMachined( const Address & addr, int id ) const;

private:
	InterfaceElements		elements_;
	const char *			name_;
};

}	// end of namespace Mercury

#ifdef CODE_INLINE
#include "interface_minder.ipp"
#endif

#endif // INTERFACE_MINDER_HPP
