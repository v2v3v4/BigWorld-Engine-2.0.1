/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ECOTYPE_GENERATORS_HPP
#define ECOTYPE_GENERATORS_HPP

#include "ecotype.hpp"
#include "flora_vertex_type.hpp"

/** 
 *	This interface describes a class that generates the
 *	vertex and texture data for an ecotype.
 */
class EcotypeGenerator
{
public:
	static EcotypeGenerator* create(DataSectionPtr pSection,
			Ecotype& target );
	virtual ~EcotypeGenerator() {};
	virtual bool load( DataSectionPtr pSection,
			Ecotype& target ) = 0;

	virtual uint32 generate(			
			const Vector2& uvOffset,
			class FloraVertexContainer* start,
			uint32 id,
			uint32 maxVerts,
			const Matrix& objectToWorld,
			const Matrix& objectToChunk,
			BoundingBox& bb ) = 0;	

	virtual bool isEmpty() const = 0;	
};


/** 
 *	This class implements the ecoytpe generator interface,
 *	and simply creates degenerate triangles so no flora is
 *	drawn.
 */
class EmptyEcotype : public EcotypeGenerator
{
	uint32 generate(		
		const Vector2& uvOffset,
		class FloraVertexContainer* pVerts,
		uint32 id,
		uint32 maxVerts,
		const Matrix& objectToWorld,
		const Matrix& objectToChunk,
		BoundingBox& bb );

	bool load( DataSectionPtr pSection, Ecotype& target )
	{
		target.textureResource("");
		target.pTexture(NULL);
		return true;
	}

	bool isEmpty() const
	{
		return true;
	}
};

/**
 *	This class implements the ecotype interface, using
 *	an array of visuals as source material.  The visuals
 *	are simple placed down on the terrain and oriented.
 */
class VisualsEcotype : public EcotypeGenerator
{
public:
	VisualsEcotype();
	bool load( DataSectionPtr pSection, Ecotype& target );
	uint32 generate(		
		const Vector2& uvOffset,
		class FloraVertexContainer* pVerts,
		uint32 id,
		uint32 maxVerts,
		const Matrix& objectToWorld,
		const Matrix& objectToChunk,
		BoundingBox& bb );
	bool isEmpty() const;	

protected:	
	bool findTextureResource(const std::string& resourceID, Ecotype& target);

	/**
	 *	This structure copies a visual into a triangle list of FloraVertices
	 */
	struct VisualCopy
	{
		bool set( Moo::VisualPtr, float flex, float scaleVariation, class Flora* flora );
		std::vector< FloraVertex > vertices_;
		float scaleVariation_;
	};

	typedef std::vector<VisualCopy>	VisualCopies;
	VisualCopies	visuals_;
	float			density_;
	class Flora*	flora_;
};

/**
 *	This class implements the ecotype interface, using
 *	a vector of Ecotypes (sub-types).  Rules are applied at
 *	any geographical location to determine which sub-type
 *	is chosen.
 */
class ChooseMaxEcotype : public EcotypeGenerator
{
public:	
	~ChooseMaxEcotype();
	bool load( DataSectionPtr pSection, Ecotype& target );
	uint32 generate(		
		const Vector2& uvOffset,
		class FloraVertexContainer* pVerts,
		uint32 id,
		uint32 maxVerts,
		const Matrix& objectToWorld,
		const Matrix& objectToChunk,
		BoundingBox& bb );	

	/**
	 * TODO: to be documented.
	 */
	class Function
	{
	public:
		virtual void load(DataSectionPtr pSection) = 0;
		virtual float operator() (const Vector2& input) = 0;
	};

	bool isEmpty() const;	

private:
	static Function* createFunction( DataSectionPtr pSection );
	EcotypeGenerator* chooseGenerator(const Vector2& position);
	typedef std::map< Function*, EcotypeGenerator* >	SubTypes;
	SubTypes	subTypes_;
};


#endif
