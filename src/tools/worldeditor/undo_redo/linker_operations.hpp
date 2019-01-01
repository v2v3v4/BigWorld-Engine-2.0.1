/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LINKER_OPERATIONS_HPP
#define LINKER_OPERATIONS_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "common/base_properties_helper.hpp"
#include "gizmo/undoredo.hpp"
#include "cstdmf/unique_id.hpp"
#include <string>


/**
 *  This class saves all three affected linkers of the link change so
 *	that the operation can be reverted if needed.
 */
class LinkerUndoChangeLinkOperation : public UndoRedo::Operation
{
public:
    explicit LinkerUndoChangeLinkOperation(
		const EditorChunkItemLinkable* startEcil,
		const EditorChunkItemLinkable* oldEcil,
		const EditorChunkItemLinkable* newEcil,
		PropertyIndex propIdx);

	/*virtual*/ void undo();

    /*virtual*/ bool iseq(UndoRedo::Operation const &other) const;

private:
    LinkerUndoChangeLinkOperation(LinkerUndoChangeLinkOperation const &);
    LinkerUndoChangeLinkOperation &operator=(LinkerUndoChangeLinkOperation const &);

	// Member variables
	UniqueID startUID_;
	std::string startCID_;
	UniqueID oldUID_;
	std::string oldCID_;
	UniqueID newUID_;
	std::string newCID_;
	PropertyIndex propIdx_;
};


/**
 *  This class saves the new link added so that it can be deleted later if
 *	needed.
 */
class LinkerUndoAddLinkOperation : public UndoRedo::Operation
{
public:
    explicit LinkerUndoAddLinkOperation(
		const EditorChunkItemLinkable* startEcil,
		const EditorChunkItemLinkable* endEcil,
		PropertyIndex propIdx);

	/*virtual*/ void undo();

    /*virtual*/ bool iseq(UndoRedo::Operation const &other) const;

private:
    LinkerUndoAddLinkOperation(LinkerUndoAddLinkOperation const &);
    LinkerUndoAddLinkOperation &operator=(LinkerUndoAddLinkOperation const &);

	// Member variables
	UniqueID startUID_;
	std::string startCID_;
	UniqueID endUID_;
	std::string endCID_;
	PropertyIndex propIdx_;
};


/**
 *  This class saves all information regarding the deletion of a link so that it
 *	can be recreated if needed.
 */
class LinkerUndoDeleteLinkOperation : public UndoRedo::Operation
{
public:
    explicit LinkerUndoDeleteLinkOperation(
		const EditorChunkItemLinkable* startEcil,
		const EditorChunkItemLinkable* endEcil,
		DataSectionPtr data,
		PropertyIndex propIdx);

	/*virtual*/ void undo();

    /*virtual*/ bool iseq(UndoRedo::Operation const &other) const;

private:
    LinkerUndoDeleteLinkOperation(LinkerUndoDeleteLinkOperation const &);
    LinkerUndoDeleteLinkOperation &operator=(LinkerUndoDeleteLinkOperation const &);

	// Member variables
	UniqueID startUID_;
	std::string startCID_;
	UniqueID endUID_;
	std::string endCID_;
	DataSectionPtr data_;
	PropertyIndex propIdx_;
};


/**
 *  This class saves all information regarding two linker objects so that their
 *	relationship can be updated.
 */
class LinkerUpdateLinkOperation : public UndoRedo::Operation
{
public:
    explicit LinkerUpdateLinkOperation(
		const EditorChunkItemLinkable* startEcil,
		std::string targetUID,
		std::string targetChunkID);

	/*virtual*/ void undo();

    /*virtual*/ bool iseq(UndoRedo::Operation const &other) const;

private:
    LinkerUpdateLinkOperation(LinkerUpdateLinkOperation const &);
    LinkerUpdateLinkOperation &operator=(LinkerUpdateLinkOperation const &);

	// Member variables
	UniqueID startUID_;
	std::string startCID_;
	UniqueID targetUID_;
	std::string targetCID_;
};


#endif // LINKER_OPERATIONS_HPP
