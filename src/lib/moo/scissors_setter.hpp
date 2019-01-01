/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SCISSORS_SETTER_HPP
#define SCISSORS_SETTER_HPP

namespace Moo
{
/**
 *	This class is a scoped scissors setter, allowing you to
 *	easily set a new scissors rectangle on the device.
 *
 *	The previous settings are restored automatically when the
 *	class goes out of scope.
 */
	class ScissorsSetter
	{
	public:
		ScissorsSetter();
		ScissorsSetter( uint32 x, uint32 y, uint32 width, uint32 height );
		~ScissorsSetter();

		static bool isAvailable();
	private:		
		RECT oldRect_;
		RECT newRect_;
	};
};	//namespace Moo

#endif	//Scissors_SETTER_HPP