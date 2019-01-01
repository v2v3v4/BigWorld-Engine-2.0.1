/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LINK_PROPERTY_HPP
#define LINK_PROPERTY_HPP


#include "gizmo/general_properties.hpp"
#include "gizmo/link_proxy.hpp"


/**
 *  This property is used to link items.
 */
class LinkProperty : public GeneralProperty
{
public:
	LinkProperty
    (
        std::string         const &name, 
        LinkProxyPtr        linkProxy,
        MatrixProxyPtr      matrix,   
		bool				alwaysShow = true
    );

	virtual const ValueType & valueType() const { RETURN_VALUETYPE( STRING ); }

	/*virtual*/ PyObject * EDCALL pyGet();
	/*virtual*/ int EDCALL pySet(PyObject * value, bool transient = false);

    MatrixProxyPtr matrix() const;
    LinkProxyPtr link() const;

	bool alwaysShow() const;

private:
    LinkProxyPtr            linkProxy_;
    MatrixProxyPtr          matrix_;
	bool					alwaysShow_;

private:
	GENPROPERTY_VIEW_FACTORY_DECLARE(LinkProperty)
};


#endif // LINK_PROPERTY_HPP
