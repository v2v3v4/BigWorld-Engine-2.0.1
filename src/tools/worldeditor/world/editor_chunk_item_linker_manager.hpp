/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EDITOR_CHUNK_ITEM_LINKER_MANAGER_HPP
#define EDITOR_CHUNK_ITEM_LINKER_MANAGER_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"

#include "chunk/chunk_link.hpp"

#include "pyscript/script.hpp"

#include "cstdmf/concurrency.hpp"
#include "cstdmf/unique_id.hpp"

#include <string>
#include <set>
#include <map>


/**
 *	This class manages the creation, editing and deletion of Linker objects.
 *	Linker objects notify the manager whenever they are modified.  It is the
 *	managers responsibility to update the appropriate Linker objects when
 *	changes are made to ensure that the space is always in a consistent state.
 *	The manager does not own the linker objects it manages, so it does not have
 *	to worry about destroying Linker objects.  Also, the individual link objects
 *	belong to the linker objects so the manager does not need to worry about
 *	these link objects either.
 */
class EditorChunkItemLinkableManager
{
public:
	// Public typedefs
	/// Used to map GUIDs from one value to another
	typedef std::map< UniqueID, UniqueID > GuidToGuidMap;


	// Public methods
	EditorChunkItemLinkableManager();

	void tick();

	void updateLink(
		EditorChunkItemLinkable* pLinkable1, EditorChunkItemLinkable* pLinkable2 );

	void tossAdd( EditorChunkItemLinkable* pLinkable );
	void tossRemove( EditorChunkItemLinkable* pLinkable );

	void idPreChange( const UniqueID& from, const UniqueID& to );

	void addLink(
		EditorChunkItemLinkable* start, EditorChunkItemLinkable* end,
		const PropertyIndex& propIdx, bool updateOld = true );

	void deleted( EditorChunkItemLinkable* pLinkable );

	void deleteLink(
		EditorChunkItemLinkable* pLinkable, EditorChunkItemLinkable* pOldLinkable,
		PropertyIndex propIdx );

	void deleteAllLinks(
		EditorChunkItemLinkable* pLinkable1, EditorChunkItemLinkable* pLinkable2 );

	EditorChunkItemLinkable* forceLoad(
		const UniqueID& linkableId, const std::string& chunkId );

	void reset();

	void removeFromLists( EditorChunkItemLinkable* pLinkable );

	void updateMappedLinkers( GuidToGuidMap& linkerGuidMapping );

	static void updateLinkerGuid(
		DataSectionPtr item, GuidToGuidMap& linkerGuidMapping );


private:
	EditorChunkItemLinkableManager(
		const EditorChunkItemLinkableManager& linkableManager );
	EditorChunkItemLinkableManager& operator=(
		const EditorChunkItemLinkableManager& linkableManager );


	// Private registration methods
	void registerLinker( EditorChunkItemLinkable* pLinkable );
	void unregisterLinker( EditorChunkItemLinkable* pLinkable );
	bool isRegistered( const UniqueID& linkableId );
	EditorChunkItemLinkable* getRegistered( const UniqueID& linkableId );
	EditorChunkItemLinkable* forceLoad( PyObjectPtr pPyLinkProp );


	// Private validation method
	bool isLinkValid(
		const EditorChunkItemLinkable* start, const EditorChunkItemLinkable* end,
		const std::string& endChunkId ) const;


	// Private methods to update cloned linker objects
	void updateCloneBackLinks(
		EditorChunkItemLinkable* pCloneLinkable,
		GuidToGuidMap& linkerCloneGuidMapping );
	void updateCloneProperties(
		EditorChunkItemLinkable* pCloneLinkable,
		GuidToGuidMap& linkerCloneGuidMapping );


	// Used to track changes in linker location
	void checkLocation( EditorChunkItemLinkable* pLinkable );
	void removeFromLocationCache( EditorChunkItemLinkable* pLinkable );
	void updateLocationInProperties( EditorChunkItemLinkable* pLinkable );
	void updateLocationInBackLinks( EditorChunkItemLinkable* pLinkable );
	void updateLocationInManagedLists( EditorChunkItemLinkable* pLinkable );


