///////////////////////////////////////////////////////////////////////  
//  SpeedTreeRT.h
//
//	*** INTERACTIVE DATA VISUALIZATION (IDV) CONFIDENTIAL AND PROPRIETARY INFORMATION ***
//
//	This software is supplied under the terms of a license agreement or
//	nondisclosure agreement with Interactive Data Visualization, Inc. and
//  may not be copied, disclosed, or exploited except in accordance with 
//  the terms of that agreement.
//
//      Copyright (c) 2003-2008 IDV, Inc.
//      All rights reserved in all media.
//
//      IDV, Inc.
//      http://www.idvinc.com
//
//  *** Release version 4.2 ***

#pragma once
#if defined(PS3) || defined(unix) || defined(__APPLE__)
#include <errno.h>
#include <new>
#endif

// storage-class specification
#if (defined(WIN32) || defined(_XBOX)) && defined(SPEEDTREE_DLL_EXPORTS)
    #define ST_STORAGE_CLASS __declspec(dllexport)
#else
    #define ST_STORAGE_CLASS
#endif

// Macintosh export control
#ifdef __APPLE__
    #pragma export on
#endif

//  specify calling convention
#if defined(WIN32) || defined(_XBOX)
    #define CALL_CONV   __cdecl
#else
    #define CALL_CONV
#endif


//  forward references
class CIndexedGeometry;
class CTreeEngine;
class CLeafGeometry;
class CLightingEngine;
class CWindEngine;
class CTreeFileAccess;
class CSimpleBillboard;
class CFrondEngine;
struct STreeInstanceData;
struct SInstanceList;
struct SEmbeddedTexCoords;
struct SCollisionObjects;
class CProjectedShadow;
class CSpeedTreeAllocator;
class CMapBank;


///////////////////////////////////////////////////////////////////////  
//  class CSpeedTreeRT
    
class ST_STORAGE_CLASS CSpeedTreeRT
{
public:
        ///////////////////////////////////////////////////////////////////////  
        //  Enumerations

        enum EWindMethod
        {
            WIND_GPU, WIND_CPU, WIND_NONE
        };

        enum ELodMethod
        {
            LOD_POP, LOD_SMOOTH, LOD_NONE = 3
        };

        enum ELightingMethod
        {
            LIGHT_DYNAMIC, LIGHT_STATIC
        };

        enum EStaticLightingStyle
        {
            SLS_BASIC, SLS_USE_LIGHT_SOURCES, SLS_SIMULATE_SHADOWS
        };

        enum ECollisionObjectType
        {
            CO_SPHERE, CO_CAPSULE, CO_BOX
        };

        enum ETextureLayers
        {
            TL_DIFFUSE,        // diffuse layer
            TL_DETAIL,         // detail layer
            TL_NORMAL,         // normal-map layer
            TL_HEIGHT,         // height/displacement-map layer
            TL_SPECULAR,       // specular mask layer
            TL_USER1,          // user-defined #1
            TL_USER2,          // user-defined #2
            TL_SHADOW,         // shadow layer (used for texcoords only - the shadow map
                               // filename is stored separately in SMapBank in m_pSelfShadowMap)
            TL_NUM_TEX_LAYERS
        };

		enum EMemoryPreference
		{
			MP_FAVOR_SMALLER_FOOTPRINT,
			MP_FAVOR_LESS_FRAGMENTATION
		};


        ///////////////////////////////////////////////////////////////////////
        //  SGeometry bit vectors
        //
        //  Passed into GetGeometry() in order to mask out unneeded geometric elements

        #define                 SpeedTree_BranchGeometry            (1 << 0)
        #define                 SpeedTree_FrondGeometry             (1 << 1)
        #define                 SpeedTree_LeafGeometry              (1 << 2)
        #define                 SpeedTree_BillboardGeometry         (1 << 3)
        #define                 SpeedTree_AllGeometry               (SpeedTree_BranchGeometry + SpeedTree_FrondGeometry + SpeedTree_LeafGeometry + SpeedTree_BillboardGeometry)


        ///////////////////////////////////////////////////////////////////////
        //  struct SGeometry declaration
        
        struct ST_STORAGE_CLASS SGeometry
        {

            ///////////////////////////////////////////////////////////////////////
            //  struct SGeometry::SIndexed declaration
            //
            //  Used to store indexed geometry - branches and fronds in SpeedTree.

            struct ST_STORAGE_CLASS SIndexed
            {
										SIndexed( );
										~SIndexed( );

                // indexed triangle strip data
                int                     m_nNumLods;             // total number of discrete LODs strip data represents
                const int*              m_pNumStrips;           // m_pNumStrips[L] == total number of strips in LOD L
                const int**             m_pStripLengths;        // m_pStripLengths[L][S] == lengths of strip S in LOD L
                const int***            m_pStrips;              // m_pStrips[L][S][I] == triangle strip index in LOD L, strip S, and index I

