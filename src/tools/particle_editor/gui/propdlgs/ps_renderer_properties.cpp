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
#include "main_frame.hpp"
#include "particle_editor.hpp"
#include "gui/propdlgs/ps_renderer_properties.hpp"
#include "controls/dir_dialog.hpp"
#include "particle/meta_particle_system.hpp"
#include "particle/actions/collide_psa.hpp"
#include "particle/renderers/particle_system_renderer.hpp"
#include "particle/renderers/mesh_particle_renderer.hpp"
#include "particle/renderers/visual_particle_renderer.hpp"
#include "particle/renderers/amp_particle_renderer.hpp"
#include "particle/renderers/blur_particle_renderer.hpp"
#include "particle/renderers/trail_particle_renderer.hpp"
#include "particle/renderers/sprite_particle_renderer.hpp"
#include "particle/renderers/point_sprite_particle_renderer.hpp"
#include "ual/ual_manager.hpp"
#include "resmgr/bwresource.hpp"
#include "resmgr/auto_config.hpp"
#include "resmgr/multi_file_system.hpp"
#include "resmgr/string_provider.hpp"
#include "appmgr/Options.hpp"
#include "gizmo/general_editor.hpp"
#include "gizmo/combination_gizmos.hpp"

using namespace std;

DECLARE_DEBUG_COMPONENT2( "GUI", 0 )

namespace
{
    const CString c_defaultDirectoryText("No Directory");

    AutoConfigString s_notFoundTexture("system/notFoundBmp");
    AutoConfigString s_notFoundModel("system/notFoundModel");	
    AutoConfigString s_notFoundMeshModel("system/notFoundMeshPSModel");

    void AddBitmapDrop
    (
        CWnd                                            *control,
        PsRendererProperties                            *dialog,
        UalDropFunctor<PsRendererProperties>::Method    method
    )
    {
		BW_GUARD;

        UalManager::instance().dropManager().add
        (
            new UalDropFunctor<PsRendererProperties>
            (
                control, 
                "bmp", 
                dialog,
                method
            )
        );
        UalManager::instance().dropManager().add
        (
            new UalDropFunctor<PsRendererProperties>
            (
                control, 
                "dds", 
                dialog,
                method
            )
        );
        UalManager::instance().dropManager().add
        (
            new UalDropFunctor<PsRendererProperties>
            (
                control, 
                "tga", 
                dialog,
                method
            )
        );
		UalManager::instance().dropManager().add
        (
            new UalDropFunctor<PsRendererProperties>
            (
                control, 
                "texanim", 
                dialog,
                method
            )
        );
    }

    void AddMeshDrop
    (
        CWnd                                            *control,
        PsRendererProperties                            *dialog,
        UalDropFunctor<PsRendererProperties>::Method    method,
        UalDropFunctor<PsRendererProperties>::Method2   method2
    )
    {
		BW_GUARD;

        UalManager::instance().dropManager().add
        (
            new UalDropFunctor<PsRendererProperties>
            (
                control, 
                "visual", 
                dialog,
                method,
                false,
                method2
            )
        );
    }

    void AddMFMDrop
    (
        CWnd                                            *control,
        PsRendererProperties                            *dialog,
        UalDropFunctor<PsRendererProperties>::Method    method
    )
    {
		BW_GUARD;

        UalManager::instance().dropManager().add
        (
            new UalDropFunctor<PsRendererProperties>
            (
                control, 
                "mfm", 
                dialog,
                method
            )
        );
    }

    bool validMeshFilename(const std::string &filename, const std::string &fullname)
    {
		BW_GUARD;

	    std::string extension = BWResource::getExtension( filename );
	    if (extension != "visual")
            return false;        
        if (!MeshParticleRenderer::quickCheckSuitableVisual(fullname))
            return false;
        return true;
    }

    bool validVisualFilename(const std::string &filename, const std::string &)
    {
		BW_GUARD;

 	    std::string extension = BWResource::getExtension( filename );
	    return extension == "visual";
    }

    bool validMaterialFilename(const std::string &filename, const std::string &)
    {
		BW_GUARD;

	    std::string extension = BWResource::getExtension( filename );
	    return extension == "mfm";
    }

    bool validTextureFilename(const std::string &, const std::string &fullname)
    {
		BW_GUARD;

		return Moo::TextureManager::instance()->isTextureFile( fullname );
    }

	typedef bool (*TestFn)(const std::string &s, const std::string &t);

    void 
    populateComboBoxWithFilenames
    (
        CComboBox       &theBox, 
        string          const &relativeDirectory, 
        TestFn          test
    )
    {
		BW_GUARD;

	    // show a wait cursor as this may take a while
	    CWaitCursor wait;

	    theBox.ResetContent();
	    theBox.ShowWindow(SW_HIDE);

        IFileSystem::Directory directory = 
            BWResource::instance().fileSystem()->readDirectory(relativeDirectory);
   	
	    if (!directory.empty())
	    {
	        theBox.InitStorage((int)directory.size(), 64);	// approx 64 characters per filename

            for 
            (
                IFileSystem::Directory::iterator it = directory.begin(); 
                it != directory.end(); 
                ++it
            )
		    {
				std::string fullname = relativeDirectory + (*it);
			    if (test((*it), fullname))
			    {
				    theBox.AddString(bw_utf8tow( *it ).c_str());
			    }
		    }
	    }

	    theBox.ShowWindow(SW_SHOW);
    }

    std::pair<UINT, UINT> spriteFX[] =
    {
        make_pair<UINT, UINT>(IDS_ADDITIVE              , SpriteParticleRenderer::FX_ADDITIVE              ),
        make_pair<UINT, UINT>(IDS_ADDITIVE_ALPHA        , SpriteParticleRenderer::FX_ADDITIVE_ALPHA        ),
        make_pair<UINT, UINT>(IDS_BLENDED               , SpriteParticleRenderer::FX_BLENDED               ),
        make_pair<UINT, UINT>(IDS_BLENDED_COLOUR        , SpriteParticleRenderer::FX_BLENDED_COLOUR        ),
        make_pair<UINT, UINT>(IDS_BLENDED_INVERSE_COLOUR, SpriteParticleRenderer::FX_BLENDED_INVERSE_COLOUR),
        make_pair<UINT, UINT>(IDS_SOLID                 , SpriteParticleRenderer::FX_SOLID                 ),
        make_pair<UINT, UINT>(IDS_SHIMMER               , SpriteParticleRenderer::FX_SHIMMER               ),
        make_pair<UINT, UINT>(IDS_SOURCE_ALPHA          , SpriteParticleRenderer::FX_SOURCE_ALPHA          )
    };
    size_t spriteFXSz = sizeof(spriteFX)/sizeof(std::pair<UINT, UINT>);

    std::pair<UINT, UINT> meshMaterialFX[] =
    {
        make_pair<UINT, UINT>(IDS_ADDITIVE, MeshParticleRenderer::FX_ADDITIVE),
        make_pair<UINT, UINT>(IDS_BLENDED , MeshParticleRenderer::FX_BLENDED ),
        make_pair<UINT, UINT>(IDS_SOLID   , MeshParticleRenderer::FX_SOLID   )
    };
    size_t meshMaterialFXSz = sizeof(meshMaterialFX)/sizeof(std::pair<UINT, UINT>);

    std::pair<UINT, UINT> meshSort[] =
    {
        make_pair<UINT, UINT>(IDS_NONE    , MeshParticleRenderer::SORT_NONE    ),
        make_pair<UINT, UINT>(IDS_QUICK   , MeshParticleRenderer::SORT_QUICK   ),
        make_pair<UINT, UINT>(IDS_ACCURATE, MeshParticleRenderer::SORT_ACCURATE),
    };
    size_t meshSortSz = sizeof(meshSort)/sizeof(std::pair<UINT, UINT>);
}


/*static*/
Vector3 PsRendererProperties::s_lastExplicitOrientation_( 1.f, 0.f, 0.f );


BEGIN_MESSAGE_MAP(PsRendererProperties, CFormView)
	ON_MESSAGE(WM_EDITNUMERIC_CHANGE, OnUpdatePsRenderProperties)

	ON_BN_CLICKED(IDC_PS_RENDERER_WORLDDEPENDENT, OnWorldDependentBtn)
	ON_BN_CLICKED(IDC_PS_RENDERER_LOCALDEPENDENT, OnLocalDependentBtn)
	ON_BN_CLICKED(IDC_PS_RENDERER_VIEWDEPENDENT, OnViewDependentBtn)
	ON_BN_CLICKED(IDC_PS_RENDERER_SPRITE, OnSpriteBtn)
	ON_BN_CLICKED(IDC_PS_RENDERER_MESH  , OnMeshBtn  )
    ON_BN_CLICKED(IDC_PS_RENDERER_VISUAL, OnVisualBtn)
	ON_BN_CLICKED(IDC_PS_RENDERER_AMP   , OnAmpBtn   )
	ON_BN_CLICKED(IDC_PS_RENDERER_TRAIL , OnTrailBtn )
	ON_BN_CLICKED(IDC_PS_RENDERER_BLUR  , OnBlurBtn  )

    ON_BN_CLICKED(IDC_PS_RENDERER_SPRITE_POINTSPRITE, OnPointSpriteBtn)

    ON_BN_CLICKED(IDC_PS_RENDERER_SPRITE_ORIENTATION, OnExplicitOrientationBtn)

	ON_BN_CLICKED(IDC_PS_RENDERER_AMP_CIRCULAR, OnGenericBtn)

	ON_CBN_SELCHANGE(IDC_PS_RENDERER_SPRITE_MATERIALFX, OnGenericBtn)
	ON_CBN_SELCHANGE(IDC_PS_RENDERER_SPRITE_TEXTURENAME, OnGenericBtn)
	ON_BN_CLICKED(IDC_PS_RENDERER_SPRITE_TEXTURENAME_DIRECTORY_BTN, OnSpriteTexturenameDirectoryBtn)

	ON_BN_CLICKED(IDC_PS_RENDERER_MESH_VISUALNAME_DIRECTORY_BTN, OnMeshVisualnameDirectoryBtn)
    ON_CBN_SELCHANGE(IDC_PS_RENDERER_MESH_VISUALNAME, OnGenericBtn)
	ON_CBN_SELCHANGE(IDC_PS_RENDERER_MESH_MATERIALFX, OnGenericBtn)
    ON_CBN_SELCHANGE(IDC_PS_RENDERER_MESH_SORT, OnGenericBtn)

	ON_BN_CLICKED(IDC_PS_RENDERER_VISUAL_VISUALNAME_DIRECTORY_BTN, OnVisualVisualnameDirectoryBtn)
    ON_CBN_SELCHANGE(IDC_PS_RENDERER_VISUAL_VISUALNAME, OnGenericBtn)

	ON_BN_CLICKED(IDC_PS_RENDERER_AMP_TEXTURENAME_DIRECTORY_BTN, OnAmpTexturenameDirectoryBtn)
	ON_CBN_SELCHANGE(IDC_PS_RENDERER_AMP_TEXTURENAME, OnGenericBtn)

	ON_BN_CLICKED(IDC_PS_RENDERER_TRAIL_TEXTURENAME_DIRECTORY_BTN, OnTrailTexturenameDirectoryBtn)
	ON_CBN_SELCHANGE(IDC_PS_RENDERER_TRAIL_TEXTURENAME, OnGenericBtn)

	ON_BN_CLICKED(IDC_PS_RENDERER_BLUR_TEXTURENAME_DIRECTORY_BTN, OnBlurTexturenameDirectoryBtn)
	ON_CBN_SELCHANGE(IDC_PS_RENDERER_BLUR_TEXTURENAME, OnGenericBtn)
