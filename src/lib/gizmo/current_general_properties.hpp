/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CURRENT_GENERAL_PROPERTIES_HPP
#define CURRENT_GENERAL_PROPERTIES_HPP

#include "general_editor.hpp"
#include "general_properties.hpp"

template<class PROPERTY>
class PropertyCollator : public GeneralProperty::View
{
public:
	PropertyCollator( PROPERTY& prop ) : prop_( prop )
	{
	}

	virtual void EDCALL elect()
	{
		BW_GUARD;

		properties_.push_back( &prop_ );
	}

	virtual void EDCALL expel()
	{
		BW_GUARD;

		properties_.erase( std::find( properties_.begin(), properties_.end(), &prop_ ) );
	}

	virtual void EDCALL select()
	{
	}

	static std::vector<PROPERTY*> properties()
	{
		return properties_;
	}

private:
	PROPERTY&						prop_;
protected:
	static std::vector<PROPERTY*>	properties_;
};


class CurrentPositionProperties : public PropertyCollator<GenPositionProperty>
{
public:
	CurrentPositionProperties( GenPositionProperty& prop ) : PropertyCollator<GenPositionProperty>( prop )
	{
	}

	static GeneralProperty::View * create( GenPositionProperty & prop )
	{
		BW_GUARD;

		return new CurrentPositionProperties( prop );
	}

	/** Get the average origin of all the selected properties */
	static Vector3 averageOrigin();


private:
	static struct ViewEnroller
	{
		ViewEnroller()
		{
			BW_GUARD;

			GenPositionProperty_registerViewFactory(
				GeneralProperty::nextViewKindID(), &create );
		}
	}	s_viewEnroller;
};


/**
 *	A view into the current rotation property of the current editor
 *	Allows WheelRotator and MatrixRotator to rotate the correct thing
 */
class CurrentRotationProperties : public PropertyCollator<GenRotationProperty> //: public GeneralProperty::View
{
public:
	CurrentRotationProperties( GenRotationProperty& prop ) : PropertyCollator<GenRotationProperty>( prop )
	{
	}

	/** Get the average origin of all the selected properties */
	static Vector3 averageOrigin();

	static GeneralProperty::View * create( GenRotationProperty & prop )
	{
		BW_GUARD;

		return new CurrentRotationProperties( prop );
	}

private:

	static struct ViewEnroller
	{
		ViewEnroller()
		{
			BW_GUARD;

			GenRotationProperty_registerViewFactory(
				GeneralProperty::nextViewKindID(), &create );
		}
	}	s_viewEnroller;
};



class CurrentScaleProperties : public PropertyCollator<GenScaleProperty>
{
public:
	CurrentScaleProperties( GenScaleProperty& prop ) : PropertyCollator<GenScaleProperty>( prop )
	{
	}

	/** Get the average origin of all the selected properties */
	static Vector3 averageOrigin();

	static GeneralProperty::View * create( GenScaleProperty& prop )
	{
		BW_GUARD;

		return new CurrentScaleProperties( prop );
	}

private:
	static struct ViewEnroller
	{
		ViewEnroller()
		{
			BW_GUARD;

			GenScaleProperty_registerViewFactory(
				GeneralProperty::nextViewKindID(), &create );
		}
	}	s_viewEnroller;
};


#endif // CURRENT_GENERAL_PROPERTIES_HPP
