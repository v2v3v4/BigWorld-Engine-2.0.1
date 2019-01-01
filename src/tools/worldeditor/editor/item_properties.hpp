/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ITEM_PROPERTIES_HPP
#define ITEM_PROPERTIES_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "gizmo/general_properties.hpp"


/**
 *	This is a MatrixProxy for chunk item pointers.
 */
class ChunkItemMatrix : public MatrixProxy, public Aligned
{
public:
	ChunkItemMatrix( ChunkItemPtr pItem );
	~ChunkItemMatrix();

	void EDCALL getMatrix( Matrix & m, bool world );
	void EDCALL getMatrixContext( Matrix & m );
	void EDCALL getMatrixContextInverse( Matrix & m );
	bool EDCALL setMatrix( const Matrix & m );

	void EDCALL recordState();
	bool EDCALL commitState( bool revertToRecord, bool addUndoBarrier );

	virtual bool EDCALL hasChanged();

	Vector3 movementSnaps() const { return movementSnaps_; }
	void movementSnaps( const Vector3& v ) { movementSnaps_ = v; }

protected:
	ChunkItemPtr	pItem_;
	bool			haveRecorded_;
private:
	Chunk *			origChunk_;
	Matrix			origPose_;

	Vector3			movementSnaps_;
	bool			warned_;

	friend class MyChunkItemMatrix;
};

class ChunkItemPositionProperty : public GenPositionProperty
{
	float length( ChunkItemPtr item );
public:
	ChunkItemPositionProperty( const std::string & name, MatrixProxyPtr pMatrix, ChunkItemPtr item )
		: GenPositionProperty( name, pMatrix, length( item ) )
	{}
};

/**
 *	Wrap some property that has an accessor to get and set it. It always
 *	uses an undoable data proxy. The set accessor should return a bool
 *	indicating whether or not the value set to was legal.
 *
 *	Template arguments are class to be overriden, and proxy to use.
 */
template <class CL, class DT> class AccessorDataProxy :
	public UndoableDataProxy<DT>
{
public:
	typedef typename DT::Data (CL::*GetFn)() const;
	typedef bool (CL::*SetFn)( const typename DT::Data & v );

	AccessorDataProxy( SmartPointer<CL> pItem,
			const std::string & propDesc, GetFn getFn, SetFn setFn ) :
		pItem_( pItem ),
		propDesc_( propDesc ),
		getFn_( getFn ),
		setFn_( setFn )
	{
	}

	virtual typename DT::Data EDCALL get() const
	{
		BW_GUARD;

		return ((*pItem_).*getFn_)();
	}

	virtual void EDCALL setTransient( typename DT::Data v )
	{
		BW_GUARD;

		((*pItem_).*setFn_)( v );
	}

	virtual bool EDCALL setPermanent( typename DT::Data v )
	{
		BW_GUARD;

		// set it
		if (!((*pItem_).*setFn_)( v )) return false;

		// ok, flag the chunk as having changed then
		WorldManager::instance().changedChunk( pItem_->chunk() );
		pItem_->edPostModify();

		// update its data section
		pItem_->edSave( pItem_->pOwnSect() );

		return true;
	}

	virtual std::string EDCALL opName()
	{
		BW_GUARD;

		return "Set " + pItem_->edDescription() + " " + propDesc_;
	}

private:
	SmartPointer<CL>	pItem_;
	std::string			propDesc_;
	GetFn				getFn_;
	SetFn				setFn_;
};

/**
 *	Wrap some property that has an accessor to get and set it. It always
 *	uses an undoable data proxy. The set accessor should return a bool
 *	indicating whether or not the value set to was legal.
 *
 *	Template arguments are class to be overriden, and proxy to use.
 */