END_MESSAGE_MAP()

IMPLEMENT_DYNCREATE(PsRendererProperties, CFormView)

PsRendererProperties::PsRendererProperties()
: 
CFormView(PsRendererProperties::IDD),
initialised_(false),
filterChanges_(false),
positionGizmo_(NULL),
positionMatrixProxy_(NULL)
{
	BW_GUARD;

	frameCount_.SetAllowNegative( false );
	frameCount_.SetNumericType( controls::EditNumeric::ENT_INTEGER);

	frameRate_.SetAllowNegative( false );
	frameRate_.SetMinimum( 0, true );
	
	width_.SetAllowNegative( false );
	width_.SetMinimum( 0, false );

	height_.SetAllowNegative( false );
	height_.SetMinimum( 0, false );
	
	steps_.SetNumericType( controls::EditNumeric::ENT_INTEGER);
	steps_.SetAllowNegative( false );
	steps_.SetMinimum( 1, true );
	steps_.SetMaximum( 1000, true );

	variation_.SetAllowNegative( false );
	variation_.SetMinimum( 0.0f, true );

	trailWidth_.SetAllowNegative( false );
	trailWidth_.SetMinimum( 0, false );
		
	trailSteps_.SetAllowNegative( false );
	trailSteps_.SetNumericType( controls::EditNumeric::ENT_INTEGER);
	trailSteps_.SetMaximum( 1000, true );

	blurWidth_.SetAllowNegative( false );
	blurWidth_.SetMinimum( 0.0f, false );

	blurTime_.SetAllowNegative( false );
	blurTime_.SetMinimum( 0.0f, false );
}

PsRendererProperties::~PsRendererProperties()
{
	BW_GUARD;

	removePositionGizmo();
}

void PsRendererProperties::OnInitialUpdate()
{
	BW_GUARD;

    CFormView::OnInitialUpdate();

	for (size_t i = 0; i < spriteFXSz; ++i)
	{
        CString fxStr;
        fxStr.LoadString(spriteFX[i].first);
        spriteMaterialFX_.AddString(fxStr);
	}

	for (size_t i = 0; i < meshSortSz; ++i)
	{
        CString sortStr;
        sortStr.LoadString(meshSort[i].first);
        meshSort_.AddString(sortStr);
	}

	textureNameDirectoryEdit_.SetWindowText(c_defaultDirectoryText);

	meshNameDirectoryEdit_.SetWindowText(c_defaultDirectoryText);
    for 
    (
        size_t i = 0; 
        i < sizeof(meshMaterialFX)/sizeof(std::pair<UINT, UINT>); 
        ++i
    )
    {
        CString meshFX;
        meshFX.LoadString(meshMaterialFX[i].first);
        meshMaterialFX_.AddString(meshFX);
    }

    textureNameDirectoryBtn_     .setBitmapID(IDB_OPEN, IDB_OPEND);
    meshNameDirectoryBtn_        .setBitmapID(IDB_OPEN, IDB_OPEND);
    visualNameDirectoryBtn_      .setBitmapID(IDB_OPEN, IDB_OPEND);
    ampTextureNameDirectoryBtn_  .setBitmapID(IDB_OPEN, IDB_OPEND);
    trailTextureNameDirectoryBtn_.setBitmapID(IDB_OPEN, IDB_OPEND);
    blurTextureNameDirectoryBtn_ .setBitmapID(IDB_OPEN, IDB_OPEND);

	if (renderer()->nameID() == SpriteParticleRenderer::nameID_)
	{
		SpriteParticleRenderer * spriteRenderer = 
            static_cast< SpriteParticleRenderer * >( renderer().get() );

		Vector3 explicitOrientation = spriteRenderer->explicitOrientation();
		explicitOrientation_.SetCheck( explicitOrientation != Vector3( 0.f, 0.f, 0.f ) );
	}

	SetParameters(SET_CONTROL);

    AddBitmapDrop
    (
        &textureName_, 
        this, 
        &PsRendererProperties::DropSpriteTexture
    );
    AddBitmapDrop
    (
        &textureNameDirectoryEdit_, 
        this, 
        &PsRendererProperties::DropSpriteTexture
    );    
    AddMeshDrop
    (
        &meshName_,
        this,
        &PsRendererProperties::DropMesh,
        &PsRendererProperties::CanDropMesh
    );
    AddMeshDrop
    (
        &meshNameDirectoryEdit_,
        this,
        &PsRendererProperties::DropMesh,
        &PsRendererProperties::CanDropMesh
    );
    AddMeshDrop
    (
        &visualName_,
        this,
        &PsRendererProperties::DropVisual,
        NULL
    );
    AddMeshDrop
    (
        &visualNameDirectoryEdit_,
        this,
        &PsRendererProperties::DropVisual,
        NULL
    );
    AddBitmapDrop
    (
        &ampTextureName_, 
        this, 
        &PsRendererProperties::DropAmpTexture
    );
    AddBitmapDrop
    (
        &ampTextureNameDirectoryEdit_, 
        this, 
        &PsRendererProperties::DropAmpTexture
    );  
    AddBitmapDrop
    (
        &trailTextureName_, 
        this, 
        &PsRendererProperties::DropTrailTexture
    );
    AddBitmapDrop
    (
        &trailTextureNameDirectoryEdit_, 
        this, 
        &PsRendererProperties::DropTrailTexture
    ); 
    AddBitmapDrop
    (
        &blurTextureName_, 
        this, 
        &PsRendererProperties::DropBlurTexture
    );
    AddBitmapDrop
    (
        &blurTextureNameDirectoryEdit_, 
        this, 
        &PsRendererProperties::DropBlurTexture
    );

    INIT_AUTO_TOOLTIP();

	initialised_ = true;
}

