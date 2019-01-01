/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PROCESS_SOCKET_STATS_HPP
#define PROCESS_SOCKET_STATS_HPP

namespace Mercury
{
class PacketReceiverStats;


/**
 *	This class is used to help collect statistics from the
 *	PacketReceiver::processSocket method.
 */
class ProcessSocketStatsHelper
{
public:
	ProcessSocketStatsHelper( PacketReceiverStats & stats );
	~ProcessSocketStatsHelper();

	void startMessageHandling( int messageLength );
	void stopMessageHandling();

	void socketReadFinished( int length );

	void onBundleFinished();
	void onCorruptedBundle();

private:
	PacketReceiverStats & stats_;
};

} // namespace Mercury

#endif // PROCESS_SOCKET_STATS_HPP
