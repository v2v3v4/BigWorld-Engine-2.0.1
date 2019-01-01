/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/**
 *  String formatting functions (type-safe versions of snprintf).
 * 
 *  These functions are like the C# string.format function.  For example you
 *  can do something like:
 * 
 *      string name = "Bill";
 *      string s    = sformat("Hello {0}, how are you {0}?", name);
 * 
 *  which would generate the string "Hello Bill, how are you Bill?".
 * 
 *  To use these functions we require that operator<< is defined for all
 *  objects that are printed.  We also limit the number of objects to ten.
 *  If you want to print more then extend the templates below in the obvious
 *  fashion.
 */


#ifndef FORMAT_HPP
#define FORMAT_HPP


/**
 *  Convert an object to a string.  This assumes that operator<<
 *  is defined for the object.
 * 
 *  @param object   The object to convert to a string.
 *  @returns        The object in string form.
 * 
 *  We specialise this for string, wstring, 
 *  combinations to go via fromStringToWString etc.
 * 
 *  Note that this does NOT work with char * and wchar_t *.
 */
template<typename CHAR_TYPE, typename T>
std::basic_string<CHAR_TYPE>
toString(T const &object);

/**
 *  Convert an object from a string.  This assumes that operator>>
 *  is defined for the object, as is the default constructor.
 * 
 *  @param str      The string to convert to an object.
 *  @returns        The object constructed from the string.
 */
template<typename CHAR_TYPE, typename T>
T fromString(std::basic_string<CHAR_TYPE> const &str);


/**
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param         str  The format string.
 *   @param         obj1 object1.
 *   @returns       The formatted string.
 */
template
<
    typename CLASS1
>
std::string 
sformat
(
    std::string     const &str, 
    CLASS1          const &obj1
);


/**
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param         str  The format string.
 *   @param         obj1 object1.
 *   @param         obj2 object2.
 *   @returns       The formatted string.
 */
template
<
    typename CLASS1,
    typename CLASS2
>
std::string  
sformat
(
    std::string     const &str, 
    CLASS1          const &obj1,
    CLASS2          const &obj2
);


/** 
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param          str  The format string.
 *   @param          obj1 object1.
 *   @param          obj2 object2.
 *   @param          obj3 object3.
 *   @returns        The formatted string.
 */
template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3
>
std::string  
sformat
(
    std::string     const &str, 
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3
);


/** 
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param          str  The format string.
 *   @param          obj1 object1.
 *   @param          obj2 object2.
 *   @param          obj3 object3.
 *   @param          obj4 object4.
 *   @returns        The formatted string.
 */
template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4
>
std::string    
sformat
(
    std::string     const &str, 
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4
);


/** 
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param          str  The format string.
 *   @param          obj1 object1.
 *   @param          obj2 object2.
 *   @param          obj3 object3.
 *   @param          obj4 object4.
 *   @param          obj5 object5.
 *   @returns        The formatted string.
 */ 
template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5
>
std::string   
sformat
(
    std::string     const &str, 
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5
);


/** 
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param          str  The format string.
 *   @param          obj1 object1.
 *   @param          obj2 object2.
 *   @param          obj3 object3.
 *   @param          obj4 object4.
 *   @param          obj5 object5.
 *   @param          obj6 object6.
 *   @returns        The formatted string.
 */
template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6
>
std::string   
sformat
(
    std::string     const &str, 
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6
);


/** 
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param          str  The format string.
 *   @param          obj1 object1.
 *   @param          obj2 object2.
 *   @param          obj3 object3.
 *   @param          obj4 object4.
 *   @param          obj5 object5.
 *   @param          obj6 object6.
 *   @param          obj7 object7.
 *   @returns        The formatted string.
 */ 
template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6,
    typename CLASS7
>
std::string   
sformat
(
    std::string     const &str, 
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6,
    CLASS7          const &obj7
);


/** 
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param          str  The format string.
 *   @param          obj1 object1.
 *   @param          obj2 object2.
 *   @param          obj3 object3.
 *   @param          obj4 object4.
 *   @param          obj5 object5.
 *   @param          obj6 object6.
 *   @param          obj7 object7.
 *   @param          obj8 object8.
 *   @returns        The formatted string.
 */
template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6,
    typename CLASS7,
    typename CLASS8
>
std::string   
sformat
(
    std::string     const &str, 
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6,
    CLASS7          const &obj7,
    CLASS8          const &obj8
);


/** 
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param          str  The format string.
 *   @param          obj1 object1.
 *   @param          obj2 object2.
 *   @param          obj3 object3.
 *   @param          obj4 object4.
 *   @param          obj5 object5.
 *   @param          obj6 object6.
 *   @param          obj7 object7.
 *   @param          obj8 object8.
 *   @param          obj9 object9.
 *   @returns        The formatted string.
 */ 
template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6,
    typename CLASS7,
    typename CLASS8,
    typename CLASS9
>
std::string   
sformat
(
    std::string     const &str, 
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6,
    CLASS7          const &obj7,
    CLASS8          const &obj8,
    CLASS9          const &obj9
);