void PsRendererProperties::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PS_RENDERER_WORLDDEPENDENT, worldDependent_);
	DDX_Control(pDX, IDC_PS_RENDERER_LOCALDEPENDENT, localDependent_);
	DDX_Control(pDX, IDC_PS_RENDERER_VIEWDEPENDENT, viewDependent_);
	DDX_Control(pDX, IDC_PS_RENDERER_SPRITE, rendererSprite_);
	DDX_Control(pDX, IDC_PS_RENDERER_MESH, rendererMesh_);
    DDX_Control(pDX, IDC_PS_RENDERER_VISUAL, rendererVisual_);
	DDX_Control(pDX, IDC_PS_RENDERER_AMP, rendererAmp_);
	DDX_Control(pDX, IDC_PS_RENDERER_TRAIL, rendererTrail_);
	DDX_Control(pDX, IDC_PS_RENDERER_BLUR, rendererBlur_);

	DDX_Control(pDX, IDC_PS_RENDERER_SPRITE_TEXTURENAME, textureName_);
	DDX_Control(pDX, IDC_PS_RENDERER_SPRITE_TEXTURENAME_DIRECTORY_BTN , textureNameDirectoryBtn_);
    DDX_Control(pDX, IDC_PS_RENDERER_SPRITE_TEXTURENAME_DIRECTORY_EDIT, textureNameDirectoryEdit_);
	DDX_Control(pDX, IDC_PS_RENDERER_SPRITE_MATERIALFX, spriteMaterialFX_);
	DDX_Control(pDX, IDC_PS_RENDERER_SPRITE_FRAMECOUNT, frameCount_);
	DDX_Control(pDX, IDC_PS_RENDERER_SPRITE_FRAMERATE, frameRate_);
    DDX_Control(pDX, IDC_PS_RENDERER_SPRITE_POINTSPRITE, pointSprite_);
	DDX_Control(pDX, IDC_PS_RENDERER_SPRITE_ORIENTATION, explicitOrientation_);
	DDX_Control(pDX, IDC_PS_RENDERER_SPRITE_ORIENTX_ED, explicitOrientX_);
	DDX_Control(pDX, IDC_PS_RENDERER_SPRITE_ORIENTY_ED, explicitOrientY_);
	DDX_Control(pDX, IDC_PS_RENDERER_SPRITE_ORIENTZ_ED, explicitOrientZ_);
	DDX_Control(pDX, IDC_PS_RENDERER_SPRITE_STATIC3, spriteStatic3_);
	DDX_Control(pDX, IDC_PS_RENDERER_SPRITE_STATIC4, spriteStatic4_);

	DDX_Control(pDX, IDC_PS_RENDERER_MESH_VISUALNAME, meshName_);
	DDX_Control(pDX, IDC_PS_RENDERER_MESH_VISUALNAME_DIRECTORY_BTN, meshNameDirectoryBtn_);
    DDX_Control(pDX, IDC_PS_RENDERER_MESH_VISUALNAME_DIRECTORY_EDIT, meshNameDirectoryEdit_);
    DDX_Control(pDX, IDC_PS_RENDERER_MESH_MATERIALFX, meshMaterialFX_);
    DDX_Control(pDX, IDC_PS_RENDERER_MESH_SORT, meshSort_);

	DDX_Control(pDX, IDC_PS_RENDERER_VISUAL_VISUALNAME, visualName_);
	DDX_Control(pDX, IDC_PS_RENDERER_VISUAL_VISUALNAME_DIRECTORY_BTN, visualNameDirectoryBtn_);
    DDX_Control(pDX, IDC_PS_RENDERER_VISUAL_VISUALNAME_DIRECTORY_EDIT, visualNameDirectoryEdit_);

	DDX_Control(pDX, IDC_PS_RENDERER_AMP_TEXTURENAME, ampTextureName_);
	DDX_Control(pDX, IDC_PS_RENDERER_AMP_TEXTURENAME_DIRECTORY_BTN, ampTextureNameDirectoryBtn_);
    DDX_Control(pDX, IDC_PS_RENDERER_AMP_TEXTURENAME_DIRECTORY_EDIT, ampTextureNameDirectoryEdit_);
	DDX_Control(pDX, IDC_PS_RENDERER_AMP_WIDTH, width_);
	DDX_Control(pDX, IDC_PS_RENDERER_AMP_HEIGHT, height_);
	DDX_Control(pDX, IDC_PS_RENDERER_AMP_STEPS, steps_);
	DDX_Control(pDX, IDC_PS_RENDERER_AMP_VARIATION, variation_);
	DDX_Control(pDX, IDC_PS_RENDERER_AMP_CIRCULAR, circular_);
	DDX_Control(pDX, IDC_PS_RENDERER_AMP_STATIC4, ampStatic4_);
	DDX_Control(pDX, IDC_PS_RENDERER_AMP_STATIC3, ampStatic3_);
	DDX_Control(pDX, IDC_PS_RENDERER_AMP_STATIC2, ampStatic2_);
	DDX_Control(pDX, IDC_PS_RENDERER_AMP_STATIC1, ampStatic1_);

	DDX_Control(pDX, IDC_PS_RENDERER_TRAIL_TEXTURENAME, trailTextureName_);
	DDX_Control(pDX, IDC_PS_RENDERER_TRAIL_TEXTURENAME_DIRECTORY_BTN, trailTextureNameDirectoryBtn_);
    DDX_Control(pDX, IDC_PS_RENDERER_TRAIL_TEXTURENAME_DIRECTORY_EDIT, trailTextureNameDirectoryEdit_);
	DDX_Control(pDX, IDC_PS_RENDERER_TRAIL_WIDTH, trailWidth_);
	DDX_Control(pDX, IDC_PS_RENDERER_TRAIL_STEPS, trailSteps_);
	DDX_Control(pDX, IDC_PS_RENDERER_TRAIL_STATIC1, trailStatic1_);
	DDX_Control(pDX, IDC_PS_RENDERER_TRAIL_STATIC2, trailStatic2_);

	DDX_Control(pDX, IDC_PS_RENDERER_BLUR_TIME, blurTime_);
	DDX_Control(pDX, IDC_PS_RENDERER_BLUR_WIDTH, blurWidth_);
	DDX_Control(pDX, IDC_PS_RENDERER_BLUR_STATIC3, blurStaticT_);
	DDX_Control(pDX, IDC_PS_RENDERER_BLUR_STATIC4, blurStaticW_);
	DDX_Control(pDX, IDC_PS_RENDERER_BLUR_TEXTURENAME, blurTextureName_);
	DDX_Control(pDX, IDC_PS_RENDERER_BLUR_TEXTURENAME_DIRECTORY_BTN, blurTextureNameDirectoryBtn_);
	DDX_Control(pDX, IDC_PS_RENDERER_BLUR_TEXTURENAME_DIRECTORY_EDIT, blurTextureNameDirectoryEdit_);

    DDX_Control(pDX, IDC_HLINE1, hline1_);
    DDX_Control(pDX, IDC_HLINE2, hline2_);
    DDX_Control(pDX, IDC_HLINE3, hline3_);
    DDX_Control(pDX, IDC_HLINE4, hline4_);
    DDX_Control(pDX, IDC_HLINE5, hline5_);
    DDX_Control(pDX, IDC_HLINE6, hline6_);
}

afx_msg LRESULT PsRendererProperties::OnUpdatePsRenderProperties(WPARAM mParam, LPARAM lParam)
{
	BW_GUARD;

	if (initialised_)
		SetParameters(SET_PSA);
	
	return 0;
}

