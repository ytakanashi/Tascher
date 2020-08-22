//StdAfx.h
//共通ヘッダファイル

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.63
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/


#ifndef COMMON_H
#define COMMON_H

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
		#define TASCHER_VERSION _T("Tascher Ver.1.63 <Debug>")
	#else
		#define TASCHER_VERSION _T("Tascher Ver.1.63 x64 <Debug>")
	#endif
#else
	#ifndef _WIN64
		#define TASCHER_VERSION _T("Tascher Ver.1.63")
	#else
		#define TASCHER_VERSION _T("Tascher Ver.1.63 x64")
	#endif
#endif

#ifndef _DEBUG
#ifdef _MSC_VER
#pragma function(memset)
void* __cdecl memset(void *pTarget,int value,size_t cb){
	char* p=(char*)pTarget;
	while(cb--)*p++=(char)value;
	return pTarget;
}
#pragma function(memcpy)
void* __cdecl memcpy(void* pDest,const void * pSrc,size_t cb){
	void* pResult=pDest;
	while(cb--){
		*(char*)pDest=*(char*)pSrc;
		pDest=(char*)pDest+1;
		pSrc=(char*)pSrc+1;
	}
	return pResult;
}
#endif
#if __cplusplus
extern "C"
#endif
int _fltused=1;
#endif

//配列の要素数
#define ARRAY_SIZEOF(array) (sizeof(array)/sizeof(array[0]))

#endif //COMMON_H
