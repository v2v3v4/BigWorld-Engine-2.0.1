/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PY_PHYSICS2_HPP
#define PY_PHYSICS2_HPP

#include "chunk/chunk_manager.hpp"
#include "chunk/chunk_space.hpp"

#include "pyscript/script.hpp"
#include "pyscript/pyobject_plus.hpp"

template<class CallbackType>
PyObject * py_collide( SpaceID spaceID, 
		const Vector3 & src, const Vector3 & dst )
{
	ChunkSpacePtr pSpace = ChunkManager::instance().space( spaceID, false );
	if (!pSpace)
	{
		PyErr_Format( PyExc_ValueError, "BigWorld.collide(): "
			"No space ID %d", int(spaceID) );
		return NULL;
	}

	CallbackType fdpt;
	float dist = pSpace->collide( src, dst, fdpt );
	if (dist < 0.f)
	{
		Py_Return;
	}

	Vector3 dir = dst-src;
	dir.normalise();
	Vector3 hitpt = src + dir * dist;

	PyObjectPtr results[4] = {
		PyObjectPtr( Script::getData( hitpt ), true ),
		PyObjectPtr( Script::getData( fdpt.tri_.v0() ), true ),
		PyObjectPtr( Script::getData( fdpt.tri_.v1() ), true ),
		PyObjectPtr( Script::getData( fdpt.tri_.v2() ), true ) };

	return Py_BuildValue( "(O,(O,O,O),i)",
		results[0].getObject(),
		results[1].getObject(),
		results[2].getObject(),
		results[3].getObject(),
		fdpt.tri_.materialKind() );
}

#endif // PY_PHYSICS2_HPP