void PsRendererProperties::SetParameters(SetOperation task)
{
	BW_GUARD;

	if (task == SET_CONTROL)
	{
		// read in
		if ( renderer()->local() )
		{
			worldDependent_.SetCheck(BST_UNCHECKED);
			localDependent_.SetCheck(BST_CHECKED);
			viewDependent_.SetCheck(BST_UNCHECKED);
		}
		else if ( renderer()->viewDependent() )
		{
			worldDependent_.SetCheck(BST_UNCHECKED);
			localDependent_.SetCheck(BST_UNCHECKED);
			viewDependent_.SetCheck(BST_CHECKED);
		}
		else
		{
			worldDependent_.SetCheck(BST_CHECKED);
			localDependent_.SetCheck(BST_UNCHECKED);
			viewDependent_.SetCheck(BST_UNCHECKED);
		}

		if 
        (
            renderer()->nameID() == SpriteParticleRenderer::nameID_
            ||
            renderer()->nameID() == PointSpriteParticleRenderer::nameID_
        )
		{
            SetSpriteEnabledState(true );
	        SetMeshEnabledState  (false);
            SetVisualEnabledState(false);
	        SetAmpEnabledState   (false);
	        SetTrailEnabledState (false);
	        SetBlurEnabledState  (false);

			SpriteParticleRenderer *spriteRenderer = 
                static_cast<SpriteParticleRenderer *>(&*renderer());
			
			SpriteParticleRenderer::MaterialFX fxSelected = spriteRenderer->materialFX();
            for (size_t i = 0; i < spriteFXSz; ++i)
            {
                if (spriteFX[i].second == fxSelected)
                    spriteMaterialFX_.SetCurSel((int)i);
            }

            CString longFilename = bw_utf8tow( spriteRenderer->textureName() ).c_str();

			// remember for next time
			Options::setOptionString( "defaults/renderer/spriteTexture", bw_wtoutf8( longFilename.GetString() ) );

			CString filename, directory; 
			GetFilenameAndDirectory(longFilename, filename, directory);

			// populate with all the textures in that directory
			string relativeDirectory = BWResource::dissolveFilename(bw_wtoutf8( directory.GetString() ));
			populateComboBoxWithFilenames(textureName_, relativeDirectory, validTextureFilename);
			textureNameDirectoryEdit_.SetWindowText(bw_utf8tow( relativeDirectory ).c_str());
			if (textureName_.SelectString(-1, filename) == -1)
			{
				//If the file with the extension specified didn't exist try the 
				// *.dds file since this will be the texture loaded by Moo anyway.
				CString temp = filename.Left( filename.Find(L".") );
				temp += L".dds";
				textureName_.SelectString(-1, temp);
			}

			frameCount_.SetIntegerValue(spriteRenderer->frameCount());
			frameRate_.SetValue(spriteRenderer->frameRate());

            bool isPointSprite = 
                (renderer()->nameID() == PointSpriteParticleRenderer::nameID_);
            pointSprite_.SetCheck(isPointSprite ? BST_CHECKED : BST_UNCHECKED);

			Vector3 explicitOrientation = spriteRenderer->explicitOrientation();
			explicitOrientX_.SetValue( explicitOrientation.x );
			explicitOrientY_.SetValue( explicitOrientation.y );
			explicitOrientZ_.SetValue( explicitOrientation.z );

			if (!isPointSprite && explicitOrientation != Vector3( 0.f, 0.f, 0.f ))
			{
				addPositionGizmo();
				Matrix mat;
				mat.setTranslate( position() );
				positionMatrixProxy_->setMatrixAlone( mat );
			}
			else
			{
				removePositionGizmo();
			}
		}
		else if (renderer()->nameID() == MeshParticleRenderer::nameID_)
		{
            OnMeshBtn();

            CString longFilename;
		    MeshParticleRenderer * meshRenderer = 
                static_cast<MeshParticleRenderer *>(&*renderer());
			longFilename = meshRenderer->visual().c_str();
            for (size_t i = 0; i < meshMaterialFXSz; ++i)
            {
                if (meshMaterialFX[i].second == meshRenderer->materialFX())
                {
                    meshMaterialFX_.SetCurSel(static_cast<int>(i));
                    break;
                }
            }

            for (size_t i = 0; i < meshSortSz; ++i)
            {
                if (meshSort[i].second == meshRenderer->sortType())
                {
                    meshSort_.SetCurSel(static_cast<int>(i));
                    break;
                }
            }

			// Remember for next time:
			Options::setOptionString( "defaults/renderer/meshVisual", bw_wtoutf8( longFilename.GetString() ) );

			CString filename, directory; 
			GetFilenameAndDirectory(longFilename, filename, directory);

			// Populate with all the textures in that directory:
			string relativeDirectory = BWResource::dissolveFilename(bw_wtoutf8( directory.GetString() ));
			populateComboBoxWithFilenames(meshName_, relativeDirectory, validMeshFilename);
			meshNameDirectoryEdit_.SetWindowText(bw_utf8tow( relativeDirectory ).c_str());
			meshName_.SelectString(-1, filename);
		}
		else if (renderer()->nameID() == VisualParticleRenderer::nameID_)
		{
            OnVisualBtn();

            CString longFilename;

			VisualParticleRenderer *visualRenderer = 
                static_cast<VisualParticleRenderer *>(&*renderer());
			longFilename = visualRenderer->visual().c_str();

			// Remember for next time:
			Options::setOptionString( "defaults/renderer/visualVisual", bw_wtoutf8( longFilename.GetString() ) );

			CString filename, directory; 
			GetFilenameAndDirectory(longFilename, filename, directory);

			// Populate with all the textures in that directory:
			string relativeDirectory = BWResource::dissolveFilename(bw_wtoutf8( directory.GetString() ));
			populateComboBoxWithFilenames(visualName_, relativeDirectory, validVisualFilename);
			visualNameDirectoryEdit_.SetWindowText(bw_utf8tow( relativeDirectory ).c_str());
			visualName_.SelectString(-1, filename);
		}
		else if(renderer()->nameID() == AmpParticleRenderer::nameID_)
		{
			OnAmpBtn();

			AmpParticleRenderer * ampRenderer = static_cast<AmpParticleRenderer *>(&*renderer());

			CString longFilename = bw_utf8tow( ampRenderer->textureName() ).c_str();

			// remember the selection
			Options::setOptionString( "defaults/renderer/ampTexture", bw_wtoutf8( longFilename.GetString() ) );

			CString filename, directory; 
			GetFilenameAndDirectory(longFilename, filename, directory);

			// populate with all the textures in that directory
			string relativeDirectory = BWResource::dissolveFilename( bw_wtoutf8( directory.GetString() ) );
			populateComboBoxWithFilenames(ampTextureName_, relativeDirectory, validTextureFilename);
			ampTextureNameDirectoryEdit_.SetWindowText(bw_utf8tow( relativeDirectory ).c_str());
			ampTextureName_.SelectString(-1, filename);

			width_.SetValue(ampRenderer->width());
			height_.SetValue(ampRenderer->height());
			steps_.SetIntegerValue(ampRenderer->steps());
			variation_.SetValue(ampRenderer->variation());
			circular_.SetCheck( ampRenderer->circular() ? BST_CHECKED : BST_UNCHECKED);
		}
		else if(renderer()->nameID() == TrailParticleRenderer::nameID_)
		{
			OnTrailBtn();

			TrailParticleRenderer * trailRenderer = static_cast<TrailParticleRenderer *>(&*renderer());

			CString longFilename = bw_utf8tow( trailRenderer->textureName() ).c_str();

			// remember the selection
			Options::setOptionString( "defaults/renderer/trailTexture", bw_wtoutf8( longFilename.GetString() ) );

			CString filename, directory; 
			GetFilenameAndDirectory(longFilename, filename, directory);

			// populate with all the textures in that directory
			string relativeDirectory = BWResource::dissolveFilename( bw_wtoutf8( directory.GetString() ) );
			populateComboBoxWithFilenames(trailTextureName_, relativeDirectory, validTextureFilename);
			trailTextureNameDirectoryEdit_.SetWindowText(bw_utf8tow( relativeDirectory ).c_str());
			trailTextureName_.SelectString(-1, filename);

			trailWidth_.SetValue(trailRenderer->width());
			trailSteps_.SetIntegerValue(trailRenderer->steps());
		}
		else if(renderer()->nameID() == BlurParticleRenderer::nameID_)
		{
			OnBlurBtn();

			BlurParticleRenderer * blurRenderer = static_cast<BlurParticleRenderer *>(&*renderer());

			CString longFilename = bw_utf8tow( blurRenderer->textureName() ).c_str();

			// remember the selection
			Options::setOptionString( "defaults/renderer/blurTexture", bw_wtoutf8( longFilename.GetString() ) );

			CString filename, directory; 
			GetFilenameAndDirectory(longFilename, filename, directory);

			// populate with all the textures in that directory
			string relativeDirectory = BWResource::dissolveFilename( bw_wtoutf8( directory.GetString() ) );
			populateComboBoxWithFilenames(blurTextureName_, relativeDirectory, validTextureFilename);
			blurTextureNameDirectoryEdit_.SetWindowText(bw_utf8tow( relativeDirectory ).c_str());
			blurTextureName_.SelectString(-1, filename);

			blurWidth_.SetValue(blurRenderer->width());
			blurTime_.SetValue(blurRenderer->time());
		}
		else
		{
			TRACE0("PsProperties::SetParameters - Unknown renderer!");
			MF_ASSERT(0);
		}
	}
	else // SET_PSA
	{
        if (!filterChanges_)
        {
		    MainFrame::instance()->PotentiallyDirty
            (
                true,
                UndoRedoOp::AK_PARAMETER,
                LocaliseUTF8(L"PARTICLEEDITOR/GUI/PS_RENDERER_PROPERTIES/SET_PARAM")
            );
        }

		// write out
		if 
        (
            renderer()->nameID() == SpriteParticleRenderer::nameID_
            ||
            renderer()->nameID() == PointSpriteParticleRenderer::nameID_
        )
		{
			SpriteParticleRenderer * spriteRenderer = static_cast<SpriteParticleRenderer *>(&*renderer());
			
			// note: assume only one materialFX selected at any one time
			// cast the int into a MaterialFX
			// AKR_TODO: make this into a teplate fn
			int selected1 = spriteMaterialFX_.GetCurSel();
            if (selected1 != -1)
            {
			    SpriteParticleRenderer::MaterialFX newFX =
                    (SpriteParticleRenderer::MaterialFX)spriteFX[selected1].second;
    			spriteRenderer->materialFX(newFX);
            }
			
			int selected = textureName_.GetCurSel();
			if (selected != -1)
			{
				CString texName, dirName;
				textureName_.GetLBText(selected, texName);
				textureNameDirectoryEdit_.GetWindowText(dirName);
				// make sure only one directory seperator
				dirName.TrimRight(L"/");
				dirName += L"/";
				spriteRenderer->textureName( bw_wtoutf8( (dirName + texName).GetString() ) );

				// remember for next time
				Options::setOptionString( "defaults/renderer/spriteTexture", bw_wtoutf8( (dirName + texName).GetString() ) );
			}

			spriteRenderer->frameCount(frameCount_.GetIntegerValue());
			spriteRenderer->frameRate(frameRate_.GetValue());

			Vector3 explicitOrientation(
				explicitOrientX_.GetValue(), explicitOrientY_.GetValue(), explicitOrientZ_.GetValue() );

			spriteRenderer->explicitOrientation( explicitOrientation );

			if (explicitOrientation != Vector3( 0.f, 0.f, 0.f ))
			{
				addPositionGizmo();
				Matrix mat;
				mat.setTranslate( position() );
				positionMatrixProxy_->setMatrixAlone( mat );
			}
			else
			{
				removePositionGizmo();
			}
		}
		else if (renderer()->nameID() == MeshParticleRenderer::nameID_)
		{
            std::string visualFile;
			int selected = meshName_.GetCurSel();
            MeshParticleRenderer *mpr =
                static_cast<MeshParticleRenderer *>(&*renderer());
			if (selected != -1)
			{
				CString visName, dirName;
				meshName_.GetLBText(selected, visName);
				meshNameDirectoryEdit_.GetWindowText(dirName);
				// make sure only one directory seperator
				dirName.TrimRight(L"/");
				dirName += L"/";
                visualFile = bw_wtoutf8( (dirName + visName).GetString() );
				{
					CWaitCursor wait;

					if (!MeshParticleRenderer::isSuitableVisual( visualFile ))
					{
						ERROR_MSG( LocaliseUTF8(L"PARTICLEEDITOR/GUI/PS_RENDERER_PROPERTIES/SELECT_VISUAL_FAIL", visualFile.c_str() ).c_str() );
						meshName_.DeleteString( selected );
						// restore previous mesh
						CString longVisName = bw_utf8tow( mpr->visual() ).c_str();
						GetFilenameAndDirectory( longVisName, visName, dirName );
						meshName_.SelectString( -1, visName );
						return;
					}
				}

				// remember for next time
				Options::setOptionString( "defaults/renderer/meshVisual", bw_wtoutf8( (dirName + visName).GetString() ) );
			}
            mpr->visual(visualFile);
            MeshParticleRenderer::MaterialFX fx =
                (MeshParticleRenderer::MaterialFX)
                meshMaterialFX[meshMaterialFX_.GetCurSel()].second;
            mpr->materialFX(fx);
            int selected1 = meshSort_.GetCurSel();
            if (selected1 != -1)
            {
                MeshParticleRenderer::SortType newSort = 
                    (MeshParticleRenderer::SortType)meshSort[selected1].second;
                mpr->sortType(newSort);
            }            
		}
		else if (renderer()->nameID() == VisualParticleRenderer::nameID_)
		{
            std::string visualFile;
			int selected = visualName_.GetCurSel();
			if (selected != -1)
			{
				CString visName, dirName;
				visualName_.GetLBText(selected, visName);
				visualNameDirectoryEdit_.GetWindowText(dirName);
				// make sure only one directory seperator
				dirName.TrimRight(L"/");
				dirName += L"/";
                visualFile = bw_wtoutf8( (dirName + visName).GetString() );		

				// remember for next time
				Options::setOptionString( "defaults/renderer/meshVisual", bw_wtoutf8( (dirName + visName).GetString() ) );
			}
            VisualParticleRenderer *vpr =
                static_cast<VisualParticleRenderer *>(&*renderer());
            vpr->visual(visualFile);
		}
		else if(renderer()->nameID() == AmpParticleRenderer::nameID_)
		{
			AmpParticleRenderer * ampRenderer = static_cast<AmpParticleRenderer *>(&*renderer());
			
			int selected = ampTextureName_.GetCurSel();
			if (selected != -1)
			{
				CString texName, dirName;
				ampTextureName_.GetLBText(selected, texName);
				ampTextureNameDirectoryEdit_.GetWindowText(dirName);
				// make sure only one directory seperator
				dirName.TrimRight(L"/");
				dirName += L"/";
				ampRenderer->textureName( bw_wtoutf8( (dirName + texName).GetString() ) );

				// remember the selection
				Options::setOptionString( "defaults/renderer/ampTexture", bw_wtoutf8( (dirName + texName).GetString() ) );
			}

			ampRenderer->width( width_.GetValue() );
			ampRenderer->height( height_.GetValue() );
			ampRenderer->steps( steps_.GetIntegerValue() );
			ampRenderer->variation( variation_.GetValue() );
			ampRenderer->circular(circular_.GetCheck() == BST_CHECKED);

		}
		else if(renderer()->nameID() == TrailParticleRenderer::nameID_)
		{
			TrailParticleRenderer * trailRenderer = static_cast<TrailParticleRenderer *>(&*renderer());
			
			int selected = trailTextureName_.GetCurSel();
			if (selected != -1)
			{
				CString texName, dirName;
				trailTextureName_.GetLBText(selected, texName);
				trailTextureNameDirectoryEdit_.GetWindowText(dirName);
				// make sure only one directory seperator
				dirName.TrimRight(L"/");
				dirName += L"/";
				trailRenderer->textureName( bw_wtoutf8( (dirName + texName).GetString() ) );

				// remember the selection
				Options::setOptionString( "defaults/renderer/trailTexture", bw_wtoutf8( (dirName + texName).GetString() ) );
			}

			trailRenderer->width(trailWidth_.GetValue());

			// zero is bad -> makes a zero ized cache in the render code
			if (trailSteps_.GetIntegerValue() == 0)
				trailSteps_.SetIntegerValue(1);
			trailRenderer->steps(trailSteps_.GetIntegerValue());
		}
		else if(renderer()->nameID() == BlurParticleRenderer::nameID_)
		{
			BlurParticleRenderer * blurRenderer = static_cast<BlurParticleRenderer *>(&*renderer());
			
			int selected = blurTextureName_.GetCurSel();
			if (selected != -1)
			{
				CString texName, dirName;
				blurTextureName_.GetLBText(selected, texName);
				blurTextureNameDirectoryEdit_.GetWindowText(dirName);
				// make sure only one directory seperator
				dirName.TrimRight(L"/");
				dirName += L"/";
				blurRenderer->textureName( bw_wtoutf8( (dirName + texName).GetString() ) );

				// remember the selection
				Options::setOptionString( "defaults/renderer/blurTexture", bw_wtoutf8( (dirName + texName).GetString() ) );
			}

			blurRenderer->width(blurWidth_.GetValue());
			blurRenderer->time(blurTime_.GetValue());
		}
		else
		{
			TRACE0("PsProperties::SetParameters - Unknown renderer!");
			MF_ASSERT(0);
		}
	}
}

