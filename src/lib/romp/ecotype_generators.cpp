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
#include "ecotype_generators.hpp"
#include "flora.hpp"
#include "flora_renderer.hpp"
#include "flora_texture.hpp"
#include "cstdmf/debug.hpp"
#include "moo/visual_manager.hpp"

//class factory methods
typedef EcotypeGenerator* (*EcotypeGeneratorCreator)();
typedef std::map< std::string, EcotypeGeneratorCreator > CreatorFns;
CreatorFns g_creatorFns;

#define ECOTYPE_FACTORY_DECLARE( b ) EcotypeGenerator* b##Generator() \
	{															 \
		return new b##Ecotype;									 \
	}

#define REGISTER_ECOTYPE_FACTORY( a, b ) g_creatorFns[a] = b##Generator;


typedef ChooseMaxEcotype::Function* (*FunctionCreator)();
typedef std::map< std::string, FunctionCreator > FunctionCreators;
FunctionCreators g_fnCreators;
#define FUNCTION_FACTORY_DECLARE( b ) ChooseMaxEcotype::Function* b##Creator() \
	{															 \
		return new b##Function;									 \
	}

#define REGISTER_FUNCTION_FACTORY( a, b ) g_fnCreators[a] = b##Creator;


DECLARE_DEBUG_COMPONENT2( "Flora", 0 )

/**
 *	This method creates and initialises an EcotypeGenerator given
 *	a data section.  The legacy xml grammar of a list of &lt;visual>
 *	sections is maintained; also the new grammar of a &lt;generator>
 *	section is introduced, using the g_creatorFns factory map.
 *
 *	@param pSection	Section containing generator data.
 *	@param target	Ecotype to store the texture resource used by
 *					the generator, or empty string ("").
 */
EcotypeGenerator* EcotypeGenerator::create( DataSectionPtr pSection,
	Ecotype& target )
{	
	EcotypeGenerator* eg = NULL;
	DataSectionPtr pInitSection = NULL;

	if (pSection)
	{
		//legacy support - visuals directly in the section
		if ( pSection->findChild("visual") )
		{			
			eg = new VisualsEcotype();
			pInitSection = pSection;			
		}
		else if ( pSection->findChild("generator") )
		{			
			DataSectionPtr pGen = pSection->findChild("generator");
			const std::string& name = pGen->sectionName();

			CreatorFns::iterator it = g_creatorFns.find(pGen->asString());
			if ( it != g_creatorFns.end() )
			{
				EcotypeGeneratorCreator egc = it->second;
				eg = egc();
				pInitSection = pGen;
			}
			else
			{
				ERROR_MSG( "Unknown ecotype generator %s\n", name.c_str() );
			}
		}

		if (eg)
		{
			if (eg->load(pInitSection,target) )
			{
				return eg;
			}
			else
			{
				delete eg;
				eg = NULL;
			}
		}
	}

    target.textureResource("");
	target.pTexture(NULL);
	return new EmptyEcotype();
}


/**
 *	This method implements the transformIntoVB interface for
 *	EmptyEcotype.  It simply NULLs out the vertices.
 */
uint32 EmptyEcotype::generate(		
		const Vector2& uvOffset,
		FloraVertexContainer* pVerts,
		uint32 idx,
		uint32 maxVerts,
		const Matrix& objectToWorld,
		const Matrix& objectToChunk,
		BoundingBox& bb )
{
	uint32 nVerts = min(maxVerts, (uint32)12);
	if (pVerts)
		pVerts->clear( nVerts );
	return nVerts;
}


VisualsEcotype::VisualsEcotype():
	density_( 1.f )
{	
}


/**
 *	This method implements the EcotypeGenerator::load interface
 *	for the VisualsEcotype class.
 */
bool VisualsEcotype::load( DataSectionPtr pSection, Ecotype& target)
{
	flora_ = target.flora();

	density_ = pSection->readFloat( "density", 1.f );

	target.textureResource("");
	target.pTexture(NULL);
	std::vector<DataSectionPtr>	pSections;
	pSection->openSections( "visual", pSections );

	for ( uint i=0; i<pSections.size(); i++ )
	{
		DataSectionPtr curr = pSections[i];

		Moo::VisualPtr visual;
		std::string visualResID = curr->asString();
		if (BWResource::openSection( visualResID ))
		{
			visual = new Moo::Visual( visualResID, true, Moo::Visual::LOAD_GEOMETRY_ONLY );
			visual->isInVisualManager( false );
		}

		if ( visual )
		{
			float flex = curr->readFloat( "flex", 1.f );
			float scaleVariation = curr->readFloat( "scaleVariation", 0.f );
			visuals_.push_back( VisualCopy() );
			bool ok = findTextureResource( pSections[i]->asString(), target );
			if (ok) ok = visuals_.back().set( visual, flex, scaleVariation, flora_ );
			if (!ok)
			{
				DEBUG_MSG( "VisualsEcotype - Removing %s because VisualCopy::set failed\n", 
					pSections[i]->asString().c_str() );
				visuals_.pop_back();
			}
		}
		else
		{
			DEBUG_MSG( "VisualsEcotype - %s does not exist\n", 
					pSections[i]->asString().c_str() );
		}
	}

	return (visuals_.size() != 0);
}


