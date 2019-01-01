/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ONCE_OFF_PACKET_HPP
#define ONCE_OFF_PACKET_HPP

#include "basictypes.hpp"
#include "fragmented_bundle.hpp"
#include "misc.hpp"

#include <set>

namespace Mercury
{

class EventDispatcher;
class NetworkInterface;

// -----------------------------------------------------------------------------
// Section: OnceOffReceipt
// -----------------------------------------------------------------------------

/**
 *	This struct is used to store details of once-off packets that we have
 *	received.
 */
class OnceOffReceipt
{
public:
	OnceOffReceipt( const Address & addr, SeqNum footerSequence ) :
		addr_( addr ),
		footerSequence_( footerSequence )
	{
	}

	// Overloaded operators declared outside the class:
	//bool operator==( const OnceOffReceipt & oor1, 
	//	const OnceOffReceipt & oor2 );
	//bool operator<( const OnceOffReceipt & oor1, 
	//	const OnceOffReceipt & oor2 );

	Address addr_;
	SeqNum footerSequence_;
};


/**
 *	Equality operator for OnceOffReceipt instances.
 */
inline bool operator==( const OnceOffReceipt & oor1, 
		const OnceOffReceipt & oor2 )
{
	return (oor1.footerSequence_ == oor2.footerSequence_) && 
		(oor1.addr_ == oor2.addr_);
}


/**
 *	Comparison operator for OnceOffReceipt instances.
 */
inline bool operator<( const OnceOffReceipt & oor1,
		const OnceOffReceipt & oor2 )
{
	return (oor1.footerSequence_ < oor2.footerSequence_) ||
		((oor1.footerSequence_ == oor2.footerSequence_) && 
			(oor1.addr_ < oor2.addr_));
}

typedef std::set< OnceOffReceipt > OnceOffReceipts;


// -----------------------------------------------------------------------------
// Section: OnceOffReceiver
// -----------------------------------------------------------------------------

class OnceOffReceiver : public TimerHandler
{
public:
	OnceOffReceiver();
	~OnceOffReceiver();

	void init( EventDispatcher & dispatcher );
	void fini();

	bool onReliableReceived( EventDispatcher & dispatcher,
			const Address & addr, SeqNum seq );

	void onceOffReliableCleanup();

	// TODO: Remove this
	FragmentedBundles & fragmentedBundles() { return fragmentedBundles_; }

private:
	virtual void handleTimeout( TimerHandle handle, void * arg );

	void initOnceOffPacketCleaning( EventDispatcher & dispatcher );

	OnceOffReceipts currOnceOffReceipts_;
	OnceOffReceipts prevOnceOffReceipts_;

	FragmentedBundles fragmentedBundles_;
	// TODO: eventually allocate FragmentedBundleInfo's from
	// a rotating list; when it gets full drop old fragments.

	TimerHandle clearFragmentedBundlesTimerHandle_;
	TimerHandle onceOffPacketCleaningTimerHandle_;
};

// -----------------------------------------------------------------------------
// Section: OnceOffPacket
// -----------------------------------------------------------------------------

class OnceOffPacket : public TimerHandler, public OnceOffReceipt
{
public:
	OnceOffPacket( const Address & addr, SeqNum footerSequence,
					Packet * pPacket = NULL );

	void registerTimer( NetworkInterface & networkInterface );

	virtual void handleTimeout( TimerHandle handle, void * arg );

	PacketPtr pPacket_;
	TimerHandle timerHandle_;
	int retries_;
};
typedef std::set< OnceOffPacket > OnceOffPackets;


// -----------------------------------------------------------------------------
// Section: OnceOffSender
// -----------------------------------------------------------------------------

class OnceOffSender
{
public:
	OnceOffSender();
	~OnceOffSender();

	void addOnceOffResendTimer( const Address & addr, SeqNum seq,
							Packet * p, NetworkInterface & networkInterface );

	void delOnceOffResendTimer( const Address & addr, SeqNum seq,
									NetworkInterface & networkInterface );
	void delOnceOffResendTimer( OnceOffPackets::iterator & iter,
									NetworkInterface & networkInterface );

	void expireOnceOffResendTimer( OnceOffPacket & packet,
									NetworkInterface & networkInterface );

	void onAddressDead( const Address & addr,
									NetworkInterface & networkInterface );

	int onceOffResendPeriod() const
	{	return onceOffResendPeriod_; }

	void onceOffResendPeriod( int microseconds )
	{	onceOffResendPeriod_ = microseconds; }

	int onceOffMaxResends() const
	{	return onceOffMaxResends_; }

	void onceOffMaxResends( int retries )
	{	onceOffMaxResends_ = retries; }

	bool hasUnackedPackets() const { return !onceOffPackets_.empty(); }

private:
	OnceOffPackets onceOffPackets_;

	int onceOffMaxResends_;
	int onceOffResendPeriod_;
};

} // namespace Mercury

#endif // ONCE_OFF_PACKET_HPP
