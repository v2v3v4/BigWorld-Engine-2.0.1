/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"

#include "water_splash.hpp"

#include "moo/moo_dx.hpp"
#include "resmgr/auto_config.hpp"
#include "duplo/py_attachment.hpp"
#include "particle/meta_particle_system.hpp"
#include "particle/particle_system.hpp"
#include "particle/actions/source_psa.hpp"
#include "particle/actions/vector_generator.hpp"
#include "particle/renderers/particle_system_renderer.hpp"
#include "moo/vertex_buffer_wrapper.hpp"
using Moo::VertexBufferWrapper;

DECLARE_DEBUG_COMPONENT2( "Romp", 0 )

// ----------------------------------------------------------------------------
// Statics
// ----------------------------------------------------------------------------

float	s_splashHeight		= 0.0f;
float	s_splashSpacing		= 0.15f;
float	s_splashScale		= 2.5f;
float	s_avgVelocity		= 0.5f;
bool	s_enableSplash		= true;
bool	s_enableVelocity	= true;

static AutoConfigString s_waterSplash( "environment/waterSplashParticles" );
static AutoConfigString s_waterImpact( "environment/waterImpactParticles" );
static AutoConfigString s_edgeMap( "environment/waterEdgeMap" );

Vector3	SplashManager::s_lastSplash_(0,0,0);

// ----------------------------------------------------------------------------
// Defines
// ----------------------------------------------------------------------------

#define VERTEX_TYPE_WAVE Moo::VertexXYZNDUV

// ----------------------------------------------------------------------------
// Section: SplashMatrixLiaison
// ----------------------------------------------------------------------------

class SplashMatrixLiaison : public MatrixLiaison, public Aligned
{
public:
	SplashMatrixLiaison():
	  worldTransform_( Matrix::identity )
	{
	}

	virtual const Matrix & getMatrix() const
	{
		return worldTransform_;
	}

	virtual bool setMatrix( const Matrix & m )
	{
		worldTransform_ = m;
		return true;
	}

private:
	Matrix	worldTransform_;
};

// ----------------------------------------------------------------------------
// Section: SplashManager
// ----------------------------------------------------------------------------

/**
 *	SplashManager contructor
 */
SplashManager::SplashManager() :
	initialised_( false )
{
}


/**
 *	SplashManager destructor
 */
SplashManager::~SplashManager()
{
	fini();
}


/**
 *	SplashManager initialisation
 */
void SplashManager::init()
{
	if (!initialised_)
	{
		static bool first = true;
		if (first)
		{
			MF_WATCH( "Client Settings/Water/Splash/enable splash", s_enableSplash, 
				Watcher::WT_READ_WRITE,
				"enable splashes" );		

			MF_WATCH( "Client Settings/Water/Splash/variable speed", s_enableVelocity, 
				Watcher::WT_READ_WRITE,
				"enable adjustments to the splash particle speed based on impact strength" );

			MF_WATCH( "Client Settings/Water/Splash/splash spacing", s_splashSpacing, 
				Watcher::WT_READ_WRITE,
				"splash spacing" );		

			MF_WATCH( "Client Settings/Water/Splash/splash scale", s_splashScale, 
				Watcher::WT_READ_WRITE,
				"splash scale" );		

			MF_WATCH( "Client Settings/Water/Splash/splash height", s_splashHeight, 
				Watcher::WT_READ_WRITE,
				"splash height" );

			MF_WATCH( "Client Settings/Water/Splash/avg vel", s_avgVelocity, 
				Watcher::WT_READ_WRITE,
				"splash avg vel" );

			first = false;
		}				
		waterSplash_.splash_ = new MetaParticleSystem();
		waterSplash_.splash_->load( s_waterSplash.value(), "" );
		waterSplash_.attachment_ = new SplashMatrixLiaison;
		waterSplash_.splash_->attach( waterSplash_.attachment_ );

		waterSplash_.impact_ = new MetaParticleSystem();		
		waterSplash_.impact_->load( s_waterImpact.value(), "" );
		waterSplash_.impact_->attach( waterSplash_.attachment_ );

		if (waterSplash_.splash_)
		{
			for (uint32 i=0; i<waterSplash_.splash_->nSystems(); i++)
			{
				ParticleSystemPtr ps = waterSplash_.splash_->systemFromIndex(i);
				if (ps)
				{
					SourcePSA * pSplashSource_ =
						static_cast<SourcePSA*>(&*ps->pAction(PSA_SOURCE_TYPE_ID));

					VectorGenerator* vel = pSplashSource_->getVelocitySource();
					if (vel->nameID() == PointVectorGenerator::nameID_)
					{
						PointVectorGenerator* pointVel = static_cast<PointVectorGenerator*>(vel);
						pointVGen_.push_back( std::make_pair(pointVel, pointVel->position().y) );
					}
					else if (vel->nameID() == CylinderVectorGenerator::nameID_)
					{
						CylinderVectorGenerator* cylVel = static_cast<CylinderVectorGenerator*>(vel);
					
						cylinderVGen_.push_back(
							std::make_pair(	cylVel,
								Vector2(cylVel->origin().y,
										cylVel->destination().y) ) );
					}
				}
			}
		}

		initialised_ = true;
	}
}


