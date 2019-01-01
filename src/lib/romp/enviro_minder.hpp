/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ENVIRO_MINDER_HPP
#define ENVIRO_MINDER_HPP

#include "cstdmf/smartpointer.hpp"
#include "duplo/pymodel.hpp"
#include "pyscript/script.hpp"
#include "pyscript/script_math.hpp"
#include "moo/camera_planes_setter.hpp"
#include "moo/visual.hpp"

class DataSection;
typedef SmartPointer<DataSection> DataSectionPtr;

class TimeOfDay;
class Weather;
class SkyGradientDome;
class SunAndMoon;
class Dapple;
class Seas;
class Rain;
class SkyLightMap;
class Flora;
class Decal;
class EnvironmentCubeMap;
class ShadowCaster;
class SkyDomeOccluder;
class SkyDomeShadows;
class ZBufferOccluder;
class ChunkObstacleOccluder;
class FootPrintRenderer;

struct WeatherSettings;
class PyMetaParticleSystem;

typedef uint32 ChunkSpaceID;

/**
 *	Something that wants to be attached to the main player model.
 */
struct PlayerAttachment
{
	PyMetaParticleSystem* pSystem;
	std::string		onNode;
};

/**
 *	A vector of PlayerAttachment objects
 */
class PlayerAttachments : public std::vector<PlayerAttachment>
{
public:
	void add( PyMetaParticleSystem* pSys, const std::string & node );
};


/**
 *	This class minds a whole lot of pointers to classes used to render
 *	and control the physical environment in the game. It deletes any
 *	pointers it holds in its destructor.
 */
class EnviroMinder
{
public:
	EnviroMinder(ChunkSpaceID spaceID);
	~EnviroMinder();

	static void init();
	static void fini();

	/**
	 * TODO: to be documented.
	 */
	struct DrawSelection
	{
		enum
		{
			// bit values for drawSkyCtrl are sorted in drawing order
			sunAndMoon	= 0x0001,
			skyGradient	= 0x0002,
			skyBoxes	= 0x0004,
			sunFlare	= 0x0008,

			all			= 0xffff
		};
		uint32		value;

		DrawSelection() : value( all ) {}

		operator uint32()			{ return value; }
		void operator=( uint32 v )	{ value = v; }
	};

	bool load( DataSectionPtr pDS, bool loadFromExternal = true );
#ifdef EDITOR_ENABLED
    bool save( DataSectionPtr pDS, bool saveToExternal = true ) const;
#endif
	const DataSectionPtr pData()			{ return data_; }

	void tick( float dTime, bool outside,
		const WeatherSettings * pWeatherOverride = NULL );

	void allowUpdate(bool val) { allowUpdate_ = val; }

	void activate();
	void deactivate();
	
	float farPlane() const;
	void setFarPlane(float farPlane);
	
	float farPlaneBaseLine() const;
	void setFarPlaneBaseLine(float farPlaneBaseLine);

	void drawHind( float dTime, DrawSelection drawWhat = DrawSelection(), bool showWeather = true );
	void drawHindDelayed( float dTime, DrawSelection drawWhat = DrawSelection() );
	void drawFore( float dTime, bool showWeather = true,
								bool showFlora = true,
								bool showFloraShadowing = false,
								bool drawOverLays = true,
								bool drawObjects = true );
	void tickSkyDomes( float dTime );
	void drawSkyDomes( bool isOcclusionPass = false );
	void drawSkySunCloudsMoon( float dTime, DrawSelection drawWhat );
	TimeOfDay *         timeOfDay()         { return timeOfDay_; }
	Weather *           weather()           { return weather_; }
	SkyGradientDome *   skyGradientDome()   { return skyGradientDome_; }
	SunAndMoon *        sunAndMoon()        { return sunAndMoon_; }
	Seas *              seas()              { return seas_; }
	Rain *              rain()              { return rain_; }
	SkyLightMap *       skyLightMap()       { return skyLightMap_; }
	Flora *				flora()				{ return flora_; }
	Decal *				decal()				{ return decal_; }
	EnvironmentCubeMap *environmentCubeMap(){ return environmentCubeMap_; }
#ifndef EDITOR_ENABLED
	FootPrintRenderer * footPrintRenderer() { return footPrintRenderer_; }
#endif // EDITOR_ENABLED

	const Vector4 & thunder() const			{ return thunder_; }
    std::vector<Moo::VisualPtr> &skyDomes() { return skyDomes_; }
	size_t skyDomesPartition() const		{ return skyDomesPartition_; }
	void skyDomesPartition( size_t p )		{ skyDomesPartition_ = p; }

	void addPySkyDome( PyAttachmentPtr att, Vector4ProviderPtr provider );
	void delPySkyDome( PyAttachmentPtr att, Vector4ProviderPtr provider );
	void delStaticSkyBoxes();

