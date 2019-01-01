/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef AUTO_CONFIG_HPP
#define AUTO_CONFIG_HPP

#include <vector>
#include "datasection.hpp"
#include "bwresource.hpp"


/**
 *	This class is an object that automatically configures itself from
 *	a global configuration file.
 */
class AutoConfig
{
public:
	static const std::string s_resourcesXML;

	AutoConfig();
	virtual ~AutoConfig();

	virtual void configureSelf( DataSectionPtr pConfigSection ) = 0;

	static void configureAllFrom( DataSectionPtr pConfigSection );
	static bool configureAllFrom( std::vector<DataSectionPtr>& pConfigSections );
	static bool configureAllFrom( const std::string& xmlResourceName );

private:
	AutoConfig( const AutoConfig& );
	AutoConfig& operator=( const AutoConfig& );

	typedef std::vector<AutoConfig*> AutoConfigs;
	static AutoConfigs * s_all;
};


/**
 *	This class is a templatised auto configurated value.
 */
template< typename T >
class BasicAutoConfig : public AutoConfig
{
public:
	BasicAutoConfig( const char * path, const T & deft = Datatype::DefaultValue<T>::val() ) :
		AutoConfig(),
		path_( path ),
		value_( deft )
	{}

	virtual void configureSelf( DataSectionPtr pConfigSection )
	{
		DataSectionPtr pValue = pConfigSection->openSection( path_ );
		if (pValue)
			value_ = pValue->as< T >();
	}

	operator const T &() const	{ return value_; }
	const T & value() const		{ return value_; }

private:
	const char *	path_;
	T				value_;
};


/**
 *	Specialisation of BasicAutoConfig for std::string.
 */
template<>
class BasicAutoConfig< std::string > : public AutoConfig
{
public:
	BasicAutoConfig( const char * path, const std::string & deft = "" );
	virtual ~BasicAutoConfig();

	virtual void configureSelf( DataSectionPtr pConfigSection );

	operator const std::string &() const	{ return value_; }
	const std::string & value() const		{ return value_; }

private:
	const char *	path_;
	std::string		value_;
};


typedef BasicAutoConfig< std::string > AutoConfigString;


/**
 *	This class is a vector of strings that automatically configures themselves.
 */
class AutoConfigStrings : public AutoConfig
{
public:
	AutoConfigStrings( const char * path, const char * tag );	

	virtual void configureSelf( DataSectionPtr pConfigSection );
	
	typedef std::vector<std::string> Vector;

	operator const Vector &() const	{ return value_; }
	const Vector & value() const	{ return value_; }

private:
	const char *	path_;
	const char *	tag_;
	Vector			value_;
};


#endif // AUTO_CONFIG_HPP