void PsRendererProperties::SetSpriteEnabledState(bool option)
{
	BW_GUARD;

	rendererSprite_.SetCheck(option ? BST_CHECKED : BST_UNCHECKED);
	if (option)
		rendererSprite_.CheckRadioButton(IDC_PS_RENDERER_SPRITE, IDC_PS_RENDERER_TRAIL, IDC_PS_RENDERER_SPRITE);

    bool isPointSprite = (renderer()->nameID() == PointSpriteParticleRenderer::nameID_);
	bool isExplicitOrientation =
		renderer()->nameID() == SpriteParticleRenderer::nameID_ &&
		explicitOrientation_.GetCheck();

	if (option && isExplicitOrientation)
	{
		addPositionGizmo();
		Matrix mat;
		mat.setTranslate( position() );
		positionMatrixProxy_->setMatrixAlone( mat );
	}
	else
	{
		removePositionGizmo();
	}
	
    pointSprite_.EnableWindow(option);
	explicitOrientation_.EnableWindow(option && !isPointSprite);
	explicitOrientX_.EnableWindow(option && isExplicitOrientation);
    explicitOrientY_.EnableWindow(option && isExplicitOrientation);
    explicitOrientZ_.EnableWindow(option && isExplicitOrientation);
	textureName_.EnableWindow(option);
    textureNameDirectoryEdit_.EnableWindow(option);
	spriteMaterialFX_.EnableWindow(option);
	textureNameDirectoryBtn_.EnableWindow(option);
	frameCount_.EnableWindow(option && !isPointSprite);
	frameRate_.EnableWindow(option && !isPointSprite);
	spriteStatic3_.EnableWindow(option);
	spriteStatic4_.EnableWindow(option);
}

void PsRendererProperties::SetMeshEnabledState(bool option)
{
	BW_GUARD;

	rendererMesh_.SetCheck(option ? BST_CHECKED : BST_UNCHECKED);

	meshName_.EnableWindow(option);
    meshNameDirectoryEdit_.EnableWindow(option);
	meshNameDirectoryBtn_.EnableWindow(option);
    meshMaterialFX_.EnableWindow(option);
    meshSort_.EnableWindow(option);
}

void PsRendererProperties::SetVisualEnabledState(bool option)
{
	BW_GUARD;

	rendererVisual_.SetCheck(option ? BST_CHECKED : BST_UNCHECKED);

	visualName_.EnableWindow(option);
    visualNameDirectoryEdit_.EnableWindow(option);
	visualNameDirectoryBtn_.EnableWindow(option);    
}

void PsRendererProperties::SetAmpEnabledState(bool option)
{
	BW_GUARD;

	rendererAmp_.SetCheck(option ? BST_CHECKED : BST_UNCHECKED);

	ampTextureName_.EnableWindow(option);
    ampTextureNameDirectoryEdit_.EnableWindow(option);
	ampTextureNameDirectoryBtn_.EnableWindow(option);
	width_.EnableWindow(option);
	height_.EnableWindow(option);
	steps_.EnableWindow(option);
	variation_.EnableWindow(option);
	circular_.EnableWindow(option);
	ampStatic4_.EnableWindow(option);
	ampStatic3_.EnableWindow(option);
	ampStatic2_.EnableWindow(option);
	ampStatic1_.EnableWindow(option);
}

void PsRendererProperties::SetTrailEnabledState(bool option)
{
	BW_GUARD;

	rendererTrail_.SetCheck(option ? BST_CHECKED : BST_UNCHECKED);

	trailTextureName_.EnableWindow(option);
    trailTextureNameDirectoryEdit_.EnableWindow(option);
	trailTextureNameDirectoryBtn_.EnableWindow(option);
	trailWidth_.EnableWindow(option);
	trailSteps_.EnableWindow(option);
	trailStatic1_.EnableWindow(option);
	trailStatic2_.EnableWindow(option);
}

void PsRendererProperties::SetBlurEnabledState(bool option)
{
	BW_GUARD;

	rendererBlur_.SetCheck(option ? BST_CHECKED : BST_UNCHECKED);

	blurTextureName_.EnableWindow(option);
    blurTextureNameDirectoryEdit_.EnableWindow(option);
	blurTextureNameDirectoryBtn_.EnableWindow(option);
	blurWidth_.EnableWindow(option);
	blurTime_.EnableWindow(option);
	blurStaticT_.EnableWindow(option);
	blurStaticW_.EnableWindow(option);
}

ParticleSystemRendererPtr PsRendererProperties::renderer()
{
	BW_GUARD;

	ParticleSystemPtr pSystem = 
        MainFrame::instance()->GetCurrentParticleSystem();
    if (pSystem == NULL)
        return NULL;
	return pSystem->pRenderer();
}

void PsRendererProperties::resetParticles()
{
	BW_GUARD;

	ParticleSystemPtr pSystem =
        MainFrame::instance()->GetCurrentParticleSystem();
    pSystem->clear();
}

void PsRendererProperties::copyRendererSettings( ParticleSystemRendererPtr src, ParticleSystemRenderer * dst )
{
	BW_GUARD;

	dst->local( src->local() );
	dst->viewDependent( src->viewDependent() );
}

void PsRendererProperties::OnGenericBtn()
{
	BW_GUARD;

	SetParameters(SET_PSA);
}

void PsRendererProperties::OnWorldDependentBtn()
{
	BW_GUARD;

	MainFrame::instance()->PotentiallyDirty
    (
		true,
		UndoRedoOp::AK_PARAMETER, 
		LocaliseUTF8(L"PARTICLEEDITOR/GUI/PS_RENDERER_PROPERTIES/CHANGE_WORLD")
	);
	localDependent_.SetCheck(BST_UNCHECKED);
	renderer()->local( false );
	viewDependent_.SetCheck(BST_UNCHECKED);
	renderer()->viewDependent( false );
}

void PsRendererProperties::OnLocalDependentBtn()
{
	BW_GUARD;

	MainFrame::instance()->PotentiallyDirty
    (
		true,
		UndoRedoOp::AK_PARAMETER, 
		LocaliseUTF8(L"PARTICLEEDITOR/GUI/PS_RENDERER_PROPERTIES/CHANGE_LOCAL")
	);
	renderer()->local(true);
	worldDependent_.SetCheck(BST_UNCHECKED);
	viewDependent_.SetCheck(BST_UNCHECKED);
	renderer()->viewDependent( false );
}

