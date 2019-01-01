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

#ifndef SUPER_MODEL_ACTION_HPP
#define SUPER_MODEL_ACTION_HPP


#include "fashion.hpp"

#include "super_model.hpp"


class ModelAction;
class ModelAnimation;


/**
 *	This class represents an action from a SuperModel perspective.
 */
class SuperModelAction : public Fashion
{
public:
	float	passed_;		///< Real time passed since action started
	float	played_;		///< Effective time action has played at full speed
	float	finish_;		///< Real time action should finish (-ve => none)
	float	lastPlayed_;	///< Value of played when last ticked
	const ModelAction * pSource_;	// source action (first if multiple)
	const ModelAnimation * pFirstAnim_;	// top model first anim

	float blendRatio() const;

	void tick( SuperModel & superModel, float dtime );

private:
	int		index[1];	// same deal as SuperModelAnimation

	SuperModelAction( SuperModel & superModel, const std::string & name );
	friend class SuperModel;

	virtual void dress( SuperModel & superModel );
};

typedef SmartPointer<SuperModelAction> SuperModelActionPtr;



#endif // SUPER_MODEL_ACTION_HPP
