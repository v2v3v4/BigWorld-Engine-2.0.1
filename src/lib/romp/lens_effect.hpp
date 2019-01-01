/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LENS_EFFECT_HPP
#define LENS_EFFECT_HPP

#include "cstdmf/smartpointer.hpp"
#include "moo/moo_math.hpp"
#include "moo/vertex_formats.hpp"
#include "resmgr/datasection.hpp"
#include <iostream>

///This const determines how quickly lens effects fade in/out
const float YOUNGEST_LENS_EFFECT = 0.f;		//at this age lens flare alpha = 1
const float OLDEST_LENS_EFFECT = 0.15f;		//at this age lens flare alpha = 0
const float KILL_LENS_EFFECT = 1.15f;		//1 second grace period before die

/**
 *	This class is describes the flare which is part of the lens effect.
 *	Flares can have secondary flares, such as coronas.
 */
class FlareData 
{
public:
	typedef std::vector< FlareData > Flares;

	FlareData();
	~FlareData();	

	void	load( DataSectionPtr pSection );

	uint32	colour() const;
	void	colour( uint32 c );

	const std::string & material() const;
	void	material( const std::string &  m );

	float	clipDepth() const;
	void	clipDepth( float d );

	float	size() const;
	void	size( float s );

	float	width() const;
	void	width( float w );

	float	height() const;
	void	height( float h );
	
	float	age() const;
	void	age( float a );

	const Flares & secondaries() const;

	void	draw( const Vector4 & clipPos, float alphaStrength, float scale, 
					uint32 lensColour ) const;

	void createMesh( const Vector4 & clipPos, float alphaStrength,
		float scale, uint32 lensColour, Moo::VertexTLUV2* vertex,
		float vizCoord = 0.5f, float halfScreenWidth = 0.f,
		float halfScreenHeight = 0.f ) const;
private:
	uint32			colour_;
	std::string		material_;
	float			clipDepth_;
	float			width_;
	float			height_;
	float			age_;

	Flares	secondaries_;
};


/**
 *	This class holds the properties of a lens effect, and
 *	performs tick/draw logic.
 */
class LensEffect : public ReferenceCount
{
public:
	static const uint32 DEFAULT_COLOUR = 0xffffffff;

	typedef std::map< float, class FlareData > OcclusionLevels;

	enum VisibilityType
	{
		VT_UNSET = 0,
		VT_TYPE1,
		VT_TYPE2
	};
	
	LensEffect();
	virtual ~LensEffect();

	virtual bool load( DataSectionPtr pSection );
	virtual bool save( DataSectionPtr pSection );

	float	age() const;
	void	ageBy( float a );

	uint32	id() const;
	void	id( uint32 value ); 

	const Vector3 & position() const;
	void	position( const Vector3 & pos );

	float	maxDistance() const;
	void	maxDistance( float md );
	float	distanceToAlpha( float dist ) const;

	bool	clampToFarPlane() const;
	void	clampToFarPlane( bool s ); 

	float	area() const;
	void	area( float a );

	float	fadeSpeed() const;
	void	fadeSpeed( float f );

	uint32 colour() const;
	void colour( uint32 c );
	void defaultColour();

	void size( float size );

	uint32	added() const;
	void	added( uint32 when );

	bool	visibilityType2() const;

	virtual void tick( float dTime, float visibility );	
	virtual void draw();

	int operator==( const LensEffect & other );

	const OcclusionLevels & occlusionLevels() const;
	
	static bool isLensEffect( const std::string & file );	

	LensEffect & operator=( const LensEffect & );

private: 
	uint32		id_;
	Vector3		position_;

	float		maxDistance_;
	float		area_;
	float		fadeSpeed_;
	float		visibility_;	
	bool		clampToFarPlane_;
	mutable VisibilityType	visibilityType_;

	uint32		colour_;

	OcclusionLevels occlusionLevels_;

	uint32		added_;
};


typedef SmartPointer< LensEffect >	LensEffectPtr;
typedef std::list<LensEffect>		LensEffects;
typedef std::map<uint32, LensEffect> LensEffectsMap;


#ifdef CODE_INLINE
#include "lens_effect.ipp"
#endif


#endif // LENS_EFFECT_HPP
