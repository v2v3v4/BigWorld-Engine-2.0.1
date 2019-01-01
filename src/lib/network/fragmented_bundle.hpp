/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef FRAGMENTED_BUNDLE_HPP
#define FRAGMENTED_BUNDLE_HPP

#include "basictypes.hpp"
#include "packet.hpp"

#include "cstdmf/timestamp.hpp"

#include <map>

namespace Mercury
{

class FragmentedBundle;
typedef SmartPointer< FragmentedBundle > FragmentedBundlePtr;

/**
 *  This class represents partially reassembled multi-packet bundles.
 */
class FragmentedBundle : public SafeReferenceCount
{
public:
	/// The age (in seconds) at which a fragmented bundle is abandoned.
	static const uint64 MAX_AGE = 10;

	FragmentedBundle( Packet * pFirstPacket );

	bool addPacket( Packet * p, bool isExternal, const char * sourceStr );

	double age() const
	{
		return (timestamp() - touched_) / stampsPerSecondD();
	}

	bool isOld() const
	{
		return timestamp() - touched_ > stampsPerSecond() * MAX_AGE;
	}

	bool isReliable() const
	{
		return pChain_->hasFlags( Packet::FLAG_IS_RELIABLE );
	}

	int chainLength() const		{ return pChain_->chainLength(); }

	PacketPtr pChain() const	{ return pChain_; }
	SeqNum		lastFragment() const	{ return lastFragment_; }

	bool isComplete() const
	{
		return (remaining_ == 0);
	}

	static void addToStream( FragmentedBundlePtr pFragments,
		BinaryOStream & data );
	static FragmentedBundlePtr createFromStream( BinaryIStream & data );

private:
	SeqNum		lastFragment_;
	int			remaining_;
	uint64		touched_;
	PacketPtr	pChain_;

public:
	/**
	 *  Keys used in FragmentedBundleMaps (below).
	 */
	class Key
	{
	public:
		Key( const Address & addr, SeqNum firstFragment ) :
			addr_( addr ),
			firstFragment_( firstFragment )
		{}

		Address		addr_;
		SeqNum		firstFragment_;
	};
};

/// This map contains partially reassembled packets received by this nub
/// off-channel.  Channels maintain their own FragmentedBundle.
typedef std::map< FragmentedBundle::Key, FragmentedBundlePtr >
	FragmentedBundles;


/**
 * 	@internal
 * 	This method compares two FragmentedBundle::Key objects.
 * 	It is needed for storing fragmented bundles in a map.
 */
inline bool operator==( const FragmentedBundle::Key & a,
	const FragmentedBundle::Key & b )
{
	return (a.firstFragment_ == b.firstFragment_) && (a.addr_ == b.addr_);
}


/**
 * 	@internal
 * 	This method compares two FragmentedBundle::Key objects.
 * 	It is needed for storing fragmented bundles in a map.
 */
inline bool operator<( const FragmentedBundle::Key & a,
	const FragmentedBundle::Key & b )
{
	return (a.firstFragment_ < b.firstFragment_) ||
		(a.firstFragment_ == b.firstFragment_ && a.addr_ < b.addr_);
}

} // namespace Mercury

#endif // FRAGMENTED_BUNDLE_HPP