/**
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param          str  The format string.
 *   @param          obj1 object1.
 *   @param          obj2 object2.
 *   @param          obj3 object3.
 *   @param          obj4 object4.
 *   @param          obj5 object5.
 *   @param          obj6 object6.
 *   @param          obj7 object7.
 *   @param          obj8 object8.
 *   @param          obj9 object9.
 *   @param          obj10 object10.
 *   @returns        The formatted string.
 */
template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6,
    typename CLASS7,
    typename CLASS8,
    typename CLASS9,
    typename CLASS10
>
std::string   
sformat
(
    std::string     const &str, 
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6,
    CLASS7          const &obj7,
    CLASS8          const &obj8,
    CLASS9          const &obj9,
    CLASS10         const &obj10
);


/**
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param          str  The format string.
 *   @param          obj1 object1.
 *   @returns        The formatted string.
 */
template
<
    typename CLASS1
>
std::wstring   
wformat
(
    std::wstring    const &str, 
    CLASS1          const &obj1
);


/**
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param          str  The format string.
 *   @param          obj1 object1.
 *   @param          obj2 object2.
 *   @returns        The formatted string.
 */ 
template
<
    typename CLASS1,
    typename CLASS2
>
std::wstring  
wformat
(
    std::wstring    const &str, 
    CLASS1          const &obj1,
    CLASS2          const &obj2
);


/** 
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param          str  The format string.
 *   @param          obj1 object1.
 *   @param          obj2 object2.
 *   @param          obj3 object3.
 *   @returns        The formatted string.
 */ 
template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3
>
std::wstring   
wformat
(
    std::wstring    const &str, 
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3
);


/** 
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param          str  The format string.
 *   @param          obj1 object1.
 *   @param          obj2 object2.
 *   @param          obj3 object3.
 *   @param          obj4 object4.
 *   @returns        The formatted string.
 */ 
template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4
>
std::wstring  
wformat
(
    std::wstring    const &str, 
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4
);


/** 
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param          str  The format string.
 *   @param          obj1 object1.
 *   @param          obj2 object2.
 *   @param          obj3 object3.
 *   @param          obj4 object4.
 *   @param          obj5 object5.
 *   @returns        The formatted string.
 */ 
template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5
>
std::wstring  
wformat
(
    std::wstring    const &str, 
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5
);


/** 
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param          str  The format string.
 *   @param          obj1 object1.
 *   @param          obj2 object2.
 *   @param          obj3 object3.
 *   @param          obj4 object4.
 *   @param          obj5 object5.
 *   @param          obj6 object6.
 *   @returns        The formatted string.
 */ 
template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6
>
std::wstring  
wformat
(
    std::wstring    const &str, 
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6
);


/**
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param          str  The format string.
 *   @param          obj1 object1.
 *   @param          obj2 object2.
 *   @param          obj3 object3.
 *   @param          obj4 object4.
 *   @param          obj5 object5.
 *   @param          obj6 object6.
 *   @param          obj7 object7.
 *   @returns        The formatted string.
 */
template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6,
    typename CLASS7
>
std::wstring  
wformat
(
    std::wstring    const &str, 
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6,
    CLASS7          const &obj7
);


/** 
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param          str  The format string.
 *   @param          obj1 object1.
 *   @param          obj2 object2.
 *   @param          obj3 object3.
 *   @param          obj4 object4.
 *   @param          obj5 object5.
 *   @param          obj6 object6.
 *   @param          obj7 object7.
 *   @param          obj8 object8.
 *   @returns        The formatted string.
 */ 
template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6,
    typename CLASS7,
    typename CLASS8
>
std::wstring  
wformat
(
    std::wstring    const &str, 
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6,
    CLASS7          const &obj7,
    CLASS8          const &obj8
);


/** 
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param          str  The format string.
 *   @param          obj1 object1.
 *   @param          obj2 object2.
 *   @param          obj3 object3.
 *   @param          obj4 object4.
 *   @param          obj5 object5.
 *   @param          obj6 object6.
 *   @param          obj7 object7.
 *   @param          obj8 object8.
 *   @param          obj9 object9.
 *   @returns        The formatted string.
 */ 
template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6,
    typename CLASS7,
    typename CLASS8,
    typename CLASS9
>
std::wstring  
wformat
(
    std::wstring    const &str, 
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6,
    CLASS7          const &obj7,
    CLASS8          const &obj8,
    CLASS9          const &obj9
);


/**
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param          str  The format string.
 *   @param          obj1 object1.
 *   @param          obj2 object2.
 *   @param          obj3 object3.
 *   @param          obj4 object4.
 *   @param          obj5 object5.
 *   @param          obj6 object6.
 *   @param          obj7 object7.
 *   @param          obj8 object8.
 *   @param          obj9 object9.
 *   @param          obj10 object10.
 *   @returns        The formatted string.
 */ 
template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6,
    typename CLASS7,
    typename CLASS8,
    typename CLASS9,
    typename CLASS10
