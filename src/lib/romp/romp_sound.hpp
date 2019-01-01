/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ROMP_SOUND_HPP
#define ROMP_SOUND_HPP

/**
 *	This abstract class is the interface between the romp library and
 *	the game sound libraries
 */

class RompSound: public SafeReferenceCount
{
public:

	virtual void playParticleSound(	const char*		label,
									const Vector3&	pos,
									float			velocitySq,
									int				srcIdx,
									uint			materialKind);

	static SmartPointer < RompSound > getProvider();

	static void setProvider( RompSound* soundProv );

private:
	static SmartPointer < RompSound > soundProvider_;
};


#endif // ROMP_SOUND_HPP