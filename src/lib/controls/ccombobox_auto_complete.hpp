/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CCOMBOBOX_AUTO_COMPLETE_HPP
#define CCOMBOBOX_AUTO_COMPLETE_HPP

#include "controls/defs.hpp"
#include "controls/fwd.hpp"

namespace controls
{
	class CONTROLS_DLL CComboBoxAutoComplete : public CComboBox
	{
		DECLARE_DYNAMIC(CComboBoxAutoComplete)

	public:
		CComboBoxAutoComplete();

		/*virtual*/ ~CComboBoxAutoComplete();

		void restrictToListBoxItems(bool option);

	private:
		virtual BOOL PreTranslateMessage(MSG* pMsg);

		afx_msg void OnCbnEditupdate();
		
		DECLARE_MESSAGE_MAP()

	private:
		CString         previousText_;
		int             previousCurSel_;
		bool            restrictToListBoxItems_;
	};
}
#endif // CCOMBOBOX_AUTO_COMPLETE_HPP
