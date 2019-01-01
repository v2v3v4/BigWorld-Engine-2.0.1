/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef _WATER_SPLASH_HPP
#define _WATER_SPLASH_HPP

#include <vector>

#include "cstdmf/smartpointer.hpp"
#include "moo/moo_math.hpp"

class MetaParticleSystem;
class SplashMatrixLiaison;
class PointVectorGenerator;
class CylinderVectorGenerator;
typedef SmartPointer<MetaParticleSystem>	MetaParticleSystemPtr;

class SplashManager
{
public:
	SplashManager();
	~SplashManager();

	void init();
	void fini();

	void draw( float dTime );
	void addSplash( const Vector4& impact, const float height, bool force=false );
private:
	struct SplashData
	{
		MetaParticleSystemPtr	splash_;
		MetaParticleSystemPtr	impact_;
		SplashMatrixLiaison*	attachment_;
	};
	SplashData					waterSplash_;
	bool						initialised_;

	typedef std::pair<PointVectorGenerator*,float>		PointVecPair;
	typedef std::pair<CylinderVectorGenerator*,Vector2> CylinderVecPair;

	std::vector< PointVecPair >		pointVGen_;
	std::vector< CylinderVecPair >	cylinderVGen_;

	static Vector3					s_lastSplash_;
};

#endif //_WATER_SPLASH_HPP
/*water_splash.hpp*/