template <class CL, class DT> class AccessorDataProxyWithName :
	public UndoableDataProxy<DT>
{
public:
	typedef typename DT::Data (CL::*GetFn)( const std::string& name ) const;
	typedef bool (CL::*SetFn)( const std::string& name, const typename DT::Data & v );
	typedef bool (CL::*RangeFn)( const std::string& name, typename DT::Data & min, typename DT::Data & max,
		int& digits );
	typedef bool (CL::*GetDefaultFn)( const std::string& name, typename DT::Data & def );
	typedef void (CL::*SetToDefaultFn)( const std::string& name );
	typedef bool (CL::*IsDefaultFn)( const std::string& name );

	AccessorDataProxyWithName( SmartPointer<CL> pItem,
			const std::string & propDesc, GetFn getFn, SetFn setFn, RangeFn rangeFn = NULL,
			GetDefaultFn getDefaultFn = NULL, SetToDefaultFn setToDefaultFn = NULL, IsDefaultFn isDefaultFn = NULL ) :
		pItem_( pItem ),
		propDesc_( propDesc ),
		getFn_( getFn ),
		setFn_( setFn ),
		rangeFn_( rangeFn ),
		getDefaultFn_( getDefaultFn ),
		setToDefaultFn_( setToDefaultFn ),
		isDefaultFn_( isDefaultFn )
	{
	}

	virtual typename DT::Data EDCALL get() const
	{
		BW_GUARD;

		return ((*pItem_).*getFn_)( propDesc_ );
	}

	virtual void EDCALL setTransient( typename DT::Data v )
	{
		BW_GUARD;

		((*pItem_).*setFn_)( propDesc_, v );
	}

	// for matrix proxy
	virtual void EDCALL getMatrix( typename DT::Data & m, bool world = true )
	{
		BW_GUARD;

		m = ((*pItem_).*getFn_)( propDesc_ );
	}
	virtual void EDCALL getMatrixContext( Matrix & m ){}
	virtual void EDCALL getMatrixContextInverse( Matrix & m ){}
	virtual bool EDCALL setMatrix( const typename DT::Data & m )
	{
		BW_GUARD;

		((*pItem_).*setFn_)( propDesc_, m );
		return true;
	}
	virtual void EDCALL recordState(){}
	virtual bool EDCALL commitState( bool revertToRecord = false, bool addUndoBarrier = true ){ return false; }

	/** If the state has changed since the last call to recordState() */
	virtual bool EDCALL hasChanged(){	return false; }

	virtual bool EDCALL setPermanent( typename DT::Data v )
	{
		BW_GUARD;

		// set it
		if (!((*pItem_).*setFn_)( propDesc_, v )) return false;

		// ok, flag the chunk as having changed then
		WorldManager::instance().changedChunk( pItem_->chunk() );
		pItem_->edPostModify();

		// update its data section
		pItem_->edSave( pItem_->pOwnSect() );

		return true;
	}

	virtual std::string EDCALL opName()
	{
		BW_GUARD;

		return "Set " + pItem_->edDescription() + " " + propDesc_;
	}

	bool getRange( int& min, int& max ) const
	{
		BW_GUARD;

		DT::Data mind, maxd;
		int digits;
		if( rangeFn_ )
		{
			bool result =  ((*pItem_).*rangeFn_)( propDesc_, mind, maxd, digits );
			min = mind;
			max = maxd;
			return result;
		}
		return false;
	}
	bool getRange( float& min, float& max, int& digits ) const
	{
		BW_GUARD;

		DT::Data mind, maxd;
		if( rangeFn_ )
		{
			bool result =  ((*pItem_).*rangeFn_)( propDesc_, mind, maxd, digits );
			min = mind;
			max = maxd;
			return result;
		}
		return false;
	}
	bool getDefault( float& def ) const
	{
		BW_GUARD;

		if( getDefaultFn_ )
			return ((*pItem_).*getDefaultFn_)( propDesc_, def );
		return false;
	}
	void setToDefault()
	{
		BW_GUARD;

		if( setToDefaultFn_ )
			((*pItem_).*setToDefaultFn_)( propDesc_ );
	}
	bool isDefault() const
	{
		BW_GUARD;

		if( isDefaultFn_ )
			return ((*pItem_).*isDefaultFn_)( propDesc_ );
		return false;
	}
private:
	SmartPointer<CL>	pItem_;
	std::string			propDesc_;
	GetFn				getFn_;
	SetFn				setFn_;
	RangeFn				rangeFn_;
	GetDefaultFn		getDefaultFn_;
	SetToDefaultFn		setToDefaultFn_;
	IsDefaultFn			isDefaultFn_;
};