>
std::wstring  
wformat
(
    std::wstring    const &str, 
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6,
    CLASS7          const &obj7,
    CLASS8          const &obj8,
    CLASS9          const &obj9,
    CLASS10         const &obj10
);


#ifdef _MFC_VER

/**
 *  Load a resource string.
 *
 *  @param resid    The resource id of the string to load.
 *  @returns        The loaded string.
 */
std::string loadString(UINT resid);

/**
 *  Load a resource string.
 *
 *  @param resid    The resource id of the string to load.
 *  @returns        The loaded string.
 */
std::wstring wloadString(UINT resid);

#endif // _MFC_VER

#ifdef WIN32

/**
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param resid   The resource id of the string to load.
 *   @param obj1    object1.
 *   @returns       The formatted string.
 */
template
<
    typename CLASS1
>
std::string 
sformat
(
    UINT            resid,
    CLASS1          const &obj1
);


/**
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param resid   The resource id of the string to load.
 *   @param obj1    object1.
 *   @param obj2    object2.
 *   @returns       The formatted string.
 */
template
<
    typename CLASS1,
    typename CLASS2
>
std::string  
sformat
(
    UINT            resid,
    CLASS1          const &obj1,
    CLASS2          const &obj2
);


/** 
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param resid   The resource id of the string to load.
 *   @param obj1    object1.
 *   @param obj2    object2.
 *   @param obj3    object3.
 *   @returns        The formatted string.
 */
template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3
>
std::string  
sformat
(
    UINT            resid, 
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3
);


/** 
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param resid   The resource id of the string to load.
 *   @param obj1    object1.
 *   @param obj2    object2.
 *   @param obj3    object3.
 *   @param obj4    object4.
 *   @returns        The formatted string.
 */
template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4
>
std::string    
sformat
(
    UINT            resid,
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4
);


/** 
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param resid   The resource id of the string to load.
 *   @param obj1    object1.
 *   @param obj2    object2.
 *   @param obj3    object3.
 *   @param obj4    object4.
 *   @param obj5    object5.
 *   @returns        The formatted string.
 */ 
template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5
>
std::string   
sformat
(
    UINT            resid,
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5
);


/** 
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param resid   The resource id of the string to load.
 *   @param obj1    object1.
 *   @param obj2    object2.
 *   @param obj3    object3.
 *   @param obj4    object4.
 *   @param obj5    object5.
 *   @param obj6    object6.
 *   @returns        The formatted string.
 */
template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6
>
std::string   
sformat
(
    UINT            resid,
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6
);


/** 
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param resid   The resource id of the string to load.
 *   @param obj1    object1.
 *   @param obj2    object2.
 *   @param obj3    object3.
 *   @param obj4    object4.
 *   @param obj5    object5.
 *   @param obj6    object6.
 *   @param obj7    object7.
 *   @returns        The formatted string.
 */ 
template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6,
    typename CLASS7
>
std::string   
sformat
(
    UINT            resid,
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6,
    CLASS7          const &obj7
);


/** 
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param resid   The resource id of the string to load.
 *   @param obj1    object1.
 *   @param obj2    object2.
 *   @param obj3    object3.
 *   @param obj4    object4.
 *   @param obj5    object5.
 *   @param obj6    object6.
 *   @param obj7    object7.
 *   @param obj8    object8.
 *   @returns        The formatted string.
 */
template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6,
    typename CLASS7,
    typename CLASS8
>
std::string   
sformat
(
    UINT            resid,
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6,
    CLASS7          const &obj7,
    CLASS8          const &obj8
);


/** 
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param resid   The resource id of the string to load.
 *   @param obj1    object1.
 *   @param obj2    object2.
 *   @param obj3    object3.
 *   @param obj4    object4.
 *   @param obj5    object5.
 *   @param obj6    object6.
 *   @param obj7    object7.
 *   @param obj8    object8.
 *   @param obj9    object9.
 *   @returns       The formatted string.
 */ 
template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6,
    typename CLASS7,
    typename CLASS8,
    typename CLASS9
>
std::string   
sformat
(
    UINT            resid,
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6,
    CLASS7          const &obj7,
    CLASS8          const &obj8,
    CLASS9          const &obj9
);


/**
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param resid   The resource id of the string to load.
 *   @param obj1    object1.
 *   @param obj2    object2.
 *   @param obj3    object3.
 *   @param obj4    object4.
 *   @param obj5    object5.
 *   @param obj6    object6.
 *   @param obj7    object7.
 *   @param obj8    object8.
 *   @param obj9    object9.
 *   @param obj10   object10.
 *   @returns       The formatted string.
 */
template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6,
    typename CLASS7,
    typename CLASS8,
    typename CLASS9,
    typename CLASS10
>
std::string   
sformat
(
    UINT            resid,
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6,
    CLASS7          const &obj7,
    CLASS8          const &obj8,
    CLASS9          const &obj9,
    CLASS10         const &obj10
);


/**
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param resid   The resource id of the string to load.
 *   @param obj1    object1.
 *   @returns       The formatted string.
 */
