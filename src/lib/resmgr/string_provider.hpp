/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef STRING_PROVIDER_HPP__
#define STRING_PROVIDER_HPP__

#include "datasection.hpp"
#include "cstdmf/smartpointer.hpp"
#include "cstdmf/string_utils.hpp"
#include <string>
#include <cstring>
#include <stdio.h>
#include <sstream>
#include <vector>
#include <set>
#include <map>

class Formatter
{
	std::wstring str_;
public:
	Formatter(){}
	Formatter( const std::wstring& str )	: str_( str ){}
	Formatter( const wchar_t* str )	: str_( str ){}
	Formatter( const std::string& str )	
	{
		// assume utf8, if not, it *should* be in ASCII, which is utf8-safe.
		bw_utf8tow( str, str_ );
	}
	Formatter( const char* str )
	{
		// assume utf8, if not, it *should* be in ASCII, which is utf8-safe.
		bw_utf8tow( str, str_ );
	}
	Formatter( float f, const wchar_t* format = L"%g" )
	{
		wchar_t s[1024];
		bw_snwprintf( s, ARRAY_SIZE( s ), format, f );
		s[ ARRAY_SIZE( s ) - 1 ] = 0;
		str_ = s;
	}
	Formatter( double d, const wchar_t* format = L"%g" )
	{
		wchar_t s[1024];
		bw_snwprintf( s, ARRAY_SIZE( s ), format, d );
		s[ ARRAY_SIZE( s ) - 1 ] = 0;
		str_ = s;
	}
	Formatter( int i, const wchar_t* format = L"%d" )
	{
		wchar_t s[1024];
		bw_snwprintf( s, ARRAY_SIZE( s ), format, i );
		s[ ARRAY_SIZE( s ) - 1 ] = 0;
		str_ = s;
	}
	Formatter( unsigned int ui, const wchar_t* format = L"%u" )
	{
		wchar_t s[1024];
		bw_snwprintf( s, ARRAY_SIZE( s ), format, ui );
		s[ ARRAY_SIZE( s ) - 1 ] = 0;
		str_ = s;
	}
	Formatter( unsigned long ul, const wchar_t* format = L"%u" )
	{
		wchar_t s[1024];
		bw_snwprintf( s, ARRAY_SIZE( s ), format, ul );
		s[ ARRAY_SIZE( s ) - 1 ] = 0;
		str_ = s;
	}
	Formatter( char ch, const wchar_t* format = L"%c" )
	{
		wchar_t s[1024];
		bw_snwprintf( s, ARRAY_SIZE( s ), format, ch );
		s[ ARRAY_SIZE( s ) - 1 ] = 0;
		str_ = s;
	}
	Formatter( unsigned char ch, const wchar_t* format = L"%c" )
	{
		wchar_t s[1024];
		bw_snwprintf( s, ARRAY_SIZE( s ), format, ch );
		s[ ARRAY_SIZE( s ) - 1 ] = 0;
		str_ = s;
	}
	Formatter( void* p, const wchar_t* format = L"%p" )
	{
		wchar_t s[1024];
		bw_snwprintf( s, ARRAY_SIZE( s ), format, p );
		s[ ARRAY_SIZE( s ) - 1 ] = 0;
		str_ = s;
	}
	const std::wstring& str() const {	return str_;	}
};