                // these vertex attributes are shared across all discrete LOD levels
                int                     m_nNumVertices;         // total vertex count in tables, referenced by all LOD levels
                const unsigned int*     m_pColors;              // RGBA values for each vertex - static lighting only (m_nNumVertices in length)
                const float*            m_pNormals;             // normals for each vertex (3 * m_nNumVertices in length)    
                const float*            m_pBinormals;           // binormals for each vertex (3 * m_nNumVertices in length)    
                const float*            m_pTangents;            // tangents for each vertex (3 * m_nNumVertices in length)        
                const float*            m_pCoords;              // coordinates for each vertex (3 * m_nNumVertices in length)
                const float*            m_pWindWeights[2];      // primary & secondary values from from 0.0 for rigid to 1.0 for flexible (m_nNumVertices in length)
                const unsigned char*    m_pWindMatrixIndices[2];// primary & secondary tables of wind matrix indices (m_nNumVertices in length)
				const float*			m_pLodFadeHints;		// hint used to fade between 3D LOD levels (shader use only, m_nNumVertices in length)
				float					m_fLodFadeDistance;		// distance around LOD transition points in which fading occurs (0.0 = no fade, 1.0 = entire range) 

                const float*            m_pTexCoords[TL_NUM_TEX_LAYERS];
            };


            ///////////////////////////////////////////////////////////////////////
            //  struct SGeometry::SLeaf declaration

            struct ST_STORAGE_CLASS SLeaf
            {
										SLeaf( );
										~SLeaf( );

                // active LOD level data
                int                     m_nDiscreteLodLevel;    // range: [0, GetNumLeafLodLevels( ) - 1]
                int                     m_nNumLeaves;           // number of leaves stored in this structure
                float                   m_fLeafRockScalar;      // passed to shader to control rocking motion
                float                   m_fLeafRustleScalar;    // passed to shader to control rustling motion

                // leaf attributes
                const unsigned char*    m_pLeafMapIndices;      // not for game/application engine use (SpeedTree toolset only)
                const unsigned char*    m_pLeafCardIndices;     // used to index m_pCards array in SLeaf (m_nNumLeaves in length)
                const float*            m_pCenterCoords;        // (x,y,z) values of the centers of leaf clusters (3 * m_nNumLeaves in length)
                const unsigned int*     m_pColors;              // RGBA values for each corner of each leaf (4 * m_nNumLeaves in length)
                const float*            m_pDimming;             // dimming factor for each leaf to use with dynamic lighting (m_nNumLeaves in length)
                const float*            m_pNormals;             // normals for each corner of each leaf (4 * 3 * m_nNumLeaves in length)
                const float*            m_pBinormals;           // binormals for each corner of each leaf (4 * 3 * m_nNumLeaves in length)
                const float*            m_pTangents;            // tangents for each corner of each leaf (4 * 3 * m_nNumLeaves in length)
                const float*            m_pWindWeights[2];      // primary & seconardy wind weights [0.0, 1.0] (m_nNumLeaves in length)
                const unsigned char*    m_pWindMatrixIndices[2];// primary & secondary wind matrix indices (m_nNumLeaves in length)

                // leaf meshes
                struct ST_STORAGE_CLASS SMesh
                {
										SMesh( );
										~SMesh( );

                    // mesh vertex attributes
                    int                 m_nNumVertices;         // number of vertices in mesh
                    const float*        m_pCoords;              // (x,y,z) coords for each vertex (3 * m_nNumVertices in length)
                    const float*        m_pTexCoords;           // texcoords for each vertex (2 * m_nNumVertices in length)
                    const float*        m_pNormals;             // normals for each vertex (3 * m_nNumVertices in length)    
                    const float*        m_pBinormals;           // binormals (bump mapping) for each vertex (3 * m_nNumVertices in length)    
                    const float*        m_pTangents;            // tangents (bump mapping) for each vertex (3 * m_nNumVertices in length)
                                    
                    // mesh indexes
                    int                 m_nNumIndices;          // length of index array
                    const int*          m_pIndices;             // triangle indices (m_nNumIndices in length)
                };

                // leaf cards
                struct ST_STORAGE_CLASS SCard
                {
										SCard( );
										~SCard( );

                    // cluster attributes
                    float               m_fWidth;               // width of the leaf cluster
                    float               m_fHeight;              // height of the leaf cluster
                    float               m_afPivotPoint[2];      // center point about which rocking/rustling occurs
                    float               m_afAngleOffsets[2];    // angular offset to help combat z-fighting among the leaves
                    const float*        m_pTexCoords;           // 4 pairs of (s,t) texcoords (8 floats total)
                    const float*        m_pCoords;              // 4 sets of (x,y,z,0) coords (16 floats total)

                    // cluster mesh
                    const SMesh*        m_pMesh;                // mesh leaf data (no mesh if NULL, use leaf cards)
					float				m_fMeshScale;			// each LOD may be successively scaled
                };
                