/**
 *	This method takes a visual, and creates a triangle list of FloraVertices.
 */
bool VisualsEcotype::VisualCopy::set( Moo::VisualPtr pVisual, float flex, float scaleVariation, Flora* flora )
{
	Moo::VertexXYZNUV * verts;
	uint32 nVerts;
	Moo::IndicesHolder	 indices;
	uint32 nIndices;
	Moo::EffectMaterialPtr  material;

	scaleVariation_ = scaleVariation;

	vertices_.clear();
	if ( pVisual->createCopy( verts, indices, nVerts, nIndices, material ) )
	{
		vertices_.resize( nIndices );

		//preprocess uvs
		BoundingBox bb;
		bb.setBounds(	Vector3(0.f,0.f,0.f),
						Vector3(0.f,0.f,0.f) );

		float numBlocksWide = float(flora->floraTexture()->blocksWide());
		float numBlocksHigh = float(flora->floraTexture()->blocksHigh());
		for ( uint32 v=0; v<nVerts; v++ )
		{
			verts[v].uv_.x /= numBlocksWide;
			verts[v].uv_.y /= numBlocksHigh;
			bb.addBounds( verts[v].pos_ );
		}

		float totalHeight = bb.maxBounds().y - bb.minBounds().y;

		//copy the mesh
		for ( uint32 i=0; i<nIndices; i++ )
		{
			vertices_[i].set( verts[indices[i]] );
			//float relativeHeight = (vertices_[i].pos_.y - bb.minBounds().y) / totalHeight;
			float relativeHeight = (verts[indices[i]].pos_.y - bb.minBounds().y) / 2.f;
			if ( relativeHeight>1.f )
				relativeHeight=1.f;
			vertices_[i].flex( relativeHeight > 0.1f ? flex * relativeHeight : 0.f );
		}

		delete [] verts;

		return true;
	}

	return false;
}


/**
 *	This method extracts the first texture property from the given visual.
 */
bool VisualsEcotype::findTextureResource(const std::string& resourceID,Ecotype& target)
{
	DataSectionPtr pMaterial = NULL;

	DataSectionPtr pSection = BWResource::openSection(resourceID);
	if ( pSection )
	{
		DataSectionPtr pRenderset = pSection->openSection("renderSet");
		if (pRenderset)
		{
			DataSectionPtr pGeometry = pRenderset->openSection("geometry");
			if (pGeometry)
			{
				DataSectionPtr pPrimitiveGroup = pGeometry->openSection("primitiveGroup");
				if (pPrimitiveGroup)
				{
					pMaterial = pPrimitiveGroup->openSection("material");
				}
			}
		}
	}

	if (!pMaterial)
	{
		ERROR_MSG( "No material in flora visual %s\n", resourceID.c_str() );
		return false;
	}

	std::string mfmName = pMaterial->readString( "mfm" );
	if ( mfmName != "" )
	{
		pMaterial = BWResource::openSection( mfmName );
		if ( !pMaterial )
		{
			ERROR_MSG( "MFM (%s) referred to in visual (%s) does not exist\n", mfmName.c_str(), resourceID.c_str() );
			return false;
		}
	}

	//First see if we've got a material defined in the visual file itself
	std::vector<DataSectionPtr> pProperties;
	pMaterial->openSections("property",pProperties);
	std::vector<DataSectionPtr>::iterator it = pProperties.begin();
	std::vector<DataSectionPtr>::iterator end = pProperties.end();
	while (it != end)
	{
		DataSectionPtr pProperty = *it++;
		DataSectionPtr pTexture = pProperty->findChild( "Texture" );
		if (pTexture)
		{
			std::string texName = pProperty->readString("Texture");
			target.textureResource( texName );
			if (texName != "")
			{
				Moo::BaseTexturePtr pMooTexture =
					Moo::TextureManager::instance()->getSystemMemoryTexture( texName );
				target.pTexture( pMooTexture );
			}
			return true;
		}
	}

	target.textureResource( "" );
	target.pTexture( NULL );
	ERROR_MSG( "Could not find a texture property in the flora visual %s\n", resourceID.c_str() );
	return false;
}


/**
 *	This method fills the vertex buffer with a single object, and returns the
 *	number of vertices used.
 */
