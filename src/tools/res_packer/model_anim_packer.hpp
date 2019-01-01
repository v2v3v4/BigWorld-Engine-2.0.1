/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef __ANIM_PACKER_HPP__
#define __ANIM_PACKER_HPP__


#include "base_packer.hpp"
#include "packers.hpp"

#include <string>

/**
 *	This class converts all .animaion files related to a model to .anca, by
 *	loading the model.
 */
class ModelAnimPacker : public BasePacker
{
private:
	enum Type {
		MODEL,
		ANIMATION,
		ANCA
	};

public:
	ModelAnimPacker() : type_( MODEL ) {}

	virtual bool prepare( const std::string & src, const std::string & dst );
	virtual bool print();
	virtual bool pack();

private:
	DECLARE_PACKER()
	std::string src_;
	std::string dst_;
	Type type_;
};

#endif // __ANIM_PACKER_HPP__
