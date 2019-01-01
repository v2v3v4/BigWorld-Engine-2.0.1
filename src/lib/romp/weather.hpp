/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef WEATHER_HPP
#define WEATHER_HPP

#include <vector>
#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"

/*~ class BigWorld.Weather
 *	@components{ client, tools }
 *
 *	This class is used to control the weather in the clients world.
 *
 *	The weather object is used to set the wind speed  
 *	and wind gustiness.
 *	The wind affects the rate at which clouds move across the sky,
 *	and the pertubations of detail objects such as grass.
 */
class Weather : public PyObjectPlus
{
	Py_Header( Weather, PyObjectPlus )
public:
	Weather();
	~Weather();

	void activate();
	void deactivate();

	void tick( float dTime );

	PyObject *			pyGetAttribute( const char * attr );
	int					pySetAttribute( const char * attr, PyObject * value );
	
	PY_METHOD_DECLARE(py_windAverage);
	PY_METHOD_DECLARE(py_windGustiness);

	void windAverage( float xv, float yv )	{ windVelX_ = xv; windVelY_ = yv; }
	const Vector2& wind() const				{ return wind_; }
	void windGustiness( float amount )		{ windGustiness_ = amount; }

private:
	Vector2			wind_;
	float			windVelX_;	//target velocity
	float			windVelY_;
	float			windGustiness_;

};


#endif // WEATHER_HPP
