/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#pragma once


#include "lights.hpp"

typedef SmartPointer< class MatrixProxy > MatrixProxyPtr;
typedef SmartPointer< class FloatProxy > FloatProxyPtr;
typedef SmartPointer< class StringProxy > StringProxyPtr;

class MatrixOperation : public UndoRedo::Operation
{
public:
	MatrixOperation( int kind, MatrixProxy* matrixProxy, Matrix oldVal ):
		UndoRedo::Operation( kind ),
		kind_(kind),
		matrixProxy_(matrixProxy),
		oldVal_(oldVal)
	{}

	virtual ~MatrixOperation() {}

	virtual void undo()
	{
		BW_GUARD;

		Matrix val;
		matrixProxy_->getMatrix( val, false );

		// first add the current state of this matrix proxy to the undo/redo list
		UndoRedo::instance().add( new MatrixOperation( kind_, matrixProxy_, val ));

		// now change the matrix back
		matrixProxy_->setMatrix( oldVal_ );
	}
	virtual bool iseq( const UndoRedo::Operation & oth ) const
	{
		return false;
	}
private:
	int kind_;
	MatrixProxy* matrixProxy_;
	Matrix oldVal_;
};

template <class CL> class MeLightMatrixProxy : public MatrixProxy
{
public:

	MeLightMatrixProxy( SmartPointer<CL> valPtr, Matrix* matrix ):
	  valPtr_(valPtr),
	  matrix_(matrix)
	{
		BW_GUARD;

		*matrix_ = Matrix::identity;
		valPtr_->worldTransform( Matrix::identity );
	}
	
	virtual void EDCALL getMatrix( Matrix & m, bool world = true )
	{
		BW_GUARD;

		m = *matrix_;
	}

	virtual void EDCALL getMatrixContext( Matrix & m )
	{
		m = Matrix::identity;
	}

	virtual void EDCALL getMatrixContextInverse( Matrix & m )
	{
		m = Matrix::identity;
	}

	virtual bool EDCALL setMatrix( const Matrix & m )
	{
		BW_GUARD;

		*matrix_ = m;
		valPtr_->worldTransform( Matrix::identity );
		Lights::instance().dirty( true );
		return true;
	}

	virtual void EDCALL setMatrixAlone( const Matrix & m )
	{
		BW_GUARD;

		*matrix_ = m;
		valPtr_->worldTransform( Matrix::identity );
		Lights::instance().dirty( true );
	}

	void EDCALL recordState()
	{
		BW_GUARD;

		recordStateMatrix_ = *matrix_;
	}

	virtual bool EDCALL commitState( bool revertToRecord = false, bool addUndoBarrier = true )
	{
		BW_GUARD;

		if (revertToRecord)
			if (*matrix_ != recordStateMatrix_)
				setMatrix(recordStateMatrix_);

		if (hasChanged())
		{
			UndoRedo::instance().add( new MatrixOperation( 0, this, recordStateMatrix_ ) );
		
			if (addUndoBarrier)
			{
				UndoRedo::instance().barrier( "Light Move", false );
			}
		}

		return true;
	}

	virtual bool EDCALL hasChanged()
	{
		BW_GUARD;

		return !!(recordStateMatrix_ != *matrix_);
	}

protected:
	SmartPointer<CL> valPtr_;
	Matrix* matrix_;
	Matrix recordStateMatrix_;
};

template <class CL> class MeLightPosMatrixProxy : public MeLightMatrixProxy<CL>
{
public:

	MeLightPosMatrixProxy( SmartPointer<CL> valPtr, Matrix* matrix ):
		MeLightMatrixProxy<CL>( valPtr, matrix )
	{
		BW_GUARD;

		valPtr_->position( matrix->applyToOrigin() );
		valPtr_->worldTransform( Matrix::identity );
	}
	
	virtual bool EDCALL setMatrix( const Matrix & m )
	{
		BW_GUARD;

		valPtr_->position( m.applyToOrigin() );
		return MeLightMatrixProxy<CL>::setMatrix( m );
	}

	virtual void EDCALL setMatrixAlone( const Matrix & m )
	{
		BW_GUARD;

		valPtr_->position( m.applyToOrigin() );
		MeLightMatrixProxy<CL>::setMatrixAlone( m );
	}
};

template <class CL> class MeLightDirMatrixProxy : public MeLightMatrixProxy<CL>
{
public:

	MeLightDirMatrixProxy( SmartPointer<CL> valPtr, Matrix* matrix ):
		MeLightMatrixProxy<CL>( valPtr, matrix )
	{
		BW_GUARD;

		valPtr_->direction( matrix->applyToUnitAxisVector(2) );
		valPtr_->worldTransform( Matrix::identity );
	}
	
	virtual bool EDCALL setMatrix( const Matrix & m )
	{
		BW_GUARD;

		valPtr_->direction( m.applyToUnitAxisVector(2) );
		return MeLightMatrixProxy<CL>::setMatrix( m );
	}

	virtual void EDCALL setMatrixAlone( const Matrix & m )
	{
		BW_GUARD;

		valPtr_->direction( m.applyToUnitAxisVector(2) );
		MeLightMatrixProxy<CL>::setMatrixAlone( m );
	}
};

