/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef _WATER_SIMULATION_HPP
#define _WATER_SIMULATION_HPP

#include <iostream>
#include <vector>

#include "cstdmf/smartpointer.hpp"

#include "moo/moo_math.hpp"
#include "moo/moo_dx.hpp"
#include "moo/com_object_wrap.hpp"
#include "moo/effect_material.hpp"
#include "moo/effect_visual_context.hpp"
#include "moo/render_target.hpp"

#include "math/boundbox.hpp"
#include "resmgr/datasection.hpp"
#include "romp_collider.hpp"

#include "moo/vertex_buffer_wrapper.hpp"
using Moo::VertexBufferWrapper;
#define VERTEX_TYPE Moo::VertexXYZDUV

#include "water_simulation.hpp"

namespace Moo
{
	class Material;
	class BaseTexture;
	typedef SmartPointer< BaseTexture > BaseTexturePtr;
}

class Water;
typedef SmartPointer< Water > WaterPtr;
class Waters;
class BackgroundTask;

/**
 * TODO: to be documented.
 */
class SimulationTextureBlock
{
public:
	SimulationTextureBlock();
	SimulationTextureBlock( int width, int height );

	void init( int width, int height );
	void fini();
	void recreate();
	void clear();
	void tickSimulation();
	bool lock();
	void unlock() { locked_ = false; }

	Moo::RenderTargetPtr& operator()(int idx) { return simTexture_[idx]; }
	Moo::RenderTargetPtr operator()(int idx) const { return simTexture_[idx]; }	

	bool push()
	{
		Moo::RenderTargetPtr rt = simTexture_[ simulationIndex_ % 3 ];
		return rt && rt->push();
	}
	void pop() { simTexture_[ simulationIndex_ % 3 ]->pop(); }

	Moo::RenderTargetPtr previous() { return simTexture_[ (simulationIndex_ + 1) % 3 ]; } 
	Moo::RenderTargetPtr current() { return simTexture_[ (simulationIndex_ + 2) % 3 ]; } 
	Moo::RenderTargetPtr result() { return (simulationIndex_>0) ? simTexture_[(simulationIndex_-1) % 3] : simTexture_[0]; }

	int width() { return width_; }
	int height() { return height_; }

private:
	Moo::RenderTargetPtr	simTexture_[3];
	int						width_, height_;
	int						simulationIndex_;
	bool					locked_;
};

typedef std::pair<Vector3,Vector3> MovementPair;
typedef std::pair<MovementPair,float> Movement;
typedef std::vector< Movement > Movements;


/**
 * A simulation.
 */
class Simulation
{
public:
	Simulation();	

	void resetIdleTimer();
	int size() const;
	void size(int size);
	void clearMovements();
	float lastUpdateTime() const;
	float idleTime() const;
	bool perturbed() const;
	void perturbed( bool val );	
	void updateTime( float dTime );
	bool hasMovements() const;
	bool shouldActivate() const;
	bool shouldDeactivate() const;	
	const Movements& movements() const;


	virtual void tick() = 0;
	virtual void activate();
	virtual void deactivate();
	virtual bool isActive() const;
	virtual void updateMovements();
	virtual void addMovement( const Vector3& from, const Vector3& to, const float diameter );
private:
	int						size_;
	float					idleTime_;
	float					lastSimTime_;
	Movements				movements_;
	uint32					lastMovementFrame_;
	bool					perturb_,
							bActive_;
};


/**
 * A rain simulation
 */
class RainSimulation : public Simulation
{
public:
	RainSimulation();

	void fini();
	void clear();
	void recreate();
	void init(int size);
	Moo::RenderTargetPtr result();
	void simulate( float dTime, float amount, Moo::EffectMaterialPtr effect );

	virtual void tick();

private:
	//Rain has it's own texture block. (seperate from the managers pool)
	SimulationTextureBlock	simulationTarget_;
};


class SimulationCell;


/**
 *	This class is holds the pool of textures (render targets) that can be used for simulations
 */

class SimulationManager
{
public:
	static void fini();	
	static void init();

