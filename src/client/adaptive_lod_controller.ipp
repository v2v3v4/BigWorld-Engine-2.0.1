/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#ifdef CODE_INLINE
#define INLINE inline
#else
#define INLINE
#endif

// INLINE void AdaptiveLODController::inlineFunction()
// {
// }


INLINE int
AdaptiveLODController::addController( const std::string& name, float * variable, float _default, float w, float s, float relativeImportance )
{
	BW_GUARD;
	MF_ASSERT_DEV( variable );

	LODController l;

	l.name_ = name;
	l.variable_ = variable;
	l.worst( w );
	l.defaultValue( _default );
	l.relativeImportance_ = relativeImportance;
	l.defaultValue( *variable );
	l.speed( s );
	l.current_ = _default;

	return this->addController( l );
}


INLINE int
AdaptiveLODController::addController( const LODController & controller )
{
	BW_GUARD;
	int retIdx = lodControllers_.size();

	lodControllers_.push_back( controller );

	return retIdx;
}


INLINE AdaptiveLODController::LODController & AdaptiveLODController::controller( int idx )
{
	BW_GUARD;
	MF_ASSERT_DEV( idx >= 0 );
	MF_ASSERT_DEV( idx < (int)lodControllers_.size() );

	return lodControllers_[ idx ];
}


INLINE void	AdaptiveLODController::minimumFPS( float fps )
{
	minimumFPS_ = fps;
}


INLINE float AdaptiveLODController::minimumFPS( void ) const
{
	return minimumFPS_;
}


INLINE int AdaptiveLODController::numControllers( void ) const
{
	return lodControllers_.size();
}


INLINE float AdaptiveLODController::effectiveFPS( void ) const
{
	return effectiveFPS_;
}


/*adaptive_lod_controller.ipp*/
