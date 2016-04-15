//Tascher.cpp
//メイン

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.60
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
#include"Hook/HookMouseMove.h"
#include<gdiplus.h>
#include<psapi.h>
#include<tlhelp32.h>
#include<shellapi.h>
#include<Shlobj.h>
#include<wbemidl.h>



//[各種コントロールのID]
#define ID_TRAYMENU 201

#define IDC_LISTVIEW 100

#define ID_TIMER_HOVER 202
#define ID_TIMER_UNIQUE_WINDOW 203
#define ID_TIMER_HIDELISTVIEW 204

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


//マウスフックハンドル
HHOOK g_hkMouseHook;

//マウスフック用DLL
HMODULE g_hHookDll;
typedef bool(*INSTALLMOUSELIBHOOK_PTR)(const DWORD,HWND);
typedef bool(*UNINSTALLMOUSELIBHOOK_PTR)();
INSTALLMOUSELIBHOOK_PTR m_pInstallMouseLibHook;
UNINSTALLMOUSELIBHOOK_PTR m_pUninstallMouseLibHook;


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
}g_eWindowListStatus;

//背景画像が有効である
bool g_bBackground;

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


typedef struct{
	HMENU hMenu;
	HMENU hSubMenu;
}MENU;

//[ウインドウリスト関係]
//表示項目
typedef struct{
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

#define INCREMENTAL_SEARCH_BUFFER 32
TCHAR g_szSearchString[INCREMENTAL_SEARCH_BUFFER];

struct LISTVIEWCOLUMN_TABLE g_ListViewColumn_Table[LISTITEM_NUM];

//RegisterWindowMessage()するメッセージ
UINT MSG_MOUSEMOVE;//[マウスの移動]
UINT MSG_TRAY;//タスクトレイ用メッセージ
UINT MSG_TRAYCREATED;//タスクバー再作成用メッセージ


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
MIGEMO_OPEN migemo_open;
MIGEMO_CLOSE migemo_close;
MIGEMO_QUERY migemo_query;
MIGEMO_RELEASE migemo_release;
MIGEMO_LOAD migemo_load;
MIGEMO_IS_ENABLE migemo_is_enable;
MIGEMO_SET_OPERATOR migemo_set_operator;

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
bool InstallMouseHook();
//マウスフック(DLL)インストール
bool InstallMouseLibHook();

//マウスフックアンインストール
bool UninstallMouseHook();
//マウスフックアンインストール
bool UninstallMouseLibHook();

//Migemo/bregonig読み込み
bool LoadMigemo();
//Migemo/bregonig解放
bool UnloadMigemo();

//アクセラレータテーブルを更新
bool UpdateAcceleratorTable();

//タスクトレイ登録/解除
void TaskTray(HWND hWnd,DWORD dwMessage,bool bEnable=true);

//プロセスのファイル名を取得
bool GetProcessFileName(DWORD dwProcessId,LPTSTR pszFileName,DWORD dwFileNameLength);

//プロセスを強制終了
bool TerminateProcess(DWORD dwProcessId);

//プロセスの優先度を取得
DWORD GetPriorityClass(DWORD dwProcessId);

//プロセスの優先度を設定
bool SetPriorityClass(DWORD dwProcessId,DWORD dwPriorityClass);

//プロセスツリーを強制終了
void TerminateProcessTree(PROCESSENTRY32* pProcessEntry32,DWORD dwParentProcessId);

//プロセスリストを作成
PROCESSENTRY32* AllocProcessList();

//プロセスリストを解放
void FreeProcessList(PROCESSENTRY32* pProcessEntry32);

//子プロセスメニューを作成
void CreateChildProcessMenu(HMENU hMenu,HMENU hProcessSubMenu,PROCESSENTRY32* pProcessEntry32,DWORD dwParentProcessId);

//特定のプロセスのコマンドラインを取得
LPTSTR AllocProcessCommandLine(DWORD dwProcessId);

//AllocProcessCommandLine()で取得したコマンドラインを解放
void FreeProcessCommandLine(TCHAR* pszCommandLine);

//キー入力を登録する
void SetKeybrd(LPINPUT lpKey,WORD wVKey,bool KeyDown);

//キー入力を行う
void ExecKey(WORD wKey,WORD wVKey);

//マウスアウトを検出する
void MouseLeaveEvent(HWND hWnd);

//列挙対象のウインドウである
bool IsWindowAvailable(HWND hWnd);
//ウインドウハンドルからファイル名を取得する
bool GetFileNameFromWindowHandle(HWND hWnd,LPTSTR lpFileName,DWORD dwFileNameLength);
#if 0
bool GetFileNameFromWindowHandle2(HWND hWnd,LPTSTR lpFileName,DWORD dwFileNameLength);
#endif

//ウインドウ列挙のコールバック
//Zオーダー順であると決め打ちしています...
BOOL CALLBACK EnumWindowsProc(HWND hWnd,LPARAM lParam);

//ツールチップを表示/非表示
void ShowToolTip(HWND hWnd,UINT uToolTipId=0,TCHAR* szText=NULL);

//アイテムを追加
void AddItem(LPWINDOW_INFO lpWindowInfo);

//「デスクトップ」を追加
void AddDesktopItem();

//「キャンセル」を追加
void AddCancelItem();

//リストビューをリサイズ
void ResizeListView(HWND hWnd);

//リストビューを表示
void ShowListView(SCC_CORNERS eCorners=SCC_CENTER);

//リストビューを非表示
void HideListView();

//ウインドウの切り替え
void TaskSwitch(LPWINDOW_INFO lpWindowInfo);

//デスクトップアイコンを取得する
HICON GetDesktopIcon(bool bSmall);

//あの手この手でアイコンを取得[小さい]
HICON GetSmallIcon(HWND hWnd);

//あの手この手でアイコンを取得[大きい]
HICON GetLargeIcon(HWND hWnd);

//リストビューアイテムを描画
bool DrawItem(LPDRAWITEMSTRUCT lpDraw);

//先頭/部分一致検索
bool Match(int iMatchMode,const TCHAR* szTargetString,const TCHAR* szSearchString);

//Migemoを用い正規表現リストを作成
TCHAR** AllocRegexes(TCHAR* szQuery,int* iRegexCount);

//正規表現リストを解放
void FreeRegexes(TCHAR**ppszRegexes,int iRegexCount);

//インクリメンタルサーチを行いウインドウを絞り込む
bool IncrementalSearch(TCHAR* szSearchString,int iMatchMode=MATCH_FORWARD,bool bFirstColumnOnly=false);

//設定変更後のインクリメンタルサーチ
bool ReMatch();

//MSG_MOUSEMOVEのイベントハンドラ
void MsgMouseMove(WPARAM wParam,LPARAM lParam);

#ifdef LOWLEVEL_MOUSEHOOK
//低レベルマウスフックプロシージャ
LRESULT CALLBACK LowLevelMouseHookProc(int nCode,WPARAM wParam,LPARAM lParam);
#endif

//Ctrl+Aでエディットボックスを全選択するフックプロシージャ
LRESULT CALLBACK CtrlAHookProc(int nCode,WPARAM wParam,LPARAM lParam);

//リストビュープロシージャ
LRESULT CALLBACK ListProc(HWND hListView, UINT uMsg, WPARAM wParam, LPARAM lParam);

//ウインドウプロシージャ
LRESULT CALLBACK WindowProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

//ウインドウクラスを登録
bool InitApplication(HINSTANCE hInstance,LPCTSTR lpszClassName);

#ifndef _DEBUG
extern "C" void WinMainCRTStartup();
#else
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInst,LPSTR lpszCmdLine,int nCmdShow);
#endif



//マウスフックインストール
bool InstallMouseHook(){
#ifdef LOWLEVEL_MOUSEHOOK
	if(g_hkMouseHook!=NULL)return true;
	g_hkMouseHook=SetWindowsHookEx(WH_MOUSE_LL,(HOOKPROC)LowLevelMouseHookProc,g_hInstance,0);
	return g_hkMouseHook!=NULL;
#else
	if(g_hHookDll!=NULL)return true;
	g_hHookDll=LoadLibrary(_T("MMHook.dll"));
	if(g_hHookDll){
		m_pInstallMouseLibHook=(INSTALLMOUSELIBHOOK_PTR)GetProcAddress(g_hHookDll,"installMouseLLHook");
		m_pUninstallMouseLibHook=(UNINSTALLMOUSELIBHOOK_PTR)GetProcAddress(g_hHookDll,"uninstallMouseLLHook");
		m_pInstallMouseLibHook(GetCurrentProcessId(),g_hMainWnd);
	}
	return g_hHookDll!=NULL;
#endif
}