/**
 *	Wrap some property that has an accessor to get and set it transiently,
 *	but must be edSave and reloaded to take permanent effect. It always
 *	uses an undoable data proxy.
 *
 *	Template arguments are class to be overriden, and proxy to use.
 */
template <class CL, class DT> class SlowPropReloadingProxy :
	public UndoableDataProxy<DT>
{
public:
	typedef typename DT::Data (CL::*GetFn)() const;
	typedef void (CL::*SetFn)( const typename DT::Data & v );

	SlowPropReloadingProxy( SmartPointer<CL> pItem,
			const std::string & propDesc, GetFn getFn, SetFn setFn ) :
		pItem_( pItem ),
		propDesc_( propDesc ),
		getFn_( getFn ),
		setFn_( setFn )
	{
	}

	virtual typename DT::Data EDCALL get() const
	{
		BW_GUARD;

		return ((*pItem_).*getFn_)();
	}

	virtual void EDCALL setTransient( typename DT::Data v )
	{
		BW_GUARD;

		((*pItem_).*setFn_)( v );
	}

	virtual bool EDCALL setPermanent( typename DT::Data v )
	{
		BW_GUARD;

		// set it
		this->setTransient( v );

		// flag the chunk as having changed
		WorldManager::instance().changedChunk( pItem_->chunk() );
		pItem_->edPostModify();

		// update its data section
		pItem_->edSave( pItem_->pOwnSect() );

		// see if we can load it (no harm in calling overriden method)
		if (!pItem_->reload()) return false;

		// if we failed then our caller will call setPermanent back to the
		//  old value so it's ok to return here with a damaged item.
		return true;
	}

	virtual std::string EDCALL opName()
	{
		BW_GUARD;

		return "Set " + pItem_->edDescription() + " " + propDesc_;
	}

private:
	SmartPointer<CL>	pItem_;
	std::string			propDesc_;
	GetFn				getFn_;
	SetFn				setFn_;
};


// helper method for representing chunk pointers
extern std::string chunkPtrToString( Chunk * pChunk );

/**
 *	Wrap a constant (for the user) chunk pointer and present it as name.
 */
template <class CL> class ConstantChunkNameProxy : public StringProxy
{
public:
	explicit ConstantChunkNameProxy( SmartPointer<CL> pItem,
									Chunk * (CL::*getFn)() const = NULL ) :
		pItem_( pItem ),
		pLastChunk_( NULL ),
		getFn_( getFn )
	{
	}

	virtual std::string EDCALL get() const
	{
		BW_GUARD;

		Chunk * pNewChunk = NULL;

		if (getFn_ == NULL)
		{
			pNewChunk = pItem_->chunk();
		}
		else
		{
			pNewChunk = ((*pItem_).*getFn_)();
		}

		if (pNewChunk != pLastChunk_)
		{
			pLastChunk_ = pNewChunk;
			cachedString_ = chunkPtrToString( pNewChunk );
		}
		return cachedString_;
	}

	virtual void EDCALL set( std::string, bool, bool ) { }

private:
	SmartPointer<CL> pItem_;
	mutable Chunk * pLastChunk_;
	mutable std::string cachedString_;
	Chunk * (CL::*getFn_)() const;
};


#endif // ITEM_PROPERTIES_HPP
