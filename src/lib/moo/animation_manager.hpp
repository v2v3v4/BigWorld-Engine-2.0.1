/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ANIMATION_MANAGER_HPP
#define ANIMATION_MANAGER_HPP

#include <map>
#include "cstdmf/smartpointer.hpp"
#include "cstdmf/concurrency.hpp"
#include "cstdmf/singleton.hpp"


namespace Moo
{

class Animation;
typedef SmartPointer<Animation> AnimationPtr;
class Node;
typedef SmartPointer<Node> NodePtr;

/**
 *	Control centre for the animation system. All animations are stored here
 *	and retrieved through the get and find methods.
 */
class AnimationManager : public Singleton< AnimationManager >
{
public:
	AnimationManager();

	AnimationPtr				get( const std::string& resourceID, NodePtr rootNode );
	AnimationPtr				get( const std::string& resourceID );

	/*
	 * Public so that ModelViewer can change the original Animation when
	 * fiddling with compression.
	 */
	AnimationPtr				find( const std::string& resourceID );

	std::string					resourceID( Animation * pAnim );

	void fullHouse( bool noMoreEntries = true );

	typedef std::map< std::string, Animation * > AnimationMap;

private:
	AnimationMap				animations_;
	SimpleMutex					animationsLock_;

	bool						fullHouse_;

	void						del( Animation * pAnimation );
	friend class Animation;

	AnimationManager(const AnimationManager&);
	AnimationManager& operator=(const AnimationManager&);
};

}

#ifdef CODE_INLINE
#include "animation_manager.ipp"
#endif


#endif // ANIMATION_MANAGER_HPP