//マウスフック(DLL)インストール
bool InstallMouseLibHook(){
	if(g_hHookDll!=NULL)return true;

	g_hHookDll=LoadLibrary(_T("MMHook.dll"));
	if(g_hHookDll){
		m_pInstallMouseLibHook=(INSTALLMOUSELIBHOOK_PTR)GetProcAddress(g_hHookDll,"installMouseLLHook");
		m_pUninstallMouseLibHook=(UNINSTALLMOUSELIBHOOK_PTR)GetProcAddress(g_hHookDll,"uninstallMouseLLHook");
		m_pInstallMouseLibHook(GetCurrentProcessId(),g_hListView);
	}
	return g_hHookDll!=NULL;
}

//マウスフックアンインストール
bool UninstallMouseHook(){
#ifdef LOWLEVEL_MOUSEHOOK
	bool bResult=UnhookWindowsHookEx(g_hkMouseHook)!=0;
	g_hkMouseHook=NULL;
	return bResult;
#else
	return UninstallMouseLibHook();
#endif
}

//マウスフック(DLL)アンインストール
bool UninstallMouseLibHook(){
	bool bResult=false;
	if(g_hHookDll){
		if(m_pUninstallMouseLibHook)m_pUninstallMouseLibHook();
		bResult=FreeLibrary(g_hHookDll)!=0;
		g_hHookDll=NULL;
	}
	return bResult;
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

		if(migemo_open!=NULL){
			g_pmigemo=migemo_open("./dict/utf-8/" "migemo-dict");
			if(g_pmigemo==NULL||!migemo_is_enable(g_pmigemo)){
				g_pmigemo=migemo_open("./dict/" "migemo-dict");
			}
			if(g_pmigemo==NULL||!migemo_is_enable(g_pmigemo)){
				migemo_close(g_pmigemo);
				g_pmigemo=NULL;
			}
		}

		g_hBregonig=LoadLibrary(_T("bregonig.dll"));

		if(g_pmigemo!=NULL&&
		   g_hBregonig!=NULL){
			BMatch=(BMATCH)GetProcAddress(g_hBregonig,"BMatchW");
			BRegfree=(BREGFREE)GetProcAddress(g_hBregonig,"BRegfreeW");
		}

		if(g_pmigemo==NULL||BMatch==NULL){
			MessageBox(g_hMainWnd,_T("migemo/dict/bregonig読み込み失敗"),_T("Tascher"),MB_OK|MB_ICONEXCLAMATION|MB_ICONEXCLAMATION);
			UnloadMigemo();
		}
	}
	return g_pmigemo!=NULL;
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

//アクセラレータテーブルを更新
bool UpdateAcceleratorTable(){
	ACCEL aKeyTable[MAX_SHORTCUTKEY]={};

	for(int i=0;i<MAX_SHORTCUTKEY;++i){
		if(!g_Config.ShortcutKey.sKeyTable[i].wKey)break;

		ACCEL aKey={FVIRTKEY,0,0};

		if(HIBYTE(g_Config.ShortcutKey.sKeyTable[i].wKey)&HOTKEYF_ALT){
			aKey.fVirt|=FALT;
		}
		if(HIBYTE(g_Config.ShortcutKey.sKeyTable[i].wKey)&HOTKEYF_CONTROL){
			aKey.fVirt|=FCONTROL;
		}
		if(HIBYTE(g_Config.ShortcutKey.sKeyTable[i].wKey)&HOTKEYF_SHIFT){
			aKey.fVirt|=FSHIFT;
		}
		aKey.key=LOBYTE(g_Config.ShortcutKey.sKeyTable[i].wKey);
		aKey.cmd=g_Config.ShortcutKey.sKeyTable[i].uCmdId;
		aKeyTable[i]=aKey;
	}

	if(g_hAccel!=NULL)DestroyAcceleratorTable(g_hAccel);
	g_hAccel=CreateAcceleratorTable(aKeyTable,MAX_SHORTCUTKEY);
	return g_hAccel!=NULL;
}

//タスクトレイ登録/解除
void TaskTray(HWND hWnd,DWORD dwMessage,bool bEnable){
	NOTIFYICONDATA NotifyIconData={sizeof(NOTIFYICONDATA),hWnd,ID_TRAYMENU,NIF_MESSAGE|NIF_ICON|NIF_TIP,MSG_TRAY,(bEnable)?g_hTrayIcon:g_hDisableTrayIcon};

	//ツールチップ
	lstrcpy(NotifyIconData.szTip,TASCHER_VERSION);

	//ERROR_TIMEOUTの場合登録できるまで繰り返す
	while(1){
		if(Shell_NotifyIcon(dwMessage,&NotifyIconData)){
			return;
		}else{
			if(GetLastError()!=ERROR_TIMEOUT){
				if(dwMessage!=NIM_DELETE){
					MessageBox(hWnd,_T("アイコンの登録に失敗しました。"),_T("Tascher"),MB_OK|MB_ICONEXCLAMATION|MB_ICONEXCLAMATION);
					SendMessage(hWnd,WM_CLOSE,(WPARAM)0,(LPARAM)0);
					return;
				}
			}
			if(Shell_NotifyIcon(NIM_MODIFY,&NotifyIconData)){
				return;
			}else{
				Sleep(1000);
			}
		}
	}
	return;
}

//プロセスのファイル名を取得
bool GetProcessFileName(DWORD dwProcessId,LPTSTR pszFileName,DWORD dwFileNameLength){
	bool bResult=false;
	HANDLE hProcess=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,false,dwProcessId);

	static BOOL(WINAPI*pQueryFullProcessImageName)(HANDLE,DWORD,LPTSTR,PDWORD);

	if(!pQueryFullProcessImageName)pQueryFullProcessImageName=(BOOL(WINAPI*)(HANDLE,DWORD,LPTSTR,PDWORD))
		GetProcAddress(GetModuleHandle(_T("kernel32")),
#ifdef UNICODE
					   "QueryFullProcessImageNameW"
#else
					   "QueryFullProcessImageNameA"
#endif
					   );
	if(pQueryFullProcessImageName!=NULL){
		bResult=pQueryFullProcessImageName(hProcess,0,pszFileName,&dwFileNameLength)!=0;
	}
	if(!bResult){
		bResult=GetProcessImageFileName(hProcess,pszFileName,dwFileNameLength)!=0;
	}
	CloseHandle(hProcess);
	return bResult;
}

//プロセスを強制終了
bool TerminateProcess(DWORD dwProcessId){
	bool bResult=false;
	HANDLE hProcess=OpenProcess(PROCESS_TERMINATE,false,dwProcessId);

	if(hProcess!=NULL)bResult=TerminateProcess(hProcess,0)!=0;
	CloseHandle(hProcess);
	return bResult;
}

//プロセスの優先度を取得
DWORD GetPriorityClass(DWORD dwProcessId){
	DWORD dwResult=0;

	HANDLE hProcess=OpenProcess(PROCESS_QUERY_INFORMATION,false,dwProcessId);

	if(hProcess!=NULL)dwResult=GetPriorityClass(hProcess);
	CloseHandle(hProcess);
	return dwResult;
}

//プロセスの優先度を設定
bool SetPriorityClass(DWORD dwProcessId,DWORD dwPriorityClass){
	bool bResult=false;

	HANDLE hProcess=OpenProcess(PROCESS_SET_INFORMATION,false,dwProcessId);

	if(hProcess!=NULL)bResult=SetPriorityClass(hProcess,dwPriorityClass)!=0;
	CloseHandle(hProcess);
	return bResult;
}

//プロセスツリーを強制終了
void TerminateProcessTree(PROCESSENTRY32* pProcessEntry32,DWORD dwParentProcessId){
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

			//プロセス終了
			TerminateProcessTree(pProcessEntry32,pProcessEntry32[i].th32ProcessID);
			TerminateProcess(pProcessEntry32[i].th32ProcessID);
		}
	}
	TerminateProcess(dwParentProcessId);
	return;
}

//プロセスリストを作成
PROCESSENTRY32* AllocProcessList(){
	PROCESSENTRY32* pProcessEntry32=NULL;
	int iNumber=0;
	HANDLE hSnapshot=NULL;
	PROCESSENTRY32 ProcessEntry32={sizeof(PROCESSENTRY32)};

	if(INVALID_HANDLE_VALUE!=(hSnapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0))){
		if(Process32First(hSnapshot,&ProcessEntry32)){
			do{
				PROCESSENTRY32* pTmp=NULL;
				if(iNumber==0)pTmp=(PROCESSENTRY32*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(++iNumber)*sizeof(PROCESSENTRY32));
				else pTmp=(PROCESSENTRY32*)HeapReAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,pProcessEntry32,(++iNumber)*sizeof(PROCESSENTRY32));
				if(pTmp==NULL)break;
				pProcessEntry32=pTmp;
				pProcessEntry32[iNumber-1]=ProcessEntry32;
			}while(Process32Next(hSnapshot,&ProcessEntry32));
		}

		CloseHandle(hSnapshot);
	}
	return pProcessEntry32;
}

