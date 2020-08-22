//Tascher.cpp
//メイン

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.63
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/


#include"StdAfx.h"
#include"CommonSettings.h"
#include"Function.h"
#include"Path.h"
#include"PrivateProfile.h"
#include"Settings.h"
#include"resources/resource.h"
#include<gdiplus.h>
#include<psapi.h>
#include<tlhelp32.h>
#include<shellapi.h>
#include<Shlobj.h>

#ifndef WS_EX_NOREDIRECTIONBITMAP
	#define WS_EX_NOREDIRECTIONBITMAP 0x00200000L
#endif

//[各種コントロールのID]
#define ID_TRAYMENU 201

#define IDC_LISTVIEW 100

#define ID_TIMER_HOVER 202
#define ID_TIMER_UNIQUE_WINDOW 203
#define ID_TIMER_HIDELISTVIEW 204
#define ID_TIMER_THUMBNAIL 205
#define ID_TIMER_READY 206
#define ID_TIMER_MOUSE_MOVE_LEFTTOP 207
#define ID_TIMER_MOUSE_MOVE_TOP 208
#define ID_TIMER_MOUSE_MOVE_RIGHTTOP 209
#define ID_TIMER_MOUSE_MOVE_LEFT 210
#define ID_TIMER_MOUSE_MOVE_RIGHT 211
#define ID_TIMER_MOUSE_MOVE_LEFTBOTTOM 212
#define ID_TIMER_MOUSE_MOVE_BOTTOM 213
#define ID_TIMER_MOUSE_MOVE_RIGHTBOTTOM 214
#define ID_TIMER_HIDE_CURSOR 215
#define ID_TIMER_CLIP_CURSOR 216

#define ID_TOOLTIP_SEARCH 300
#define ID_TOOLTIP_INFO 301

#define ID_HOTKEY 1002


//[ハンドル系]
//メインウインドウハンドル
HWND g_hMainWnd;
//インスタンスハンドル
HINSTANCE g_hInstance;
//タスクトレイ用アイコン
HICON g_hTrayIcon;
HICON g_hDisableTrayIcon;

//プロシージャ保存用
WNDPROC DefListViewProc;
//リストビューハンドル
HWND g_hListView;

//検索文字列用ツールチップハンドル
HWND g_hToolTipSearch;
//情報表示用ツールチップハンドル
HWND g_hToolTipInfo;

//表示中メニューのウインドウハンドル
HWND g_hMenuWnd;

//Migemo,bregexp
HMODULE g_hMigemo;
HMODULE g_hBregonig;

//dwmapi
HMODULE g_hDwmApiDll;
HWND g_hThumbnailWnd;
bool g_bDwmApi;
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
HRESULT(WINAPI*pDwmIsCompositionEnabled)(BOOL*);
HRESULT(WINAPI*pDwmGetWindowAttribute)(HWND,DWORD,LPCVOID,DWORD);
HRESULT(WINAPI*pDwmRegisterThumbnail)(HWND,HWND,PHTHUMBNAIL);
HRESULT(WINAPI*pDwmUnregisterThumbnail)(HTHUMBNAIL);
HRESULT(WINAPI*pDwmUpdateThumbnailProperties)(HTHUMBNAIL,const DWM_THUMBNAIL_PROPERTIES*);
HRESULT(WINAPI*pDwmQueryThumbnailSourceSize)(HTHUMBNAIL,PSIZE);


//アクセラレータテーブルハンドル
HACCEL g_hAccel;

//リストビュー用フォント
HFONT g_hDefaultItemFont;
HFONT g_hSelectedItemFont;

#if BACKGROUND_NO_GDIPLUS
//表示用のHBITMAP
HBITMAP g_hBitmap;
//オリジナルのHBITMAP
HBITMAP g_hBitmapOriginal;
#else
#include<gdiplus.h>
//表示用Bitmap
Gdiplus::Bitmap* g_pBitmap;
//バックップ用Bitmap
Gdiplus::Bitmap* g_pBitmapBack;
//オリジナルのBitmap
Gdiplus::Bitmap* g_pBitmapOriginal;
#endif

//設定構造体
struct CONFIG g_Config;

//[愉快なフラグたち]
//無効
bool g_bDisable;

//設定ダイアログが表示されている
bool g_bSettingsDialog;

//ウインドウリストの状態
enum WINDOWLIST_STATUS{
	WLS_HIDE=0,
	WLS_CREATING,
	WLS_CREATED,
	WLS_DISPLAYED,
	WLS_SWITCHING,
}g_eWindowListStatus;

//四隅表示のコーナー種
SCC_CORNERS g_eCorners;

//背景画像が有効である
bool g_bBackground;

//ホットキーから表示
bool g_bHotKeyMode;

//インクリメンタルサーチでの検索方法
int g_iMatchMode;

//左側の項目のみ検索する
bool g_bFirstColumnOnly;

//Migemoで検索
bool g_bMigemoSearch;

//Migemo検索中マッチするアイテムが無い
bool g_bMigemoNomatch;

//候補が1つなら切り替える
bool g_bEnterUniqueWindow;

//不透明度
BYTE g_byOpacity;

//修飾キーの状態
BYTE g_byModKey;

//サムネイル設定
UINT g_uThumbnail,g_uThumbnailBak;

//サムネイルを元の大きさで表示
bool g_bThumbnailOriginalSize;

//非表示用カーソル
HCURSOR g_hBlankCursor;

typedef struct{
	HMENU hMenu;
	HMENU hSubMenu;
}MENU;

typedef struct{
	HANDLE hThread;
	DWORD dwThreadId;
}THREAD;

//[ウインドウリスト関係]
//表示項目
typedef struct{
	HICON hIcon;//アイコン
	TCHAR szFileName[MAX_PATH];//ファイル名
	TCHAR szWindowTitle[MAX_PATH];//ウインドウタイトル

	//非表示項目
	TCHAR szFilePath[MAX_PATH];//ファイルパス
	HWND hWnd;//ウインドウハンドル
	bool bForegorund;//フォアグラウンドウインドウである
	bool bIconic;//最小化(アイコン化)されている
	bool bTopMost;//最前面ウインドウである
	bool bDesktopItem;//「デスクトップ」である
	bool bCancelItem;//「キャンセル」である
	bool bUWPApp;//UWPAppである
	HWND hUICoreWnd;//UWPAppのWindows.UI.Core.CoreWindowであるウインドウハンドル
	HTHUMBNAIL hThumbnail;
	THREAD Thread;
}WINDOW_INFO,*LPWINDOW_INFO;

//ウインドウの情報用ヒープ
HANDLE g_hHeapWindowInfo;

//列挙されたウインドウの情報
WINDOW_INFO* g_WindowInfo;

//列挙されたウインドウ数
int g_iWindowCount;

//表示するウインドウの情報
LPWINDOW_INFO* g_DisplayWindowInfo;

//表示するウインドウ数
int g_iDisplayWindowCount;

//フレームの幅、高さ
LONG g_lFrameWidth,g_lFrameHeight;

#define INCREMENTAL_SEARCH_BUFFER 32
TCHAR g_szSearchString[INCREMENTAL_SEARCH_BUFFER];

struct LISTVIEWCOLUMN_TABLE g_ListViewColumn_Table[LISTITEM_NUM];

//RegisterWindowMessage()するメッセージ
UINT MSG_MOUSEMOVE;//[マウスの移動]
UINT MSG_MOUSEWHEEL;//マウスホイール
UINT MSG_TRAY;//タスクトレイ用メッセージ
UINT MSG_TRAYCREATED;//タスクバー再作成用メッセージ

//X1、X2クリック用独自メッセージ
#define WM_X1BUTTONDOWN WM_APP+1
#define WM_X2BUTTONDOWN WM_APP+2
#define WM_X1BUTTONDBLCLK WM_APP+3
#define WM_X2BUTTONDBLCLK WM_APP+4

//情報取得用スレッド
#define WM_GETWINDOWINFO WM_APP+5
enum{
	GWI_ICON=0,
	GWI_TITLE,
};
//メインウインドウ→スレッド終了通知
#define WM_EXITGETWINDOWINFOTHREAD WM_APP+6
//TaskSwitch用スレッド
//スレッド→メインウインドウ終了通知
#define WM_CLOSETHREAD WM_APP+7

struct{
	SCC_CORNERS eCorners;
	UINT uId;
}const SccTimerTable[]={
	{SCC_LEFTTOP,ID_TIMER_MOUSE_MOVE_LEFTTOP},
	{SCC_TOP,ID_TIMER_MOUSE_MOVE_TOP},
	{SCC_RIGHTTOP,ID_TIMER_MOUSE_MOVE_RIGHTTOP},
	{SCC_LEFT,ID_TIMER_MOUSE_MOVE_LEFT},
	{SCC_RIGHT,ID_TIMER_MOUSE_MOVE_RIGHT},
	{SCC_LEFTBOTTOM,ID_TIMER_MOUSE_MOVE_LEFTBOTTOM},
	{SCC_BOTTOM,ID_TIMER_MOUSE_MOVE_BOTTOM},
	{SCC_RIGHTBOTTOM,ID_TIMER_MOUSE_MOVE_RIGHTBOTTOM},
};


//ダブルバッファ[XP以降]
#ifndef LVS_EX_DOUBLEBUFFER
	#define LVS_EX_DOUBLEBUFFER 0x00010000
#endif


//Migemo
#define MIGEMO_OPINDEX_NEWLINE 5
typedef struct _migemo migemo;
typedef migemo*(__stdcall*MIGEMO_OPEN)(const char* dict);
typedef void(__stdcall*MIGEMO_CLOSE)(migemo* object);
typedef unsigned char*(__stdcall*MIGEMO_QUERY)(migemo* object,const unsigned char* query);
typedef void(__stdcall*MIGEMO_RELEASE)(migemo* object,unsigned char* string);
typedef int(__stdcall*MIGEMO_LOAD)(migemo* obj,int dict_id,const char* dict_file);
typedef int(__stdcall*MIGEMO_IS_ENABLE)(migemo* obj);
typedef int(__stdcall*MIGEMO_SET_OPERATOR)(migemo* object,int index,const unsigned char* op);
typedef int (*MIGEMO_PROC_INT2CHAR)(unsigned int, unsigned char*);
typedef void(__stdcall*MIGEMO_SETPROC_INT2CHAR)(migemo* object,MIGEMO_PROC_INT2CHAR proc);
MIGEMO_OPEN migemo_open;
MIGEMO_CLOSE migemo_close;
MIGEMO_QUERY migemo_query;
MIGEMO_RELEASE migemo_release;
MIGEMO_LOAD migemo_load;
MIGEMO_IS_ENABLE migemo_is_enable;
MIGEMO_SET_OPERATOR migemo_set_operator;
MIGEMO_SETPROC_INT2CHAR migemo_setproc_int2char;

//bregexp
typedef struct bregexp{
	const TCHAR* outp;
	const TCHAR* outendp;
	const int splitctr;
	const TCHAR** splitp;
	INT_PTR rsv1;
	TCHAR* parap;
	TCHAR* paraendp;
	TCHAR* transtblp;
	TCHAR** startp;
	TCHAR** endp;
	int nparens;
}BREGEXP;

#define BREGEXP_MAX_ERROR_MESSAGE_LEN 80

typedef int(*BMATCH)(TCHAR* str,TCHAR* target,TCHAR* targetendp,BREGEXP** rxp,TCHAR* msg);
typedef void(*BREGFREE)(BREGEXP* rx);
BMATCH BMatch;
BREGFREE BRegfree;

//Migemoオブジェクト
migemo* g_pmigemo;



//マウスフックインストール
bool RegisterRawInput(HWND hWnd){
	UINT nDevices;

	if(GetRawInputDeviceList(NULL,&nDevices,sizeof(RAWINPUTDEVICELIST))!=0){
		return false;
	}

	RAWINPUTDEVICE Rid[1];
	Rid[0].usUsagePage=0x01;
	Rid[0].usUsage=0x02;
	Rid[0].dwFlags=RIDEV_INPUTSINK;
	Rid[0].hwndTarget=hWnd;

	return RegisterRawInputDevices(Rid,1,sizeof(Rid[0]))!=0;
}

//マウスフック(リストビュー)アンインストール
bool UnregisterRawInput(){
	RAWINPUTDEVICE Rid[1];
	Rid[0].usUsagePage=0x01;
	Rid[0].usUsage=0x02;
	Rid[0].dwFlags=RIDEV_REMOVE;
	Rid[0].hwndTarget=NULL;

	return RegisterRawInputDevices(Rid,1,sizeof(Rid[0]))!=0;
}

//Migemo/bregonig解放
bool UnloadMigemo(){
	if(migemo_close!=NULL)migemo_close(g_pmigemo);
	g_pmigemo=NULL;
	migemo_open=NULL;
	migemo_close=NULL;
	migemo_query=NULL;
	migemo_release=NULL;
	migemo_load=NULL;
	migemo_is_enable=NULL;
	migemo_set_operator=NULL;
	FreeLibrary(g_hMigemo);
	g_hMigemo=NULL;
	BMatch=NULL;
	BRegfree=NULL;
	FreeLibrary(g_hBregonig);
	g_hBregonig=NULL;
	return true;
}

int int2char(unsigned int in,unsigned char* out){
	if(in<0x80||!out){
		switch(in){
			//デフォルトではエスケープされない文字を処理
			case '[':
			case ']':
			case '~':
				if(out){
					out[0]='\\';
					out[1]=in&0xFF;
				}
				return 2;
		}
		//default_int2charに投げる
		return 0;
	}

	//UTF-8向け処理
	if(in<0x800){
		out[0]=0xc0+(in>>6);
		out[1]=0x80+(in&0x3f);
		return 2;
	}else if(in<0x10000){
		out[0]=0xe0+(in>>12);
		out[1]=0x80+((in>>6)&0x3f);
		out[2]=0x80+(in&0x3f);
		return 3;
	}else if(in<0x200000){
		out[0]=0xf0+(in>>18);
		out[1]=0x80+((in>>12)&0x3f);
		out[2]=0x80+((in>>6)&0x3f);
		out[3]=0x80+(in&0x3f);
		return 4;
	}else if(in<0x4000000){
		out[0]=0xf8+(in>>24);
		out[1]=0x80+((in>>18)&0x3f);
		out[2]=0x80+((in>>12)&0x3f);
		out[3]=0x80+((in>>6)&0x3f);
		out[4]=0x80+(in&0x3f);
		return 5;
	}else{
		out[0]=0xfc+(in>>30);
		out[1]=0x80+((in>>24)&0x3f);
		out[2]=0x80+((in>>18)&0x3f);
		out[3]=0x80+((in>>12)&0x3f);
		out[4]=0x80+((in>>6)&0x3f);
		out[5]=0x80+(in&0x3f);
		return 6;
	}
}

//Migemo/bregonig読み込み
bool LoadMigemo(){
	if(g_pmigemo!=NULL)return true;

	g_pmigemo=NULL;
	g_hMigemo=LoadLibrary(_T("migemo.dll"));

	if(g_hMigemo!=NULL){
		migemo_open=(MIGEMO_OPEN)GetProcAddress(g_hMigemo,"migemo_open");
		migemo_close=(MIGEMO_CLOSE)GetProcAddress(g_hMigemo,"migemo_close");
		migemo_query=(MIGEMO_QUERY)GetProcAddress(g_hMigemo,"migemo_query");
		migemo_release=(MIGEMO_RELEASE)GetProcAddress(g_hMigemo,"migemo_release");
		migemo_load=(MIGEMO_LOAD)GetProcAddress(g_hMigemo,"migemo_load");
		migemo_is_enable=(MIGEMO_IS_ENABLE)GetProcAddress(g_hMigemo,"migemo_is_enable");
		migemo_set_operator=(MIGEMO_SET_OPERATOR)GetProcAddress(g_hMigemo,"migemo_set_operator");
		migemo_setproc_int2char=(MIGEMO_SETPROC_INT2CHAR)GetProcAddress(g_hMigemo,"migemo_setproc_int2char");

		if(migemo_open!=NULL){
			//migemo.dllがあるディレクトリから取得
			TCHAR szMigemoDirectory[MAX_PATH]={};

			if(GetModuleFileName(g_hMigemo,szMigemoDirectory,ARRAY_SIZEOF(szMigemoDirectory))){
				TCHAR szCurrentDirectory[MAX_PATH]={};

				PathRemoveFileSpec(szMigemoDirectory);
				GetCurrentDirectory(ARRAY_SIZEOF(szCurrentDirectory),szCurrentDirectory);
				SetCurrentDirectory(szMigemoDirectory);
				g_pmigemo=migemo_open("./dict/utf-8/" "migemo-dict");
				if(g_pmigemo==NULL||!migemo_is_enable(g_pmigemo)){
					g_pmigemo=migemo_open("./dict/" "migemo-dict");
				}
				if(g_pmigemo==NULL||!migemo_is_enable(g_pmigemo)){
					migemo_close(g_pmigemo);
					g_pmigemo=NULL;
				}
				SetCurrentDirectory(szCurrentDirectory);
			}
		}
		//'[',']','~'をエスケープするため自前で処理
		migemo_setproc_int2char(g_pmigemo,int2char);

		g_hBregonig=LoadLibrary(_T("bregonig.dll"));

		if(g_pmigemo!=NULL&&
		   g_hBregonig!=NULL){
			BMatch=(BMATCH)GetProcAddress(g_hBregonig,"BMatchW");
			BRegfree=(BREGFREE)GetProcAddress(g_hBregonig,"BRegfreeW");
		}

		if(g_pmigemo==NULL||BMatch==NULL){
			MessageBox(NULL,_T("migemo/dict/bregonig読み込み失敗"),_T("Tascher"),MB_OK|MB_ICONEXCLAMATION);
			UnloadMigemo();
		}
	}
	return g_pmigemo!=NULL;
}

HMONITOR GetActiveMonitor(){
	HMONITOR hMonitor=NULL;

	switch(g_Config.MultiMonitor.iActiveMonitorMode){
		case ACTIVEMONITOR_BY_WINDOW:
			hMonitor=MonitorFromWindow(GetForegroundWindow(),MONITOR_DEFAULTTONEAREST);
			break;
		case ACTIVEMONITOR_BY_NAME:
			hMonitor=MonitorFromName(g_Config.MultiMonitor.szMonitorName);
			if(hMonitor!=NULL)break;
			//fall through
		case ACTIVEMONITOR_BY_CURSOR:{
			POINT pt;

			GetCursorPos(&pt);
			hMonitor=MonitorFromPoint(pt,MONITOR_DEFAULTTONEAREST);
			break;
		}
	}
	return hMonitor;
}

//一時的な設定をリセット
void ResetTemporarySetting(){
	//インクリメンタルサーチの検索方法を保存
	g_iMatchMode=g_Config.IncrementalSearch.iMatchMode;
	//検索する項目設定を保存
	g_bFirstColumnOnly=g_Config.IncrementalSearch.bFirstColumnOnly;
	//Migemo検索リセット
	g_bMigemoSearch=false;
	//候補が1つなら切り替える設定を保存
	g_bEnterUniqueWindow=g_Config.IncrementalSearch.bEnterUniqueWindow;
	//サムネイル設定を保存
	g_uThumbnail=g_Config.ListView.uThumbnail;
	if(g_uThumbnail!=THUMBNAIL_NO)g_uThumbnailBak=g_uThumbnail;
	else g_uThumbnailBak=THUMBNAIL_SECOND_COLUMN;
	//サムネイルを元の大きさで表示する設定を保存
	g_bThumbnailOriginalSize=g_Config.ListView.bThumbnailOriginalSize;
}

//タスクトレイ登録/解除
void TaskTray(HWND hWnd,DWORD dwMessage,bool bEnable=true){
	NOTIFYICONDATA NotifyIconData={sizeof(NOTIFYICONDATA),hWnd,ID_TRAYMENU,NIF_MESSAGE|NIF_ICON|NIF_TIP,MSG_TRAY,(bEnable)?g_hTrayIcon:g_hDisableTrayIcon};

	//ツールチップ
	lstrcpy(NotifyIconData.szTip,TASCHER_VERSION);

//	while(1){
	for(int i=0;i<10;i++){
		if(Shell_NotifyIcon(dwMessage,&NotifyIconData)){
			return;
		}else{
			if(GetLastError()!=ERROR_TIMEOUT){
				if(dwMessage!=NIM_DELETE){
//					MessageBox(hWnd,_T("アイコンの登録に失敗しました。"),_T("Tascher"),MB_OK|MB_ICONEXCLAMATION|MB_ICONEXCLAMATION);
//					SendMessage(hWnd,WM_CLOSE,(WPARAM)0,(LPARAM)0);
//					return;
				}
			}
			if(Shell_NotifyIcon(NIM_MODIFY,&NotifyIconData)){
				return;
			}
			Sleep(1000);
		}
	}
	return;
}

//子プロセスメニューを作成
void CreateChildProcessMenu(HMENU hMenu,HMENU hProcessSubMenu,PROCESSENTRY32* pProcessEntry32,DWORD dwParentProcessId){
	if(pProcessEntry32==NULL||!dwParentProcessId)return;

	for(size_t i=0,iSize=HeapSize(GetProcessHeap(),0,pProcessEntry32)/sizeof(PROCESSENTRY32);
		i<iSize;
		++i){
		if(dwParentProcessId==pProcessEntry32[i].th32ParentProcessID){
			FILETIME ftParentProcess,ftProcess,ftDummy;

			HANDLE hProcess=OpenProcess(PROCESS_QUERY_INFORMATION,false,pProcessEntry32[i].th32ParentProcessID);
			GetProcessTimes(hProcess,&ftParentProcess,&ftDummy,&ftDummy,&ftDummy);
			CloseHandle(hProcess);
			hProcess=OpenProcess(PROCESS_QUERY_INFORMATION,false,pProcessEntry32[i].th32ProcessID);
			GetProcessTimes(hProcess,&ftProcess,&ftDummy,&ftDummy,&ftDummy);
			CloseHandle(hProcess);

			//親プロセスより作成時間が早い
			if(CompareFileTime(&ftParentProcess,&ftProcess)==1)continue;

			MENUITEMINFO mii={sizeof(MENUITEMINFO)};
			mii.fMask=MIIM_STRING|MIIM_SUBMENU;
			mii.dwTypeData=pProcessEntry32[i].szExeFile;

			HMENU hSubMenu=CreatePopupMenu();

			int iSubMenuCount=0;

			//processメニュー追加
			for(;iSubMenuCount<GetMenuItemCount(hProcessSubMenu);iSubMenuCount++){
				MENUITEMINFO miiProcess={sizeof(MENUITEMINFO)};

				miiProcess.fMask=MIIM_ID|MIIM_TYPE;
				miiProcess.dwTypeData=NULL;
				GetMenuItemInfo(hProcessSubMenu,iSubMenuCount,true,&miiProcess);
				if(miiProcess.cch){
					TCHAR* pszMenuString=(TCHAR*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(++miiProcess.cch)*sizeof(TCHAR));
					miiProcess.dwTypeData=pszMenuString;
					GetMenuItemInfo(hProcessSubMenu,iSubMenuCount,true,&miiProcess);
					miiProcess.fMask|=MIIM_DATA;
					miiProcess.dwItemData=pProcessEntry32[i].th32ProcessID;
					InsertMenuItem(hSubMenu,iSubMenuCount,true,&miiProcess);
					HeapFree(GetProcessHeap(),0,pszMenuString);
					pszMenuString=NULL;
				}else{
					miiProcess.fMask&=~MIIM_STRING;
					InsertMenuItem(hSubMenu,iSubMenuCount,true,&miiProcess);
				}
			}

			//その他メニュー追加
			MENUITEMINFO miiSubMenu={sizeof(MENUITEMINFO)};
			miiSubMenu.fMask=MIIM_ID|MIIM_TYPE;

			//セパレータ追加
			miiSubMenu.fType=MFT_SEPARATOR;
			InsertMenuItem(hSubMenu,iSubMenuCount++,true,&miiSubMenu);

			//copyname追加
			miiSubMenu.fType=MFT_STRING;
			miiSubMenu.dwTypeData=_T("copy&name");
			miiSubMenu.wID=IDM_KEY_COPY_NAME;
			InsertMenuItem(hSubMenu,iSubMenuCount++,true,&miiSubMenu);

			//copycommandline追加
			miiSubMenu.fType=MFT_STRING;
			miiSubMenu.dwTypeData=_T("copy&commandline");
			miiSubMenu.wID=IDM_KEY_COPY_COMMANDLINE;
			InsertMenuItem(hSubMenu,iSubMenuCount++,true,&miiSubMenu);

			//セパレータ追加
			miiSubMenu.fType=MFT_SEPARATOR;
			InsertMenuItem(hSubMenu,iSubMenuCount++,true,&miiSubMenu);

			//info追加
			miiSubMenu.fType=MFT_STRING;
			miiSubMenu.dwTypeData=_T("in&fo");
			miiSubMenu.wID=IDM_KEY_INFO;
			InsertMenuItem(hSubMenu,iSubMenuCount++,true,&miiSubMenu);


			mii.hSubMenu=hSubMenu;
			InsertMenuItem(hMenu,0,true,&mii);
			CreateChildProcessMenu(hMenu,hProcessSubMenu,pProcessEntry32,pProcessEntry32[i].th32ProcessID);
		}
	}
	return;
}
//ウインドウのアイコンを取得
HICON GetWindowIcon(HWND hWnd,bool bUWPApp=false,bool bDesktopItem=false,bool bCancelItem=false){
	HICON hIcon=NULL;

	if(g_Config.ListView.iIcon!=LISTICON_NO){
		if(!IsHungAppWindow(hWnd)||
		   bDesktopItem||
		   bCancelItem){
			if(g_Config.ListView.iIcon>LISTICON_NO_SMALL){
				if(bDesktopItem){
					//「デスクトップ」アイコンを読み込む
					hIcon=GetDesktopIcon(g_Config.ListView.iIcon==LISTICON_SMALL);
				}else if(bCancelItem){
					//「キャンセル」アイコンを読み込む
					hIcon=LoadIcon(NULL,IDI_ERROR);
				}else{
					if(g_Config.ListView.iIcon==LISTICON_SMALL){
						hIcon=GetSmallIcon(hWnd,bUWPApp);
					}
					if(hIcon==NULL){
						//小さいアイコンが取得できない場合、大きなアイコンを利用
						hIcon=GetLargeIcon(hWnd,bUWPApp);
					}

					//デフォルトのアプリケーションアイコンを取得
					if(hIcon==NULL){
						HWND hParent=GetParent(hWnd);
						if(hParent)hIcon=GetWindowIcon(hParent,bUWPApp,bDesktopItem,bCancelItem);
						if(hIcon==NULL){
							hIcon=LoadIcon(NULL,IDI_APPLICATION);
						}
					}
				}
			}
		}
	}
	return hIcon;
}

//アイテムを追加
void AddItem(LPWINDOW_INFO lpWindowInfo){
	bool bFirstTime=g_iWindowCount==0;
	WINDOW_INFO* pTmp=NULL;

	if(bFirstTime)pTmp=(WINDOW_INFO*)HeapAlloc(g_hHeapWindowInfo,HEAP_ZERO_MEMORY,(++g_iWindowCount)*sizeof(WINDOW_INFO));
	else pTmp=(WINDOW_INFO*)HeapReAlloc(g_hHeapWindowInfo,HEAP_ZERO_MEMORY,g_WindowInfo,(++g_iWindowCount)*sizeof(WINDOW_INFO));

	g_iDisplayWindowCount=g_iWindowCount;

	g_WindowInfo=pTmp;
	g_WindowInfo[g_iWindowCount-1]=*lpWindowInfo;

	LPWINDOW_INFO* ppTmp;

	if(bFirstTime)ppTmp=(LPWINDOW_INFO*)HeapAlloc(g_hHeapWindowInfo,HEAP_ZERO_MEMORY,(g_iDisplayWindowCount)*sizeof(LPWINDOW_INFO));
	else ppTmp=(LPWINDOW_INFO*)HeapReAlloc(g_hHeapWindowInfo,HEAP_ZERO_MEMORY,g_DisplayWindowInfo,(g_iDisplayWindowCount)*sizeof(LPWINDOW_INFO));

	g_DisplayWindowInfo=ppTmp;
	return;
}

//「デスクトップ」を追加
void AddDesktopItem(){
	WINDOW_INFO wiDesktop={};

#if 1
	//「デスクトップ」をグローバルな構造体に追加
	wiDesktop.hIcon=GetWindowIcon(NULL,false,true,false);
	lstrcpy(wiDesktop.szFileName,_T("Desktop"));
	lstrcpy(wiDesktop.szWindowTitle,_T("デスクトップ"));
	wiDesktop.bDesktopItem=true;

	AddItem(&wiDesktop);
#else
	SHFILEINFO shFileInfo={};
	LPITEMIDLIST lvItemList=NULL;
	IShellFolder *pIShellFolder=NULL;
	IMalloc *pMalloc;

	SHGetMalloc(&pMalloc);

	CoInitialize(NULL);

	if(SUCCEEDED(SHGetSpecialFolderLocation(NULL,CSIDL_DESKTOP,&lvItemList))){
		TCHAR szFilePath[MAX_PATH]={};

		//パスを取得
		SHGetPathFromIDList(lvItemList,szFilePath);

		//表示名を取得
		SHGetFileInfo((LPCTSTR)lvItemList,0,&shFileInfo,sizeof(SHFILEINFO),SHGFI_DISPLAYNAME|SHGFI_PIDL);

		//「デスクトップ」をグローバルな構造体に追加
		lstrcpy(wiDesktop.szFileName,_T("Desktop"));
		lstrcpy(wiDesktop.szWindowTitle,shFileInfo.szDisplayName);
		lstrcpy(wiDesktop.szFilePath,szFilePath);
		wiDesktop.bDesktopItem=true;
		AddItem(&wiDesktop);

		pMalloc->Free(lvItemList);
	}
	pMalloc->Release();
	CoUninitialize();
#endif
	return;
}

//「キャンセル」を追加
void AddCancelItem(){
	WINDOW_INFO wiCancel={};

	wiCancel.hIcon=GetWindowIcon(NULL,false,false,true);
	lstrcpy(wiCancel.szFileName,_T("Cancel"));
	lstrcpy(wiCancel.szWindowTitle,_T("キャンセル"));
	//「キャンセル」をグローバルな構造体に追加
	wiCancel.bCancelItem=true;

	AddItem(&wiCancel);
	return;
}

