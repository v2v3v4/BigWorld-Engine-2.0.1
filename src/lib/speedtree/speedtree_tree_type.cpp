/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


// Module Interface
#include "speedtree_config.hpp"

// BW Tech Hearders
#include "cstdmf/debug.hpp"
#include "cstdmf/diary.hpp"

#if SPEEDTREE_SUPPORT // -------------------------------------------------------

// BW Tech Headers
#include "cstdmf/dogwatch.hpp"
#include "cstdmf/concurrency.hpp"
#include "cstdmf/smartpointer.hpp"
#include "resmgr/bwresource.hpp"
#include "moo/render_context.hpp"
#include "romp/enviro_minder.hpp"
#include "romp/weather.hpp"

#include "speedtree_tree_type.hpp"

// SpeedTree API
#include <SpeedTreeRT.h>
#include <SpeedTreeAllocator.h>

namespace speedtree {

namespace { // anonymous
// Named contants
const float c_maxWindVelocity = 15.0f;
} // namespace anonymous

float TSpeedTreeType::s_windVelX_ = 0;
float TSpeedTreeType::s_windVelZ_ = 0;

TSpeedTreeType::WindAnimation	TSpeedTreeType::s_defaultWind_;
TSpeedTreeType::WindAnimation*	TSpeedTreeType::s_currentWind_ = NULL;

int TSpeedTreeType::DrawData::s_count_ = 0;

/**
 *	Stores the current wind information. It will
 *	later be used to animate the trees to the wind.
 *
 *	@param	envMinder	the current EnvironMander object.
 */
void TSpeedTreeType::saveWindInformation( EnviroMinder * envMinder )
{
	BW_GUARD;
	if ( envMinder != NULL )
	{
		TSpeedTreeType::s_windVelX_ = envMinder->weather()->wind().x;
		TSpeedTreeType::s_windVelZ_ = envMinder->weather()->wind().y;
	}
}

//TODO: count the wind instances and update a watcher
TSpeedTreeType::WindAnimation::WindAnimation( ) :
	lastTickTime_( -1 ),
	leafAnglesCount_( 0 ),
	bLeaves_( true ),
	lastEffect_( NULL ),
	speedWind_( NULL )
{
	BW_GUARD;
	speedWind_ = new CSpeedWind();
}

TSpeedTreeType::WindAnimation::~WindAnimation( )
{
	BW_GUARD;
	delete speedWind_;	speedWind_ = NULL;
}

int TSpeedTreeType::WindAnimation::matrixCount() const
{
	BW_GUARD;
	return speedWind_->GetNumLeafAngleMatrices();
}

TSpeedTreeType::WindAnimation::UsedEffectMap TSpeedTreeType::WindAnimation::s_usedEffectMap_;

/*static*/ TSpeedTreeType::WindAnimation* 
TSpeedTreeType::WindAnimation::lastWindUsed(ID3DXEffect* pEffect)
{
	BW_GUARD;
	UsedEffectMap::iterator it = s_usedEffectMap_.find(pEffect);
	if (it != s_usedEffectMap_.end())
	{
		return (*it).second;
	}
	else
		return NULL;
}

/*static*/ void
TSpeedTreeType::WindAnimation::lastWindUsed(ID3DXEffect* pEffect, WindAnimation* pWind)
{
	BW_GUARD;
	UsedEffectMap::iterator it = s_usedEffectMap_.find(pEffect);
	if (it != s_usedEffectMap_.end())
	{
		(*it).second = pWind;
	}
	else
	{
		s_usedEffectMap_[pEffect] = pWind;
	}
}

/**
 * Upload the wind parameters to the given effect.
 */
void TSpeedTreeType::WindAnimation::apply( ComObjectWrap<ID3DXEffect> pEffect )
{
	BW_GUARD;	
//This optimisation seems to be broken by the depth pass stuff.....disabling for now
//	if (lastWindUsed(pEffect.pComObject()) == this)
//		return;
//	else
//		lastWindUsed(pEffect.pComObject(), this);

	int matricesCount = speedWind_->GetNumWindMatrices();
	D3DXHANDLE bwind = pEffect->GetParameterByName( NULL, "g_windMatrices" );
	for ( int i=0; i<matricesCount; ++i )
	{
		const float * windMatrix = speedWind_->GetWindMatrix( i );
		D3DXHANDLE hWindMatrix = 
			pEffect->GetParameterElement( bwind, i );
		if ( hWindMatrix )
		{
			pEffect->SetMatrix( hWindMatrix, (const D3DXMATRIX*)windMatrix );
		}
	}
	if ( bLeaves_ )
	{
		pEffect->SetVectorArray( "g_leafAngles",
								(D3DXVECTOR4*)&anglesTable_[0], 
								leafAnglesCount_ );
	}
}


/**
 *	Updates all wind related data.
 */
void TSpeedTreeType::WindAnimation::update()
{
	BW_GUARD;
	if ( this->lastTickTime_ != TSpeedTreeType::s_time_ )
	{
		const float windVelX = TSpeedTreeType::s_windVelX_;
		const float windVelZ = TSpeedTreeType::s_windVelZ_;
		
		Vector3 wind( windVelX, 0, windVelZ );
		float strength = std::min( wind.length()/c_maxWindVelocity, 1.0f );
		wind.normalise();
		speedWind_->SetWindStrengthAndDirection(
			strength, wind.x, wind.y, wind.z );

		float camDir[3], camPos[3];
		CSpeedTreeRT::GetCamera( camPos, camDir );
		const float & time = TSpeedTreeType::s_time_;
		speedWind_->Advance( time, true, true,
			camDir[0], camDir[1], camDir[2] );

		this->lastTickTime_ = time;
		
		if ( bLeaves_ )
		{
			// upload leaf angle table
			static const int numAngles = 128;
			leafAnglesCount_ = speedWind_->GetNumLeafAngleMatrices();
			MF_ASSERT(leafAnglesCount_ <= numAngles);
			
			static float windRockAngles[numAngles];
			static float windRustleAngles[numAngles];

			bool success =
				speedWind_->GetRockAngles( windRockAngles ) &&
				speedWind_->GetRustleAngles( windRustleAngles );
			if (success)
			{
				anglesTable_.resize( leafAnglesCount_*4 );
				for ( int i=0; i<leafAnglesCount_; ++i )
				{
					anglesTable_[i*4+0] = DEG_TO_RAD(windRockAngles[i]);
					anglesTable_[i*4+1] = DEG_TO_RAD(windRustleAngles[i]);
				}
			}
		}

		lastEffect_ = NULL;
	}
}


/**
 *	Initialise the SpeedWind object.
 */
bool TSpeedTreeType::WindAnimation::init( const std::string& iniFile )
{
	BW_GUARD;
	speedWind_->Reset();
	DataSectionPtr rootSection = BWResource::instance().rootSection();
	BinaryPtr windIniData = rootSection->readBinary( iniFile );
	if ( windIniData.exists() )
	{
		if ( speedWind_->Load( windIniData->cdata(), windIniData->len() ) )
		{
			// The number of matrices is tightly linked to the shaders...
			MF_ASSERT( speedWind_->GetNumWindMatrices( ) == 6 )
			return true;
		}
	}
	return false;
}


/**
 *	Updates all wind related data for this tree type.
 */
void TSpeedTreeType::updateWind()
{
	BW_GUARD;
	if ( TSpeedTreeType::s_playAnimation_ && pWind_ )
		pWind_->update( );
}

/**
 *	Uploads wind matrix constants to the rendering device.
 */
void TSpeedTreeType::uploadWindMatrixConstants( Moo::EffectMaterialPtr effect )
{
	BW_GUARD_PROFILER( TSpeedTreeType_setEffectParam );

	static DogWatch windWatch("Wind Matrix");
	ScopedDogWatch watcher(windWatch);

	ComObjectWrap<ID3DXEffect> pEffect = effect->pEffect()->pEffect();

	//TODO: remove this overriding of the leaf state
	// and replace it with proper sharing of param in the shaders
	bool oldVal = this->pWind_->hasLeaves();
	this->pWind_->hasLeaves( false );
	TSpeedTreeType::setWind( this->pWind_, pEffect );
	this->pWind_->hasLeaves( oldVal );
	pEffect->CommitChanges();
}


/**
 *	Sets up the SpeedWind object for this tree type. First, 
 *	try to load a tree specific setup file. If that can't be 
 *	found, the default wind is used.
 *
  *	@param	speedTree		the speedtree object that will use this speedwind.
 *	@param	datapath		data path where to look for the tree specific setup.
 *	@param	defaultWindIni	default ini file to use if tree specific not found.
 */
/*static*/
TSpeedTreeType::WindAnimation* TSpeedTreeType::setupSpeedWind(
	CSpeedTreeRT      & speedTree, 
	const std::string & datapath,
	const std::string & defaultWindIni )
{
	BW_GUARD;
	TSpeedTreeType::WindAnimation* pWind = NULL;
	static bool first = true;
	if ( first )
	{
		first = false;
		if ( !TSpeedTreeType::s_defaultWind_.init( defaultWindIni ) )
		{
			CRITICAL_MSG( "SpeedTree default wind .ini file not found\n" );
		}
	}

	std::string resName  = datapath + "speedwind.ini";
	// Check if there's an overriding ini
	if ( BWResource::fileExists( resName ) )
	{
		pWind = new TSpeedTreeType::WindAnimation();
		if ( !pWind->init( resName ) )
		{
			//Failed, fallback to the default wind.
			delete pWind;
			pWind = &TSpeedTreeType::s_defaultWind_;
		}
	}
	else
	{
		pWind = &TSpeedTreeType::s_defaultWind_;
	}
	speedTree.SetLeafRockingState(true);
	return pWind;
}


/**
 *	Add an instance of this tree to the list of deferred
 *	trees for later rendering.
 *
 *	@param	drawData	the DrawData struct for this instance.
 */
void TSpeedTreeType::deferTree( DrawData * drawData )
{
	BW_GUARD;	
#ifdef OPT_BUFFERS
	this->deferredTrees_.push_back( drawData );
#else
	this->deferredTrees_.push_back( *drawData );
#endif
	++TSpeedTreeType::s_deferredCount_;
}


/**
 *	Checks for an reference to the passed draw data.
 *	Used to ensure no invalid draw data pointers are left
 *	hanging.
 *
 *	@param	drawData	the DrawData struct to delete
 */
void TSpeedTreeType::deleteDeferred( DrawData * drawData )
{
	BW_GUARD;
	DrawDataVector::iterator it = this->deferredTrees_.begin();
	while (it != this->deferredTrees_.end())
	{
		if ((*it) == drawData)
			it = this->deferredTrees_.erase( it );
		else
			++it;
	}
}


}  // namespace speedtree

#endif // SPEEDTREE_SUPPORT