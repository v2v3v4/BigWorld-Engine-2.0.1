/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef COORD_MODE_PROVIDER_HPP
#define COORD_MODE_PROVIDER_HPP

class CoordModeProvider
{
public:
	enum CoordMode
	{
		COORDMODE_WORLD = 0,
		COORDMODE_OBJECT,
		COORDMODE_VIEW
	};
	virtual CoordMode getCoordMode() const
	{
		return COORDMODE_WORLD;
	}

	static CoordModeProvider* ins( CoordModeProvider* instance = NULL )
	{
		if (instance == NULL)
		{
			if (s_instance_ == NULL)
			{
				s_internal_ = true;
				s_instance_ = new CoordModeProvider();
			}
		}
		else
		{
			if (s_instance_ && s_internal_)
				delete s_instance_;
			s_instance_ = instance;
			s_internal_ = false;
		}
		return s_instance_;
	}

	static void fini()
	{
		if (s_internal_)
			delete s_instance_;

		s_instance_ = NULL;
	}
private:
	static CoordModeProvider*	s_instance_;
	static bool					s_internal_;
};

#endif	// COORD_MODE_PROVIDER_HPP
