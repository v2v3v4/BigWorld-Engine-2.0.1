/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SPEEDTREE_RENDERER_UTIL_HPP_
#define SPEEDTREE_RENDERER_UTIL_HPP_

#include "speedtree_config_lite.hpp"
#include "speedtree_vertex_types.hpp"


namespace speedtree {

/***
 *

This file declares the data structures that actually hold the render data
for speedtrees, and also functions to extract it from a CSpeedTreeRT object.
Listed below are the core data structures and functions defined here. Please
refer to their inlined doxygen documentation for detailed information.

Listed below are the core data structures and functions defined here. Please
refer to their inlined doxygen documentation for detailed information.

>> Structs
	>>  CommonTraits
	>>	BranchTraits
	>>	FrondTraits
	>>	LeafTraits
	>>	BillboardTraits

>> Functions:
	>>	createPartRenderData()
 
 *
 ***/

	/**
 *	Traits structs
 *
 *	Because branches, fronds, leaves and billboards share very similar 
 *	data structures, some of the logic of extracting their rendering data
 *	from a CSpeedTreeRT object is implemented as a template functions. But,
 *	because they are not always exactly the same, a Traits struct is used 
 *	by the template function to specialise its behaviour for each of these 
 *	tree parts.
 *
 *	@see CommonTraits, BranchTraits, FrondTraits, LeafTraits, BillboardTraits
 */

/**
 *	Trait struct used for handling the common data in fronds/branches.
 */
class CommonTraits
{
public:
	typedef BranchVertex VertexType;
	typedef TreePartRenderData< VertexType > RenderDataType;

	static bool isSingleFaced() { return true; }

#ifdef OPT_BUFFERS
	typedef VertexBufferWrapper< VertexType >	VBufferType;
	typedef SmartPointer< VBufferType >			VertexBufferPtr;
	typedef SmartPointer< IndexBufferWrapper >  IndexBufferPtr;
	
	static VertexBufferPtr				s_vertexBuffer_;
	static STVector< VertexType >		s_vertexList_;
	static IndexBufferPtr				s_indexBuffer_;
	static STVector< uint32 >			s_indexList_;
#endif
};

/**
 *	Trait struct used to retrieve branch information from a 
 *	CSpeedTreeRT object. 
 */
class BranchTraits : public CommonTraits
{
public:
	static std::string getTextureFilename( 
							const CSpeedTreeRT::SMapBank & textureBank )
	{ 
		BW_GUARD;
		std::string fullName =
			textureBank.m_pBranchMaps[CSpeedTreeRT::TL_DIFFUSE];
		return BWResource::getFilename( fullName );
	}
	
	#if SPT_ENABLE_NORMAL_MAPS
		static std::string getNormalMapFilename(
			const CSpeedTreeRT::SMapBank & textureBank )
		{ 
			BW_GUARD;
			std::string fullName = 
				textureBank.m_pBranchMaps[CSpeedTreeRT::TL_NORMAL];
			return BWResource::getFilename( fullName );
		}
	#endif // SPT_ENABLE_NORMAL_MAPS

	static const CSpeedTreeRT::SGeometry::SIndexed & 
		getGeometryData( CSpeedTreeRT & speedTree )
	{ 
		BW_GUARD;
		static CSpeedTreeRT::SGeometry geometry;
		speedTree.GetGeometry( geometry, SpeedTree_BranchGeometry ); 
		return geometry.m_sBranches;
	}

	static int getLodLevelCount( const CSpeedTreeRT & speedTree )
	{
		BW_GUARD;
		return speedTree.GetNumBranchLodLevels();
	}
};

/**
 *	Trait struct used to retrieve fronds information from a 
 *	CSpeedTreeRT object.
 */
class FrondTraits : public CommonTraits
{
public:
	static std::string 
		getTextureFilename( const CSpeedTreeRT::SMapBank & textureBank )
	{
		BW_GUARD;
		std::string fullName = 
			textureBank.m_pCompositeMaps[CSpeedTreeRT::TL_DIFFUSE];
		return BWResource::getFilename( fullName );
	}

	#if SPT_ENABLE_NORMAL_MAPS
		static std::string getNormalMapFilename(
			const CSpeedTreeRT::SMapBank & textureBank )
		{ 
			BW_GUARD;
			std::string fullName = 
				textureBank.m_pCompositeMaps[CSpeedTreeRT::TL_NORMAL];
			return BWResource::getFilename( fullName );
		}
	#endif // SPT_ENABLE_NORMAL_MAPS

	static const CSpeedTreeRT::SGeometry::SIndexed & 
		getGeometryData( CSpeedTreeRT & speedTree )
	{ 
		BW_GUARD;
		static CSpeedTreeRT::SGeometry geometry;
		speedTree.GetGeometry( geometry, SpeedTree_FrondGeometry );
		return geometry.m_sFronds;
	}

