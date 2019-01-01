/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DOWNLOAD_STREAMER_HPP
#define DOWNLOAD_STREAMER_HPP

class DownloadStreamer
{
public:
	DownloadStreamer();

	int maxDownloadRate() const;
	int curDownloadRate() const;
	int maxClientDownloadRate() const;
	int downloadRampUpRate() const;
	int downloadBacklogLimit() const;

	float downloadScaleBack() const;
	void modifyDownloadRate( int delta );

private:
	int					curDownloadRate_;
};

#endif // DOWNLOAD_STREAMER_HPP