// for index out of the range, set it to empty string
inline const wchar_t* formatString( const wchar_t* format, const Formatter& f1 = Formatter(), const Formatter& f2 = Formatter(),
						 const Formatter& f3 = Formatter(), const Formatter& f4 = Formatter(),
						 const Formatter& f5 = Formatter(), const Formatter& f6 = Formatter(),
						 const Formatter& f7 = Formatter(), const Formatter& f8 = Formatter() )
{
	static const int MAX_LOCALISED_STRING_LENGTH = 10240;
#ifdef WIN32
	__declspec(thread) static wchar_t sz[ MAX_LOCALISED_STRING_LENGTH + 1 ];
#else//WIN32
	static wchar_t sz[ MAX_LOCALISED_STRING_LENGTH + 1 ];
#endif//WIN32

	std::wstring strs[ 8 ];
	strs[ 0 ] = f1.str();	strs[ 1 ] = f2.str();	strs[ 2 ] = f3.str();	strs[ 3 ] = f4.str();
	strs[ 4 ] = f5.str();	strs[ 5 ] = f6.str();	strs[ 6 ] = f7.str();	strs[ 7 ] = f8.str();
	std::wstring result;
	while( const wchar_t* escape = std::wcschr( format, L'%' ) )
	{
		result += std::wstring( format, escape - format );
		format = escape;
		if( format[1] == 0 )
		{
			++format;
			break;
		}
		if( format[1] == L'%' )
		{
			result += L'%';
			++format;
		}
		else if( format[1] >= L'0' && format[1] <= L'7' )
		{
			result += strs[ format[1] - L'0' ];
			++format;
		}
		/*
		else
			0;// wrong escape ==> empty strings
		*/
		++format;
	}
	result += format;

	std::wcsncpy( sz, result.c_str(), MAX_LOCALISED_STRING_LENGTH );

	sz[ MAX_LOCALISED_STRING_LENGTH ] = 0;
	return sz;
}

#define LANGUAGE_NAME_TAG ( L"LanguageName" )
#define ENGLISH_LANGUAGE_NAME_TAG ( L"EnglishLanguageName" )
#define DEFAULT_LANGUAGE_NAME ( L"en" )
#define DEFAULT_COUNTRY_NAME ( L"us" )

class StringID
{
public:
	StringID() : key_(0) {}
	//explicit StringID( const char* str );
	explicit StringID( const wchar_t* str );
	unsigned int key() const	{	return key_;	}
	const std::wstring& str() const	{	return str_;	}
	bool operator<( const StringID& that ) const;// should be a free function, but ...
private:
	std::wstring str_;
	unsigned int key_;
};

class Language : public SafeReferenceCount
{
public:
	virtual void load( DataSectionPtr language, const std::wstring& root = L"" ) = 0;
	virtual const wchar_t* getLanguageName() const = 0;
	virtual const wchar_t* getLanguageEnglishName() const = 0;
	virtual const std::wstring& getIsoLangName() const = 0;
	virtual const std::wstring& getIsoCountryName() const = 0;
	virtual const wchar_t* getString( const StringID& id ) const = 0;
	static std::pair<std::wstring, std::wstring> splitIsoLangCountryName( const std::wstring& isoLangCountryName );
	static const int ISO_NAME_LENGTH = 2;

	inline std::string getIsoLangNameUTF8() const
	{
		std::string langName;
		bw_wtoutf8( getIsoLangName(), langName );
		return langName;
	}
	inline std::string getIsoCountryNameUTF8() const
	{
		std::string countryName;
		bw_wtoutf8( getIsoCountryName(), countryName );
		return countryName;
	}
};

typedef SmartPointer<Language> LanguagePtr;

struct LanguageNotifier
{
	LanguageNotifier();
	virtual ~LanguageNotifier();
	virtual void changed() = 0;
};

/**
	Localised String Provider, after setting to the appropriate language/country
	You can call localised string provider with an id to get back a string
 */

class StringProvider
{
	std::set<LanguageNotifier*> notifiers_;
	std::vector<LanguagePtr> languages_;
	LanguagePtr currentLanguage_;
	LanguagePtr currentMainLanguage_;
	LanguagePtr defaultLanguage_;

	const wchar_t* str( const StringID& id ) const;
	const wchar_t* formatString( const StringID& formatID, const Formatter& f1 = Formatter(), const Formatter& f2 = Formatter(),
						 const Formatter& f3 = Formatter(), const Formatter& f4 = Formatter(),
						 const Formatter& f5 = Formatter(), const Formatter& f6 = Formatter(),
						 const Formatter& f7 = Formatter(), const Formatter& f8 = Formatter() )
	{
		return ::formatString( str( formatID ), f1, f2, f3, f4, f5, f6, f7, f8 );
	}
public:
	enum DefResult
	{
		RETURN_NULL_IF_NOT_EXISTING,
		RETURN_PARAM_IF_NOT_EXISTING
	};


