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

#ifndef MODEL_ACTIONS_ITERATOR_HPP
#define MODEL_ACTIONS_ITERATOR_HPP

#include "forward_declarations.hpp"
#include "model.hpp"


/**
 * TODO: to be documented.
 */
class ModelActionsIterator
{
public:
	ModelActionsIterator( const Model * pModel );
	const ModelAction & operator*();
	const ModelAction * operator->();
	void operator++( int );
	bool operator==( const ModelActionsIterator & other ) const;
	bool operator!=( const ModelActionsIterator & other ) const;
private:
	const Model * pModel_;
	int index_;
};


#endif // MODEL_ACTIONS_ITERATOR_HPP
