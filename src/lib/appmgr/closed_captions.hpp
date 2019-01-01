/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CLOSED_CAPTIONS_HPP
#define CLOSED_CAPTIONS_HPP

#include "commentary.hpp"
#include "cstdmf/concurrency.hpp"
#include "romp/font.hpp"

#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"

class SimpleGUIComponent;
class TextGUIComponent;

/**
 *	This class displays commentary messages, for those
 *	who cannot hear because they just can't, or they
 *	haven't got a sound card.
 */
class ClosedCaptions : public Commentary::View, public PyObjectPlus
{
	Py_Header( ClosedCaptions, PyObjectPlus )

public:
	ClosedCaptions( int bufferSize = 5, PyTypePlus * pType = &s_type_ );
	~ClosedCaptions();

	void onAddMsg( const std::wstring & msg, int id );
	void update( float dTime );

	void visible( bool state );
	bool visible() const;

	PyObject *	pyGetAttribute( const char * attr );
	int			pySetAttribute( const char * attr, PyObject * value );

	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, visible, visible )

	PY_METHOD_DECLARE( py_update )

	PY_METHOD_DECLARE( py_addAsView )
	PY_METHOD_DECLARE( py_delAsView )

	PY_FACTORY_DECLARE()

private:
	ClosedCaptions( const ClosedCaptions& );
	ClosedCaptions& operator=( const ClosedCaptions& );

	void parseEventQueue();
	void addMsg( const std::wstring & msg, int id );

	class Caption
	{
	public:
		Caption()
			:age_( 0.f )
		{
		}

		SimpleGUIComponent * backing_;
		TextGUIComponent* component_;
		float			  age_;
	};
	typedef std::vector<Caption>	Captions;

	SimpleGUIComponent * root_;
	
	Captions	msgs_;
	int			head_;
	CachedFontPtr font_;
	float		timeout_;
	float		fadeout_;
	float		clipHeight_;
	Vector2		margin_;

	struct PendingMessage
	{
		PendingMessage( const std::wstring& msg, int id )
			:msg_( msg ),
			 id_( id )
		{
		};

		std::wstring msg_;
		int id_;
	};
	typedef std::vector< PendingMessage > PendingMessages;
	PendingMessages	pendingMessages_;
};


#include "ashes/simple_gui_component.hpp"

#ifdef CODE_INLINE
#include "closed_captions.ipp"
#endif

#endif // CLOSED_CAPTIONS_HPP
