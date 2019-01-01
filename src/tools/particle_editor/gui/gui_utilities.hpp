/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef GUI_UTILITIES_HPP
#define GUI_UTILITIES_HPP

#include "common/user_messages.hpp"

float       StringToFloat(CString stringValue);
int         StringToInt(CString stringValue);

CString     FloatToString(float value);
CString     IntToString(int value);

COLORREF    RGBtoCOLORREF(const Vector4 & color);
Vector4     COLORREFtoRGB(const COLORREF & colorREF);

void
GetFilenameAndDirectory
(
    CString                 const &longFilename, 
    CString                 &filename, 
    CString                 &directory
);

typedef bool (*PopulateTestFunction)( const std::string & s );

void 
PopulateComboBoxWithFilenames
(
    CComboBox               &comboBox, 
    std::string             const &dir, 
    PopulateTestFunction    test
);

enum SetOperation
{
    SET_CONTROL = 1,
    SET_PSA     = 2
};

#define SET_CHECK_PARAMETER(task, var)                                         \
    {                                                                          \
        if (task == SET_CONTROL)                                               \
        {                                                                      \
            var ## _.SetCheck(action()->var() ? BST_CHECKED : BST_UNCHECKED);  \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            action()->var(var ## _.GetCheck() == BST_CHECKED);                 \
        }                                                                      \
    }


#define SET_FLOAT_PARAMETER(task, var)                                         \
    {                                                                          \
        if (task == SET_CONTROL)                                               \
        {                                                                      \
            var ## _.SetValue(action()->var());                                \
        }                                                                      \
        else if (task == SET_PSA)                                              \
        {                                                                      \
            action()->var(var ## _.GetValue());                                \
        }                                                                      \
        else                                                                   \
        {                                                                      \
        }                                                                      \
    }


#define SET_INT_PARAMETER(task, var)                                           \
    {                                                                          \
        if (task == SET_CONTROL)                                               \
        {                                                                      \
            var ## _.SetIntegerValue(action()->var());                         \
        }                                                                      \
        else if (task == SET_PSA)                                              \
        {                                                                      \
            action()->var(var ## _.GetIntegerValue());                         \
        }                                                                      \
        else                                                                   \
        {                                                                      \
        }                                                                      \
    }

#define SET_VECTOR3_PARAMETER(task, var, X, Y, Z)                              \
    {                                                                          \
        if (task == SET_CONTROL)                                               \
        {                                                                      \
            X ## _.SetValue(action()->var().x);                                \
            Y ## _.SetValue(action()->var().y);                                \
            Z ## _.SetValue(action()->var().z);                                \
        }                                                                      \
        else if (task == SET_PSA)                                              \
        {                                                                      \
            Vector3 newVector                                                  \
                    (                                                          \
                        X ## _.GetValue(),                                     \
                        Y ## _.GetValue(),                                     \
                        Z ## _.GetValue()                                      \
                    );                                                         \
            action()->var(newVector);                                          \
        }                                                                      \
        else                                                                   \
        {                                                                      \
        }                                                                      \
    }

#define SET_STDSTRING_PARAMETER(task, var)                                     \
    {                                                                          \
        if (task == SET_CONTROL)                                               \
        {                                                                      \
            var ## _ = action()->var();                                        \
        }                                                                      \
        else if (task == SET_PSA)                                              \
        {                                                                      \
            action()->var(var ## _);                                           \
        }                                                                      \
    }

#define SET_CEDIT_STRING_PARAMETER(task, var)                                  \
    {                                                                          \
        if (task == SET_CONTROL)                                               \
        {                                                                      \
            var ## _.SetWindowText(action()->var().c_str());                   \
        }                                                                      \
        else if (task == SET_PSA)                                              \
        {                                                                      \
            CString theString;                                                 \
            var ## _.GetWindowText(theString);                                 \
            action()->var(theString.GetBuffer());                              \
        }                                                                      \
    }

#endif // GUI_UTILITIES_HPP