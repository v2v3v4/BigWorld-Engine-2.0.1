/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef RAW_IMPORT_DLG_HPP
#define RAW_IMPORT_DLG_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "worldeditor/resource.h"
#include "controls/edit_numeric.hpp"
#include "controls/dib_section32.hpp"
#include "controls/image_control.hpp"
#include "controls/auto_tooltip.hpp"


class RawImportDlg : public CDialog
{
public:
    enum { IDD = IDD_RAW_IMPORT_DLG };

    explicit RawImportDlg(char const *filename);

    /*virtual*/ ~RawImportDlg();

    void getResult
    (
        unsigned int    &width, 
        unsigned int    &height, 
        bool            &littleEndian
    ) const;

protected:
    /*virtual*/ BOOL OnInitDialog();

    /*virtual*/ void DoDataExchange(CDataExchange *dx);

    /*virtual*/ void OnOK();

    afx_msg void OnUpdateImage();

    DECLARE_MESSAGE_MAP()

    DECLARE_AUTO_TOOLTIP(RawImportDlg, CDialog)

private:
    std::string             	filename_;
    CEdit                   	filenameEdit_;
    CComboBox               	sizeCB_;
    controls::ImageControl8		bmpImage_;
    CButton                 	littleEndianButton_;
    CButton                 	bigEndianButton_;
    uint8                   	*data_;
    size_t                  	dataSize_;
    unsigned int            	selWidth_;
    unsigned int            	selHeight_;
    bool                    	littleEndian_;
};


#endif // RAW_IMPORT_DLG_HPP
