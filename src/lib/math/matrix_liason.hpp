/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MATRIX_LIASON_HPP
#define MATRIX_LIASON_HPP

#include "matrix.hpp"

/**
 *	Interface class for top level attachments
 */
class MatrixLiaison
{
public:
	virtual const Matrix & getMatrix() const = 0;
	virtual bool setMatrix( const Matrix & m ) = 0;
};

#endif //MATRIX_LIASON_HPP