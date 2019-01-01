/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MODEL_COMPOUND_HPP
#define MODEL_COMPOUND_HPP


#include "cstdmf/stringmap.hpp"


namespace Moo
{
class VisualCompound;
class TransformHolder;
}

class ModelCompound;
typedef SmartPointer<ModelCompound> ModelCompoundPtr;

/**
 *	This class contains a model compound instance.
 *	It is used to render an instance of a simple model
 *	handled through the model compound.
 */
class ModelCompoundInstance : public ReferenceCount
{
public:
	ModelCompoundInstance();
	~ModelCompoundInstance();
	bool init( ModelCompoundPtr pModelCompound, 
		const Matrix& transform, uint32 batchCookie );

	bool draw();

private:
	float lodDistance();

	typedef std::pair< float, Moo::TransformHolder* > LodHolder;
	typedef std::vector< LodHolder > LodHolders;

	ModelCompoundPtr pModelCompound_;

	LodHolders	lodHolders_;
	Vector3		position_;
	float		yScale_;
	
	ModelCompoundInstance( const ModelCompoundInstance& );
	ModelCompoundInstance& operator=( const ModelCompoundInstance& );

	friend ModelCompound;
};

typedef SmartPointer<ModelCompoundInstance> ModelCompoundInstancePtr;

/**
 *	This class manages the ModelCompounds and instances,
 *	a modelcompound uses the visual compound combined with LOD
 *	values loaded from a model file to render simple models
 *	more efficiently.
 */
class ModelCompound : public SafeReferenceCount
{
public:
	~ModelCompound();

	void	add( ModelCompoundInstance* instance );
	void	del( ModelCompoundInstance* instance );

	static ModelCompoundInstancePtr get( const std::string& name, 
		const Matrix& transform, uint32 batchCookie );

	typedef std::pair< float, std::string > LodCompound;
	typedef std::vector<LodCompound> LodCompounds;

	const	LodCompounds& lodCompounds() { return lodCompounds_; }
	
	void	invalidate();

	bool	valid() { return valid_; }

private:
	ModelCompound();
	bool init( const std::string& modelName );

	LodCompounds	lodCompounds_;

	SimpleMutex		mutex_;

	bool	valid_;
	
	typedef std::vector<ModelCompoundInstance*> Instances;
	Instances instances_;

	typedef StringHashMap<ModelCompoundPtr> CompoundMap;
	static CompoundMap	compoundMap_;
	static SimpleMutex	compoundsMutex_;	


	ModelCompound( const ModelCompound& );
	ModelCompound& operator=( const ModelCompound& );
};


#endif // MODEL_COMPOUND_HPP
