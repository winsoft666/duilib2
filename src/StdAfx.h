#if !defined(AFX_STDAFX_H__E30B2003_188B_4EB4_AB99_3F3734D6CE6C__INCLUDED_)
#define AFX_STDAFX_H__E30B2003_188B_4EB4_AB99_3F3734D6CE6C__INCLUDED_

#pragma once

#ifndef __FILET__
    #define __DUILIB_STR2WSTR(str)	L##str
    #define _DUILIB_STR2WSTR(str)	__DUILIB_STR2WSTR(str)
    #ifdef _UNICODE
        #define __FILET__	_DUILIB_STR2WSTR(__FILE__)
        #define __FUNCTIONT__	_DUILIB_STR2WSTR(__FUNCTION__)
    #else
        #define __FILET__	__FILE__
        #define __FUNCTIONT__	__FUNCTION__
    #endif
#endif

#define _CRT_SECURE_NO_DEPRECATE

// Remove pointless warning messages
#ifdef _MSC_VER
    #pragma warning (disable : 4511) // copy operator could not be generated
    #pragma warning (disable : 4512) // assignment operator could not be generated
    #pragma warning (disable : 4702) // unreachable code (bugs in Microsoft's STL)
    #pragma warning (disable : 4786) // identifier was truncated
    #pragma warning (disable : 4996) // function or variable may be unsafe (deprecated)
    #ifndef _CRT_SECURE_NO_WARNINGS
        #define _CRT_SECURE_NO_WARNINGS // eliminate deprecation warnings for VS2005
    #endif
#endif // _MSC_VER

#ifdef UILIB_WITH_CEF
    #include "Internal/Cef/CefGloablContext.h"
#endif
#include "UILibInternal.h"
#include "Internal/stb_image.h"
#include "Internal/unzip.h"

#include <olectl.h>
#include <Shlwapi.h>



#define CLAMP(x,a,b) (MIN(b,MAX(a,x)))


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__E30B2003_188B_4EB4_AB99_3F3734D6CE6C__INCLUDED_)
