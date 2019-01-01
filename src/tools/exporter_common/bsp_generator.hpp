/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BSP_GENERATOR_HPP
#define BSP_GENERATOR_HPP

#include <string>
#include <vector>


/**
 *	This function generates a bsp from a visual.
 *
 *	@param	visualName	The visual file to extract the BSP from.
 *	@param	bspName		The name of the BSP.
 *	@param	materialIDs	The list of materialIDs to be returned.
 *	@return	Success or failure.
 */
bool generateBSP( const std::string & visualName,
		const std::string & bspName,
		std::vector<std::string>& materialIDs );


#endif
