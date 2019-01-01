/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef VERTICES_MANAGER_HPP
#define VERTICES_MANAGER_HPP

#include <list>
#include "cstdmf/concurrency.hpp"
#include "vertices.hpp"
#include "device_callback.hpp"

namespace Moo
{

/**
 *	This singleton class keeps track of and loads Vertices.
 */
class VerticesManager : public Moo::DeviceCallback
{
public:
	typedef std::map< std::string, Vertices * > VerticesMap;

	~VerticesManager();

	static VerticesManager*		instance();

	VerticesPtr					get( const std::string& resourceID, int numNodes = 0 );

	virtual void				deleteManagedObjects();
	virtual void				createManagedObjects();

	static void			init();
	static void			fini();

private:
	VerticesManager();
	VerticesManager(const VerticesManager&);
	VerticesManager& operator=(const VerticesManager&);

	static void del( Vertices* pVertices );
	void addInternal( Vertices* pVertices );
	void delInternal( Vertices* pVertices );

	VerticesPtr find( const std::string & resourceID );

	void setMorphOption( int optionIndex );

	bool enableMorphVertices_;

	VerticesMap					vertices_;
	SimpleMutex					verticesLock_;

	friend Vertices::~Vertices();
	
	static VerticesManager*	pInstance_;


};

}

#endif // VERTICES_MANAGER_HPP
