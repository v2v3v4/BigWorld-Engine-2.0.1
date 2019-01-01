/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ENTITY_DESCRIPTION_DEBUG_HPP
#define ENTITY_DESCRIPTION_DEBUG_HPP

class EntityDescription;
class EntityDescriptionMap;

/**
 *	This is a simple namespace that contains functions to help debug
 *	EntityDescription related issues.
 */
namespace EntityDescriptionDebug
{
void dump( const EntityDescription & desc, int detailLevel = 100 );
void dump( const EntityDescriptionMap & desc, int detailLevel = 100 );
}

#endif // ENTITY_DESCRIPTION_DEBUG_HPP
