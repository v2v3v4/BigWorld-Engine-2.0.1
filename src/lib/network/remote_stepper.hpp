/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef REMOTE_STEPPER_HPP
#define REMOTE_STEPPER_HPP

#include <string>

class Endpoint;


/**
 *	This class allows code to be stepped through remotely
 */
class RemoteStepper
{
public:
	RemoteStepper();
	~RemoteStepper();

	static void start();
	static void step( const std::string & desc, bool wait = true );

private:
	void stepInt( const std::string & desc, bool wait );
	bool tryaccept();

	Endpoint	*lep_;
	Endpoint	*cep_;
};

#endif // REMOTE_STEPPER_HPP