template
<
    typename CLASS1
>
std::wstring   
wformat
(
    UINT            resid,
    CLASS1          const &obj1
);


/**
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param resid   The resource id of the string to load.
 *   @param obj1    object1.
 *   @param obj2    object2.
 *   @returns       The formatted string.
 */ 
template
<
    typename CLASS1,
    typename CLASS2
>
std::wstring  
wformat
(
    UINT            resid,
    CLASS1          const &obj1,
    CLASS2          const &obj2
);


/** 
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param resid   The resource id of the string to load.
 *   @param obj1    object1.
 *   @param obj2    object2.
 *   @param obj3    object3.
 *   @returns       The formatted string.
 */ 
template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3
>
std::wstring   
wformat
(
    UINT            resid,
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3
);


/** 
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param resid   The resource id of the string to load.
 *   @param obj1    object1.
 *   @param obj2    object2.
 *   @param obj3    object3.
 *   @param obj4    object4.
 *   @returns       The formatted string.
 */ 
template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4
>
std::wstring  
wformat
(
    UINT            resid,
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4
);


/** 
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param resid   The resource id of the string to load.
 *   @param obj1    object1.
 *   @param obj2    object2.
 *   @param obj3    object3.
 *   @param obj4    object4.
 *   @param obj5    object5.
 *   @returns       The formatted string.
 */ 
template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5
>
std::wstring  
wformat
(
    UINT            resid,
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5
);


/** 
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param resid   The resource id of the string to load.
 *   @param obj1    object1.
 *   @param obj2    object2.
 *   @param obj3    object3.
 *   @param obj4    object4.
 *   @param obj5    object5.
 *   @param obj6    object6.
 *   @returns       The formatted string.
 */ 
template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6
>
std::wstring  
wformat
(
    UINT            resid, 
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6
);


/**
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param resid   The resource id of the string to load.
 *   @param obj1    object1.
 *   @param obj2    object2.
 *   @param obj3    object3.
 *   @param obj4    object4.
 *   @param obj5    object5.
 *   @param obj6    object6.
 *   @param obj7    object7.
 *   @returns       The formatted string.
 */
template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6,
    typename CLASS7
>
std::wstring  
wformat
(
    UINT            resid,
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6,
    CLASS7          const &obj7
);


/** 
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param resid   The resource id of the string to load.
 *   @param obj1    object1.
 *   @param obj2    object2.
 *   @param obj3    object3.
 *   @param obj4    object4.
 *   @param obj5    object5.
 *   @param obj6    object6.
 *   @param obj7    object7.
 *   @param obj8    object8.
 *   @returns       The formatted string.
 */ 
template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6,
    typename CLASS7,
    typename CLASS8
>
std::wstring  
wformat
(
    UINT            resid,
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6,
    CLASS7          const &obj7,
    CLASS8          const &obj8
);


/** 
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param resid   The resource id of the string to load.
 *   @param obj1    object1.
 *   @param obj2    object2.
 *   @param obj3    object3.
 *   @param obj4    object4.
 *   @param obj5    object5.
 *   @param obj6    object6.
 *   @param obj7    object7.
 *   @param obj8    object8.
 *   @param obj9    object9.
 *   @returns       The formatted string.
 */ 
template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6,
    typename CLASS7,
    typename CLASS8,
    typename CLASS9
>
std::wstring  
wformat
(
    UINT            resid,
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6,
    CLASS7          const &obj7,
    CLASS8          const &obj8,
    CLASS9          const &obj9
);


/**
 *  Print to a string.
 * 
 *  Print format string replacing {0} etc with the first
 *  object and so on.
 * 
 *   @param resid   The resource id of the string to load.
 *   @param obj1    object1.
 *   @param obj2    object2.
 *   @param obj3    object3.
 *   @param obj4    object4.
 *   @param obj5    object5.
 *   @param obj6    object6.
 *   @param obj7    object7.
 *   @param obj8    object8.
 *   @param obj9    object9.
 *   @param obj10   object10.
 *   @returns       The formatted string.
 */ 
template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6,
    typename CLASS7,
    typename CLASS8,
    typename CLASS9,
    typename CLASS10
>
std::wstring  
wformat
(
    UINT            resid,
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6,
    CLASS7          const &obj7,
    CLASS8          const &obj8,
    CLASS9          const &obj9,
    CLASS10         const &obj10
);
#endif // WIN32


/**
 *  Private details
 */
namespace details
{
    bool
    sformatNextObj
    (
        std::string         &result,
        std::string         const &str,
        size_t              &pos,
        size_t              &idx
    );

    bool
    wformatNextObj
    (
        std::wstring        &result,
        std::wstring        const &str,
        size_t              &pos,
        size_t              &idx
    );
}

//
// Implementation
//


template<typename CHAR_TYPE, typename T>
inline
std::basic_string<CHAR_TYPE>
toString(T const &object)
{
    std::basic_ostringstream<CHAR_TYPE> output;
    output << object;
    return output.str();
}