template <class CL> class MeLightPosDirMatrixProxy : public MeLightMatrixProxy<CL>
{
public:

	MeLightPosDirMatrixProxy( SmartPointer<CL> valPtr, Matrix* matrix ):
		MeLightMatrixProxy<CL>( valPtr, matrix )
	{
		BW_GUARD;

		valPtr_->position( matrix->applyToOrigin() );
		valPtr_->direction( matrix->applyToUnitAxisVector(2) );
		valPtr_->worldTransform( Matrix::identity );
	}
	
	virtual bool EDCALL setMatrix( const Matrix & m )
	{
		BW_GUARD;

		valPtr_->position( m.applyToOrigin() );
		valPtr_->direction( m.applyToUnitAxisVector(2) );
		Vector3 test = valPtr_->direction();
		return MeLightMatrixProxy<CL>::setMatrix( m );
	}

	virtual void EDCALL setMatrixAlone( const Matrix & m )
	{
		BW_GUARD;

		valPtr_->position( m.applyToOrigin() );
		valPtr_->direction( m.applyToUnitAxisVector(2) );
		MeLightMatrixProxy<CL>::setMatrixAlone( m );
	}
};

template <class CL> class MeSpotLightPosDirMatrixProxy : public MeLightMatrixProxy<CL>
{
public:

	MeSpotLightPosDirMatrixProxy( SmartPointer<CL> valPtr, Matrix* matrix ):
		MeLightMatrixProxy<CL>( valPtr, matrix )
	{
		BW_GUARD;

		valPtr_->position( matrix->applyToOrigin() );
		valPtr_->direction( matrix->applyToUnitAxisVector(2) );
		valPtr_->worldTransform( Matrix::identity );
	}

	virtual bool EDCALL setMatrix( const Matrix & m )
	{
		BW_GUARD;

		valPtr_->position( m.applyToOrigin() );
		valPtr_->direction( m.applyToUnitAxisVector(2) );
		Vector3 test = valPtr_->direction();
		return MeLightMatrixProxy<CL>::setMatrix( m );
	}

	virtual void EDCALL setMatrixAlone( const Matrix & m )
	{
		BW_GUARD;

		valPtr_->position( m.applyToOrigin() );
		valPtr_->direction( m.applyToUnitAxisVector(2) );
		MeLightMatrixProxy<CL>::setMatrixAlone( m );
	}
};

template <class CL> class MeLightFloatProxy: public UndoableDataProxy<FloatProxy>
{
public:
	typedef typename float (CL::*GetFn)() const;
	typedef void (CL::*SetFn)( float v );

	MeLightFloatProxy( SmartPointer<CL> valPtr, GetFn getFn, SetFn setFn, const std::string& name, float initVal ):
	  valPtr_(valPtr),
	  getFn_(getFn),
	  setFn_(setFn),
	  name_(name)
	{
		BW_GUARD;

		setTransient( initVal );
	}

	float EDCALL get() const
	{
		BW_GUARD;

		return ((*valPtr_).*getFn_)();
	}

	void EDCALL setTransient( float v )
	{
		BW_GUARD;

		((*valPtr_).*setFn_)( v );
		valPtr_->worldTransform( Matrix::identity );
		Lights::instance().dirty( true );
	}

	virtual bool EDCALL setPermanent( float v )
	{
		BW_GUARD;

		setTransient( v );
		return true;
	}

	virtual std::string EDCALL opName()
	{
		BW_GUARD;

		return "Change to light " + name_;
	}

private:
	SmartPointer<CL> valPtr_;
	GetFn getFn_;
	SetFn setFn_;
	std::string name_;
};

class MeLightConeAngleProxy : public UndoableDataProxy<FloatProxy>
{
public:
	MeLightConeAngleProxy( SmartPointer<Moo::SpotLight> light, float initVal ):
		light_( light )
	{
		BW_GUARD;

		setTransient( initVal );
	}

	virtual float EDCALL get() const
	{
		BW_GUARD;

		return RAD_TO_DEG( acosf( light_->cosConeAngle() ));
	}

	virtual bool getRange( float& min, float& max, int& digits ) const
	{
		min = 0.1f;
		max = 179.9f;
		digits = 1;
		return true;
	}

	virtual void EDCALL setTransient( float f )
	{
		BW_GUARD;

		light_->cosConeAngle( cosf( DEG_TO_RAD(f) ) );
		Lights::instance().dirty( true );
	}

	virtual bool EDCALL setPermanent( float f )
	{
		BW_GUARD;

		// complain if it's invalid
		if (f < 0.1f || f > 179.9f) return false;

		// set it
		this->setTransient( f );

		return true;
	}

	virtual std::string EDCALL opName()
	{
		return "change to light cone angle";
	}

private:
	SmartPointer<Moo::SpotLight>	light_;
};

/**
 *	This helper class gets and sets a light colour.
 *	Since all the lights have a 'colour' accessor, but there's no
 *	base class that collects them (for setting anyway), we use a template.
 */
template <class CL> class MeLightColourWrapper: public UndoableDataProxy<ColourProxy>
{
public:
	MeLightColourWrapper( SmartPointer<CL> valPtr ) :
		valPtr_(valPtr)
	{
	}

	virtual Moo::Colour EDCALL get() const
	{
		BW_GUARD;

		return valPtr_->colour();
	}

	virtual void EDCALL setTransient( Moo::Colour v )
	{
		BW_GUARD;

		valPtr_->colour( v );
		Lights::instance().dirty( true );
	}

	virtual bool EDCALL setPermanent( Moo::Colour v )
	{
		BW_GUARD;

		// make sure its valid
		if (v.r < 0.f) v.r = 0.f;
		if (v.r > 1.f) v.r = 1.f;
		if (v.g < 0.f) v.g = 0.f;
		if (v.g > 1.f) v.g = 1.f;
		if (v.b < 0.f) v.b = 0.f;
		if (v.b > 1.f) v.b = 1.f;
		v.a = 1.f;

		// set it
		this->setTransient( v );

		return true;
	}

	virtual std::string EDCALL opName()
	{
		return "light colour change";
	}

private:
	SmartPointer<CL> valPtr_;
};
