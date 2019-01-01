/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EDITOR_CHUNK_COMMON_HPP
#define EDITOR_CHUNK_COMMON_HPP

#include <vector>
#include <string>

class EditorChunkCommonLoadSave
{
public:
#ifdef EDITOR_ENABLED
	EditorChunkCommonLoadSave() : hidden_(false), frozen_(false) {}
	virtual ~EditorChunkCommonLoadSave(){}

	virtual bool edCommonLoad( DataSectionPtr pSection );
	virtual bool edCommonSave( DataSectionPtr pSection );

	virtual void edCommonChanged() = 0;

	/**
	 * Hide or reveal this item.
	 */
	virtual void edHidden( bool value );
	virtual bool edHidden( ) const;

	/**
	 * Freeze or unfreeze this item.
	 */
	virtual void edFrozen( bool value );
	virtual bool edFrozen( ) const;

private:
	bool hidden_, frozen_;
#endif
};

#endif // EDITOR_CHUNK_COMMON_HPP
