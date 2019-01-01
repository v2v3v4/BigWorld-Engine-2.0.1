/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef FOLDER_SETTER_HPP
#define FOLDER_SETTER_HPP

class FolderSetter
{
	static const int MAX_PATH_SIZE = 8192;
	char envFolder_[ MAX_PATH_SIZE ];
	char curFolder_[ MAX_PATH_SIZE ];
public:
	FolderSetter()
	{
		BW_GUARD;

		GetCurrentDirectory( MAX_PATH_SIZE, envFolder_ );
		GetCurrentDirectory( MAX_PATH_SIZE, curFolder_ );
	}
	void enter()
	{
		BW_GUARD;

		GetCurrentDirectory( MAX_PATH_SIZE, envFolder_ );
		SetCurrentDirectory( curFolder_ );
	}
	void leave()
	{
		BW_GUARD;

		GetCurrentDirectory( MAX_PATH_SIZE, curFolder_ );
		SetCurrentDirectory( envFolder_ );
	}
};

class FolderGuard
{
	FolderSetter& setter_;
public:
	FolderGuard( FolderSetter& setter ) : setter_( setter )
	{
		BW_GUARD;

		setter_.enter();
	}
	~FolderGuard()
	{
		BW_GUARD;

		setter_.leave();
	}
};

#endif//FOLDER_SETTER_HPP