template<>
inline
std::basic_string<char>
toString(std::string const &object)
{
    return object;
}


template<>
inline
std::basic_string<wchar_t>
toString(wchar_t * const &object)
{
    return object;
}


template<typename CHAR_TYPE, typename T>
inline
T 
fromString(std::basic_string<CHAR_TYPE> const &str)
{
    T result;
    std::basic_istringstream<CHAR_TYPE> input(str);
    input >> result;
    return result;
}


template
<
    typename CLASS1
>
inline
std::string  
sformat
(
    std::string     const &str, 
    CLASS1          const &obj1
)
{
    std::string result;
    size_t pos = 0;
    size_t idx = 0;
    while (details::sformatNextObj(result, str, pos, idx))
    {
        switch (idx%1)
        {
        case 0: result += toString<char>(obj1); break;
        }
    }
    return result;
}


template
<
    typename CLASS1,
    typename CLASS2
>
inline
std::string   
sformat
(
    std::string     const &str, 
    CLASS1          const &obj1,
    CLASS2          const &obj2
)
{
    std::string result;
    size_t pos = 0;
    size_t idx = 0;
    while (details::sformatNextObj(result, str, pos, idx))
    {
        switch (idx%2)
        {
        case 0: result += toString<char>(obj1); break;
        case 1: result += toString<char>(obj2); break;
        }
    }
    return result;
}


template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3
>
inline
std::string   
sformat
(
    std::string     const &str, 
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3
)
{
    std::string result;
    size_t pos = 0;
    size_t idx = 0;
    while (details::sformatNextObj(result, str, pos, idx))
    {
        switch (idx%3)
        {
        case 0: result += toString<char>(obj1); break;
        case 1: result += toString<char>(obj2); break;
        case 2: result += toString<char>(obj3); break;
        }
    }
    return result;
}


template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4
>
inline
std::string   
sformat
(
    std::string     const &str, 
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4
)
{
    std::string result;
    size_t pos = 0;
    size_t idx = 0;
    while (details::sformatNextObj(result, str, pos, idx))
    {
        switch (idx%4)
        {
        case 0: result += toString<char>(obj1); break;
        case 1: result += toString<char>(obj2); break;
        case 2: result += toString<char>(obj3); break;
        case 3: result += toString<char>(obj4); break;
        }
    }
    return result;
}


template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5
>
inline
std::string  
sformat
(
    std::string     const &str, 
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5
)
{
    std::string result;
    size_t pos = 0;
    size_t idx = 0;
    while (details::sformatNextObj(result, str, pos, idx))
    {
        switch (idx%5)
        {
        case 0: result += toString<char>(obj1); break;
        case 1: result += toString<char>(obj2); break;
        case 2: result += toString<char>(obj3); break;
        case 3: result += toString<char>(obj4); break;
        case 4: result += toString<char>(obj5); break;
        }
    }
    return result;
}


template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6
>
inline
std::string   
sformat
(
    std::string     const &str, 
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6
)
{
    std::string result;
    size_t pos = 0;
    size_t idx = 0;
    while (details::sformatNextObj(result, str, pos, idx))
    {
        switch (idx%6)
        {
        case 0: result += toString<char>(obj1); break;
        case 1: result += toString<char>(obj2); break;
        case 2: result += toString<char>(obj3); break;
        case 3: result += toString<char>(obj4); break;
        case 4: result += toString<char>(obj5); break;
        case 5: result += toString<char>(obj6); break;
        }
    }
    return result;
}


template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6,
    typename CLASS7
>
inline
std::string   
sformat
(
    std::string     const &str, 
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6,
    CLASS7          const &obj7
)
{
    std::string result;
    size_t pos = 0;
    size_t idx = 0;
    while (details::sformatNextObj(result, str, pos, idx))
    {
        switch (idx%7)
        {
        case 0: result += toString<char>(obj1); break;
        case 1: result += toString<char>(obj2); break;
        case 2: result += toString<char>(obj3); break;
        case 3: result += toString<char>(obj4); break;
        case 4: result += toString<char>(obj5); break;
        case 5: result += toString<char>(obj6); break;
        case 6: result += toString<char>(obj7); break;
        }
    }
    return result;
}


template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6,
    typename CLASS7,
    typename CLASS8
>
inline
std::string  
sformat
(
    std::string     const &str, 
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6,
    CLASS7          const &obj7,
    CLASS8          const &obj8
)
{
    std::string result;
    size_t pos = 0;
    size_t idx = 0;
    while (details::sformatNextObj(result, str, pos, idx))
    {
        switch (idx%8)
        {
        case 0: result += toString<char>(obj1); break;
        case 1: result += toString<char>(obj2); break;
        case 2: result += toString<char>(obj3); break;
        case 3: result += toString<char>(obj4); break;
        case 4: result += toString<char>(obj5); break;
        case 5: result += toString<char>(obj6); break;
        case 6: result += toString<char>(obj7); break;
        case 7: result += toString<char>(obj8); break;
        }
    }
    return result;
}


