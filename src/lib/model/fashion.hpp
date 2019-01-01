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

#ifndef FASHION_HPP
#define FASHION_HPP

#include "forward_declarations.hpp"


typedef std::vector<FashionPtr> FashionVector;


/**
 *	This abstract class represents a fashion in which a supermodel
 *	may be drawn. It could be an animation at a certain time
 *	and blend ratio, or it could be a material override, or
 *	it could be a morph with associated parameters
 */
class Fashion : public ReferenceCount
{
public:
	Fashion();
	virtual ~Fashion();

protected:
	virtual void dress( class SuperModel & superModel ) = 0;
	virtual void undress( class SuperModel & superModel );

	friend class SuperModel;
};




#endif // FASHION_HPP