//プロセスリストを解放
void FreeProcessList(PROCESSENTRY32* pProcessEntry32){
	HeapFree(GetProcessHeap(),0,pProcessEntry32);
	pProcessEntry32=NULL;
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

//特定のプロセスのコマンドラインを取得
LPTSTR AllocProcessCommandLine(DWORD dwProcessId){
	TCHAR* pszCommandLine=NULL;

	CoInitializeEx(0,COINIT_MULTITHREADED);

	if(SUCCEEDED(CoInitializeSecurity(NULL,-1,NULL,NULL,RPC_C_AUTHN_LEVEL_DEFAULT,RPC_C_IMP_LEVEL_IMPERSONATE,NULL,EOAC_NONE,NULL))){
		IWbemLocator* pWbemLocator=NULL;

		if(SUCCEEDED(CoCreateInstance(CLSID_WbemLocator,0,CLSCTX_INPROC_SERVER,IID_IWbemLocator,(LPVOID*)&pWbemLocator))){
			IWbemServices* pWbemServices=NULL;

			if(SUCCEEDED(pWbemLocator->ConnectServer(_T("ROOT\\CIMV2"),NULL,NULL,NULL,0,NULL,NULL,&pWbemServices))){
				if(SUCCEEDED(CoSetProxyBlanket(pWbemServices,RPC_C_AUTHN_WINNT,RPC_C_AUTHZ_NONE,NULL,RPC_C_AUTHN_LEVEL_CALL,RPC_C_IMP_LEVEL_IMPERSONATE,NULL,EOAC_NONE))){
					IEnumWbemClassObject* pEnum=NULL;
					TCHAR szQuery[128];

					wsprintf(szQuery,_T("SELECT ProcessId,CommandLine FROM Win32_Process WHERE ProcessID = %d"),dwProcessId);
					if(SUCCEEDED(pWbemServices->ExecQuery(_T("WQL"),szQuery,WBEM_FLAG_FORWARD_ONLY,NULL,&pEnum))){
						IWbemClassObject* result=NULL;
						ULONG uReturned=0;

						if(SUCCEEDED(pEnum->Next(WBEM_INFINITE,1,&result,&uReturned))){
							VARIANT CommandLine;

							if(SUCCEEDED(result->Get(_T("CommandLine"),0,&CommandLine,0,0))&&
							   !(CommandLine.vt==VT_NULL)){
								int iLength=SysStringLen(CommandLine.bstrVal);
								pszCommandLine=(TCHAR*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(iLength+1)*sizeof(TCHAR));
								lstrcpy(pszCommandLine,CommandLine.bstrVal);
							}
							result->Release();
						}
						pEnum->Release();
					}
					pWbemServices->Release();
				}
			}
			pWbemLocator->Release();
		}
	}

	CoUninitialize();
	return pszCommandLine;
}

//AllocProcessCommandLine()で取得したコマンドラインを解放
void FreeProcessCommandLine(TCHAR* pszCommandLine){
	HeapFree(GetProcessHeap(),0,pszCommandLine);
	pszCommandLine=NULL;
}

//キー入力を登録する
void SetKeybrd(LPINPUT lpKey,WORD wVKey,bool KeyDown){
	lpKey->type=INPUT_KEYBOARD;
	lpKey->ki.wVk=wVKey;
	lpKey->ki.wScan=(WORD)MapVirtualKey(wVKey,0);
	lpKey->ki.dwFlags=(KeyDown?0:KEYEVENTF_KEYUP);
	lpKey->ki.dwExtraInfo=0;
	lpKey->ki.time=0;
}

//キー入力を行う
void ExecKey(WORD wKey,WORD wVKey){
	INPUT Key[6];
	LPINPUT lpKey=Key;
	SetKeybrd(lpKey++,wKey,false);
	if(wVKey)SetKeybrd(lpKey++,wVKey,false);
	if(wVKey)SetKeybrd(lpKey++,wVKey,true);
	SetKeybrd(lpKey++,wKey,true);
	SetKeybrd(lpKey++,wKey,false);
	if(wVKey)SetKeybrd(lpKey++,wVKey,false);
	SendInput((UINT)(lpKey-Key),Key,sizeof(INPUT));
	return;
}

//マウスアウトを検出する
void MouseLeaveEvent(HWND hWnd){
	TRACKMOUSEEVENT tme={sizeof(TRACKMOUSEEVENT),TME_LEAVE,hWnd};

	TrackMouseEvent(&tme);
	return;
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
			//「タスクバー相当の表示」が有効であれば、
			//非表示の親を持つ、WS_POPUP属性のウインドウを除外する
			if(hParent&&!(GetWindowLongPtr(hParent,GWL_STYLE)&WS_DISABLED)&&!(GetWindowLongPtr(hParent,GWL_STYLE)&WS_VISIBLE)&&
			   (GetWindowLongPtr(hWnd,GWL_STYLE)&WS_POPUP))return !g_Config.ListView.bTaskBarEquivalent;
			else return true;
	}
	return false;
}

//ウインドウハンドルからファイル名を取得する
bool GetFileNameFromWindowHandle(HWND hWnd,LPTSTR lpFileName,DWORD dwFileNameLength){
	bool bResult=false;
	DWORD dwProcessId=0;
//	HMODULE hModule=NULL;
	if(GetWindowThreadProcessId(hWnd,&dwProcessId)!=0){
		bResult=GetProcessFileName(dwProcessId,lpFileName,dwFileNameLength);
#if 0
		HANDLE hProcess=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,false,dwProcessId);

		if(hProcess){
			//EnumProcessModules()は不要、環境によっては遅い
//			if(EnumProcessModules(hProcess,&hModule,sizeof(hModule),&dwRet)){

				/*if(GetModuleFileNameEx(hProcess,hModule,lpFileName,dwFileNameLength)!=0){
					bResult=true;
				}else */if(GetProcessImageFileName(hProcess,lpFileName,dwFileNameLength)!=0){
					//GetProcessImageFileName()は32bit->64bitが可能
					bResult=true;
				}

//			}
		}
		CloseHandle(hProcess);
#endif
	}
	return bResult;
}

#if 0
//ウインドウハンドルからファイル名を取得する
//処理速度が遅いので採用しないことに
bool GetFileNameFromWindowHandle2(HWND hWnd,LPTSTR lpFileName,DWORD dwFileNameLength/*NULL文字を含めた文字数*/){
	bool bResult=false;
	DWORD dwProcessId=0;
	HANDLE hSnapshot=NULL;
	PROCESSENTRY32 ProcessEntry32={sizeof(PROCESSENTRY32)};

	//ウインドウハンドルからプロセスIDを取得
	if(GetWindowThreadProcessId(hWnd,&dwProcessId)!=0){
		if(INVALID_HANDLE_VALUE!=(hSnapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0))){
			bool bResultProcess32=Process32First(hSnapshot,&ProcessEntry32)!=0;

			while(bResultProcess32){
				if(dwProcessId==ProcessEntry32.th32ProcessID){
					//ファイル名をコピー
					lstrcpyn(lpFileName,ProcessEntry32.szExeFile,dwFileNameLength);
					bResult=true;
				}
				bResultProcess32=Process32Next(hSnapshot,&ProcessEntry32)!=0;
			}

			CloseHandle(hSnapshot);
		}
	}
	return bResult;
}
#endif

//ウインドウ列挙のコールバック
//Zオーダー順であると決め打ちしています...
BOOL CALLBACK EnumWindowsProc(HWND hWnd,LPARAM lParam){
	WINDOW_INFO WindowInfo={};
	TCHAR szFilePath[MAX_PATH]={};

	//ファイルパス取得
	GetFileNameFromWindowHandle(hWnd,szFilePath,MAX_PATH);

#if 0
	if(lstrcmp(szFilePath,_T(""))==0)GetFileNameFromWindowHandle2(hWnd,szFilePath,MAX_PATH);
#endif

	if(IsWindowAvailable(hWnd)){

		//ファイル名
		//ファイル名だけ取り出す
		lstrcpy(WindowInfo.szFileName,PathFindFileName(szFilePath));
		//拡張子を除く
		PathRemoveExtension(WindowInfo.szFileName);

		//ウインドウタイトル
		//取得はDrawItem()で
		lstrcpy(WindowInfo.szWindowTitle,_T(""));

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
	}

	return TRUE;
}

