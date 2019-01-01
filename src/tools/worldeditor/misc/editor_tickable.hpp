/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EDITOR_TICKABLE_HPP
#define EDITOR_TICKABLE_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"


class EditorTickable: public ReferenceCount
{
public:
	virtual ~EditorTickable() {};

	virtual void tick() = 0;
};


typedef SmartPointer<EditorTickable> EditorTickablePtr;


#endif // EDITOR_TICKABLE_HPP
