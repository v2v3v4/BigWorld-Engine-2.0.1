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

#include "interface_minder.hpp"

#ifndef CODE_INLINE
#include "interface_minder.ipp"
#endif

#include "machined_utils.hpp"
#include "network_interface.hpp"

DECLARE_DEBUG_COMPONENT2( "Network", 0 );

namespace Mercury
{

// -----------------------------------------------------------------------------
// Section: InterfaceMinder
// -----------------------------------------------------------------------------

/**
 * 	This method adds an interface element (Mercury method) to the interface minder.
 *  @param name             Name of the interface element.
 * 	@param lengthStyle		Specifies whether the message is fixed or variable.
 *	@param lengthParam		This depends on lengthStyle.
 *	@param pHandler			The message handler for this interface.
 */
InterfaceElement & InterfaceMinder::add( const char * name,
	int8 lengthStyle, int lengthParam, InputMessageHandler * pHandler )
{
	// Set up the new bucket and add it to the list
	InterfaceElement element( name, elements_.size(), lengthStyle, lengthParam,
		pHandler );

	elements_.push_back( element );
	return elements_.back();
}


/**
 * 	This method registers all the minded interface elements with an interface.
 *
 * 	@param nub				The nub with which to register the interfaces.
 */
void InterfaceMinder::registerWithInterface( NetworkInterface & networkInterface )
{
	for (uint i=0; i < elements_.size(); ++i)
	{
		InterfaceElement & element = elements_[i];
		networkInterface.interfaceTable().serve( element, i, element.pHandler() );
	}
}


/**
 *  This method registers this interface with machined on behalf of the nub.
 */
Reason InterfaceMinder::registerWithMachined( const Address & addr,
	int id ) const
{
	return MachineDaemon::registerWithMachined( addr, name_, id );
}

} // namespace Mercury

// interface_minder.cpp
