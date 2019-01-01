/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SPEEDTREE_UTIL_HPP
#define SPEEDTREE_UTIL_HPP

#include "cstdmf/stdmf.hpp"
#include "cstdmf/smartpointer.hpp"


namespace speedtree {

/**
 *	Creates a string that uniquely identifies a tree loaded from 
 *	the given filename and generated with the provided seed number.
 *
 *	@param	filename	full path to the SPT file used to load the tree.
 *	@param	seed		seed number used to generate the tree.
 *
 *	@return				the requested string.
 */
std::string createTreeDefName( const char* filename, uint seed );

} //namespace speedtree
#endif // SPEEDTREE_UTIL_HPP