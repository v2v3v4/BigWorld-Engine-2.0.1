/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef UPDATABLE_HPP
#define UPDATABLE_HPP

/**
 * 	This interface should be implemented by any object that wants to receive
 * 	updates. The CellApp can register objects that implement this interface
 * 	to receive updates every game tick.
 */ 	
class Updatable
{
public:
	Updatable() : removalHandle_( -1 ) {}
	virtual ~Updatable() {};

	/**
	 *	This virtual method is called regularly on every registered Updatable
	 *	object.
	 */
	virtual void update() = 0;

private:
	friend class Updatables;
	int removalHandle_;
};

#endif // UPDATABLE_HPP