template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6,
    typename CLASS7,
    typename CLASS8,
    typename CLASS9
>
inline
std::string  
sformat
(
    std::string     const &str, 
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6,
    CLASS7          const &obj7,
    CLASS8          const &obj8,
    CLASS9          const &obj9
)
{
    std::string result;
    size_t pos = 0;
    size_t idx = 0;
    while (details::sformatNextObj(result, str, pos, idx))
    {
        switch (idx%9)
        {
        case 0: result += toString<char>(obj1); break;
        case 1: result += toString<char>(obj2); break;
        case 2: result += toString<char>(obj3); break;
        case 3: result += toString<char>(obj4); break;
        case 4: result += toString<char>(obj5); break;
        case 5: result += toString<char>(obj6); break;
        case 6: result += toString<char>(obj7); break;
        case 7: result += toString<char>(obj8); break;
        case 8: result += toString<char>(obj9); break;
        }
    }
    return result;
}


template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6,
    typename CLASS7,
    typename CLASS8,
    typename CLASS9,
    typename CLASS10
>
inline
std::string  
sformat
(
    std::string     const &str, 
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6,
    CLASS7          const &obj7,
    CLASS8          const &obj8,
    CLASS9          const &obj9,
    CLASS10         const &obj10
)
{
    std::string result;
    size_t pos = 0;
    size_t idx = 0;
    while (details::sformatNextObj(result, str, pos, idx))
    {
        switch (idx%10)
        {
        case 0: result += toString<char>(obj1);  break;
        case 1: result += toString<char>(obj2);  break;
        case 2: result += toString<char>(obj3);  break;
        case 3: result += toString<char>(obj4);  break;
        case 4: result += toString<char>(obj5);  break;
        case 5: result += toString<char>(obj6);  break;
        case 6: result += toString<char>(obj7);  break;
        case 7: result += toString<char>(obj8);  break;
        case 8: result += toString<char>(obj9);  break;
        case 9: result += toString<char>(obj10); break;
        }
    }
    return result;
}


template
<
    typename CLASS1
>
inline
std::wstring  
wformat
(
    std::wstring    const &str, 
    CLASS1          const &obj1
)
{
    std::wstring  result;
    size_t pos = 0;
    size_t idx = 0;
    while (details::wformatNextObj(result, str, pos, idx))
    {
        switch (idx%1)
        {
        case 0: result += toString<wchar_t>(obj1); break;
        }
    }
    return result;
}


template
<
    typename CLASS1,
    typename CLASS2
>
inline
std::wstring  
wformat
(
    std::wstring    const &str, 
    CLASS1          const &obj1,
    CLASS2          const &obj2
)
{
    std::wstring  result;
    size_t pos = 0;
    size_t idx = 0;
    while (details::wformatNextObj(result, str, pos, idx))
    {
        switch (idx%2)
        {
        case 0: result += toString<wchar_t>(obj1); break;
        case 1: result += toString<wchar_t>(obj2); break;
        }
    }
    return result;
}


template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3
>
inline
std::wstring  
wformat
(
    std::wstring    const &str, 
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3
)
{
    std::wstring  result;
    size_t pos = 0;
    size_t idx = 0;
    while (details::wformatNextObj(result, str, pos, idx))
    {
        switch (idx%3)
        {
        case 0: result += toString<wchar_t>(obj1); break;
        case 1: result += toString<wchar_t>(obj2); break;
        case 2: result += toString<wchar_t>(obj3); break;
        }
    }
    return result;
}


template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4
>
inline
std::wstring  
wformat
(
    std::wstring    const &str, 
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4
)
{
    std::wstring result;
    size_t pos = 0;
    size_t idx = 0;
    while (details::wformatNextObj(result, str, pos, idx))
    {
        switch (idx%4)
        {
        case 0: result += toString<wchar_t>(obj1); break;
        case 1: result += toString<wchar_t>(obj2); break;
        case 2: result += toString<wchar_t>(obj3); break;
        case 3: result += toString<wchar_t>(obj4); break;
        }
    }
    return result;
}


template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5
>
inline
std::wstring  
wformat
(
    std::wstring    const &str, 
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5
)
{
    std::wstring  result;
    size_t pos = 0;
    size_t idx = 0;
    while (details::wformatNextObj(result, str, pos, idx))
    {
        switch (idx%5)
        {
        case 0: result += toString<wchar_t>(obj1); break;
        case 1: result += toString<wchar_t>(obj2); break;
        case 2: result += toString<wchar_t>(obj3); break;
        case 3: result += toString<wchar_t>(obj4); break;
        case 4: result += toString<wchar_t>(obj5); break;
        }
    }
    return result;
}