	static int getLodLevelCount( const CSpeedTreeRT & speedTree )
	{
		BW_GUARD;
		return speedTree.GetNumFrondLodLevels();
	}
};

/**
 *	Trait struct used to retrieve leaves geometry data from a 
 *	CSpeedTreeRT object. 
 */
class LeafTraits
{
public:
	typedef LeafVertex VertexType;
	typedef TreePartRenderData< VertexType > RenderDataType;

	static std::string
		getTextureFilename( const CSpeedTreeRT::SMapBank & textureBank )
	{
		BW_GUARD;
		std::string fullName = 
			textureBank.m_pCompositeMaps[CSpeedTreeRT::TL_DIFFUSE];
		return BWResource::getFilename( fullName );
	}

	#if SPT_ENABLE_NORMAL_MAPS
		static std::string getNormalMapFilename(
			const CSpeedTreeRT::SMapBank & textureBank )
		{ 
			BW_GUARD;
			std::string fullName = 
				textureBank.m_pCompositeMaps[CSpeedTreeRT::TL_NORMAL];
			return BWResource::getFilename( fullName );
		}
	#endif // SPT_ENABLE_NORMAL_MAPS

#ifdef OPT_BUFFERS
	typedef VertexBufferWrapper< VertexType >	VBufferType;
	typedef SmartPointer< VBufferType >			VertexBufferPtr;
	typedef SmartPointer< IndexBufferWrapper >  IndexBufferPtr;	

	static VertexBufferPtr				s_vertexBuffer_;
	static STVector< VertexType >		s_vertexList_;
	static IndexBufferPtr				s_indexBuffer_;
	static STVector< uint32 >			s_indexList_;
#endif
};

/**
 *	Trait struct used to retrieve billboards information from a 
 *	CSpeedTreeRT object. 
 */
class BillboardTraits
{
public:
	typedef BillboardVertex VertexType;
	typedef TreePartRenderData< VertexType > RenderDataType;

	static std::string 
		getTextureFilename( const CSpeedTreeRT::SMapBank & textureBank )
	{
		BW_GUARD;
		std::string fullName = 
			textureBank.m_pCompositeMaps[CSpeedTreeRT::TL_DIFFUSE];
		return BWResource::getFilename( fullName );
	}