                SCard*                  m_pCards;               // to be accessed as m_pCards[nLodLevel][m_pLeafCardIndices[nLeaf]] where nLeaf
                                                                // is the current leaf being drawn or accessed
            };


            ///////////////////////////////////////////////////////////////////////
            //  struct SGeometry::S360Billboard declaration

            struct ST_STORAGE_CLASS S360Billboard
            {
										S360Billboard( );

                // GPU-only
                float                   m_fWidth;               // width of billboard (based on 3D model size)
                float                   m_fHeight;              // height of billboard (based on 3D model size)
                int                     m_nNumImages;           // # of 360-degree images stored in CAD export
                const float*            m_pTexCoordTable;       // all of the texture coordinates for all 360-degree images
                
                // both GPU and CPU
                const float*            m_pCoords;              // pointer to billboarded coords (4 * 3 floats in all)
                const float*            m_pTexCoords[2];        // two sets of texcoords (one for each overlapping bb),
                                                                // eight texcoords in m_pTexCoords[0] and m_pTexCoords[1]
                const float*            m_pNormals;             // normal for each vertex (4 * 3 floats in all)
                const float*            m_pBinormals;           // binormal for each vertex (4 * 3 floats in all)
                const float*            m_pTangents;            // tangent for each vertex (4 * 3 floats in all)
                float                   m_afAlphaTestValues[2]; // alpha test values for each overlapping bb
				float					m_fTransitionPercent;	// used to control the amount of overlap and fading between
																// adjacent 360 bb images
            };


            ///////////////////////////////////////////////////////////////////////
            //  struct SGeometry::SHorzBillboard declaration

            struct ST_STORAGE_CLASS SHorzBillboard
            {
										SHorzBillboard( );

                const float*            m_pCoords;				// pointer to billboarded coords (4 * 3 floats)
				float					m_afTexCoords[8];		// texcoords for all corners (s,t),(s,t), ...
                float                   m_afNormals[4][3];		// normals for all four corners
                float                   m_afBinormals[4][3];	// binormals for all four corners
                float                   m_afTangents[4][3];		// tangents for all for corners
                float                   m_fAlphaTestValue;		// alpha test value used for fade-in
            };


									SGeometry( );
									~SGeometry( );


            ///////////////////////////////////////////////////////////////////////
            //  branch geometry

            SIndexed                m_sBranches;                // holds the branch vertices and index buffers for all
                                                                // of the discrete LOD levels

            ///////////////////////////////////////////////////////////////////////
            //  frond geometry

            SIndexed                m_sFronds;                  // holds the frond vertices and index buffers for all
                                                                // of the discrete LOD levels

            ///////////////////////////////////////////////////////////////////////
            //  leaf geometry

            int                     m_nNumLeafLods;             // m_pLeaves contains m_nNumLeafLods elements
            SLeaf*                  m_pLeaves;                  // contains all of the LOD information in one dynamic array


            ///////////////////////////////////////////////////////////////////////
            //  billboard geometry

            S360Billboard           m_s360Billboard;			// 3D billboard rendering data (two quads are active together)
            SHorzBillboard          m_sHorzBillboard;			// horizontal billboard rendering data (just one quad, all in scene fade in/out together)
        };


        ///////////////////////////////////////////////////////////////////////
        //  struct SLodValues declaration

        struct ST_STORAGE_CLASS SLodValues
        {
									SLodValues( );

            // branches
            int                     m_nBranchActiveLod;         // 0 is highest LOD, -1 is inactive
            float                   m_fBranchAlphaTestValue;    // 0.0 to 255.0 alpha testing value, used for fading

            // fronds
            int                     m_nFrondActiveLod;          // 0 is highest LOD, -1 is inactive
            float                   m_fFrondAlphaTestValue;     // 0.0 to 255.0 alpha testing value, used for fading

            // leaves
            int                     m_anLeafActiveLods[2];      // 0 is highest LOD, -1 is inactive
            float                   m_afLeafAlphaTestValues[2]; // 0.0 to 255.0 alpha testing value, used for fading

            // billboard
            float                   m_fBillboardFadeOut;        // 0.0 = faded out, 1.0 = opaque; meant to interp between
																// alpha test values 255 and ~84
        };


        ///////////////////////////////////////////////////////////////////////  
        //  struct SMapBank declaration

        struct ST_STORAGE_CLASS SMapBank
        {
									SMapBank( );
									~SMapBank( );

            // branches
            const char**            m_pBranchMaps;              // filename = m_pBranchMaps[ETextureLayers]

            // not for game/application engine use (SpeedTree toolset only)
            unsigned int            m_uiNumLeafMaps;
            const char***           m_pLeafMaps;
            unsigned int            m_uiNumFrondMaps;
            const char***           m_pFrondMaps;

            // composite & billboards
            const char**            m_pCompositeMaps;           // filename = m_pCompositeMaps[ETextureLayers]
            const char**            m_pBillboardMaps;           // filename = m_pBillboardMaps[ETextureLayers]