void PsRendererProperties::OnViewDependentBtn()
{
	BW_GUARD;

	MainFrame::instance()->PotentiallyDirty
    (
		true,
		UndoRedoOp::AK_PARAMETER, 
		LocaliseUTF8(L"PARTICLEEDITOR/GUI/PS_RENDERER_PROPERTIES/CHANGE_VIEW")
	);
	renderer()->viewDependent(true);
	worldDependent_.SetCheck(BST_UNCHECKED);
	localDependent_.SetCheck(BST_UNCHECKED);
	renderer()->local( false );
}

void PsRendererProperties::OnSpriteBtn()
{
	BW_GUARD;

	SetSpriteEnabledState(true );
	SetMeshEnabledState  (false);
    SetVisualEnabledState(false);
	SetAmpEnabledState   (false);
	SetTrailEnabledState (false);
	SetBlurEnabledState  (false);

    resetParticles();

    bool isPointSprite = (pointSprite_.GetCheck() == BST_CHECKED);

	// maybe create a new renderer
	if 
    (
        (
            !isPointSprite 
            &&
            (renderer()->nameID() != SpriteParticleRenderer::nameID_)
        )
        ||
        (
            isPointSprite 
            &&
            (renderer()->nameID() != PointSpriteParticleRenderer::nameID_)
        )
    )
	{
		MainFrame::instance()->PotentiallyDirty
        (
            true,
            UndoRedoOp::AK_PARAMETER,
            LocaliseUTF8(L"PARTICLEEDITOR/GUI/PS_RENDERER_PROPERTIES/CHANGE_SPRITE")
        );

		// check to see haven't already specified a renderer
		string defaultTexture = Options::getOptionString( "defaults/renderer/spriteTexture", s_notFoundTexture );

		// Make sure the default texture exists, if not use the AutoConfig default	
		if (!BWResource::fileExists( defaultTexture ))
		{
			defaultTexture = s_notFoundTexture;
		}
			
		SpriteParticleRenderer *spriteRenderer = NULL;
        if (isPointSprite)
            spriteRenderer = new PointSpriteParticleRenderer( defaultTexture );
        else
            spriteRenderer = new SpriteParticleRenderer( defaultTexture );

        // Copy rotation information if the old renderer was a sprite based
        // renderer.
        if 
        (
            renderer()->nameID() == SpriteParticleRenderer::nameID_
            ||
            renderer()->nameID() == PointSpriteParticleRenderer::nameID_
        )
        {
            SpriteParticleRenderer *oldRenderer = 
                static_cast<SpriteParticleRenderer *>(&*renderer());
            spriteRenderer->rotated(oldRenderer->rotated());
        }

		// copy the local and viewdependent settings
		this->copyRendererSettings( this->renderer(), spriteRenderer );

		ParticleSystemPtr pSystem = MainFrame::instance()->GetCurrentParticleSystem();
		// the old renderer is auto removed when changed
		pSystem->pRenderer(spriteRenderer);

		int selected = textureName_.GetCurSel();
        filterChanges_ = true;
		if (selected != -1)
		{
			SetParameters(SET_PSA);
		}
		else
		{
			SetParameters(SET_CONTROL);
		}
        filterChanges_ = false;

		// tell the collidePSA (if exists)
		CollidePSA * col = 
            static_cast<CollidePSA*>(&*MainFrame::instance()->GetCurrentParticleSystem()->pAction(PSA_COLLIDE_TYPE_ID));
		if (col)
			col->spriteBased(true);
	}

	SetSpriteEnabledState(true );
	SetMeshEnabledState  (false);
    SetVisualEnabledState(false);
	SetAmpEnabledState   (false);
	SetTrailEnabledState (false);
	SetBlurEnabledState  (false);
}

void PsRendererProperties::OnMeshBtn()
{
	BW_GUARD;

	SetSpriteEnabledState(false);
	SetMeshEnabledState  (true );
    SetVisualEnabledState(false);
	SetAmpEnabledState   (false);
	SetTrailEnabledState (false);
	SetBlurEnabledState  (false);    

    // Create a new renderer?
    if (renderer()->nameID() != MeshParticleRenderer::nameID_)
    {
        resetParticles();

        if (!filterChanges_)
        {
		    MainFrame::instance()->PotentiallyDirty
            (
                true,
                UndoRedoOp::AK_PARAMETER, 
                LocaliseUTF8(L"PARTICLEEDITOR/GUI/PS_RENDERER_PROPERTIES/CHANGE_MESH")
            );
        }

        MeshParticleRenderer *mpr = new MeshParticleRenderer();
        if (meshMaterialFX_.GetCurSel() == -1)
            meshMaterialFX_.SetCurSel(0);
        MeshParticleRenderer::MaterialFX fx =
            (MeshParticleRenderer::MaterialFX)
            meshMaterialFX[meshMaterialFX_.GetCurSel()].second;
        mpr->materialFX(fx);  

		// copy the local and viewdependent settings
		this->copyRendererSettings( this->renderer(), mpr );

		ParticleSystemPtr pSystem = 
            MainFrame::instance()->GetCurrentParticleSystem();
		pSystem->pRenderer(mpr);

		int selected = meshName_.GetCurSel();
		if (selected != -1)
		{
            filterChanges_ = true;
			// use any parameters already set
			SetParameters(SET_PSA);
            filterChanges_ = false;
		}
		else
		{
			// get default parameters from the psa
			string notFoundMesh = 
                s_notFoundMeshModel.value().substr
                ( 
                    0, 
                    s_notFoundMeshModel.value().length() - 6 
                ) + ".visual";
            mpr->visual(notFoundMesh);
			SetParameters(SET_CONTROL);
		}

		// Tell the collidePSA (if exists)
		CollidePSA * col = 
            static_cast<CollidePSA*>(&*MainFrame::instance()->GetCurrentParticleSystem()->pAction(PSA_COLLIDE_TYPE_ID));
		if (col)
			col->spriteBased(false);
	}
}


void PsRendererProperties::OnVisualBtn()
{
	BW_GUARD;

	SetSpriteEnabledState(false);
	SetMeshEnabledState  (false);
    SetVisualEnabledState(true );
	SetAmpEnabledState   (false);
	SetTrailEnabledState (false);
	SetBlurEnabledState  (false);    

    // Create a new renderer?
    if (renderer()->nameID() != VisualParticleRenderer::nameID_)
    {
        resetParticles();

        if (!filterChanges_)
        {
		    MainFrame::instance()->PotentiallyDirty
            (
                true,
                UndoRedoOp::AK_PARAMETER, 
                LocaliseUTF8(L"PARTICLEEDITOR/GUI/PS_RENDERER_PROPERTIES/CHANGE_VISUAL")
            );
        }

        VisualParticleRenderer *vpr = new VisualParticleRenderer();     

		// copy the local and viewdependent settings
		this->copyRendererSettings( this->renderer(), vpr );

		ParticleSystemPtr pSystem = 
            MainFrame::instance()->GetCurrentParticleSystem();
		pSystem->pRenderer(vpr);

		int selected = visualName_.GetCurSel();
		if (selected != -1)
		{
            filterChanges_ = true;
			// use any parameters already set
			SetParameters(SET_PSA);
            filterChanges_ = false;
		}
		else
		{
			// get default parameters from the psa
			string notFoundVisual = 
                s_notFoundModel.value().substr
                ( 
                    0, 
                    s_notFoundModel.value().length() - 6 
                ) + ".visual";
            vpr->visual(notFoundVisual);
			SetParameters(SET_CONTROL);
		}

		// Tell the collidePSA (if exists)
		CollidePSA * col = 
            static_cast<CollidePSA*>(&*MainFrame::instance()->GetCurrentParticleSystem()->pAction(PSA_COLLIDE_TYPE_ID));
		if (col)
			col->spriteBased(false);
	}
}


void PsRendererProperties::OnAmpBtn()
{
	BW_GUARD;

	SetSpriteEnabledState(false);
	SetMeshEnabledState  (false);
    SetVisualEnabledState(false);
	SetAmpEnabledState   (true );
	SetTrailEnabledState (false);
	SetBlurEnabledState  (false);

    resetParticles();

	// maybe create a new renderer
	if (renderer()->nameID() != AmpParticleRenderer::nameID_)
	{
		MainFrame::instance()->PotentiallyDirty
        (
            true,
            UndoRedoOp::AK_PARAMETER, 
            LocaliseUTF8(L"PARTICLEEDITOR/GUI/PS_RENDERER_PROPERTIES/CHANGE_AMP")
        );

		// remove the old renderer is auto removed when changed
		AmpParticleRenderer * ampRenderer = new AmpParticleRenderer;

		// copy the local and viewdependent settings
		this->copyRendererSettings( this->renderer(), ampRenderer );

		ParticleSystemPtr pSystem = MainFrame::instance()->GetCurrentParticleSystem();
		pSystem->pRenderer(ampRenderer);

		int selected = ampTextureName_.GetCurSel();
		if (selected != -1)
		{
			// retrieve the current texture name
			SetParameters(SET_PSA);
		}
		else
		{
			string defaultTexture = Options::getOptionString( "defaults/renderer/ampTexture", s_notFoundTexture );

			// Make sure the default texture exists, if not use the AutoConfig default	
			if (!BWResource::fileExists( defaultTexture ))
			{
				defaultTexture = s_notFoundTexture;
			}
			
			ampRenderer->textureName( defaultTexture );
			SetParameters(SET_CONTROL);
		}

		// tell the collidePSA (if exists)
		CollidePSA * col = 
			static_cast<CollidePSA*>(&*MainFrame::instance()->GetCurrentParticleSystem()->pAction(PSA_COLLIDE_TYPE_ID));
		if (col)
			col->spriteBased(true);
	}
}

