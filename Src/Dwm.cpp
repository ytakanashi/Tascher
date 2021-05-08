//Dwm.cpp
//Dwm操作関数

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.64
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/


#include"StdAfx.h"
#include"Dwm.h"

namespace dwm{

HMODULE g_hDwmApiDll;
//dwmapi.h
HRESULT(WINAPI*pDwmIsCompositionEnabled)(BOOL*);
HRESULT(WINAPI*pDwmGetWindowAttribute)(HWND,DWORD,LPCVOID,DWORD);
HRESULT(WINAPI*pDwmRegisterThumbnail)(HWND,HWND,PHTHUMBNAIL);
HRESULT(WINAPI*pDwmUnregisterThumbnail)(HTHUMBNAIL);
HRESULT(WINAPI*pDwmUpdateThumbnailProperties)(HTHUMBNAIL,const DWM_THUMBNAIL_PROPERTIES*);
HRESULT(WINAPI*pDwmQueryThumbnailSourceSize)(HTHUMBNAIL,PSIZE);

bool g_bLoaded;

bool IsLoaded(){
	return g_bLoaded;
}

//d2d1.dll/DWrite.dll読み込み
bool Load(){
	if(g_hDwmApiDll!=NULL)return true;

	g_bLoaded=false;

	TCHAR szDwmApiDllPath[MAX_PATH]={};

	GetSystemDirectory(szDwmApiDllPath,ARRAY_SIZEOF(szDwmApiDllPath));
	lstrcat(szDwmApiDllPath,_T("\\dwmapi.dll"));
	if((g_hDwmApiDll=LoadLibrary(szDwmApiDllPath))==NULL)return false;

	pDwmIsCompositionEnabled=(HRESULT(WINAPI*)(BOOL*))
		GetProcAddress(g_hDwmApiDll,"DwmIsCompositionEnabled");
	pDwmGetWindowAttribute=(HRESULT(WINAPI*)(HWND,DWORD,LPCVOID,DWORD))
		GetProcAddress(g_hDwmApiDll,"DwmGetWindowAttribute");
	pDwmRegisterThumbnail=(HRESULT(WINAPI*)(HWND,HWND,PHTHUMBNAIL))
		GetProcAddress(g_hDwmApiDll,"DwmRegisterThumbnail");
	pDwmUnregisterThumbnail=(HRESULT(WINAPI*)(HTHUMBNAIL))
		GetProcAddress(g_hDwmApiDll,"DwmUnregisterThumbnail");
	pDwmUpdateThumbnailProperties=(HRESULT(WINAPI*)(HTHUMBNAIL,const DWM_THUMBNAIL_PROPERTIES*))
		GetProcAddress(g_hDwmApiDll,"DwmUpdateThumbnailProperties");
	pDwmQueryThumbnailSourceSize=(HRESULT(WINAPI*)(HTHUMBNAIL,PSIZE))
		GetProcAddress(g_hDwmApiDll,"DwmQueryThumbnailSourceSize");

	BOOL bEnable=FALSE;

	g_bLoaded=pDwmIsCompositionEnabled&&
		pDwmGetWindowAttribute&&
			pDwmRegisterThumbnail&&
				pDwmUnregisterThumbnail&&
					pDwmUpdateThumbnailProperties&&
						pDwmQueryThumbnailSourceSize&&
							SUCCEEDED(pDwmIsCompositionEnabled(&bEnable))&&
								bEnable;

	return g_bLoaded;
}

//Dwmapi.dll解放
bool Unload(){
	if(g_hDwmApiDll!=NULL){
		FreeLibrary(g_hDwmApiDll);
		g_hDwmApiDll=NULL;
	}
	g_bLoaded=false;
	return true;
}

HRESULT GetWindowAttribute(HWND hWnd,DWORD dwAttribute,LPCVOID lpAttribute,DWORD dwSize){
	if(pDwmGetWindowAttribute==NULL)return E_FAIL;
	return pDwmGetWindowAttribute(hWnd,dwAttribute,lpAttribute,dwSize);
}

HRESULT RegisterThumbnail(HWND hwndDestination,HWND hwndSource,PHTHUMBNAIL phThumbnailId){
	if(pDwmRegisterThumbnail==NULL)return E_FAIL;
	return pDwmRegisterThumbnail(hwndDestination,hwndSource,phThumbnailId);
}

HRESULT UnregisterThumbnail(HTHUMBNAIL hThumbnailId){
	if(pDwmUnregisterThumbnail==NULL)return E_FAIL;
	return pDwmUnregisterThumbnail(hThumbnailId);
}

HRESULT UpdateThumbnailProperties(HTHUMBNAIL hThumbnailId,const DWM_THUMBNAIL_PROPERTIES* ptnProperties){
	if(pDwmUpdateThumbnailProperties==NULL)return E_FAIL;
	return pDwmUpdateThumbnailProperties(hThumbnailId,ptnProperties);
}

HRESULT QueryThumbnailSourceSize(HTHUMBNAIL hThumbnail,PSIZE pSize){
	if(pDwmQueryThumbnailSourceSize==NULL)return E_FAIL;
	return pDwmQueryThumbnailSourceSize(hThumbnail,pSize);
}

}//namespace dwm