            // self-shadow
            const char*             m_pSelfShadowMap;           // only one map per tree, or NULL if none
        };


        ///////////////////////////////////////////////////////////////////////  
        //  struct SLightShaderParams declaration
		//
		//	These parameters are controlled in CAD and are used as pixel shader hints to help
		//  combat some of the darkness that commonly comes into play with multiple texture layers.

		struct ST_STORAGE_CLASS SLightShaderParams
		{
						SLightShaderParams( );

			float       m_fGlobalLightScalar;					// scale all 3D geometry by this value
			float       m_fBranchLightScalar;					// scale branch geometry by this value
			float       m_fFrondLightScalar;					// scale frond geometry by this value
			float       m_fLeafLightScalar;						// scale leaf mesh/card geometry by this value
			float       m_fAmbientScalar;						// adjust all ambients by this value
			float       m_fBillboardDarkSideLightScalar;		// billboard light scalar used on the dark side of the billboards
			float       m_fBillboardBrightSideLightScalar;		// billboard light scalar used on the bright side of the billboards
			float       m_fBillboardAmbientScalar;				// scales the ambient value of the billboard material
		};

        ///////////////////////////////////////////////////////////////////////  
        //  Constructor/Destructor

								CSpeedTreeRT( );
		virtual					~CSpeedTreeRT( );


        ///////////////////////////////////////////////////////////////////////  
        //  Memory management

static  void          CALL_CONV SetAllocator(CSpeedTreeAllocator* pAllocator);
static	size_t		  CALL_CONV	GetHeapUsage(void); // in bytes
        void*                   operator new(size_t sBlockSize);
        void*                   operator new[](size_t sBlockSize);
        void                    operator delete(void* pBlock);
        void                    operator delete[](void* pBlock);

static	void		  CALL_CONV SetMemoryPreference(EMemoryPreference eMemPref);
static	EMemoryPreference CALL_CONV GetMemoryPreference(void);


        ///////////////////////////////////////////////////////////////////////  
        //  Specifying a tree model

        bool                    Compute(const float* pTransform = 0, unsigned int nSeed = 1, bool bCompositeStrips = true);
        CSpeedTreeRT*           Clone(void) const;
        const CSpeedTreeRT*     InstanceOf(void) const;
        CSpeedTreeRT*           MakeInstance(void);
        void                    DeleteTransientData(void);

        bool                    LoadTree(const char* pFilename);
        bool                    LoadTree(const unsigned char* pBlock, unsigned int nNumBytes);
        unsigned char*          SaveTree(unsigned int& nNumBytes, bool bSaveLeaves = false) const;

        void                    GetTreeSize(float& fSize, float& fVariance) const;
        void                    SetTreeSize(float fNewSize, float fNewVariance = 0.0f);
        unsigned int            GetSeed(void) const;

        const float*            GetTreePosition(void) const;
        void                    SetTreePosition(float x, float y, float z);

        void                    SetLeafTargetAlphaMask(unsigned char ucMask = 0x54);


        ///////////////////////////////////////////////////////////////////////  
        //  Lighting

        //  lighting style
        ELightingMethod         GetBranchLightingMethod(void) const;
        void                    SetBranchLightingMethod(ELightingMethod eMethod);
        ELightingMethod         GetLeafLightingMethod(void) const;
        void                    SetLeafLightingMethod(ELightingMethod eMethod);
        ELightingMethod         GetFrondLightingMethod(void) const;
        void                    SetFrondLightingMethod(ELightingMethod eMethod);

        EStaticLightingStyle    GetStaticLightingStyle(void) const;
        void                    SetStaticLightingStyle(EStaticLightingStyle eStyle);
        float                   GetLeafLightingAdjustment(void) const;
        void                    SetLeafLightingAdjustment(float fScalar);

        //  global lighting state
static  bool          CALL_CONV GetLightState(unsigned int nLightIndex);
static  void          CALL_CONV SetLightState(unsigned int nLightIndex, bool bLightOn);
static  const float*  CALL_CONV GetLightAttributes(unsigned int nLightIndex);
static  void          CALL_CONV SetLightAttributes(unsigned int nLightIndex, const float afLightAttributes[16]);

        //  materials
        const float*            GetBranchMaterial(void) const;
        void                    SetBranchMaterial(const float afMaterial[13]);
		const float*            GetFrondMaterial(void) const;
		void                    SetFrondMaterial(const float afMaterial[13]);
        const float*            GetLeafMaterial(void) const;
        void                    SetLeafMaterial(const float afMaterial[13]);

		//	shader lighting support
		void                    GetLightShaderParams(SLightShaderParams& sParams) const;


        ///////////////////////////////////////////////////////////////////////  
        //  Camera

static  void          CALL_CONV GetCamera(float afPosition[3], float afDirection[3]);
static  void          CALL_CONV SetCamera(const float afPosition[3], const float afDirection[3]);
static  void          CALL_CONV GetCameraAngles(float& fAzimuth, float& fPitch); // values are in degrees