//ツールチップを表示/非表示
void ShowToolTip(HWND hWnd,UINT uToolTipId,TCHAR* szText){
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
	lstrcpy(wiDesktop.szFileName,_T("Desktop"));
	lstrcpy(wiDesktop.szWindowTitle,_T("デスクトップ"));
	wiDesktop.bDesktopItem=true;

	AddItem(&wiDesktop);
#endif

#if 0
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

	lstrcpy(wiCancel.szFileName,_T("Cancel"));
	lstrcpy(wiCancel.szWindowTitle,_T("キャンセル"));
	//「キャンセル」をグローバルな構造体に追加
	wiCancel.bCancelItem=true;

	AddItem(&wiCancel);
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

	if(iHeight>GetSystemMetrics(SM_CYSCREEN)){
		iHeight=GetSystemMetrics(SM_CYSCREEN)-iItemHeight*3;//ある程度余裕を持たせる
	}

	RECT rcListView;
	GetWindowRect(g_hListView,&rcListView);

	//リサイズ
	if((rcListView.bottom-rcListView.top)>iHeight){
		SetWindowPos(g_hMainWnd,
					 NULL,
					 0,0,
					 iWidth,iHeight,
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
					 iWidth,iHeight,
					 SWP_NOMOVE|SWP_NOZORDER);
	}

	if(!g_bBackground){
		ListView_RedrawItems(g_hListView,0,g_iDisplayWindowCount);
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
			0.0f,0.0f,0.0f,(Gdiplus::REAL)g_Config.Background.byAlpha/100,0.0f,
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
}

//リストビューを表示
void ShowListView(SCC_CORNERS eCorners){
	if(g_eWindowListStatus==WLS_CREATING)return;

	if(g_eWindowListStatus==WLS_DISPLAYED){
		//表示済みであれば非表示に
		HideListView();
		return;
	}

	//マウスフックアンインストール
	UninstallMouseHook();

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
		}

		//画面の中央に移動
		SetCenterWindow(g_hMainWnd);

		RECT rcMainWnd;

		GetWindowRect(g_hMainWnd,&rcMainWnd);

		//10回チャレンジ!
		for(int i=0;i<10;++i){
			if(eCorners!=SCC_NONE){
				//カーソルをウインドウ内に閉じこめる
				ClipCursor(&rcMainWnd);
				ClipCursor(NULL);
			}

			switch(eCorners){
				case SCC_LEFTTOP:
					//左上なら右下に
					SetCornerCursor(g_hMainWnd,SCC_RIGHTBOTTOM);
					break;

				case SCC_RIGHTTOP:
					//右上なら左下に
					SetCornerCursor(g_hMainWnd,SCC_LEFTBOTTOM);
					break;

				case SCC_LEFTBOTTOM:
					//左下なら右上に
					SetCornerCursor(g_hMainWnd,SCC_RIGHTTOP);
					break;

				case SCC_RIGHTBOTTOM:
					//右下なら左上に
					SetCornerCursor(g_hMainWnd,SCC_LEFTTOP);
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

		g_eWindowListStatus=WLS_CREATED;

		ShowWindow(g_hMainWnd,SW_SHOWNORMAL);

		//最前面表示
		SetWindowPos(g_hMainWnd,
					 HWND_TOPMOST,
					 0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);

		SetForegroundWindowEx(g_hMainWnd);
		//親に送らないとWM_ACTIVATEAPPが使えない
		SetFocus(g_hMainWnd);

		if(g_Config.ListView.bMouseOut){
			InstallMouseLibHook();
		}

		//インクリメンタルサーチの検索方法を保存
		g_iMatchMode=g_Config.IncrementalSearch.iMatchMode;
		//検索する項目設定を保存
		g_bFirstColumnOnly=g_Config.IncrementalSearch.bFirstColumnOnly;
		//Migemo検索リセット
		g_bMigemoSearch=false;
		//候補が1つなら切り替える設定を保存
		g_bEnterUniqueWindow=g_Config.IncrementalSearch.bEnterUniqueWindow;

		if(g_Config.ListView.bMouseOut&&
		   eCorners!=SCC_NONE&&
		   eCorners!=SCC_CENTER){

			//四隅からの表示で既にカーソルがウインドウリスト外なら確定
			POINT pt={};

			GetCursorPos(&pt);

			if(!PtInRect(&rcMainWnd,pt)){
				int iIndex=ListView_GetSelectedItem(g_hListView);

				if(iIndex!=-1){
					TaskSwitch(g_DisplayWindowInfo[iIndex]);
				}else{
					HideListView();
				}
			}
		}
	}
	return;
}

//リストビューを非表示
void HideListView(){
	static LONG lInterlockedFlag;
	if(g_eWindowListStatus==WLS_HIDE)return;

	if(InterlockedCompareExchange(&lInterlockedFlag,TRUE,FALSE)==FALSE){

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
			SetForegroundWindowEx(g_hListView);
			SetTimer(g_hListView,
					 ID_TIMER_HIDELISTVIEW,
					 0,
					 NULL);
			return;
		}


		//マウスフック(DLL)をアンインストール
		UninstallMouseLibHook();

		//ツールチップ非表示
		ShowToolTip(g_hListView);

		//ウインドウを透明に
		if(g_Config.ListView.byAlpha){
			SetLayeredWindowAttributes(g_hMainWnd,0,0,LWA_ALPHA);
		}

		ListView_SelectNone(g_hListView);

		for(int i=0;i<lstrlen(g_szSearchString);++i){
			g_szSearchString[i]='\0';
		}

		ListView_SetItemCountEx(g_hListView,0,LVSICF_NOINVALIDATEALL|LVSICF_NOSCROLL);

		HeapFree(g_hHeapWindowInfo,0,g_DisplayWindowInfo);
		HeapFree(g_hHeapWindowInfo,0,g_WindowInfo);
		g_DisplayWindowInfo=NULL;
		g_WindowInfo=NULL;

		g_iWindowCount=g_iDisplayWindowCount=0;

		if(g_Config.ListView.byAlpha){
			SetWindowPos(g_hMainWnd,
						 NULL,
						 0,0,
						 0,0,
						 SWP_NOMOVE|SWP_NOZORDER);
		}

		//最前面表示解除
		//SWP_HIDEWINDOWは使用せず、ShowWindow(SW_HIDE)すること
		SetWindowPos(g_hMainWnd,
					 HWND_NOTOPMOST,
					 0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);

		ShowWindow(g_hMainWnd,SW_HIDE);

		if(g_Config.ListView.byAlpha){
			//不透明度を元に戻す
			SetLayeredWindowAttributes(g_hMainWnd,0,g_Config.ListView.byAlpha,LWA_ALPHA);
		}
		g_eWindowListStatus=WLS_HIDE;

		//Bitmapを削除
		FreeBitmap(g_pBitmap);
		FreeBitmap(g_pBitmapBack);

		//マウスフックインストール
		InstallMouseHook();

		InterlockedExchange(&lInterlockedFlag,FALSE);
	}
	return;
}

//ウインドウの切り替え
void TaskSwitch(LPWINDOW_INFO lpWindowInfo){
	if(lpWindowInfo==NULL)return;

	if(lpWindowInfo->bDesktopItem){
		//「デスクトップ」
		SendMessage(g_hMainWnd,WM_COMMAND,MAKEWPARAM(IDM_KEY_DESKTOP,0),0);
	}else if(lpWindowInfo->bCancelItem){
		//「キャンセル」
		SendMessage(g_hMainWnd,WM_COMMAND,MAKEWPARAM(IDM_KEY_CANCEL,0),0);
	}else{
		//通常のウインドウ
		if(!IsHungAppWindow(lpWindowInfo->hWnd)){
			if(IsIconic(lpWindowInfo->hWnd)){
				//最小化されている場合まず元に戻す
				ShowWindow(lpWindowInfo->hWnd,SW_RESTORE);
			}
			SetForegroundWindowEx(lpWindowInfo->hWnd);
			SetFocus(lpWindowInfo->hWnd);
		}
	}
	HideListView();
	return;
}

