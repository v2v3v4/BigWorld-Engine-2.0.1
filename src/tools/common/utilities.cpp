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
#include "utilities.hpp"
#include "resmgr/string_provider.hpp"
#include "resmgr/bwresource.hpp"
#include "resmgr/multi_file_system.hpp"
#include "chunk/chunk_space.hpp"

#ifdef _MFC_VER

/*static*/ void Utilities::stretchToRight( CWnd* parent, CWnd& widget, int pageWidth, int border )
{
	BW_GUARD;

	CRect rect;
	widget.GetWindowRect( &rect );
    parent->ScreenToClient( &rect );
	widget.SetWindowPos( 0, rect.left, rect.top, pageWidth - rect.left - border, rect.Height(), SWP_NOZORDER );
}

/*static*/ void Utilities::stretchToBottomRight( CWnd* parent, CWnd& widget, int pageWidth, int bx, int pageHeight, int by )
{
	BW_GUARD;

	CRect rect;
	widget.GetWindowRect( &rect );
    parent->ScreenToClient( &rect );
	widget.SetWindowPos( 0, rect.left, rect.top, pageWidth - rect.left - bx, pageHeight - rect.top - by, SWP_NOZORDER );
}

/*static*/ void Utilities::moveToRight( CWnd* parent, CWnd& widget, int pageWidth, int border )
{
	BW_GUARD;

	CRect rect;
	widget.GetWindowRect( &rect );
    parent->ScreenToClient( &rect );
	int width = rect.right - rect.left;
	widget.SetWindowPos( 0, pageWidth - width - border , rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
}

/*static*/ void Utilities::moveToBottom( CWnd* parent, CWnd& widget, int pageHeight, int border )
{
	BW_GUARD;

	CRect rect;
	widget.GetWindowRect( &rect );
    parent->ScreenToClient( &rect );
	int height = rect.bottom - rect.top;
	widget.SetWindowPos( 0, rect.left, pageHeight - height - border, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
}

/*static*/ void Utilities::moveToTopRight( CWnd* parent, CWnd& widget, int pageWidth, int bx, int py )
{
	BW_GUARD;

	CRect rect;
	widget.GetWindowRect( &rect );
    parent->ScreenToClient( &rect );
	int width = rect.right - rect.left;
	widget.SetWindowPos( 0, pageWidth - bx , py, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
}

/*static*/ void Utilities::moveToBottomLeft( CWnd* parent, CWnd& widget, int pageHeight, int by, int px )
{
	BW_GUARD;

	CRect rect;
	widget.GetWindowRect( &rect );
    parent->ScreenToClient( &rect );
	int height = rect.bottom - rect.top;
	widget.SetWindowPos( 0, px , pageHeight - height - by, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
}

/*static*/ void Utilities::centre( CWnd* parent, CWnd& widget, int pageWidth )
{
	BW_GUARD;

	CRect rect;
	widget.GetWindowRect( &rect );
    parent->ScreenToClient( &rect );
	int width = rect.right - rect.left;
	widget.SetWindowPos( 0, pageWidth/2 - width/2 , rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
}

/*static*/ void Utilities::fieldEnabledState( CEdit& field, bool enable, const std::wstring& text )
{
	BW_GUARD;

	field.SetWindowText( text.c_str() );
	field.SetReadOnly( !enable );
	field.ModifyStyle( enable ? WS_DISABLED : 0, enable ? 0 : WS_DISABLED );
}

#endif // _MFC_VER

/*static*/ std::string Utilities::pythonSafeName( const std::string& s )
{
	BW_GUARD;

    std::string r;

    bool isFirst = true;

	std::string::const_iterator i = s.begin();
	for (; i != s.end(); ++i)
    {
        if (isFirst && isdigit( *i ))
		{
			r.push_back('_');
		}

		char c = tolower( *i );

        if ((c >= 'a' && c <= 'z') || isdigit( c ) || (c == '_'))
        {
			r.push_back( *i );
        }
        else if (c == ' ')
            r.push_back('_');

		isFirst = false;
    }

	return r;
}

/*static*/ std::wstring Utilities::memorySizeToStr( uint32 memorySizeInBytes )
{
	BW_GUARD;

    std::wstring result;
    int suffixIndex = 0;
    float memorySize = (float)memorySizeInBytes;

    while ( ( memorySize >= 1000.f ) && ( suffixIndex < 4 ))
    {
        memorySize /= 1024.f;
        suffixIndex++;
    }

    switch ( suffixIndex )
    {
        case 0:
            result = Localise(L"COMMON/UTILITIES/BYTES", Formatter( memorySize, L"%.1f") );
            break;
        case 1:
            result = Localise(L"COMMON/UTILITIES/KILOBYTES", Formatter( memorySize, L"%.1f") );
            break;
        case 2:
            result = Localise(L"COMMON/UTILITIES/MEGABYTES", Formatter( memorySize, L"%.1f") );
            break;
        case 3:
			result = Localise(L"COMMON/UTILITIES/GIGABYTES", Formatter( memorySize, L"%.1f") );
            break;
        case 4:
            result = Localise(L"COMMON/UTILITIES/TERABYTES", Formatter( memorySize, L"%.1f") );
            break;
    }

    return result;
}


namespace
{

void gatherInternalChunks( const std::string& spaceFolder,
						  const std::string& subFolder,
						  std::set<std::string>& result )
{
	MultiFileSystemPtr fs = BWResource::instance().fileSystem();
	std::string folder = spaceFolder + subFolder;
	IFileSystem::Directory dir = fs->readDirectory( folder );

	for (IFileSystem::Directory::iterator iter = dir.begin(); iter != dir.end(); ++iter)
	{
		if (iter->length() == 15 && iter->substr( 8 ) == "i.chunk")
		{
			result.insert( subFolder + iter->substr( 0, 9 ) );
		}
		else if (fs->getFileType( folder + *iter ) == IFileSystem::FT_DIRECTORY)
		{
			gatherInternalChunks( spaceFolder, subFolder + *iter + '/', result );
		}
	}
}

}

/*static*/ std::set<std::string> 
Utilities::gatherInternalChunks
( 
    const std::string &     spaceFolder
)
{
	BW_GUARD;

	std::set<std::string> result;

	::gatherInternalChunks( spaceFolder, "", result );

	return result;
}


/**
 *  This function return all the chunks name of the given mapping
 *  in a string set
 *
 *  @param mapping  The given chunk mapping to collect all chunks
 *  @return         a string set of all chunks
 */
/*static*/ std::set<std::string> Utilities::gatherChunks( GeometryMapping* mapping )
{
	BW_GUARD;

	std::set<std::string> chunks = Utilities::gatherInternalChunks( mapping->path() );

	for( int i = mapping->minGridX(); i <= mapping->maxGridX(); ++i )
	{
		for( int j = mapping->minGridY(); j <= mapping->maxGridY(); ++j )
		{
			chunks.insert( mapping->outsideChunkIdentifier( i, j ) );
		}
	}

	return chunks;
}

/**
 *  This function composes a matrix from scale, translation and 
 *  euler-angles (in degrees).
 *
 *  @param m        Return parameter that receives the composed matrix
 *  @param scale    Scale
 *  @param trans    Translation
 *  @param rot      Rotation (in degrees)
 */
void Utilities::compose
(
    Matrix          &m, 
    Vector3         const &scale, 
    Vector3         const &trans, 
    Vector3         const &rot
)
{
	BW_GUARD;

    Matrix rotm;
    rotm.setRotate(DEG_TO_RAD(rot.x), DEG_TO_RAD(rot.y), DEG_TO_RAD(rot.z));

    Matrix scalem;
    scalem.setScale(scale);

    Matrix transm;
    transm.setTranslate(trans);

    m = rotm;
    m.postMultiply(scalem);
    m.postMultiply(transm);
}


/**
 *  This function returns the axes of the coord system composed using the
 *  scale, translation and euler-angles (in degrees) passed in.
 *
 *  @param axis0    Return parameter for the first axis of the coord system
 *  @param axis1    Return parameter for the second axis of the coord system
 *  @param axis2    Return parameter for the third axis of the coord system
 *  @param scale    Scale
 *  @param trans    Translation
 *  @param rot      Rotation (in degrees)
 */
void Utilities::compose
(
	Vector4         *axis0, 
	Vector4         *axis1, 
	Vector4         *axis2, 
    Vector3         const &scale, 
    Vector3         const &trans, 
    Vector3         const &rot
)
{
	BW_GUARD;

	Matrix result;
	Utilities::compose(result, scale, trans, rot);

	if ( axis0 )
	{
		Vector3 vec = result.applyToUnitAxisVector(0);
		*axis0 = Vector4( vec.x, vec.y, vec.z, 0.0f );
	}
	if ( axis1 )
	{
		Vector3 vec = result.applyToUnitAxisVector(1);
		*axis1 = Vector4( vec.x, vec.y, vec.z, 0.0f );
	}
	if ( axis2 )
	{
		Vector3 vec = result.applyToUnitAxisVector(2);
		*axis2 = Vector4( vec.x, vec.y, vec.z, 0.0f );
	}
}

/**
 *  This function decomposes a matrix into scale, translation and
 *  euler-angles (in degrees).
 *
 *  @param m        Matrix to be decomposed
 *  @param scale    Receives the resulting scale vector
 *  @param trans    Receives the resulting translation vector
 *  @param rot      Receives the resulting rotation angles (in degrees)
 */
void Utilities::decompose
(
    Matrix          const &m, 
    Vector3         &scale, 
    Vector3         &trans, 
    Vector3         &rot
)
{
	BW_GUARD;

    Matrix matrix = m;

    // The translation:
    trans = matrix.applyToOrigin();
	matrix.translation(Vector3::zero());

    // The scale:
    scale.x = matrix.column(0).length();
    scale.y = matrix.column(1).length();
    scale.z = matrix.column(2).length();

    // Remove the effect of the scale to get the rotations
    if (scale.x != 0)
        matrix.column(0, matrix.column(0)*(1.0f/scale.x));
    if (scale.y != 0)									
        matrix.column(1, matrix.column(1)*(1.0f/scale.y));
    if (scale.z != 0)									
        matrix.column(2, matrix.column(2)*(1.0f/scale.z));

    // The rotation:
    rot.x = RAD_TO_DEG(matrix.yaw  ());
    rot.y = RAD_TO_DEG(matrix.pitch());
    rot.z = RAD_TO_DEG(matrix.roll ());
}


/**
 *  This function decomposes two non-parallel vectors into scale, translation
 *  and euler-angles (in degrees). It receives only two vectors, and the third
 *	vector of the coord system is calculated with their cross product.
 *
 *  @param axis0    First axis of the coord system to be decomposed
 *  @param axis1    Second axis of the coord system to be decomposed
 *  @param scale    Receives the resulting scale vector
 *  @param trans    Receives the resulting translation vector
 *  @param rot      Receives the resulting rotation angles (in degrees)
 */
void Utilities::decompose
(
    Vector4         const &axis0, 
    Vector4         const &axis1, 
    Vector3         &scale, 
    Vector3         &trans, 
    Vector3         &rot
)
{
	BW_GUARD;

    Vector3 u(axis0.x, axis0.y, axis0.z);
    Vector3 v(axis1.x, axis1.y, axis1.z);
    Vector3 w;
    w.crossProduct(v, u);
    Matrix xform(Matrix::identity);
    xform.column(0, axis0);
    xform.column(1, Vector4(w.x, w.y, w.z, 0.0f));
    xform.column(2, axis1);

	decompose(xform, scale, trans, rot);
}