        ///////////////////////////////////////////////////////////////////////  
        //  Wind 

static  void          CALL_CONV SetTime(float fTime);
        void                    ComputeWindEffects(bool bBranches, bool bLeaves, bool bFronds = true);
        void                    ResetLeafWindState(void);

        bool                    GetLeafRockingState(void) const;
        void                    SetLeafRockingState(bool bFlag);
        void                    SetNumLeafRockingGroups(unsigned int nRockingGroups);
        
        EWindMethod             GetLeafWindMethod(void) const;
        void                    SetLeafWindMethod(EWindMethod eMethod);
        EWindMethod             GetBranchWindMethod(void) const;
        void                    SetBranchWindMethod(EWindMethod eMethod);
        EWindMethod             GetFrondWindMethod(void) const;
        void                    SetFrondWindMethod(EWindMethod eMethod);

        float                   GetWindStrength(void) const;
        float                   SetWindStrength(float fNewStrength, float fOldStrength = -1.0f, float fFrequencyTimeOffset = -1.0f);
        void                    SetWindStrengthAndLeafAngles(float fNewStrength, const float* pRockAngles = 0, const float* pRustleAngles = 0, unsigned int uiNumRockAngles = 0);

static  void          CALL_CONV SetNumWindMatrices(int nNumMatrices);
static  void          CALL_CONV SetWindMatrix(unsigned int nMatrixIndex, const float afMatrix[16]);
        void                    GetLocalMatrices(unsigned int& nStartingIndex, unsigned int& nMatrixSpan);
        void                    SetLocalMatrices(unsigned int nStartingMatrix, unsigned int nMatrixSpan);

        
        ///////////////////////////////////////////////////////////////////////  
        //  LOD

        void                    ComputeLodLevel(void);
        float                   GetLodLevel(void) const;
        void                    SetLodLevel(float fLodLevel);
static  void          CALL_CONV SetDropToBillboard(bool bFlag);
		void                    GetLodValues(SLodValues& sLodValues, float fLodLevel = -1.0f);

        void                    GetLodLimits(float& fNear, float& fFar) const;
        void                    SetLodLimits(float fNear, float fFar);

        int                     GetDiscreteBranchLodLevel(float fLodLevel = -1.0f) const;
        int                     GetDiscreteLeafLodLevel(float fLodLevel = -1.0f) const;
        int                     GetDiscreteFrondLodLevel(float fLodLevel = -1.0f) const;

        int                     GetNumBranchLodLevels(void) const;
        int                     GetNumLeafLodLevels(void) const;
        int                     GetNumFrondLodLevels(void) const;

static  void          CALL_CONV SetHorzBillboardFadeAngles(float fStart, float fEnd); // in degrees
static  void          CALL_CONV GetHorzBillboardFadeAngles(float& fStart, float& fEnd); // in degrees
static	float		  CALL_CONV GetHorzBillboardFadeValue(void); // 0.0 = transparent, 1.0 = opaque


        ///////////////////////////////////////////////////////////////////////  
        //  Geometry

        void                    DeleteBranchGeometry(void);
        void                    DeleteFrondGeometry(void);
        void                    DeleteLeafGeometry(void);
        unsigned char*          GetFrondGeometryMapIndexes(int nLodLevel) const;
        const float*            GetLeafBillboardTable(unsigned int& nNumEntries) const;
        void                    GetGeometry(SGeometry& sGeometry, unsigned int uiBitVector = SpeedTree_AllGeometry);
        void                    UpdateLeafCards(SGeometry& sGeometry);
        void                    UpdateBillboardGeometry(SGeometry& sGeometry);
static  void                    UpdateBillboardLighting(SGeometry::S360Billboard& sBillboard);


        ///////////////////////////////////////////////////////////////////////  
        //  Textures

        void                    GetMapBank(SMapBank& sMapBank) const;
static  const char*             GetTextureLayerName(ETextureLayers eLayer);
        void                    SetLeafTextureCoords(unsigned int nLeafMapIndex, const float afTexCoords[8]);
        void                    SetFrondTextureCoords(unsigned int nFrondMapIndex, const float afTexCoords[8]);
static  bool          CALL_CONV GetTextureFlip(void);
static  void          CALL_CONV SetTextureFlip(bool bFlag);
        void                    SetBranchTextureFilename(const char* pFilename);
        void                    SetLeafTextureFilename(unsigned int nLeafMapIndex, const char* pFilename);
        void                    SetFrondTextureFilename(unsigned int nFrondMapIndex, const char* pFilename);


        ///////////////////////////////////////////////////////////////////////  
        //  Statistics & information

static  bool          CALL_CONV Authorize(const char* pKey);
static  bool          CALL_CONV IsAuthorized(void);
static  const char*   CALL_CONV GetCurrentError(void);
static  void          CALL_CONV ResetError(void);
static  const char*   CALL_CONV Version(void);