/**
 *	SplashManager finalisation
 */
void SplashManager::fini()
{
	if (initialised_)
	{
		if (waterSplash_.attachment_)
		{
			waterSplash_.splash_->detach();
			waterSplash_.impact_->detach();
			delete waterSplash_.attachment_;
			waterSplash_.attachment_ = NULL;
		}

		waterSplash_.splash_ = NULL;
		waterSplash_.impact_ = NULL;

		initialised_ = false;
	}
}


/**
 *	Adds a splash to be system....
 */
void SplashManager::addSplash( const Vector4& impact, const float height, bool forceSplash )
{ 
	static uint lastAdded = 0;
	
	Vector3 impactV3(impact.x,impact.y,impact.z);
	uint splashIndex = 0;

	if (!forceSplash && fabs((impactV3 - s_lastSplash_).length()) < s_splashSpacing )
		return;

	s_lastSplash_ = impactV3;

	if (!forceSplash)
		waterSplash_.splash_->spawn();
	waterSplash_.impact_->spawn();

	Matrix splashScale;
	splashScale.setScale(s_splashScale,s_splashScale,s_splashScale);		

	Matrix splashTransform;
	splashTransform.setTranslate( impact.x, height + s_splashHeight, impact.z );
	splashTransform.preMultiply(splashScale);

	if (waterSplash_.attachment_)
		waterSplash_.attachment_->setMatrix(splashTransform);

	// Varying the velocities of the splash particles...
	if (s_enableVelocity)
	{
		float speed = impact.w * s_avgVelocity;
		uint i=0;
		for (; i<pointVGen_.size(); i++)
		{
			PointVectorGenerator* vgen = pointVGen_[i].first;
			Vector3 pos(vgen->position());
			pos.y = pointVGen_[i].second * speed;			
			vgen->position(pos);
		}
		for (i=0; i<cylinderVGen_.size(); i++)
		{
			CylinderVectorGenerator* vgen = cylinderVGen_[i].first;
			
			Vector3 orig(vgen->origin());
			orig.y = cylinderVGen_[i].second.x * speed;
			vgen->origin(orig);

			Vector3 dest(vgen->destination());
			dest.y = cylinderVGen_[i].second.y * speed;
			vgen->destination(dest);
		}
	}	
}


/**
 *	SplashManager draw function
 */
void SplashManager::draw( float dTime )
{
	if (s_enableSplash)
	{
		if (waterSplash_.splash_ != NULL)
		{
			waterSplash_.splash_->tick(dTime);
			waterSplash_.splash_->draw( Matrix::identity, NULL );
			if ( waterSplash_.impact_ != NULL )
			{
				waterSplash_.impact_->tick(dTime);
				waterSplash_.impact_->draw( Matrix::identity, NULL );
			}
		}
	}
}



// ----------------------------------------------------------------------------
// Section: WaterEdgeMesh
//
// NOTE: experimental, not currently being used...
// ----------------------------------------------------------------------------

class WaterEdgeWaves
{
public:
	typedef SmartPointer< Moo::VertexBufferWrapper< VERTEX_TYPE_WAVE > > WaveVertexBufferPtr;

	WaterEdgeWaves() : 
		initialised_( false ),
		edgeTexture_( NULL )		
	{
	}

	void draw( Moo::EffectMaterialPtr effect );
	bool init();
	void rebuild();
	void loadResources();
	void createIndices();
	void createManagedObjects();	

private:
	typedef std::vector< int32 >		MeshIndices;

	MeshIndices							wWaveIndex_;
	std::vector<Vector2>				wWaveTex_;
	
	ComObjectWrap< DX::IndexBuffer >	waveIndexBuffer_;
	uint32								nWaveIndices_;

	WaveVertexBufferPtr					waveVertexBufferPtr_;
	//VertexBufferPtr						waveVertexBufferPtr_;
	uint32								nWaveVertices_;

	Moo::BaseTexturePtr					edgeTexture_;

	bool								initialised_;
};