#ifdef CHECK_DISABLE_UWPAPP_ENUMCHILD
BOOL CALLBACK EnumChildWindowsProc(HWND hWnd,LPARAM lParam){
	TCHAR szClassName[256]={};

	GetClassName(hWnd,szClassName,ARRAY_SIZEOF(szClassName));
	if(lstrcmp(szClassName,_T("Windows.UI.Core.CoreWindow"))==0){
		*(bool*)lParam=true;
		return FALSE;
	}
	return TRUE;
}
#endif

BOOL CALLBACK EnumChildWindowsProc2(HWND hWnd,LPARAM lParam){
	if(IsWindowOnCurrentVirtualDesktop(hWnd)){
		*(bool*)lParam=true;
		return FALSE;
	}
	return TRUE;
}

//列挙対象のウインドウである
bool IsWindowAvailable(HWND hWnd){
	RECT rc;
	HWND hParent=NULL;

	GetWindowRect(hWnd,&rc);
	hParent=GetParent(hWnd);

	if(GetWindowLongPtr(hWnd,GWL_STYLE)&WS_VISIBLE&&//可視状態
		!(GetWindowLongPtr(hWnd,GWL_STYLE)&WS_DISABLED)&&//操作可能である
		!(GetWindowLongPtr(hWnd,GWL_EXSTYLE)&WS_EX_TOOLWINDOW)&&//ツールウインドウでない
		(rc.right-rc.left)&&(rc.bottom-rc.top)){//幅、高さが共に0でない(Delphi対策)
			if(GetWindowLongPtr(hWnd,GWL_EXSTYLE)&(WS_EX_NOREDIRECTIONBITMAP)){
				//非表示のUWPAppを除く
				bool bVisible=false;
#ifdef CHECK_DISABLE_UWPAPP_ENUMCHILD
				EnumChildWindows(hWnd,EnumChildWindowsProc,(LPARAM)&bVisible);
#else
				if(g_bDwmApi){
					BOOL bCloaked=FALSE;

					if(SUCCEEDED(pDwmGetWindowAttribute(hWnd,DWMWA_CLOAKED,&bCloaked,sizeof(BOOL)))){
						bVisible=!bCloaked;
					}
				}
#endif
				if(!bVisible)return false;
			}
			/*
			//「タスクバー相当の表示」が有効であれば、
			//非表示の親を持つ、WS_POPUP属性のウインドウを除外する
			if(hParent&&!(GetWindowLongPtr(hParent,GWL_STYLE)&WS_DISABLED)&&!(GetWindowLongPtr(hParent,GWL_STYLE)&WS_VISIBLE)&&
			   (GetWindowLongPtr(hWnd,GWL_STYLE)&WS_POPUP))return !g_Config.ListView.bTaskBarEquivalent;
			else */return true;
	}
	return false;
}

//除外対象のファイル名である
bool IsExcludeFileName(const TCHAR* szFileName){
	//末尾に';'を付加しておく
	if(g_Config.Exclude.szFileName[lstrlen(g_Config.Exclude.szFileName)-1]!=';'){
		lstrcat(g_Config.Exclude.szFileName,_T(";"));
	}
	const TCHAR* p1=g_Config.Exclude.szFileName;
	const TCHAR* p2=NULL;
	int iLength=lstrlen(szFileName);

	while(*p1){
		//';'までp2を進める
		for(p2=p1+1;*p2&&*p2!=';';p2++);
		if(*p2=='\0')break;

		//対象と同じ長さ
		if(p2-p1==iLength){
			//文字列チェック開始
			while(p1!=p2){
				if(ChrCmpI(*p1,szFileName[iLength-(p2-p1)])==0){
					//同じ文字
					p1++;
				}else{
					break;
				}
			}
			if(p1==p2){
				//一致
				return true;
			}
		}
		p1=p2+1;
	}
	return false;
}

DWORD WINAPI GetWindowInfoThread(LPVOID lvParam){
	MSG msg;

	PeekMessage(&msg,NULL,0,0,PM_NOREMOVE);
	SetEvent(*(HANDLE*)lvParam);

	while(GetMessage(&msg,NULL,0,0)>0){
		if(msg.message==WM_GETWINDOWINFO){
			int i=(int)msg.lParam;
			if(g_iWindowCount<i)break;

			switch(msg.wParam){
				case GWI_ICON:
					if(!IsHungAppWindow(g_WindowInfo[i].hWnd)){
						if(!g_WindowInfo[i].bUWPApp){
							g_WindowInfo[i].hIcon=GetWindowIcon(g_WindowInfo[i].hWnd);
						}else{
							g_WindowInfo[i].hIcon=GetWindowIcon(g_WindowInfo[i].hUICoreWnd,true);
						}
						ListView_RedrawItems(g_hListView,i,i);
					}
					break;
				case GWI_TITLE:
					if(!IsHungAppWindow(g_WindowInfo[i].hWnd)){
						//ウインドウタイトル取得
						InternalGetWindowText(g_WindowInfo[i].hWnd,g_WindowInfo[i].szWindowTitle,MAX_PATH);
						ListView_RedrawItems(g_hListView,i,i);
					}
					break;
			}
		}else if(msg.message==WM_EXITGETWINDOWINFOTHREAD){
			break;
		}
	}

	ExitThread(0);
	return 0;
}

//ウインドウ列挙のコールバック
//Zオーダー順であると決め打ちしています...
BOOL CALLBACK EnumWindowsProc(HWND hWnd,LPARAM lParam){
	WINDOW_INFO WindowInfo={};
	TCHAR szFilePath[MAX_PATH]={};
	HWND hUICoreWnd=NULL;

	//ファイルパス取得
	GetFileNameFromWindowHandle(hWnd,szFilePath,MAX_PATH);


	if(IsWindowAvailable(hWnd)){
		if(GetWindowLongPtr(hWnd,GWL_EXSTYLE)&(WS_EX_NOREDIRECTIONBITMAP)){
			hUICoreWnd=FindWindowEx(hWnd,NULL,_T("Windows.UI.Core.CoreWindow"),NULL);
			if(!hUICoreWnd){
				//最小化されている場合、子にWindows.UI.Core.CoreWindowがない
				//その場合、トップレベルのWindows.UI.Core.CoreWindowを列挙、そのプロセスIdから取得したAppUserModelIdと
				//ApplicationFrameWindowのウインドウハンドルから取得したAppUserModelIdとを比べ、一致するかで判断する
				TCHAR szClassName[128]={};
				HWND hCurrent=GetTopWindow(NULL);
				TCHAR szApplicationUserModelIdFromWindow[256]={};
				TCHAR szApplicationUserModelIdFromProcess[256]={};

				//ApplicationFrameWindowのウインドウハンドルからAppUserModelIdを取得
				GetAppUserModelIdFromWindow(hWnd,szApplicationUserModelIdFromWindow,ARRAY_SIZEOF(szApplicationUserModelIdFromWindow));

				do{
					if(!IsHungAppWindow(hCurrent)&&
					   GetWindowLongPtr(hCurrent,GWLP_HWNDPARENT)==NULL&&
					   GetWindowLongPtr(hCurrent,GWL_EXSTYLE)&(WS_EX_NOREDIRECTIONBITMAP)/*&&
					   IsWindowEnabled(hCurrent)*/){//IsWindowEnabled(hCurrent)だとタスクバーにあるが中断しているウインドウを弾いてしまう
						GetClassName(hCurrent,szClassName,ARRAY_SIZEOF(szClassName));
						if(lstrcmp(szClassName,_T("Windows.UI.Core.CoreWindow"))==0){
							DWORD dwProcessId=0;
							GetWindowThreadProcessId(hCurrent,&dwProcessId);
							//Windows.UI.Core.CoreWindowのプロセスIdからAppUserModelIdを取得
							GetAppUserModelIdFromProcess(dwProcessId,szApplicationUserModelIdFromProcess,ARRAY_SIZEOF(szApplicationUserModelIdFromProcess));

							if(lstrcmp(szApplicationUserModelIdFromWindow,szApplicationUserModelIdFromProcess)==0){
								hUICoreWnd=hCurrent;
								break;
							}
						}

					}
				}while((hCurrent=GetNextWindow(hCurrent,GW_HWNDNEXT))!=NULL);
			}
			if(hUICoreWnd){
				WindowInfo.bUWPApp=true;
				WindowInfo.hUICoreWnd=hUICoreWnd;
				GetFileNameFromWindowHandle(hUICoreWnd,szFilePath,MAX_PATH);
			}
		}

		//ファイル名
		//ファイル名だけ取り出す
		lstrcpy(WindowInfo.szFileName,PathFindFileName(szFilePath));
		//拡張子を除く
		PathRemoveExtension(WindowInfo.szFileName);

		//アクティブモニタのウインドウか否か
		if(g_Config.MultiMonitor.bOnlyActiveMonitor){
			HMONITOR hMonitor=GetActiveMonitor();
			if(hMonitor!=MonitorFromWindow(hWnd,MONITOR_DEFAULTTONEAREST)){
				return TRUE;
			}
		}

		//アクティブな仮想デスクトップのウインドウか否か
		if(g_Config.VirtualDesktop.bOnlyActiveVirtualDesktop){
			if(!IsWindowOnCurrentVirtualDesktop(hWnd)){
				bool bOnCurrent=false;

				HWND hParent=GetParent(hWnd);

				do{
					if(GetWindowLongPtr(hParent,GWL_STYLE)&WS_VISIBLE&&
					   !(GetWindowLongPtr(hWnd,GWL_STYLE)&WS_DISABLED)&&
					   !(GetWindowLongPtr(hParent,GWL_EXSTYLE)&WS_EX_TOOLWINDOW)&&
					   //(!WS_POPUP)||(WS_POPUP&&WS_CAPTION)||WS_EX_APPWINDOW
					   ((!(GetWindowLongPtr(hParent,GWL_STYLE)&WS_POPUP)||
						GetWindowLongPtr(hParent,GWL_STYLE)&WS_POPUP&&GetWindowLongPtr(hParent,GWL_STYLE)&WS_CAPTION)||
						GetWindowLongPtr(hParent,GWL_EXSTYLE)&WS_EX_APPWINDOW)&&
					   //WS_BORDER||WS_CAPTION||WS_SYSMENU
					   (GetWindowLongPtr(hParent,GWL_STYLE)&WS_CAPTION||
						GetWindowLongPtr(hParent,GWL_STYLE)&WS_BORDER||
						GetWindowLongPtr(hParent,GWL_STYLE)&WS_SYSMENU)){
						if(IsWindowOnCurrentVirtualDesktop(hParent)){
							bOnCurrent=true;
							break;
						}
					}
				}while((hParent=GetParent(hParent))!=NULL);

				if(!bOnCurrent)return TRUE;
			}
		}

		//除外対象のファイル名か否か
		if(lstrlen(g_Config.Exclude.szFileName)&&
		   IsExcludeFileName(WindowInfo.szFileName)){
			return TRUE;
		}

		//ファイルパス
		lstrcpy(WindowInfo.szFilePath,szFilePath);

		//ウインドウハンドル
		WindowInfo.hWnd=hWnd;

		//フォアグラウンドウインドウかどうか
		WindowInfo.bForegorund=(hWnd==GetForegroundWindow());

		//最小化(アイコン化)されているかどうか
		WindowInfo.bIconic=IsIconic(hWnd)!=0;

		//最前面ウインドウかどうか
		WindowInfo.bTopMost=IsWindowTopMost(hWnd);

		//グローバルな構造体に追加
		AddItem(&WindowInfo);

		HANDLE hInitEvent=CreateEvent(NULL,true,false,NULL);

		g_WindowInfo[g_iWindowCount-1].Thread.hThread=CreateThread(NULL,0,GetWindowInfoThread,&hInitEvent,0,&g_WindowInfo[g_iWindowCount-1].Thread.dwThreadId);
		WaitForSingleObject(hInitEvent,INFINITE);
		CloseHandle(hInitEvent);

		//アイコン
		PostThreadMessage(g_WindowInfo[g_iWindowCount-1].Thread.dwThreadId,WM_GETWINDOWINFO,GWI_ICON,g_iWindowCount-1);
		//ウインドウタイトル
		PostThreadMessage(g_WindowInfo[g_iWindowCount-1].Thread.dwThreadId,WM_GETWINDOWINFO,GWI_TITLE,g_iWindowCount-1);
		//スレッドを終了させる
		PostThreadMessage(g_WindowInfo[g_iWindowCount-1].Thread.dwThreadId,WM_EXITGETWINDOWINFOTHREAD,0,0);
	}



	return TRUE;
}

//ツールチップを表示/非表示
void ShowToolTip(HWND hWnd,UINT uToolTipId=0,TCHAR* szText=NULL){
	TOOLINFO ti={sizeof(TOOLINFO)};
	TCHAR* pszText=NULL;

	if(szText!=NULL&&szText[0]!='\0'){
		pszText=(TCHAR*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(lstrlen(szText)+12)*sizeof(TCHAR));
		if(!g_bMigemoSearch){
			ti.lpszText=szText;
		}else{
			wsprintf(pszText,_T("[%s] %s"),(!g_bMigemoNomatch)?_T("M"):_T("m"),szText);
			ti.lpszText=pszText;
		}
	}
	ti.hwnd=hWnd;
	ti.uId=uToolTipId;

	HWND hToolTip=NULL;
	if(uToolTipId==ID_TOOLTIP_SEARCH)hToolTip=g_hToolTipSearch;
	else if(uToolTipId==ID_TOOLTIP_INFO)hToolTip=g_hToolTipInfo;

	if(pszText!=NULL){
		RECT rc;

		GetWindowRect(hWnd,&rc);

		//ツールチップ文字列更新
		SendMessage(hToolTip,TTM_UPDATETIPTEXT,0,(LPARAM)&ti);
		//ツールチップ表示位置
		SendMessage(hToolTip,TTM_TRACKPOSITION,0,(LPARAM)MAKELONG(rc.left+3,rc.top+3));
		//改行できるように幅を設定
		SendMessage(hToolTip,TTM_SETMAXTIPWIDTH,(WPARAM)0,(LPARAM)0);
		//ツールチップを表示
		SendMessage(hToolTip,TTM_TRACKACTIVATE,true,(LPARAM)&ti);

		HeapFree(GetProcessHeap(),0,pszText);
		pszText=NULL;
	}else{
		//ツールチップ非表示
		if(hToolTip==NULL){
			SendMessage(g_hToolTipSearch,TTM_TRACKACTIVATE,false,(LPARAM)&ti);
			SendMessage(g_hToolTipInfo,TTM_TRACKACTIVATE,false,(LPARAM)&ti);
		}else{
			SendMessage(hToolTip,TTM_TRACKACTIVATE,false,(LPARAM)&ti);
		}
	}
}

//サムネイルを表示/非表示する
void ShowThumbnail(bool bShow=true){
	KillTimer(g_hThumbnailWnd,MAKEWPARAM(ID_TIMER_THUMBNAIL,0));

	if(g_Config.ListView.iThumbnailDelay||!bShow){
		for(int i=0;i<g_iWindowCount;++i){
			if(g_WindowInfo[i].hThumbnail){
				pDwmUnregisterThumbnail(g_WindowInfo[i].hThumbnail);
				g_WindowInfo[i].hThumbnail=NULL;
			}
		}
	}

	if(!bShow||!g_uThumbnail){
		ShowWindow(g_hThumbnailWnd,SW_HIDE);
		return;
	}


	if(g_bDwmApi){
		SetTimer(g_hThumbnailWnd,
				 MAKEWPARAM(ID_TIMER_THUMBNAIL,0),
				 g_Config.ListView.iThumbnailDelay,
				 NULL);
	}
	return;
}

//リストビューをリサイズ
void ResizeListView(HWND hWnd){
	RECT rc;
	int iWidth=0;
	int iHeight=0;
	int iItemHeight=0;

	iWidth=ListView_GetAllColumnWidth(g_ListViewColumn_Table);
	if(iWidth==0){
		g_Config.ListView.iFileNameWidth=DEFAULT_FILENAME_WIDTH;
		g_Config.ListView.iWindowTitleWidth=DEFAULT_WINDOWTITLE_WIDTH;

		LisView_InitColumn(g_ListViewColumn_Table,&g_Config,g_hListView);
		iWidth=ListView_GetAllColumnWidth(g_ListViewColumn_Table);
	}

	if(iWidth>GetSystemMetrics(SM_CXSCREEN)){
		iWidth=GetSystemMetrics(SM_CXSCREEN);
	}

	//一行分の高さを取得
	ListView_GetItemRect(g_hListView,0,&rc,LVIR_BOUNDS);
	iItemHeight=rc.bottom-rc.top;

	iHeight=iItemHeight*g_iDisplayWindowCount;

	//モニタの高さを取得
	HMONITOR hMonitor=GetActiveMonitor();
	MONITORINFO info;
	info.cbSize=sizeof(info);
	GetMonitorInfo(hMonitor,&info);
	int iMonitorHeight=info.rcWork.bottom-info.rcWork.top;

	if(iHeight>iMonitorHeight){
		int iDiff=iMonitorHeight-iItemHeight*3;
		//一行分の高さの倍数に切り下げ
		iHeight=(iDiff%iItemHeight)?((iDiff-1)/iItemHeight)*iItemHeight:iDiff;
	}

	RECT rcListView;
	GetWindowRect(g_hListView,&rcListView);

	//リサイズ
	if((rcListView.bottom-rcListView.top)>iHeight){
		SetWindowPos(g_hMainWnd,
					 NULL,
					 0,0,
					 (!g_Config.ListView.bDialogFrame)?iWidth:iWidth+g_lFrameWidth,
					 (!g_Config.ListView.bDialogFrame)?iHeight:iHeight+g_lFrameHeight,
					 SWP_NOMOVE|SWP_NOZORDER);
	}else{
		//リストビューを先にリサイズして親の背景を表示させない
		SetWindowPos(g_hListView,
					 NULL,
					 0,0,
					 iWidth,iHeight,
					 SWP_NOMOVE|SWP_NOZORDER);

		SetWindowPos(g_hMainWnd,
					 NULL,
					 0,0,
					 (!g_Config.ListView.bDialogFrame)?iWidth:iWidth+g_lFrameWidth,
					 (!g_Config.ListView.bDialogFrame)?iHeight:iHeight+g_lFrameHeight,
					 SWP_NOMOVE|SWP_NOZORDER);
	}

	if(!g_bBackground){
		ListView_RedrawItems(g_hListView,0,g_iDisplayWindowCount);
		ShowThumbnail();
		return;
	}

	//以下、背景描画のための準備

	//オリジナルの背景画像からリストビュー表示分だけ切り出す

#if BACKGROUND_NO_GDIPLUS
	HDC hDC=GetDC(g_hListView);
	HDC hMem=CreateCompatibleDC(hDC);
	HDC hMem2=CreateCompatibleDC(hDC);
	HGDIOBJ hOld,hOld2;

	g_hBitmap=CreateCompatibleBitmap(hDC,iWidth,iHeight);
	hOld=SelectObject(hMem,g_hBitmap);
	hOld2=SelectObject(hMem2,g_hBitmapOriginal);

	BITMAP bi;
	GetObject(g_hBitmapOriginal,sizeof(bi),&bi);

	if(bi.bmWidth<iWidth||
	   bi.bmHeight<iHeight||
	   g_Config.Background.byBrightness!=255){
		//背景色決定
		RECT rcListView={0,0,iWidth,iHeight};
		HBRUSH hBrush=CreateSolidBrush(g_Config.Background.clrBackground);
		FillRect(hMem,&rcListView,hBrush);
		DeleteObject(hBrush);
	}

	int xOffset=0;
	int yOffset=0;

	if(g_Config.Background.byXOffsetPercent){
		if(bi.bmWidth<iWidth){
			xOffset=(iWidth-bi.bmWidth)*g_Config.Background.byXOffsetPercent/100;
			xOffset=0-xOffset;
		}else{
			xOffset=(bi.bmWidth-iWidth)*g_Config.Background.byXOffsetPercent/100;
		}
	}
	if(g_Config.Background.byYOffsetPercent){
		if(bi.bmHeight<iHeight){
			yOffset=(iHeight-bi.bmHeight)*g_Config.Background.byYOffsetPercent/100;
			yOffset=0-yOffset;
		}else{
			//iItemHeight*g_iWindowCountを高さとする
			int iHeight2=iItemHeight*g_iWindowCount;

			if(iHeight2>GetSystemMetrics(SM_CYSCREEN)){
				iHeight2=GetSystemMetrics(SM_CYSCREEN)-iItemHeight*3;//ある程度余裕を持たせる
			}
			yOffset=(bi.bmHeight-iHeight2)*g_Config.Background.byYOffsetPercent/100;
		}
	}

	if(g_Config.Background.byBrightness==255){
		SetStretchBltMode(hMem,HALFTONE);

		StretchBlt(hMem,
				   0,0,
				   bi.bmWidth*g_Config.Background.byResizePercent/100,bi.bmHeight*g_Config.Background.byResizePercent/100,
				   hMem2,
				   xOffset,yOffset,
				   bi.bmWidth,bi.bmHeight,
				   SRCCOPY);
	}else{
		//座標の計算がよく分からないので一時的にBitBlt()でコピー
		HBITMAP g_hBitmapTmp=CreateCompatibleBitmap(hDC,iWidth,iHeight);
		HDC hMem3=CreateCompatibleDC(hDC);
		HGDIOBJ hOld3=SelectObject(hMem3,g_hBitmapTmp);

		SetStretchBltMode(hMem3,HALFTONE);
		StretchBlt(hMem3,
				   0,0,
				   bi.bmWidth*g_Config.Background.byResizePercent/100,bi.bmHeight*g_Config.Background.byResizePercent/100,
				   hMem2,
				   xOffset,yOffset,
				   bi.bmWidth,bi.bmHeight,
				   SRCCOPY);


		//明るさ調整
		BLENDFUNCTION bf={AC_SRC_OVER,0,g_Config.Background.byBrightness,0};

		AlphaBlend(hMem,
				   0,0,
				   iWidth,iHeight,
				   hMem3,
				   0,0,
				   iWidth,iHeight,
				   bf);

		SelectObject(hMem3,hOld3);
		DeleteDC(hMem3);
	}

	SelectObject(hMem,hOld);
	SelectObject(hMem2,hOld2);
	DeleteDC(hMem);
	DeleteDC(hMem2);
	ReleaseDC(g_hListView,hDC);

#else
	if(g_pBitmapBack==NULL){
		FreeBitmap(g_pBitmap);
		g_pBitmap=new Gdiplus::Bitmap(iWidth,iHeight);
		Gdiplus::Graphics graphics(g_pBitmap);
		Gdiplus::Rect rcSrc(g_Config.Background.iXOffset,g_Config.Background.iYOffset,
							g_pBitmapOriginal->GetWidth()*g_Config.Background.byResizePercent/100,
							g_pBitmapOriginal->GetHeight()*g_Config.Background.byResizePercent/100);
		//不透明度設定
		Gdiplus::ImageAttributes imageattr;
		Gdiplus::ColorMatrix colormatrix={
			1.0f,0.0f,0.0f,0.0f,0.0f,
			0.0f,1.0f,0.0f,0.0f,0.0f,
			0.0f,0.0f,1.0f,0.0f,0.0f,
			0.0f,0.0f,0.0f,(Gdiplus::REAL)g_Config.Background.byOpacity/100,0.0f,
			0.0f,0.0f,0.0f,0.0f,1.0f
		};
		imageattr.SetColorMatrix(&colormatrix);
		graphics.DrawImage(g_pBitmapOriginal,
						   rcSrc,
						   0,0,
						   g_pBitmapOriginal->GetWidth(),g_pBitmapOriginal->GetHeight(),
						   Gdiplus::UnitPixel,&imageattr);
		//バックアップ
		g_pBitmapBack=g_pBitmap->Clone(0,0,iWidth,iHeight,PixelFormatDontCare);
	}else{
		FreeBitmap(g_pBitmap);
		g_pBitmap=new Gdiplus::Bitmap(iWidth,iHeight);
		Gdiplus::Graphics graphics(g_pBitmap);
		Gdiplus::Rect rcSrc(0,0,iWidth,iHeight);
		//バックアップから描画
		graphics.DrawImage(g_pBitmapBack,
						   rcSrc,0,0,iWidth,iHeight,Gdiplus::UnitPixel);
	}
#endif

	//再描画
	ListView_RedrawItems(g_hListView,0,g_iDisplayWindowCount);
	ShowThumbnail();
}

//リストビューを非表示
void HideListView(){
	static LONG lInterlockedFlag;
	if(g_eWindowListStatus==WLS_HIDE)return;

	if(InterlockedCompareExchange(&lInterlockedFlag,TRUE,FALSE)==FALSE){
		if(g_eWindowListStatus==WLS_HIDE)return;

		if(g_hMenuWnd&&IsWindowVisible(g_hMenuWnd)){
			while(IsWindowVisible(g_hMenuWnd)){
				//メニューを閉じる
				//Windows 10で動作しない?
//				SendMessage(g_hListView,WM_CANCELMODE,0,0);

				SendMessage(g_hMenuWnd,WM_KEYDOWN,VK_ESCAPE,0);
				Sleep(10);
			}
			g_hMenuWnd=NULL;

			//このままHideListView()の処理を続行するとTascherがフォーカスを掴んだまま?になるのでタイマーを仕掛けて回避
			InterlockedExchange(&lInterlockedFlag,FALSE);
			SetForegroundWindow(g_hListView);
			SetTimer(g_hListView,
					 ID_TIMER_HIDELISTVIEW,
					 0,
					 NULL);
			return;
		}

		//サムネイルウインドウ非表示
		ShowThumbnail(false);

		//ツールチップ非表示
		ShowToolTip(g_hListView);

		for(int i=0;i<g_iWindowCount;i++){
			DWORD dwExitCode=0;

			GetExitCodeThread(g_WindowInfo[i].Thread.hThread,&dwExitCode);
			if(dwExitCode==STILL_ACTIVE){
				PostThreadMessage(g_WindowInfo[i].Thread.dwThreadId,WM_EXITGETWINDOWINFOTHREAD,0,0);

				if(WaitForSingleObject(g_WindowInfo[i].Thread.hThread,1000)!=WAIT_OBJECT_0){
					TerminateThread(g_WindowInfo[i].Thread.hThread,0);
				}
				CloseHandle(g_WindowInfo[i].Thread.hThread);
			}
		}


		if(g_Config.ListView.byOpacity!=0&&
		   g_Config.ListView.byOpacity!=100){
			//ウインドウを透明に
			SetLayeredWindowAttributes(g_hMainWnd,0,0,LWA_ALPHA);
		}

		g_bHotKeyMode=false;
		g_byModKey=0;

		//アイコン解放
		for(int i=0;i<g_iWindowCount;++i){
			DestroyIcon(g_WindowInfo[i].hIcon);
			g_WindowInfo[i].hIcon=NULL;
		}

		for(int i=0;i<lstrlen(g_szSearchString);++i){
			g_szSearchString[i]='\0';
		}

		g_iWindowCount=g_iDisplayWindowCount=0;

		ListView_SetItemCountEx(g_hListView,0,LVSICF_NOINVALIDATEALL|LVSICF_NOSCROLL);

		HeapFree(g_hHeapWindowInfo,0,g_DisplayWindowInfo);
		HeapFree(g_hHeapWindowInfo,0,g_WindowInfo);
		g_DisplayWindowInfo=NULL;
		g_WindowInfo=NULL;

		ShowWindow(g_hMainWnd,SW_HIDE);

		if(g_Config.ListView.byOpacity!=0&&
		   g_Config.ListView.byOpacity!=100){
			//不透明度を元に戻す
			SetLayeredWindowAttributes(g_hMainWnd,0,g_Config.ListView.byOpacity*255/100,LWA_ALPHA);
		}
		g_eWindowListStatus=WLS_HIDE;

		if(!g_Config.IncrementalSearch.bNoUnloadMigemo)UnloadMigemo();

		//Bitmapを削除
		FreeBitmap(g_pBitmap);
		FreeBitmap(g_pBitmapBack);

		InterlockedExchange(&lInterlockedFlag,FALSE);
	}
	return;
}

//ウインドウの切り替えスレッド
DWORD WINAPI TaskSwitchThread(LPVOID lvParam){
	HWND hWnd=*(HWND*)lvParam;

	//5000ms以上応答がなければ実行しない
	if(!IsHungAppWindow(hWnd)&&
	   SendMessageTimeout(hWnd,WM_NULL,0,0,SMTO_ABORTIFHUNG,5000,NULL)!=0){
		SetForegroundWindowEx(hWnd);
	}

	HANDLE hCurrentThread=GetCurrentThread();
	DuplicateHandle(GetCurrentProcess(),hCurrentThread,GetCurrentProcess(),&hCurrentThread,0,FALSE,DUPLICATE_SAME_ACCESS);

	PostMessage(g_hMainWnd,WM_CLOSETHREAD,(WPARAM)hCurrentThread,0);

	ExitThread(0);
	return 0;
}

//ウインドウの切り替えスレッド起動
bool SetForegroundWindowEx(LPWINDOW_INFO lpWindowInfo){
	if(IsHungAppWindow(lpWindowInfo->hWnd))return false;

	static HANDLE hThread=NULL;
	DWORD dwExitCode=0;

	GetExitCodeThread(hThread,&dwExitCode);
	if(hThread&&dwExitCode==STILL_ACTIVE){
		TerminateThread(hThread,0);
		CloseHandle(hThread);
	}

	hThread=CreateThread(NULL,0,TaskSwitchThread,&lpWindowInfo->hWnd,0,NULL);

	return true;
}

//ウインドウの切り替え
void TaskSwitch(LPWINDOW_INFO lpWindowInfo){
	if(lpWindowInfo==NULL)return;

	WINDOWLIST_STATUS eWindowListStatus=g_eWindowListStatus;

	//WM_ACTIVATEAPP対策
	g_eWindowListStatus=WLS_SWITCHING;

	//SetForegroundWindowEx()で切り替え対象の最前面化ON/OFFをする際のちらつき防止
	SetWindowPos(g_hMainWnd,
				 HWND_NOTOPMOST,
				 0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);

	if(lpWindowInfo->bDesktopItem){
		//「デスクトップ」
		SendMessage(g_hMainWnd,WM_COMMAND,MAKEWPARAM(IDM_KEY_DESKTOP,0),0);
	}else if(lpWindowInfo->bCancelItem){
		//「キャンセル」
		SendMessage(g_hMainWnd,WM_COMMAND,MAKEWPARAM(IDM_KEY_CANCEL,0),0);
	}else{
		//通常のウインドウ
		if(!IsHungAppWindow(lpWindowInfo->hWnd)){
			SetForegroundWindowEx(lpWindowInfo);
		}
	}
	g_eWindowListStatus=eWindowListStatus;

	HideListView();
	return;
}

