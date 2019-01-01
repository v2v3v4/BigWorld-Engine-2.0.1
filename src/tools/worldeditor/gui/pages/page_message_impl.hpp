/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PAGE_MESSAGE_IMPL_HPP
#define PAGE_MESSAGE_IMPL_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "common/page_messages.hpp"


class BBMsgImpl: public MsgsImpl
{
public:

	BBMsgImpl( PageMessages* msgs );
	~BBMsgImpl();

	void OnNMClickMsgList(NMHDR *pNMHDR, LRESULT *pResult);
	void OnNMCustomdrawMsgList(NMHDR *pNMHDR, LRESULT *pResult);

private:
	PageMessages* msgs_;
	HFONT boldFont;
};


#endif // PAGE_MESSAGE_IMPL_HPP