void WaterEdgeWaves::loadResources()
{
	edgeTexture_ = Moo::TextureManager::instance()->get(s_edgeMap, true, true, true, "texture/water");
}
void WaterEdgeWaves::draw( Moo::EffectMaterialPtr effect )
{
	if (!init())
		return;

	if (nWaveVertices_)
	{		
		Moo::rc().setFVF( VERTEX_TYPE_WAVE::fvf() );
		waveVertexBufferPtr_->activate();
		ComObjectWrap<ID3DXEffect> pEffect = effect->pEffect()->pEffect();
		effect->hTechnique( "water_wave" );
		if (edgeTexture_)
			pEffect->SetTexture("edgeTexture", edgeTexture_->pTexture());
		if (Moo::rc().device()->SetIndices( waveIndexBuffer_.pComObject() ) == D3D_OK)
		{
			effect->begin();
			for (uint32 pass=0; pass<effect->nPasses();pass++)
			{
				effect->beginPass(pass);
				Moo::rc().drawIndexedPrimitive( D3DPT_TRIANGLESTRIP, 0, 0, nWaveVertices_, 0, nWaveIndices_ - 2 );
				effect->endPass();
			}
			effect->end();
		}
	}
}


bool WaterEdgeWaves::init()
{
	if (!initialised_)
	{
//		// BUILD THE WAVE VERTS
//		// can i just put this in the other water buffer? probably.. with new indices.
//		DEBUG_MSG( "Water waves using %d vertices\n", nWaveVertices_ );
//
//		typedef VertexBuffer< VERTEX_TYPE_WAVE > WVBufferType;
//		//typedef VertexBuffer< VERTEX_TYPE > WVBufferType;
//		waveVertexBufferPtr_ = new WVBufferType;
//				
//		if (waveVertexBufferPtr_->reset( nWaveVertices_ ) &&
//			waveVertexBufferPtr_->lock())
//		{
//			WVBufferType::iterator vbIt  = waveVertexBufferPtr_->begin();
//			WVBufferType::iterator vbEnd = waveVertexBufferPtr_->end();
//						
//			uint32 index = 0;
//			//WaterAlphas::iterator waterRigidity = wRigid_.begin();
//			//WaterAlphas::iterator waterAlpha = wAlpha_.begin();
//
//			float z = -config_.size_.y * 0.5f;
//			float zT = 0.f;
//
//			for (uint32 zIndex = 0; zIndex < gridSizeZ_; zIndex++)
//			{
//				if ((zIndex+1) == gridSizeZ_)
//				{
//					z = config_.size_.y * 0.5f;
//					zT = config_.size_.y;
//				}
//
//				float x = -config_.size_.x * 0.5f;
//				float xT = 0.f;
//				for (uint32 xIndex = 0; xIndex < gridSizeX_; xIndex++)
//				{
//					if ((xIndex+1) == gridSizeX_)
//					{
//						x = config_.size_.x * 0.5f;
//						xT = config_.size_.x;
//					}
//
//					if (wWaveIndex_[ index ] >= 0)
//					{
//						// Set the position of the water point.
//						//vbIt->pos_.set( x, 0, z );
//						vbIt->pos_.set( x, 0.1f, z );
//						
//						vbIt->normal_.set( 0, 1, 0 );
//
//		//							if (config_.useEdgeAlpha_)
////							vbIt->colour_ = *(waterAlpha);
//		//			vbIt->colour_ = *(waterAlpha);
//		///							else
//					vbIt->colour_ = uint32(0xffffffff);
//
//						//wAlpha_[ index ] = ( 255UL ) << 24;
//
//						//vbIt->uv_.set( xT / config_.textureTessellation_, zT / config_.textureTessellation_);
//
//						float uv = (xT / config_.textureTessellation_) + (zT / config_.textureTessellation_);
//
//						vbIt->uv_.set( uv, uv);
//						++vbIt;
//					}
//
//					//++waterRigidity;
//					//++waterAlpha;
//					++index;
//					x += config_.tessellation_;
//					xT += config_.tessellation_;
//				}
//				z += config_.tessellation_;
//				zT += config_.tessellation_;
//			}
//			waveVertexBufferPtr_->unlock();		
//		}
//		else
//		{
//			ERROR_MSG(
//				"Water::createManagedObjects: "
//				"Could not create/lock vertex buffer\n");
//		}
//		MF_ASSERT( waveVertexBufferPtr_.getObject() != 0 );
//
//		initialised_ = true;
	}

	return initialised_;
}


void WaterEdgeWaves::rebuild()
{	
	initialised_ = false;

	wWaveIndex_.clear();
	wWaveTex_.clear();

	createIndices();
}


/**
 *
 */
