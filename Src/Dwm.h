//Dwm.h
//Dwm操作関数

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.64
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/


#ifndef TASCHER_DWM_H
#define TASCHER_DWM_H


typedef HANDLE HTHUMBNAIL;
typedef HTHUMBNAIL* PHTHUMBNAIL;
typedef enum _DWMWINDOWATTRIBUTE{
	DWMWA_NCRENDERING_ENABLED=1,
	DWMWA_NCRENDERING_POLICY,
	DWMWA_TRANSITIONS_FORCEDISABLED,
	DWMWA_ALLOW_NCPAINT,
	DWMWA_CAPTION_BUTTON_BOUNDS,
	DWMWA_NONCLIENT_RTL_LAYOUT,
	DWMWA_FORCE_ICONIC_REPRESENTATION,
	DWMWA_FLIP3D_POLICY,
	DWMWA_EXTENDED_FRAME_BOUNDS,
	DWMWA_HAS_ICONIC_BITMAP,
	DWMWA_DISALLOW_PEEK,
	DWMWA_EXCLUDED_FROM_PEEK,
	DWMWA_CLOAK,
	DWMWA_CLOAKED,
	DWMWA_FREEZE_REPRESENTATION,
	DWMWA_LAST
}DWMWINDOWATTRIBUTE;
#define DWM_TNP_RECTDESTINATION       0x00000001
#define DWM_TNP_RECTSOURCE            0x00000002
#define DWM_TNP_OPACITY               0x00000004
#define DWM_TNP_VISIBLE               0x00000008
#define DWM_TNP_SOURCECLIENTAREAONLY  0x0000001
typedef struct _DWM_THUMBNAIL_PROPERTIES{
	DWORD dwFlags;
	RECT rcDestination;
	RECT rcSource;
	BYTE opacity;
	BOOL fVisible;
	BOOL fSourceClientAreaOnly;
}DWM_THUMBNAIL_PROPERTIES,*PDWM_THUMBNAIL_PROPERTIES;



namespace dwm{
//Dwmapi.dll読み込み
bool Load();
//Dwmapi.dll解放
bool Unload();

bool IsLoaded();

HRESULT GetWindowAttribute(HWND hWnd,DWORD dwAttribute,LPCVOID lpAttribute,DWORD dwSize);

HRESULT RegisterThumbnail(HWND hwndDestination,HWND hwndSource,PHTHUMBNAIL phThumbnailId);

HRESULT UnregisterThumbnail(HTHUMBNAIL hThumbnailId);

HRESULT UpdateThumbnailProperties(HTHUMBNAIL hThumbnailId,const DWM_THUMBNAIL_PROPERTIES* ptnProperties);

HRESULT QueryThumbnailSourceSize(HTHUMBNAIL hThumbnail,PSIZE pSize);



}//namespace dwm

#endif //TASCHER_DWM_H
