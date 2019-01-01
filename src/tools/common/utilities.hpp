/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ULTILITIES_HPP
#define ULTILITIES_HPP

#include "math/vector3.hpp"
#include "math/vector4.hpp"
#include "math/matrix.hpp"
#include <string>
#include <vector>
#include <set>

class GeometryMapping;

class Utilities
{
public:
#ifdef _MFC_VER
	static void stretchToRight( CWnd* parent, CWnd& widget, int pageWidth, int border );
	static void stretchToBottomRight( CWnd* parent, CWnd& widget, int pageWidth, int bx, int pageHeight, int by );

	static void moveToRight( CWnd* parent, CWnd& widget, int pageWidth, int border );
	static void moveToBottom( CWnd* parent, CWnd& widget, int pageHeight, int border );

	static void moveToTopRight( CWnd* parent, CWnd& widget, int pageWidth, int bx, int py );
	static void moveToBottomLeft( CWnd* parent, CWnd& widget, int pageHeight, int by, int px );

	static void centre( CWnd* parent, CWnd& widget, int pageWidth );

	static void fieldEnabledState( CEdit& field, bool enable, const std::wstring& text = L"" );
#endif // _MFC_VER

	static std::string pythonSafeName( const std::string& s );
	static std::wstring memorySizeToStr( uint32 memorySizeInBytes );

    static std::set<std::string> gatherInternalChunks( const std::string & spaceFolder );
	static std::set<std::string> gatherChunks( GeometryMapping* mapping );

    static void compose
    (
        Matrix          &m, 
        Vector3         const &scale, 
        Vector3         const &trans, 
        Vector3         const &rot
    );
    static void compose
    (
		Vector4         *axis0, 
		Vector4         *axis1, 
		Vector4         *axis2, 
        Vector3         const &scale, 
        Vector3         const &trans, 
        Vector3         const &rot
    );
	static void decompose
	(
		Matrix          const &m, 
		Vector3         &scale, 
		Vector3         &trans, 
		Vector3         &rot
	);
	static void decompose
	(
		Vector4         const &axis0, 
		Vector4         const &axis1, 
		Vector3         &scale, 
		Vector3         &trans, 
		Vector3         &rot
	);
};

#endif // ULTILITIES_HPP
