/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MRU_HPP
#define MRU_HPP

class MRU
{
public:
	
	MRU();

	static MRU& instance();

	void update( const std::string& mruName, const std::string& file, bool add = true );
	void read( const std::string& mruName, std::vector<std::string>& files );
	void getDir( const std::string& mruName, std::string& dir, const std::string& defaultDir = "" );

private:

	unsigned maxMRUs_;
};

#endif // MRU_HPP