uint32 VisualsEcotype::generate(	
	const Vector2& uvOffset,
	FloraVertexContainer* pVerts,
	uint32 idx,
	uint32 maxVerts,
	const Matrix& objectToWorld,
	const Matrix& objectToChunk,
	BoundingBox& bb )
{
	if ( visuals_.size() )
	{			
		VisualCopy& visual = visuals_[idx%visuals_.size()];

		uint32 nVerts = visual.vertices_.size();		

		if ( nVerts > maxVerts)
			return 0;

		//nextRandomFloat are here to keep determinism.
		float rDensity = flora_->nextRandomFloat();
		float rScale = flora_->nextRandomFloat();

		if ( rDensity < density_ )
		{
			if (pVerts)
			{
				Matrix scaledObjectToChunk;
				Matrix scaledObjectToWorld;

				float scale = fabsf(rScale) * visual.scaleVariation_ + 1.f;
				scaledObjectToChunk.setScale( scale, scale, scale );
				scaledObjectToChunk.postMultiply( objectToChunk );
				scaledObjectToWorld.setScale( scale, scale, scale );
				scaledObjectToWorld.postMultiply( objectToWorld );

				//calculate animation block number for this object
				int blockX = abs((int)floorf(objectToWorld.applyToOrigin().x / BLOCK_WIDTH));
				int blockZ = abs((int)floorf(objectToWorld.applyToOrigin().y / BLOCK_WIDTH));
				int blockNum = (blockX%7 + (blockZ%7) * 7);
			
				pVerts->uvOffset( uvOffset.x, uvOffset.y );
				pVerts->blockNum( blockNum );
				pVerts->addVertices( &*visual.vertices_.begin(), nVerts, &scaledObjectToChunk );			

				for ( uint32 i=0; i<nVerts; i++ )
				{				
					bb.addBounds( scaledObjectToWorld.applyPoint( visual.vertices_[i].pos_ ) );
				}
			}

			return nVerts;			
		}
		else
		{
			if (pVerts)
			{
				pVerts->clear( nVerts );
			}
			return nVerts;
		}
	}

	return 0;
}


/**
 *	This method returns true if the ecotype is empty.  Being empty means
 *	it will not draw any detail objects, and neighbouring ecotypes will not
 *	encroach.
 */
bool VisualsEcotype::isEmpty() const
{
	return (visuals_.size() == 0);
}


/**
 *	This class implements the ChooseMaxEcotype::Function interface,
 *	and provides values at any geographical location based on a
 *	2-dimensions perlin noise function.  The frequency of the noise
 *	is specified in the xml file and set during the load() method.
 */
class NoiseFunction : public ChooseMaxEcotype::Function
{
public:
	void load( DataSectionPtr pSection )
	{
		frequency_ = pSection->readFloat( "frequency", 10.f );
	}

	float operator() (const Vector2& input)
	{
		float value = noise_.sumHarmonics2( input, 0.75f, frequency_, 3.f );		
		return value;
	}

private:
	PerlinNoise	noise_;
	float		frequency_;
};


/**
 *	This class implements the ChooseMaxEcotype::Function interface,
 *	and simply provides a random value for any given geographical
 *	location.
 */
class RandomFunction : public ChooseMaxEcotype::Function
{
public:
	void load( DataSectionPtr pSection )
	{		
	}

	float operator() (const Vector2& input)
	{
		//TODO : does this break the deterministic nature of flora?
		return float(rand()) / (float)RAND_MAX;
	}

private:		
};


/**
 *	This class implements the ChooseMaxEcotype::Function interface,
 *	and provides a fixed value at any geographical location.  The
 *	value is specified in the xml file, and set during the load() method.
 */
class FixedFunction : public ChooseMaxEcotype::Function
{
public:
	void load( DataSectionPtr pSection )
	{
		value_ = pSection->readFloat( "value", 0.5f );
	}

	float operator() (const Vector2& input)
	{
		return value_;
	}

private:
	float value_;
};


/**
 *	This is the ChooseMaxEcotype destructor.
 */
ChooseMaxEcotype::~ChooseMaxEcotype()
{
	for (SubTypes::iterator it = subTypes_.begin(); it != subTypes_.end(); ++it)
	{
		delete it->first;
		delete it->second;
	}
}


/**
 *	This static method creates a ChooseMaxEcotype::Function object based on the
 *	settings in the provided data section.  It sources creation functions from
 *	the g_fnCreators map that contains all noise generators registered at
 *	startup.
 */