	PlayerAttachments & playerAttachments()	{ return playerAttachments_; }
	bool playerDead() const					{ return playerDead_; }
	void playerDead( bool isDead )			{ playerDead_ = isDead; }

#ifdef EDITOR_ENABLED
    std::string         timeOfDayFile() const;
    void                timeOfDayFile(std::string const &filename);

    std::string         skyGradientDomeFile() const;
    void                skyGradientDomeFile(std::string const &filename);
#endif

	static bool EnviroMinder::primitiveVideoCard();

	std::vector<Vector4ProviderPtr> skyDomeControllers_;
	Vector4ProviderPtr	weatherControl_;
	Vector4ProviderPtr	sunlightControl_;
	Vector4ProviderPtr	ambientControl_;
	Vector4ProviderPtr	fogControl_;

private:
	EnviroMinder( const EnviroMinder& );
	EnviroMinder& operator=( const EnviroMinder& );

	void decideLightingAndFog();

    void loadTimeOfDay(DataSectionPtr data, bool loadFromExternal);
    void loadSkyGradientDome(DataSectionPtr data, bool loadFromExternal,
            float &farPlane);

	void beginClipPlaneBiasDraw( float bias );
	void endClipPlaneBiasDraw();

	TimeOfDay		  *           timeOfDay_;
	Weather			  *           weather_;
	SkyGradientDome	  *           skyGradientDome_;
	SunAndMoon		  *           sunAndMoon_;
	Seas			  *           seas_;
	Rain			  *           rain_;
	SkyDomeShadows	  *			  skyDomeShadows_;
	SkyDomeOccluder   *           skyDomeOccluder_;
	ZBufferOccluder	  *			  zBufferOccluder_;
	ChunkObstacleOccluder *		  chunkObstacleOccluder_;
	
	//TODO : unsupported feature at the moment.  to be finished.
	//Dapple          *           dapple_;
	SkyLightMap       *           skyLightMap_;
	Flora             *           flora_;
	Decal             *           decal_;
	EnvironmentCubeMap *           environmentCubeMap_;
#ifndef EDITOR_ENABLED
	FootPrintRenderer *         footPrintRenderer_;
#endif // EDITOR_ENABLED
	
	std::vector<Moo::VisualPtr> skyDomes_;
	size_t						skyDomesPartition_;
	std::vector<PyAttachmentPtr> pySkyDomes_;	
	Vector4				        thunder_;
	PlayerAttachments	        playerAttachments_;
	bool				        playerDead_;
	DataSectionPtr	            data_;
	static EnviroMinder*		s_activatedEM_;

	float						farPlaneBaseLine_;
	float						farPlane_;
	
	bool 						allowUpdate_;

	// Saved in begin/end ClipPlaneBiasDraw
	float						savedNearPlane_;
	float						savedFarPlane_;

#ifdef EDITOR_ENABLED
    std::string                 todFile_;
    std::string                 sgdFile_;
#endif
};


/**
 *	Manages all graphics settings related to the EnviroMinder.
 */
class EnviroMinderSettings
{
public:
	void init(DataSectionPtr resXML);
	
	void activate(EnviroMinder * activeMinder);		
	void refresh();
	
    bool isInitialised() const;

	void shadowCaster( ShadowCaster* shadowCaster );
	ShadowCaster* shadowCaster() const;

	static EnviroMinderSettings & instance();

private:
	typedef std::vector<float> FloatVector;
	typedef Moo::GraphicsSetting::GraphicsSettingPtr GraphicsSettingPtr;

#ifndef EDITOR_ENABLED
	void setFarPlaneOption(int optionIndex);

	GraphicsSettingPtr farPlaneSettings_;
	FloatVector        farPlaneOptions_;
#endif // EDITOR_ENABLED

	EnviroMinder     * activeMinder_;
	ShadowCaster     * shadowCaster_;

private:
	EnviroMinderSettings() :
#ifndef EDITOR_ENABLED
		farPlaneSettings_(NULL),
		farPlaneOptions_(),
#endif // EDITOR_ENABLED
		activeMinder_(NULL),
		shadowCaster_(NULL)
	{}
};


/**
 *  This class encapsulates the graphics setup required for rendering sky boxes
 *	and similar elements. It's used in both EnviroMinder and in ModelEditor
 *	directly when loading skyboxes as models, so changes here need to be tested
 *	when editing skyboxes in ModelEditor.
 */
class SkyBoxScopedSetup
{
public:
	SkyBoxScopedSetup();
	~SkyBoxScopedSetup();
private:
	D3DVIEWPORT9 oldVp_;
	Moo::CameraPlanesSetter cps_;
};


#ifdef CODE_INLINE
#include "enviro_minder.ipp"
#endif

#endif // ENVIRO_MINDER_HPP
