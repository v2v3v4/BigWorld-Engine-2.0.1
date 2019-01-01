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

#ifndef MODEL_ACTION_HPP
#define MODEL_ACTION_HPP


#include "forward_declarations.hpp"
#include "match_info.hpp"
#include "resmgr/forward_declarations.hpp"


class Capabilities;


/**
 *	Inner class to represent the base Model's action
 */
class ModelAction : public ReferenceCount
{
public:
	ModelAction( DataSectionPtr pSect );
	~ModelAction();

	bool valid( const ::Model & model ) const;
	bool promoteMotion() const;

	uint32 sizeInBytes() const;

	std::string	name_;
	std::string	animation_;
	float		blendInTime_;
	float		blendOutTime_;
	int			track_;
	bool		filler_;
	bool		isMovement_;
	bool		isCoordinated_;
	bool		isImpacting_;

	int			flagSum_;

	bool		isMatchable_;
	MatchInfo	matchInfo_;
};



#endif // ACTION_HPP
