//StdAfx.h
//共通ヘッダファイル

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.64
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/


#ifndef TASCHER_STDAFX_H
#define TASCHER_STDAFX_H

#ifndef STRICT
	#define STRICT
#endif
#ifndef UNICODE
	#define UNICODE
#endif
#ifndef _UNICODE
	#define _UNICODE
#endif

#ifndef _WIN32_WINNT
	#define _WIN32_WINNT 0x0501
#endif

#ifndef WINVER
	#define WINVER 0x0501
#endif

#ifndef _WIN32_IE
	#define _WIN32_IE _WIN32_IE_IE55
#endif

#include<windows.h>
#include<windowsx.h>
#include<tchar.h>
#include<commctrl.h>
#include<shlwapi.h>


#ifdef _DEBUG
	#ifndef _WIN64
		#define TASCHER_VERSION _T("Tascher Ver.1.64 <Debug>")
	#else
		#define TASCHER_VERSION _T("Tascher Ver.1.64 x64 <Debug>")
	#endif
#else
	#ifndef _WIN64
		#define TASCHER_VERSION _T("Tascher Ver.1.64")
	#else
		#define TASCHER_VERSION _T("Tascher Ver.1.64 x64")
	#endif
#endif

#ifndef _DEBUG
	#ifdef _MSC_VER
		extern "C" void * __cdecl memset(void *, int, size_t);
		#pragma intrinsic(memset)
		extern "C" void* __cdecl memcpy(void* pDest,const void * pSrc,size_t cb);
		#pragma intrinsic(memset)
	#endif
	#if __cplusplus
		extern "C"
	#endif
	int _fltused;
#endif

//配列の要素数
#define ARRAY_SIZEOF(array) (sizeof(array)/sizeof(array[0]))

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) {if(p!=NULL){(p)->Release();(p)=NULL;}}
#endif

#endif //TASCHER_STDAFX_H