void PsRendererProperties::OnTrailBtn()
{
	BW_GUARD;

	SetSpriteEnabledState(false);
	SetMeshEnabledState  (false);
    SetVisualEnabledState(false);
	SetAmpEnabledState   (false);
	SetTrailEnabledState (true );
	SetBlurEnabledState  (false);

    resetParticles();

	// maybe create a new renderer
	if (renderer()->nameID() != TrailParticleRenderer::nameID_)
	{
		MainFrame::instance()->PotentiallyDirty
        (
            true,
            UndoRedoOp::AK_PARAMETER, 
            LocaliseUTF8(L"PARTICLEEDITOR/GUI/PS_RENDERER_PROPERTIES/CHANGE_TRAIL")
        );

		// remove the old renderer is auto removed when changed
		TrailParticleRenderer * trailRenderer = new TrailParticleRenderer;

		// copy the local and viewdependent settings
		this->copyRendererSettings( this->renderer(), trailRenderer );

		ParticleSystemPtr pSystem = MainFrame::instance()->GetCurrentParticleSystem();
		pSystem->pRenderer(trailRenderer);

		int selected = trailTextureName_.GetCurSel();
		if (selected != -1)
		{
			// use any parameters already set
			SetParameters(SET_PSA);
		}
		else
		{
			// get default parameters from the psa
			string defaultTexture = Options::getOptionString( "defaults/renderer/trailTexture", s_notFoundTexture );
			
			// Make sure the default texture exists, if not use the AutoConfig default	
			if (!BWResource::fileExists( defaultTexture ))
			{
				defaultTexture = s_notFoundTexture;
			}
			
			trailRenderer->textureName( defaultTexture );
			SetParameters(SET_CONTROL);
		}

		// tell the collidePSA (if exists)
		CollidePSA * col = 
			static_cast<CollidePSA*>(&*MainFrame::instance()->GetCurrentParticleSystem()->pAction(PSA_COLLIDE_TYPE_ID));
		if (col)
			col->spriteBased(true);
	}
}

void PsRendererProperties::OnBlurBtn()
{
	BW_GUARD;

	SetSpriteEnabledState(false);
	SetMeshEnabledState  (false);
    SetVisualEnabledState(false);
	SetAmpEnabledState   (false);
	SetTrailEnabledState (false);
	SetBlurEnabledState  (true );

    resetParticles();

	// maybe create a new renderer
	if (renderer()->nameID() != BlurParticleRenderer::nameID_)
	{
		MainFrame::instance()->PotentiallyDirty
        (
            true, 
            UndoRedoOp::AK_PARAMETER,
            LocaliseUTF8(L"PARTICLEEDITOR/GUI/PS_RENDERER_PROPERTIES/CHANGE_BLUR")
        );

		// Set the new renderer:
		BlurParticleRenderer * blurRenderer = new BlurParticleRenderer;

		// copy the local and viewdependent settings
		this->copyRendererSettings( this->renderer(), blurRenderer );

		ParticleSystemPtr pSystem = MainFrame::instance()->GetCurrentParticleSystem();
		pSystem->pRenderer(blurRenderer);

		int selected = blurTextureName_.GetCurSel();
		if (selected != -1)
		{
			// use any parameters already set
			SetParameters(SET_PSA);
		}
		else
		{
			// get default parameters from the psa
			string defaultTexture = Options::getOptionString( "defaults/renderer/blurTexture", s_notFoundTexture );

			// Make sure the default texture exists, if not use the AutoConfig default	
			if (!BWResource::fileExists( defaultTexture ))
			{
				defaultTexture = s_notFoundTexture;
			}

			blurRenderer->textureName( defaultTexture );
			SetParameters(SET_CONTROL);
		}

		// tell the collidePSA (if exists)
		CollidePSA * col = 
			static_cast<CollidePSA*>(&*MainFrame::instance()->GetCurrentParticleSystem()->pAction(PSA_COLLIDE_TYPE_ID));
		if (col)
			col->spriteBased(true);
	}
}

void PsRendererProperties::OnSpriteTexturenameDirectoryBtn()
{
	BW_GUARD;

	DirDialog dlg; 

	dlg.windowTitle_ = Localise(L"PARTICLEEDITOR/OPEN");
	dlg.promptText_ = Localise(L"PARTICLEEDITOR/CHOOSE_DIR");
	dlg.fakeRootDirectory_ = dlg.basePath();

	CString startDir;
	textureNameDirectoryEdit_.GetWindowText(startDir);
	if (startDir != c_defaultDirectoryText)
		dlg.startDirectory_ = BWResource::resolveFilenameW(startDir.GetString()).c_str();

	if (dlg.doBrowse( AfxGetApp()->m_pMainWnd )) 
	{
		dlg.userSelectedDirectory_ += "/";
		string relativeDirectory = BWResource::dissolveFilename(bw_wtoutf8( dlg.userSelectedDirectory_.GetString() ));
		textureNameDirectoryEdit_.SetWindowText(bw_utf8tow( relativeDirectory ).c_str());

		populateComboBoxWithFilenames(textureName_, relativeDirectory, validTextureFilename);
		textureName_.SetCurSel(0);
		SetParameters(SET_PSA);
	}
}

void PsRendererProperties::OnMeshVisualnameDirectoryBtn()
{
	BW_GUARD;

	DirDialog dlg; 

	dlg.windowTitle_ = Localise(L"PARTICLEEDITOR/OPEN");
	dlg.promptText_ = Localise(L"PARTICLEEDITOR/CHOOSE_DIR");
	dlg.fakeRootDirectory_ = dlg.basePath();

	CString startDir;
	meshNameDirectoryEdit_.GetWindowText(startDir);
	if (startDir != c_defaultDirectoryText)
		dlg.startDirectory_ = BWResource::resolveFilenameW(startDir.GetString()).c_str();

	if (dlg.doBrowse( AfxGetApp()->m_pMainWnd )) 
	{
		dlg.userSelectedDirectory_ += "/";
		string relativeDirectory = BWResource::dissolveFilename(bw_wtoutf8( dlg.userSelectedDirectory_.GetString() ));
		meshNameDirectoryEdit_.SetWindowText(bw_utf8tow( relativeDirectory ).c_str());

		populateComboBoxWithFilenames
        (
            meshName_, 
            relativeDirectory, 
            validMeshFilename
        );
		meshName_.SetCurSel(0);
		SetParameters(SET_PSA);
	}
}

void PsRendererProperties::OnVisualVisualnameDirectoryBtn()
{
	BW_GUARD;

	DirDialog dlg; 

	dlg.windowTitle_ = Localise(L"PARTICLEEDITOR/OPEN");
	dlg.promptText_ = Localise(L"PARTICLEEDITOR/CHOOSE_DIR");
	dlg.fakeRootDirectory_ = dlg.basePath();

	CString startDir;
	visualNameDirectoryEdit_.GetWindowText(startDir);
	if (startDir != c_defaultDirectoryText)
		dlg.startDirectory_ = BWResource::resolveFilenameW(startDir.GetString()).c_str();

	if (dlg.doBrowse( AfxGetApp()->m_pMainWnd )) 
	{
		dlg.userSelectedDirectory_ += "/";
		string relativeDirectory = BWResource::dissolveFilename(bw_wtoutf8( dlg.userSelectedDirectory_.GetString() ));
		visualNameDirectoryEdit_.SetWindowText(bw_utf8tow( relativeDirectory ).c_str());

		populateComboBoxWithFilenames
        (
            visualName_, 
            relativeDirectory, 
            validVisualFilename
        );
		visualName_.SetCurSel(0);
		SetParameters(SET_PSA);
	}
}

void PsRendererProperties::OnAmpTexturenameDirectoryBtn()
{
	BW_GUARD;

	DirDialog dlg; 

	dlg.windowTitle_ = Localise(L"PARTICLEEDITOR/OPEN");
	dlg.promptText_ = Localise(L"PARTICLEEDITOR/CHOOSE_DIR");
	dlg.fakeRootDirectory_ = dlg.basePath();

	CString startDir;
	ampTextureNameDirectoryEdit_.GetWindowText(startDir);
	if (startDir != c_defaultDirectoryText)
		dlg.startDirectory_ = BWResource::resolveFilenameW(startDir.GetString()).c_str();

	if (dlg.doBrowse( AfxGetApp()->m_pMainWnd )) 
	{
		dlg.userSelectedDirectory_ += "/";
		string relativeDirectory = BWResource::dissolveFilename(bw_wtoutf8( dlg.userSelectedDirectory_.GetString() ));
		ampTextureNameDirectoryEdit_.SetWindowText(bw_utf8tow( relativeDirectory ).c_str());

		populateComboBoxWithFilenames(ampTextureName_, relativeDirectory, validTextureFilename);
		ampTextureName_.SetCurSel(0);
		SetParameters(SET_PSA);
	}
}

void PsRendererProperties::OnTrailTexturenameDirectoryBtn()
{
	BW_GUARD;

	DirDialog dlg; 

	dlg.windowTitle_ = Localise(L"PARTICLEEDITOR/OPEN");
	dlg.promptText_ = Localise(L"PARTICLEEDITOR/CHOOSE_DIR");
	dlg.fakeRootDirectory_ = dlg.basePath();

	CString startDir;
	trailTextureNameDirectoryEdit_.GetWindowText(startDir);
	if (startDir != c_defaultDirectoryText)
		dlg.startDirectory_ = BWResource::resolveFilenameW(startDir.GetString()).c_str();

	if (dlg.doBrowse( AfxGetApp()->m_pMainWnd )) 
	{
		dlg.userSelectedDirectory_ += "/";
		string relativeDirectory = BWResource::dissolveFilename(bw_wtoutf8( dlg.userSelectedDirectory_.GetString() ));
		trailTextureNameDirectoryEdit_.SetWindowText(bw_utf8tow( relativeDirectory ).c_str());

		populateComboBoxWithFilenames(trailTextureName_, relativeDirectory, validTextureFilename);
		trailTextureName_.SetCurSel(0);
		SetParameters(SET_PSA);
	}
}

