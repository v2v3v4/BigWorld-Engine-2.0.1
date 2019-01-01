/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef _MORPHER_HOLDER_HPP_
#define _MORPHER_HOLDER_HPP_

#include "mfxexp.hpp"
#include "wm3.h"

class MorpherHolder
{
public:
	MorpherHolder(INode* node)
	{
		// Disable the Morpher modifier if it is present
		mod_ = MFXExport::findMorphModifier( node );
		if ( mod_ )
			mod_->DisableMod();
	}

	~MorpherHolder()
	{
		if (mod_)
			mod_->EnableMod();
	}

private:
	Modifier* mod_;
};

#endif  // morpher_holder.hpp