	void load( DataSectionPtr file );
	unsigned int languageNum() const;
	LanguagePtr getLanguage( unsigned int index ) const;

	void setLanguage();
	void setLanguage( unsigned int language );
	void setLanguages( const std::wstring& langName, const std::wstring& countryName );

	const wchar_t* str( const wchar_t* id, DefResult def = RETURN_PARAM_IF_NOT_EXISTING ) const;

	const wchar_t* formatString( const wchar_t* formatID, const Formatter& f1 = Formatter(), const Formatter& f2 = Formatter(),
						 const Formatter& f3 = Formatter(), const Formatter& f4 = Formatter(),
						 const Formatter& f5 = Formatter(), const Formatter& f6 = Formatter(),
						 const Formatter& f7 = Formatter(), const Formatter& f8 = Formatter() )
	{
		return ::formatString( str( formatID ), f1, f2, f3, f4, f5, f6, f7, f8 );
	}


	LanguagePtr currentLanguage() const;

	static StringProvider& instance();

	void registerNotifier( LanguageNotifier* notifier );
	void unregisterNotifier( LanguageNotifier* notifier );
	void notify();
};

inline const wchar_t* formatLocalisedString( const wchar_t* format, const Formatter& f1, const Formatter& f2 = Formatter(),
						 const Formatter& f3 = Formatter(), const Formatter& f4 = Formatter(),
						 const Formatter& f5 = Formatter(), const Formatter& f6 = Formatter(),
						 const Formatter& f7 = Formatter(), const Formatter& f8 = Formatter() )
{
	return StringProvider::instance().formatString( format, f1, f2, f3, f4, f5, f6, f7, f8 );
}

inline const wchar_t* Localise( const wchar_t* key, StringProvider::DefResult def = StringProvider::RETURN_PARAM_IF_NOT_EXISTING )
{
	return StringProvider::instance().str( key, def );
}

//-----------------------------------------------------------------------------
inline const wchar_t* Localise( const wchar_t* format, const Formatter& f1, const Formatter& f2 = Formatter(),
						 const Formatter& f3 = Formatter(), const Formatter& f4 = Formatter(),
						 const Formatter& f5 = Formatter(), const Formatter& f6 = Formatter(),
						 const Formatter& f7 = Formatter(), const Formatter& f8 = Formatter() )
{
	return formatLocalisedString( format, f1, f2, f3, f4, f5, f6, f7, f8 );
}

inline std::string LocaliseUTF8( const wchar_t* format, const Formatter& f1, const Formatter& f2 = Formatter(),
								const Formatter& f3 = Formatter(), const Formatter& f4 = Formatter(),
								const Formatter& f5 = Formatter(), const Formatter& f6 = Formatter(),
								const Formatter& f7 = Formatter(), const Formatter& f8 = Formatter() )
{
	std::string localisedString;
	bw_wtoutf8( formatLocalisedString( format, f1, f2, f3, f4, f5, f6, f7, f8 ), localisedString );
	return localisedString;
}

#ifdef WIN32
inline std::string LocaliseACP( const wchar_t* format, const Formatter& f1, const Formatter& f2 = Formatter(),
								const Formatter& f3 = Formatter(), const Formatter& f4 = Formatter(),
								const Formatter& f5 = Formatter(), const Formatter& f6 = Formatter(),
								const Formatter& f7 = Formatter(), const Formatter& f8 = Formatter() )
{
	std::string localisedString;
	bw_wtoacp( formatLocalisedString( format, f1, f2, f3, f4, f5, f6, f7, f8 ), localisedString );
	return localisedString;
}
#endif

