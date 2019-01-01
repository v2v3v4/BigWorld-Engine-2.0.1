/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef VISUAL_MANAGER_HPP
#define VISUAL_MANAGER_HPP

#include <map>
#include "cstdmf/concurrency.hpp"
#include "cstdmf/smartpointer.hpp"

#include "visual.hpp"


namespace Moo
{

/**
 *	This singleton class keeps track of and loads Visuals.
 */
class VisualManager
{
public:
	typedef std::map< std::string, Visual * > VisualMap;

	~VisualManager();

	static VisualManager* instance();

	static void init();
	static void fini();

	VisualPtr get( const std::string& resourceID, bool loadIfMissing = true );

	void fullHouse( bool noMoreEntries = true );

	void add( Visual * pVisual, const std::string & resourceID );

private:
	VisualManager();
	VisualManager(const VisualManager&);
	VisualManager& operator=(const VisualManager&);
	
	static void del( Visual* pVisual );
	void delInternal( Visual* pVisual );

	VisualPtr find( const std::string & resourceID );

	VisualMap visuals_;
	SimpleMutex	visualsLock_;

	bool fullHouse_;

	static VisualManager* pInstance_;

	friend Visual::~Visual();
};

}

#ifdef CODE_INLINE
#include "visual_manager.ipp"
#endif




#endif // VISUAL_MANAGER_HPP
