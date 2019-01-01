/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

// alpha_gui_shader.ipp
#ifdef CODE_INLINE
#define INLINE inline
#else
#define INLINE
#endif


INLINE void AlphaGUIShader::constants(
	float t1, float t2, float alpha, float speed )
{
	this->touch();

	constants_[0] = t1;
	constants_[1] = t2;
	constants_[2] = alpha;
	constants_[3] = speed;
}


INLINE void AlphaGUIShader::stop( float v )
{
	this->touch();
	constants_[0] = v;
}

INLINE void AlphaGUIShader::start( float v )
{
	this->touch();
	constants_[1] = v;
}

INLINE void AlphaGUIShader::value( float v )
{
	this->touch();
	constants_[2] = v;
}

INLINE void AlphaGUIShader::touch()
{
	timer_ = 0.f;
	oldAlpha_ = currentAlpha_;
}

/*~ function AlphaGUIShader.reset
 *	@components{ client, tools }
 *
 *	This function resets the interpolation of alpha to start at the current
 *	value and go for a number of seconds specified by the speed attribute
 *  moving linearly to the value specified by the alpha attribute.
 *
 *	This can be useful if you want to reset the interpolation without actually
 *	changing the initial alpha value.
 */
INLINE void	AlphaGUIShader::reset()
{
	oldAlpha_ = constants_[2];
	currentAlpha_ = constants_[2];
	timer_ = 0.f;
}




// alpha_gui_shader.ipp