/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PROTO_MODULE_HPP
#define PROTO_MODULE_HPP

#include "moo/base_texture.hpp"
#include "appmgr/module.hpp"
#include "bigbang/grid_coord.hpp"


class ClosedCaptions;
class ParticleSystem;

class PeModule : public FrameworkModule
{
public:
	PeModule();
	~PeModule();

	virtual bool init(DataSectionPtr pSection);

	virtual void onStart();
	virtual int  onStop();

	virtual bool updateState(float dTime);
	virtual void render(float dTime);

	virtual bool handleKeyEvent(const KeyEvent &event);
	virtual bool handleMouseEvent(const MouseEvent &event);

	static PeModule& instance() { ASSERT(s_instance_); return *s_instance_; }

	ParticleSystem * activeParticleSystem() const { return particleSystem_; }

	float lastTimeStep() const { return lastTimeStep_; }
	float averageFPS() const { return averageFps_; }

	typedef PeModule This;
	PY_MODULE_STATIC_METHOD_DECLARE( py_setParticleSystem )

protected:
	void beginRender();
	void endRender();

	void renderChunks();
	void renderTerrain(float dTime);
    void renderScale();

	bool initPyScript();
	bool finiPyScript();

	Vector2 currentGridPos();
	Vector3 gridPosToWorldPos(Vector2 gridPos);

private:
	PeModule(PeModule const&);              // not permitted
	PeModule& operator=(PeModule const&);   // not permitted

private:
	Moo::BaseTexturePtr         mapTexture_;
    static PeModule             *s_instance_;           // The last created PeModule 
	Vector3                     viewPosition_;          // x, y camera position, z is zoom
	POINT                       lastCursorPosition_;    // Pos. of camera when started to look about
	GridCoord                   localToWorld_;          
	Vector2                     selectionStart_;        // Start of current selection
    GridRect                    currentSelection_;      // Current selection with mouse
	unsigned int                gridWidth_;             // Width of grid in chunks
	unsigned int                gridHeight_;            // Height of grid in chunks
	float                       lastTimeStep_;
	float                       averageFps_;
	PyObject                    *scriptDict_;
	ParticleSystem              *particleSystem_;
	SmartPointer<ClosedCaptions> cc_;
};

#endif // PROTO_MODULE_HPP