        void                    GetBoundingBox(float afBounds[6]) const;
        int                     GetNumLeafTriangles(float fLodLevel = -1.0f);
        int                     GetNumBranchTriangles(float fLodLevel = -1.0f);
        int                     GetNumFrondTriangles(float fLodLevel = -1.0f);


        ///////////////////////////////////////////////////////////////////////  
        //  Collision objects

        int                     GetNumCollisionObjects(void);
        void                    GetCollisionObject(unsigned int nIndex, ECollisionObjectType& eType, float afPosition[3], float afDimensions[3], float afEulerAngles[3]);
		void					ScaleCollisionObjects(float fScale);

        
        ///////////////////////////////////////////////////////////////////////  
        //  User Data

        const char*             GetUserData(void) const;

private:
        CSpeedTreeRT(const CSpeedTreeRT* pOrig);

        void                    ComputeLeafStaticLighting(void);
        void                    ComputeSelfShadowTexCoords(void);
static  void          CALL_CONV NotifyAllTreesOfEvent(int nMessage);
static  void          CALL_CONV SetError(const char* pError);


        ///////////////////////////////////////////////////////////////////////  
        //  File I/O

        void                    ParseLodInfo(CTreeFileAccess* pFile);
        void                    ParseWindInfo(CTreeFileAccess* pFile);
        void                    ParseTextureCoordInfo(CTreeFileAccess* pFile);
        void                    ParseCollisionObjects(CTreeFileAccess* pFile);
        void                    SaveTextureCoords(CTreeFileAccess* pFile) const;
        void                    SaveCollisionObjects(CTreeFileAccess* pFile) const;
        void                    ParseShadowProjectionInfo(CTreeFileAccess* pFile);
        void                    SaveUserData(CTreeFileAccess* pFile) const;
        void                    ParseUserData(CTreeFileAccess* pFile);
        void                    SaveSupplementalTexCoordInfo(CTreeFileAccess* pFile) const;
        void                    ParseSupplementalTexCoordInfo(CTreeFileAccess* pFile);
        void                    SaveStandardShaderInfo(CTreeFileAccess* pFile) const;
        void                    ParseStandardShaderInfo(CTreeFileAccess* pFile);
        void                    SaveStandardShaderInfo(CTreeFileAccess& cFile) const;
        void                    ParseStandardShaderInfo(CTreeFileAccess& cFile);
        void                    SaveSupplementalCollisionObjectsInfo(CTreeFileAccess& cFile) const;
        void                    ParseSupplementalCollisionObjectsInfo(CTreeFileAccess& cFile);
        void                    SaveSupplementalLodInfo(CTreeFileAccess& cFile) const;
        void                    ParseSupplementalLodInfo(CTreeFileAccess& cFile);

        void                    RecoverDeprecatedMaps(void);
static  char*         CALL_CONV CopyUserData(const char* pData);


        ///////////////////////////////////////////////////////////////////////  
        //  Geometry
        
        void                    GetBranchGeometry(SGeometry& sGeometry);
        void                    GetFrondGeometry(SGeometry& sGeometry);
        void                    GetLeafGeometry(SGeometry& sGeometry);
        void                    GetBillboardGeometry(SGeometry& sGeometry);
        void                    SetupHorizontalBillboard(void);
		float                   ComputeLodCurve(float fStart, float fEnd, float fPercent, bool bConcaveUp);
		float                   ComputeLodCurveBB(float fStart, float fEnd, float fPercent);


        ///////////////////////////////////////////////////////////////////////  
        //  Member variables

        // general
        CTreeEngine*            m_pEngine;                  		// core tree-generating engine
        CIndexedGeometry*       m_pBranchGeometry;          		// abstraction mechanism for branch geometry
        CLeafGeometry*          m_pLeafGeometry;            		// abstraction mechanism for leaf geometry
        SGeometry::SLeaf*       m_pLeafLods;
        CLightingEngine*        m_pLightingEngine;          		// engine for computing static/dynamic lighting data
        CWindEngine*            m_pWindEngine;              		// engine for computing CPU/GPU wind effects
        CSimpleBillboard*       m_pSimpleBillboard;
static	EMemoryPreference		m_eMemoryPreference;

        // leaf lod
        ELodMethod              m_eLeafLodMethod;           		// which leaf wind method is currently being used
        float                   m_fLeafLodTransitionRadius; 		// determines how much blending occurs between two separate leaf LOD levels
        float                   m_fLeafLodCurveExponent;    		// exponent value used in the leaf LOD blending equation
        float                   m_fLeafSizeIncreaseFactor;  		// value that controls how much larger leaf clusters get as LOD decreases
        float                   m_fLeafTransitionFactor;    		// value that controls the intersection point of SMOOTH_1 transitions
        float*                  m_pLeafLodSizeFactors;      		// array, GetNumLeafLodLevels()'s in size, containing leaf LOD scale factors

