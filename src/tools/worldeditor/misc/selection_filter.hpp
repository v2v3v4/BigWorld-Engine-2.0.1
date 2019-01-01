/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SELECTION_FILTER_HPP
#define SELECTION_FILTER_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"


class SelectionFilter
{
public:
	enum SelectMode { SELECT_ANY, SELECT_SHELLS, SELECT_NOSHELLS };
private:
	static std::vector<std::string> typeFilters_;
	static std::vector<std::string> noSelectTypeFilters_;

	static SelectMode selectMode_;
public:
	/**
	 * Return true if the given item is selectable.
	 *
	 * ignoreCurrentSelection: Don't let the currently selected items affect 
	 * the selectability of the given item.
	 *
	 * ignoreCameraChunk: Don't allow the shell model for the chunk the camera
	 * is in to be selected.
	 *
	 * ignoreVisibility: Don't use the visibility check.
	 *
	 * ignoreFrozen: Don't use the frozen check.
	 *	 
	 */
	static bool canSelect( ChunkItem* item,
		bool ignoreCurrentSelection = false, bool ignoreCameraChunk = true,
		bool ignoreVisibility = false, bool ignoreFrozen = false );

	/**
	 * Set the typeFilters_ field to filters, which is a
	 * "|" seperated list of filters
	 */
	static void typeFilters( std::string filters );
	static std::string typeFilters();

	/**
	 * As typeFilters_, but these will never be selected
	 */
	static void noSelectTypeFilters( std::string filters );
	static std::string noSelectTypeFilters();

	static void setSelectMode( SelectMode selectMode )	{ selectMode_ = selectMode; }
	static SelectMode getSelectMode()
	{
		return selectMode_;
	}
};


#endif // SELECTION_FILTER_HPP