template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6
>
inline
std::wstring  
wformat
(
    std::wstring    const &str, 
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6
)
{
    std::wstring  result;
    size_t pos = 0;
    size_t idx = 0;
    while (details::wformatNextObj(result, str, pos, idx))
    {
        switch (idx%6)
        {
        case 0: result += toString<wchar_t>(obj1); break;
        case 1: result += toString<wchar_t>(obj2); break;
        case 2: result += toString<wchar_t>(obj3); break;
        case 3: result += toString<wchar_t>(obj4); break;
        case 4: result += toString<wchar_t>(obj5); break;
        case 5: result += toString<wchar_t>(obj6); break;
        }
    }
    return result;
}


template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6,
    typename CLASS7
>
inline
std::wstring  
wformat
(
    std::wstring    const &str, 
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6,
    CLASS7          const &obj7
)
{
    std::wstring  result;
    size_t pos = 0;
    size_t idx = 0;
    while (details::wformatNextObj(result, str, pos, idx))
    {
        switch (idx%7)
        {
        case 0: result += toString<wchar_t>(obj1); break;
        case 1: result += toString<wchar_t>(obj2); break;
        case 2: result += toString<wchar_t>(obj3); break;
        case 3: result += toString<wchar_t>(obj4); break;
        case 4: result += toString<wchar_t>(obj5); break;
        case 5: result += toString<wchar_t>(obj6); break;
        case 6: result += toString<wchar_t>(obj7); break;
        }
    }
    return result;
}


template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6,
    typename CLASS7,
    typename CLASS8
>
inline
std::wstring  
wformat
(
    std::wstring    const &str, 
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6,
    CLASS7          const &obj7,
    CLASS8          const &obj8
)
{
    std::wstring  result;
    size_t pos = 0;
    size_t idx = 0;
    while (details::wformatNextObj(result, str, pos, idx))
    {
        switch (idx%8)
        {
        case 0: result += toString<wchar_t>(obj1); break;
        case 1: result += toString<wchar_t>(obj2); break;
        case 2: result += toString<wchar_t>(obj3); break;
        case 3: result += toString<wchar_t>(obj4); break;
        case 4: result += toString<wchar_t>(obj5); break;
        case 5: result += toString<wchar_t>(obj6); break;
        case 6: result += toString<wchar_t>(obj7); break;
        case 7: result += toString<wchar_t>(obj8); break;
        }
    }
    return result;
}


template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6,
    typename CLASS7,
    typename CLASS8,
    typename CLASS9
>
inline
std::wstring  
wformat
(
    std::wstring    const &str, 
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6,
    CLASS7          const &obj7,
    CLASS8          const &obj8,
    CLASS9          const &obj9
)
{
    std::wstring  result;
    size_t pos = 0;
    size_t idx = 0;
    while (details::wformatNextObj(result, str, pos, idx))
    {
        switch (idx%9)
        {
        case 0: result += toString<wchar_t>(obj1); break;
        case 1: result += toString<wchar_t>(obj2); break;
        case 2: result += toString<wchar_t>(obj3); break;
        case 3: result += toString<wchar_t>(obj4); break;
        case 4: result += toString<wchar_t>(obj5); break;
        case 5: result += toString<wchar_t>(obj6); break;
        case 6: result += toString<wchar_t>(obj7); break;
        case 7: result += toString<wchar_t>(obj8); break;
        case 8: result += toString<wchar_t>(obj9); break;
        }
    }
    return result;
}


template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6,
    typename CLASS7,
    typename CLASS8,
    typename CLASS9,
    typename CLASS10
>
inline
std::wstring  
wformat
(
    std::wstring    const &str, 
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6,
    CLASS7          const &obj7,
    CLASS8          const &obj8,
    CLASS9          const &obj9,
    CLASS10         const &obj10
)
{
    std::wstring  result;
    size_t pos = 0;
    size_t idx = 0;
    while (details::wformatNextObj(result, str, pos, idx))
    {
        switch (idx%10)
        {
        case 0: result += toString<wchar_t>(obj1);  break;
        case 1: result += toString<wchar_t>(obj2);  break;
        case 2: result += toString<wchar_t>(obj3);  break;
        case 3: result += toString<wchar_t>(obj4);  break;
        case 4: result += toString<wchar_t>(obj5);  break;
        case 5: result += toString<wchar_t>(obj6);  break;
        case 6: result += toString<wchar_t>(obj7);  break;
        case 7: result += toString<wchar_t>(obj8);  break;
        case 8: result += toString<wchar_t>(obj9);  break;
        case 9: result += toString<wchar_t>(obj10); break;
        }
    }
    return result;
}


#ifdef WIN32

template
<
    typename CLASS1
>
std::string 
sformat
(
    UINT            resid,
    CLASS1          const &obj1
)
{
    std::string s = loadString(resid);
    return sformat(s, obj1);
}


template
<
    typename CLASS1,
    typename CLASS2
>
std::string  
sformat
(
    UINT            resid,
    CLASS1          const &obj1,
    CLASS2          const &obj2
)
{
    std::string s = loadString(resid);
    return sformat(s, obj1, obj2);
}


template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3
>
std::string  
sformat
(
    UINT            resid, 
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3
)
{
    std::string s = loadString(resid);
    return sformat(s, obj1, obj2, obj3);
}