	// Updates linker properties
	void updateChunkIdsInProps(
		EditorChunkItemLinkable* updatingLinker,
		EditorChunkItemLinkable* chunkChangedLinker );
	bool isLinkerReferencedInProp(
		const EditorChunkItemLinkable* pLinkable, PyObjectPtr pPyLinkProp ) const;


	// Main methods used to load and create links
	void addLink(
		const UniqueID& linkFromUID, const std::string& linkFromCID,
		const UniqueID& linkToUID, const std::string& linkToCID );
	void addLink(
		const EditorChunkItemLinkable* pLinker, PyObjectPtr pPyLinkProp );
	bool addLinkInternal(
		EditorChunkItemLinkable* start, EditorChunkItemLinkable* end,
		const PropertyIndex& propIdx, EditorChunkItemLinkable** old = 0,
		bool updatePrevious = true );


	// Methods used to create and destroy link objects as chunks are loaded and
	// unloaded
	void addLinkToOutstandingBackLinks(
		const UniqueID& linkFromUID, const std::string& linkFromCID,
		const UniqueID& linkToUID, const std::string& linkToCID );
	void addLinkToOutstandingBackLinks(
		const EditorChunkItemLinkable* pLinkable, PyObjectPtr pPyLinkProp );
	void addAllLinksToOutstandingList( EditorChunkItemLinkable* pLinkable );
	void addAllLinksToOutstandingBackLinksList( EditorChunkItemLinkable* pLinkable );
	void addAllBacklinksToOutstandingList( EditorChunkItemLinkable* pLinkable );
	void addAllBackLinksToOutstandingBackLinksList(
		EditorChunkItemLinkable* pLinkable );


	// Internal worker method for deleting a link
	void deleteLinkInternal( EditorChunkItemLinkable* pLinkable,
		EditorChunkItemLinkable* pOldLinkable, PropertyIndex propIdx );


	// Helper methods to remove all references between linker objects 
	void removeAllRefs(
		EditorChunkItemLinkable* from, EditorChunkItemLinkable* pointingTo );
	void removeAllBackLinksToLinker( EditorChunkItemLinkable* to );


    // Allow these undo/redo classes access to our private methods
	friend LinkerUndoChangeLinkOperation;
	friend LinkerUndoAddLinkOperation;


	/// Used to store information about a link.
	class Link
	{
	public:
		Link(
			UniqueID fromUID, std::string fromCID,
			UniqueID toUID, std::string toCID );

		bool operator==( const Link& rhs ) const;

		UniqueID	fromUID_;
		std::string	fromCID_;
		UniqueID	toUID_;
		std::string	toCID_;
	};

public:
	// Public typedef
	typedef std::pair<UniqueID, Link>						ULPair;

private:
	// Private typedefs
	typedef std::map<UniqueID, std::string>					LocationCacheMap;
	typedef std::set<EditorChunkItemLinkable *>				LinkerSet;
	typedef std::set<EditorChunkItemLinkable *>::iterator	LinkerSetIt;
	typedef std::multimap<UniqueID, Link>					ULMultimap;
	typedef std::multimap<UniqueID, Link>::iterator			ULMultimapIt;
	typedef std::pair<UniqueID, std::string>				UCPair;	// GUID ChunkId


    // Private member variables
	LinkerSet		    registeredLinkers_;
	LocationCacheMap	locationCache_;
	ULMultimap			outstandingLinks_;
	ULMultimap			outstandingBackLinks_;
	ULMultimap			chunkLinks_;
	ULMultimap			chunkBackLinks_;

	// Mutexes used to control container access
	SimpleMutex							registeredLinkersMutex_;
	SimpleMutex							locationCacheMutex_;
	SimpleMutex							outstandingLinksMutex_;
	SimpleMutex							outstandingBackLinksMutex_;
	SimpleMutex							chunkLinksMutex_;
	SimpleMutex							chunkBackLinksMutex_;
};


#endif	// EDITOR_CHUNK_ITEM_LINKER_MANAGER_HPP
