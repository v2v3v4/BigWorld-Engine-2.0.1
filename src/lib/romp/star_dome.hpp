/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef StarDome_HPP
#define StarDome_HPP

#include <iostream>
#include "moo/Visual.hpp"
#include "moo/material.hpp"
#include "moo/managed_texture.hpp"

class StarDome
{
public:
	StarDome();
	~StarDome();

	void		init( void );
	void		draw( float timeOfDay );
private:

	StarDome(const StarDome&);
	StarDome& operator=(const StarDome&);

	Moo::VisualPtr					visual_;
	Moo::Material					mat_;
	Moo::BaseTexturePtr				texture_;

	friend std::ostream& operator<<(std::ostream&, const StarDome&);
};

#ifdef CODE_INLINE
#include "star_dome.ipp"
#endif




#endif
/*StarDome.hpp*/