        // instancing & ref counting
        unsigned int*           m_pInstanceRefCount;        		// single value shared among instances - number of active instances
        STreeInstanceData*      m_pInstanceData;            		// if instance, differentiating data is stored here
        SInstanceList*          m_pInstanceList;            		// each tree contains a list of its instances
static  unsigned int            m_uiAllRefCount;            		// single value shared by all CSpeedTreeRT instances

        // other
        int                     m_nFrondLevel;              		// from SpeedTreeCAD - branch level where fronds begin
        float*                  m_pTreeSizes;               		// contains all tree extents, including billboard sizes
        float                   m_fTargetAlphaValue;        		// value used for leaf alpha mask function
        bool                    m_bTreeComputed;            		// some operations are not valid once the geometry has been computed
        int                     m_nBranchWindLevel;         		// from SpeedTreeCAD - branch level where wind effects are active

        // texture coords
        SEmbeddedTexCoords*     m_pEmbeddedTexCoords;       		// embedded leaf and billboard texture coords
static  bool                    m_bTextureFlip;             		// used to flip coordinates for DirectX, Gamebryo, etc.

        // shadow projection
        CProjectedShadow*       m_pProjectedShadow;         		// self-shadow projection

        // billboard
static  bool                    m_bDropToBillboard;         		// flag specifying if last LOD will be simple single billboard
static  float                   m_fCameraAzimuth;
static  float                   m_fCameraPitch;

        // collision objects
        SCollisionObjects*      m_pCollisionObjects;        		// collision objects

        // fronds
        CFrondEngine*           m_pFrondEngine;             		// engine for computing fronds based on branch geometry
        CIndexedGeometry*       m_pFrondGeometry;           		// abstraction mechanism for frond geometry

        // user data
        char*                   m_pUserData;                		// user specified data

        // horizontal billboard
        bool                    m_b360Billboard;            		// indicates that a 360 degree billboard sequence is present
        bool                    m_bHorizontalBillboard;     		// indicates that a horizontal billboard is present in the embedded texcoords
        float                   m_afHorizontalCoords[12];   		// vertices of the horizontal billboard
static  float                   m_fHorizontalFadeStartAngle;		// in degrees
static  float                   m_fHorizontalFadeEndAngle;  		// in degrees
static  float                   m_fHorizontalFadeValue;

        // standard shader support
        float                   m_fBranchLightScalar;       		// branch light scalar used by standard SpeedTree pixel shaders
        float                   m_fFrondLightScalar;        		// frond light scalar used by standard SpeedTree pixel shaders
        float                   m_fLeafLightScalar;         		// leaf light scalar used by standard SpeedTree pixel shaders
        float                   m_fGlobalLightScalar;       		// global value used to multiply branch, frond, and leaf light scalars
        float                   m_fAmbientScalar;           		// value used to scale the ambient material of branches, fronds, and leaves
        float                   m_fBillboardDarkSideLightScalar;    // billboard light scalar used on the dark side of the billboards
        float                   m_fBillboardBrightSideLightScalar;  // billboard light scalar used on the bright side of the billboards
        float                   m_fBillboardAmbientScalar;          // scales the ambient value of the billboard material

        // lod parameters (most of these are here for faster, more convenient LOD computations in GetLodValues())
        int                     m_nNumBranchLodLevels;				// the number of discrete branch LODs, set in SpeedTreeCAD
        int                     m_nNumFrondLodLevels;				// the number of discrete frond LODs, set in SpeedTreeCAD
        int                     m_nNumLeafLodLevels;				// the number of discrete leaf LODs, set in SpeedTreeCAD
        float                   m_fTransitionWidth;					// in LOD range [0,1], twice the m_fLeafLodTransitionRadius value; used to compute LOD values at run-time
        float                   m_fCrestWidth;						// in LOD range [0,1], width between leaf LOD transitions
        float                   m_fCrestWidthBB;					// in LOD range [0,1], width between billboard LOD transitions
        float                   m_fCycleLength;						// in LOD range [0,1], m_fCrestWidth + m_fTransitionWidth
        float                   m_fCycleLengthBB;					// in LOD range [0,1], m_fCrestWidthBB + m_fTransitionWidth
		float					m_fBranchLodFadeDistance;			// in LOD range [0,1], controls intermediate branch fading behavior (SPEEDTREE_BRANCH_FADING in ref app)
		float					m_fFrondLodFadeDistance;			// in LOD range [0,1], controls intermediate frond fading behavior (SPEEDTREE_BRANCH_FADING in ref app)
		bool					m_bApplyFadingToExtrusions;			// determines if intermediate fading applies to extruded frond geometry
		float					m_fBillboardTransition;				// billboard transition percentage, set in SpeedTreeCAD

        // maps
        CMapBank*               m_pMapBank;							// holds all of the texture map data for this tree model
};


/////////////////////////////////////////////////////////////////////////////
// Forward reference - for implementation hiding

