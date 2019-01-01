/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LIGHT_CONTAINER_DEBUGGER_HPP
#define LIGHT_CONTAINER_DEBUGGER_HPP

#include "editor_renderable.hpp"
#include "chunk/chunk_item.hpp"

class LightContainerDebugger : public EditorRenderable
{
public:
	static LightContainerDebugger* instance();
	static void fini();

	void	setItems( const std::vector<ChunkItemPtr>& items ) { items_ = items; }
	size_t	numItems() const { return items_.size(); }
	void	clearItems() { items_.clear(); }

	void render();

private:
	static SmartPointer<LightContainerDebugger> s_instance_;
	LightContainerDebugger();	

	std::vector<ChunkItemPtr> items_;
};

#endif