template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4
>
std::string    
sformat
(
    UINT            resid,
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4
)
{
    std::string s = loadString(resid);
    return sformat(s, obj1, obj2, obj3, obj4);
}


template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5
>
std::string   
sformat
(
    UINT            resid,
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5
)
{
    std::string s = loadString(resid);
    return sformat(s, obj1, obj2, obj3, obj4, obj5);
}


template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6
>
std::string   
sformat
(
    UINT            resid,
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6
)
{
    std::string s = loadString(resid);
    return sformat(s, obj1, obj2, obj3, obj4, obj5, obj6);
}


template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6,
    typename CLASS7
>
std::string   
sformat
(
    UINT            resid,
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6,
    CLASS7          const &obj7
)
{
    std::string s = loadString(resid);
    return sformat(s, obj1, obj2, obj3, obj4, obj5, obj6, obj7);
}


template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6,
    typename CLASS7,
    typename CLASS8
>
std::string   
sformat
(
    UINT            resid,
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6,
    CLASS7          const &obj7,
    CLASS8          const &obj8
)
{
    std::string s = loadString(resid);
    return sformat(s, obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8);
}


template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6,
    typename CLASS7,
    typename CLASS8,
    typename CLASS9
>
std::string   
sformat
(
    UINT            resid,
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6,
    CLASS7          const &obj7,
    CLASS8          const &obj8,
    CLASS9          const &obj9
)
{
    std::string s = loadString(resid);
    return sformat(s, obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8, obj9);
}


template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6,
    typename CLASS7,
    typename CLASS8,
    typename CLASS9,
    typename CLASS10
>
std::string   
sformat
(
    UINT            resid,
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6,
    CLASS7          const &obj7,
    CLASS8          const &obj8,
    CLASS9          const &obj9,
    CLASS10         const &obj10
)
{
    std::string s = loadString(resid);
    return 
        sformat(s, obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8, obj9, obj10);
}


template
<
    typename CLASS1
>
std::wstring   
wformat
(
    UINT            resid,
    CLASS1          const &obj1
)
{
    std::wstring s = wloadString(resid);
    return wformat(s, obj1);
}


template
<
    typename CLASS1,
    typename CLASS2
>
std::wstring  
wformat
(
    UINT            resid,
    CLASS1          const &obj1,
    CLASS2          const &obj2
)
{
    std::wstring s = wloadString(resid);
    return wformat(s, obj1, obj2);
}


template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3
>
std::wstring   
wformat
(
    UINT            resid,
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3
)
{
    std::wstring s = wloadString(resid);
    return wformat(s, obj1, obj2, obj3);
}


template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4
>
std::wstring  
wformat
(
    UINT            resid,
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4
)
{
    std::wstring s = wloadString(resid);
    return wformat(s, obj1, obj2, obj3, obj4);
}


template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5
>
std::wstring  
wformat
(
    UINT            resid,
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5
)
{
    std::wstring s = wloadString(resid);
    return wformat(s, obj1, obj2, obj3, obj4, obj5);
}


template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6
>
std::wstring  
wformat
(
    UINT            resid, 
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6
)
{
    std::wstring s = wloadString(resid);
    return wformat(s, obj1, obj2, obj3, obj4, obj5, obj6);
}


template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6,
    typename CLASS7
>
std::wstring  
wformat
(
    UINT            resid,
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6,
    CLASS7          const &obj7
)
{
    std::wstring s = wloadString(resid);
    return wformat(s, obj1, obj2, obj3, obj4, obj5, obj6, obj7);
}


template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6,
    typename CLASS7,
    typename CLASS8
>
std::wstring  
wformat
(
    UINT            resid,
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6,
    CLASS7          const &obj7,
    CLASS8          const &obj8
)
{
    std::wstring s = wloadString(resid);
    return wformat(s, obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8);
}

 
template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6,
    typename CLASS7,
    typename CLASS8,
    typename CLASS9
>
std::wstring  
wformat
(
    UINT            resid,
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6,
    CLASS7          const &obj7,
    CLASS8          const &obj8,
    CLASS9          const &obj9
)
{
    std::wstring s = wloadString(resid);
    return wformat(s, obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8, obj9);
}


template
<
    typename CLASS1,
    typename CLASS2,
    typename CLASS3,
    typename CLASS4,
    typename CLASS5,
    typename CLASS6,
    typename CLASS7,
    typename CLASS8,
    typename CLASS9,
    typename CLASS10
>
std::wstring  
wformat
(
    UINT            resid,
    CLASS1          const &obj1,
    CLASS2          const &obj2,
    CLASS3          const &obj3,
    CLASS4          const &obj4,
    CLASS5          const &obj5,
    CLASS6          const &obj6,
    CLASS7          const &obj7,
    CLASS8          const &obj8,
    CLASS9          const &obj9,
    CLASS10         const &obj10
)
{
    std::wstring s = wloadString(resid);
    return 
        wformat(s, obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8, obj9, obj10);
}


#endif // WIN32


#endif // FORMAT_HPP