//デスクトップアイコンを取得する
HICON GetDesktopIcon(bool bSmall){
	HICON hIconLarge=NULL,hIconSmall=NULL;
	LPITEMIDLIST lvItemList=NULL;
	IShellFolder *pIShellFolder=NULL;
	IExtractIcon *pIExtractIcon=NULL;
	IMalloc *pMalloc;
	SHGetMalloc(&pMalloc);

	CoInitialize(NULL);
	if(SUCCEEDED(SHGetSpecialFolderLocation(NULL,CSIDL_DESKTOP,&lvItemList))){
		SHGetDesktopFolder(&pIShellFolder);
		if(SUCCEEDED(pIShellFolder->GetUIObjectOf(NULL,1,(LPCITEMIDLIST*)&lvItemList,IID_IExtractIcon,0,(void**)&pIExtractIcon))){
			int iIndex=0;
			UINT uFlags;
			TCHAR szLocation[MAX_PATH]={};

			if(SUCCEEDED(pIExtractIcon->GetIconLocation(GIL_FORSHELL,szLocation,MAX_PATH,&iIndex,&uFlags))){
				pIExtractIcon->Extract(szLocation,iIndex,&hIconLarge,&hIconSmall,MAKELONG(32,16));
			}
		}
		pIExtractIcon->Release();
		pIShellFolder->Release();
		pMalloc->Free(lvItemList);
	}
	pMalloc->Release();
	CoUninitialize();
	return (!bSmall)?hIconLarge:hIconSmall;
}

//あの手この手でアイコンを取得[小さい]
HICON GetSmallIcon(HWND hWnd){
	HICON hIcon=NULL;

	if(hWnd==NULL||IsHungAppWindow(hWnd))return NULL;

	//300msで支度しな!
	SendMessageTimeout(hWnd,WM_GETICON,ICON_SMALL,0,SMTO_ABORTIFHUNG,300,(PDWORD_PTR)&hIcon);
	if(hIcon==NULL){
		hIcon=(HICON)GetClassLongPtr(hWnd,GCLP_HICONSM);
	}
	if(hIcon==NULL){
		HWND hParent=GetParent(hWnd);
		if(IsWindowVisible(hParent))return GetSmallIcon(hParent);
	}
	return hIcon;
}