class CSpeedWindDef;


/////////////////////////////////////////////////////////////////////////////
// class CSpeedWind

class ST_STORAGE_CLASS CSpeedWind
{
public:
friend	class CSpeedWindDef;

							CSpeedWind( );
							CSpeedWind(const CSpeedWind& cCopy);
		virtual				~CSpeedWind( );

		// operators
		CSpeedWind&			operator=(const CSpeedWind& cRight);

		// parameter setting
		void				Reset(void);
		void				SetQuantities(int iNumWindMatrices, int iNumLeafAngles);
		void				SetWindResponse(float fResponse, float fReponseLimit);
		void				SetWindStrengthAndDirection(float fWindStrength, float fWindDirectionX, float fWindDirectionY, float fWindDirectionZ);
		void				SetMaxBendAngle(float fMaxBendAngle);
		void				SetExponents(float fBranchExponent, float fLeafExponent);
		void				SetGusting(float fGustStrengthMin, float fGustStrengthMax, float fGustFrequency, float fGustDurationMin, float fGustDurationMax);
		void				SetBranchHorizontal(float fLowWindAngle, float fHighWindAngle, float fLowWindSpeed, float fHighWindSpeed);
		void				SetBranchVertical(float fLowWindAngle, float fHighWindAngle, float fLowWindSpeed, float fHighWindSpeed);
		void				SetLeafRocking(float fLowWindAngle, float fHighWindAngle, float fLowWindSpeed, float fHighWindSpeed);
		void				SetLeafRustling(float fLowWindAngle, float fHighWindAngle, float fLowWindSpeed, float fHighWindSpeed);

		// parameter getting
		void				GetQuantities(int& iNumWindMatrices, int& iNumLeafAngles) const;
		void				GetWindResponse(float& fResponse, float& fReponseLimit) const;
		void				GetWindStrengthAndDirection(float& fWindStrength, float& fWindDirectionX, float& fWindDirectionY, float& fWindDirectionZ) const;
		float				GetMaxBendAngle(void) const;
		void				GetExponents(float& fBranchExponent, float& fLeafExponent) const;
		void				GetGusting(float& fGustStrengthMin, float& fGustStrengthMax, float& fGustFrequency, float& fGustDurationMin, float& fGustDurationMax) const;
		void				GetBranchHorizontal(float& fLowWindAngle, float& fHighWindAngle, float& fLowWindSpeed, float& fHighWindSpeed) const;
		void				GetBranchVertical(float& fLowWindAngle, float& fHighWindAngle, float& fLowWindSpeed, float& fHighWindSpeed) const;
		void				GetLeafRocking(float& fLowWindAngle, float& fHighWindAngle, float& fLowWindSpeed, float& fHighWindSpeed) const;
		void				GetLeafRustling(float& fLowWindAngle, float& fHighWindAngle, float& fLowWindSpeed, float& fHighWindSpeed) const;
		float				GetTime(void) const;

		// render interface
		void				Advance(float fCurrentTime, bool bUpdateMatrices = true, bool bUpdateLeafAngleMatrices = false, float fCameraX = 0.0f, float fCameraY = 0.0f, float fCameraZ = 1.0f);
		void				UpdateSpeedTreeRT(void) const;
		void				UpdateTree(CSpeedTreeRT* pTree) const;
		float				GetFinalStrength(void);
		unsigned int		GetNumWindMatrices(void) const;
		const float*		GetWindMatrix(unsigned int uiIndex) const;
		unsigned int		GetNumLeafAngleMatrices(void) const;
		const float*		GetLeafAngleMatrix(unsigned int uiIndex) const;
		bool				GetRustleAngles(float* pAngles) const; // assumes pointer to already-allocated memory
		bool				GetRockAngles(float* pAngles) const; // assumes pointer to already-allocated memory

		// file I/O
		bool				Load(const char* pFilename);
		bool				Load(const char* pData, unsigned int uiNumBytes);
		bool				Save(const char* pFilename) const;
		const char*			Save(unsigned int& uiNumBytes) const;
		const char*			GetFilename(void) const;

		// blending SpeedWinds into this one
		void				InterpolateTarget(const CSpeedWind* pWind1, const CSpeedWind* pWind2, float fInterpolation);
		void				InterpolateCurrent(const CSpeedWind* pWind1, const CSpeedWind* pWind2, float fInterpolation, bool bUpdateMatrices = true, bool bUpdateLeafAngleMatrices = false, float fCameraX = 0.0f, float fCameraY = 0.0f, float fCameraZ = 1.0f);
		void				BlendParameters(const CSpeedWind** pWinds, const float* pWeights, unsigned int uiNumWinds);

		// internal use only
		void				SetUseRandomBuffer(bool bUse, unsigned int uiSeed = 0);

		CSpeedWindDef*		m_pDefinition;  // SpeedWind variables stored here
};


// Macintosh export control
#ifdef __APPLE__
#pragma export off
#endif
