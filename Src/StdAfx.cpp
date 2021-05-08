/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.64
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/


#include"StdAfx.h"

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
	int _fltused=1;
#endif
