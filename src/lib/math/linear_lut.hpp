/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LINEAR_LUT_HPP
#define LINEAR_LUT_HPP

#include "vector2.hpp"
#include <vector>


/**
 *	This class implements a look-up table that can interpolate between entries.
 *	It also implements boundary conditions.
 */
class LinearLUT
{
public:
	enum BoundaryCondition
	{
		BC_ZERO,				// values outside of table are 0.0
		BC_CONSTANT_EXTEND,		// values outside of table take end values
		BC_WRAP,				// values outside of table are wrapped back into table
		BC_LINEAR_EXTEND		// values outside of table are linearly extended
	};

	LinearLUT();

	float operator()(float x) const;

	void data(std::vector<Vector2> const &d);
	std::vector<Vector2> const &data() const;

	void lowerBoundaryCondition(BoundaryCondition bc);
	BoundaryCondition lowerBoundaryCondition() const;

	void upperBoundaryCondition(BoundaryCondition bc);
	BoundaryCondition upperBoundaryCondition() const;

private:
	std::vector<Vector2>			data_;
	BoundaryCondition				lowerBC_;
	BoundaryCondition				upperBC_;
	mutable size_t					cachedPos_;
};

#endif // LinearLUT
