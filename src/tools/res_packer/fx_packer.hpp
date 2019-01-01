/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef __FX_PACKER_HPP__
#define __FX_PACKER_HPP__


#include "base_packer.hpp"
#include "packers.hpp"

#include <string>

/**
 *	This class simply copies all .fxo files that already exist, it does not
 *	rebuild the shaders here (a separate offline shader compilation tool
 *	exists for this). Sources are only copied if res_packer has been
 *	configured to do so (see config.hpp).
 */
class FxPacker : public BasePacker
{
public:
	enum Type {
		FX,
		FXH,
		FXO
	};

	FxPacker() : type_( FX ) {}

	virtual bool prepare( const std::string & src, const std::string & dst );
	virtual bool print();
	virtual bool pack();

private:
	DECLARE_PACKER()
	std::string src_;
	std::string dst_;
	Type type_;
};

#endif // __FX_PACKER_HPP__