//リストビューを表示
void ShowListView(SCC_CORNERS eCorners=SCC_CENTER){
	if(g_eWindowListStatus==WLS_CREATING)return;

	if(g_eWindowListStatus==WLS_DISPLAYED){
		//表示済みであれば非表示に
		HideListView();
		return;
	}

	if(g_DisplayWindowInfo!=NULL||
	   g_WindowInfo!=NULL){
		return;
	}

	g_eCorners=eCorners;

	HWND hForegroundWnd=GetForegroundWindow();

	//ウインドウの列挙
	EnumWindows((WNDENUMPROC)EnumWindowsProc,0);

	if(g_iWindowCount){
		g_eWindowListStatus=WLS_CREATING;

		if(g_Config.ListView.bDesktopItem){
			//「デスクトップ」を追加
			AddDesktopItem();
		}

		if(g_Config.ListView.bCancelItem){
			//「キャンセル」を追加
			AddCancelItem();
		}

		//表示用に情報をコピー
		for(int i=0;i<g_iDisplayWindowCount;++i){
			g_DisplayWindowInfo[i]=&g_WindowInfo[i];
		}


		//アイテム数を設定
		ListView_SetItemCountEx(g_hListView,g_iWindowCount,LVSICF_NOINVALIDATEALL|LVSICF_NOSCROLL);

		ResizeListView(g_hMainWnd);

		//最前面だったウインドウを選択する
		//bSelectSecondWindowが有効であれば、2番目のウインドウを選択する
		//WS_EX_TOPMOSTかつ最小化されているものはスキップする
		//EnumWindows()がZオーダー順でありますように...
		if(!g_Config.ListView.bSelectSecondWindow||g_iWindowCount==1){
			ListView_SelectItem(g_hListView,0);
		}else{
			for(int i=1;i<g_iWindowCount;++i){
				//「デスクトップ」か「キャンセル」であればスキップ
				if(g_WindowInfo[i].bDesktopItem||g_WindowInfo[i].bCancelItem)continue;
				//WS_EX_TOPMOSTであればスキップ
				if(g_WindowInfo[i].bTopMost&&g_WindowInfo[i].bIconic)continue;
				//フォアグラウンドウインドウであればスキップ
				if(g_WindowInfo[i].bForegorund)continue;
				ListView_SelectItem(g_hListView,i);
				break;
			}
			if(g_hListView,ListView_GetSelectedItem(g_hListView)==-1){
				ListView_SelectItem(g_hListView,0);
			}
		}

		if(g_Config.IncrementalSearch.iMigemoMode!=MIGEMO_NO&&
		   !g_Config.IncrementalSearch.bNoUnloadMigemo)LoadMigemo();

		//画面の中央に移動
		switch(g_Config.MultiMonitor.iActiveMonitorMode){
			case ACTIVEMONITOR_BY_CURSOR:
				SetCenterWindow(g_hMainWnd);
				break;
			case ACTIVEMONITOR_BY_WINDOW:{
				SetCenterWindow(g_hMainWnd,hForegroundWnd);
				break;
			}
			case ACTIVEMONITOR_BY_NAME:
				SetCenterWindow(g_hMainWnd,g_Config.MultiMonitor.szMonitorName);
				break;
		}

		RECT rcMainWnd;

		GetWindowRect(g_hMainWnd,&rcMainWnd);

		if(g_Config.ListView.iHideCursorTime==0){
			//カーソルちらつき防止のため0設定ならここで処理
			SetCursor(g_hBlankCursor);
			SetClassLongPtr(g_hListView,GCLP_HCURSOR,(LONG_PTR)g_hBlankCursor);
		}

		ShowWindow(g_hMainWnd,SW_SHOWNORMAL);

		SetForegroundWindow(g_hMainWnd);


		//10回チャレンジ!
		for(int i=0;i<10;++i){
			if(eCorners!=SCC_NONE){
				//カーソルをウインドウ内に閉じこめる
				ClipCursor(&rcMainWnd);
				ClipCursor(NULL);
			}

			switch(eCorners){
				case SCC_LEFTTOP:
				case SCC_TOP:
				case SCC_RIGHTTOP:
				case SCC_LEFT:
				case SCC_RIGHT:
				case SCC_LEFTBOTTOM:
				case SCC_BOTTOM:
				case SCC_RIGHTBOTTOM:
					SetCursorCorner(g_hMainWnd,
									eCorners,
									(!g_bHotKeyMode)?g_Config.ShowWindow.iMouseCursorMarginHorizontal:g_Config.ShowWindow.iHotKeyCursorMarginHorizontal,
									(!g_bHotKeyMode)?g_Config.ShowWindow.iMouseCursorMarginVertical:g_Config.ShowWindow.iHotKeyCursorMarginVertical);

					break;

				case SCC_CENTER:
					//中央
					SetCenterCursor(g_hMainWnd);
					break;

				case SCC_NONE:
				default:
					break;
			}
		}

		if(eCorners!=SCC_NONE&&
		   (!g_bHotKeyMode&&g_Config.ShowWindow.bMouseMoveCursorSelectedWindow||
			g_bHotKeyMode&&g_Config.ShowWindow.bHotKeyMoveCursorSelectedWindow)&&
		   g_Config.ShowWindow.bMoveCursorAtStart){
			//「選択項目にカーソルを移動」が有効なら表示時に移動
			POINT ptOrig;

			GetCursorPos(&ptOrig);
			RECT rc;

			ListView_GetSubItemRect(g_hListView,ListView_GetSelectedItem(g_hListView),
									g_ListViewColumn_Table[0].bVisible&&g_ListViewColumn_Table[1].bVisible,
									LVIR_BOUNDS,&rc);

			int iListHeight=rc.bottom-rc.top;

			POINT pt={0,0};

			if(iListHeight<=((!g_bHotKeyMode)?g_Config.ShowWindow.iMouseCursorMarginVertical:g_Config.ShowWindow.iHotKeyCursorMarginVertical)){
				pt.x=rc.left;
				pt.y=rc.top;
				ClientToScreen(g_hListView,&pt);
				SetCursorPos(ptOrig.x,pt.y+iListHeight/2);
			}else{
				GetCursorCornerPos(&pt,
								   eCorners,
								   &rc,
								   0,
								   (!g_bHotKeyMode)?g_Config.ShowWindow.iMouseCursorMarginVertical:g_Config.ShowWindow.iHotKeyCursorMarginVertical);
				ClientToScreen(g_hListView,&pt);
				SetCursorPos(ptOrig.x,pt.y);
			}
		}

		g_eWindowListStatus=WLS_CREATED;

		//管理者権限で動作しているウインドウがアクティブだとSetCursorCorner(のSetCursorPos())に失敗するため、
		//カーソル移動の前にTascherをアクティブにしておく
//		ShowWindow(g_hMainWnd,SW_SHOWNORMAL);
//		SetForegroundWindow(g_hMainWnd);

		//最前面表示
		SetWindowPos(g_hMainWnd,
					 HWND_TOPMOST,
					 0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		//親に送らないとWM_ACTIVATEAPPが使えない?
		SetFocus(g_hMainWnd);
	}
	return;
}

//リストビューアイテムを描画
bool DrawItem(LPDRAWITEMSTRUCT lpDraw){
	HDC hDC=lpDraw->hDC;
	HWND hListView=lpDraw->hwndItem;
	UINT uItemID=lpDraw->itemID;
	HICON hIcon=NULL;
	HBRUSH hBrush=NULL;
	RECT rc={};
	HFONT hOldFont=NULL;
#if BACKGROUND_NO_GDIPLUS
	HDC hMem=NULL;
	HGDIOBJ hOld=NULL;
#endif

	if(lpDraw->CtlType!=ODT_LISTVIEW)return false;
	if(!g_iDisplayWindowCount||
	   int(uItemID+1)>g_iDisplayWindowCount||
	   g_DisplayWindowInfo[uItemID]==NULL)return false;

	int iOrders[LISTITEM_NUM];
	ListView_GetColumnOrderArray(hListView,LISTITEM_NUM,&iOrders);

	if(g_bBackground){
#if 0
		//LVS_EX_DOUBLEBUFFERが有効だと正しく描画されない
		hBrush=CreatePatternBrush(g_hBitmap);
		ListView_GetSubItemRect(hListView,uItemID,0,LVIR_BOUNDS,&rc);

		//背景画像描画
		FillRect(hDC,&rc,hBrush);
		DeleteObject(hBrush);
#endif

#if BACKGROUND_NO_GDIPLUS
		ListView_GetSubItemRect(hListView,uItemID,0,LVIR_BOUNDS,&rc);
		hMem=CreateCompatibleDC(hDC);
		hOld=SelectObject(hMem,g_hBitmap);
		BitBlt(hDC,
			   rc.left,rc.top,
			   rc.right-rc.left,
			   rc.bottom-rc.top,
			   hMem,
			   rc.left,rc.top,
			   SRCCOPY);
#else
		ListView_GetSubItemRect(hListView,uItemID,0,LVIR_BOUNDS,&rc);

		Gdiplus::Graphics graphics(hDC);

		//背景を描画
		COLORREF clr=(lpDraw->itemState&ODS_SELECTED)?g_Config.SelectedItemDesign.clrTextBk:g_Config.DefaultItemDesign.clrTextBk;
		Gdiplus::SolidBrush brush(Gdiplus::Color(GetRValue(clr),GetGValue(clr),GetBValue(clr)));
		graphics.FillRectangle(&brush,
							   rc.left,rc.top,
							   rc.right-rc.left,rc.bottom-rc.top);

		graphics.DrawImage(g_pBitmap,
						   rc.left,rc.top,
						   rc.left,rc.top,
						   rc.right-rc.left,rc.bottom-rc.top,
						   Gdiplus::UnitPixel);
#endif

	}

	for(int iItem=0;iItem<Header_GetItemCount(ListView_GetHeader(hListView));iItem++){

		if(!ListView_GetColumnWidth(hListView,iItem))continue;

		//カラム全体のサイズを取得
		ListView_GetSubItemRect(hListView,uItemID,iItem,LVIR_BOUNDS,&rc);

		if(lpDraw->itemState&ODS_SELECTED){
			//選択行
			//フォント
			hOldFont=(HFONT)SelectObject(hDC,g_hSelectedItemFont);
			//文字色
			SetTextColor(hDC,g_Config.SelectedItemDesign.clrText);
			//背景色
			if(!g_bBackground){
				hBrush=CreateSolidBrush(g_Config.SelectedItemDesign.clrTextBk);
			}
		}else{
			//通常行
			//フォント
			hOldFont=(HFONT)SelectObject(hDC,g_hDefaultItemFont);
			//文字色
			SetTextColor(hDC,g_Config.DefaultItemDesign.clrText);
			//背景色
			if(!g_bBackground){
				hBrush=CreateSolidBrush(g_Config.DefaultItemDesign.clrTextBk);
			}
		}

		if(!g_bBackground){
			//背景を塗りつぶす
			FillRect(hDC,&rc,hBrush);
			DeleteObject(hBrush);
		}

		//文字列表示領域のサイズを取得
		ListView_GetSubItemRect(hListView,uItemID,iItem,LVIR_LABEL,&rc);

		switch(ListView_GetColumnIndex(g_ListViewColumn_Table,iOrders[iItem])){
			case -1:
			case 0:
				//1番目のカラム
				//文字表示位置を移動させる
				if(g_Config.ListView.iIcon>LISTICON_NO_SMALL){
					rc.left=(g_Config.ListView.iIcon==LISTICON_SMALL)?16:32;
					OffsetRect(&rc,5,0);
				}else{
					rc.left=0;
				}
				OffsetRect(&rc,3,0);

				//右端を調整
				TEXTMETRIC tm;

				//小文字の平均文字幅を取得
				GetTextMetrics(hDC,&tm);
				rc.right-=tm.tmAveCharWidth;
				break;

			case 1:
				//2番目のカラム
				//文字表示位置を移動させる
				if(iItem==0){
					rc.left-=(g_Config.ListView.iIcon==LISTICON_NO_SMALL||g_Config.ListView.iIcon==LISTICON_SMALL)?16:32;
					//右端を調整
					TEXTMETRIC tm;

					//小文字の平均文字幅を取得
					GetTextMetrics(hDC,&tm);
					rc.right-=tm.tmAveCharWidth;
				}else{
					OffsetRect(&rc,3,0);
				}
				break;

			default:
				break;
		}

		switch(iItem){
			case LISTITEM_FILENAME:
				DrawText(hDC,g_DisplayWindowInfo[uItemID]->szFileName,-1,&rc,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
				break;

			case LISTITEM_WINDOWTITLE:
				DrawText(hDC,g_DisplayWindowInfo[uItemID]->szWindowTitle,-1,&rc,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
				break;

			default:
				break;
		}

		//フォントの復元
		SelectObject(hDC,hOldFont);
	}

	if(g_DisplayWindowInfo[uItemID]->hIcon){
		//アイコンを描写する
		ListView_GetItemRect(hListView,uItemID,&rc,LVIR_ICON);
		DrawIconEx(hDC,
				   3/*rc.left*/,
				   rc.top,
				   g_DisplayWindowInfo[uItemID]->hIcon,
				   (g_Config.ListView.iIcon==LISTICON_SMALL)?16:32,
				   (g_Config.ListView.iIcon==LISTICON_SMALL)?16:32,
				   0,0,
				   DI_IMAGE|DI_MASK);
	}

#if BACKGROUND_NO_GDIPLUS
	SelectObject(hMem,hOld);
	DeleteDC(hMem);
#endif

	return true;
}

//先頭/部分一致検索
bool Match(int iMatchMode,const TCHAR* szTargetString,const TCHAR* szSearchString){
	if(iMatchMode==MATCH_FLEX){
		//あいまい
		if(!lstrlen(szSearchString)||!lstrlen(szTargetString))return false;
		if(lstrlen(szSearchString)>lstrlen(szTargetString))return false;

		const TCHAR* pszSearchString=szSearchString;
		const TCHAR* pszTargetString=szTargetString;
		bool bEnd=false;

		while(*pszSearchString&&*pszTargetString){
			while(*pszSearchString==' '){
				//検索文字列に空白があった場合スキップ
				pszSearchString++;
				if(!*pszSearchString){
					bEnd=true;
					break;
				}
			}
			if(bEnd)break;

			if(ChrCmpI(*pszSearchString,*(pszTargetString++))==0){
				pszSearchString++;
			}
		}
		return !(*pszSearchString);
	}else if(StrChr(szSearchString,' ')==NULL){
		return (iMatchMode==MATCH_FORWARD)?
				//先頭一致
				(StrCmpNI(szSearchString,
						  szTargetString,
						  lstrlen(szSearchString))==0):
				//部分一致
				(StrStrI(szTargetString,
						 szSearchString)!=NULL);
	}else{
		TCHAR szSearch[INCREMENTAL_SEARCH_BUFFER]={};

		lstrcpy(szSearch,szSearchString);

		bool bResult=true;
		int iIndex=0;

		for(int i=0;;++i){
			if(szSearchString[i]==' '){
				szSearch[i]='\0';
				bResult=(iMatchMode==MATCH_FORWARD)?
						//先頭一致
						(StrCmpNI(szSearch+iIndex,
								  szTargetString,
								  lstrlen(szSearch+iIndex))==0):
						//部分一致
						(StrStrI(szTargetString,
								 szSearch+iIndex)!=NULL);
				iIndex=i+1;
			}else if(szSearchString[i]=='\0'){
				if(lstrlen(szSearch+iIndex)){
					bResult=(iMatchMode==MATCH_FORWARD)?
						//先頭一致
						(StrCmpNI(szSearch+iIndex,
								 szTargetString,
								  lstrlen(szSearch+iIndex))==0):
					//部分一致
					(StrStrI(szTargetString,
							 szSearch+iIndex)!=NULL);
				}
				break;
			}
		}

		return bResult;
	}
}

//Migemoを用い正規表現リストを作成
TCHAR** AllocRegexes(TCHAR* szQuery,int* iRegexCount){
	*iRegexCount=path::CountCharacter(szQuery,' ');
	*iRegexCount=(*iRegexCount==0)?1:*iRegexCount+1;
	if(szQuery[lstrlen(szQuery)-1]==' ')--*iRegexCount;

	//FreeRegexes()で解放すること!
	TCHAR**ppszRegexes=(TCHAR**)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(TCHAR*)**iRegexCount);

	TCHAR* pszQuery=(TCHAR*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(lstrlen(szQuery)+1)*sizeof(TCHAR));

	lstrcpy(pszQuery,szQuery);

	int iIndex=0;

	for(int i=0,iRegexIndex=0;iRegexIndex<*iRegexCount;++i){
		if(szQuery[i]==' '||
		   (szQuery[i]=='\0'&&lstrlen(pszQuery+iIndex))){
			pszQuery[i]='\0';

			int iQueryLength=WideCharToMultiByte(CP_UTF8,0,pszQuery+iIndex,-1,NULL,0,NULL,NULL);
			char* pszQueryA=(char*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,iQueryLength);
			WideCharToMultiByte(CP_UTF8,0,pszQuery+iIndex,-1,pszQueryA,iQueryLength,NULL,NULL);
			unsigned char*pszRegexA=migemo_query(g_pmigemo,(const unsigned char*)pszQueryA);
			HeapFree(GetProcessHeap(),0,pszQueryA);
			pszQueryA=NULL;
			int iRegexLength=lstrlenA((char*)pszRegexA);
			ppszRegexes[iRegexIndex]=(TCHAR*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(iRegexLength+1)*sizeof(TCHAR));
			MultiByteToWideChar(CP_UTF8,0,(LPCSTR)pszRegexA,-1,ppszRegexes[iRegexIndex],(iRegexLength+1)*sizeof(TCHAR));
			migemo_release(g_pmigemo,pszRegexA);

			iIndex=i+1;
			iRegexIndex++;
			if(szQuery[i]=='\0')break;
		}
	}

	HeapFree(GetProcessHeap(),0,pszQuery);
	pszQuery=NULL;
	return ppszRegexes;
}

//正規表現リストを解放
void FreeRegexes(TCHAR**ppszRegexes,int iRegexCount){
	HeapFree(GetProcessHeap(),0,ppszRegexes);
	ppszRegexes=NULL;
}

//インクリメンタルサーチを行いウインドウを絞り込む
bool IncrementalSearch(TCHAR* szSearchString,int iMatchMode=MATCH_FORWARD,bool bFirstColumnOnly=false){
	bool bResult=false;
	int iSearchStringBack=0;

	for(int iLength=lstrlen(szSearchString);iSearchStringBack<iLength&&szSearchString[iSearchStringBack]!='\0';++iSearchStringBack);
	--iSearchStringBack;

	//選択中アイテム保存
	bool bSelected=false;
	HWND hSelectedItemWnd=NULL;
	int iSelectedItem=ListView_GetSelectedItem(g_hListView);
	hSelectedItemWnd=(iSelectedItem!=-1)?g_DisplayWindowInfo[iSelectedItem]->hWnd:NULL;

#if 0
	//表示非表示問わずすべてのウインドウを対象に検索する場合
	if(g_bMigemoSearch){
		for(int i=0;i<g_iWindowCount;++i){
			g_DisplayWindowInfo[i]=&g_WindowInfo[i];
		}
		g_iDisplayWindowCount=g_iWindowCount;
	}
#endif

	LPWINDOW_INFO* DisplayWindowInfoBak;
	int iDisplayWindowCount=g_iDisplayWindowCount;

	DisplayWindowInfoBak=(LPWINDOW_INFO*)HeapAlloc(g_hHeapWindowInfo,HEAP_ZERO_MEMORY,(iDisplayWindowCount)*sizeof(LPWINDOW_INFO));

	for(int i=0;i<iDisplayWindowCount;++i){
		DisplayWindowInfoBak[i]=g_DisplayWindowInfo[i];
	}

	if(szSearchString[0]!='\0'){
		LPWINDOW_INFO* DisplayWindowInfoTmp;

		DisplayWindowInfoTmp=(LPWINDOW_INFO*)HeapAlloc(g_hHeapWindowInfo,HEAP_ZERO_MEMORY,(iDisplayWindowCount)*sizeof(LPWINDOW_INFO));

		for(int i=0;i<iDisplayWindowCount;++i){
			DisplayWindowInfoTmp[i]=g_DisplayWindowInfo[i];
		}

		int iOrder=0;
		int iOrders[LISTITEM_NUM];

		//先頭のカラムを取得
		ListView_GetColumnOrderArray(g_hListView,LISTITEM_NUM,iOrders);
		iOrder=(g_ListViewColumn_Table[iOrders[0]].bVisible)?iOrders[0]:iOrders[1];

		int iCount=g_iDisplayWindowCount;
		g_iDisplayWindowCount=0;
		bool bFullSearch=!bFirstColumnOnly&&g_ListViewColumn_Table[iOrders[1]].bVisible;

		if(!g_bMigemoSearch){
			//先頭のカラムを対象に絞り込む
			for(int i=0;i<iCount;++i){
				bool bResult=false;
				TCHAR* pszTargetString=(iOrder==LISTITEM_FILENAME)?DisplayWindowInfoTmp[i]->szFileName:DisplayWindowInfoTmp[i]->szWindowTitle;
				TCHAR* pszTargetString2=(iOrders[1]==LISTITEM_FILENAME)?DisplayWindowInfoTmp[i]->szFileName:DisplayWindowInfoTmp[i]->szWindowTitle;

				if(g_iMatchMode!=MATCH_FLEX){
					bResult=(Match(iMatchMode,
								   pszTargetString,
								   szSearchString)||
							 (bFullSearch&&
							  Match(iMatchMode,
									pszTargetString2,
									szSearchString)));
				}else{
					//あいまい検索であればカラムの文字列を結合して検索
					int iTargetStringsLength=(lstrlen(pszTargetString)+1);
					if(bFullSearch)iTargetStringsLength+=(lstrlen(pszTargetString2)+1);
					TCHAR* pszTargetStrings=(TCHAR*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(iTargetStringsLength+1)*sizeof(TCHAR));
					wsprintf(pszTargetStrings,_T("%s%s"),pszTargetString,(bFullSearch)?pszTargetString2:_T(""));

					bResult=Match(iMatchMode,
								  pszTargetStrings,
								  szSearchString);

					HeapFree(GetProcessHeap(),0,pszTargetStrings);
					pszTargetStrings=NULL;
				}

				if(bResult){
					g_DisplayWindowInfo[g_iDisplayWindowCount]=DisplayWindowInfoTmp[i];
					DisplayWindowInfoTmp[i]=NULL;
					g_iDisplayWindowCount++;
				}
			}
		}else{
			int iRegexCount=0;

			//あいまい検索
			migemo_set_operator(g_pmigemo,MIGEMO_OPINDEX_NEWLINE,(const unsigned char*)((iMatchMode!=MATCH_FLEX)?"":".*?"));

			//正規表現リストを作成
			TCHAR**ppszRegexes=AllocRegexes(szSearchString,&iRegexCount);

			for(int i=0;i<iCount;++i){
				bool bResult=true;
				TCHAR msg[BREGEXP_MAX_ERROR_MESSAGE_LEN];

				for(int iRegexIndex=0;bResult&&iRegexIndex<iRegexCount;++iRegexIndex){
					BREGEXP* rxp=NULL;
					TCHAR* pszPattern=NULL;

					pszPattern=(TCHAR*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(lstrlen(ppszRegexes[iRegexIndex])+12)*sizeof(wchar_t));
					wsprintf(pszPattern,_T("%s%s/i"),(iMatchMode==MATCH_FORWARD)?_T("m/^"):_T("m/"),ppszRegexes[iRegexIndex]);

					TCHAR* pszTargetString=(iOrder==LISTITEM_FILENAME)?DisplayWindowInfoTmp[i]->szFileName:DisplayWindowInfoTmp[i]->szWindowTitle;
					TCHAR* pszTargetString2=(iOrders[1]==LISTITEM_FILENAME)?DisplayWindowInfoTmp[i]->szFileName:DisplayWindowInfoTmp[i]->szWindowTitle;

					if(g_iMatchMode!=MATCH_FLEX){
						bResult=(BMatch(pszPattern,
										pszTargetString,
										pszTargetString+lstrlen(pszTargetString),
										&rxp,msg)==1||
								 (bFullSearch&&
								  BMatch(pszPattern,
										 pszTargetString2,
										 pszTargetString2+lstrlen(pszTargetString2),
										 &rxp,msg)==1));
					}else{
						//あいまい検索であればカラムの文字列を結合して検索
						int iTargetStringsLength=(lstrlen(pszTargetString)+1);
						if(bFullSearch)iTargetStringsLength+=(lstrlen(pszTargetString2)+1);
						TCHAR* pszTargetStrings=(TCHAR*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(iTargetStringsLength+1)*sizeof(TCHAR));
						wsprintf(pszTargetStrings,_T("%s%s"),pszTargetString,(bFullSearch)?pszTargetString2:_T(""));

						bResult=(BMatch(pszPattern,
								  pszTargetStrings,
								  pszTargetStrings+lstrlen(pszTargetStrings),
								  &rxp,msg)==1);

						HeapFree(GetProcessHeap(),0,pszTargetStrings);
						pszTargetStrings=NULL;
					}
					if(rxp){
						BRegfree(rxp);
					}
					HeapFree(GetProcessHeap(),0,pszPattern);
					pszPattern=NULL;
				}

				if(bResult){
					g_DisplayWindowInfo[g_iDisplayWindowCount]=DisplayWindowInfoTmp[i];
					DisplayWindowInfoTmp[i]=NULL;
					g_iDisplayWindowCount++;
				}
			}
			//正規表現リストを解放
			FreeRegexes(ppszRegexes,iRegexCount);
		}

		//アイテム選択復元
		if(g_iDisplayWindowCount&&
		   hSelectedItemWnd){
			for(int i=0;i<g_iDisplayWindowCount;++i){
				if(g_DisplayWindowInfo[i]->hWnd==hSelectedItemWnd){
					ListView_SelectItem(g_hListView,i);
					bSelected=true;
					break;
				}
			}
		}
		HeapFree(g_hHeapWindowInfo,0,DisplayWindowInfoTmp);
		DisplayWindowInfoTmp=NULL;
	}

	//Migemo検索中マッチするアイテムがない
	g_bMigemoNomatch=false;

	if(!g_iDisplayWindowCount){
		g_bMigemoNomatch=true;
		g_iDisplayWindowCount=iDisplayWindowCount;

		for(int i=0;i<iDisplayWindowCount;++i){
			g_DisplayWindowInfo[i]=DisplayWindowInfoBak[i];
		}

		if(iSearchStringBack!=-1){
			if(!g_bMigemoSearch){
				//入力を取り消す
				szSearchString[iSearchStringBack]='\0';
			}else{
				SendMessage(g_hListView,WM_SETREDRAW,false,0);

				//ウインドウ数確定
				ListView_SetItemCountEx(g_hListView,g_iDisplayWindowCount,LVSICF_NOINVALIDATEALL|LVSICF_NOSCROLL);
				//リサイズ
				ResizeListView(g_hMainWnd);

				SendMessage(g_hListView,WM_SETREDRAW,true,0);
				ListView_RedrawItems(g_hListView,0,g_iDisplayWindowCount);
			}
		}
	}else{
		SendMessage(g_hListView,WM_SETREDRAW,false,0);

		//ウインドウ数確定
		ListView_SetItemCountEx(g_hListView,g_iDisplayWindowCount,LVSICF_NOINVALIDATEALL|LVSICF_NOSCROLL);
		DWORD dwProcessId=0;
		bool bMouseOut=(GetWindowThreadProcessId(ControlFromPoint(),&dwProcessId)!=0&&
						dwProcessId!=GetCurrentProcessId());

		//リサイズ
		ResizeListView(g_hMainWnd);

		SendMessage(g_hListView,WM_SETREDRAW,true,0);
		ListView_RedrawItems(g_hListView,0,g_iDisplayWindowCount);

		if(g_Config.ListView.bMouseOut){
			//カーソルがウインドウリスト外にあるなら右上へ移動させる
			if(!bMouseOut&&GetWindowThreadProcessId(ControlFromPoint(),&dwProcessId)!=0&&
			   dwProcessId!=GetCurrentProcessId()){
				RECT rc;

				GetWindowRect(g_hListView,&rc);
				ClipCursor(&rc);
				ClipCursor(NULL);
			}
		}

		//候補のウインドウが1つなら確定
		if(g_iDisplayWindowCount==1&&
		   g_bEnterUniqueWindow){
			SetTimer(g_hListView,
					 MAKEWPARAM(ID_TIMER_UNIQUE_WINDOW,0),
					 (g_Config.IncrementalSearch.iUniqueWindowDelay>0)?g_Config.IncrementalSearch.iUniqueWindowDelay:0,
					 NULL);
		}


		bResult=true;
	}

	HeapFree(g_hHeapWindowInfo,0,DisplayWindowInfoBak);
	DisplayWindowInfoBak=NULL;

	if(!bSelected||ListView_GetSelectedItem(g_hListView)==-1){
		//順序が近いウインドウを選択
		int iItem=(g_iDisplayWindowCount>1&&iDisplayWindowCount>1)?(((g_iDisplayWindowCount-1)*iSelectedItem)*10/(iDisplayWindowCount-1)+5)/10
			:0;

		ListView_SelectItem(g_hListView,INRANGE(0,iItem,g_iDisplayWindowCount-1)?iItem:0);
	}

	ShowToolTip(g_hListView,ID_TOOLTIP_SEARCH,szSearchString);
	ShowToolTip(g_hListView,ID_TOOLTIP_INFO);

	return bResult;
}

//設定変更後のインクリメンタルサーチ
bool ReMatch(){
	if(g_szSearchString[0]=='\0')return true;

	//表示を元に戻す
	for(int i=0;i<g_iWindowCount;++i){
		g_DisplayWindowInfo[i]=&g_WindowInfo[i];
	}
	g_iDisplayWindowCount=g_iWindowCount;
	TCHAR szSearchStringBak[INCREMENTAL_SEARCH_BUFFER]={};
	lstrcpy(szSearchStringBak,g_szSearchString);

	//再び絞り込む
	if(StrChr(g_szSearchString,' ')!=NULL){
		TCHAR szSearchString[INCREMENTAL_SEARCH_BUFFER]={};
		bool bResult=true;

		for(int i=0;bResult&&i<lstrlen(g_szSearchString);++i){
			//一文字ずつ検索
			lstrcpyn(szSearchString+i,g_szSearchString+i,sizeof(TCHAR));
			bResult=IncrementalSearch(szSearchString,g_iMatchMode,g_bFirstColumnOnly);
			if(!bResult)break;
		}
		if(bResult){
			ShowToolTip(g_hListView,ID_TOOLTIP_SEARCH,g_szSearchString);
			return true;
		}
	}else{
		if(IncrementalSearch(g_szSearchString,g_iMatchMode,g_bFirstColumnOnly)){
			return true;
		}
	}

	for(int i=0;i<g_iWindowCount;++i){
		g_DisplayWindowInfo[i]=&g_WindowInfo[i];
	}
	g_iDisplayWindowCount=g_iWindowCount;

	int iSelectedItem=ListView_GetSelectedItem(g_hListView);
	ListView_SelectItem(g_hListView,iSelectedItem);

	lstrcpy(g_szSearchString,szSearchStringBak);
	return false;
}

SCC_CORNERS GetCursorCorner(){
	POINT ptMouse;

	GetCursorPos(&ptMouse);

	RECT rc;

	if(g_Config.ShowWindow.bMouseEachMonitor){
		MONITORINFOEX info;

		HMONITOR hMonitor=MonitorFromPoint(ptMouse,MONITOR_DEFAULTTONEAREST);

		info.cbSize=sizeof(MONITORINFOEX);

		GetMonitorInfo(hMonitor,&info);

		rc=info.rcMonitor;
	}else{
		MONITOR_INFO* mi=AllocMonitorsList();
		rc.left=rc.right=rc.bottom=rc.top=0;

		for(size_t i=0,iSize=HeapSize(GetProcessHeap(),0,mi)/sizeof(MONITOR_INFO);
			i<iSize;
			++i){
			if(rc.left>mi[i].info.rcMonitor.left)rc.left=mi[i].info.rcMonitor.left;
			if(rc.right<mi[i].info.rcMonitor.right)rc.right=mi[i].info.rcMonitor.right;
			if(rc.top>mi[i].info.rcMonitor.top)rc.top=mi[i].info.rcMonitor.top;
			if(rc.bottom<mi[i].info.rcMonitor.bottom)rc.bottom=mi[i].info.rcMonitor.bottom;
		}

		FreeMonitorsList(mi);
	}

	//画面端と判断する位置を調整
	rc.left+=g_Config.ShowWindow.iLeftCornerDistance;
	rc.right-=g_Config.ShowWindow.iRightCornerDistance;
	rc.top+=g_Config.ShowWindow.iTopCornerDistance;
	rc.bottom-=g_Config.ShowWindow.iBottomCornerDistance;

	if(ptMouse.x<=rc.left){
		if(ptMouse.y==0&&g_Config.ShowWindow.ccCursorCorners[SCC_LEFT].bValid){
			//左上
			return SCC_LEFTTOP;
		}else if(rc.bottom<=ptMouse.y&&g_Config.ShowWindow.ccCursorCorners[SCC_LEFTBOTTOM].bValid){
			//左下
			return SCC_LEFTBOTTOM;
		}else if(g_Config.ShowWindow.ccCursorCorners[SCC_LEFT].bValid){
			//左
			return SCC_LEFT;
		}
	}else if(rc.right<=ptMouse.x){
		if(ptMouse.y<=rc.top&&g_Config.ShowWindow.ccCursorCorners[SCC_RIGHTTOP].bValid){
			//右上
			return SCC_RIGHTTOP;
		}else if(rc.bottom<=ptMouse.y&&g_Config.ShowWindow.ccCursorCorners[SCC_RIGHTBOTTOM].bValid){
			//右下
			return SCC_RIGHTBOTTOM;
		}else if(g_Config.ShowWindow.ccCursorCorners[SCC_RIGHT].bValid){
			//右
			return SCC_RIGHT;
		}
	}else if(ptMouse.y<=rc.top){
		if(g_Config.ShowWindow.ccCursorCorners[SCC_TOP].bValid){
			//上
			return SCC_TOP;
		}
	}else if(rc.bottom<=ptMouse.y){
		if(g_Config.ShowWindow.ccCursorCorners[SCC_BOTTOM].bValid){
			//下
			return SCC_BOTTOM;
		}
	}
	return SCC_NONE;
}

//MSG_MOUSEMOVEのイベントハンドラ
void MsgMouseMove(WPARAM wParam,LPARAM lParam){
	SCC_CORNERS eCorners=GetCursorCorner();

	if(g_Config.ShowWindow.iCursorCornerDelay&&
	   eCorners!=SCC_NONE){
		for(int i=0;i<ARRAY_SIZEOF(SccTimerTable);i++){
			KillTimer(g_hMainWnd,SccTimerTable[i].uId);
			if(g_eWindowListStatus==WLS_HIDE&&!g_bDisable&&!g_bSettingsDialog&&
			   eCorners==SccTimerTable[i].eCorners){
				SetTimer(g_hMainWnd,SccTimerTable[i].uId,g_Config.ShowWindow.iCursorCornerDelay,NULL);
			}
		}
	}else{
		if(eCorners!=SCC_NONE){
			ShowListView(g_Config.ShowWindow.ccCursorCorners[eCorners].eDestCorner);
		}
	}
	return;
}

//Ctrl+Aでエディットボックスを全選択するフックプロシージャ
LRESULT CALLBACK CtrlAHookProc(int nCode,WPARAM wParam,LPARAM lParam){
	if(nCode<0)return CallNextHookEx(NULL/*This parameter is ignored.(by MSDN)*/,nCode,wParam,lParam);

	LPMSG lpMsg=(MSG*)lParam;
	TCHAR szClassName[128];
	HWND hWnd;

	if(lpMsg->message==WM_KEYDOWN){
		if(lpMsg->wParam=='A'&&
		   GetAsyncKeyState(VK_CONTROL)&0x8000&&
		   GetAsyncKeyState(VK_SHIFT)>=0&&
		   GetAsyncKeyState(VK_MENU)>=0){
			//Ctrl+Aが押下されている場合
			hWnd=GetFocus();
			if(hWnd!=NULL){
				GetClassName(hWnd,szClassName,ARRAY_SIZEOF(szClassName));
				if(lstrcmpi(szClassName,WC_EDIT)==0){
					//フォーカスがエディットコントロールにある
					//全選択
					SendMessage(hWnd,EM_SETSEL,0,-1);
					return true;
				}
			}
		}
	}
	return CallNextHookEx(NULL/*This parameter is ignored.(by MSDN)*/,nCode,wParam,lParam);
}

//リストビュープロシージャ
LRESULT CALLBACK ListProc(HWND hListView, UINT uMsg, WPARAM wParam, LPARAM lParam){
	if(g_bDisable)return CallWindowProc(DefListViewProc,hListView,uMsg,wParam,lParam);

	static bool bMenuLoop;
	static bool bMouseIn;
	static bool bMouseTracking;
	static bool bHideCursor;

	static bool bLButtonWheelTimer;
	static bool bRButtonWheelTimer;
	static bool bX1ButtonWheelTimer;
	static bool bX2ButtonWheelTimer;

	switch(uMsg){
		case WM_ERASEBKGND:
			//ちらつき防止
			return FALSE;
#if 0
		case WM_NCHITTEST:
			//Alt+ドラッグでウインドウの移動
			if(GetAsyncKeyState(VK_MENU)&0x8000){
				return HTTRANSPARENT;
			}
			break;
#endif

		case WM_ENTERMENULOOP:
			bMenuLoop=true;
			break;

		case WM_EXITMENULOOP:
			bMenuLoop=false;
			ShowThumbnail();
			break;

		case WM_INITMENU:
			//親へ投げる
			SendMessage(g_hMainWnd,uMsg,wParam,lParam);
			return 0;

		case WM_MENUSELECT:
			//親へ投げる
			SendMessage(g_hMainWnd,uMsg,wParam,lParam);
			return 0;

		case WM_CHAR:
			if(g_Config.ListView.iTimeOut>0||
			   IsCursorInWindow(g_hMainWnd)&&g_Config.ListView.bDragTimeOut&&IsDragging()){
				//タイムアウトで確定するためタイマーを設定
				SetTimer(g_hListView,
						 MAKEWPARAM(IDM_KEY_SWITCH,ListView_GetSelectedItem(g_hListView)),
						 (g_Config.ListView.iTimeOut>0)?g_Config.ListView.iTimeOut:DEFAULT_SELECT_TIMEOUT,
						 NULL);
			}

			if(lstrlen(g_szSearchString)+sizeof(TCHAR)<INCREMENTAL_SEARCH_BUFFER){
				int iSearchStringBack=0;
				for(int iLength=lstrlen(g_szSearchString);iSearchStringBack<iLength&&g_szSearchString[iSearchStringBack]!='\0';++iSearchStringBack);
				--iSearchStringBack;

				//先頭スペース/二重スペース防止
				if((g_szSearchString[0]=='\0'||g_szSearchString[iSearchStringBack]==' ')&&
				   (TCHAR)wParam==' ')return false;

				TCHAR c=(TCHAR)wParam;

				//制御文字は処理しない
				if(c<=31)return false;

				bool bShift=(GetAsyncKeyState(VK_SHIFT)&0x8000)!=0;

				if(g_pmigemo!=NULL&&
				   g_Config.IncrementalSearch.iMigemoMode!=MIGEMO_NO){
					if(g_szSearchString[0]=='\0'){
						g_bMigemoSearch=(!bShift&&g_Config.IncrementalSearch.iMigemoMode==MIGEMO_LOWERCASE)||
							(bShift&&g_Config.IncrementalSearch.iMigemoMode==MIGEMO_UPPERCASE)||
								g_Config.IncrementalSearch.iMigemoMode==MIGEMO_ALWAYS;
					}

					if(!g_Config.IncrementalSearch.bMigemoCaseSensitive){
						//連文節検索を行わない場合であれば大文字小文字を統一
						if(c>='A'&&c<='Z'){
							c+=32;
						}
						if((g_bMigemoSearch&&g_Config.IncrementalSearch.iMigemoMode==MIGEMO_UPPERCASE)||
						   (!g_bMigemoSearch&&g_Config.IncrementalSearch.iMigemoMode==MIGEMO_LOWERCASE)){
							if(c>='a'&&c<='z'){
								c-=32;
							}
						}
					}
				}

				TCHAR key[2]={c};
				lstrcat(g_szSearchString,key);

				if(!g_bMigemoSearch||g_Config.IncrementalSearch.iMigemoDelay==0){
					IncrementalSearch(g_szSearchString,g_iMatchMode,g_bFirstColumnOnly);
				}else{
					ShowToolTip(g_hListView,ID_TOOLTIP_SEARCH,g_szSearchString);
					SetTimer(g_hMainWnd,uMsg,g_Config.IncrementalSearch.iMigemoDelay,NULL);
				}
				return false;
			}
			//音が鳴るのを防止
			return FALSE;

		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:{
			//とりあえず選択する
			int iIndex=ListView_GetHitItem(g_hListView);
			ListView_SelectItem(g_hListView,iIndex);

			//CtrlとShiftキーの状態を保存
			//ダブルクリックとShift(or Ctrl)の組み合わせで2回目のクリックの段階でキーを押下していなくても処理するように
			g_byModKey=0;
			if(GET_KEYSTATE_WPARAM(wParam)&MK_SHIFT)g_byModKey|=MODF_SHIFT;
			if(GET_KEYSTATE_WPARAM(wParam)&MK_CONTROL)g_byModKey|=MODF_CONTROL;

			bool bDoubleClickCmd=false;

			if(!g_Config.ListView.bDisableDoubleClick){
				for(int i=0;i<SMOUSE_NUM;++i){
					if(!g_Config.Mouse.sMouseTable[i].uCmdId)continue;
					if(uMsg==WM_LBUTTONDOWN&&
					   MouseType_Table[i].uMessageType==WM_LBUTTONDBLCLK||
					   uMsg==WM_RBUTTONDOWN&&
					   MouseType_Table[i].uMessageType==WM_RBUTTONDBLCLK){
						bDoubleClickCmd=true;
						break;
					}
				}
			}

			if(!bDoubleClickCmd){
				bool bMouseWheelCmd=false;

				for(int i=0;i<SMOUSE_NUM;++i){
					if(!g_Config.Mouse.sMouseTable[i].uCmdId)continue;
					if(MouseType_Table[i].uMouseWheel&&
					   MouseType_Table[i].uMessageType==uMsg){
						bMouseWheelCmd=true;
						break;
					}
				}

				if(!bMouseWheelCmd){
					//選択して処理へ
					SetTimer(g_hMainWnd,uMsg,50,NULL);
				}else{
					//クリック+ホイールの設定があるため、一旦待機
					if(uMsg==WM_LBUTTONDOWN){
						bLButtonWheelTimer=true;
					}else{
						bRButtonWheelTimer=true;
					}
					SetTimer(g_hMainWnd,uMsg,g_Config.ListView.iMouseWheelClickTime,NULL);
				}
			}else{
				//ダブルクリックか否か判定するためにタイマーを使用
				SetTimer(g_hMainWnd,uMsg,g_Config.ListView.iDoubleClickTime,NULL);
			}
			return FALSE;
		}

		case WM_LBUTTONDBLCLK:
			KillTimer(g_hMainWnd,WM_LBUTTONDOWN);
			SendMessage(g_hMainWnd,WM_TIMER,uMsg,0);
			return FALSE;

		case WM_RBUTTONDBLCLK:
			KillTimer(g_hMainWnd,WM_RBUTTONDOWN);
			SendMessage(g_hMainWnd,WM_TIMER,uMsg,0);
			return FALSE;

		case WM_LBUTTONUP:
			if(bLButtonWheelTimer){
				bLButtonWheelTimer=false;
				KillTimer(g_hMainWnd,WM_LBUTTONDOWN);
				SendMessage(g_hMainWnd,WM_TIMER,WM_LBUTTONDOWN,0);
			}
			bLButtonWheelTimer=false;
			break;

		case WM_RBUTTONUP:
			if(bRButtonWheelTimer){
				bRButtonWheelTimer=false;
				KillTimer(g_hMainWnd,WM_RBUTTONDOWN);
				SendMessage(g_hMainWnd,WM_TIMER,WM_RBUTTONDOWN,0);
			}
			bRButtonWheelTimer=false;
			break;

		case WM_MBUTTONDOWN:
			if(GET_KEYSTATE_WPARAM(wParam)&MK_MBUTTON){
				g_byModKey=0;
				if(GET_KEYSTATE_WPARAM(wParam)&MK_SHIFT)g_byModKey|=MODF_SHIFT;
				if(GET_KEYSTATE_WPARAM(wParam)&MK_CONTROL)g_byModKey|=MODF_CONTROL;

				bool bDoubleClickCmd=false;

				if(!g_Config.ListView.bDisableDoubleClick){
					for(int i=0;i<SMOUSE_NUM;++i){
						if(!g_Config.Mouse.sMouseTable[i].uCmdId)continue;
						if(uMsg==WM_MBUTTONDOWN&&
						   MouseType_Table[i].uMessageType==WM_MBUTTONDBLCLK){
							bDoubleClickCmd=true;
							break;
						}
					}
				}

				if(!bDoubleClickCmd){
					SendMessage(g_hMainWnd,WM_TIMER,uMsg,0);
				}else{
					//ダブルクリックか否か判定するためにタイマーを使用
					SetTimer(g_hMainWnd,uMsg,g_Config.ListView.iDoubleClickTime,NULL);
				}
				return FALSE;
			}
			break;

		case WM_MBUTTONDBLCLK:
			if(GET_KEYSTATE_WPARAM(wParam)&MK_MBUTTON){
				KillTimer(g_hMainWnd,WM_MBUTTONDOWN);
				SendMessage(g_hMainWnd,WM_TIMER,uMsg,0);
				return FALSE;
			}
			break;

		case WM_XBUTTONDOWN:{
			UINT uXButtonMsg=(GET_XBUTTON_WPARAM(wParam)&MK_XBUTTON1)?WM_X1BUTTONDOWN:WM_X2BUTTONDOWN;

			g_byModKey=0;
			if(GET_KEYSTATE_WPARAM(wParam)&MK_SHIFT)g_byModKey|=MODF_SHIFT;
			if(GET_KEYSTATE_WPARAM(wParam)&MK_CONTROL)g_byModKey|=MODF_CONTROL;

			if(g_Config.ListView.bDisableDoubleClick){
				bool bMouseWheelCmd=false;

				for(int i=0;i<SMOUSE_NUM;++i){
					if(!g_Config.Mouse.sMouseTable[i].uCmdId)continue;
					if(MouseType_Table[i].uMouseWheel&&
					   MouseType_Table[i].uMessageType==uMsg){
						bMouseWheelCmd=true;
						break;
					}
				}

				if(!bMouseWheelCmd){
					SetTimer(g_hMainWnd,uXButtonMsg,50,NULL);
				}else{
					//クリック+ホイールの設定があるため、一旦待機
					if(uXButtonMsg==WM_X1BUTTONDOWN){
						bX1ButtonWheelTimer=true;
					}else{
						bX2ButtonWheelTimer=true;
					}
					SetTimer(g_hMainWnd,uXButtonMsg,g_Config.ListView.iMouseWheelClickTime,NULL);
				}
			}else{
				//ダブルクリックか否か判定するためにタイマーを使用
				SetTimer(g_hMainWnd,uXButtonMsg,g_Config.ListView.iDoubleClickTime,NULL);
			}
			return FALSE;
		}

		case WM_XBUTTONDBLCLK:
			if(GET_XBUTTON_WPARAM(wParam)&MK_XBUTTON1){
				KillTimer(g_hMainWnd,WM_X1BUTTONDOWN);
				SendMessage(g_hMainWnd,WM_TIMER,WM_X1BUTTONDBLCLK,0);
			}else{
				KillTimer(g_hMainWnd,WM_X2BUTTONDOWN);
				SendMessage(g_hMainWnd,WM_TIMER,WM_X2BUTTONDBLCLK,0);
			}
			return FALSE;

		case WM_XBUTTONUP:
			if(GET_XBUTTON_WPARAM(wParam)&MK_XBUTTON1){
				if(bX1ButtonWheelTimer){
					bX1ButtonWheelTimer=false;
					KillTimer(g_hMainWnd,WM_X1BUTTONDOWN);
					SendMessage(g_hMainWnd,WM_TIMER,WM_X1BUTTONDOWN,0);
				}
				bX1ButtonWheelTimer=false;
			}else{
				if(bX2ButtonWheelTimer){
					bX2ButtonWheelTimer=false;
					KillTimer(g_hMainWnd,WM_X2BUTTONDOWN);
					SendMessage(g_hMainWnd,WM_TIMER,WM_X2BUTTONDOWN,0);
				}
				bX2ButtonWheelTimer=false;
			}
			break;

		case WM_KEYDOWN:
			//キー入力無効
			return FALSE;

		case WM_NCCALCSIZE:
			//スクロールバーを削除
			ShowScrollBar(hListView,SB_BOTH,false);
			break;

		case WM_SIZE:
			if(wParam==0&&lParam==0){
				bMouseIn=bMouseTracking=bMenuLoop=false;
			}
			break;

		case WM_TIMER:{
			switch(LOWORD(wParam)){
				case ID_TIMER_READY:
					//WM_SETFOCUS(WLS_CREATED時)の続き...
					KillTimer(hListView,wParam);
					//一時的な設定をリセット
					ResetTemporarySetting();

					bLButtonWheelTimer=bRButtonWheelTimer=bX1ButtonWheelTimer=bX2ButtonWheelTimer=false;

					ShowThumbnail();

					if(g_Config.ListView.iTimeOut>0||
					   IsCursorInWindow(g_hMainWnd)&&g_Config.ListView.bDragTimeOut&&IsDragging()){
						//タイムアウトで確定するためタイマーを設定
						SetTimer(g_hListView,
								MAKEWPARAM(IDM_KEY_SWITCH,ListView_GetSelectedItem(g_hListView)),
								(g_Config.ListView.iTimeOut>0)?g_Config.ListView.iTimeOut:DEFAULT_SELECT_TIMEOUT,
								NULL);
					}

					g_eWindowListStatus=WLS_DISPLAYED;

					//カーソルを閉じ込める
					if(g_eCorners!=SCC_NONE&&
					   (g_Config.ShowWindow.iMouseClipCursorMode!=CLIP_CURSOR_NO&&!g_bHotKeyMode||
						g_Config.ShowWindow.iHotKeyClipCursorMode!=CLIP_CURSOR_NO&&g_bHotKeyMode)){
						bool bMoveCursorAtStart=
							(!g_bHotKeyMode&&g_Config.ShowWindow.bMouseMoveCursorSelectedWindow||
							 g_bHotKeyMode&&g_Config.ShowWindow.bHotKeyMoveCursorSelectedWindow)&&
								g_Config.ShowWindow.bMoveCursorAtStart;
						int iClipCursorMode=(!g_bHotKeyMode)?g_Config.ShowWindow.iMouseClipCursorMode:g_Config.ShowWindow.iHotKeyClipCursorMode;
						int iClipCursorTime=(!g_bHotKeyMode)?g_Config.ShowWindow.iMouseClipCursorTime:g_Config.ShowWindow.iHotKeyClipCursorTime;
						RECT rcClip;

						if(bMoveCursorAtStart){
							ListView_GetItemRect(g_hListView,ListView_GetSelectedItem(g_hListView),
												 &rcClip,
												 LVIR_BOUNDS);
							POINT ptLeftTop={rcClip.left,rcClip.top},ptRightBottom={rcClip.right,rcClip.bottom};

							ClientToScreen(g_hListView,&ptLeftTop);
							ClientToScreen(g_hListView,&ptRightBottom);
							rcClip.left=ptLeftTop.x;
							rcClip.top=ptLeftTop.y;
							rcClip.bottom=ptRightBottom.y;
							rcClip.right=ptRightBottom.x;
						}else{
							GetWindowRect(g_hListView,&rcClip);
						}

						if(iClipCursorMode==CLIP_CURSOR_TOP_BOTTOM){
							rcClip.top+=3;
							rcClip.bottom-=3;
							rcClip.left-=500;
							rcClip.right+=500;
						}else if(iClipCursorMode==CLIP_CURSOR_ALL){
							rcClip.top+=3;
							rcClip.bottom-=3;
							rcClip.left+=3;
							rcClip.right-=3;
						}
						ClipCursor(&rcClip);
						SetTimer(g_hListView,
								 ID_TIMER_CLIP_CURSOR,
								 iClipCursorTime,
								 NULL);
					}

					//カーソル非表示設定
					KillTimer(hListView,ID_TIMER_HIDE_CURSOR);
					if(g_Config.ListView.iHideCursorTime!=0){
						SetCursor((HCURSOR)LoadImage(NULL,IDC_ARROW,IMAGE_CURSOR,0,0,LR_DEFAULTSIZE|LR_SHARED));
						SetClassLongPtr(hListView,
										GCLP_HCURSOR,
										(LONG_PTR)LoadImage(NULL,IDC_ARROW,IMAGE_CURSOR,0,0,LR_DEFAULTSIZE|LR_SHARED));
						bHideCursor=false;
					}
					if(g_Config.ListView.iHideCursorTime>=0){
						SetTimer(hListView,
								 ID_TIMER_HIDE_CURSOR,
								 g_Config.ListView.iHideCursorTime,
								 NULL);
					}

					if(g_Config.ListView.bMouseOut&&
					   g_eCorners!=SCC_NONE){
						if(!IsCursorInWindow(g_hListView)){
							//既にカーソルがウインドウリスト外なら確定
							int iIndex=ListView_GetSelectedItem(g_hListView);

							if(iIndex!=-1){
								TaskSwitch(g_DisplayWindowInfo[iIndex]);
							}else{
								HideListView();
							}
						}
					}
					break;
	
				case IDM_KEY_SWITCH:{
					KillTimer(hListView,wParam);
					if(g_eWindowListStatus!=WLS_DISPLAYED)break;
					if(GetAsyncKeyState(VK_MENU)&0x8000)break;
					if(bMenuLoop)break;
					if(IsWindowVisible(g_hToolTipInfo))break;

					if(g_Config.ListView.iTimeOut>0||
					   IsCursorInWindow(g_hMainWnd)&&g_Config.ListView.bDragTimeOut&&IsDragging()){

						//タイムアウトで確定
						int iSelectedItem=ListView_GetSelectedItem(hListView);

						if(iSelectedItem==HIWORD(wParam)){
							SendMessage(g_hMainWnd,WM_COMMAND,MAKEWPARAM(IDM_KEY_SWITCH,0),0);
						}
					}
					break;
				}
#if 0
				case ID_TIMER_HOVER:{
					//マウスホバーで選択
					KillTimer(hListView,wParam);
					if(g_eWindowListStatus!=WLS_DISPLAYED)break;

					int iIndex=ListView_GetHitItem(hListView);

					if(iIndex==HIWORD(wParam)){
						ListView_SelectItem(hListView,iIndex);
					}
					break;
				}
#endif
				case ID_TIMER_UNIQUE_WINDOW:{
					//候補が1つになった時の切り替え
					KillTimer(hListView,wParam);

					if(g_eWindowListStatus!=WLS_DISPLAYED)break;
					if(bMenuLoop)break;
					if(IsWindowVisible(g_hToolTipInfo))break;

					if(g_iDisplayWindowCount==1){
						TaskSwitch(g_DisplayWindowInfo[0]);
					}
					break;
				}

				case ID_TIMER_HIDELISTVIEW:
					//メニュー表示中のホットキーによるキャンセル時の不具合対策
					HideListView();
					break;

				case ID_TIMER_HIDE_CURSOR:
					//カーソルを隠す
					KillTimer(hListView,wParam);

					if(g_eWindowListStatus==WLS_DISPLAYED&&
					   g_Config.ListView.iHideCursorTime>=0){
						SetCursor(g_hBlankCursor);
						SetClassLongPtr(hListView,GCLP_HCURSOR,(LONG_PTR)g_hBlankCursor);
						bHideCursor=true;
					}
					break;

				case ID_TIMER_CLIP_CURSOR:
					//カーソル閉じ込めを解除
					KillTimer(hListView,wParam);
					ClipCursor(NULL);
					break;
			}
			break;
		}

		case WM_MOUSELEAVE:{
			bMouseIn=bMouseTracking=false;
			//マウスカーソルがウインドウ外に移動した場合
			if(g_eWindowListStatus!=WLS_DISPLAYED)break;

			if(!g_Config.ListView.bMouseOut)break;

			if(GetAsyncKeyState(VK_MENU)&0x8000)break;
#if 0
			if(ControlFromPoint()==hListView)break;

			TCHAR szClassName[128];

			GetClassName(ControlFromPoint(),szClassName,ARRAY_SIZEOF(szClassName));
			if(lstrcmp(szClassName,TOOLTIPS_CLASS)==0||
			   lstrcmp(szClassName,_T("#32768"))==0)break;
#endif
			//カーソル下のウインドウ判定
			DWORD dwProcessId=0;

			if(GetWindowThreadProcessId(ControlFromPoint(),&dwProcessId)!=0&&
			   dwProcessId==GetCurrentProcessId()){
				break;
			}

			//メニュー表示中
			if(bMenuLoop)break;

			int iIndex=ListView_GetSelectedItem(hListView);

			if(iIndex!=-1){
				TaskSwitch(g_DisplayWindowInfo[iIndex]);
			}else{
				HideListView();
			}
			break;
		}

		default:{
			if(uMsg==MSG_MOUSEMOVE){
				if(g_eWindowListStatus==WLS_DISPLAYED){
					if(IsCursorInWindow(g_hMainWnd)){
						bMouseIn=true;
						if(g_Config.ListView.iHideCursorTime>0){
							KillTimer(hListView,ID_TIMER_HIDE_CURSOR);
							if(bHideCursor){
								SetCursor((HCURSOR)LoadImage(NULL,IDC_ARROW,IMAGE_CURSOR,0,0,LR_DEFAULTSIZE|LR_SHARED));
								SetClassLongPtr(hListView,
												GCLP_HCURSOR,
												(LONG_PTR)LoadImage(NULL,IDC_ARROW,IMAGE_CURSOR,0,0,LR_DEFAULTSIZE|LR_SHARED));
								bHideCursor=false;
							}
							SetTimer(hListView,
									 ID_TIMER_HIDE_CURSOR,
									 g_Config.ListView.iHideCursorTime,
									 NULL);
						}

						//マウスホバーで選択
						if(bMenuLoop)break;
						if(GetAsyncKeyState(VK_MENU)&0x8000)break;
						if(IsWindowVisible(g_hToolTipInfo))break;

						if(g_Config.ListView.bMouseHover||
						   g_Config.ListView.bDragMouseHover&&IsDragging()){
							int iIndex=ListView_GetHitItem(hListView);
							if(iIndex<0)break;
#if 0
							UINT uiHoverTime=0;
							SystemParametersInfo(SPI_GETMOUSEHOVERTIME,0,&uiHoverTime,0);
							SetTimer(hListView,MAKEWPARAM(ID_TIMER_HOVER,iIndex),uiHoverTime,NULL);
#endif
							ListView_SelectItem(hListView,iIndex);
						}
						break;
					}else if(g_Config.ListView.bMouseOut&&
							 bMouseIn){
						if(GetForegroundWindow()!=g_hMainWnd)SetForegroundWindow(g_hMainWnd);
						SetFocus(hListView);
						if(!bMouseTracking){
							bMouseTracking=true;
							MouseLeaveEvent(hListView);
						}
					}
				}
			}else if(uMsg==MSG_MOUSEWHEEL&&
					 g_eWindowListStatus==WLS_DISPLAYED&&
					 IsCursorInWindow(g_hMainWnd)){
				//マウスホイール操作
				BYTE byMod=0;

				if(GetAsyncKeyState(VK_SHIFT)&0x8000)byMod|=MODF_SHIFT;
				if(GetAsyncKeyState(VK_CONTROL)&0x8000)byMod|=MODF_CONTROL;
				if(!IsDragging()&&GetAsyncKeyState(VK_LBUTTON)&0x8000)byMod|=MODF_LBUTTON;
				if(!IsDragging()&&GetAsyncKeyState(VK_RBUTTON)&0x8000)byMod|=MODF_RBUTTON;
				if(GetAsyncKeyState(VK_XBUTTON1)&0x8000)byMod|=MODF_X1BUTTON;
				if(GetAsyncKeyState(VK_XBUTTON2)&0x8000)byMod|=MODF_X2BUTTON;


				for(int i=0;i<SMOUSE_NUM;++i){
					if(!g_Config.Mouse.sMouseTable[i].uCmdId)continue;
					if(MouseType_Table[i].byMod==byMod){
						if(MouseType_Table[i].uMouseWheel==MOUSEWHEEL_UP&&
						   GET_WHEEL_DELTA_WPARAM(wParam)>0||
						   MouseType_Table[i].uMouseWheel==MOUSEWHEEL_DOWN&&
							 GET_WHEEL_DELTA_WPARAM(wParam)<0){
							if(MouseType_Table[i].byMod&MODF_LBUTTON){
								KillTimer(g_hMainWnd,WM_LBUTTONDOWN);
								bLButtonWheelTimer=false;
							}
							if(MouseType_Table[i].byMod&MODF_RBUTTON){
								KillTimer(g_hMainWnd,WM_RBUTTONDOWN);
								bRButtonWheelTimer=false;
							}
							if(MouseType_Table[i].byMod&MODF_MBUTTON){
								KillTimer(g_hMainWnd,WM_MBUTTONDOWN);
							}
							if(MouseType_Table[i].byMod&MODF_X1BUTTON){
								KillTimer(g_hMainWnd,WM_X1BUTTONDOWN);
								bX1ButtonWheelTimer=false;
							}
							if(MouseType_Table[i].byMod&MODF_X2BUTTON){
								KillTimer(g_hMainWnd,WM_X2BUTTONDOWN);
								bX2ButtonWheelTimer=false;
							}
							SendMessage(g_hMainWnd,WM_COMMAND,MAKEWPARAM(g_Config.Mouse.sMouseTable[i].uCmdId,0),0);
							break;
						}
					}
				}
			}
			break;
		}
	}

	return CallWindowProc(DefListViewProc,hListView,uMsg,wParam,lParam);
}

//サムネイル表示ウインドウプロシージャ
LRESULT CALLBACK ThumbnailWindowProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam){
	switch(uMsg){
		case WM_ERASEBKGND:
		case WM_ENTERMENULOOP:
		case WM_EXITMENULOOP:
		case WM_INITMENU:
		case WM_MENUSELECT:
		case WM_CHAR:
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_KEYDOWN:
		case WM_NCCALCSIZE:
		case WM_MOUSEWHEEL:
		case WM_SIZE:
		case WM_MOUSEMOVE:
		case WM_MOUSELEAVE:
			SendMessage(g_hListView,uMsg,wParam,lParam);
			break;
		case WM_TIMER:
			if(LOWORD(wParam)==ID_TIMER_THUMBNAIL){
				KillTimer(hWnd,wParam);

				if(g_Config.ListView.iThumbnailDelay==0){
					//表示までの時間が0ならちらつき防止のためにここで初期化
					for(int i=0;i<g_iWindowCount;++i){
						if(g_WindowInfo[i].hThumbnail){
							pDwmUnregisterThumbnail(g_WindowInfo[i].hThumbnail);
							g_WindowInfo[i].hThumbnail=NULL;
						}
					}
				}

				int iIndex=ListView_GetSelectedItem(g_hListView);

				if(iIndex<0)break;

				if(g_hMenuWnd&&IsWindowVisible(g_hMenuWnd)||
				   g_hToolTipInfo&&IsWindowVisible(g_hToolTipInfo)){
					break;
				}

				if(g_DisplayWindowInfo[iIndex]->bCancelItem){
					break;
				}

				if(!g_DisplayWindowInfo[iIndex]->hThumbnail){
					pDwmRegisterThumbnail(g_hThumbnailWnd,
										  (!g_DisplayWindowInfo[iIndex]->bDesktopItem)?g_DisplayWindowInfo[iIndex]->hWnd:GetShellWindow(),
										  &g_DisplayWindowInfo[iIndex]->hThumbnail);
				}
				if(g_DisplayWindowInfo[iIndex]->hThumbnail){

					SIZE size;

					pDwmQueryThumbnailSourceSize(g_DisplayWindowInfo[iIndex]->hThumbnail,&size);

					RECT rcDest={0,0,0,0};
					float rY=(float)g_Config.ListView.iThumbnailSize/size.cy;
					if(!g_bThumbnailOriginalSize&&
					   g_Config.ListView.iThumbnailSize<size.cy){
						rcDest.bottom=(UINT)(size.cy*rY);
						rcDest.right=(UINT)(size.cx*rY);
					}else{
						rcDest.bottom=size.cy;
						rcDest.right=size.cx;
					}

					DWM_THUMBNAIL_PROPERTIES dtp;
					dtp.dwFlags=DWM_TNP_RECTDESTINATION|DWM_TNP_VISIBLE|DWM_TNP_SOURCECLIENTAREAONLY;
					dtp.fSourceClientAreaOnly=FALSE;
					dtp.fVisible=TRUE;
					dtp.rcDestination=rcDest;
					pDwmUpdateThumbnailProperties(g_DisplayWindowInfo[iIndex]->hThumbnail,&dtp);

					RECT rc;
					int iOrders[LISTITEM_NUM];

					//先頭のカラムを取得
					ListView_GetColumnOrderArray(g_hListView,LISTITEM_NUM,iOrders);
					int iFirstColumn=(g_ListViewColumn_Table[iOrders[0]].bVisible)?iOrders[0]:iOrders[1];

					int iSubItem=0;

					switch(g_Config.ListView.uThumbnailVerticalAlign){
						case THUMBNAIL_VERTICALALIGN_TOP:
							break;
						case THUMBNAIL_VERTICALALIGN_MIDDLE:
							iSubItem=g_iDisplayWindowCount/3;
							break;
						case THUMBNAIL_VERTICALALIGN_BOTTOM:
							iSubItem=g_iWindowCount-1;
							break;
						case THUMBNAIL_VERTICALALIGN_SELECTED_ITEM:
							iSubItem=iIndex;
							break;
						default:
							break;
					}
					ListView_GetSubItemRect(g_hListView,
											iSubItem,
											0,
											LVIR_BOUNDS,&rc);

					rc.left=0;

					if(g_uThumbnail==THUMBNAIL_FIRST_COLUMN){
						rc.left+=((g_Config.ListView.iIcon==LISTICON_SMALL)?
								  16+5:
								  ((g_Config.ListView.iIcon<=LISTICON_NO_SMALL)?
								   0:
								   32+5));
					}

					if(g_uThumbnail==THUMBNAIL_SECOND_COLUMN){
						if(iFirstColumn==LISTITEM_FILENAME){
							rc.left=g_Config.ListView.iFileNameWidth;
						}else if(iFirstColumn==LISTITEM_WINDOWTITLE){
							rc.left=g_Config.ListView.iWindowTitleWidth;
						}
					}

					POINT pt={rc.left,rc.top};

					if(g_uThumbnail==THUMBNAIL_LEFT_SIDE){
						pt.x=-(rcDest.right-rcDest.left);
					}else if(g_uThumbnail==THUMBNAIL_RIGHT_SIDE){
						pt.x=rc.right;
					}

					if(g_Config.ListView.uThumbnailVerticalAlign==THUMBNAIL_VERTICALALIGN_BOTTOM){
						pt.y=rc.bottom-(rcDest.bottom-rcDest.top);
					}

					ClientToScreen(g_hListView,&pt);

					SetWindowPos(g_hThumbnailWnd,HWND_TOPMOST,pt.x,pt.y,rcDest.right-rcDest.left,rcDest.bottom-rcDest.top,SWP_NOACTIVATE|SWP_NOSENDCHANGING|SWP_SHOWWINDOW);
				}
			}
			break;
		default:
			break;
	}
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

//ウインドウプロシージャ
LRESULT CALLBACK WindowProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam){
	//タスクトレイメニューハンドル
	static MENU mTray;
	//コマンドメニュー群
	static MENU mMainCmd;
	static MENU mSelectCmd;
	static MENU mCopyInfoCmd;
	static MENU mIncrementalSearchCmd;
	static MENU mWindowCmd;
	static MENU mProcessCmd;
	static MENU mOpacityCmd;
	static MENU mCommand;

	//子プロセスメニュー関係
	static HMENU hChildProcessMenu;
	static ULONG_PTR ulSelectedProcessId;
	//モニタ一覧メニュー
	static HMENU hMoveToMonitorMenu;

	static HWND hSelectedItemWnd;

	switch(uMsg){
		case WM_NCHITTEST:
			//Aeroが有効だとちらつく?(DWMとマウスフックの相性が悪い?)ので機能廃止
			//ウインドウの移動
//			return HTCAPTION;
			//WS_THICKFRAMEでもサイズ変更させないため
			return FALSE;

		case WM_ERASEBKGND:
			//ちらつき防止
			return FALSE;

		case WM_CREATE:{
			INITCOMMONCONTROLSEX ic={sizeof(INITCOMMONCONTROLSEX)};

			//リストビュー、ホットキー
			ic.dwICC=ICC_LISTVIEW_CLASSES|ICC_HOTKEY_CLASS;
			InitCommonControlsEx(&ic);

			CoInitialize(NULL);

			{
				//フレームの幅と高さを計算
				RECT rcWindow,rcClient;

				GetClientRect(hWnd,&rcClient);
				rcWindow=rcClient;
				AdjustWindowRectEx(&rcWindow,(DWORD)GetWindowLongPtr(hWnd,GWL_STYLE),false,(DWORD)GetWindowLongPtr(hWnd,GWL_EXSTYLE));
				g_lFrameWidth=((rcWindow.right-rcWindow.left)-(rcClient.right-rcClient.left));
				g_lFrameHeight=((rcWindow.bottom-rcWindow.top)-(rcClient.bottom-rcClient.top));
			}

			if(!g_Config.ListView.bDialogFrame){
				//フレーム削除
				SetWindowLongPtr(hWnd,GWL_EXSTYLE,GetWindowLongPtr(hWnd,GWL_EXSTYLE)&~WS_EX_WINDOWEDGE);
				SetWindowLongPtr(hWnd,GWL_STYLE,GetWindowLongPtr(hWnd,GWL_STYLE)&~WS_THICKFRAME);
				SetWindowPos(hWnd,NULL,0,0,0,0,(SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED));
			}

			//リストビューを作成
			g_hListView=CreateWindowEx(WS_EX_ACCEPTFILES,
									   WC_LISTVIEW,
									   _T(""),
									   WS_CHILD|WS_VISIBLE|LVS_REPORT|LVS_ICON|LVS_SINGLESEL|LVS_NOCOLUMNHEADER|LVS_OWNERDATA|LVS_OWNERDRAWFIXED,
									   0,0,0,0,
									   hWnd,
									   (HMENU)IDC_LISTVIEW,
									   g_hInstance,
									   NULL);

			//IMEを無効化
			ImmAssociateContext(g_hListView,0);

			//ダブルバッファ追加
			ListView_SetExtendedListViewStyle(g_hListView,
											  ListView_GetExtendedListViewStyle(g_hListView)|
											  LVS_EX_DOUBLEBUFFER);

			//サブクラス化
			DefListViewProc=(WNDPROC)GetWindowLongPtr(g_hListView,GWLP_WNDPROC);
			SetWindowLongPtr(g_hListView,GWLP_WNDPROC,(LONG_PTR)ListProc);

			//カラムを設定
			LisView_InitColumn(g_ListViewColumn_Table,&g_Config,g_hListView);

			LVCOLUMN lvColumn;
			lvColumn.mask=LVCF_FMT|LVCF_TEXT|LVCF_SUBITEM;
			lvColumn.fmt=LVCFMT_LEFT;
			//現在のファイル名
			lvColumn.pszText=(LPTSTR)_T("ファイル名");
			lvColumn.iSubItem=LISTITEM_FILENAME;
			ListView_InsertColumn(g_hListView,LISTITEM_FILENAME,&lvColumn);
			//新しいファイル名
			lvColumn.pszText=(LPTSTR)_T("ウインドウタイトル");
			lvColumn.iSubItem=LISTITEM_WINDOWTITLE;
			ListView_InsertColumn(g_hListView,LISTITEM_WINDOWTITLE,&lvColumn);

			//リストビューカラムの幅を設定
			for(UINT i=0;i<ARRAY_SIZEOF(g_ListViewColumn_Table);++i){
				ListView_SetColumnWidth(g_hListView,g_ListViewColumn_Table[i].iListItem,*g_ListViewColumn_Table[i].piWidth);
			}

			//リストビューカラムの並び順を設定
			ListView_SetColumnOrderArray(g_hListView,LISTITEM_NUM,g_Config.ListView.iColumnOrder);

			//ツールチップを作成
			g_hToolTipSearch=CreateWindowEx(WS_EX_TOPMOST|WS_EX_TRANSPARENT,
									  TOOLTIPS_CLASS,
									  NULL,
									  WS_POPUP|TTS_NOFADE|TTS_NOANIMATE|TTS_NOPREFIX,
									  CW_USEDEFAULT,
									  CW_USEDEFAULT,
									  CW_USEDEFAULT,
									  CW_USEDEFAULT,
									  hWnd,
									  NULL,
									  g_hInstance,
									  NULL);

			TOOLINFO ti={sizeof(TOOLINFO)};

			ti.uFlags=TTF_TRACK|TTF_SUBCLASS;
			ti.hwnd=g_hListView;
			ti.hinst=g_hInstance;
			ti.uId=ID_TOOLTIP_SEARCH;
			ti.lpszText=NULL;
			//ツールチップを登録
			SendMessage(g_hToolTipSearch,TTM_ADDTOOL,0,(LPARAM)&ti);

			g_hToolTipInfo=CreateWindowEx(WS_EX_TOPMOST|WS_EX_TRANSPARENT,
									  TOOLTIPS_CLASS,
									  NULL,
									  WS_POPUP|TTS_NOFADE|TTS_NOANIMATE|TTS_NOPREFIX,
									  CW_USEDEFAULT,
									  CW_USEDEFAULT,
									  CW_USEDEFAULT,
									  CW_USEDEFAULT,
									  hWnd,
									  NULL,
									  g_hInstance,
									  NULL);

			ti.uId=ID_TOOLTIP_INFO;
			//ツールチップを登録
			SendMessage(g_hToolTipInfo,TTM_ADDTOOL,0,(LPARAM)&ti);

			//フォントを読み込む
			g_hDefaultItemFont=CreateGUIFont(g_Config.DefaultItemDesign.szFont,g_Config.DefaultItemDesign.iFontSize,g_Config.DefaultItemDesign.iFontStyle);
			g_hSelectedItemFont=CreateGUIFont(g_Config.SelectedItemDesign.szFont,g_Config.SelectedItemDesign.iFontSize,g_Config.SelectedItemDesign.iFontStyle);

			//タスクトレイメニューを読み込む
			mTray.hMenu=LoadMenu(g_hInstance,MAKEINTRESOURCE(IDR_TRAYMENU));
			mTray.hSubMenu=GetSubMenu(mTray.hMenu,0);

			//タスクトレイアイコン読み込み
			g_hTrayIcon=(HICON)LoadImage(g_hInstance,MAKEINTRESOURCE(IDI_ICON1),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
			g_hDisableTrayIcon=(HICON)LoadImage(g_hInstance,MAKEINTRESOURCE(IDI_ICON_DISABLE),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);

			//タスクトレイに登録
			TaskTray(hWnd,NIM_ADD);

			//非表示用カーソル読み込み
			g_hBlankCursor=(HCURSOR)LoadImage(g_hInstance,MAKEINTRESOURCE(IDI_CURSOR_BLANK),IMAGE_CURSOR,0,0,LR_DEFAULTSIZE);

			//イメージリスト登録
			ListView_InitImageList(g_hListView,g_Config.ListView.iIcon==LISTICON_NO_SMALL||g_Config.ListView.iIcon==LISTICON_SMALL);

			//フォントを読み込む
			g_hDefaultItemFont=CreateGUIFont(g_Config.DefaultItemDesign.szFont,g_Config.DefaultItemDesign.iFontSize,g_Config.DefaultItemDesign.iFontStyle);
			g_hSelectedItemFont=CreateGUIFont(g_Config.SelectedItemDesign.szFont,g_Config.SelectedItemDesign.iFontSize,g_Config.SelectedItemDesign.iFontStyle);

			//壁紙設定
			if(lstrlen(g_Config.Background.szImagePath)){
#if BACKGROUND_NO_GDIPLUS
				g_hBitmapOriginal=ReadPicture(g_Config.Background.szImagePath);
				g_bBackground=g_hBitmapOriginal!=NULL;
#else
				FreeBitmap(g_pBitmap);
				FreeBitmap(g_pBitmapBack);
				FreeBitmap(g_pBitmapOriginal);
				g_pBitmapOriginal=Gdiplus::Bitmap::FromFile(g_Config.Background.szImagePath);
				g_bBackground=g_pBitmapOriginal!=NULL;
#endif
			}

			//ホットキーを登録する
			if(g_Config.ShowWindow.wHotKey){
				RegistHotKey(hWnd,ID_HOTKEY,g_Config.ShowWindow.wHotKey);
			}

			//コマンドメニュー群読み込み
			mMainCmd.hMenu=LoadMenu(g_hInstance,MAKEINTRESOURCE(IDR_MAINMENU));
			mMainCmd.hSubMenu=GetSubMenu(mMainCmd.hMenu,0);
			mSelectCmd.hMenu=LoadMenu(g_hInstance,MAKEINTRESOURCE(IDR_SELECTMENU));
			mSelectCmd.hSubMenu=GetSubMenu(mSelectCmd.hMenu,0);
			mCopyInfoCmd.hMenu=LoadMenu(g_hInstance,MAKEINTRESOURCE(IDR_COPYINFOMENU));
			mCopyInfoCmd.hSubMenu=GetSubMenu(mCopyInfoCmd.hMenu,0);
			mIncrementalSearchCmd.hMenu=LoadMenu(g_hInstance,MAKEINTRESOURCE(IDR_INCREMENTALSEARCHMENU));
			mIncrementalSearchCmd.hSubMenu=GetSubMenu(mIncrementalSearchCmd.hMenu,0);
			mWindowCmd.hMenu=LoadMenu(g_hInstance,MAKEINTRESOURCE(IDR_WINDOWMENU));
			mWindowCmd.hSubMenu=GetSubMenu(mWindowCmd.hMenu,0);
			mProcessCmd.hMenu=LoadMenu(g_hInstance,MAKEINTRESOURCE(IDR_PROCESSMENU));
			mProcessCmd.hSubMenu=GetSubMenu(mProcessCmd.hMenu,0);
			mOpacityCmd.hMenu=LoadMenu(g_hInstance,MAKEINTRESOURCE(IDR_OPACITYMENU));
			mOpacityCmd.hSubMenu=GetSubMenu(mOpacityCmd.hMenu,0);
			mCommand.hMenu=LoadMenu(g_hInstance,MAKEINTRESOURCE(IDR_COMMANDMENU));
			mCommand.hSubMenu=GetSubMenu(mCommand.hMenu,0);

			//コマンドメニュー作成
			for(int i=0,iCount=GetMenuItemCount(mMainCmd.hSubMenu);i<iCount;++i){
				MENUITEMINFO mii={sizeof(MENUITEMINFO)};
				mii.fMask=MIIM_TYPE;
				GetMenuItemInfo(mMainCmd.hSubMenu,i,true,&mii);
				if(mii.fType==MFT_SEPARATOR){
					int iMenuIndex=++i;
					mii.fMask=MIIM_SUBMENU|MIIM_STRING;

					mii.hSubMenu=mSelectCmd.hSubMenu;
					mii.dwTypeData=_T("s&elect");
					InsertMenuItem(mMainCmd.hSubMenu,iMenuIndex++,true,&mii);

					mii.hSubMenu=mIncrementalSearchCmd.hSubMenu;
					mii.dwTypeData=_T("&incrementalsearch");
					InsertMenuItem(mMainCmd.hSubMenu,iMenuIndex++,true,&mii);

					//windowメニューにopacityメニュー挿入
					for(int ii=0,iWindowCmdCount=GetMenuItemCount(mWindowCmd.hSubMenu);ii<iWindowCmdCount;++ii){
						MENUITEMINFO miiWindowCmd={sizeof(MENUITEMINFO)};

						miiWindowCmd.fMask=MIIM_ID;
						GetMenuItemInfo(mWindowCmd.hSubMenu,ii,true,&miiWindowCmd);
						if(miiWindowCmd.wID==IDM_KEY_TOPMOST){
							int iOpacityMenuIndex=ii+2;
							miiWindowCmd.fMask=MIIM_SUBMENU|MIIM_STRING;

							miiWindowCmd.hSubMenu=mOpacityCmd.hSubMenu;
							miiWindowCmd.dwTypeData=_T("opacit&y");
							InsertMenuItem(mWindowCmd.hSubMenu,iOpacityMenuIndex,true,&miiWindowCmd);
							break;
						}
					}

					mii.hSubMenu=mWindowCmd.hSubMenu;
					mii.dwTypeData=_T("&window");
					InsertMenuItem(mMainCmd.hSubMenu,iMenuIndex++,true,&mii);

					mii.hSubMenu=mProcessCmd.hSubMenu;
					mii.dwTypeData=_T("&process");
					InsertMenuItem(mMainCmd.hSubMenu,iMenuIndex++,true,&mii);

					mii.hSubMenu=mCopyInfoCmd.hSubMenu;
					mii.dwTypeData=_T("c&opyinfo");
					InsertMenuItem(mMainCmd.hSubMenu,iMenuIndex++,true,&mii);

					mii.hSubMenu=mCommand.hSubMenu;
					mii.dwTypeData=_T("co&mmand");
					InsertMenuItem(mMainCmd.hSubMenu,iMenuIndex++,true,&mii);
					break;
				}
			}

			TCHAR szDllPath[MAX_PATH]={};
			GetSystemDirectory(szDllPath,ARRAY_SIZEOF(szDllPath));
			lstrcat(szDllPath,_T("\\dwmapi.dll"));
			g_hDwmApiDll=LoadLibrary(szDllPath);

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

			g_bDwmApi=pDwmIsCompositionEnabled&&
						pDwmGetWindowAttribute&&
						pDwmRegisterThumbnail&&
						pDwmUnregisterThumbnail&&
						pDwmUpdateThumbnailProperties&&
						pDwmQueryThumbnailSourceSize&&
						SUCCEEDED(pDwmIsCompositionEnabled(&bEnable))&&
						bEnable;

			//DwmRegisterThumbnail()でサムネイルを表示するウインドウはトップレベルでなければならない
			TCHAR szClassName[]=_T("TascherThumbnailWindowClass");
			WNDCLASS wc={};

			wc.style		=0;
			wc.lpfnWndProc	=(WNDPROC)ThumbnailWindowProc;
			wc.cbClsExtra	=0;
			wc.cbWndExtra	=0;
			wc.hInstance	=g_hInstance;
			wc.hIcon		=NULL;
			wc.hCursor		=NULL;
			wc.hbrBackground=NULL;
			wc.lpszMenuName	=NULL;
			wc.lpszClassName=szClassName;
			RegisterClass(&wc);

			g_hThumbnailWnd=CreateWindowEx(WS_EX_LAYERED|WS_EX_NOACTIVATE|WS_EX_TRANSPARENT,
										   szClassName,
										   NULL,
										   0,
										   0,0,0,0,
										   NULL,
										   (HMENU)0,
										   g_hInstance,
										   NULL);

			//デフォルトの不透明度(トグル用)
			g_byOpacity=50;
			return 0;
		}

		case WM_INITMENU:{
			ClipCursor(NULL);
			{
				//HMENUからHWNDを取得する関数はないためWM_INITMENUで取得
				HWND hFindWnd=FindWindow(_T("#32768"),NULL);
				if((HMENU)SendMessage(hFindWnd,MN_GETHMENU,0,0)!=NULL){
					g_hMenuWnd=hFindWnd;
				}
			}

			MENUITEMINFO mii={sizeof(MENUITEMINFO),MIIM_STATE};

			if((HMENU)wParam==mTray.hSubMenu){
				//「無効」メニューのチェックを設定
				mii.fState=(g_bDisable?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo(mTray.hSubMenu,IDM_MENU_DISABLE,false,&mii);
			}else if((HMENU)wParam==mMainCmd.hSubMenu||
					 (HMENU)wParam==mIncrementalSearchCmd.hSubMenu||
					 (HMENU)wParam==mWindowCmd.hSubMenu||
					 (HMENU)wParam==mProcessCmd.hSubMenu||
					 (HMENU)wParam==mOpacityCmd.hSubMenu||
					 (HMENU)wParam==mCommand.hSubMenu||
					 (HMENU)wParam==hChildProcessMenu){
				//thumbnail
				mii.fState=(g_uThumbnail?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo((HMENU)wParam,IDM_KEY_THUMBNAIL,false,&mii);
				//forwardmatch
				mii.fState=(g_iMatchMode==MATCH_FORWARD?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo((HMENU)wParam,IDM_KEY_FORWARD_MATCH,false,&mii);
				//partialmatch
				mii.fState=(g_iMatchMode==MATCH_PARTICAL?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo((HMENU)wParam,IDM_KEY_PARTIAL_MATCH,false,&mii);
				//flexmatch
				mii.fState=(g_iMatchMode==MATCH_FLEX?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo((HMENU)wParam,IDM_KEY_FLEX_MATCH,false,&mii);
				//searchscope
				mii.fState=(!g_bFirstColumnOnly?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo((HMENU)wParam,IDM_KEY_SEARCHSCOPE,false,&mii);
				//uniquewindow
				mii.fState=(g_bEnterUniqueWindow?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo((HMENU)wParam,IDM_KEY_UNIQUE_WINDOW,false,&mii);

				int iIndex=ListView_GetSelectedItem(g_hListView);

				if(iIndex<0)break;

				WINDOWPLACEMENT wndpl={sizeof(WINDOWPLACEMENT)};

				GetWindowPlacement(g_DisplayWindowInfo[iIndex]->hWnd,&wndpl);

				//minimize
				mii.fState=(wndpl.showCmd==SW_SHOWMINIMIZED?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo((HMENU)wParam,IDM_KEY_MINIMIZE,false,&mii);

				//maximize
				mii.fState=(wndpl.showCmd==SW_SHOWMAXIMIZED?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo((HMENU)wParam,IDM_KEY_MAXIMIZE,false,&mii);

				//topmost
				mii.fState=(IsWindowTopMost(g_DisplayWindowInfo[iIndex]->hWnd)?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo((HMENU)wParam,IDM_KEY_TOPMOST,false,&mii);


				//不透明度
				BYTE byOpacity=255;

				GetLayeredWindowAttributes(g_DisplayWindowInfo[iIndex]->hWnd,NULL,&byOpacity,NULL);
				byOpacity=((int)((byOpacity*100/255)+5)/10)*10;

				//opacity0%
				mii.fState=(byOpacity==0?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo((HMENU)wParam,IDM_KEY_OPACITY0,false,&mii);
				//opacity10%
				mii.fState=(byOpacity==10?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo((HMENU)wParam,IDM_KEY_OPACITY10,false,&mii);
				//opacity20%
				mii.fState=(byOpacity==20?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo((HMENU)wParam,IDM_KEY_OPACITY20,false,&mii);
				//opacity30%
				mii.fState=(byOpacity==30?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo((HMENU)wParam,IDM_KEY_OPACITY30,false,&mii);
				//opacity40%
				mii.fState=(byOpacity==40?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo((HMENU)wParam,IDM_KEY_OPACITY40,false,&mii);
				//opacity50%
				mii.fState=(byOpacity==50?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo((HMENU)wParam,IDM_KEY_OPACITY50,false,&mii);
				//opacity60%
				mii.fState=(byOpacity==60?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo((HMENU)wParam,IDM_KEY_OPACITY60,false,&mii);
				//opacity70%
				mii.fState=(byOpacity==70?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo((HMENU)wParam,IDM_KEY_OPACITY70,false,&mii);
				//opacity80%
				mii.fState=(byOpacity==80?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo((HMENU)wParam,IDM_KEY_OPACITY80,false,&mii);
				//opacity90%
				mii.fState=(byOpacity==90?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo((HMENU)wParam,IDM_KEY_OPACITY90,false,&mii);
				//opacity100%
				mii.fState=(byOpacity==100?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo((HMENU)wParam,IDM_KEY_OPACITY100,false,&mii);

				//親プロセスの優先度
				DWORD dwProcessId=0;

				GetWindowThreadProcessId((!g_DisplayWindowInfo[iIndex]->bUWPApp)?g_DisplayWindowInfo[iIndex]->hWnd:g_DisplayWindowInfo[iIndex]->hUICoreWnd,&dwProcessId);

				DWORD dwPriority=GetPriorityClass(dwProcessId);

				//idlepriority
				mii.fState=(dwPriority==IDLE_PRIORITY_CLASS?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo((HMENU)wParam,IDM_KEY_IDLE_PRIORITY,false,&mii);
				//belownormalpriority
				mii.fState=(dwPriority==BELOW_NORMAL_PRIORITY_CLASS?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo((HMENU)wParam,IDM_KEY_BELOW_NORMAL_PRIORITY,false,&mii);
				//normalpriority
				mii.fState=(dwPriority==NORMAL_PRIORITY_CLASS?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo((HMENU)wParam,IDM_KEY_NORMAL_PRIORITY,false,&mii);
				//abovenormalpriority
				mii.fState=(dwPriority==ABOVE_NORMAL_PRIORITY_CLASS?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo((HMENU)wParam,IDM_KEY_ABOVE_NORMAL_PRIORITY,false,&mii);
				//highpriority
				mii.fState=(dwPriority==HIGH_PRIORITY_CLASS?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo((HMENU)wParam,IDM_KEY_HIGH_PRIORITY,false,&mii);
				//realtimepriority
				mii.fState=(dwPriority==REALTIME_PRIORITY_CLASS?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo((HMENU)wParam,IDM_KEY_REALTIME_PRIORITY,false,&mii);

				//コマンド
				int iSubMenuCount=GetMenuItemCount(mCommand.hSubMenu);

				//全て削除
				for(int i=0;i<iSubMenuCount;i++){
					DeleteMenu(mCommand.hSubMenu,0,MF_BYPOSITION);
				}
				//有効なコマンドのみ追加
				iSubMenuCount=0;
				mii.fMask=MIIM_ID|MIIM_TYPE;
				mii.fType=MFT_STRING;
				for(int i=1;i<=MAX_COMMAND;i++){
					if(lstrlen(g_Config.Command[i].szCommandName)){
						mii.dwTypeData=g_Config.Command[i].szCommandName;
						mii.wID=IDM_KEY_COMMAND1+i-1;
						InsertMenuItem(mCommand.hSubMenu,iSubMenuCount++,true,&mii);
					}
				}

			}
			break;
		}

		case WM_MENUSELECT:{
			HMENU hMenu=(HMENU)lParam;

			if(hMenu!=NULL&&HIWORD(wParam)!=0xFFFF){
				MENUITEMINFO mii={sizeof(MENUITEMINFO),MIIM_DATA|MIIM_ID};

				GetMenuItemInfo(hMenu,LOWORD(wParam),HIWORD(wParam)&MF_POPUP,&mii);
				if(mii.dwItemData!=0){
					//プロセスIDを保存
					ulSelectedProcessId=mii.dwItemData;

				}

				if(!ulSelectedProcessId){
					break;
				}

				//子プロセスの優先度

				mii.fMask=MIIM_STATE;

				DWORD dwPriority=GetPriorityClass((DWORD)ulSelectedProcessId);

				//idlepriority
				mii.fState=(dwPriority==IDLE_PRIORITY_CLASS?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo(hMenu,IDM_KEY_IDLE_PRIORITY,false,&mii);
				//belownormalpriority
				mii.fState=(dwPriority==BELOW_NORMAL_PRIORITY_CLASS?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo(hMenu,IDM_KEY_BELOW_NORMAL_PRIORITY,false,&mii);
				//normalpriority
				mii.fState=(dwPriority==NORMAL_PRIORITY_CLASS?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo(hMenu,IDM_KEY_NORMAL_PRIORITY,false,&mii);
				//abovenormalpriority
				mii.fState=(dwPriority==ABOVE_NORMAL_PRIORITY_CLASS?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo(hMenu,IDM_KEY_ABOVE_NORMAL_PRIORITY,false,&mii);
				//highpriority
				mii.fState=(dwPriority==HIGH_PRIORITY_CLASS?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo(hMenu,IDM_KEY_HIGH_PRIORITY,false,&mii);
				//realtimepriority
				mii.fState=(dwPriority==REALTIME_PRIORITY_CLASS?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo(hMenu,IDM_KEY_REALTIME_PRIORITY,false,&mii);
			}
			return 0;
		}

		case WM_HOTKEY:
			switch(LOWORD(wParam)){
				case ID_HOTKEY:
					if(g_eWindowListStatus==WLS_CREATING||g_bDisable||g_bSettingsDialog)break;
					if(g_eWindowListStatus==WLS_DISPLAYED){
						//表示されている場合
						if(!g_Config.ListView.bHotKey){
							//非表示
							SendMessage(g_hMainWnd,WM_COMMAND,MAKEWPARAM(IDM_KEY_CANCEL,0),0);
						}else{
							//切り替え
							SendMessage(g_hMainWnd,WM_COMMAND,MAKEWPARAM(IDM_KEY_SWITCH,0),0);
						}
					}else{
						g_bHotKeyMode=true;
						//表示
						ShowListView(g_Config.ShowWindow.eHotKeyCursorCorner);
					}
					break;

				default:
					break;
			}
			break;

		case WM_DRAWITEM:
			DrawItem((LPDRAWITEMSTRUCT)lParam);
			return true;

		case WM_SIZE:
			MoveWindow(g_hListView,0,0,LOWORD(lParam),HIWORD(lParam),true);
			break;

		case WM_SETFOCUS:
			SetFocus(g_hListView);

			if(g_eWindowListStatus==WLS_CREATED){
				//ShowListView()の続き...
				if(g_eCorners!=SCC_NONE){
					RECT rcMainWnd;

					GetWindowRect(g_hMainWnd,&rcMainWnd);
					ClipCursor(&rcMainWnd);
					POINT pt;

					GetCursorPos(&pt);

					//Tascherにフォーカスを持ってくるためクリック
					//ドラッグがキャンセルされないようドラッグ中はクリックしない
					if(SetCursorCorner(g_hMainWnd,
									   g_eCorners,
									   (!g_bHotKeyMode)?g_Config.ShowWindow.iMouseCursorMarginHorizontal:g_Config.ShowWindow.iHotKeyCursorMarginHorizontal,
									   (!g_bHotKeyMode)?g_Config.ShowWindow.iMouseCursorMarginVertical:g_Config.ShowWindow.iHotKeyCursorMarginVertical)&&
					   IsCursorInWindow(g_hMainWnd)&&
					   !(GetAsyncKeyState(VK_LBUTTON)&0x8000)&&
					   !(GetAsyncKeyState(VK_RBUTTON)&0x8000)){
						INPUT data[]={
							{INPUT_MOUSE,0,0,0,MOUSEEVENTF_LEFTDOWN,0,0,},
							{INPUT_MOUSE,0,0,0,MOUSEEVENTF_LEFTUP,0,0,}
						};
						SendInput(2,data,sizeof(INPUT));
						//2度SendInput()しておかないと表示後すぐの左クリックが効かない場合がある
						SendInput(2,data,sizeof(INPUT));
					}

					SetCursorPos(pt.x,pt.y);
					ClipCursor(NULL);
				}
				//タイマーで処理を分離しなければクリックの送信がWLS_DISPLAYEDの状態で送られてしまう
				SetTimer(g_hListView,ID_TIMER_READY,0,NULL);
			}
			break;

		case WM_ACTIVATEAPP:
			if(g_eWindowListStatus!=WLS_DISPLAYED)break;
			if(!(BOOL)wParam&&(DWORD)lParam!=GetWindowThreadProcessId(hWnd,NULL)){
				HideListView();
			}
			break;

		case WM_NOTIFY:{
			if(g_eWindowListStatus!=WLS_DISPLAYED)break;
			int iIndex=ListView_GetSelectedItem(g_hListView);

			if(iIndex!=-1){
				LPNMHDR lpNMHdr=(LPNMHDR)lParam;
				switch(lpNMHdr->code){
					case LVN_ITEMCHANGED:
						//アイテムの選択が変更された場合

						ClipCursor(NULL);

						ShowToolTip(g_hListView,ID_TOOLTIP_INFO);
						ListView_RedrawItems(lpNMHdr->hwndFrom,((LPNMLISTVIEW)lParam)->iItem,((LPNMLISTVIEW)lParam)->iItem);
						if(g_Config.ListView.iTimeOut>0||
						   g_Config.ListView.bDragTimeOut&&IsCursorInWindow(g_hMainWnd)&&IsDragging()){
							//タイムアウトで確定するためタイマーを設定
							SetTimer(g_hListView,
									 MAKEWPARAM(IDM_KEY_SWITCH,ListView_GetSelectedItem(g_hListView)),
									 (g_Config.ListView.iTimeOut>0)?g_Config.ListView.iTimeOut:DEFAULT_SELECT_TIMEOUT,
									 NULL);
						}

						if(!g_bHotKeyMode&&g_Config.ShowWindow.bMouseMoveCursorSelectedWindow||
						   g_bHotKeyMode&&g_Config.ShowWindow.bHotKeyMoveCursorSelectedWindow){
							//選択項目にマウスを移動
							POINT ptOrig;

							GetCursorPos(&ptOrig);

							if(WindowFromPoint(ptOrig)==g_hListView){
								RECT rc;

								ListView_GetSubItemRect(g_hListView,ListView_GetSelectedItem(g_hListView),
														g_ListViewColumn_Table[0].bVisible&&g_ListViewColumn_Table[1].bVisible,
														LVIR_BOUNDS,&rc);

								POINT pt={rc.left,rc.top};
								ClientToScreen(g_hListView,&pt);

								SetCursorPos(ptOrig.x,pt.y+(rc.bottom-rc.top)/2);
							}
						}

						if(g_eWindowListStatus==WLS_DISPLAYED&&
						   ((LPNMLISTVIEW)lParam)->uNewState&LVIS_SELECTED){
							ShowThumbnail();
						}

						break;

					default:
						break;
				}//switch(lpNMHdr->code)
				break;
			}
			break;
		}

		case WM_TIMER:{
			BYTE byMod=0;

			switch(wParam){
				case ID_TIMER_MOUSE_MOVE_LEFTTOP:
				case ID_TIMER_MOUSE_MOVE_TOP:
				case ID_TIMER_MOUSE_MOVE_RIGHTTOP:
				case ID_TIMER_MOUSE_MOVE_LEFT:
				case ID_TIMER_MOUSE_MOVE_RIGHT:
				case ID_TIMER_MOUSE_MOVE_LEFTBOTTOM:
				case ID_TIMER_MOUSE_MOVE_BOTTOM:
				case ID_TIMER_MOUSE_MOVE_RIGHTBOTTOM:{
					KillTimer(hWnd,wParam);
					SCC_CORNERS eCorners=GetCursorCorner();

					if(g_eWindowListStatus==WLS_HIDE&&!g_bDisable&&!g_bSettingsDialog){
						for(int i=0;i<ARRAY_SIZEOF(SccTimerTable);i++){
							if(eCorners==SccTimerTable[i].eCorners&&
							   SccTimerTable[i].uId==wParam){
								ShowListView(g_Config.ShowWindow.ccCursorCorners[eCorners].eDestCorner);
								break;
							}
						}
					}
					break;
				}

				case WM_CHAR:
					KillTimer(hWnd,wParam);
					IncrementalSearch(g_szSearchString,g_iMatchMode,g_bFirstColumnOnly);
					return DefWindowProc(hWnd,uMsg,wParam,lParam);

				case MAKEWPARAM(IDM_KEY_BS_STRING,1):
					//インクリメンタルサーチの文字列を1つ削る
					KillTimer(hWnd,wParam);
					//表示を元に戻す
					for(int i=0;i<g_iWindowCount;++i){
						g_DisplayWindowInfo[i]=&g_WindowInfo[i];
					}
					g_iDisplayWindowCount=g_iWindowCount;

					IncrementalSearch(g_szSearchString,g_iMatchMode,g_bFirstColumnOnly);

					if(hSelectedItemWnd){
						for(int i=0;i<g_iDisplayWindowCount;++i){
							if(g_DisplayWindowInfo[i]->hWnd==hSelectedItemWnd){
								ListView_SelectItem(g_hListView,i);
								break;
							}
						}
					}

					if(g_szSearchString[0]=='\0'){
						//ツールチップ非表示
						ShowToolTip(g_hListView);
					}
					return DefWindowProc(hWnd,uMsg,wParam,lParam);

				//キー押下確認
				case WM_MBUTTONDOWN:
					KillTimer(hWnd,wParam);
					//fall through
				case WM_MBUTTONDBLCLK:
					byMod|=MODF_MBUTTON;
					break;
				case WM_LBUTTONDOWN:
					KillTimer(hWnd,wParam);
					//fall through
				case WM_LBUTTONDBLCLK:
					byMod|=MODF_LBUTTON;
					break;
				case WM_RBUTTONDOWN:
					KillTimer(hWnd,wParam);
					//fall through
				case WM_RBUTTONDBLCLK:
					byMod|=MODF_RBUTTON;
					break;
				case WM_X1BUTTONDOWN:
					KillTimer(hWnd,wParam);
					//fall through
				case WM_X1BUTTONDBLCLK:
					byMod|=MODF_X1BUTTON;
					break;
				case WM_X2BUTTONDOWN:
					KillTimer(hWnd,wParam);
					//fall through
				case WM_X2BUTTONDBLCLK:
					byMod|=MODF_X2BUTTON;
					break;
				default:
					return DefWindowProc(hWnd,uMsg,wParam,lParam);
			}

			//保存していたShiftとCtrlの状態を適用
			byMod|=g_byModKey;
			g_byModKey=0;

			for(int i=0;i<SMOUSE_NUM;++i){
				if(!g_Config.Mouse.sMouseTable[i].uCmdId)continue;
				if(wParam==MouseType_Table[i].uMessageType&&
				   MouseType_Table[i].byMod==byMod){
					if(MouseType_Table[i].uMouseWheel==MOUSEWHEEL_UP&&
					   GET_WHEEL_DELTA_WPARAM(wParam)/WHEEL_DELTA>0){
						//マウスホイール上
						SendMessage(g_hMainWnd,WM_COMMAND,MAKEWPARAM(g_Config.Mouse.sMouseTable[i].uCmdId,0),0);
						break;
					}else if(MouseType_Table[i].uMouseWheel==MOUSEWHEEL_DOWN&&
						 GET_WHEEL_DELTA_WPARAM(wParam)/WHEEL_DELTA<0){
						//マウスホイール下
						SendMessage(g_hMainWnd,WM_COMMAND,MAKEWPARAM(g_Config.Mouse.sMouseTable[i].uCmdId,0),0);
						break;
					}else if(MouseType_Table[i].uMouseWheel==MOUSEWHEEL_NO){
						//マウスホイールなし
						SendMessage(g_hMainWnd,WM_COMMAND,MAKEWPARAM(g_Config.Mouse.sMouseTable[i].uCmdId,0),0);
						break;
					}
				}
			}
			break;
		}

		case WM_COMMAND:
			switch(LOWORD(wParam)){
				case IDM_KEY_SWITCH:{
					//切り替え
					if(g_eWindowListStatus!=WLS_DISPLAYED)break;

					int iIndex=ListView_GetSelectedItem(g_hListView);
					if(iIndex!=-1){
						ShowThumbnail(false);
						TaskSwitch(g_DisplayWindowInfo[iIndex]);
					}else{
						HideListView();
					}
					break;
				}

				case IDM_KEY_CANCEL:
					if(g_eWindowListStatus!=WLS_DISPLAYED)break;
					//切り替えをキャンセル
					HideListView();
					break;

				case IDM_KEY_PREV:{
					//前のアイテムを選択
					int iIndex=ListView_GetSelectedItem(g_hListView);

					if(g_bBackground&&
					   iIndex==ListView_GetTopIndex(g_hListView)){
						//背景画像のスクロールを防止
						EnableWindow(g_hListView,false);

						ListView_SelectPrevItem(g_hListView);

						EnableWindow(g_hListView,true);

						SetFocus(g_hMainWnd);
					}else{
						ListView_SelectPrevItem(g_hListView);
					}
					break;
				}

				case IDM_KEY_NEXT:{
					//次のアイテムを選択
					int iIndex=ListView_GetSelectedItem(g_hListView);

					if(g_bBackground&&
					   iIndex==ListView_GetTopIndex(g_hListView)+ListView_GetCountPerPage(g_hListView)-1){
						EnableWindow(g_hListView,false);

						ListView_SelectNextItem(g_hListView);

						EnableWindow(g_hListView,true);

						SetFocus(g_hMainWnd);
					}else{
						ListView_SelectNextItem(g_hListView);
					}
					break;
				}

				case IDM_KEY_FIRST:{
					//先頭のアイテムを選択
					if(g_bBackground&&
					   ListView_GetTopIndex(g_hListView)<g_iDisplayWindowCount){
						EnableWindow(g_hListView,false);

						ListView_SelectFirstItem(g_hListView);

						EnableWindow(g_hListView,true);
						SetFocus(g_hMainWnd);
					}else{
						ListView_SelectFirstItem(g_hListView);
					}
					break;
				}

				case IDM_KEY_LAST:{
					//末尾のアイテムを選択
					if(g_bBackground&&
					   ListView_GetTopIndex(g_hListView)<g_iDisplayWindowCount){
						EnableWindow(g_hListView,false);

						ListView_SelectLastItem(g_hListView);

						EnableWindow(g_hListView,true);
						SetFocus(g_hMainWnd);
					}else{
						ListView_SelectLastItem(g_hListView);
					}
					break;
				}

				case IDM_KEY_SETTINGS:
					//設定
					HideListView();
					if(g_eWindowListStatus!=WLS_DISPLAYED){
						PostMessage(g_hMainWnd,WM_COMMAND,IDM_MENU_SETTINGS,0);
					}
					break;

				case IDM_KEY_QUIT:
					//終了
					g_bDisable=true;
					PostMessage(g_hMainWnd,WM_CLOSE,0,0);
					break;

				case IDM_KEY_SWAP:
					//カラム入れ替え
					if(ListView_GetColumnCount(g_ListViewColumn_Table)!=2)return false;
					ListView_ChangeColumnOrder(g_ListViewColumn_Table,g_hListView);
					//再描画
					ListView_RedrawItems(g_hListView,0,ListView_GetItemCount(g_hListView)-1);
					//再びインクリメンタルサーチ
					if(!ReMatch()){
						//操作取り消し
						SendMessage(g_hMainWnd,WM_COMMAND,MAKEWPARAM(IDM_KEY_SWAP,0),0);
					}
					ShowThumbnail();
					break;

				case IDM_KEY_FORWARD_MATCH:{
					//先頭一致検索
					int iMatchMode=g_iMatchMode;
					g_iMatchMode=MATCH_FORWARD;
					//再びインクリメンタルサーチ
					if(!ReMatch()){
						//操作取り消し
						g_iMatchMode=iMatchMode;
						ReMatch();
					}
					break;
				}

				case IDM_KEY_PARTIAL_MATCH:{
					//部分一致検索
					int iMatchMode=g_iMatchMode;
					g_iMatchMode=MATCH_PARTICAL;
					//再びインクリメンタルサーチ
					if(!ReMatch()){
						//操作取り消し
						g_iMatchMode=iMatchMode;
						ReMatch();
					}
					break;
				}

				case IDM_KEY_FLEX_MATCH:{
					//あいまい検索
					int iMatchMode=g_iMatchMode;
					g_iMatchMode=MATCH_FLEX;
					//再びインクリメンタルサーチ
					if(!ReMatch()){
						//操作取り消し
						g_iMatchMode=iMatchMode;
						ReMatch();
					}
					break;
				}

				case IDM_KEY_SEARCHSCOPE:
					//検索項目範囲切り替え
					g_bFirstColumnOnly^=true;
					//再びインクリメンタルサーチ
					if(!ReMatch()){
						//操作取り消し
						g_bFirstColumnOnly^=true;
						ReMatch();
					}
					break;

				case IDM_KEY_UNIQUE_WINDOW:
					//候補が1つなら切り替える
					g_bEnterUniqueWindow^=true;

					if(g_bEnterUniqueWindow){
						if(g_iDisplayWindowCount==1){
						SetTimer(g_hListView,
								 MAKEWPARAM(ID_TIMER_UNIQUE_WINDOW,0),
								 (g_Config.IncrementalSearch.iUniqueWindowDelay>0)?g_Config.IncrementalSearch.iUniqueWindowDelay:0,
								 NULL);
						}
					}else{
						KillTimer(g_hListView,ID_TIMER_UNIQUE_WINDOW);
					}
					break;

				case IDM_KEY_RESET:
					//先頭選択、インクリメンタルサーチ終了
					ListView_SelectNone(g_hListView);
					//ツールチップ非表示
					ShowToolTip(g_hListView);
					//一時的な設定をリセット
					ResetTemporarySetting();
					//インクリメンタルサーチ終了
					for(int i=0;i<lstrlen(g_szSearchString);++i){
						g_szSearchString[i]='\0';
					}
					//表示を元に戻す
					for(int i=0;i<g_iWindowCount;++i){
						g_DisplayWindowInfo[i]=&g_WindowInfo[i];
					}
					g_iDisplayWindowCount=g_iWindowCount;
					//再び絞り込む
					IncrementalSearch(g_szSearchString);
					break;

				case IDM_KEY_MINIMIZE:{
					//最小化
					int iIndex=ListView_GetSelectedItem(g_hListView);
					if(iIndex<0)break;

					if(!IsHungAppWindow(g_DisplayWindowInfo[iIndex]->hWnd)&&
					   IsWindowEnabled(g_DisplayWindowInfo[iIndex]->hWnd)){
						HWND hWndBak=g_DisplayWindowInfo[iIndex]->hWnd;

						HideListView();
						PostMessage(hWndBak,WM_SYSCOMMAND,SC_MINIMIZE,0);
					}
					HideListView();
					break;
				}

				case IDM_KEY_MAXIMIZE:{
					//最大化
					int iIndex=ListView_GetSelectedItem(g_hListView);
					if(iIndex<0)break;
					if(!IsHungAppWindow(g_DisplayWindowInfo[iIndex]->hWnd)&&
					   IsWindowEnabled(g_DisplayWindowInfo[iIndex]->hWnd)){
						HWND hWndBak=g_DisplayWindowInfo[iIndex]->hWnd;

						HideListView();
						PostMessage(hWndBak,WM_SYSCOMMAND,SC_MAXIMIZE,0);
					}
					HideListView();
					break;
				}

				case IDM_KEY_RESTORE:{
					//大きさを元に戻す
					int iIndex=ListView_GetSelectedItem(g_hListView);
					if(iIndex<0)break;
					if(!IsHungAppWindow(g_DisplayWindowInfo[iIndex]->hWnd)&&
					   IsWindowEnabled(g_DisplayWindowInfo[iIndex]->hWnd)){
						HWND hWndBak=g_DisplayWindowInfo[iIndex]->hWnd;

						HideListView();
						PostMessage(hWndBak,WM_SYSCOMMAND,SC_RESTORE,0);
					}
					HideListView();
					break;
				}

				case IDM_KEY_MOVE:{
					//移動
					int iIndex=ListView_GetSelectedItem(g_hListView);
					if(iIndex<0)break;
					if(!IsHungAppWindow(g_DisplayWindowInfo[iIndex]->hWnd)&&
					   IsWindowEnabled(g_DisplayWindowInfo[iIndex]->hWnd)){
						HWND hWndBak=g_DisplayWindowInfo[iIndex]->hWnd;

						TaskSwitch(g_DisplayWindowInfo[iIndex]);
						HideListView();

						ReleaseCapture();
						PostMessage(hWndBak,WM_SYSCOMMAND,SC_MOVE,0);
						//マウスカーソルで移動できるよう、キー入力
						ExecKey(VK_RIGHT,0);
					}
					HideListView();
					break;
				}

				case IDM_KEY_SIZE:{
					//サイズ変更
					int iIndex=ListView_GetSelectedItem(g_hListView);
					if(iIndex<0)break;
					if(!IsHungAppWindow(g_DisplayWindowInfo[iIndex]->hWnd)&&
					   IsWindowEnabled(g_DisplayWindowInfo[iIndex]->hWnd)){
						HWND hWndBak=g_DisplayWindowInfo[iIndex]->hWnd;

						TaskSwitch(g_DisplayWindowInfo[iIndex]);
						HideListView();
						ReleaseCapture();
						PostMessage(hWndBak,WM_SYSCOMMAND,SC_SIZE,0);
						//マウスカーソルで移動できるよう、キー入力
						ExecKey(VK_RIGHT,0);
					}

					HideListView();
					break;
				}

				case IDM_KEY_CENTER:{
					//画面中央に移動
					int iIndex=ListView_GetSelectedItem(g_hListView);
					if(iIndex<0)break;
					if(!IsHungAppWindow(g_DisplayWindowInfo[iIndex]->hWnd)&&
					   IsWindowEnabled(g_DisplayWindowInfo[iIndex]->hWnd)){
						HWND hWndBak=g_DisplayWindowInfo[iIndex]->hWnd;
						bool bIsZoomed=IsZoomed(hWndBak)!=0;

						TaskSwitch(g_DisplayWindowInfo[iIndex]);
						HideListView();

						POINT pt;

						GetCursorPos(&pt);

						HMONITOR hMonitor=MonitorFromPoint(pt,MONITOR_DEFAULTTONEAREST);

						MoveToMonitor(hWndBak,hMonitor);
						if(!bIsZoomed){
							SetCenterWindow(hWndBak);
						}
					}

					HideListView();
					break;
				}

				case IDM_KEY_TOPMOST:{
					//最前面表示する/しない
					int iIndex=ListView_GetSelectedItem(g_hListView);
					if(iIndex<0)break;
					if(!IsHungAppWindow(g_DisplayWindowInfo[iIndex]->hWnd)&&
					   IsWindowEnabled(g_DisplayWindowInfo[iIndex]->hWnd)){
						SetWindowPos(g_DisplayWindowInfo[iIndex]->hWnd,
									 (!IsWindowTopMost(g_DisplayWindowInfo[iIndex]->hWnd))?HWND_TOPMOST:HWND_NOTOPMOST,
									 0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
					}
					HideListView();
					break;
				}

				case IDM_KEY_BACKGROUND:{
					//最背面表示する
					int iIndex=ListView_GetSelectedItem(g_hListView);
					if(iIndex<0)break;
					if(!IsHungAppWindow(g_DisplayWindowInfo[iIndex]->hWnd)&&
					   IsWindowEnabled(g_DisplayWindowInfo[iIndex]->hWnd)){
						HWND hWndBak=g_DisplayWindowInfo[iIndex]->hWnd;

						//Zオーダー先頭を対象とした場合、ShowWindow(g_hMainWnd,SW_HIDE)するとHWND_BOTTOMしたウインドウに切り替わってしまうため、先にTascherを隠す
						HideListView();
						SetWindowPos(hWndBak,
									 HWND_BOTTOM,
									 0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
						break;
					}
					HideListView();
					break;
				}

				case IDM_KEY_CLOSE:{
					//閉じる
					int iIndex=ListView_GetSelectedItem(g_hListView);
					if(iIndex<0)break;
					if(!IsHungAppWindow(g_DisplayWindowInfo[iIndex]->hWnd)&&
					   IsWindowEnabled(g_DisplayWindowInfo[iIndex]->hWnd)){
						PostMessage(g_DisplayWindowInfo[iIndex]->hWnd,WM_SYSCOMMAND,SC_CLOSE,0);
					}
					HideListView();
					break;
				}

				case IDM_KEY_TERMINATE:{
					//プロセスを強制終了
					int iIndex=ListView_GetSelectedItem(g_hListView);
					if(iIndex<0)break;

					HWND hSelectedWnd=(!g_DisplayWindowInfo[iIndex]->bUWPApp)?g_DisplayWindowInfo[iIndex]->hWnd:g_DisplayWindowInfo[iIndex]->hUICoreWnd;
					if(IsWindowEnabled(hSelectedWnd)){
						DWORD dwProcessId=0;

						if(GetWindowThreadProcessId(hSelectedWnd,&dwProcessId)!=0){
							TerminateProcess(dwProcessId);
						}
					}
					HideListView();
					break;
				}

				case IDM_KEY_TERMINATETREE:{
					//プロセスツリーを強制終了
					int iIndex=ListView_GetSelectedItem(g_hListView);
					if(iIndex<0)break;

					HWND hSelectedWnd=(!g_DisplayWindowInfo[iIndex]->bUWPApp)?g_DisplayWindowInfo[iIndex]->hWnd:g_DisplayWindowInfo[iIndex]->hUICoreWnd;
					if(IsWindowEnabled(hSelectedWnd)){
						DWORD dwProcessId=0;

						if(GetWindowThreadProcessId(hSelectedWnd,&dwProcessId)!=0){
							PROCESSENTRY32* pProcessEntry32=AllocProcessList();

							if(pProcessEntry32==NULL)break;
							TerminateProcessTree(pProcessEntry32,dwProcessId);
							FreeProcessList(pProcessEntry32);
						}
					}
					HideListView();
					break;
				}

				case IDM_KEY_BS_STRING:{
					//インクリメンタルサーチの文字列を1つ削る
					if(g_Config.ListView.iTimeOut>0||
					   IsCursorInWindow(hWnd)&&g_Config.ListView.bDragTimeOut&&IsDragging()){
						//タイムアウトで確定するためタイマーを設定
						SetTimer(g_hListView,
								 MAKEWPARAM(IDM_KEY_SWITCH,ListView_GetSelectedItem(g_hListView)),
								 (g_Config.ListView.iTimeOut>0)?g_Config.ListView.iTimeOut:DEFAULT_SELECT_TIMEOUT,
								 NULL);
					}

					int iSearchStringBack=0;

					for(int iLength=lstrlen(g_szSearchString);iSearchStringBack<iLength&&g_szSearchString[iSearchStringBack]!='\0';++iSearchStringBack);
					--iSearchStringBack;
					if(iSearchStringBack!=-1){
						g_szSearchString[iSearchStringBack]='\0';

						//選択中アイテム保存
						int iSelectedItem=ListView_GetSelectedItem(g_hListView);
						hSelectedItemWnd=(iSelectedItem!=-1)?g_DisplayWindowInfo[iSelectedItem]->hWnd:NULL;

						if(!g_bMigemoSearch||g_Config.IncrementalSearch.iMigemoDelay==0){
							//表示を元に戻す
							for(int i=0;i<g_iWindowCount;++i){
								g_DisplayWindowInfo[i]=&g_WindowInfo[i];
							}
							g_iDisplayWindowCount=g_iWindowCount;

							//再び絞り込む
							IncrementalSearch(g_szSearchString,g_iMatchMode,g_bFirstColumnOnly);
							//アイテム選択復元
							if(hSelectedItemWnd){
								for(int i=0;i<g_iDisplayWindowCount;++i){
									if(g_DisplayWindowInfo[i]->hWnd==hSelectedItemWnd){
										ListView_SelectItem(g_hListView,i);
										break;
									}
								}
							}
						}else{
							ShowToolTip(g_hListView,ID_TOOLTIP_SEARCH,g_szSearchString);
							SetTimer(g_hMainWnd,wParam,g_Config.IncrementalSearch.iMigemoDelay,NULL);
							break;
						}
					}
					if(g_szSearchString[0]=='\0'){
						//ツールチップ非表示
						ShowToolTip(g_hListView);
					}

					break;
				}

				case IDM_KEY_IDLE_PRIORITY:
				case IDM_KEY_BELOW_NORMAL_PRIORITY:
				case IDM_KEY_NORMAL_PRIORITY:
				case IDM_KEY_ABOVE_NORMAL_PRIORITY:
				case IDM_KEY_HIGH_PRIORITY:
				case IDM_KEY_REALTIME_PRIORITY:{
					//プロセスの優先度を設定
					int iIndex=ListView_GetSelectedItem(g_hListView);
					if(iIndex<0)break;

					HWND hSelectedWnd=(!g_DisplayWindowInfo[iIndex]->bUWPApp)?g_DisplayWindowInfo[iIndex]->hWnd:g_DisplayWindowInfo[iIndex]->hUICoreWnd;
					if(IsWindowEnabled(hSelectedWnd)){
						DWORD dwProcessId=0;
						DWORD dwPriorityClass=0;

						switch(LOWORD(wParam)){
							case IDM_KEY_IDLE_PRIORITY:dwPriorityClass=IDLE_PRIORITY_CLASS;break;
							case IDM_KEY_BELOW_NORMAL_PRIORITY:dwPriorityClass=BELOW_NORMAL_PRIORITY_CLASS;break;
							case IDM_KEY_NORMAL_PRIORITY:dwPriorityClass=NORMAL_PRIORITY_CLASS;break;
							case IDM_KEY_ABOVE_NORMAL_PRIORITY:dwPriorityClass=ABOVE_NORMAL_PRIORITY_CLASS;break;
							case IDM_KEY_HIGH_PRIORITY:dwPriorityClass=HIGH_PRIORITY_CLASS;break;
							case IDM_KEY_REALTIME_PRIORITY:dwPriorityClass=REALTIME_PRIORITY_CLASS;break;
						}

						if(GetWindowThreadProcessId(hSelectedWnd,&dwProcessId)!=0){
							SetPriorityClass(dwProcessId,dwPriorityClass);
						}
					}
					HideListView();
					break;
				}

				case IDM_KEY_CHILDPROCESS:{
					//子プロセスを表示
					int iIndex=ListView_GetSelectedItem(g_hListView);
					if(iIndex<0)break;

					HWND hSelectedWnd=(!g_DisplayWindowInfo[iIndex]->bUWPApp)?g_DisplayWindowInfo[iIndex]->hWnd:g_DisplayWindowInfo[iIndex]->hUICoreWnd;
					DWORD dwProcessId=0;

					if(!IsHungAppWindow(hSelectedWnd)&&
					   IsWindowEnabled(hSelectedWnd)&&
					   GetWindowThreadProcessId(hSelectedWnd,&dwProcessId)!=0){
						//初期化
						ulSelectedProcessId=0;

						//プロセスリスト取得
						PROCESSENTRY32* pProcessEntry32=AllocProcessList();

						if(pProcessEntry32==NULL)break;
						//子プロセスメニュー作成
						DestroyMenu(hChildProcessMenu);
						hChildProcessMenu=CreatePopupMenu();
						CreateChildProcessMenu(hChildProcessMenu,mProcessCmd.hSubMenu,pProcessEntry32,dwProcessId);
						FreeProcessList(pProcessEntry32);

						ShowToolTip(g_hListView,ID_TOOLTIP_INFO);

						RECT rc={};
						ListView_GetSubItemRect(g_hListView,iIndex,0,LVIR_BOUNDS,&rc);

						POINT pt={rc.left+((g_Config.ListView.iIcon==LISTICON_SMALL)?
										   16:
										   ((g_Config.ListView.iIcon<=LISTICON_NO_SMALL)?
											0:
											32)),
							rc.top};
						ClientToScreen(g_hListView,&pt);

						DWORD dwProcessId=0;
						bool bMouseOut=(GetWindowThreadProcessId(ControlFromPoint(),&dwProcessId)!=0&&
										dwProcessId!=GetCurrentProcessId());

						int iId=TrackPopupMenu(hChildProcessMenu,
											   TPM_RETURNCMD|TPM_TOPALIGN,
											   pt.x,pt.y,
											   0,
											   g_hListView,
											   NULL);

						if(iId&&ulSelectedProcessId){
							//カーソルがウインドウリスト外にあるなら内側へ移動させる
							if(!bMouseOut&&GetWindowThreadProcessId(ControlFromPoint(),&dwProcessId)!=0&&
							   dwProcessId!=GetCurrentProcessId()){
								RECT rc;

								GetWindowRect(g_hListView,&rc);
								ClipCursor(&rc);
								ClipCursor(NULL);
							}

							DWORD dwPriorityClass=0;

							switch(iId){
								case IDM_KEY_TERMINATE:{
									//プロセスを強制終了
									TerminateProcess((DWORD)ulSelectedProcessId);
									HideListView();
									break;
								}

								case IDM_KEY_TERMINATETREE:{
									//プロセスツリーを強制終了
									PROCESSENTRY32* pProcessEntry32=AllocProcessList();

									if(pProcessEntry32==NULL)break;
									TerminateProcessTree(pProcessEntry32,(DWORD)ulSelectedProcessId);
									FreeProcessList(pProcessEntry32);

									HideListView();
									break;
								}

								//プロセスの優先度を設定
								case IDM_KEY_IDLE_PRIORITY:dwPriorityClass=IDLE_PRIORITY_CLASS;break;
								case IDM_KEY_BELOW_NORMAL_PRIORITY:dwPriorityClass=BELOW_NORMAL_PRIORITY_CLASS;break;
								case IDM_KEY_NORMAL_PRIORITY:dwPriorityClass=NORMAL_PRIORITY_CLASS;break;
								case IDM_KEY_ABOVE_NORMAL_PRIORITY:dwPriorityClass=ABOVE_NORMAL_PRIORITY_CLASS;break;
								case IDM_KEY_HIGH_PRIORITY:dwPriorityClass=HIGH_PRIORITY_CLASS;break;
								case IDM_KEY_REALTIME_PRIORITY:dwPriorityClass=REALTIME_PRIORITY_CLASS;break;

								case IDM_KEY_COPY_NAME:{
									//ファイル名をコピー
									TCHAR szFileName[MAX_PATH];

									if(GetProcessFileName((DWORD)ulSelectedProcessId,szFileName,MAX_PATH)!=0){
										SetClipboardText(g_hMainWnd,szFileName,lstrlen(szFileName));
									}
									break;
								}

								case IDM_KEY_COPY_COMMANDLINE:{
									//コマンドラインをコピー
									TCHAR* pszCommandLine=AllocProcessCommandLine((DWORD)ulSelectedProcessId);
									SetClipboardText(g_hMainWnd,pszCommandLine,lstrlen(pszCommandLine));
									FreeProcessCommandLine(pszCommandLine);
									break;
								}

								case IDM_KEY_INFO:{
									//情報を表示
									TCHAR szFileName[MAX_PATH];
									TCHAR szWindowTitle[MAX_PATH];
									TCHAR szTopMost[24];
									TCHAR szShowCmd[24];
									TCHAR szPriority[64];
									TCHAR szElevated[24];

									//ファイル名
									if(!GetProcessFileName((DWORD)ulSelectedProcessId,szFileName,MAX_PATH)){
										lstrcpy(szFileName,_T(""));
									}

									//ウインドウタイトル
									//EnumWindows()使う程重要ではないので適当に...
									HWND hWnd=GetTopWindow(NULL);

									do{
										if(!IsHungAppWindow(hWnd)&&
											GetWindowLongPtr(hWnd,GWLP_HWNDPARENT)==NULL&&
											IsWindowAvailable(hWnd)&&
											IsWindowEnabled(hWnd)){
											DWORD dwPId=0;

											GetWindowThreadProcessId(hWnd,&dwPId);
											if(ulSelectedProcessId==dwPId)break;
										}
									}while((hWnd=GetNextWindow(hWnd,GW_HWNDNEXT))!=NULL);
									if(hWnd!=NULL&&
									   !IsHungAppWindow(hWnd)){
										InternalGetWindowText(hWnd,szWindowTitle,MAX_PATH);
									}else{
										lstrcpy(szWindowTitle, _T(""));
									}

									//コマンドライン
									TCHAR* pszCommandLine=AllocProcessCommandLine((DWORD)ulSelectedProcessId);

									WINDOWPLACEMENT wndpl={sizeof(WINDOWPLACEMENT)};

									GetWindowPlacement(hWnd,&wndpl);

									//最前面表示か否か
									if(IsWindowTopMost(hWnd)){
										lstrcpy(szTopMost,_T("WS_EX_TOPMOST\n"));
									}else{
										lstrcpy(szTopMost,_T(""));
									}

									//ウインドウの表示状態
									switch(wndpl.showCmd){
										case SW_HIDE:lstrcpy(szShowCmd,_T("SW_HIDE"));break;
										case SW_MINIMIZE:lstrcpy(szShowCmd,_T("SW_MINIMIZE"));break;
										case SW_RESTORE:lstrcpy(szShowCmd,_T("SW_RESTORE"));break;
										case SW_SHOW:lstrcpy(szShowCmd,_T("SW_SHOW"));break;
										case SW_SHOWMAXIMIZED:lstrcpy(szShowCmd,_T("SW_SHOWMAXIMIZED"));break;
										case SW_SHOWMINIMIZED:lstrcpy(szShowCmd,_T("SW_SHOWMINIMIZED"));break;
										case SW_SHOWMINNOACTIVE:lstrcpy(szShowCmd,_T("SW_SHOWMINNOACTIVE"));break;
										case SW_SHOWNA:lstrcpy(szShowCmd,_T("SW_SHOWNA"));break;
										case SW_SHOWNOACTIVATE:lstrcpy(szShowCmd,_T("SW_SHOWNOACTIVATE"));break;
										case SW_SHOWNORMAL:lstrcpy(szShowCmd,_T("SW_SHOWNORMAL"));break;
										default:lstrcpy(szShowCmd,_T(""));break;
									}

									//優先度
									switch(GetPriorityClass((DWORD)ulSelectedProcessId)){
										case IDLE_PRIORITY_CLASS:lstrcpy(szPriority,_T("IDLE_PRIORITY_CLASS"));break;
										case BELOW_NORMAL_PRIORITY_CLASS:lstrcpy(szPriority,_T("BELOW_NORMAL_PRIORITY_CLASS"));break;
										case NORMAL_PRIORITY_CLASS:lstrcpy(szPriority,_T("NORMAL_PRIORITY_CLASS"));break;
										case ABOVE_NORMAL_PRIORITY_CLASS:lstrcpy(szPriority,_T("ABOVE_NORMAL_PRIORITY_CLASS"));break;
										case HIGH_PRIORITY_CLASS:lstrcpy(szPriority,_T("HIGH_PRIORITY_CLASS"));break;
										case REALTIME_PRIORITY_CLASS:lstrcpy(szPriority,_T("REALTIME_PRIORITY_CLASS"));break;
										default:lstrcpy(szPriority,_T(""));break;
									}

									//管理者特権
									wsprintf(szElevated,_T("Elevated=%s"),(IsAdministratorProcess((DWORD)ulSelectedProcessId))?_T("Yes"):_T("No"));

									TCHAR* pszText=(TCHAR*)HeapAlloc(GetProcessHeap(),
														 HEAP_ZERO_MEMORY,
														 (lstrlen(szFileName)+
														  lstrlen(szWindowTitle)+
														  lstrlen(pszCommandLine)+
														  lstrlen(szTopMost)+
														  lstrlen(szShowCmd)+
														  lstrlen(szPriority)+
														  lstrlen(szElevated)+
														  12)*sizeof(TCHAR));
									wsprintf(pszText,
											 //szFileName
											 _T("%s\n")
											 //szWindowTitle
											 _T("%s\n")
											 //pszCommandLine
											 _T("%s\n")
											 //szTopMost
											 _T("%s")
											 //szShowCmd
											 _T("%s\n")
											 //szPriority
											 _T("%s\n")
											 //szElevated
											 _T("%s"),
											 szFileName,
											 szWindowTitle,
											 pszCommandLine,
											 szTopMost,
											 szShowCmd,
											 szPriority,
											 szElevated);

									FreeProcessCommandLine(pszCommandLine);

									TOOLINFO ti={sizeof(TOOLINFO)};

									ti.lpszText=pszText;
									ti.hwnd=g_hListView;
									ti.uId=ID_TOOLTIP_INFO;

									RECT rc;

									ListView_GetSubItemRect(g_hListView,iIndex,0,LVIR_BOUNDS,&rc);
									rc.left+=((g_Config.ListView.iIcon==LISTICON_SMALL)?
											  16:
											  ((g_Config.ListView.iIcon<=LISTICON_NO_SMALL)?
											   0:
											   32));

									POINT pt={rc.left,rc.top};

									ClientToScreen(g_hListView,&pt);
									//ツールチップ文字列更新
									SendMessage(g_hToolTipInfo,TTM_UPDATETIPTEXT,0,(LPARAM)&ti);
									//ツールチップ表示位置
									SendMessage(g_hToolTipInfo,TTM_TRACKPOSITION,0,(LPARAM)MAKELONG(pt.x,pt.y));
									//改行できるように幅(ウインドウリストと同一)を設定
									SendMessage(g_hToolTipInfo,TTM_SETMAXTIPWIDTH,(WPARAM)0,(LPARAM)(rc.right-rc.left));
									//ツールチップを表示
									SendMessage(g_hToolTipInfo,TTM_TRACKACTIVATE,true,(LPARAM)&ti);

									HeapFree(GetProcessHeap(),0,pszText);
									pszText=NULL;
									break;
								}
							}
							//プロセスの優先度を設定
							if(dwPriorityClass){
								SetPriorityClass((DWORD)ulSelectedProcessId,dwPriorityClass);
								HideListView();
							}
						}
					}
					break;
				}

				case IDM_KEY_COPY_NAME:{
					//ファイル名をコピー
					int iIndex=ListView_GetSelectedItem(g_hListView);
					if(iIndex<0)break;

					DWORD dwProcessId=0;
					HWND hSelectedWnd=(!g_DisplayWindowInfo[iIndex]->bUWPApp)?g_DisplayWindowInfo[iIndex]->hWnd:g_DisplayWindowInfo[iIndex]->hUICoreWnd;

					if(!IsHungAppWindow(hSelectedWnd)&&
						IsWindowEnabled(hSelectedWnd)&&
						GetWindowThreadProcessId(hSelectedWnd,&dwProcessId)!=0){
						TCHAR szFileName[MAX_PATH];

						if(GetProcessFileName(dwProcessId,szFileName,MAX_PATH)!=0){
							SetClipboardText(g_hMainWnd,szFileName,lstrlen(szFileName));
						}
					}
					break;
				}

				case IDM_KEY_COPY_TITLE:{
					//ウインドウタイトルをコピー
					int iIndex=ListView_GetSelectedItem(g_hListView);
					if(iIndex<0)break;

					DWORD dwProcessId=0;

					if(!IsHungAppWindow(g_DisplayWindowInfo[iIndex]->hWnd)&&
					   IsWindowEnabled(g_DisplayWindowInfo[iIndex]->hWnd)&&
					   GetWindowThreadProcessId(g_DisplayWindowInfo[iIndex]->hWnd,&dwProcessId)!=0){
						SetClipboardText(g_hMainWnd,g_DisplayWindowInfo[iIndex]->szWindowTitle,lstrlen(g_DisplayWindowInfo[iIndex]->szWindowTitle));
					}
					break;
				}

				case IDM_KEY_COPY_COMMANDLINE:{
					//コマンドラインをコピー
					int iIndex=ListView_GetSelectedItem(g_hListView);
					if(iIndex<0)break;

					DWORD dwProcessId=0;
					HWND hSelectedWnd=(!g_DisplayWindowInfo[iIndex]->bUWPApp)?g_DisplayWindowInfo[iIndex]->hWnd:g_DisplayWindowInfo[iIndex]->hUICoreWnd;

					if(!IsHungAppWindow(hSelectedWnd)&&
						IsWindowEnabled(hSelectedWnd)&&
						GetWindowThreadProcessId(hSelectedWnd,&dwProcessId)!=0){
						TCHAR* pszCommandLine=AllocProcessCommandLine(dwProcessId);
						SetClipboardText(g_hMainWnd,pszCommandLine,lstrlen(pszCommandLine));
						FreeProcessCommandLine(pszCommandLine);
					}
					break;
				}

				case IDM_KEY_INFO:{
					//情報を表示
					int iIndex=ListView_GetSelectedItem(g_hListView);
					if(iIndex<0)break;
					DWORD dwProcessId=0;
					HWND hSelectedWnd=(!g_DisplayWindowInfo[iIndex]->bUWPApp)?g_DisplayWindowInfo[iIndex]->hWnd:g_DisplayWindowInfo[iIndex]->hUICoreWnd;

					if(!IsHungAppWindow(hSelectedWnd)&&
						IsWindowEnabled(hSelectedWnd)&&
						GetWindowThreadProcessId(hSelectedWnd,&dwProcessId)!=0){
						TCHAR szFileName[MAX_PATH];
						TCHAR* pszCommandLine=AllocProcessCommandLine(dwProcessId);
						TCHAR szTopMost[24];
						TCHAR szShowCmd[24];
						TCHAR szPriority[64];
						TCHAR szElevated[24];

						//ファイル名
						if(!GetProcessFileName(dwProcessId,szFileName,MAX_PATH)){
							lstrcpy(szFileName,_T(""));
						}

						//最前面表示か否か
						if(IsWindowTopMost(hSelectedWnd)){
							lstrcpy(szTopMost,_T("WS_EX_TOPMOST\n"));
						}else{
							lstrcpy(szTopMost,_T(""));
						}

						WINDOWPLACEMENT wndpl={sizeof(WINDOWPLACEMENT)};

						GetWindowPlacement(hSelectedWnd,&wndpl);

						//ウインドウの表示状態
						switch(wndpl.showCmd){
							case SW_HIDE:lstrcpy(szShowCmd,_T("SW_HIDE"));break;
							case SW_MINIMIZE:lstrcpy(szShowCmd,_T("SW_MINIMIZE"));break;
							case SW_RESTORE:lstrcpy(szShowCmd,_T("SW_RESTORE"));break;
							case SW_SHOW:lstrcpy(szShowCmd,_T("SW_SHOW"));break;
							case SW_SHOWMAXIMIZED:lstrcpy(szShowCmd,_T("SW_SHOWMAXIMIZED"));break;
							case SW_SHOWMINIMIZED:lstrcpy(szShowCmd,_T("SW_SHOWMINIMIZED"));break;
							case SW_SHOWMINNOACTIVE:lstrcpy(szShowCmd,_T("SW_SHOWMINNOACTIVE"));break;
							case SW_SHOWNA:lstrcpy(szShowCmd,_T("SW_SHOWNA"));break;
							case SW_SHOWNOACTIVATE:lstrcpy(szShowCmd,_T("SW_SHOWNOACTIVATE"));break;
							case SW_SHOWNORMAL:lstrcpy(szShowCmd,_T("SW_SHOWNORMAL"));break;
							default:lstrcpy(szShowCmd,_T(""));break;
						}

						//優先度
						switch(GetPriorityClass(dwProcessId)){
							case IDLE_PRIORITY_CLASS:lstrcpy(szPriority,_T("IDLE_PRIORITY_CLASS"));break;
							case BELOW_NORMAL_PRIORITY_CLASS:lstrcpy(szPriority,_T("BELOW_NORMAL_PRIORITY_CLASS"));break;
							case NORMAL_PRIORITY_CLASS:lstrcpy(szPriority,_T("NORMAL_PRIORITY_CLASS"));break;
							case ABOVE_NORMAL_PRIORITY_CLASS:lstrcpy(szPriority,_T("ABOVE_NORMAL_PRIORITY_CLASS"));break;
							case HIGH_PRIORITY_CLASS:lstrcpy(szPriority,_T("HIGH_PRIORITY_CLASS"));break;
							case REALTIME_PRIORITY_CLASS:lstrcpy(szPriority,_T("REALTIME_PRIORITY_CLASS"));break;
							default:lstrcpy(szPriority,_T(""));break;
						}

						//管理者特権
						wsprintf(szElevated,_T("Elevated=%s"),(IsAdministratorProcess(dwProcessId))?_T("Yes"):_T("No"));

						TCHAR* pszText=(TCHAR*)HeapAlloc(GetProcessHeap(),
														 HEAP_ZERO_MEMORY,
														 (lstrlen(szFileName)+
														  lstrlen(g_DisplayWindowInfo[iIndex]->szWindowTitle)+
														  lstrlen(pszCommandLine)+
														  lstrlen(szTopMost)+
														  lstrlen(szShowCmd)+
														  lstrlen(szPriority)+
														  lstrlen(szElevated)+
														  12)*sizeof(TCHAR));
						wsprintf(pszText,
								 //szFileName
								 _T("%s\n")
								 //g_DisplayWindowInfo[iIndex]->szWindowTitle,
								 _T("%s\n")
								 //pszCommandLine
								 _T("%s\n")
								 //szTopMost
								 _T("%s")
								 //szShowCmd
								 _T("%s\n")
								 //szPriority
								 _T("%s\n")
								 //szElevated
								 _T("%s"),
								 szFileName,
								 g_DisplayWindowInfo[iIndex]->szWindowTitle,
								 pszCommandLine,
								 szTopMost,
								 szShowCmd,
								 szPriority,
								 szElevated);

						FreeProcessCommandLine(pszCommandLine);

						TOOLINFO ti={sizeof(TOOLINFO)};

						ti.lpszText=pszText;
						ti.hwnd=g_hListView;
						ti.uId=ID_TOOLTIP_INFO;

						RECT rc;

						ListView_GetSubItemRect(g_hListView,iIndex,0,LVIR_BOUNDS,&rc);
						rc.left+=((g_Config.ListView.iIcon==LISTICON_SMALL)?
								   16:
								   ((g_Config.ListView.iIcon<=LISTICON_NO_SMALL)?
									0:
									32));

						POINT pt={rc.left,rc.top};

						ClientToScreen(g_hListView,&pt);
						//ツールチップ文字列更新
						SendMessage(g_hToolTipInfo,TTM_UPDATETIPTEXT,0,(LPARAM)&ti);
						//ツールチップ表示位置
						SendMessage(g_hToolTipInfo,TTM_TRACKPOSITION,0,(LPARAM)MAKELONG(pt.x,pt.y));
						//改行できるように幅(ウインドウリストと同一)を設定
						SendMessage(g_hToolTipInfo,TTM_SETMAXTIPWIDTH,(WPARAM)0,(LPARAM)(rc.right-rc.left));
						//ツールチップを表示
						SendMessage(g_hToolTipInfo,TTM_TRACKACTIVATE,true,(LPARAM)&ti);

						HeapFree(GetProcessHeap(),0,pszText);
						pszText=NULL;
					}
					break;
				}

				case IDM_KEY_THUMBNAIL:
					//サムネイルを表示
					if(g_uThumbnail){
						g_uThumbnailBak=g_uThumbnail;
						g_uThumbnail=THUMBNAIL_NO;
					}else{
						g_uThumbnail=g_uThumbnailBak;
					}
					if(g_uThumbnail){
						int iIndex=ListView_GetSelectedItem(g_hListView);
						if(iIndex<0)break;

						ShowThumbnail();
					}else{
						ShowThumbnail(false);
					}
					break;

				case IDM_KEY_THUMBNAIL_ORIGINAL:
					g_bThumbnailOriginalSize^=true;
					if(g_uThumbnail){
						int iIndex=ListView_GetSelectedItem(g_hListView);
						if(iIndex<0)break;

						ShowThumbnail();
					}else{
						ShowThumbnail(false);
					}
					break;

				case IDM_KEY_OPACITY0:
				case IDM_KEY_OPACITY10:
				case IDM_KEY_OPACITY20:
				case IDM_KEY_OPACITY30:
				case IDM_KEY_OPACITY40:
				case IDM_KEY_OPACITY50:
				case IDM_KEY_OPACITY60:
				case IDM_KEY_OPACITY70:
				case IDM_KEY_OPACITY80:
				case IDM_KEY_OPACITY90:
				case IDM_KEY_OPACITY100:
				case IDM_KEY_OPACITYTOGGLE:{
					//不透明度
					//resource.hのIDM_KEY_OPACITY*を順番に並べておくこと...
					int iIndex=ListView_GetSelectedItem(g_hListView);
					if(iIndex<0)break;

					if(!IsHungAppWindow(g_DisplayWindowInfo[iIndex]->hWnd)&&
					   IsWindowEnabled(g_DisplayWindowInfo[iIndex]->hWnd)){
						if(LOWORD(wParam)==IDM_KEY_OPACITYTOGGLE){
							BYTE byOpacity=255;

							GetLayeredWindowAttributes(g_DisplayWindowInfo[iIndex]->hWnd,NULL,&byOpacity,NULL);
							if(byOpacity!=255){
								byOpacity=100;
							}else{
								byOpacity=g_byOpacity;
							}
							SendMessage(hWnd,WM_COMMAND,MAKEWPARAM(byOpacity/10+IDM_KEY_OPACITY0,0),0);
							break;
						}else if(LOWORD(wParam)!=IDM_KEY_OPACITY100){
							SetWindowLongPtr(g_DisplayWindowInfo[iIndex]->hWnd,GWL_EXSTYLE,GetWindowLongPtr(g_DisplayWindowInfo[iIndex]->hWnd,GWL_EXSTYLE)|WS_EX_LAYERED);
							g_byOpacity=(LOWORD(wParam)-IDM_KEY_OPACITY0)*10;
							SetLayeredWindowAttributes(g_DisplayWindowInfo[iIndex]->hWnd,0,g_byOpacity*255/100,LWA_ALPHA);
						}else{
							SetWindowLongPtr(g_DisplayWindowInfo[iIndex]->hWnd,GWL_EXSTYLE,GetWindowLongPtr(g_DisplayWindowInfo[iIndex]->hWnd,GWL_EXSTYLE)&~WS_EX_LAYERED);
						}
					}
					HideListView();
					break;
				}

				case IDM_KEY_CENTERCURSOR:{
					//切り替えてカーソルをウインドウ中央へ移動
					int iIndex=ListView_GetSelectedItem(g_hListView);
					if(iIndex<0)break;
					if(!IsHungAppWindow(g_DisplayWindowInfo[iIndex]->hWnd)&&
					   IsWindowEnabled(g_DisplayWindowInfo[iIndex]->hWnd)){
						HWND hWndBak=g_DisplayWindowInfo[iIndex]->hWnd;

						TaskSwitch(g_DisplayWindowInfo[iIndex]);
						SetCenterCursor(hWndBak);
						HideListView();
					}
					HideListView();
					break;
				}

				case IDM_KEY_MOVETOACTIVEMONITOR:{
					//アクティブなモニタへウインドウを移動
					int iIndex=ListView_GetSelectedItem(g_hListView);
					if(iIndex<0)break;
					if(!IsHungAppWindow(g_DisplayWindowInfo[iIndex]->hWnd)&&
					   IsWindowEnabled(g_DisplayWindowInfo[iIndex]->hWnd)){
						HWND hWndBak=g_DisplayWindowInfo[iIndex]->hWnd;
						HMONITOR hMonitor=MonitorFromWindow(hWnd,MONITOR_DEFAULTTONEAREST);

						TaskSwitch(g_DisplayWindowInfo[iIndex]);
						HideListView();

						MoveToMonitor(hWndBak,hMonitor);
					}
					HideListView();
					break;
				}

				case IDM_KEY_MOVETOMONITOR:{
					//指定モニタへウインドウを移動
					int iIndex=ListView_GetSelectedItem(g_hListView);
					if(iIndex<0)break;

					HWND hSelectedWnd=g_DisplayWindowInfo[iIndex]->hWnd;

					if(!IsHungAppWindow(hSelectedWnd)&&
					   IsWindowEnabled(hSelectedWnd)){
						HMONITOR hCurrentMonitor=MonitorFromWindow(hSelectedWnd,MONITOR_DEFAULTTONEAREST);
						MONITORINFO info;
						info.cbSize=sizeof(info);
						GetMonitorInfo(hCurrentMonitor,&info);

						MONITOR_INFO* pMonitorInfo=AllocMonitorsList();

						if(pMonitorInfo==NULL)break;
						DestroyMenu(hMoveToMonitorMenu);
						hMoveToMonitorMenu=CreatePopupMenu();

						MENUITEMINFO mii={sizeof(MENUITEMINFO)};
						mii.fMask=MIIM_ID|MIIM_TYPE;

						for(size_t i=0,iSize=HeapSize(GetProcessHeap(),0,pMonitorInfo)/sizeof(MONITOR_INFO);
							i<iSize;
							++i){
							if(pMonitorInfo[i].hMonitor!=hCurrentMonitor){
								if(StrStr(pMonitorInfo[i].info.szDevice,_T("DISPLAY"))){
									TCHAR szDevice[CCHDEVICENAME];

									lstrcpy(szDevice,pMonitorInfo[i].info.szDevice);
									StrReplace(szDevice,_T("DISPLAY"),_T("DISPLAY&"));
									mii.dwTypeData=szDevice;
								}else{
									mii.dwTypeData=pMonitorInfo[i].info.szDevice;
								}
								mii.wID=GetMenuItemCount(hMoveToMonitorMenu)+1;
								InsertMenuItem(hMoveToMonitorMenu,GetMenuItemCount(hMoveToMonitorMenu)+1,true,&mii);
							}
						}

						FreeMonitorsList(pMonitorInfo);

						if(!GetMenuItemCount(hMoveToMonitorMenu))break;

						ShowToolTip(g_hListView,ID_TOOLTIP_INFO);

						RECT rc={};
						ListView_GetSubItemRect(g_hListView,iIndex,0,LVIR_BOUNDS,&rc);

						POINT pt={rc.left+((g_Config.ListView.iIcon==LISTICON_SMALL)?
										   16:
										   ((g_Config.ListView.iIcon<=LISTICON_NO_SMALL)?
											0:
											32)),
							rc.top};
						ClientToScreen(g_hListView,&pt);

						DWORD dwProcessId=0;
						bool bMouseOut=(GetWindowThreadProcessId(ControlFromPoint(),&dwProcessId)!=0&&
										dwProcessId!=GetCurrentProcessId());

						int iId=TrackPopupMenu(hMoveToMonitorMenu,
											   TPM_RETURNCMD|TPM_TOPALIGN,
											   pt.x,pt.y,
											   0,
											   g_hListView,
											   NULL);

						if(iId){
							//カーソルがウインドウリスト外にあるなら内側へ移動させる
							if(!bMouseOut&&GetWindowThreadProcessId(ControlFromPoint(),&dwProcessId)!=0&&
							   dwProcessId!=GetCurrentProcessId()){
								RECT rc;

								GetWindowRect(g_hListView,&rc);
								ClipCursor(&rc);
								ClipCursor(NULL);
							}

							mii.dwTypeData=NULL;
							GetMenuItemInfo(hMoveToMonitorMenu,iId-1,true,&mii);
							if(mii.cch){
								TCHAR* pszMenuString=(TCHAR*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(++mii.cch)*sizeof(TCHAR));
								mii.dwTypeData=pszMenuString;
								GetMenuItemInfo(hMoveToMonitorMenu,iId-1,true,&mii);
								if(StrStr(pszMenuString,_T("DISPLAY&"))){
									StrReplace(pszMenuString,_T("DISPLAY&"),_T("DISPLAY"));
								}
								MoveToMonitor(hSelectedWnd,MonitorFromName(pszMenuString));
								HeapFree(GetProcessHeap(),0,pszMenuString);
								pszMenuString=NULL;
							}
						}
						break;
					}
					break;
				}

				case IDM_KEY_DESKTOP:{
					//デスクトップを表示
					HideListView();
#if 1
					ToggleDesktop();
#else

					WORD wVKey=0;
					INPUT Key;

					if(GetAsyncKeyState(VK_SHIFT)&0x8000)wVKey|=VK_SHIFT;
					if(GetAsyncKeyState(VK_CONTROL)&0x8000)wVKey|=VK_CONTROL;
					if(GetAsyncKeyState(VK_MENU)&0x8000)wVKey|=VK_MENU;

					//修飾キーを一時的にUP
					SetKeybrd(&Key,wVKey,false);
					SendInput(1,&Key,sizeof(INPUT));
					//Win+D
					ExecKey('D',VK_LWIN);
					SetKeybrd(&Key,wVKey,true);
					SendInput(1,&Key,sizeof(INPUT));
#endif
					break;
				}

				//コマンドメニュー
				case IDM_KEY_MENU:
				//selectメニュー
				case IDM_KEY_SELECTMENU:
				//copyinfoメニュー
				case IDM_KEY_COPYINFOMENU:
				//incrementalsearchメニュー
				case IDM_KEY_INCREMENTALSEARCHMENU:
				//windowメニュー
				case IDM_KEY_WINDOWMENU:
				//processメニュー
				case IDM_KEY_PROCESSMENU:
				//opacityメニュー
				case IDM_KEY_OPACITYMENU:
				case IDM_KEY_COMMANDMENU:{
					int iIndex=ListView_GetSelectedItem(g_hListView);
					if(iIndex<0)iIndex=0;

					ShowToolTip(g_hListView,ID_TOOLTIP_INFO);

					RECT rc={};
					ListView_GetSubItemRect(g_hListView,iIndex,0,LVIR_BOUNDS,&rc);

					POINT pt={rc.left+((g_Config.ListView.iIcon==LISTICON_SMALL)?
									   16:
									   ((g_Config.ListView.iIcon<=LISTICON_NO_SMALL)?
										0:
										32)),
						rc.top};
					ClientToScreen(g_hListView,&pt);

					HMENU* phMenu=NULL;

					switch(LOWORD(wParam)){
						case IDM_KEY_MENU:
							phMenu=&mMainCmd.hSubMenu;
							break;
						case IDM_KEY_SELECTMENU:
							phMenu=&mSelectCmd.hSubMenu;
							break;
						case IDM_KEY_COPYINFOMENU:
							phMenu=&mCopyInfoCmd.hSubMenu;
							break;
						case IDM_KEY_INCREMENTALSEARCHMENU:
							phMenu=&mIncrementalSearchCmd.hSubMenu;
							break;
						case IDM_KEY_WINDOWMENU:
							phMenu=&mWindowCmd.hSubMenu;
							break;
						case IDM_KEY_PROCESSMENU:
							phMenu=&mProcessCmd.hSubMenu;
							break;
						case IDM_KEY_OPACITYMENU:
							phMenu=&mOpacityCmd.hSubMenu;
							break;
						case IDM_KEY_COMMANDMENU:
							phMenu=&mCommand.hSubMenu;
							break;
					}

					DWORD dwProcessId=0;
					bool bMouseOut=(GetWindowThreadProcessId(ControlFromPoint(),&dwProcessId)!=0&&
									dwProcessId!=GetCurrentProcessId());

					int iId=TrackPopupMenu(*phMenu,
										   TPM_RETURNCMD|TPM_TOPALIGN,
										   pt.x,pt.y,
										   0,
										   g_hListView,
										   NULL);

					if(iId){
						SendMessage(g_hMainWnd,WM_COMMAND,MAKEWPARAM(iId,0),0);
						if(g_eWindowListStatus==WLS_DISPLAYED&&
						   g_Config.ListView.bMouseOut){
							//カーソルがウインドウリスト外にあるなら右上へ移動させる
							if(!bMouseOut&&GetWindowThreadProcessId(ControlFromPoint(),&dwProcessId)!=0&&
							   dwProcessId!=GetCurrentProcessId()){
								RECT rc;

								GetWindowRect(g_hListView,&rc);
								ClipCursor(&rc);
								ClipCursor(NULL);
							}
						}
					}
					break;
				}


				case IDM_MENU_SHOW:
					//表示
					if(g_eWindowListStatus==WLS_HIDE&&!g_bDisable&&!g_bSettingsDialog){
						ShowListView();
					}
					break;

				case IDM_MENU_DISABLE:
					//無効
					g_bDisable=!g_bDisable;

					if(!g_bDisable){
						//ホットキーを登録する
						RegistHotKey(hWnd,ID_HOTKEY,g_Config.ShowWindow.wHotKey);
					}else{
						//ホットキーを解除する
						UnregistHotKey(hWnd,ID_HOTKEY,g_Config.ShowWindow.wHotKey);
					}
					TaskTray(hWnd,NIM_MODIFY,!g_bDisable);
					break;

				case IDM_MENU_SETTINGS:
				case IDM_MENU_VERSION:{
					//設定
					//バージョン情報
					if(!g_bSettingsDialog){
						//ホットキーを解除する
						if(g_Config.ShowWindow.wHotKey)UnregistHotKey(hWnd,ID_HOTKEY,g_Config.ShowWindow.wHotKey);
						//WM_INPUT受け取り終了
						UnregisterRawInput();

						//Ctrl+Aフックインストール
						HHOOK hkCtrlA=SetWindowsHookEx(WH_MSGFILTER,CtrlAHookProc,NULL,GetCurrentThreadId());

						ShowSettingsDialog(hWnd,(LOWORD(wParam)==IDM_MENU_SETTINGS)?TAB_GENERAL:TAB_VERSION);

						//Ctrl+Aフックアンインストール
						UnhookWindowsHookEx(hkCtrlA);

						//ホットキーを登録する
						if(g_Config.ShowWindow.wHotKey){
							RegistHotKey(hWnd,ID_HOTKEY,g_Config.ShowWindow.wHotKey);
						}

						//一時的な設定をリセット
						ResetTemporarySetting();

						//Migemo読み込み/解放
						if(g_Config.IncrementalSearch.iMigemoMode==MIGEMO_NO){
							UnloadMigemo();
						}else{
							if(g_Config.IncrementalSearch.bNoUnloadMigemo)LoadMigemo();
						}

						if(g_Config.ListView.bDialogFrame){
							//フレーム追加
							SetWindowLongPtr(hWnd,GWL_EXSTYLE,GetWindowLongPtr(hWnd,GWL_EXSTYLE)|WS_EX_WINDOWEDGE);
							SetWindowLongPtr(hWnd,GWL_STYLE,GetWindowLongPtr(hWnd,GWL_STYLE)|WS_THICKFRAME);
						}else{
							//フレーム削除
							SetWindowLongPtr(hWnd,GWL_EXSTYLE,GetWindowLongPtr(hWnd,GWL_EXSTYLE)&~WS_EX_WINDOWEDGE);
							SetWindowLongPtr(hWnd,GWL_STYLE,GetWindowLongPtr(hWnd,GWL_STYLE)&~WS_THICKFRAME);
						}
						SetWindowPos(hWnd,NULL,0,0,0,0,(SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED));

						//アクセラレータテーブルを読み込む
						g_hAccel=UpdateAcceleratorTable(g_hAccel,g_Config.ShortcutKey.sKeyTable);

						//不透明度を設定
						if(g_Config.ListView.byOpacity!=100){
							SetWindowLongPtr(hWnd,GWL_EXSTYLE,GetWindowLongPtr(hWnd,GWL_EXSTYLE)|WS_EX_LAYERED);
							SetLayeredWindowAttributes(hWnd,0,g_Config.ListView.byOpacity*255/100,LWA_ALPHA);
						}else{
							SetWindowLongPtr(hWnd,GWL_EXSTYLE,GetWindowLongPtr(hWnd,GWL_EXSTYLE)&~WS_EX_LAYERED);
						}

						//カラムの情報を適用
						LisView_InitColumn(g_ListViewColumn_Table,&g_Config,g_hListView);

						//イメージリスト登録
						ListView_InitImageList(g_hListView,g_Config.ListView.iIcon==LISTICON_NO_SMALL||g_Config.ListView.iIcon==LISTICON_SMALL);

						//壁紙設定
						if(lstrlen(g_Config.Background.szImagePath)){
#if BACKGROUND_NO_GDIPLUS
							g_hBitmapOriginal=ReadPicture(g_Config.Background.szImagePath);
							g_bBackground=g_hBitmapOriginal!=NULL;
#else
							FreeBitmap(g_pBitmap);
							FreeBitmap(g_pBitmapBack);
							FreeBitmap(g_pBitmapOriginal);
							g_pBitmapOriginal=Gdiplus::Bitmap::FromFile(g_Config.Background.szImagePath);
							g_bBackground=g_pBitmapOriginal!=NULL;
#endif
						}else{
#if BACKGROUND_NO_GDIPLUS
							if(g_hBitmap)DeleteObject(g_hBitmap);
							if(g_hBitmapOriginal)DeleteObject(g_hBitmapOriginal);
							g_hBitmap=g_hBitmapOriginal=NULL;
#else
							FreeBitmap(g_pBitmap);
							FreeBitmap(g_pBitmapBack);
							FreeBitmap(g_pBitmapOriginal);
#endif
							g_bBackground=false;
						}
						//WM_INPUT受け取り開始
						RegisterRawInput(hWnd);
					}
					break;
				}

				case IDM_MENU_EXIT:
					//終了
					PostMessage(hWnd,WM_CLOSE,0,0);
					break;

				default:
					//コマンド
					if(IDM_KEY_COMMAND1<=LOWORD(wParam)&&
					   IDM_KEY_COMMAND50>=LOWORD(wParam)){
						int iCommandIndex=LOWORD(wParam)-IDM_KEY_COMMAND1+1;

						if(lstrlen(g_Config.Command[iCommandIndex].szCommandName)&&
						   lstrlen(g_Config.Command[iCommandIndex].szFilePath)){
							int iIndex=ListView_GetSelectedItem(g_hListView);
							TCHAR szParameters[MAX_PATH];

							lstrcpy(szParameters,g_Config.Command[iCommandIndex].szParameters);
							if(lstrlen(szParameters)&&iIndex>=0){
								DWORD dwProcessId=0;
								TCHAR szFilePath[MAX_PATH];
								HWND hSelectedWnd=(!g_DisplayWindowInfo[iIndex]->bUWPApp)?g_DisplayWindowInfo[iIndex]->hWnd:g_DisplayWindowInfo[iIndex]->hUICoreWnd;
								if(!IsHungAppWindow(hSelectedWnd)&&
								   IsWindowEnabled(hSelectedWnd)&&
								   GetWindowThreadProcessId(hSelectedWnd,&dwProcessId)!=0){

									if(!GetProcessFileName(dwProcessId,szFilePath,MAX_PATH)){
										lstrcpy(szFilePath,_T(""));
									}
								}

								if(StrStr(szParameters,_T("%F"))){
									//フルパス
									if(lstrlen(szFilePath)){
										StrReplace(szParameters,_T("%F"),szFilePath);
									}
								}
								if(StrStr(szParameters,_T("%D"))){
									//ディレクトリパス
									TCHAR szDirectory[MAX_PATH];
									TCHAR*p=NULL;
									if(p=StrRChr(szFilePath,&szFilePath[lstrlen(szFilePath)],'\\')){
										lstrcpyn(szDirectory,szFilePath,(int)(p-szFilePath+1));
										StrReplace(szParameters,_T("%D"),szDirectory);
									}
								}
								if(StrStr(szParameters,_T("%N"))){
									//ファイル名
									TCHAR szFileName[MAX_PATH];
									if(lstrlen(szFilePath)){
										if(StrChr(szFilePath,'\\')){
											lstrcpy(szFileName,(StrRChr(szFilePath,&szFilePath[lstrlen(szFilePath)],'\\'))+1);
										}else{
											lstrcpy(szFileName,szFilePath);
										}
									}
									StrReplace(szParameters,_T("%N"),szFileName);
								}
								if(StrStr(szParameters,_T("%T"))){
									//タイトル
									StrReplace(szParameters,_T("%T"),g_DisplayWindowInfo[iIndex]->szWindowTitle);
								}
								if(StrStr(szParameters,_T("%H"))){
									//ハンドル(10進数)
									TCHAR szHandle[128];
									wsprintf(szHandle,_T("%d"),g_DisplayWindowInfo[iIndex]->hWnd);
									StrReplace(szParameters,_T("%H"),szHandle);
								}
								if(StrStr(szParameters,_T("%X"))){
									//ハンドル(16進数)
									TCHAR szHandle[128];
									wsprintf(szHandle,_T("%#x"),g_DisplayWindowInfo[iIndex]->hWnd);
									StrReplace(szParameters,_T("%X"),szHandle);
								}
								if(StrStr(szParameters,_T("%C"))){
									//クラス名
									TCHAR szClassName[128];
									GetClassName(g_DisplayWindowInfo[iIndex]->hWnd,szClassName,ARRAY_SIZEOF(szClassName));
									StrReplace(szParameters,_T("%C"),szClassName);
								}
								if(StrStr(szParameters,_T("%%"))){
									//%
									StrReplace(szParameters,_T("%%"),_T("%"));
								}
							}

							if(g_Config.Command[iCommandIndex].uCommandOption==CMDOPT_SWITCH){
								//切り替え
								SendMessage(g_hMainWnd,WM_COMMAND,MAKEWPARAM(IDM_KEY_SWITCH,0),0);
							}else if(g_Config.Command[iCommandIndex].uCommandOption==CMDOPT_CANCEL){
								//キャンセル
								SendMessage(g_hMainWnd,WM_COMMAND,MAKEWPARAM(IDM_KEY_CANCEL,0),0);
							}

							if(IsAdministratorProcess(GetProcessId(GetCurrentProcess()))&&
							   lstrcmp(CommandMode_Table[g_Config.Command[iCommandIndex].uCommandMode].szVerb,
									   CommandMode_Table[0].szVerb)==0){
								//Tascherを管理者権限で起動している場合の「実行/開く」
								//管理者権限で動作しないようにする
								ShellExecuteNonElevated(g_Config.Command[iCommandIndex].szFilePath,
														lstrlen(szParameters)?szParameters:NULL,
														lstrlen(g_Config.Command[iCommandIndex].szWorkingDirectory)?g_Config.Command[iCommandIndex].szWorkingDirectory:NULL,
														g_Config.Command[iCommandIndex].uCmdShow);
							}else{
								ShellExecute(NULL,
											 CommandMode_Table[g_Config.Command[iCommandIndex].uCommandMode].szVerb,
											 g_Config.Command[iCommandIndex].szFilePath,
											 lstrlen(szParameters)?szParameters:NULL,
											 lstrlen(g_Config.Command[iCommandIndex].szWorkingDirectory)?g_Config.Command[iCommandIndex].szWorkingDirectory:NULL,
											 g_Config.Command[iCommandIndex].uCmdShow);
							}
						}
					}
					break;
			}
			break;

		case WM_CLOSETHREAD:{
			DWORD dwExitCode=0;

			GetExitCodeThread((HANDLE)wParam,&dwExitCode);
			if(dwExitCode==STILL_ACTIVE){
				if(WaitForSingleObject((HANDLE)wParam,300)!=WAIT_OBJECT_0){
					TerminateThread((HANDLE)wParam,0);
				}
				CloseHandle((HANDLE)wParam);
			}
			break;
		}

		case WM_CLOSE:{
			//タスクトレイアイコン削除
			TaskTray(hWnd,NIM_DELETE,false);
			//ホットキーを解除する
			if(g_Config.ShowWindow.wHotKey)UnregistHotKey(hWnd,ID_HOTKEY,g_Config.ShowWindow.wHotKey);

			//リストビューカラムの幅を保存
			for(UINT i=0;i<ARRAY_SIZEOF(g_ListViewColumn_Table);++i){
				*g_ListViewColumn_Table[i].piWidth=ListView_GetColumnWidth(g_hListView,g_ListViewColumn_Table[i].iListItem);
			}

			//リストビューカラムの並び順を保存
			ListView_GetColumnOrderArray(g_hListView,LISTITEM_NUM,g_Config.ListView.iColumnOrder);

			//cfgファイルに書き込む
			WritePrivateProfile(hWnd);

			//背景画像を削除
#if BACKGROUND_NO_GDIPLUS
			if(g_hBitmap)DeleteObject(g_hBitmap);
			if(g_hBitmapOriginal)DeleteObject(g_hBitmapOriginal);
#else
			FreeBitmap(g_pBitmap);
			FreeBitmap(g_pBitmapBack);
			FreeBitmap(g_pBitmapOriginal);
#endif

			//アイコン解放
			for(int i=0;i<g_iWindowCount;++i){
				DestroyIcon(g_DisplayWindowInfo[i]->hIcon);
				g_DisplayWindowInfo[i]->hIcon=NULL;
			}

			//カーソル解放
			DestroyCursor(g_hBlankCursor);

			ShowThumbnail(false);
			DestroyWindow(g_hThumbnailWnd);
			FreeLibrary(g_hDwmApiDll);

			//メニュー解放
			DestroyMenu(mTray.hMenu);
			DestroyMenu(mMainCmd.hMenu);
			DestroyMenu(mSelectCmd.hMenu);
			DestroyMenu(mCopyInfoCmd.hMenu);
			DestroyMenu(mIncrementalSearchCmd.hMenu);
			DestroyMenu(mWindowCmd.hMenu);
			DestroyMenu(mProcessCmd.hMenu);
			DestroyMenu(mOpacityCmd.hMenu);
			DestroyMenu(mCommand.hMenu);
			DestroyMenu(hChildProcessMenu);

			//サブクラス化解除
			if(DefListViewProc!=NULL){
				SetWindowLongPtr(g_hListView,GWLP_WNDPROC,(LONG_PTR)DefListViewProc);
				DefListViewProc=NULL;
			}
			DestroyWindow(g_hListView);
			CoUninitialize();
			DestroyWindow(hWnd);
			break;
		}

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		default:
			if(uMsg==WM_INPUT){
				if(!g_bDisable&&!g_bSettingsDialog){
					if(g_eWindowListStatus==WLS_HIDE){
						PRAWINPUT pRawInput;
						UINT dwSize;

						GetRawInputData((HRAWINPUT)lParam,RID_INPUT,NULL,&dwSize,sizeof(RAWINPUTHEADER));

						if(!dwSize)break;

						pRawInput=(PRAWINPUT)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,dwSize);
						if(!pRawInput)break;
						if(GetRawInputData((HRAWINPUT)lParam,RID_INPUT,pRawInput,&dwSize,sizeof(RAWINPUTHEADER))==dwSize){
							if(pRawInput->header.dwType==RIM_TYPEMOUSE){
								if(!(pRawInput->data.mouse.lLastX==0&&
								   pRawInput->data.mouse.lLastY==0)){
									if(!g_Config.ShowWindow.bMouseWheel){
										HeapFree(GetProcessHeap(),0,pRawInput);
										MsgMouseMove(0,0);
										break;
									}
								}
								if(pRawInput->data.mouse.usButtonFlags&RI_MOUSE_WHEEL){
									if(g_Config.ShowWindow.bMouseWheel){
										HeapFree(GetProcessHeap(),0,pRawInput);
										MsgMouseMove(0,0);
										break;
									}
								}
							}
						}
						HeapFree(GetProcessHeap(),0,pRawInput);
					}else if(g_eWindowListStatus==WLS_DISPLAYED){
						PRAWINPUT pRawInput;
						UINT dwSize;

						GetRawInputData((HRAWINPUT)lParam,RID_INPUT,NULL,&dwSize,sizeof(RAWINPUTHEADER));

						if(!dwSize)break;

						pRawInput=(PRAWINPUT)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,dwSize);
						if(!pRawInput)break;
						if(GetRawInputData((HRAWINPUT)lParam,RID_INPUT,pRawInput,&dwSize,sizeof(RAWINPUTHEADER))==dwSize){
							if(pRawInput->data.mouse.usButtonFlags&RI_MOUSE_WHEEL){
								PostMessage(g_hListView,MSG_MOUSEWHEEL,MAKEWPARAM(0,pRawInput->data.mouse.usButtonData),0);
							}

							if(pRawInput->header.dwType==RIM_TYPEMOUSE){
								if(!(pRawInput->data.mouse.lLastX==0&&
								   pRawInput->data.mouse.lLastY==0)){
									PostMessage(g_hListView,MSG_MOUSEMOVE,0,0);
								}
							}
						}
						HeapFree(GetProcessHeap(),0,pRawInput);
					}
				}
			}else if(uMsg==MSG_TRAY){
				//タスクトレイアイコンの操作
				if(wParam==ID_TRAYMENU){
					if(lParam==WM_LBUTTONDOWN){
						//タスクトレイアイコンが左クリックされた
						if(g_eWindowListStatus==WLS_HIDE&&!g_bDisable&&!g_bSettingsDialog){
							ShowListView();
						}
						return 0;
					}
					if(lParam==WM_RBUTTONDOWN){
						//タスクトレイアイコンが右クリックされた
						POINT pt;

						GetCursorPos(&pt);
						SetForegroundWindow(hWnd);
						TrackPopupMenu(mTray.hSubMenu,TPM_BOTTOMALIGN,pt.x,pt.y,0,hWnd,NULL);
					}
				}
			}else if(uMsg==MSG_TRAYCREATED){
				//タスクバーが再作成されたら、トレイアイコンを再度追加
				TaskTray(g_hMainWnd,NIM_ADD,true);
			}
			break;
	}
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

//メッセージループを処理するための条件を追加
bool PreTranslateAccelerator(MSG *pMsg){
	//インクリメンタルサーチ中ならスペース挿入
	if(g_eWindowListStatus==WLS_DISPLAYED){
		if((*pMsg).message==WM_KEYDOWN&&
		   (*pMsg).wParam==VK_SPACE&&
		   g_szSearchString[0]!='\0'){
			SendMessage(g_hListView,WM_CHAR,' ',0);
			return false;
		}
	}
	return true;
}

//メッセージループを処理するための条件を追加
bool PreTranslateMessage(MSG *pMsg){
	//SendInput()での左右クリック処理をリストビューに反映させない
	if(g_eWindowListStatus==WLS_CREATED&&
	   g_eCorners!=SCC_NONE){
		if((*pMsg).message==WM_LBUTTONDOWN||
		   (*pMsg).message==WM_LBUTTONUP||
		   (*pMsg).message==WM_RBUTTONDOWN||
		   (*pMsg).message==WM_RBUTTONUP){
			return false;
		}
	}
	return true;
}

//ウインドウクラスを登録
bool InitApplication(HINSTANCE hInstance,LPCTSTR lpszClassName){
	WNDCLASS wc={};

	wc.style		=0;
	wc.lpfnWndProc	=(WNDPROC)WindowProc;
	wc.cbClsExtra	=0;
	wc.cbWndExtra	=0;
	wc.hInstance	=hInstance;
	wc.hIcon		=LoadIcon(hInstance,MAKEINTRESOURCE(IDI_ICON1));
	wc.hCursor		=NULL;
	wc.hbrBackground=(HBRUSH)HOLLOW_BRUSH;
	wc.lpszMenuName	=NULL;
	wc.lpszClassName=lpszClassName;
	return RegisterClass(&wc)!=0;
}

#ifndef _DEBUG
extern "C" void WinMainCRTStartup(){
#else
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInst,LPSTR lpszCmdLine,int nCmdShow){
#endif
	TCHAR szAppName[]=TASCHER_VERSION;
	TCHAR szMutexName[]=_T("{C0E49B9C-D5CB-4fa9-91F3-152C4A1317E6}");
	TCHAR szClassName[]=_T("TascherWindowClass");
	HWND hWnd;
	MSG msg;
	HANDLE hMutex;
	bool bAlreadyExists=false;

	//タスクトレイ用メッセージ登録
	MSG_TRAY=RegisterWindowMessage(_T("MSG_TRAY"));

	{
		SECURITY_DESCRIPTOR sd;
		SECURITY_ATTRIBUTES sa;

		InitializeSecurityDescriptor(&sd,SECURITY_DESCRIPTOR_REVISION);
		SetSecurityDescriptorDacl(&sd,TRUE,0,FALSE);
		sa.nLength=sizeof(sa);
		sa.lpSecurityDescriptor=&sd;
		sa.bInheritHandle=TRUE;
		hMutex=CreateMutex(&sa,false,szMutexName);
		if(GetLastError()==ERROR_ALREADY_EXISTS){
			bAlreadyExists=true;
		}
	}


	if(!StrStr(GetCommandLine(),szMutexName)){
		if(bAlreadyExists){
			//起動済みであれば元のTascherのウインドウリストを表示
			PostMessage(HWND_BROADCAST,MSG_TRAY,ID_TRAYMENU,WM_LBUTTONDOWN);
			CloseHandle(hMutex);
			ExitProcess(0);
		}
	}

	bool(WINAPI*pfChangeWindowMessageFilter)(UINT,DWORD)=(bool(WINAPI*)(UINT,DWORD))GetProcAddress(GetModuleHandle(_T("user32.dll")),"ChangeWindowMessageFilter");
	if(pfChangeWindowMessageFilter!=NULL){
		//MSGFLT_ADD
		pfChangeWindowMessageFilter(MSG_TRAY,1);
	}

	//cfgファイルを読み込む
	ReadPrivateProfile();

	if(g_Config.bRunAsAdministrator&&
	   !IsAdministratorProcess(GetProcessId(GetCurrentProcess()))){
		if(!StrStr(GetCommandLine(),szMutexName)){
			//管理者権限で起動
			SHELLEXECUTEINFO sei={sizeof(sei)};
			TCHAR szExePath[MAX_PATH]={};

			if(GetModuleFileName(NULL,szExePath,ARRAY_SIZEOF(szExePath))){
				sei.fMask|=SEE_MASK_WAITFORINPUTIDLE|SEE_MASK_FLAG_NO_UI|SEE_MASK_NO_CONSOLE|SEE_MASK_NOASYNC|SEE_MASK_NOCLOSEPROCESS;
				sei.lpVerb=_T("runas");
				sei.lpFile=szExePath;
				sei.lpParameters=szMutexName;
				ShellExecuteEx(&sei);
				ExitProcess(0);
			}
		}else{
			if(IDNO==MessageBox(NULL,_T("管理者権限での起動が失敗しました。\nはい→通常権限で起動します。\nいいえ→Tascherを終了します。"),_T("Tascher"),MB_YESNO|MB_ICONQUESTION)){
				ExitProcess(0);
			}
		}
	}

	//インスタンスハンドル取得
	g_hInstance=GetModuleHandle(NULL);

	//タスクバー再作成用メッセージ登録
	MSG_TRAYCREATED=RegisterWindowMessage(_T("TaskbarCreated"));
	//WM_MOUSEMOVE用メッセージ登録
	MSG_MOUSEMOVE=RegisterWindowMessage(_T("MSG_MOUSEMOVE"));
	//WM_MMOUSEWHEEL用メッセージ登録
	MSG_MOUSEWHEEL=RegisterWindowMessage(_T("MSG_MOUSEWHEEL"));

	ULONG_PTR ulToken;
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;

	Gdiplus::GdiplusStartup(&ulToken,&gdiplusStartupInput,NULL);

	//Migemo,bregonig読み込み
	if(g_Config.IncrementalSearch.iMigemoMode!=MIGEMO_NO&&
	   g_Config.IncrementalSearch.bNoUnloadMigemo){
		LoadMigemo();
	}

	//ウインドウ情報用ヒープ作成
	g_hHeapWindowInfo=HeapCreate(NULL,0,0);

	//ウインドウクラスを登録
	if(InitApplication(g_hInstance,szClassName)==0)ExitProcess(0);

	hWnd=CreateWindowEx(WS_EX_TOOLWINDOW|WS_EX_TOPMOST|WS_EX_ACCEPTFILES|WS_EX_WINDOWEDGE,
						szClassName,
						szAppName,
						WS_POPUP|WS_CLIPCHILDREN|WS_THICKFRAME,
						CW_USEDEFAULT,CW_USEDEFAULT,
						CW_USEDEFAULT,CW_USEDEFAULT,
						NULL,
						NULL,
						g_hInstance,
						NULL);

	if(hWnd==NULL)ExitProcess(0);
	//メインウインドウハンドルを保存
	g_hMainWnd=hWnd;

	//WM_INPUT受け取り開始
	RegisterRawInput(hWnd);

	//アクセラレータテーブルを読み込む
	g_hAccel=UpdateAcceleratorTable(g_hAccel,g_Config.ShortcutKey.sKeyTable);

	//不透明度を設定
	if(g_Config.ListView.byOpacity!=100){
		SetWindowLongPtr(hWnd,GWL_EXSTYLE,GetWindowLongPtr(hWnd,GWL_EXSTYLE)|WS_EX_LAYERED);
		SetLayeredWindowAttributes(hWnd,0,g_Config.ListView.byOpacity*255/100,LWA_ALPHA);
	}else{
		SetWindowLongPtr(hWnd,GWL_EXSTYLE,GetWindowLongPtr(hWnd,GWL_EXSTYLE)&~WS_EX_LAYERED);
	}

	while(GetMessage(&msg,NULL,0,0)){
		if(PreTranslateAccelerator(&msg)){
			if(!TranslateAccelerator(hWnd,g_hAccel,&msg)){
				if(PreTranslateMessage(&msg)){
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
		}
	}

	//ウインドウ情報用ヒープ破棄
	HeapDestroy(g_hHeapWindowInfo);

	//Migemo/bregonig解放
	UnloadMigemo();

	//アクセラレータテーブルを破棄
	DestroyAcceleratorTable(g_hAccel);

	//WM_INPUT受け取り終了
	UnregisterRawInput();

	Gdiplus::GdiplusShutdown(ulToken);

	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
	ExitProcess((UINT)msg.wParam);
}