	void loadResources();
	void recreateBlocks();
	void tickSimulations();
	void resetSimulations();
	Moo::RenderTargetPtr rainTexture();
	float maxIdleTime() { return s_maxIdleTime_; }
	bool rainActive() { return !rainSimulation_.shouldDeactivate(); }
	void drawMovements( Moo::EffectMaterialPtr effect, Simulation* sim );
	void setMaxTextures( int value ) { s_maxTextureBlocks_ = value; }	
	void simulateRain( float dTime, float amount, Moo::EffectMaterialPtr effect );
	void releaseSimulationTextureBlock( SimulationTextureBlock* block );
	void simulateBlock( Moo::EffectMaterialPtr effect, SimulationTextureBlock* block, int borderSize=0 );
	SimulationTextureBlock* requestSimulationTextureBlock( SimulationCell* cell );
	void setupSim( SimulationTextureBlock * block, ComObjectWrap<ID3DXEffect>& pEffect, float dTime );

	Moo::BaseTexturePtr cookieTexture() { return cookieTexture_; }
	Moo::BaseTexturePtr nullSim() { return nullSim_; }
	int simulationSize() const { return simulationSize_; }

#ifdef _DEBUG
	static bool s_paused_;
#endif

	static SimulationManager&		instance( ) { return *s_instance_; }
private:
	typedef std::vector< SimulationTextureBlock > TextureBlocks;
	TextureBlocks					blocks_;
	int								availableBlocks_;
	int								simulationSize_;

	Moo::BaseTexturePtr				cookieTexture_; //interaction cookie
	Moo::BaseTexturePtr				nullSim_;

	static SimulationManager*		s_instance_;
	static float					s_maxIdleTime_;
	static int						s_maxTextureBlocks_;

	//Rain stuff
	RainSimulation					rainSimulation_;

	void initInternal();
	void finiInternal();

	SimulationManager();
	~SimulationManager();
};


/**
 *	This class is represents an area of water simulation.
 */
class SimulationCell : public Simulation
{
public:
	SimulationCell();

	void init( int texSize, float cellSize );	
	virtual void clear();

	// Simulation activity
	virtual void tick();
	virtual void activate();
	virtual void deactivate();
	virtual bool isActive() const;
	void simulate( Moo::EffectMaterialPtr effect, float dTime, Waters& group );

	void stitchEdges( Moo::EffectMaterialPtr effect, float dTime, Waters& group );

	// Inter-cell activation / simulation
	void edge( int e ) { edge_ = e; }
	int edge( ) { return edge_; }
	void addEdge( int e ){ edge_ |= (1<<e); }	
	int getActiveEdge( float threshold );
	void bindAsNeighbour( Moo::EffectMaterialPtr effect, const std::string& binding );
	virtual void bindNeighbour( Moo::EffectMaterialPtr effect, int edge ) { }

	void mark( uint32 mark );
	uint32 mark() const;

	SimulationTextureBlock* simulationBlock() const;
	void simulationBlock( SimulationTextureBlock* val);

	static void createUnmanaged();

	void push() { simulationTarget_->push(); }
	void pop() { simulationTarget_->pop(); }

	Moo::RenderTargetPtr previous() { return simulationTarget_->previous(); } 
	Moo::RenderTargetPtr current() { return simulationTarget_->current(); } 
	Moo::RenderTargetPtr result() { return simulationTarget_->result(); }

	static float s_hitTime,s_waveSpeedSquared,s_landScaleX,s_landScaleY,s_landScaleZ;


	const bool edgeActivation() const { return edgeActivation_; }
	void edgeActivation(bool val) { edgeActivation_ = val; }

protected:
	// flags an activation as indirect (not from a movement entering the cell).
	bool edgeActivation_;

	uint32 tickMark_;

	//TODO: multiple edges for activation
	//void addActiveEdge(int edge) { activeEdges_ |= (1 << edge); } //not used yet...
	//byte activeEdges_;
private:
	int						edge_;
	float					cellSize_;
	SimulationTextureBlock*	simulationTarget_;
};


#ifdef CODE_INLINE
#include "water_simulation.ipp"
#endif

#endif //_WATER_SIMULATION_HPP
/*water_simulation.hpp*/
