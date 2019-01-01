/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef RENDER_CONTEXT_DEBUG
#define RENDER_CONTEXT_DEBUG

#ifdef _DEBUG

#include <iostream>
class RenderContext;

namespace Moo
{
    std::ostream &printRenderContextState(std::ostream &out, RenderContext &rc);
}

#endif // _DEBUG

#endif // RENDER_CONTEXT_DEBUG
