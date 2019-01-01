/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifdef _MSC_VER 
#pragma once
#endif

#ifndef MODEL_MATTER_HPP
#define MODEL_MATTER_HPP

#include "forward_declarations.hpp"
#include "moo/forward_declarations.hpp"
#include "moo/visual.hpp"


/**
 *	Inner class to represent the base Model's dye definition
 */
class Matter
{
public:
	typedef std::vector<TintPtr>	Tints;
	typedef std::vector< Moo::Visual::PrimitiveGroup * >	PrimitiveGroups;

public:
	Matter(	const std::string & name,
			const std::string & replaces );
	Matter( const std::string & name,
			const std::string & replaces,
			const Tints & tints );
	Matter( const Matter & other );
	~Matter();

	Matter & operator= ( const Matter & other );

	void emulsify( int tint = 0 );
	void soak();


	std::string				name_;
	std::string				replaces_;

	Tints					tints_;
	PrimitiveGroups			primitiveGroups_;

private:
	int		emulsion_;
	int		emulsionCookie_;
};


#endif // MODEL_MATTER_HPP