	#if SPT_ENABLE_NORMAL_MAPS
		static std::string getNormalMapFilename(
			const CSpeedTreeRT::SMapBank & textureBank )
		{ 
			BW_GUARD;
			std::string fullName = 
				textureBank.m_pCompositeMaps[CSpeedTreeRT::TL_NORMAL];
			return BWResource::getFilename( fullName );
		}
	#endif // SPT_ENABLE_NORMAL_MAPS
};

// ---------------------------------------------------------- TreePartRenderData

/**
 *	Computes the total space required to serialise the render 
 *	data stored by this object
 *
 *	@see		write(), read()
 *
 *	@return		total space required (in bytes).
 */
template< typename VertexType >
int TreePartRenderData<VertexType>::size() const
{
	BW_GUARD;
	/**
	Lod data:
		1 x lod num (m)               4 bytes
		  m x   vertices size (n)     4 bytes
			    vertices data	      n * sizeof(vetextype) bytes
			    strips num (o)        4 bytes
		    o x   strip size (p)      4 bytes
				  strip data          p * 2 bytes
				
		4 + m * ( (4+n*sizeof(VertexType) + (4+o * (4+p*2)))
		
	Texture maps:
		diffuse map filename length (q) 4 bytes
		diffuse map filename            q bytes
		normal map filename length (r)  4 bytes
		normal map filename             r bytes
		
	**/
	int totalSize = 0;
	
	// number of verts
	totalSize += sizeof(int);
	if ( verts_ )
		totalSize += verts_->count() * sizeof(VertexType);

	// number of lods
	totalSize += sizeof(int);                                         // 4 +
	
	int m = this->lod_.size();
	for ( int i=0; i<m; ++i )                                             // m *
	{
		// number of indices for this strip
		totalSize += sizeof(int);                                    // (4 +
			
		// index data
		if (lod_[i].index_)
		{
			totalSize += lod_[i].index_->count() * sizeof(uint32);
		}
	}

	// texture maps		
	totalSize += sizeof(int);
	bool a = this->diffuseMap_.exists();
	totalSize += a ? this->diffuseMap_->resourceID().length() : 0;
	
	totalSize += sizeof(int);
	bool b = this->normalMap_.exists();
	totalSize += b ? this->normalMap_->resourceID().length() : 0;

	return totalSize;
}
/**
 *	Serialises the render data stored by this object into the block
 *	of memory passed as argument. Assumes there is enough space in
 *	the memory block provided (as returned by the size() function).
 *
 *	@see		size(), read()
 *
 *	@param		pdata	pointer to block of memory where to write data to.
 *
 *	@return		one byte past the last byte written.
 */
template< typename VertexType >
char * TreePartRenderData<VertexType>::write(char * pdata,
				const VertexVector& verts, const IndexVector& indices ) const
{
	BW_GUARD;
	char * data = pdata;

	// vertex data
	if ( this->verts_ )
	{
		int n = this->verts_->count();
		*(int*)data = n;
		data += sizeof(int);

		if ( n > 0 )
		{
			memcpy( data, 
					&verts[ verts_->start() ],
					sizeof( VertexType ) * n );
			data += n * sizeof(VertexType);
		}
	}
	else
	{
		*(int*)data = 0;
		data += sizeof(int);
	}

	// number of lods
	int m = this->lod_.size();
	*(int*)data = m;
	data += sizeof(int);

	for ( int i=0; i<m; ++i )
	{
		if ( this->lod_[i].index_ )
		{
			int n = this->lod_[i].index_->count();
			*(int*)data = n;
			data += sizeof(int);
			if ( n > 0 )
			{
				memcpy( data, 
					&indices[ this->lod_[i].index_->start() ],
					sizeof( uint32 ) * n );
				data += n * sizeof( uint32 );
			}
		}
		else
		{
			*(int*)data = 0;
			data += sizeof(int);
		}
	}

	// texture maps
	const std::string & diff = this->diffuseMap_.exists() ?	
		this->diffuseMap_->resourceID() : "";
	int q = diff.length();
	*(int*)data = q;
	data += sizeof(int);
	memcpy(data, diff.c_str(), q);
	data += q;
	
	const std::string & norm = this->normalMap_.exists() ?
		this->normalMap_->resourceID() : "";
	int r = norm.length();
	*(int*)data = r;
	data += sizeof(int);
	memcpy( data, norm.c_str(), r );
	data += r;
	
	MF_ASSERT(data - pdata == this->size());
	
	return data;
}


/**
 *	Deserialises render data from the block of memory passed as argument.
 *	Assumes the data was saved using the write() function.
 *
 *	@see		write(), read()
 *
 *	@param		pdata	pointer to block of memory where to read data from.
 *
 *	@return		one byte past the last byte read.
 */
template< typename VertexType >
const char * TreePartRenderData<VertexType>::read( const char * data,
								VertexVector& verts, IndexVector& indices )
{
	BW_GUARD;
	// number of verts
	int n = *(int*)data;
	data += sizeof(int);

	// vertex data
	if ( n > 0 )
	{
		verts_ = verts.newSlot( n );

		verts.resize( verts_->start() + n );

		memcpy( &verts[ verts_->start() ],
				data,
				sizeof( VertexType ) * n );
		data += n * sizeof(VertexType);
	}

	// number of lods
	int m = *(int*)data;
	data += sizeof(int);
	
	for ( int i=0; i<m; ++i )
	{
		LodData lod;
	
		int p = *(int*)data;
		data += sizeof(int);

		// index data
		if ( p > 0 )
		{
			lod.index_ = indices.newSlot( p );
			indices.resize( lod.index_->start() + p );		
			memcpy( &indices[ lod.index_->start() ],
					data,
					sizeof( uint32 ) * p );
			data += p * sizeof( uint32 );
		}				

		this->lod_.push_back( lod );
	}
	
	// texture maps
	int q = *(int*)data;
	data += sizeof(int);
	if ( q>0 )
	{
		char * diffCStr = new char[q + 1];
		memset( diffCStr, '\0', q + 1 );
		strncpy( diffCStr, data, q );
		std::string diffStr(diffCStr);
		delete [] diffCStr;
		this->diffuseMap_ = Moo::TextureManager::instance()->get(diffStr,
							 true, true, true, "texture/speedtree/diffuse");
		data += q;

		if ( !this->diffuseMap_ || this->diffuseMap_->pTexture() == NULL )
		{
			std::string errorMsg = "SpeedTree: Cannot load diffuse map: ";
			throw std::runtime_error( errorMsg + diffStr );
		}
	}
	
	int r = *(int*)data;
	data += sizeof(int);
	if ( r>0 )
	{
		char * normCStr = new char[r + 1];
		memset( normCStr, '\0', r + 1 );
		strncpy( normCStr, data, r );
		std::string normStr(normCStr);
		delete [] normCStr;
	
		if ( !normStr.empty() )
		{
			this->normalMap_ = Moo::TextureManager::instance()->get(normStr,
							true, false, true, "texture/speedtree/normals");

			data += r;

			if ( this->normalMap_ && this->normalMap_->pTexture() == NULL )
			{
				std::string errorMsg = "SpeedTree: Cannot load normal map: ";
				throw std::runtime_error(errorMsg + normStr);
			}
		}
	}
		
	return data;
}	


/**
 *	Computes the total space required to serialise the render 
 *	data stored by this object
 *
 *	@see		write(), read()
 *
 *	@return		total space required (in bytes).
 */
template< typename VertexType >
int TreePartRenderData<VertexType>::oldSize() const
{
	BW_GUARD;
	/**
	Lod data:
		1 x lod num (m)               4 bytes
		  m x   vertices size (n)     4 bytes
			    vertices data	      n * sizeof(vetextype) bytes
			    strips num (o)        4 bytes
		    o x   strip size (p)      4 bytes
				  strip data          p * 2 bytes
				
		4 + m * ( (4+n*sizeof(VertexType) + (4+o * (4+p*2)))
		
	Texture maps:
		diffuse map filename length (q) 4 bytes
		diffuse map filename            q bytes
		normal map filename length (r)  4 bytes
		normal map filename             r bytes
		
	**/
	int totalSize = 0;
	
	// number of lods
	totalSize += sizeof(int);                                         // 4 +
	
	int m = this->lod_.size();
	for ( int i=0; i<m; ++i )                                             // m *
	{
		// number of vertices for this lod
		totalSize += sizeof(int);                                         // (4 +
		
		// vertex data
		totalSize += this->lod_[i].vertices_->size() * sizeof(VertexType); // n * sizeof(VertexType) +
		
		// number of strips for this lod
		totalSize += sizeof(int);                                        // (4 +
		int o = this->lod_[i].strips_.size();
		for ( int j=0; j<o; ++j )                                         // o *
		{
			// number of indices for this strip
			totalSize += sizeof(int);                                    // (4 +
			
			// index data
			totalSize += 
				this->lod_[i].strips_[j]->size() * sizeof(unsigned short); // p*2)))
		}
	}

	// texture maps		
	totalSize += sizeof(int);
	bool a = this->diffuseMap_.exists();
	totalSize += a ? this->diffuseMap_->resourceID().length() : 0;
	
	totalSize += sizeof(int);
	bool b = this->normalMap_.exists();
	totalSize += b ? this->normalMap_->resourceID().length() : 0;

	return totalSize;
}
/**
 *	Serialises the render data stored by this object into the block
 *	of memory passed as argument. Assumes there is enough space in
 *	the memory block provided (as returned by the size() function).
 *
 *	@see		size(), read()
 *
 *	@param		pdata	pointer to block of memory where to write data to.
 *
 *	@return		one byte past the last byte written.
 */
template< typename VertexType >
char * TreePartRenderData<VertexType>::oldWrite(char * pdata) const
{
	BW_GUARD;
	char * data = pdata;

	// number of lods
	int m = this->lod_.size();
	*(int*)data = m;
	data += sizeof(int);

	for ( int i=0; i<m; ++i )
	{
		// number of vertices for this lod
		int n = this->lod_[i].vertices_->size();
		*(int*)data = n;
		data += sizeof(int);

		// vertex data
		if ( n > 0 )
		{
			this->lod_[i].vertices_->dump( (VertexType*)data );
			data += n * sizeof(VertexType);
		}
		
		// number of strips for this lod
		int o = this->lod_[i].strips_.size();
		*(int*)data = o;
		data += sizeof(int);
		
		for ( int j=0; j<o; ++j )
		{
			// number of indices for this strip
			int p = this->lod_[i].strips_[j]->size();
			*(int*)data = p;
			data += sizeof(int);

			// index data
			if ( p > 0 )
			{
				this->lod_[i].strips_[j]->dump((unsigned short*)data);
				data += p * sizeof(unsigned short);
			}
		}
	}

	// texture maps
	const std::string & diff = this->diffuseMap_.exists() ?	
		this->diffuseMap_->resourceID() : "";
	int q = diff.length();
	*(int*)data = q;
	data += sizeof(int);
	memcpy(data, diff.c_str(), q);
	data += q;
	
	const std::string & norm = this->normalMap_.exists() ?
		this->normalMap_->resourceID() : "";
	int r = norm.length();
	*(int*)data = r;
	data += sizeof(int);
	memcpy( data, norm.c_str(), r );
	data += r;
	
	MF_ASSERT(data - pdata == this->oldSize());
	
	return data;
}


/**
 *	Deserialises render data from the block of memory passed as argument.
 *	Assumes the data was saved using the write() function.
 *
 *	@see		write(), read()
 *
 *	@param		data	pointer to block of memory where to read data from.
 *
 *	@return		one byte past the last byte read.
 */
template< typename VertexType >
const char * TreePartRenderData<VertexType>::oldRead( const char * data )
{
	BW_GUARD;
	// number of lods
	int m = *(int*)data;
	data += sizeof(int);
	
	for ( int i=0; i<m; ++i )
	{
		LodData lod;
	
		// number of vertices for this lod
		int n = *(int*)data;
		data += sizeof(int);

		// vertex data
		lod.vertices_ = new VertexBufferWrapper();
		if ( n > 0 )
		{
			if (lod.vertices_->reset( n ))
			{
				lod.vertices_->copy( (VertexType*)data );
			}

			data += n * sizeof(VertexType);
		}

		// number of strips for this lod
		int o = *(int*)data;
		data += sizeof(int);
		
		for ( int j=0; j<o; ++j )
		{
			lod.strips_.push_back( new IndexBufferWrapper() ); 
		
			// number of indices for this strip
			int p = *(int*)data;
			data += sizeof(int);

			// index data
			if ( p > 0 )
			{
				if (lod.strips_.back()->reset( p ))
				{
					lod.strips_.back()->copy( (unsigned short*)data );
				}

				data += p * sizeof(unsigned short);
			}				
		}

		this->lod_.push_back( lod );
	}
	
	// texture maps
	int q = *(int*)data;
	data += sizeof(int);
	if ( q>0 )
	{
		char * diffCStr = new char[q + 1];
		memset( diffCStr, '\0', q + 1 );
		strncpy( diffCStr, data, q );
		std::string diffStr(diffCStr);
		delete [] diffCStr;
		this->diffuseMap_ = Moo::TextureManager::instance()->get(diffStr,
							 true, true, true, "texture/speedtree/diffuse");
		data += q;

		if ( !this->diffuseMap_ || this->diffuseMap_->pTexture() == NULL )
		{
			std::string errorMsg = "SpeedTree: Cannot load diffuse map: ";
			throw std::runtime_error( errorMsg + diffStr );
		}
	}
	
	int r = *(int*)data;
	data += sizeof(int);
	if ( r>0 )
	{
		char * normCStr = new char[r + 1];
		memset( normCStr, '\0', r + 1 );
		strncpy( normCStr, data, r );
		std::string normStr(normCStr);
		delete [] normCStr;
	
		if ( !normStr.empty() )
		{
			this->normalMap_ = Moo::TextureManager::instance()->get(normStr,
							true, false, true, "texture/speedtree/normals");

			data += r;

			if ( this->normalMap_ && this->normalMap_->pTexture() == NULL )
			{
				std::string errorMsg = "SpeedTree: Cannot load normal map: ";
				throw std::runtime_error(errorMsg + normStr);
			}
		}
	}
		
	return data;
}	

// --------------------------------------------------- Data extraction functions

// If only the .dds is available the normal call to correct case
// will not work so we need an extra function to do it.
// If the .dds does not exist the name will remain unchanged.
static std::string correctCaseIfOnlyDDS( std::string resName )
{
	std::string ext = std::string( "." ) + BWResource::getExtension( resName );
	std::string ddsName = BWResource::changeExtension( resName, std::string( ".dds" ) );
	ddsName = BWResource::instance().correctCaseOfPath( ddsName );
	return BWResource::changeExtension( ddsName, ext );
}

/**
 *	Retrieves basic render data from a speed tree from a speed tree 
 *	object for a specific tree part. Uses the trait object for that.
 *
 *	@param	speedTree	speedtree object where to get the data from.
 *	@param	datapath	path where to look for aditional data files.
 *
 *	@return				a new RenderDataType object.
 */
template< typename TreePartTraits >
typename TreePartTraits::RenderDataType getCommonRenderData(
	CSpeedTreeRT      & speedTree,
	const std::string & datapath )
{
	BW_GUARD;
	typename TreePartTraits::RenderDataType result;

	// get part texture maps
	CSpeedTreeRT::SMapBank textureBank;
	speedTree.GetMapBank( textureBank );

	// diffuse map
	std::string difFileName = TreePartTraits::getTextureFilename( textureBank );
	std::string difFullFileName = datapath + difFileName;

	if ( difFileName == "" ) // tree part doesn't exist.
		return result;

	// Since we are getting this string from the .spt file we can not assume
	// the case is correct, therefore we correct the case
	difFullFileName = BWResource::instance().correctCaseOfPath( difFullFileName );
	difFullFileName = correctCaseIfOnlyDDS( difFullFileName );

	result.diffuseMap_ = Moo::TextureManager::instance()->get( difFullFileName,
								true, true, true, "texture/speedtree/diffuse" );
	if ( !result.diffuseMap_ || result.diffuseMap_->pTexture() == NULL )
	{
		std::string errorMsg = "SpeedTree: Cannot load diffuse map: ";
		throw std::runtime_error( errorMsg + difFullFileName );
	}
	
	#if SPT_ENABLE_NORMAL_MAPS
		std::string normFileName  = 
			TreePartTraits::getNormalMapFilename( textureBank );
		if ( !normFileName.empty() )
		{
			std::string normFullFileName = datapath + normFileName;

			// Since we are getting this string from the .spt file we can not assume
			// the case is correct, therefore we correct the case
			normFullFileName = BWResource::instance().correctCaseOfPath( normFullFileName );
			normFullFileName = correctCaseIfOnlyDDS( normFullFileName );

			result.normalMap_ = Moo::TextureManager::instance()->get(
									normFullFileName, true, false, true,
									"texture/speedtree/normals" );

			if ( result.normalMap_ && result.normalMap_->pTexture() == NULL )
			{
				std::string errorMsg = "SpeedTree: Cannot load normal map: ";
				throw std::runtime_error( errorMsg + difFullFileName );
			}
		}
	#endif // SPT_ENABLE_NORMAL_MAPS
	
	return result;
}
// Compression range taken from the SpeedTree reference
// application for the wind matrix index / weight compression.
const float c_fWindWeightCompressionRange = 0.98f;

/**
 *	Creates a render data object to render the part of
 *	a tree specified by the trait template parameter.
 *
 *	@param	speedTree	speedtree object where to get the data from.
 *	@param	speedWind	SpeedTree wind object to use in wind calculations.
 *	@param	datapath	path where to look for aditional data files.
 *
 *	@return				a new RenderDataType object.
 */
template< typename TreePartTraits >
typename TreePartTraits::RenderDataType createPartRenderData(
	CSpeedTreeRT      & speedTree,
	CSpeedWind	      & speedWind,
	const std::string & datapath )
{
	BW_GUARD;
	TreePartTraits::RenderDataType result =
		getCommonRenderData< TreePartTraits >( speedTree, datapath );

	// extract geometry for all lod levels
	const CSpeedTreeRT::SGeometry::SIndexed & data = 
		TreePartTraits::getGeometryData( speedTree );

#ifdef OPT_BUFFERS
	if ( data.m_nNumVertices > 0 )
	{
		result.verts_ = TreePartTraits::s_vertexList_.newSlot( data.m_nNumVertices );

		MF_ASSERT( data.m_pCoords );
		for ( int i=0; i < data.m_nNumVertices; ++i )
		{
			TreePartTraits::VertexType vert;

			// .xyz = coords
			// .w = self-shadow s-texcoord
			//float fSelfShadowCoord_S = m_bBranchesReceiveShadows ? sBranches.m_pTexCoords[CSpeedTreeRT::TL_SHADOW][i * 2 + 0] : 0.0f;
			//cBuffer.Vertex4(sBranches.m_pCoords[i * 3 + 0], sBranches.m_pCoords[i * 3 + 1], sBranches.m_pCoords[i * 3 + 2], fSelfShadowCoord_S);

			// position (in speedtree, z is up)
			vert.position_.x = +data.m_pCoords[i * 3 + 0];
			vert.position_.y = +data.m_pCoords[i * 3 + 1];
			vert.position_.z = -data.m_pCoords[i * 3 + 2];

			//// .xyz = normal
			//// .w = self-shadow t-texcoord
			//st_assert(sBranches.m_pNormals);
			//for (i = 0; i < sBranches.m_nNumVertices; ++i)
			//{
			//	float fSelfShadowCoord_T = m_bBranchesReceiveShadows ? sBranches.m_pTexCoords[CSpeedTreeRT::TL_SHADOW][i * 2 + 1] : 0.0f;
			//	cBuffer.TexCoord4(0, sBranches.m_pNormals[i * 3 + 0], sBranches.m_pNormals[i * 3 + 1], sBranches.m_pNormals[i * 3 + 2], fSelfShadowCoord_T);
			//}
			vert.normal_.x   = +data.m_pNormals[i * 3 + 0];
			vert.normal_.y   = +data.m_pNormals[i * 3 + 1];
			vert.normal_.z   = -data.m_pNormals[i * 3 + 2];

			#if SPT_ENABLE_NORMAL_MAPS
				// tangents (binormals are derived in the vertex shader)
				// .xyz = tangent
				// .w = lod fade hint
				//MF_ASSERT( data.m_pTangents );
				//float afTexCoords[ ] = 
				//{ 
				//	data.m_pTangents[i * 3 + 0],
				//	data.m_pTangents[i * 3 + 1],
				//	data.m_pTangents[i * 3 + 2]
				//	//,data.m_pLodFadeHints[i]
				//};
				//	cBuffer.TexCoord4v(3, afTexCoords);
		
				vert.binormal_.x = +data.m_pBinormals[i * 3 + 0];
				vert.binormal_.y = +data.m_pBinormals[i * 3 + 1];
				vert.binormal_.z = -data.m_pBinormals[i * 3 + 2];

				vert.tangent_.x  = +data.m_pTangents[i * 3 + 0];
				vert.tangent_.y  = +data.m_pTangents[i * 3 + 1];
				vert.tangent_.z  = -data.m_pTangents[i * 3 + 2];
			#endif // SPT_ENABLE_NORMAL_MAPS

			const float* pDiffuseCoords = data.m_pTexCoords[CSpeedTreeRT::TL_DIFFUSE] + i * 2;
			vert.texCoords_[0] = pDiffuseCoords[0];
			vert.texCoords_[1] = pDiffuseCoords[1];

			// wind animation
			float fWindMatrixIndex1 = float( int( data.m_pWindMatrixIndices[0][i] * 10.0f / speedWind.GetNumWindMatrices() ) );
			float fWindMatrixWeight1 = c_fWindWeightCompressionRange * data.m_pWindWeights[0][i];
			float fWindMatrixIndex2 = float( int( data.m_pWindMatrixIndices[1][i] * 10.0f / speedWind.GetNumWindMatrices( ) ) );
			float fWindMatrixWeight2 = c_fWindWeightCompressionRange * data.m_pWindWeights[1][i];

			vert.windIndex_ = fWindMatrixIndex1 + fWindMatrixWeight1;
			vert.windWeight_ = fWindMatrixIndex2 + fWindMatrixWeight2;

			TreePartTraits::s_vertexList_.push_back( vert );

		}

        // setup indices
		int nNumLods = data.m_nNumLods;
        if ( nNumLods > 0 )
        {
			bool lodEnded = false;
            for ( int nLod = 0; nLod < nNumLods; ++nLod )
            {
				// add new lod level
				typedef TreePartTraits::RenderDataType::LodData LodData;
				result.lod_.push_back( LodData() );
				LodData & lodData = result.lod_.back();
				lodData.index_ = NULL;

				MF_ASSERT( data.m_pNumStrips );
				int stripsCount = data.m_pNumStrips[nLod];
				int indexCount = 0;
				for ( int strip=0; strip<stripsCount; ++strip )
				{
					indexCount += data.m_pStripLengths[nLod][strip];
				}
				for ( int strip=0; strip<stripsCount; ++strip )
				{
					if (strip==0)
						lodData.index_ = TreePartTraits::s_indexList_.newSlot( indexCount );
					else
					{
						TreePartTraits::s_indexList_.push_back( TreePartTraits::s_indexList_.back() );
						TreePartTraits::s_indexList_.push_back( data.m_pStrips[nLod][strip][0] );
					}

					int nNumVertsInStrip = data.m_pStripLengths[nLod][strip];
					for ( int i = 0; i < nNumVertsInStrip; ++i )
					{
						TreePartTraits::s_indexList_.push_back( data.m_pStrips[nLod][strip][i] );
					}
				}
            }
        }
	}
#else
	// create and fill the index buffer for each LOD
	for ( int lod=0; lod<data.m_nNumLods; ++lod )
	{
		// add new lod level
		typedef TreePartTraits::RenderDataType::LodData LodData;
		result.lod_.push_back( LodData() );
		LodData & lodData = result.lod_.back();

		typedef VertexBufferWrapper< TreePartTraits::VertexType > VBufferType;
		VBufferType * vertexBuffer = vertexBuffer = new VBufferType;

		// create index buffer for lod level
		int stripsCount = data.m_pNumStrips[lod];
		for ( int strip=0; strip<stripsCount; ++strip )
		{
			int vCount = data.m_nNumVertices;

			lodData.strips_.push_back( new IndexBufferWrapper );
			int iCount = data.m_pStripLengths[lod][strip];
			if ( iCount > 0 )
			{
				const int * iData = data.m_pStrips[lod][strip];
				bool result;
				if ( TreePartTraits::isSingleFaced() )
				{
					result = 
						lodData.strips_.back()->reset( iCount ) &&
						lodData.strips_.back()->copy( iData );
				}
				else
				{
					// if double sided, copy indices 
					// twice into ibuffer. Second copy runs
					// backwards. To not repeat last index.
					int * zero = new int[iCount*2-1];

					int i = 0;
					for ( ; i<iCount-1; ++i )
					{
						zero[i] = iData[i];
						zero[iCount+i] = iData[iCount-2-i] + vCount;
					}
					zero[i] = iData[i];
					result = 
						lodData.strips_.back()->reset( iCount*2-1 );
						lodData.strips_.back()->copy( zero );

					delete [] zero;
				}
				if ( !result )
				{
					ERROR_MSG(
						"createPartRenderData: "
						"Could not create/copy index buffer\n");

					// abort this lod
					continue;
				}
			}

			// create vertex buffer for lod level
			if ( lod == 0 )
			{
				if ( vCount > 0 )
				{
					// if this should be double sided, copy vertices
					// twice into vbuffer (and flip normals of second copy)
					int facesCount = TreePartTraits::isSingleFaced() ? 1 : 2;
					if ( vertexBuffer->reset( vCount*facesCount ) &&
						vertexBuffer->lock() )
					{
						VBufferType::iterator vbIt  = vertexBuffer->begin();
						VBufferType::iterator vbEnd = vertexBuffer->end();
						while ( vbIt != vbEnd )
						{
							int distance = std::distance( vbIt, vbEnd );
							int i = (vCount * facesCount - distance) % vCount;

							// position (in speedtree, z is up)
							vbIt->position_.x = +data.m_pCoords[i * 3 + 0];
							vbIt->position_.y = +data.m_pCoords[i * 3 + 1];
							vbIt->position_.z = -data.m_pCoords[i * 3 + 2];

							// TODO: remove normal flipping (no longer in use)
							// normals (flip normals for second copy of vbuffer)
							float flip = (distance/vCount == 0) ? +1.0f : -1.0f;
							vbIt->normal_.x   = +data.m_pNormals[i * 3 + 0] * flip;
							vbIt->normal_.y   = +data.m_pNormals[i * 3 + 1] * flip;
							vbIt->normal_.z   = -data.m_pNormals[i * 3 + 2] * flip;

							#if SPT_ENABLE_NORMAL_MAPS
								vbIt->binormal_.x = +data.m_pBinormals[i * 3 + 0] * flip;
								vbIt->binormal_.y = +data.m_pBinormals[i * 3 + 1] * flip;
								vbIt->binormal_.z = -data.m_pBinormals[i * 3 + 2] * flip;

								vbIt->tangent_.x  = +data.m_pTangents[i * 3 + 0] * flip;
								vbIt->tangent_.y  = +data.m_pTangents[i * 3 + 1] * flip;
								vbIt->tangent_.z  = -data.m_pTangents[i * 3 + 2] * flip;
							#endif // SPT_ENABLE_NORMAL_MAPS

							// texcoords
							vbIt->texCoords_[0] = data.m_pTexCoords[CSpeedTreeRT::TL_DIFFUSE][i * 2 + 0];
							vbIt->texCoords_[1] = data.m_pTexCoords[CSpeedTreeRT::TL_DIFFUSE][i * 2 + 1];

							// wind animation
							float fWindMatrixIndex1 = float( int( data.m_pWindMatrixIndices[0][i] * 10.0f / speedWind.GetNumWindMatrices() ) );
							float fWindMatrixWeight1 = c_fWindWeightCompressionRange * data.m_pWindWeights[0][i];
							float fWindMatrixIndex2 = float( int( data.m_pWindMatrixIndices[1][i] * 10.0f / speedWind.GetNumWindMatrices( ) ) );
							float fWindMatrixWeight2 = c_fWindWeightCompressionRange * data.m_pWindWeights[1][i];

							vbIt->windIndex_ = fWindMatrixIndex1 + fWindMatrixWeight1;
							vbIt->windWeight_ = fWindMatrixIndex2 + fWindMatrixWeight2;

							++vbIt;
						}
						vertexBuffer->unlock();
					}
					else
					{
						ERROR_MSG(
							"createPartRenderData: "
							"Could not create/lock vertex buffer\n");
					}
				}
			}
			MF_ASSERT(vertexBuffer != NULL);
		}
		lodData.vertices_ = vertexBuffer;
	}
#endif // OPT_BUFFERS
	return result;
}

	
/**
 *	Creates a render data object to render the leafs part of a tree.
 *
 *	@param	speedTree			speedtree object where to get the data from.
 *  @param	speedWind			SpeedTree wind object to use for wind calculations.
 *	@param	filename			name of the file from where the tree was loaded.
 *	@param	datapath			path where to look for aditional data files.
 *	@param	windAnglesCount		number of wind angles used to animate trees.
 *	@param	leafRockScalar		(out) will store leaf's rock animation factor.
 *	@param	leafRustleScalar	(out) will store leaf's rustle animation factor.
 *
 *	@return						The filled up LeafRenderData object.
 */
LeafRenderData createLeafRenderData(
	CSpeedTreeRT      & speedTree,
	CSpeedWind	      & speedWind,
	const std::string & filename,
	const std::string & datapath,
	int                 windAnglesCount,
	float             & leafRockScalar,
	float             & leafRustleScalar );

/**
 *	Creates a render data object to render the billboard part of a tree.
 *
 *	@param	speedTree	speedtree object where to get the data from.
 *	@param	datapath	path where to look for aditional data files.
 */
speedtree::TreePartRenderData< speedtree::BillboardVertex > createBillboardRenderData(
	CSpeedTreeRT      & speedTree,
	const std::string & datapath );

} //namespace speedtree

#endif // SPEEDTREE_RENDERER_UTIL_HPP_