void WaterEdgeWaves::createIndices( )
{
//	uint32 index = 0;
//	int32 waterIndex = 0;
//	for (int zIndex = 0; zIndex < int( gridSizeZ_ ); zIndex++ )
//	{
//		for (int xIndex = 0; xIndex < int( gridSizeX_ ); xIndex++ )
//		{
//			int realNeighbours = 0;
//			for (int cz = zIndex - 1; cz <= zIndex + 1; cz++ )
//			{
//				for (int cx = xIndex - 1; cx <= xIndex + 1; cx++ )
//				{
//					if (( cx >= 0 && cx < int( gridSizeX_ ) ) &&
//						( cz >= 0 && cz < int( gridSizeZ_ ) ))
//					{
//						//if (!wRigid_[ cx + ( cz * gridSizeX_ ) ])
//						if (wRigid_[ cx + ( cz * gridSizeX_ ) ])
//						{
//							realNeighbours++;
//						}
//					}
//				}
//			}
//
//			//if (wRigid_[ xIndex + ( zIndex * gridSizeX_ ) ])
//			//{
//			//	realNeighbours++;
//			//}
//
////			wWaveTex_
//
//			if (realNeighbours > 0)
//				wWaveIndex_.push_back(waterIndex++);
//			else
//				wWaveIndex_.push_back(-1);
//		}
//	}
//	memoryClaim( wWaveIndex_ );
//
//	nWaveVertices_ = waterIndex; 
}


void WaterEdgeWaves::createManagedObjects()
{	
////////////////////
// Create the indices for the waves

	//	int xMax = int( ceilf( (size().x) / config_.tessellation_)  ) + 1;
	//	int zMax = int( ceilf( (size().y) / config_.tessellation_)  ) + 1;

	//	if (xMax > int( gridSizeX_ ))
	//		xMax = int( gridSizeX_ );

	//	if (zMax > int( gridSizeZ_ ))
	//		zMax = int( gridSizeZ_ );

	//std::vector< uint32 > indices;

	//uint32 lastIndex = 0;
	//bool instrip = false;
	//uint32 gridIndex = 0;

	//for (uint z = 0; z < uint32(zMax) - 1; z++)
	//{
	//	for (uint x = 0; x < uint32(xMax); x++)
	//	{
	//		if (!instrip &&
	//			wWaveIndex_[ gridIndex ] >= 0 &&
	//			wWaveIndex_[ gridIndex + 1 ] >= 0 &&
	//			wWaveIndex_[ gridIndex + gridSizeX_ ] >= 0 &&
	//			wWaveIndex_[ gridIndex + gridSizeX_ + 1 ] >= 0
	//			)
	//		{
	//			indices.push_back( lastIndex );
	//			indices.push_back( uint32( wWaveIndex_[ gridIndex ] ));
	//			indices.push_back( uint32( wWaveIndex_[ gridIndex ] ));
	//			indices.push_back( uint32( wWaveIndex_[ gridIndex + gridSizeX_] ));
	//			instrip = true;
	//		}
	//		else
	//		{
	//			if (wWaveIndex_[ gridIndex ] >= 0 &&
	//				wWaveIndex_[ gridIndex + gridSizeX_] >= 0  &&
	//				instrip)
	//			{
	//				indices.push_back( uint32( wWaveIndex_[ gridIndex ] ) );
	//				indices.push_back( uint32( wWaveIndex_[ gridIndex + gridSizeX_] ) );
	//				lastIndex = uint32( wWaveIndex_[ gridIndex + gridSizeX_] );
	//			}
	//			else
	//				instrip = false;
	//		}

	//		if (x == xMax - 1)
	//			instrip = false;

	//		++gridIndex;
	//	}

	//	gridIndex += (gridSizeX_ - uint32(xMax)) + 0;
	//}


	//// if it doesnt have any indices then the water doesnt have any edges
	//if (indices.size() != 0)
	//{
	//	indices.erase( indices.begin(), indices.begin() + 2 );
	//	nWaveIndices_ = indices.size();

	//	// Create the index buffer
	//	DX::IndexBuffer* pBuffer = NULL;
	//	if( SUCCEEDED( Moo::rc().device()->CreateIndexBuffer( nWaveIndices_ * sizeof( DWORD ),
	//		D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_MANAGED, &pBuffer, NULL ) ) )
	//	{
	//		DWORD* pIndices;
	//		waveIndexBuffer_ = pBuffer;
	//		pBuffer->Release();
	//		if(SUCCEEDED( waveIndexBuffer_->Lock(	0,
	//											nWaveIndices_ * sizeof(DWORD),
	//											(void**)&pIndices, 
	//											0 )))
	//		{
	//			memcpy( pIndices, &indices.front(), sizeof(DWORD) * nWaveIndices_ );
	//			waveIndexBuffer_->Unlock();
	//		}
	//	}
	//	memoryCounterAdd( water );
	//	memoryClaim( &*waveIndexBuffer_ );
	//}

}


// water_splash.cpp
