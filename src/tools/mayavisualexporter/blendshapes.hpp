/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef __blendshapes_hpp__
#define __blendshapes_hpp__

class BlendShapes
{
public:
	BlendShapes();
	virtual ~BlendShapes();

	struct Object
	{
		std::string name;
		std::vector< vector3<float> > vertices;
		std::vector< float > weights; // one for each frame, sequential
	};
	
	typedef std::vector< Object > Objects;

	uint32 count();

	bool hasBlendShape( MObject& object );

	// implementation is inefficient, may need to be optimised (initialises each blend shape in turn)
	// NOTE: leaves the blend shape object in a finalised state
	bool isBlendShapeTarget( MObject& object );
	
	// initialise blend shape index
	bool initialise( uint32 index );

	// initialise blend shape for MObject, returns false if no Blendshape on object
	bool initialise( MObject& object );

	// free up any memory used by the blend shapes
	void finalise();

	// name of the current blend shape
	std::string name();

	uint32 numBases();
	uint32 numTargets( uint32 base );
	
	uint32 countTargets();

	// get a reference to the base vertex positions
	Object& base( uint32 base );
	
	// get a reference to a target vertex positions
	Object& target( uint32 base, uint32 index );
	
	// returns a delta for vertex index between the base and target
	vector3<float> delta( uint32 base, uint32 target, uint32 index );

	// required to capture original mesh targets
	static void disable();
	
	// enable should be called when finished with blend shapes
	static void enable();

protected:
	std::vector<MObject> _blendShapes;

	// name of the currently initialised blend shape
	std::string _name;

	// first is base index, then the vertices
	Objects _bases;

	// first is base index, then target index, the vertices
	std::vector< Objects > _targets;
 };

#endif // __blendshapes_hpp__