void PsRendererProperties::OnBlurTexturenameDirectoryBtn()
{
	BW_GUARD;

	DirDialog dlg; 

	dlg.windowTitle_ = Localise(L"PARTICLEEDITOR/OPEN");
	dlg.promptText_ = Localise(L"PARTICLEEDITOR/CHOOSE_DIR");
	dlg.fakeRootDirectory_ = dlg.basePath();

	CString startDir;
	blurTextureNameDirectoryEdit_.GetWindowText(startDir);
	if (startDir != c_defaultDirectoryText)
		dlg.startDirectory_ = BWResource::resolveFilenameW(startDir.GetString()).c_str();

	if (dlg.doBrowse( AfxGetApp()->m_pMainWnd )) 
	{
		dlg.userSelectedDirectory_ += "/";
		string relativeDirectory = BWResource::dissolveFilename(bw_wtoutf8( dlg.userSelectedDirectory_.GetString() ));
		blurTextureNameDirectoryEdit_.SetWindowText(bw_utf8tow( relativeDirectory ).c_str());

		populateComboBoxWithFilenames(blurTextureName_, relativeDirectory, validTextureFilename);
		blurTextureName_.SetCurSel(0);
		SetParameters(SET_PSA);
	}
}

void PsRendererProperties::OnPointSpriteBtn()
{
	BW_GUARD;

    OnSpriteBtn();
}

void PsRendererProperties::OnExplicitOrientationBtn()
{
	BW_GUARD;

	if (renderer()->nameID() == SpriteParticleRenderer::nameID_)
	{
		MainFrame::instance()->PotentiallyDirty
        (
            true,
            UndoRedoOp::AK_PARAMETER,
            LocaliseUTF8(L"PARTICLEEDITOR/GUI/PS_RENDERER_PROPERTIES/CHANGE_SPRITE")
        );

		SpriteParticleRenderer * spriteRenderer = 
			static_cast< SpriteParticleRenderer * >( renderer().get() );

		if (explicitOrientation_.GetCheck())
		{
			explicitOrientX_.SetValue( s_lastExplicitOrientation_.x );
			explicitOrientY_.SetValue( s_lastExplicitOrientation_.y );
			explicitOrientZ_.SetValue( s_lastExplicitOrientation_.z );
		}
		else
		{
			s_lastExplicitOrientation_.x = explicitOrientX_.GetValue();
			s_lastExplicitOrientation_.y = explicitOrientY_.GetValue();
			s_lastExplicitOrientation_.z = explicitOrientZ_.GetValue();
			s_lastExplicitOrientation_.normalise();
			explicitOrientX_.SetValue( 0.f );
			explicitOrientY_.SetValue( 0.f );
			explicitOrientZ_.SetValue( 0.f );
		}

		spriteRenderer->explicitOrientation( Vector3(
			explicitOrientX_.GetValue(),
			explicitOrientY_.GetValue(),
			explicitOrientZ_.GetValue() ) );
	}

    OnSpriteBtn();
}

bool PsRendererProperties::DropSpriteTexture(UalItemInfo *ii)
{
	BW_GUARD;

    string textureFile = bw_wtoutf8( ii->longText() );
    string dir         = BWResource::getFilePath(textureFile);
    string reldir      = BWResource::dissolveFilename(dir);
    string file        = BWResource::getFilename(textureFile);
    populateComboBoxWithFilenames(textureName_, reldir, validTextureFilename);
    textureName_.SelectString(-1, bw_utf8tow( file ).c_str());
    textureNameDirectoryEdit_.SetWindowText(bw_utf8tow( reldir ).c_str());
    SetParameters(SET_PSA);
	UalManager::instance().history().add( ii->assetInfo() );
    return true;
}

bool PsRendererProperties::DropMesh(UalItemInfo *ii)
{
	BW_GUARD;

    string meshFile = bw_wtoutf8( ii->longText() );
    string dir      = BWResource::getFilePath(meshFile);
    string reldir   = BWResource::dissolveFilename(dir);
    string file     = BWResource::getFilename(meshFile);
    if (validMeshFilename(file.c_str(), meshFile.c_str()))
    {
        populateComboBoxWithFilenames(meshName_, reldir, validMeshFilename);
        meshName_.SelectString(-1, bw_utf8tow( file ).c_str());
        meshNameDirectoryEdit_.SetWindowText(bw_utf8tow( reldir ).c_str());
        SetParameters(SET_PSA);
		UalManager::instance().history().add( ii->assetInfo() );
        return true;
    }
    else
    {
        return false;
    }
}

bool PsRendererProperties::DropVisual(UalItemInfo *ii)
{
	BW_GUARD;

    string visualFile = bw_wtoutf8( ii->longText() );
    string dir        = BWResource::getFilePath(visualFile);
    string reldir     = BWResource::dissolveFilename(dir);
    string file       = BWResource::getFilename(visualFile);
    if (validVisualFilename(file.c_str(), visualFile.c_str()))
    {
        populateComboBoxWithFilenames(visualName_, reldir, validVisualFilename);
        visualName_.SelectString(-1, bw_utf8tow( file ).c_str());
        visualNameDirectoryEdit_.SetWindowText(bw_utf8tow( reldir ).c_str());
        SetParameters(SET_PSA);
		UalManager::instance().history().add( ii->assetInfo() );
        return true;
    }
    else
    {
        return false;
    }
}

bool PsRendererProperties::DropAmpTexture(UalItemInfo *ii)
{
	BW_GUARD;

    string textureFile = bw_wtoutf8( ii->longText() );
    string dir         = BWResource::getFilePath(textureFile);
    string reldir      = BWResource::dissolveFilename(dir);
    string file        = BWResource::getFilename(textureFile);
    populateComboBoxWithFilenames(ampTextureName_, reldir, validTextureFilename);
    ampTextureName_.SelectString(-1, bw_utf8tow( file ).c_str());
    ampTextureNameDirectoryEdit_.SetWindowText(bw_utf8tow( reldir ).c_str());
    SetParameters(SET_PSA);
	UalManager::instance().history().add( ii->assetInfo() );
    return true;
}

bool PsRendererProperties::DropTrailTexture(UalItemInfo *ii)
{
	BW_GUARD;

    string textureFile = bw_wtoutf8( ii->longText() );
    string dir         = BWResource::getFilePath(textureFile);
    string reldir      = BWResource::dissolveFilename(dir);
    string file        = BWResource::getFilename(textureFile);
    populateComboBoxWithFilenames(trailTextureName_, reldir, validTextureFilename);
    trailTextureName_.SelectString(-1, bw_utf8tow( file ).c_str());
    trailTextureNameDirectoryEdit_.SetWindowText(bw_utf8tow( reldir ).c_str());
    SetParameters(SET_PSA);
	UalManager::instance().history().add( ii->assetInfo() );
    return true;
}

bool PsRendererProperties::DropBlurTexture(UalItemInfo *ii)
{
	BW_GUARD;

    string textureFile = bw_wtoutf8( ii->longText() );
    string dir         = BWResource::getFilePath(textureFile);
    string reldir      = BWResource::dissolveFilename(dir);
    string file        = BWResource::getFilename(textureFile);
    populateComboBoxWithFilenames(blurTextureName_, reldir, validTextureFilename);
    blurTextureName_.SelectString(-1, bw_utf8tow( file ).c_str());
    blurTextureNameDirectoryEdit_.SetWindowText(bw_utf8tow( reldir ).c_str());
    SetParameters(SET_PSA);
	UalManager::instance().history().add( ii->assetInfo() );
    return true;
}

CRect PsRendererProperties::CanDropMesh(UalItemInfo *ii)
{
	BW_GUARD;

    string meshFile = bw_wtoutf8( ii->longText() );
    string dir      = BWResource::getFilePath(meshFile);
    string reldir   = BWResource::dissolveFilename(dir);
    string file     = BWResource::getFilename(meshFile);
    if (validMeshFilename(file.c_str(), meshFile.c_str()))
    {        
        return CRect(-1, -1, -1, -1); // drop permitted, used default processing
    }
    else
    {
        return CRect(0, 0, 0, 0); // drop not permitted
    }
}


void PsRendererProperties::position( const Vector3 & position )
{
	BW_GUARD;

	Vector3 normalised( position );
	normalised.normalise();

	explicitOrientX_.SetValue( normalised.x );
	explicitOrientY_.SetValue( normalised.y );
	explicitOrientZ_.SetValue( normalised.z );

	filterChanges_ = true;
	SetParameters( SET_PSA );
	filterChanges_ = false;
}

Vector3 PsRendererProperties::position() const
{
	BW_GUARD;

	Vector3 normalised( explicitOrientX_.GetValue(),
					explicitOrientY_.GetValue(), explicitOrientZ_.GetValue() );
	normalised.normalise();
	return normalised;
}


void PsRendererProperties::addPositionGizmo()
{
	BW_GUARD;

	if (positionGizmo_)
	{
		return;	// already been created
	}

	GeneralEditorPtr editor = new GeneralEditor();
	positionMatrixProxy_ = new VectorGeneratorMatrixProxy< PsRendererProperties >(
		this, 
		&PsRendererProperties::position, 
		&PsRendererProperties::position );
	editor->addProperty( new GenPositionProperty( "vector", positionMatrixProxy_ ) );
	positionGizmo_ = new VectorGizmo( MODIFIER_ALT, positionMatrixProxy_, 0xFFFFFF00, 0.015f, NULL, 0.1f );
	GizmoManager::instance().addGizmo( positionGizmo_ );
	GeneralEditor::Editors newEditors;
	newEditors.push_back( editor );
	GeneralEditor::currentEditors( newEditors );
}

void PsRendererProperties::removePositionGizmo()
{
	BW_GUARD;

	if (!positionGizmo_)
	{
		return;	// already been deleted
	}

	GizmoManager::instance().removeGizmo( positionGizmo_ );
	positionGizmo_ = NULL;
}
