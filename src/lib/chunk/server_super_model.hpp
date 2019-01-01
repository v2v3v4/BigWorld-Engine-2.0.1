/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SERVER_SUPER_MODEL_HPP

#include "cstdmf/smartpointer.hpp"

#include "resmgr/datasection.hpp"

#include <string>
#include <vector>

class BSPTree;
class BoundingBox;

typedef SmartPointer< class Model > ModelPtr;

/**
 *	This class is the base class for Models implemented for the server.
 */
class Model : public SafeReferenceCount
{
public:
	static ModelPtr get( const std::string & resourceID );

	virtual bool valid() const = 0;
	virtual const BSPTree * decompose() const	{ return 0; }
	virtual const BoundingBox & boundingBox() const = 0;

protected:
	static Model * load( const std::string & resourceID, DataSectionPtr pFile );
};


/**
 *	This class implements the SuperModel for the server. It is basically just a
 *	list of Models.
 */
class SuperModel
{
public:
	SuperModel( const std::vector< std::string > & modelIDs );

	int nModels() const						{ return models_.size(); }

	ModelPtr topModel( int i )				{ return models_[i]; }

	void boundingBox( BoundingBox& bb ) const;

private:
	std::vector< ModelPtr >	models_;
};

#endif // SERVER_SUPER_MODEL_HPP
