/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef __portal_hpp__
#define __portal_hpp__

class Portal
{
public:
	Portal();
	~Portal();

	// returns the number of portals
	uint32 count();

	// initialises Portal for portal "index" - access functions now use this
	// portal. this must be called before accessing vertex data
	bool initialise( uint32 index );

	// frees up memory used by the portal
	void finalise();

	MDagPathArray& portals();

	std::string name();
	std::string flags();
	std::string label();
	std::vector<Point3>& positions();

protected:
	// dag paths to portal
	MDagPathArray _portals;

	// geometry data
	std::string _name;		// name of the portal mesh
	std::string _flags;		// type of portal
	std::string _label;		// label of portal
	std::vector<Point3> _positions;
};

#endif // __portal_hpp__