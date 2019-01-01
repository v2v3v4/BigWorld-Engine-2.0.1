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
#include "post_proc_property_editor.hpp"
#include "base_post_processing_node.hpp"


namespace
{
	/**
	 *	This local function splits "instr" into words separated by "separator"
	 *	and puts each word as an element of the return vector "retTokens".
	 *
	 *	@param instr	String with a list of items separated by "separator".
	 *	@param separator	Separator character.
	 *	@param retTokens	Return param, vector with the items.
	 */
	void split( const std::wstring & instr, const std::wstring & separator, 
				std::vector< std::wstring > & retTokens )
	{
		BW_GUARD;

		std::wstring::size_type start = 0;
		std::wstring::size_type end = 0;

		while ((end = instr.find( separator, start )) != std::wstring::npos)
		{
			retTokens.push_back( instr.substr( start, end - start ) );
			start = end + separator.length();
		}

		retTokens.push_back( instr.substr( start ) );
	}


	/**
	 *	This local function retrieves the list of filename extensions found in
	 *	a GeneralProperty file spec string.
	 *
	 *	@param filespec		GeneralProperty file spec string.
	 *	@param retExtensions	List of filename extensions found in filespec.
	 */
	void filespecExtensions( const std::wstring & filespec, 
							std::vector< std::wstring > & retExtensions )
	{
		BW_GUARD;

		if (filespec.empty())
		{
			return;
		}

		std::vector< std::wstring > specSections;
		split( filespec, L"|", specSections );

		if (specSections.size() >= 2)
		{
			std::vector< std::wstring > exts;
			split( specSections[ 1 ], L";", exts );

			for (std::vector< std::wstring >::iterator it = exts.begin();
				it != exts.end(); ++it)
			{
				std::wstring::size_type dotPos = (*it).rfind( L'.' );
				if (dotPos != std::wstring::npos)
				{
					retExtensions.push_back( (*it).substr( dotPos + 1 ) );
				}
			}
		}
	}


	/**
	 *	This local function returns whether or not "filespec" is a texture.
	 *
	 *	@param filespec		GeneralProperty file spec string.
	 *	@return Whether or not "filespec" is a texture.
	 */
	bool isTexture( const std::wstring & filespec )
	{
		BW_GUARD;

		bool ret = false;

		std::vector< std::wstring > exts;
		filespecExtensions( filespec, exts );

		for (std::vector< std::wstring >::iterator it = exts.begin();
			it != exts.end(); ++it)
		{
			if ((*it) == L"tga" ||
				(*it) == L"bmp" ||
				(*it) == L"dds" ||
				(*it) == L"jpg" ||
				(*it) == L"png" ||
				(*it) == L"texanim")
			{
				ret = true;
				break;
			}
		}

		return ret;
	}


	/**
	 *	This local function returns whether or not "filespec" is a render
	 *	target.
	 *
	 *	@param filespec		GeneralProperty file spec string.
	 *	@return Whether or not "filespec" is a render target.
	 */
	bool isRenderTarget( const std::wstring & filespec )
	{
		BW_GUARD;

		bool ret = false;

		std::vector< std::wstring > exts;
		filespecExtensions( filespec, exts );

		for (std::vector< std::wstring >::iterator it = exts.begin();
			it != exts.end(); ++it)
		{
			if ((*it) == L"rt")
			{
				ret = true;
				break;
			}
		}

		return ret;
	}


	/**
	 *	This local function returns whether or not "filespec" is a shader file.
	 *
	 *	@param filespec		GeneralProperty file spec string.
	 *	@return Whether or not "filespec" is a shader file.
	 */
	bool isShader( const std::wstring & filespec )
	{
		BW_GUARD;

		bool ret = false;

		std::vector< std::wstring > exts;
		filespecExtensions( filespec, exts );

		for (std::vector< std::wstring >::iterator it = exts.begin();
			it != exts.end(); ++it)
		{
			if ((*it) == L"fx")
			{
				ret = true;
				break;
			}
		}

		return ret;
	}

} // anonymous namespace


/**
 *	Constructor.
 *
 *	@param node	Post processing node this editor will be editing.
 */
PostProcPropertyEditor::PostProcPropertyEditor( BasePostProcessingNodePtr node ) :
	node_( node )
{
	BW_GUARD;

	node->edEdit( this );

	constructorOver_ = true;
}


/**
 *	This method returns the node's properties of a type in "types".
 *
 *	@param types	A combination of TEXTURES, RENDER_TARGETS and/or SHADERS.
 *	@param retProps	Return param, properties found of a type in "types".
 */
void PostProcPropertyEditor::getProperties( int types, std::vector< std::string > & retProps ) const
{
	BW_GUARD;

	for (PropList::const_iterator it = properties_.begin(); it != properties_.end(); ++it)
	{
		if (((types & TEXTURES) && isTexture( (*it)->fileFilter() )) ||
			((types & RENDER_TARGETS) && isRenderTarget( (*it)->fileFilter() )) ||
			((types & SHADERS) && isShader( (*it)->fileFilter() )))
		{
			retProps.push_back( (*it)->name() );
		}
	}
}


/**
 *	This method sets the value of a node's property.
 *
 *	@param name		Name of the property to change.
 *	@param value	New value for the property.
 */
void PostProcPropertyEditor::setProperty( const std::string & name, const std::string & value )
{
	BW_GUARD;

	for (PropList::iterator it = properties_.begin(); it != properties_.end(); ++it)
	{
		if ((*it)->name() == name)
		{
			PyObject * pyVal = NULL;
			if ((*it)->valueType().fromString( value, pyVal ))
			{
				(*it)->pySet( pyVal );
				Py_DECREF( pyVal );
			}
			else
			{
				ERROR_MSG( "Error changing post processing property '%s'\n",
							(*it)->name() );
			}
			break;
		}
	}
}
