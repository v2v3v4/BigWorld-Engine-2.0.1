/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ASYN_MSG_HPP
#define ASYN_MSG_HPP

class AsyncMessage
{
public:
	void reportMessage( const wchar_t* msg, bool severity );
	void show();
	void hide();
	bool isShow() const;
	HWND handle();
	const wchar_t* getLogFileName() const;
	void printMsg( const wchar_t * msg ) const;

private:
	std::wstring dateMsg( const wchar_t * msg ) const;
	void writeToLog( const wchar_t * msg ) const;
};

#endif//ASYN_MSG_HPP
