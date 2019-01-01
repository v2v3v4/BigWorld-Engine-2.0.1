/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DX_ENUM_HPP__
#define DX_ENUM_HPP__


/******************************************************************************
	The Enum Type Support for Material ( fx files )
	The relation between name and value is 1:1, i.e. no 2 names will have same values
	This is because we will save the result as value and look them up in the name table
	when loaded again
******************************************************************************/
class DXEnum : public Singleton<DXEnum>
{
public:
	typedef int EnumType;
private:
	class EnumEntry
	{
		std::map<std::string, EnumType> nameToValueMap_;
		std::map<EnumType, std::string> valueToNameMap_;
	public:
		EnumEntry(){}
		EnumEntry( DataSectionPtr dataSection );
		typedef std::map<std::string, EnumType>::size_type size_type;
		size_type size() const;
		const std::string& entry( size_type index ) const;
		EnumType value( const std::string& name ) const;
		const std::string& name( EnumType value ) const;
	};

	std::map<std::string, EnumEntry> enums_;
	std::map<std::string, std::string> alias_;
	const std::string& getRealType( const std::string& typeName ) const;
public:
	typedef EnumEntry::size_type size_type;
	DXEnum( const std::string& resFileName );
	bool isEnum( const std::string& typeName ) const;
	size_type size( const std::string& typeName ) const;
	const std::string& entry( const std::string& typeName, size_type index ) const;
	EnumType value( const std::string& typeName, const std::string& name ) const;
	const std::string& name( const std::string& typeName, EnumType value ) const;
};

#endif//DX_ENUM_HPP__