//-----------------------------------------------------------------------------
// from WIDE key to UTF8/ACP
inline std::string LocaliseUTF8( const wchar_t * key, StringProvider::DefResult def = StringProvider::RETURN_PARAM_IF_NOT_EXISTING )
{
	std::string localisedString;
	bw_wtoutf8( StringProvider::instance().str( key, def ), localisedString );
	return localisedString;
}

#ifdef WIN32
inline std::string LocaliseACP( const wchar_t * key, StringProvider::DefResult def = StringProvider::RETURN_PARAM_IF_NOT_EXISTING )
{
	std::string localisedString;
	bw_wtoacp( StringProvider::instance().str( key, def ), localisedString );
	return localisedString;
}
#endif

//-----------------------------------------------------------------------------
// from NARROW key to UTF8/ACP
inline std::string LocaliseUTF8( const char * key, StringProvider::DefResult def = StringProvider::RETURN_PARAM_IF_NOT_EXISTING )
{
	std::wstring wkey;
	bw_ansitow( key, wkey );
	return LocaliseUTF8( wkey.c_str(), def );
}

#ifdef WIN32
inline std::string LocaliseACP( const char * key, StringProvider::DefResult def = StringProvider::RETURN_PARAM_IF_NOT_EXISTING )
{
	std::wstring wkey;
	bw_ansitow( key, wkey );
	return LocaliseACP( wkey.c_str(), def );
}
#endif

inline bool isLocaliseToken( const char* s )
{
	return s && s[0] == '`';
}

inline bool isLocaliseToken( const wchar_t* s )
{
	return s && s[0] == L'`';
}


//-----------------------------------------------------------------------------
// Localise caches for speed.
inline std::string LocaliseStaticUTF8( const wchar_t * key )
{
	typedef std::map< const wchar_t *, std::string > LocaliseCache;
	static LocaliseCache s_cache;

	LocaliseCache::const_iterator it = s_cache.find( key );

	if (it == s_cache.end())
	{
		it = s_cache.insert(
				std::make_pair( key, LocaliseUTF8( key ) ) ).first;
	}

	return (*it).second;
}



#ifdef WIN32
#include <commctrl.h>
class WindowTextNotifier : LanguageNotifier
{
	std::map<HWND, StringID> windows_;
	std::map<HWND, std::vector<StringID> > combos_;
	std::map<std::pair<HMENU, UINT>, StringID> menus_;
	std::map<HWND, WNDPROC> subClassMap_;
	HHOOK callWinRetHook_;
	HHOOK callWinHook_;
	WNDPROC comboWndProc_;
	WNDPROC toolTipProc_;
public:
	WindowTextNotifier();
	~WindowTextNotifier();

	void set( HWND hwnd, const wchar_t* id );

	void set( HMENU menu );

	void addComboString( HWND hwnd, const wchar_t* id );
	void deleteComboString( HWND hwnd, std::vector<StringID>::size_type index );
	void insertComboString( HWND hwnd, std::vector<StringID>::size_type index, const wchar_t* id );
	void resetContent( HWND hwnd );

	virtual void changed();

	static WindowTextNotifier& instance();
	static void fini();
	static LRESULT CALLBACK CallWndRetProc( int nCode, WPARAM wParam, LPARAM lParam );
	static LRESULT CALLBACK CallWndProc( int nCode, WPARAM wParam, LPARAM lParam );
	static LRESULT CALLBACK ComboProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
	static LRESULT CALLBACK ToolTipProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
	static LRESULT CALLBACK ToolTipParentProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
};

inline void localiseWindowText( HWND hwnd, const wchar_t* id )
{
	WindowTextNotifier::instance().set( hwnd, id );
}

inline void Localise( HWND hwnd, const wchar_t* id )
{
	localiseWindowText( hwnd, id );
}

std::string localiseFileName( const std::string& filename,
	LanguagePtr lang = StringProvider::instance().currentLanguage() );


#endif//WIN32


#endif//STRING_PROVIDER_HPP__