//あの手この手でアイコンを取得[大きい]
HICON GetLargeIcon(HWND hWnd){
	HICON hIcon=NULL;

	if(hWnd==NULL||IsHungAppWindow(hWnd))return NULL;

	//300msで支度しな!
	SendMessageTimeout(hWnd,WM_GETICON,ICON_BIG,0,SMTO_ABORTIFHUNG,300,(PDWORD_PTR)&hIcon);
	if(hIcon==NULL){
		hIcon=(HICON)GetClassLongPtr(hWnd,GCLP_HICON);
	}
	if(hIcon==NULL){
		HWND hParent=GetParent(hWnd);
		if(IsWindowVisible(hParent))return GetLargeIcon(hParent);
	}
	return hIcon;
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

	if(g_hListView!=hListView)return false;
	if(g_eWindowListStatus!=WLS_DISPLAYED)return false;
	if(lpDraw->CtlType!=ODT_LISTVIEW)return false;
	if(!g_iDisplayWindowCount||int(uItemID+1)>g_iDisplayWindowCount)return false;

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
				//ウインドウタイトル取得
				if(!g_DisplayWindowInfo[uItemID]->bDesktopItem&&
				   !g_DisplayWindowInfo[uItemID]->bCancelItem&&
				   !IsHungAppWindow(g_DisplayWindowInfo[uItemID]->hWnd)){
					//300msで支度しな!
					SendMessageTimeout(g_DisplayWindowInfo[uItemID]->hWnd,WM_GETTEXT,MAX_PATH,(LPARAM)g_DisplayWindowInfo[uItemID]->szWindowTitle,SMTO_ABORTIFHUNG,300,NULL);
				}
				DrawText(hDC,g_DisplayWindowInfo[uItemID]->szWindowTitle,-1,&rc,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
				break;

			default:
				break;
		}

		//フォントの復元
		SelectObject(hDC,hOldFont);
	}

	//アイコンを描写する
	if(g_Config.ListView.iIcon!=LISTICON_NO){
		if(!IsHungAppWindow(g_DisplayWindowInfo[uItemID]->hWnd)||
		   g_DisplayWindowInfo[uItemID]->bDesktopItem||
		   g_DisplayWindowInfo[uItemID]->bCancelItem){
			if(g_Config.ListView.iIcon>LISTICON_NO_SMALL){
				if(g_DisplayWindowInfo[uItemID]->bDesktopItem){
					//「デスクトップ」アイコンを読み込む
					hIcon=GetDesktopIcon(g_Config.ListView.iIcon==LISTICON_SMALL);
				}else if(g_DisplayWindowInfo[uItemID]->bCancelItem){
					//「キャンセル」アイコンを読み込む
					hIcon=LoadIcon(NULL,IDI_ERROR);
				}else{
					if(g_Config.ListView.iIcon==LISTICON_SMALL){
						hIcon=GetSmallIcon(g_DisplayWindowInfo[uItemID]->hWnd);
					}
					if(hIcon==NULL){
						//小さいアイコンが取得できない場合、大きなアイコンを利用
						hIcon=GetLargeIcon(g_DisplayWindowInfo[uItemID]->hWnd);
					}

					//デフォルトのアプリケーションアイコンを取得
					if(hIcon==NULL){
						hIcon=LoadIcon(NULL,IDI_APPLICATION);
					}
				}

				if(hIcon){
					ListView_GetItemRect(hListView,uItemID,&rc,LVIR_ICON);
					DrawIconEx(hDC,
							   3/*rc.left*/,
							   rc.top,
							   hIcon,
							   (g_Config.ListView.iIcon==LISTICON_SMALL)?16:32,
							   (g_Config.ListView.iIcon==LISTICON_SMALL)?16:32,
							   0,0,
							   DI_IMAGE|DI_MASK);
				}

				DestroyIcon(hIcon);
				hIcon=NULL;
			}
		}
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

			if(CharLower((LPTSTR)*pszSearchString)==CharLower((LPTSTR)*(pszTargetString++))){
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
bool IncrementalSearch(TCHAR* szSearchString,int iMatchMode,bool bFirstColumnOnly){
	bool bResult=false;
	int iSearchStringBack=0;

	for(int iLength=lstrlen(szSearchString);iSearchStringBack<iLength&&szSearchString[iSearchStringBack]!='\0';++iSearchStringBack);
	--iSearchStringBack;

	//選択中アイテム保存
	HWND hSelectedItemWnd=NULL;

	{
		int iSelectedItem=ListView_GetSelectedItem(g_hListView);
		hSelectedItemWnd=(iSelectedItem!=-1)?g_DisplayWindowInfo[iSelectedItem]->hWnd:NULL;
	}

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
		//未取得のウインドウタイトル処理
		if(g_DisplayWindowInfo[i]->szWindowTitle[0]=='\0'){
			//300msで支度しな!
			SendMessageTimeout(g_DisplayWindowInfo[i]->hWnd,WM_GETTEXT,MAX_PATH,(LPARAM)g_DisplayWindowInfo[i]->szWindowTitle,SMTO_ABORTIFHUNG,300,NULL);
		}

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

	if(ListView_GetSelectedItem(g_hListView)==-1){
		ListView_SelectItem(g_hListView,0);
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

//MSG_MOUSEMOVEのイベントハンドラ
void MsgMouseMove(WPARAM wParam,LPARAM lParam){
	POINT ptMouse;

	//lParamから取れなかったので、GetCursorPos()で取得
	GetCursorPos(&ptMouse);

	int iMonitorX=GetSystemMetrics(SM_CXSCREEN);
	int iMonitorY=GetSystemMetrics(SM_CYSCREEN);

	//右端だと判断する幅を3に調整
	iMonitorX-=3;
	iMonitorY-=3;
	if(ptMouse.x==0){
		if(ptMouse.y==0&&g_Config.ShowWindow.bLeftTop){
			//左上
			ShowListView(SCC_LEFTTOP);
		}else if(iMonitorY<=ptMouse.y&&g_Config.ShowWindow.bLeftBottom){
			//左下
			ShowListView(SCC_LEFTBOTTOM);
		}
	}else if(iMonitorX<=ptMouse.x){
		if(ptMouse.y==0&&g_Config.ShowWindow.bRightTop){
			//右上
			ShowListView(SCC_RIGHTTOP);
		}else if(iMonitorY<=ptMouse.y&&g_Config.ShowWindow.bRightBottom){
			//右下
			ShowListView(SCC_RIGHTBOTTOM);
		}
	}
	return;
}

#ifdef LOWLEVEL_MOUSEHOOK
//低レベルマウスフックプロシージャ
LRESULT CALLBACK LowLevelMouseHookProc(int nCode,WPARAM wParam,LPARAM lParam){
	if(nCode!=HC_ACTION)return CallNextHookEx(NULL/*This parameter is ignored.(by MSDN)*/,nCode,wParam,lParam);

	LPMSG lpMsg=(LPMSG)lParam;

	if(wParam==WM_MOUSEMOVE){
		PostMessage(g_hMainWnd,MSG_MOUSEMOVE,lpMsg->wParam,lpMsg->lParam);
	}
	return CallNextHookEx(NULL/*This parameter is ignored.(by MSDN)*/,nCode,wParam,lParam);
}
#endif

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

	static bool bMouseIn;
	static bool bMenuLoop;
	static bool bMouseTracking;

	switch(uMsg){
		case WM_ERASEBKGND:
			//ちらつき防止
			return FALSE;

		case WM_NCHITTEST:
			//Ctrl+ドラッグでウインドウの移動
			if(GetAsyncKeyState(VK_MENU)&0x8000){
				return HTTRANSPARENT;
			}
			break;

		case WM_ENTERMENULOOP:
			bMenuLoop=true;
			break;

		case WM_EXITMENULOOP:
			bMenuLoop=false;
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
			   GetAsyncKeyState(VK_LBUTTON)&0x8000){
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
						g_bMigemoSearch=(!bShift&&g_Config.IncrementalSearch.iMigemoMode==MIGEMO_DEFAULT)||
							(bShift&&g_Config.IncrementalSearch.iMigemoMode==MIGEMO_NODEFAULT);
					}

					if(!g_Config.IncrementalSearch.bMigemoCaseSensitive){
						//連文節検索を行わない場合であれば大文字小文字を統一
						if(c>='A'&&c<='Z'){
							c+=32;
						}
						if((g_bMigemoSearch&&g_Config.IncrementalSearch.iMigemoMode==MIGEMO_NODEFAULT)||
						   (!g_bMigemoSearch&&g_Config.IncrementalSearch.iMigemoMode==MIGEMO_DEFAULT)){
							if(c>='a'&&c<='z'){
								c-=32;
							}
						}
					}
				}

				TCHAR key[2]={c};
				lstrcat(g_szSearchString,key);

				if(!g_bMigemoSearch||
				   (g_bMigemoSearch&&g_Config.IncrementalSearch.iMigemoDelay==0)){
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
			//選択防止
			bool bAllocated=false;

			for(int i=0;i<SMOUSE_NUM;++i){
				if(!g_Config.Mouse.sMouseTable[i].uCmdId)continue;
				if(MouseType_Table[i].uMessageType==uMsg){
					bAllocated=true;
					break;
				}
			}
			if(!bAllocated)return FALSE;
			break;
		}

		case WM_KEYDOWN:
			//キー入力無効
			return FALSE;

		case WM_NCCALCSIZE:
			//スクロールバーを削除
			ShowScrollBar(hListView,SB_BOTH,false);
			break;

		case WM_MOUSEWHEEL:
			//マウスホイール操作
			if(!(HIWORD(wParam)&0x8000)){//上へ
				SendMessage(g_hMainWnd,WM_COMMAND,MAKEWPARAM(g_Config.Mouse.sMouseTable[SMOUSE_UWHEEL].uCmdId,0),0);
			}else{//下へ
				SendMessage(g_hMainWnd,WM_COMMAND,MAKEWPARAM(g_Config.Mouse.sMouseTable[SMOUSE_DWHEEL].uCmdId,0),0);
			}
			break;

		case WM_SIZE:
			if(wParam==0&&lParam==0){
				bMouseIn=bMouseTracking=bMenuLoop=false;
			}
			break;

		case WM_MOUSEMOVE:
			if(g_eWindowListStatus==WLS_DISPLAYED){
				if(g_Config.ListView.bMouseOut){
					POINT pt={};
					static POINT ptBak={};
					RECT rc={};

					GetCursorPos(&pt);
					if(ptBak.x&&ptBak.y){
						//IDM_KEY_NEXT等々で末尾->先頭移動となるとWM_MOUSEMOVEが飛んでくる?のでbreak
						if(pt.x==ptBak.x||pt.y==ptBak.y){
							ptBak=pt;
							break;
						}
					}
					ptBak=pt;
					GetWindowRect(hListView,&rc);

					if(PtInRect(&rc,pt)){
						bMouseIn=true;

						//マウスホバーで選択
						if(bMenuLoop)break;
						if(GetAsyncKeyState(VK_MENU)&0x8000)break;
						if(IsWindowVisible(g_hToolTipInfo))break;

						if(g_Config.ListView.bMouseHover||
						   GetAsyncKeyState(VK_LBUTTON)&0x8000){
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
					}else if(bMouseIn){
						if(GetForegroundWindow()!=g_hMainWnd)SetForegroundWindowEx(g_hMainWnd);
						SetFocus(hListView);
						if(!bMouseTracking){
							bMouseTracking=true;
							MouseLeaveEvent(hListView);
						}
					}
				}
			}else{
				break;
			}
			break;

		case WM_TIMER:{
			switch(LOWORD(wParam)){
				case IDM_KEY_SWITCH:{
					KillTimer(hListView,wParam);
					if(g_eWindowListStatus!=WLS_DISPLAYED)break;
					if(GetAsyncKeyState(VK_MENU)&0x8000)break;
					if(bMenuLoop)break;
					if(IsWindowVisible(g_hToolTipInfo))break;

					if(g_Config.ListView.iTimeOut>0||
					   GetAsyncKeyState(VK_LBUTTON)&0x8000){

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
			}
			break;
		}

		case WM_MOUSELEAVE:{
			bMouseIn=false;
			bMouseTracking=false;
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

		default:
			break;
	}

	return CallWindowProc(DefListViewProc,hListView,uMsg,wParam,lParam);
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

	//子プロセスメニュー関係
	static HMENU hChildProcessMenu;
	static ULONG_PTR ulSelectedProcessId;

	static HWND hSelectedItemWnd;

	switch(uMsg){
		case WM_NCHITTEST:
			//ウインドウの移動
			return HTCAPTION;

		case WM_ERASEBKGND:
			//ちらつき防止
			return FALSE;

		case WM_CREATE:{
			INITCOMMONCONTROLSEX ic={sizeof(INITCOMMONCONTROLSEX)};

			//リストビュー、ホットキー
			ic.dwICC=ICC_LISTVIEW_CLASSES|ICC_HOTKEY_CLASS;
			InitCommonControlsEx(&ic);

			CoInitialize(NULL);

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

					mii.hSubMenu=mWindowCmd.hSubMenu;
					mii.dwTypeData=_T("&window");
					InsertMenuItem(mMainCmd.hSubMenu,iMenuIndex++,true,&mii);

					mii.hSubMenu=mProcessCmd.hSubMenu;
					mii.dwTypeData=_T("&process");
					InsertMenuItem(mMainCmd.hSubMenu,iMenuIndex++,true,&mii);

					mii.hSubMenu=mCopyInfoCmd.hSubMenu;
					mii.dwTypeData=_T("c&opyinfo");
					InsertMenuItem(mMainCmd.hSubMenu,iMenuIndex++,true,&mii);
					break;
				}
			}

			return 0;
		}

		case WM_INITMENU:{
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
			}
			return 0;
		}

		case WM_SHOWWINDOW:
			if(wParam){
				//表示するならマウスの移動を有効に。
				g_eWindowListStatus=WLS_DISPLAYED;
				if(g_Config.ListView.iTimeOut>0||
				   GetAsyncKeyState(VK_LBUTTON)&0x8000){
					//タイムアウトで確定するためタイマーを設定
					SetTimer(g_hListView,
							 MAKEWPARAM(IDM_KEY_SWITCH,ListView_GetSelectedItem(g_hListView)),
							 (g_Config.ListView.iTimeOut>0)?g_Config.ListView.iTimeOut:DEFAULT_SELECT_TIMEOUT,
							 NULL);
				}

			}
			break;

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
						//表示
						ShowListView(SCC_NONE);
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
					case NM_CLICK:
					case NM_RCLICK:
						if(g_Config.ListView.bDisableDoubleClick){
							SendMessage(hWnd,WM_TIMER,lpNMHdr->code,0);
						}else{
							//ダブルクリックか否か判定するためにタイマーを使用
							SetTimer(hWnd,lpNMHdr->code,GetDoubleClickTime(),NULL);
						}
						break;

					case NM_DBLCLK:
						KillTimer(hWnd,NM_CLICK);
					case NM_RDBLCLK:
						if(lpNMHdr->code==NM_RDBLCLK){
							KillTimer(hWnd,NM_RCLICK);
						}
						SendMessage(hWnd,WM_TIMER,lpNMHdr->code,0);
						break;

					case LVN_ITEMCHANGED:
						//アイテムの選択が変更された場合
						ShowToolTip(g_hListView,ID_TOOLTIP_INFO);
						ListView_RedrawItems(lpNMHdr->hwndFrom,((LPNMLISTVIEW)lParam)->iItem,((LPNMLISTVIEW)lParam)->iItem);
						if(g_Config.ListView.iTimeOut>0||
						   GetAsyncKeyState(VK_LBUTTON)&0x8000){
							//タイムアウトで確定するためタイマーを設定
							SetTimer(g_hListView,
									 MAKEWPARAM(IDM_KEY_SWITCH,ListView_GetSelectedItem(g_hListView)),
									 (g_Config.ListView.iTimeOut>0)?g_Config.ListView.iTimeOut:DEFAULT_SELECT_TIMEOUT,
									 NULL);
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
			switch(wParam){
				case WM_CHAR:
					KillTimer(hWnd,wParam);
					IncrementalSearch(g_szSearchString,g_iMatchMode,g_bFirstColumnOnly);
					return DefWindowProc(hWnd,uMsg,wParam,lParam);
				case MAKEWPARAM(IDM_KEY_BS_STRING,1):
					//インクリメンタルサーチの文字列を一つ削る
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
				case NM_CLICK:
				case NM_RCLICK:
					KillTimer(hWnd,wParam);
					break;
				case NM_DBLCLK:
				case NM_RDBLCLK:
					break;
				default:
					return DefWindowProc(hWnd,uMsg,wParam,lParam);
			}

			bool bShift=(GetAsyncKeyState(VK_SHIFT)&0x8000)!=0;
			bool bControl=(GetAsyncKeyState(VK_CONTROL)&0x8000)!=0;
			UINT uMouseType=0;

			switch(wParam){
				case NM_CLICK:uMouseType=SMOUSE_LCLICK;break;
				case NM_DBLCLK:uMouseType=SMOUSE_LDCLICK;break;
				case NM_RCLICK:uMouseType=SMOUSE_RCLICK;break;
				case NM_RDBLCLK:uMouseType=SMOUSE_RDCLICK;break;
			}
			if(bShift&&bControl)uMouseType+=12;
			else if(bShift)uMouseType+=4;
			else if(bControl)uMouseType+=8;
			SendMessage(hWnd,WM_COMMAND,MAKEWPARAM(g_Config.Mouse.sMouseTable[uMouseType].uCmdId,0),0);
			break;
		}

		case WM_COMMAND:
			switch(LOWORD(wParam)){
				case IDM_KEY_SWITCH:{
					//切り替え
					if(g_eWindowListStatus!=WLS_DISPLAYED)break;

					int iIndex=ListView_GetSelectedItem(g_hListView);
					if(iIndex!=-1){
						TaskSwitch(g_DisplayWindowInfo[iIndex]);
					}else{
						HideListView();
					}
					break;
				}

				case IDM_KEY_CANCEL:
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
					//インクリメンタルサーチの検索方法を保存
					g_iMatchMode=g_Config.IncrementalSearch.iMatchMode;
					//検索する項目設定を保存
					g_bFirstColumnOnly=g_Config.IncrementalSearch.bFirstColumnOnly;
					//Migemoモードリセット
					g_bMigemoSearch=false;
					//候補が1つなら切り替える設定を保存
					g_bEnterUniqueWindow=g_Config.IncrementalSearch.bEnterUniqueWindow;
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
						ShowWindow(g_DisplayWindowInfo[iIndex]->hWnd,SW_RESTORE);
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

				case IDM_KEY_CLOSE:{
					//閉じる
					int iIndex=ListView_GetSelectedItem(g_hListView);
					if(iIndex<0)break;
					if(!IsHungAppWindow(g_DisplayWindowInfo[iIndex]->hWnd)&&
					   IsWindowEnabled(g_DisplayWindowInfo[iIndex]->hWnd)){
						PostMessage(g_DisplayWindowInfo[iIndex]->hWnd,WM_CLOSE,(WPARAM)0,(LPARAM)0);
					}
					HideListView();
					break;
				}

				case IDM_KEY_TERMINATE:{
					//プロセスを強制終了
					int iIndex=ListView_GetSelectedItem(g_hListView);
					if(iIndex<0)break;
					if(IsWindowEnabled(g_DisplayWindowInfo[iIndex]->hWnd)){
						DWORD dwProcessId=0;

						if(GetWindowThreadProcessId(g_DisplayWindowInfo[iIndex]->hWnd,&dwProcessId)!=0){
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
					if(IsWindowEnabled(g_DisplayWindowInfo[iIndex]->hWnd)){
						DWORD dwProcessId=0;

						if(GetWindowThreadProcessId(g_DisplayWindowInfo[iIndex]->hWnd,&dwProcessId)!=0){
							PROCESSENTRY32* pProcessEntry32=AllocProcessList();

							if(pProcessEntry32==NULL)break;
							TerminateProcessTree(pProcessEntry32,dwProcessId);
							FreeProcessList(pProcessEntry32);
						}
					}
					HideListView();
					break;
				}

				case IDM_KEY_AND:
					//AND検索
					if(g_szSearchString[0]!='\0')SendMessage(g_hListView,WM_CHAR,' ',0);
					//インクリメンタルサーチ中でなければ確定
					else SendMessage(hWnd,WM_COMMAND,MAKEWPARAM(IDM_KEY_SWITCH,0),0);
					break;

				case IDM_KEY_BS_STRING:{
					//インクリメンタルサーチの文字列を一つ削る
					if(g_Config.ListView.iTimeOut>0||
					   GetAsyncKeyState(VK_LBUTTON)&0x8000){
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

						if(!g_bMigemoSearch||
						   (g_bMigemoSearch&&g_Config.IncrementalSearch.iMigemoDelay==0)){
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
					if(IsWindowEnabled(g_DisplayWindowInfo[iIndex]->hWnd)){
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

						if(GetWindowThreadProcessId(g_DisplayWindowInfo[iIndex]->hWnd,&dwProcessId)!=0){
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

					DWORD dwProcessId=0;

					if(!IsHungAppWindow(g_DisplayWindowInfo[iIndex]->hWnd)&&
					   IsWindowEnabled(g_DisplayWindowInfo[iIndex]->hWnd)&&
					   GetWindowThreadProcessId(g_DisplayWindowInfo[iIndex]->hWnd,&dwProcessId)!=0){
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
							//カーソルがウインドウリスト外にあるなら右上へ移動させる
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
									if(hWnd!=NULL){
										//300msで支度しな!
										SendMessageTimeout(hWnd,WM_GETTEXT,MAX_PATH,(LPARAM)szWindowTitle,SMTO_ABORTIFHUNG,300,NULL);
									}else{
										lstrcpy(szWindowTitle, _T(""));
									}

									//コマンドライン
									TCHAR* pszCommandLine=AllocProcessCommandLine((DWORD)ulSelectedProcessId);

									WINDOWPLACEMENT wndpl={sizeof(WINDOWPLACEMENT)};

									GetWindowPlacement(hWnd,&wndpl);

									//最前面表示か否か
									if(GetWindowLongPtr(hWnd,GWL_EXSTYLE)&WS_EX_TOPMOST){
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

									TCHAR* pszText=(TCHAR*)HeapAlloc(GetProcessHeap(),
														 HEAP_ZERO_MEMORY,
														 (lstrlen(szFileName)+
														  lstrlen(szWindowTitle)+
														  lstrlen(pszCommandLine)+
														  lstrlen(szTopMost)+
														  lstrlen(szShowCmd)+
														  lstrlen(szPriority)+
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
											 _T("%s"),
											 szFileName,
											 szWindowTitle,
											 pszCommandLine,
											 szTopMost,
											 szShowCmd,
											 szPriority);

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

					if(!IsHungAppWindow(g_DisplayWindowInfo[iIndex]->hWnd)&&
						IsWindowEnabled(g_DisplayWindowInfo[iIndex]->hWnd)&&
						GetWindowThreadProcessId(g_DisplayWindowInfo[iIndex]->hWnd,&dwProcessId)!=0){
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

					if(!IsHungAppWindow(g_DisplayWindowInfo[iIndex]->hWnd)&&
					   IsWindowEnabled(g_DisplayWindowInfo[iIndex]->hWnd)&&
					   GetWindowThreadProcessId(g_DisplayWindowInfo[iIndex]->hWnd,&dwProcessId)!=0){
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

					if(!IsHungAppWindow(g_DisplayWindowInfo[iIndex]->hWnd)&&
					   IsWindowEnabled(g_DisplayWindowInfo[iIndex]->hWnd)&&
					   GetWindowThreadProcessId(g_DisplayWindowInfo[iIndex]->hWnd,&dwProcessId)!=0){
						TCHAR szFileName[MAX_PATH];
						TCHAR* pszCommandLine=AllocProcessCommandLine(dwProcessId);
						TCHAR szTopMost[24];
						TCHAR szShowCmd[24];
						TCHAR szPriority[64];

						//ファイル名
						if(!GetProcessFileName(dwProcessId,szFileName,MAX_PATH)){
							lstrcpy(szFileName,_T(""));
						}

						//最前面表示か否か
						if(GetWindowLongPtr(g_DisplayWindowInfo[iIndex]->hWnd,GWL_EXSTYLE)&WS_EX_TOPMOST){
							lstrcpy(szTopMost,_T("WS_EX_TOPMOST\n"));
						}else{
							lstrcpy(szTopMost,_T(""));
						}

						WINDOWPLACEMENT wndpl={sizeof(WINDOWPLACEMENT)};

						GetWindowPlacement(g_DisplayWindowInfo[iIndex]->hWnd,&wndpl);

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

						TCHAR* pszText=(TCHAR*)HeapAlloc(GetProcessHeap(),
														 HEAP_ZERO_MEMORY,
														 (lstrlen(szFileName)+
														  lstrlen(g_DisplayWindowInfo[iIndex]->szWindowTitle)+
														  lstrlen(pszCommandLine)+
														  lstrlen(szTopMost)+
														  lstrlen(szShowCmd)+
														  lstrlen(szPriority)+
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
								 _T("%s"),
								 szFileName,
								 g_DisplayWindowInfo[iIndex]->szWindowTitle,
								 pszCommandLine,
								 szTopMost,
								 szShowCmd,
								 szPriority);

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

				case IDM_KEY_DESKTOP:{
					//デスクトップを表示
					HideListView();
#if 0
					ToggleDesktop();
#endif

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
				case IDM_KEY_PROCESSMENU:{
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
						//マウスフックアンインストール
						UninstallMouseHook();
						UninstallMouseLibHook();

						//Ctrl+Aフックインストール
						HHOOK hkCtrlA=SetWindowsHookEx(WH_MSGFILTER,CtrlAHookProc,NULL,GetCurrentThreadId());

						ShowSettingsDialog(hWnd,(LOWORD(wParam)==IDM_MENU_SETTINGS)?TAB_GENERAL:TAB_VERSION);

						//Ctrl+Aフックアンインストール
						UnhookWindowsHookEx(hkCtrlA);

						//ホットキーを登録する
						if(g_Config.ShowWindow.wHotKey){
							RegistHotKey(hWnd,ID_HOTKEY,g_Config.ShowWindow.wHotKey);
						}

						//インクリメンタルサーチの検索方法を保存
						g_iMatchMode=g_Config.IncrementalSearch.iMatchMode;
						//検索する項目設定を保存
						g_bFirstColumnOnly=g_Config.IncrementalSearch.bFirstColumnOnly;
						//Migemo検索リセット
						g_bMigemoSearch=false;
						//候補が1つなら切り替える設定を保存
						g_bEnterUniqueWindow=g_Config.IncrementalSearch.bEnterUniqueWindow;

						//Migemo読み込み/解放
						if(g_Config.IncrementalSearch.iMigemoMode==MIGEMO_NO){
							UnloadMigemo();
						}else{
							LoadMigemo();
						}

						//アクセラレータテーブルを読み込む
						UpdateAcceleratorTable();

						//不透明度を設定
						SetLayeredWindowAttributes(hWnd,0,g_Config.ListView.byAlpha,LWA_ALPHA);

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

						//マウスフックインストール
						InstallMouseHook();
					}
					break;
				}

				case IDM_MENU_EXIT:
					//終了
					PostMessage(hWnd,WM_CLOSE,0,0);
					break;

				default:
					break;
			}
			break;

		case WM_CLOSE:{
			//最前面表示解除
			SetWindowPos(g_hMainWnd,
						 HWND_NOTOPMOST,
						 0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);

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

			//メニュー解放
			DestroyMenu(mTray.hMenu);
			DestroyMenu(mMainCmd.hMenu);
			DestroyMenu(mSelectCmd.hMenu);
			DestroyMenu(mCopyInfoCmd.hMenu);
			DestroyMenu(mIncrementalSearchCmd.hMenu);
			DestroyMenu(mWindowCmd.hMenu);
			DestroyMenu(mProcessCmd.hMenu);
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
#ifdef LOWLEVEL_MOUSEHOOK
			if(uMsg==MSG_MOUSEMOVE){
#else
			if(uMsg==WM_MOUSEMOVE){
#endif
				//WM_MOUSEMOVE
				if(g_eWindowListStatus==WLS_HIDE&&!g_bDisable&&!g_bSettingsDialog){
					MsgMouseMove(wParam,lParam);
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

	hMutex=CreateMutex(NULL,false,szMutexName);
	if(GetLastError()==ERROR_ALREADY_EXISTS){
		//起動済み
		MessageBox(NULL,_T("Tascherは既に起動しています。"),_T("Tascher"),MB_OK|MB_ICONEXCLAMATION|MB_ICONEXCLAMATION);
		CloseHandle(hMutex);
		ExitProcess(0);
	}

	//インスタンスハンドル取得
	g_hInstance=GetModuleHandle(NULL);

	//タスクトレイ用メッセージ登録
	MSG_TRAY=RegisterWindowMessage(_T("MSG_TRAY"));
	//タスクバー再作成用メッセージ登録
	MSG_TRAYCREATED=RegisterWindowMessage(_T("TaskbarCreated"));

#ifdef LOWLEVEL_MOUSEHOOK
	//WM_MOUSEMOVE用メッセージ登録
	MSG_MOUSEMOVE=RegisterWindowMessage(_T("MSG_MOUSEMOVE"));
#endif

	ULONG_PTR ulToken;
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;

	Gdiplus::GdiplusStartup(&ulToken,&gdiplusStartupInput,NULL);

	//cfgファイルを読み込む
	ReadPrivateProfile();

	//ウインドウクラスを登録
	if(InitApplication(g_hInstance,szClassName)==0)ExitProcess(0);

	hWnd=CreateWindowEx(WS_EX_TOOLWINDOW|WS_EX_TOPMOST|WS_EX_ACCEPTFILES|WS_EX_LAYERED,
						szClassName,
						szAppName,
						WS_POPUP|WS_CLIPCHILDREN,
						CW_USEDEFAULT,CW_USEDEFAULT,
						CW_USEDEFAULT,CW_USEDEFAULT,
						NULL,
						NULL,
						g_hInstance,
						NULL);

	if(hWnd==NULL)ExitProcess(0);
	//メインウインドウハンドルを保存
	g_hMainWnd=hWnd;

	//不透明度を設定
	SetLayeredWindowAttributes(hWnd,0,g_Config.ListView.byAlpha,LWA_ALPHA);

	//メッセージ受け取り許可
	bool(WINAPI*p_changeWindowMessageFilter)(UINT,DWORD)=(bool(WINAPI*)(UINT,DWORD))::GetProcAddress(::GetModuleHandle(_T("user32.dll")),"ChangeWindowMessageFilter");
	if(p_changeWindowMessageFilter!=NULL){
		//MSGFLT_ADD
		p_changeWindowMessageFilter(WM_MOUSEMOVE,1);
	}

	//マウスフックインストール
	InstallMouseHook();

	//アクセラレータテーブルを読み込む
	UpdateAcceleratorTable();

	//Migemo,bregonig読み込み
	if(g_Config.IncrementalSearch.iMigemoMode!=MIGEMO_NO){
		LoadMigemo();
	}

	//ウインドウ情報用ヒープ作成
	g_hHeapWindowInfo=HeapCreate(NULL,0,0);

	while(GetMessage(&msg,NULL,0,0)){
		if(!TranslateAccelerator(hWnd,g_hAccel,&msg)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	//ウインドウ情報用ヒープ破棄
	HeapDestroy(g_hHeapWindowInfo);

	//Migemo/bregonig解放
	UnloadMigemo();

	//アクセラレータテーブルを破棄
	DestroyAcceleratorTable(g_hAccel);

	//マウスフックアンインストール
	UninstallMouseHook();

	Gdiplus::GdiplusShutdown(ulToken);

	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
	ExitProcess((UINT)msg.wParam);
}