ChooseMaxEcotype::Function* ChooseMaxEcotype::createFunction( DataSectionPtr pSection )
{
	const std::string& name = pSection->sectionName();	
	FunctionCreators::iterator it = g_fnCreators.find(name);
	if ( it != g_fnCreators.end() )
	{
		FunctionCreator fc = it->second;
		ChooseMaxEcotype::Function*fn = fc();
		fn->load(pSection);
		return fn;
	}
	else
	{
		ERROR_MSG( "Unknown ChooseMaxEcotype::Function %s\n", name.c_str() );
	}
	return NULL;
}


/**
 *	This method initialises the ChooseMaxEcotype generator given the datasection
 *	that is passed in.  The texture resource used by the generator is returned.
 *
 *	@param pSection	section containing generator data
 *	@param target	Ecotype to contain the texture resource used by
 *	                the generator, or empty string ("").
 *
 *  @returns True on success, false on error.
 */
bool ChooseMaxEcotype::load( DataSectionPtr pSection, Ecotype& target )
{
	target.textureResource( "" );
	target.pTexture( NULL );

	DataSectionIterator it = pSection->begin();
	DataSectionIterator end = pSection->end();

	while (it != end)
	{
		DataSectionPtr pFunction = *it++;
		Function* fn = ChooseMaxEcotype::createFunction(pFunction);		

		if (fn)
		{			
			std::string oldTextureResource = target.textureResource();
			Moo::BaseTexturePtr pTexture = target.pTexture();

			//NOTE we pass in the parent section ( pFunction ) because of legacy support
			//for a simple list of <visual> sections.  If not for legacy support, we'd
			//open up the 'generator' section and pass that into the ::create method.
			EcotypeGenerator* ecotype = EcotypeGenerator::create(pFunction,target);
			if (oldTextureResource != "")
			{
				if (target.textureResource() != "" && target.textureResource() != oldTextureResource)
				{
					ERROR_MSG( "All ecotypes within a choose_max section must reference the same texture\n" );					
				}
				target.textureResource( oldTextureResource );
				target.pTexture( pTexture );
			}			

			subTypes_.insert( std::make_pair(fn,ecotype) );			
		}
	}	

	return true;
}


/** 
 * This method implements the transformIntoVB interface for the
 *	ChooseMaxEcotype generator.  It chooses the best ecotype
 *	for the given geographical location, and delegates the vertex
 *	creation duties to the chosen generator.
 */
uint32 ChooseMaxEcotype::generate(
		const Vector2& uvOffset,
		class FloraVertexContainer* pVerts,
		uint32 idx,
		uint32 maxVerts,
		const Matrix& objectToWorld,
		const Matrix& objectToChunk,
		BoundingBox& bb )
{
	Vector2 pos2( objectToWorld.applyToOrigin().x, objectToWorld.applyToOrigin().z );
	EcotypeGenerator* eg = chooseGenerator(pos2);
	if (eg)	
		return eg->generate(uvOffset, pVerts, idx, maxVerts, objectToWorld, objectToChunk, bb);	
	return 0;
}


/** 
 *	This method chooses the best ecotype generator given the current
 *	position.  It does this by asking for the probability of all
 *	generators at the given position, and returning the maximum.
 *
 *	@param pos input position to seed the generators' probability functions.
 */
EcotypeGenerator* ChooseMaxEcotype::chooseGenerator(const Vector2& pos)
{
	EcotypeGenerator* chosen = NULL;
	float best = -0.1f;

	SubTypes::iterator it = subTypes_.begin();
	SubTypes::iterator end = subTypes_.end();

	while (it != end)
	{
		Function& fn = *it->first;		
		float curr = fn(pos);
		if (curr>best)
		{
			best = curr;
			chosen = it->second;
		}
		it++;
	}

	return chosen;
}

/**
 *	This method returns true if the ecotype is empty.  Being empty means
 *	it will not draw any detail objects, and neighbouring ecotypes will not
 *	encroach.
 */
bool ChooseMaxEcotype::isEmpty() const
{
	return false;
}

ECOTYPE_FACTORY_DECLARE( Empty );
ECOTYPE_FACTORY_DECLARE( Visuals );
ECOTYPE_FACTORY_DECLARE( ChooseMax );

FUNCTION_FACTORY_DECLARE( Noise );
FUNCTION_FACTORY_DECLARE( Random );
FUNCTION_FACTORY_DECLARE( Fixed );

bool registerFactories()
{
	REGISTER_ECOTYPE_FACTORY( "empty", Empty );
	REGISTER_ECOTYPE_FACTORY( "visual", Visuals );
	REGISTER_ECOTYPE_FACTORY( "chooseMax", ChooseMax );

	REGISTER_FUNCTION_FACTORY( "noise", Noise );
	REGISTER_FUNCTION_FACTORY( "random", Random );
	REGISTER_FUNCTION_FACTORY( "fixed", Fixed );
	return true;
};
bool alwaysSuccessful = registerFactories();
