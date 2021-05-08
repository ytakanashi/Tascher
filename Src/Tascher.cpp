//Tascher.cpp
//メイン

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.64
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/


#include"StdAfx.h"
#include"CommonSettings.h"
#include"Utilities.h"
#include"Path.h"
#include"PrivateProfile.h"
#include"Settings.h"
#include"COM.h"
#include"Migemo.h"
#include"Direct2D.h"
#include"WIC.h"
#include"Gdiplus.h"
#include"Dwm.h"
#include"WebBrowserTab.h"
#include"resources/resource.h"
#include<psapi.h>
#include<tlhelp32.h>
#include<shellapi.h>
#include<Shlobj.h>



//[各種コントロールのID]

enum{
ID_TRAYMENU=100,
IDC_LISTVIEW,
ID_TIMER_HOVER,
ID_TIMER_UNIQUE_WINDOW,
ID_TIMER_HIDELISTVIEW,
ID_TIMER_THUMBNAIL,
ID_TIMER_READY,
ID_TIMER_MOUSE_MOVE_LEFTTOP,
ID_TIMER_MOUSE_MOVE_TOP,
ID_TIMER_MOUSE_MOVE_RIGHTTOP,
ID_TIMER_MOUSE_MOVE_LEFT,
ID_TIMER_MOUSE_MOVE_RIGHT,
ID_TIMER_MOUSE_MOVE_LEFTBOTTOM,
ID_TIMER_MOUSE_MOVE_BOTTOM,
ID_TIMER_MOUSE_MOVE_RIGHTBOTTOM,
ID_TIMER_HIDE_CURSOR,
ID_TIMER_CLIP_CURSOR,
ID_TOOLTIP_INFO,
ID_HOTKEY,
};


//[ハンドル系]
//メインウインドウハンドル
HWND g_hMainWnd;

//リストビューハンドル
HWND g_hListView;

//検索文字列用ウインドウ
HWND g_hSearchWnd;

//情報表示用ツールチップハンドル
HWND g_hInfoToolTip;

//表示中メニューのウインドウハンドル
HWND g_hMenuWnd;

//サムネイルウインドウハンドル
HWND g_hThumbnailWnd;

//Direct2Dによるリストビュー描画
direct2d::ListView* g_pDirectWriteListView;

//アクセラレータテーブルハンドル
HACCEL g_hAccel;

//リストビュー用フォント
HFONT g_hDefaultItemFont;
HFONT g_hSelectedItemFont;

//ツールチップ用フォント
HFONT g_hSearchWindowFonts[INCREMENTALSEARCH_SEARCHWINDOWDESIGN_NUM];

//GDI+による背景画像描画
gdiplus::BackgroundImage* g_pGdiplusBackgroundImage;

//設定構造体
CONFIG g_Config;

//非表示用カーソル
HCURSOR g_hBlankCursor;

struct MENU{
	HMENU hMenu;
	HMENU hSubMenu;
	bool Load(UINT uId){
		hMenu=LoadMenu(GetModuleHandle(NULL),MAKEINTRESOURCE(uId));
		hSubMenu=GetSubMenu(hMenu,0);
		return hMenu&&hSubMenu;
	}
};

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
	bool bWinUIDesktop;//WinUI(WinUIDesktopWin32WindowClass)ウインドウ
	HTHUMBNAIL hThumbnail;
	THREAD Thread;
	ID2D1Bitmap* pD2DBitmap;
	COLORREF clrUWPAppBackground;//UWPApp用背景色
	bool bUWPAppBackground;
	//WebBrowserTab用
	bool bWebBrowserTabItem;
	TCHAR szURL[MAX_PATH];
	TCHAR szId[128];
	int iWebBrowserIndex;
	//WebBrowserTab時のみ使用
	DWORD dwProcessId;
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

//WebBrowserTabリスト操作
webbrowsertab::List* g_pWebBrowserTab;

const int INCREMENTAL_SEARCH_BUFFER=32;
TCHAR g_szSearchString[INCREMENTAL_SEARCH_BUFFER];
//インクリメンタルサーチウインドウ表示用
TCHAR* g_pszDisplaySearchString;

struct LISTVIEWCOLUMN_TABLE g_ListViewColumn_Table[LISTITEM_NUM];

const int THREAD_TIMEOUT=10000;

//RegisterWindowMessage()するメッセージ
UINT MSG_MOUSEMOVE;//[マウスの移動]
UINT MSG_MOUSEWHEEL;//マウスホイール
UINT MSG_TRAY;//タスクトレイ用メッセージ
UINT MSG_TRAYCREATED;//タスクバー再作成用メッセージ

//X1、X2クリック用独自メッセージ
enum{
WM_X1BUTTONDOWN=WM_APP+1,
WM_X2BUTTONDOWN,
WM_X1BUTTONDBLCLK,
WM_X2BUTTONDBLCLK,

//情報取得用スレッド
WM_GETWINDOWINFO_TITLE,
WM_GETWINDOWINFO_ALL,

//TaskSwitch用スレッド
//スレッド→メインウインドウ終了通知
WM_CLOSETHREAD,
};

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

const UINT uCommandIndex[]={
		0,
		IDM_KEY_COMMAND1,
		IDM_KEY_COMMAND2,
		IDM_KEY_COMMAND3,
		IDM_KEY_COMMAND4,
		IDM_KEY_COMMAND5,
		IDM_KEY_COMMAND6,
		IDM_KEY_COMMAND7,
		IDM_KEY_COMMAND8,
		IDM_KEY_COMMAND9,
		IDM_KEY_COMMAND10,
		IDM_KEY_COMMAND11,
		IDM_KEY_COMMAND12,
		IDM_KEY_COMMAND13,
		IDM_KEY_COMMAND14,
		IDM_KEY_COMMAND15,
		IDM_KEY_COMMAND16,
		IDM_KEY_COMMAND17,
		IDM_KEY_COMMAND18,
		IDM_KEY_COMMAND19,
		IDM_KEY_COMMAND20,
		IDM_KEY_COMMAND21,
		IDM_KEY_COMMAND22,
		IDM_KEY_COMMAND23,
		IDM_KEY_COMMAND24,
		IDM_KEY_COMMAND25,
		IDM_KEY_COMMAND26,
		IDM_KEY_COMMAND27,
		IDM_KEY_COMMAND28,
		IDM_KEY_COMMAND29,
		IDM_KEY_COMMAND30,
		IDM_KEY_COMMAND31,
		IDM_KEY_COMMAND32,
		IDM_KEY_COMMAND33,
		IDM_KEY_COMMAND34,
		IDM_KEY_COMMAND35,
		IDM_KEY_COMMAND36,
		IDM_KEY_COMMAND37,
		IDM_KEY_COMMAND38,
		IDM_KEY_COMMAND39,
		IDM_KEY_COMMAND40,
		IDM_KEY_COMMAND41,
		IDM_KEY_COMMAND42,
		IDM_KEY_COMMAND43,
		IDM_KEY_COMMAND44,
		IDM_KEY_COMMAND45,
		IDM_KEY_COMMAND46,
		IDM_KEY_COMMAND47,
		IDM_KEY_COMMAND48,
		IDM_KEY_COMMAND49,
		IDM_KEY_COMMAND50,
	};


namespace status{
	//無効
	bool bDisable;

	//設定ダイアログが表示されている
	bool bShowSettingsDialog;

	//ウインドウリストの状態
	enum class WINDOWLIST{
		HIDE=0,
		CREATING,
		CREATED,
		DISPLAYED,
		SWITCHING,
	}eWindowList;

	//四隅表示のコーナー種
	SCC_CORNERS eCorners;

	//背景画像が有効である
	bool bBackground;

	//ホットキーから表示
	bool bHotKeyMode;

	//修飾キーの状態
	BYTE byModKey;

	//Migemo検索中マッチするアイテムが無い
	bool bMigemoNomatch;

	//マウス操作からコマンドを実行
	bool bMouseCommand;

	//EnumWindows()でウインドウリストを取得した
	bool bEnumWindows;
};

//一時的な設定
namespace tempconf{
	//インクリメンタルサーチでの検索方法
	int iMatchMode;

	//左側の項目のみ検索する
	bool bFirstColumnOnly;

	//Migemoで検索
	bool bMigemoSearch;

	//候補が1つなら切り替える
	bool bEnterUniqueWindow;

	//表示項目
	bool bWindowItem;
	bool bWebBrowserTabItem;

	//不透明度
	BYTE byOpacity;

	//サムネイル設定
	UINT uThumbnail,uThumbnailBak;

	//サムネイルを元の大きさで表示
	bool bThumbnailOriginalSize;

	//一時的な設定をリセット
	void Reset(){
		//インクリメンタルサーチの検索方法
		iMatchMode=g_Config.IncrementalSearch.iMatchMode;
		//検索する項目設定
		bFirstColumnOnly=g_Config.IncrementalSearch.bFirstColumnOnly;
		//Migemo検索
		bMigemoSearch=false;
		//候補が1つなら切り替える設定
		bEnterUniqueWindow=g_Config.IncrementalSearch.bEnterUniqueWindow;
		//サムネイル設定
		uThumbnail=g_Config.ListView.uThumbnail;
		if(uThumbnail!=THUMBNAIL_NO)uThumbnailBak=uThumbnail;
		else uThumbnailBak=THUMBNAIL_SECOND_COLUMN;
		//サムネイルを元の大きさで表示する設定
		bThumbnailOriginalSize=g_Config.ListView.bThumbnailOriginalSize;
		//ウインドウアイテム/ウェブブラウザタブアイテムの表示/非表示
		bWindowItem=g_Config.ListView.bWindowItem;
		bWebBrowserTabItem=g_Config.ListView.bWebBrowserTabItem;
	}
};

//インクリメンタルサーチの状況
UINT GetCurrentIncrementalSearchState(){
	if(tempconf::bMigemoSearch){
		if(!status::bMigemoNomatch){
			//Migemo検索一致
			return INCREMENTALSEARCH_SEARCHWINDOWDESIGN_MIGEMOMATCH;
		}else{
			//Migemo検索不一致
			return INCREMENTALSEARCH_SEARCHWINDOWDESIGN_MIGEMONOMATCH;
		}
	}else{
		//デフォルト
		return INCREMENTALSEARCH_SEARCHWINDOWDESIGN_DEFAULT;
	}
}

//現在のインクリメンタルサーチデザインを取得
SEARCHWINDOWDESIGN* GetCurrentSearchWindowDesign(){
	switch(GetCurrentIncrementalSearchState()){
		case INCREMENTALSEARCH_SEARCHWINDOWDESIGN_MIGEMOMATCH:
			return &g_Config.IncrementalSearch.MigemoMatchSearchWindowDesign;
		case INCREMENTALSEARCH_SEARCHWINDOWDESIGN_MIGEMONOMATCH:
			return &g_Config.IncrementalSearch.MigemoNoMatchSearchWindowDesign;
		case INCREMENTALSEARCH_SEARCHWINDOWDESIGN_DEFAULT:
		default:
			return &g_Config.IncrementalSearch.DefaultSearchWindowDesign;
	}
}

int ScaleWidth(int iWidth,int iDefDpi=96){
	UINT dpiX=0,dpiY=0;

	HMONITOR hMonitor=g_Config.GetActiveMonitor();

	if(!GetDpiForMonitor(hMonitor,&dpiX,&dpiY)){
		HDC hActiveMonitorDC=CreateMonitorDC(hMonitor);
		dpiX=GetDeviceCaps(hActiveMonitorDC,LOGPIXELSX);
		DeleteDC(hActiveMonitorDC);
	}

	return MulDiv(iWidth,dpiX,iDefDpi);
}

int ScaleHeight(int iHeight,int iDefDpi=96){
	UINT dpiX=0,dpiY=0;

	HMONITOR hMonitor=g_Config.GetActiveMonitor();

	if(!GetDpiForMonitor(hMonitor,&dpiX,&dpiY)){
		HDC hActiveMonitorDC=CreateMonitorDC(hMonitor);
		dpiY=GetDeviceCaps(hActiveMonitorDC,LOGPIXELSY);
		DeleteDC(hActiveMonitorDC);
	}

	return MulDiv(iHeight,dpiY,iDefDpi);
}

void ReleaseBrush(){
	if(g_pDirectWriteListView!=NULL){
		g_pDirectWriteListView->ReleaseTextFormat();
		g_pDirectWriteListView->ReleaseTextBrush();
		g_pDirectWriteListView->ReleaseTextBkBrush();
		g_pDirectWriteListView->ReleaseDialogFrameBrush();
	}
}

void CreateBrush(){
	if(g_pDirectWriteListView!=NULL&&
	   g_pDirectWriteListView->IsAvailable()&&
	   g_pDirectWriteListView->GetRenderTarget()!=NULL){
		g_pDirectWriteListView->CreateDefaultTextFormat(g_hDefaultItemFont);
		g_pDirectWriteListView->CreateSelectedTextFormat(g_hSelectedItemFont);
		g_pDirectWriteListView->CreateDefaultTextBrush(g_Config.DefaultItemDesign.clrText);
		g_pDirectWriteListView->CreateSelectedTextBrush(g_Config.SelectedItemDesign.clrText);
		g_pDirectWriteListView->CreateDefaultTextBkBrush(g_Config.DefaultItemDesign.clrTextBk,g_Config.DefaultItemDesign.clrTextBkGradientEnd);
		g_pDirectWriteListView->CreateSelectedTextBkBrush(g_Config.SelectedItemDesign.clrTextBk,g_Config.SelectedItemDesign.clrTextBkGradientEnd);
		g_pDirectWriteListView->CreateDialogFrameBrush(g_Config.ListView.clrDialogFrame);
		g_pDirectWriteListView->SetTextRenderingParams(g_Config.DirectWrite.uGamma,
													 g_Config.DirectWrite.uEnhancedContrast,
													 g_Config.DirectWrite.uClearTypeLevel,
													 g_Config.DirectWrite.uRenderingMode);
	}
}

void ReloadFont(){
	//フォントを解放
	if(g_hDefaultItemFont!=NULL)DeleteObject(g_hDefaultItemFont);
	if(g_hSelectedItemFont!=NULL)DeleteObject(g_hSelectedItemFont);
	for(int i=0;i<INCREMENTALSEARCH_SEARCHWINDOWDESIGN_NUM;i++){
		if(g_hSearchWindowFonts[i]!=NULL)DeleteObject(g_hSearchWindowFonts[i]);
	}

	//フォントを再読み込み
	g_hDefaultItemFont=CreateGUIFont(g_Config.DefaultItemDesign.szFont,g_Config.DefaultItemDesign.iFontSize,g_Config.DefaultItemDesign.iFontStyle);
	g_hSelectedItemFont=CreateGUIFont(g_Config.SelectedItemDesign.szFont,g_Config.SelectedItemDesign.iFontSize,g_Config.SelectedItemDesign.iFontStyle);
	g_hSearchWindowFonts[INCREMENTALSEARCH_SEARCHWINDOWDESIGN_DEFAULT]=CreateGUIFont(g_Config.IncrementalSearch.DefaultSearchWindowDesign.szFont,g_Config.IncrementalSearch.DefaultSearchWindowDesign.iFontSize,g_Config.IncrementalSearch.DefaultSearchWindowDesign.iFontStyle);
	g_hSearchWindowFonts[INCREMENTALSEARCH_SEARCHWINDOWDESIGN_MIGEMOMATCH]=CreateGUIFont(g_Config.IncrementalSearch.MigemoMatchSearchWindowDesign.szFont,g_Config.IncrementalSearch.MigemoMatchSearchWindowDesign.iFontSize,g_Config.IncrementalSearch.MigemoMatchSearchWindowDesign.iFontStyle);
	g_hSearchWindowFonts[INCREMENTALSEARCH_SEARCHWINDOWDESIGN_MIGEMONOMATCH]=CreateGUIFont(g_Config.IncrementalSearch.MigemoNoMatchSearchWindowDesign.szFont,g_Config.IncrementalSearch.MigemoNoMatchSearchWindowDesign.iFontSize,g_Config.IncrementalSearch.MigemoNoMatchSearchWindowDesign.iFontStyle);
}
void InitializeImageList(){
	g_Config.ListView.iIconWidth=ScaleWidth(g_Config.GetListIconSize()+g_Config.ListView.iIconMargin*2);
	ListView_InitColumn(g_ListViewColumn_Table,&g_Config,g_hListView);

	//既存のイメージリストを破棄
	HIMAGELIST hImageList=ListView_GetImageList(g_hListView,LVSIL_SMALL);
	if(hImageList!=NULL)ImageList_Destroy(hImageList);

	HDC hActiveMonitorDC=CreateMonitorDC(g_Config.GetActiveMonitor());
	int iLineHeight=ScaleWidth(g_Config.GetListIconHeight()+g_Config.ListView.iIconMargin*2);

	ListView_SetImageList(g_hListView,
						  ImageList_Create(
							  ScaleWidth(iLineHeight),
							  ScaleHeight(iLineHeight),
							  ILC_COLOR24,0,0),
						  LVSIL_SMALL);

	for(UINT i=0;i<ARRAY_SIZEOF(g_ListViewColumn_Table);++i){
		ListView_SetColumnWidth(g_hListView,g_ListViewColumn_Table[i].iListItem,
								ScaleWidth(*g_ListViewColumn_Table[i].piWidth));
	}

	//リストビューカラムの並び順を設定
	ListView_SetColumnOrderArray(g_hListView,LISTITEM_NUM,g_Config.ListView.iColumnOrder);

	DeleteDC(hActiveMonitorDC);
}

void FilterItemType(){
	g_iDisplayWindowCount=0;
	if(!tempconf::bWindowItem&&!tempconf::bWebBrowserTabItem)tempconf::bWindowItem=true;
	for(int i=0;i<g_iWindowCount;i++){
		if(!tempconf::bWindowItem&&
		   !g_WindowInfo[i].bDesktopItem&&
			   !g_WindowInfo[i].bCancelItem&&
			   !g_WindowInfo[i].bWebBrowserTabItem)continue;
		if(!tempconf::bWebBrowserTabItem&&
		   g_WindowInfo[i].bWebBrowserTabItem)continue;
		g_DisplayWindowInfo[g_iDisplayWindowCount++]=&g_WindowInfo[i];
	}
	return;
}

//タスクトレイ登録/解除
void TaskTray(HWND hWnd,HICON hIcon,DWORD dwMessage){
	NOTIFYICONDATA NotifyIconData={};
	NotifyIconData.cbSize=sizeof(NOTIFYICONDATA);
	NotifyIconData.hWnd=hWnd;
	NotifyIconData.uID=ID_TRAYMENU;
	NotifyIconData.uFlags=NIF_MESSAGE|NIF_ICON|NIF_TIP;
	NotifyIconData.uCallbackMessage=MSG_TRAY;
	NotifyIconData.hIcon=hIcon;

	//ツールチップ
	lstrcpy(NotifyIconData.szTip,TASCHER_VERSION);

	for(int i=0;i<10;i++){
		if(Shell_NotifyIcon(dwMessage,&NotifyIconData)){
			return;
		}else{
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

			MENUITEMINFO mii={};
			mii.cbSize=sizeof(MENUITEMINFO);
			mii.fMask=MIIM_STRING|MIIM_SUBMENU;
			mii.dwTypeData=pProcessEntry32[i].szExeFile;

			HMENU hSubMenu=CreatePopupMenu();

			int iSubMenuCount=0;

			//processメニュー追加
			for(;iSubMenuCount<GetMenuItemCount(hProcessSubMenu);iSubMenuCount++){
				MENUITEMINFO miiProcess={};
				miiProcess.cbSize=sizeof(MENUITEMINFO);
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
			MENUITEMINFO miiSubMenu={};
			miiSubMenu.cbSize=sizeof(MENUITEMINFO);
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
HICON GetWindowIcon(HWND hWnd,bool bUWPApp=false){
	HICON hIcon=NULL;

	if(g_Config.ListView.iIcon>LISTICON_NO_SMALL&&
	   !IsHungAppWindow(hWnd)){
		if(!g_Config.ListView.bIconFromWindow||
		   bUWPApp){
			hIcon=GetIcon(hWnd,g_Config.GetListIconSize(),bUWPApp,g_Config.ListView.bUWPAppNoBackgroundColor);
		}else{
			switch(g_Config.ListView.iIcon){
				case LISTICON_SMALL:
					hIcon=GetSmallIconFromWindow(hWnd);
					break;
				case LISTICON_BIG:
				case LISTICON_BIG48:
					hIcon=GetLargeIconFromWindow(hWnd);
					break;
				default:
					break;
			}
		}

		//デフォルトのアプリケーションアイコンを取得
		if(hIcon==NULL){
			hIcon=LoadIcon(NULL,IDI_APPLICATION);
		}
	}
	return hIcon;
}

//アイテムを追加
bool AddItem(LPWINDOW_INFO lpWindowInfo){
	bool bFirstTime=g_iWindowCount==0;
	WINDOW_INFO* pTmp=NULL;

	if(bFirstTime)pTmp=(WINDOW_INFO*)HeapAlloc(g_hHeapWindowInfo,HEAP_ZERO_MEMORY,(++g_iWindowCount)*sizeof(WINDOW_INFO));
	else pTmp=(WINDOW_INFO*)HeapReAlloc(g_hHeapWindowInfo,HEAP_ZERO_MEMORY,g_WindowInfo,(++g_iWindowCount)*sizeof(WINDOW_INFO));

	if(pTmp==NULL){
		--g_iWindowCount;
		return false;
	}

	g_iDisplayWindowCount=g_iWindowCount;
	g_WindowInfo=pTmp;

	LPWINDOW_INFO* ppTmp;

	if(bFirstTime)ppTmp=(LPWINDOW_INFO*)HeapAlloc(g_hHeapWindowInfo,HEAP_ZERO_MEMORY,(g_iDisplayWindowCount)*sizeof(LPWINDOW_INFO));
	else ppTmp=(LPWINDOW_INFO*)HeapReAlloc(g_hHeapWindowInfo,HEAP_ZERO_MEMORY,g_DisplayWindowInfo,(g_iDisplayWindowCount)*sizeof(LPWINDOW_INFO));

	if(ppTmp==NULL){
		--g_iDisplayWindowCount;
		g_iWindowCount=g_iDisplayWindowCount;

		if(g_iWindowCount==0){
			HeapFree(g_hHeapWindowInfo,0,g_WindowInfo);
			g_WindowInfo=NULL;
		}else{
			WINDOW_INFO* pTmp2=(WINDOW_INFO*)HeapReAlloc(g_hHeapWindowInfo,HEAP_ZERO_MEMORY,g_WindowInfo,(g_iWindowCount)*sizeof(WINDOW_INFO));
			if(pTmp2!=NULL){
				g_WindowInfo=pTmp2;
			}
		}
		return false;
	}

	g_WindowInfo[g_iWindowCount-1]=*lpWindowInfo;

	g_DisplayWindowInfo=ppTmp;
	return true;
}

//「デスクトップ」を追加
void AddDesktopItem(){
	WINDOW_INFO wiDesktop={};

	//「デスクトップ」をグローバルな構造体に追加
	wiDesktop.hIcon=(g_Config.ListView.iIcon>LISTICON_NO_SMALL)?GetDesktopIcon(g_Config.ListView.iIcon==LISTICON_SMALL):NULL;
	lstrcpy(wiDesktop.szFileName,_T("Desktop"));
	lstrcpy(wiDesktop.szWindowTitle,_T("デスクトップ"));
	wiDesktop.bDesktopItem=true;

	AddItem(&wiDesktop);
	return;
}

//「キャンセル」を追加
void AddCancelItem(){
	WINDOW_INFO wiCancel={};

	if(g_Config.ListView.iIcon>LISTICON_NO_SMALL){
		//shellapi.h
		typedef struct _SHSTOCKICONINFO{
			DWORD cbSize;
			HICON hIcon;
			int iSysImageIndex;
			int iIcon;
			WCHAR szPath[MAX_PATH];
		}SHSTOCKICONINFO;

		enum SHSTOCKICONID{SIID_ERROR=80,};

		HRESULT(WINAPI*pSHGetStockIconInfo)(SHSTOCKICONID,UINT,SHSTOCKICONINFO*)=
			(HRESULT(WINAPI*)(SHSTOCKICONID,UINT,SHSTOCKICONINFO*))GetProcAddress(GetModuleHandle(_T("shell32")),"SHGetStockIconInfo");
		if(pSHGetStockIconInfo!=NULL){
			SHSTOCKICONINFO shStockIconInfo={};
			shStockIconInfo.cbSize=sizeof(SHSTOCKICONINFO);
			//SHGSI_ICONLOCATION=0
			if(SUCCEEDED(pSHGetStockIconInfo(SIID_ERROR,0,&shStockIconInfo))){
				HMODULE hIconLib=LoadLibrary(shStockIconInfo.szPath);
				if(hIconLib!=NULL&&
				   shStockIconInfo.iIcon!=0){
					wiCancel.hIcon=(HICON)LoadImage(hIconLib,MAKEINTRESOURCE(-(shStockIconInfo.iIcon)),IMAGE_ICON,g_Config.GetListIconSize(),g_Config.GetListIconSize(),LR_DEFAULTCOLOR);
					FreeLibrary(hIconLib);
				}
			}
		}
		if(wiCancel.hIcon==NULL){
			wiCancel.hIcon=LoadIcon(NULL,IDI_ERROR);
		}
	}
	lstrcpy(wiCancel.szFileName,_T("Cancel"));
	lstrcpy(wiCancel.szWindowTitle,_T("キャンセル"));
	//「キャンセル」をグローバルな構造体に追加
	wiCancel.bCancelItem=true;

	AddItem(&wiCancel);
	return;
}

//ブラウザタブを追加
void AddWebBrowserTabItem(webbrowsertab::TAB_DATA* pTabData,const TCHAR* pszFilePath,int iWebBrowserIndex,DWORD dwProcessId){
	if(pTabData==NULL)return;
	WINDOW_INFO wiWebBrowserTab={};

	if(lstrlen(pTabData->pszFaviconURL)){
		IStream* pIStream=NULL;
		Gdiplus::Bitmap* pBitmap=NULL;
		if(SUCCEEDED(URLOpenBlockingStream(NULL,pTabData->pszFaviconURL,&pIStream,0,NULL))){
			pBitmap=Gdiplus::Bitmap::FromStream(pIStream);
			if(pBitmap!=NULL){
				pBitmap->GetHICON(&wiWebBrowserTab.hIcon);
			}
		}
	}
	if(wiWebBrowserTab.hIcon==NULL){
		PrivateExtractIcons(pszFilePath,0,g_Config.GetListIconSize(),g_Config.GetListIconSize(),&wiWebBrowserTab.hIcon,NULL,1,0);
	}

	lstrcpy(wiWebBrowserTab.szFileName,PathFindFileName(pszFilePath));
	PathRemoveExtension(wiWebBrowserTab.szFileName);
	lstrcpy(wiWebBrowserTab.szWindowTitle,pTabData->pszTitle);
	lstrcpy(wiWebBrowserTab.szFilePath,pszFilePath);
	lstrcpy(wiWebBrowserTab.szURL,pTabData->pszURL);
	lstrcpy(wiWebBrowserTab.szId,pTabData->pszId);
	wiWebBrowserTab.iWebBrowserIndex=iWebBrowserIndex;
	wiWebBrowserTab.dwProcessId=dwProcessId;
	wiWebBrowserTab.bWebBrowserTabItem=true;

	AddItem(&wiWebBrowserTab);
	return;
}

#ifdef CHECK_DISABLE_UWPAPP_ENUMCHILD
BOOL CALLBACK EnumChildCoreWindowsProc(HWND hWnd,LPARAM lParam){
	TCHAR szClassName[256]={};

	GetClassName(hWnd,szClassName,ARRAY_SIZEOF(szClassName));
	if(lstrcmp(szClassName,_T("Windows.UI.Core.CoreWindow"))==0){
		*(bool*)lParam=true;
		return FALSE;
	}
	return TRUE;
}
#endif

//列挙対象のウインドウである
bool IsWindowAvailable(HWND hWnd){
	RECT rc={};

	GetWindowRect(hWnd,&rc);

	if(GetWindowLongPtr(hWnd,GWL_STYLE)&WS_VISIBLE&&//可視状態
//		!(GetWindowLongPtr(hWnd,GWL_STYLE)&WS_DISABLED)&&//操作可能である
		!(GetWindowLongPtr(hWnd,GWL_EXSTYLE)&WS_EX_TOOLWINDOW)&&//ツールウインドウでない
		(rc.right-rc.left)&&(rc.bottom-rc.top)){//幅、高さが共に0でない(Delphi対策)
			if(GetWindowLongPtr(hWnd,GWL_EXSTYLE)&(WS_EX_NOREDIRECTIONBITMAP)){
				//非表示のUWPAppを除く
				bool bVisible=false;
#ifdef CHECK_DISABLE_UWPAPP_ENUMCHILD
				EnumChildWindows(hWnd,EnumChildCoreWindowsProc,(LPARAM)&bVisible);
#else
				if(dwm::IsLoaded()){
					BOOL bCloaked=FALSE;

					if(SUCCEEDED(dwm::GetWindowAttribute(hWnd,DWMWA_CLOAKED,&bCloaked,sizeof(BOOL)))){
						bVisible=!bCloaked;
					}
				}
#endif
				if(!bVisible)return false;
			}
			return true;
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
	MSG msg={};

	PeekMessage(&msg,NULL,0,0,PM_NOREMOVE);
	SetEvent(*(HANDLE*)lvParam);

	while(GetMessage(&msg,NULL,0,0)>0){
		WINDOW_INFO* pWindowInfo=(WINDOW_INFO*)msg.wParam;

		if(pWindowInfo==NULL){
			PostQuitMessage(0);
			break;
		}

		switch(msg.message){
			case WM_GETWINDOWINFO_TITLE:{
				TCHAR szWindowTitle[MAX_PATH]={};
				InternalGetWindowText(pWindowInfo->hWnd,szWindowTitle,MAX_PATH);
				lstrcpy(pWindowInfo->szWindowTitle,szWindowTitle);
				PostQuitMessage(0);
				break;
			}

			case WM_GETWINDOWINFO_ALL:{
				TCHAR szWindowTitle[MAX_PATH]={};
				InternalGetWindowText(pWindowInfo->hWnd,szWindowTitle,MAX_PATH);
				lstrcpy(pWindowInfo->szWindowTitle,szWindowTitle);

				if(!IsHungAppWindow(pWindowInfo->hWnd)){
					if(!g_Config.DirectWrite.bDirectWrite){
						if(!pWindowInfo->bUWPApp){
							pWindowInfo->hIcon=GetWindowIcon(pWindowInfo->hWnd,pWindowInfo->bWinUIDesktop);
						}else{
							pWindowInfo->hIcon=GetWindowIcon(pWindowInfo->hUICoreWnd,true);
						}
					}else{
						wic::Bitmap WicBitmap;

						if(!pWindowInfo->bUWPApp&&
						   !pWindowInfo->bWinUIDesktop){
							pWindowInfo->hIcon=GetWindowIcon(pWindowInfo->hWnd,false);
							if(pWindowInfo->hIcon!=NULL&&
							   g_pDirectWriteListView!=NULL){
								//アイコンをID2D1Bitmapに変換
								pWindowInfo->pD2DBitmap=WicBitmap.CreateID2D1BitmapFromHICON(g_pDirectWriteListView->GetRenderTarget(),pWindowInfo->hIcon);
								DestroyIcon(pWindowInfo->hIcon);
								pWindowInfo->hIcon=NULL;
							}
						}else if(g_pDirectWriteListView!=NULL){
							//HICONを経由せずに取得
							if(!GetUWPAppIconD2D1(g_pDirectWriteListView->GetRenderTarget(),
												  (pWindowInfo->bUWPApp)?pWindowInfo->hUICoreWnd:pWindowInfo->hWnd,
												  &pWindowInfo->pD2DBitmap,
												  (g_Config.ListView.bUWPAppNoBackgroundColor)?NULL:&pWindowInfo->bUWPAppBackground,
												  (g_Config.ListView.bUWPAppNoBackgroundColor)?NULL:&pWindowInfo->clrUWPAppBackground)){
								//取得に失敗した場合はIDI_APPLICATIONを使用
								if((pWindowInfo->hIcon=LoadIcon(NULL,IDI_APPLICATION))!=NULL){
									//アイコンをID2D1Bitmapに変換
									pWindowInfo->pD2DBitmap=WicBitmap.CreateID2D1BitmapFromHICON(g_pDirectWriteListView->GetRenderTarget(),pWindowInfo->hIcon);
									DestroyIcon(pWindowInfo->hIcon);
									pWindowInfo->hIcon=NULL;
								}
							}
						}
					}
				}
				PostQuitMessage(0);
				break;
			}
		}
	}

	ExitThread(0);
	return 0;
}

bool GetWindowInfo(WINDOW_INFO* pWindowInfo,HWND hWnd){
	if(pWindowInfo==NULL)return false;

	TCHAR szFilePath[MAX_PATH]={};
	TCHAR szClassName[256]={};
	HWND hUICoreWnd=NULL;

	GetClassName(hWnd,szClassName,ARRAY_SIZEOF(szClassName));

	//Tascher自身を列挙しないようにする
	TCHAR szTascherClassName[256]={};

	GetClassName(g_hMainWnd,szTascherClassName,ARRAY_SIZEOF(szTascherClassName));

	if(lstrcmp(szClassName,szTascherClassName)==0){
		return false;
	}

	if(lstrcmp(szClassName,_T("Windows.Internal.Shell.TabProxyWindow"))==0){
		//Microsoft Edgeのタブが列挙できるものの切り替え方法が分からないため弾く
		return false;
	}

	//ファイルパス取得
	GetFileNameFromWindowHandle(hWnd,szFilePath,MAX_PATH);

	if(lstrcmp(szClassName,_T("WinUIDesktopWin32WindowClass"))==0){
		pWindowInfo->bWinUIDesktop=true;
	}

	if(GetWindowLongPtr(hWnd,GWL_EXSTYLE)&(WS_EX_NOREDIRECTIONBITMAP)){
		hUICoreWnd=GetCoreWindow(hWnd);
		if(hUICoreWnd){
			pWindowInfo->bUWPApp=true;
			pWindowInfo->hUICoreWnd=hUICoreWnd;
			GetFileNameFromWindowHandle(hUICoreWnd,szFilePath,MAX_PATH);
		}
	}


	//ファイル名
	//ファイル名だけ取り出す
	lstrcpy(pWindowInfo->szFileName,PathFindFileName(szFilePath));
	//拡張子を除く
	PathRemoveExtension(pWindowInfo->szFileName);

	//アクティブモニタのウインドウか否か
	if(g_Config.MultiMonitor.bOnlyActiveMonitor){
		HMONITOR hMonitor=g_Config.GetActiveMonitor();
		if(hMonitor!=MonitorFromWindow(hWnd,MONITOR_DEFAULTTONEAREST)){
			return false;
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

			if(!bOnCurrent)return false;
		}
	}

	//除外対象のファイル名か否か
	if(lstrlen(g_Config.Exclude.szFileName)&&
	   IsExcludeFileName(pWindowInfo->szFileName)){
		return false;
	}

	//ファイルパス
	lstrcpy(pWindowInfo->szFilePath,szFilePath);

	//ウインドウハンドル
	pWindowInfo->hWnd=hWnd;

	//フォアグラウンドウインドウかどうか
	pWindowInfo->bForegorund=(hWnd==GetForegroundWindow());

	//最小化(アイコン化)されているかどうか
	pWindowInfo->bIconic=IsIconic(hWnd)!=0;

	//最前面ウインドウかどうか
	pWindowInfo->bTopMost=IsWindowTopMost(hWnd);

	return true;
}

//ウインドウ列挙のコールバック
//Zオーダー順であると決め打ちしています...
BOOL CALLBACK EnumWindowsProc(HWND hWnd,LPARAM lParam){
	WINDOW_INFO WindowInfo={};

	//AllocWindowListFromApplicationView()の時はlParam=trueとしてIsWindowAvailable()をスキップする
	if(lParam||
	   IsWindowAvailable(hWnd)&&!GetWindow(hWnd,GW_OWNER)&&!GetParent(hWnd)){
		if(!GetWindowInfo(&WindowInfo,hWnd))return TRUE;
		if(!AddItem(&WindowInfo))return TRUE;
	}

	return TRUE;
}

//ツールチップを表示/非表示
void ShowInfoToolTip(TCHAR* szText=NULL){
	TOOLINFO ti={};
	ti.cbSize=sizeof(TOOLINFO);
	TCHAR* pszText=NULL;

	if(szText!=NULL&&szText[0]!='\0'){
		pszText=(TCHAR*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(lstrlen(szText)+12)*sizeof(TCHAR));
		ti.lpszText=szText;
	}

	if(pszText!=NULL){
		RECT rcListView={};

		GetWindowRect(g_hListView,&rcListView);

		//ツールチップ文字列更新
		SendMessage(g_hInfoToolTip,TTM_UPDATETIPTEXT,0,(LPARAM)&ti);
		//ツールチップ表示位置
		SendMessage(g_hInfoToolTip,TTM_TRACKPOSITION,0,(LPARAM)MAKELONG(rcListView.left+3,rcListView.top+3));
		//改行できるように幅を設定
		SendMessage(g_hInfoToolTip,TTM_SETMAXTIPWIDTH,(WPARAM)0,(LPARAM)0);
		//ツールチップを表示
		SendMessage(g_hInfoToolTip,TTM_TRACKACTIVATE,true,(LPARAM)&ti);

		HeapFree(GetProcessHeap(),0,pszText);
		pszText=NULL;
	}else{
		//ツールチップ非表示
		SendMessage(g_hInfoToolTip,TTM_TRACKACTIVATE,false,(LPARAM)&ti);
	}
}

//インクリメンタルサーチウインドウ表示/非表示
void ShowSearchWnd(TCHAR* szText=NULL){
	HeapFree(GetProcessHeap(),0,g_pszDisplaySearchString);
	g_pszDisplaySearchString=NULL;

	if(szText!=NULL&&szText[0]!='\0'){
		int iSearchStringCount=0;
		TCHAR* p=GetCurrentSearchWindowDesign()->szTextFormat;

		while((p=StrStr(p,_T("%S")))!=NULL){
			iSearchStringCount++;
			p++;
		}

		g_pszDisplaySearchString=(TCHAR*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(lstrlen(szText)*iSearchStringCount+lstrlen(GetCurrentSearchWindowDesign()->szTextFormat)+12)*sizeof(TCHAR));

		lstrcpy(g_pszDisplaySearchString,GetCurrentSearchWindowDesign()->szTextFormat);
		do{
			StrReplace(g_pszDisplaySearchString,_T("%S"),szText);
		}while(StrStr(g_pszDisplaySearchString,_T("%S")));
	}

	if(g_pszDisplaySearchString!=NULL){
		RECT rc={};

		GetWindowRect(g_hListView,&rc);

		HDC hDC=GetDC(g_hSearchWnd);
		SIZE size;
		HFONT hOldFont=(HFONT)SelectObject(hDC,g_hSearchWindowFonts[GetCurrentIncrementalSearchState()]);

		GetTextExtentPoint32(hDC,g_pszDisplaySearchString,lstrlen(g_pszDisplaySearchString),&size);

		int iWidth=g_Config.IncrementalSearch.iMargin+size.cx+g_Config.IncrementalSearch.iMargin;

		if(iWidth>rc.right-rc.left)iWidth=rc.right-rc.left;
		SetWindowPos(g_hSearchWnd,
					 HWND_TOPMOST,
					 rc.left,rc.top,
					 iWidth,
					 g_Config.IncrementalSearch.iMargin+size.cy+g_Config.IncrementalSearch.iMargin,
					 SWP_NOACTIVATE|SWP_NOSENDCHANGING|SWP_SHOWWINDOW|SWP_NOCOPYBITS);
		SelectObject(hDC,hOldFont);
		ReleaseDC(g_hSearchWnd,hDC);
	}else{
		ShowWindow(g_hSearchWnd,SW_HIDE);
	}
}

//サムネイルを表示/非表示する
void ShowThumbnail(bool bShow=true){
	KillTimer(g_hThumbnailWnd,MAKEWPARAM(ID_TIMER_THUMBNAIL,0));

	if(g_Config.ListView.iThumbnailDelay||!bShow){
		for(int i=0;i<g_iWindowCount;++i){
			if(g_WindowInfo[i].hThumbnail){
				dwm::UnregisterThumbnail(g_WindowInfo[i].hThumbnail);
				g_WindowInfo[i].hThumbnail=NULL;
			}
		}
	}

	if(!bShow||!tempconf::uThumbnail){
		ShowWindow(g_hThumbnailWnd,SW_HIDE);
		return;
	}


	if(dwm::IsLoaded()){
		SetTimer(g_hThumbnailWnd,
				 MAKEWPARAM(ID_TIMER_THUMBNAIL,0),
				 g_Config.ListView.iThumbnailDelay,
				 NULL);
	}
	return;
}

//リストビューをリサイズ
void ResizeListView(){
	RECT rc={};
	int iWidth=0;
	int iHeight=0;
	int iItemHeight=0;

	iWidth=ListView_GetAllColumnWidth(g_ListViewColumn_Table);
	if(iWidth==0){
		g_Config.ListView.iIconWidth=g_Config.GetListIconSize()+g_Config.ListView.iIconMargin*2;
		g_Config.ListView.iFileNameWidth=DEFAULT_FILENAME_WIDTH;
		g_Config.ListView.iWindowTitleWidth=DEFAULT_WINDOWTITLE_WIDTH;

		ListView_InitColumn(g_ListViewColumn_Table,&g_Config,g_hListView);
		iWidth=ListView_GetAllColumnWidth(g_ListViewColumn_Table);
	}

	iWidth=ScaleWidth(iWidth);

	if(iWidth>GetSystemMetrics(SM_CXSCREEN)){
		iWidth=GetSystemMetrics(SM_CXSCREEN);
	}

	//一行分の高さを取得
	ListView_GetItemRect(g_hListView,0,&rc,LVIR_BOUNDS);
	iItemHeight=rc.bottom-rc.top;

	iHeight=iItemHeight*g_iDisplayWindowCount;

	//モニタの高さを取得
	HMONITOR hMonitor=g_Config.GetActiveMonitor();
	MONITORINFO info={};
	info.cbSize=sizeof(MONITORINFO);
	GetMonitorInfo(hMonitor,&info);
	int iMonitorHeight=info.rcWork.bottom-info.rcWork.top;

	//アイテム3行分のスペースを空けておく
	if(iHeight>iMonitorHeight-iItemHeight*3){
		int iDiff=iMonitorHeight-iItemHeight*3;

		if(status::eWindowList==status::WINDOWLIST::DISPLAYED){
			RECT rcMainWnd={};
			GetWindowRect(g_hMainWnd,&rcMainWnd);
			iDiff=iMonitorHeight-rcMainWnd.top-iItemHeight*3/2;
		}

		//一行分の高さの倍数に切り下げ
		iHeight=(iDiff%iItemHeight)?((iDiff-1)/iItemHeight)*iItemHeight:iDiff;
	}

	RECT rcListView={};
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

	if(g_Config.DirectWrite.bDirectWrite&&
	   g_pDirectWriteListView!=NULL&&
	   g_pDirectWriteListView->IsAvailable()){
		g_pDirectWriteListView->GetRenderTarget()->Resize(D2D1::SizeU(iWidth,iHeight));
	}

	if(!status::bBackground){
		ListView_RedrawItems(g_hListView,0,g_iDisplayWindowCount);
		ShowThumbnail();
		return;
	}

	//オリジナルの背景画像からリストビュー表示分だけ切り出す
	//Direct2Dモードでは描画時に切り出す
	if(!g_Config.DirectWrite.bDirectWrite||
	   g_pDirectWriteListView!=NULL&&!g_pDirectWriteListView->IsAvailable()&&
	   g_pGdiplusBackgroundImage!=NULL){
		g_pGdiplusBackgroundImage->CreateBitmap(iWidth,iHeight,
											  g_Config.Background.iXOffset,
											  g_Config.Background.iYOffset,
											  g_Config.Background.byResizePercent,
											  g_Config.Background.byOpacity);
	}

	//再描画
	ListView_RedrawItems(g_hListView,0,g_iDisplayWindowCount);
	ShowThumbnail();
}

//リストビューを非表示
void HideListView(){
	KillTimer(g_hListView,ID_TIMER_HIDELISTVIEW);
	static LONG lInterlockedFlag;
	if(status::eWindowList==status::WINDOWLIST::HIDE)return;

	if(InterlockedCompareExchange(&lInterlockedFlag,TRUE,FALSE)==FALSE){
		if(status::eWindowList==status::WINDOWLIST::HIDE)return;

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

		//インクリメンタルサーチウインドウ非表示
		ShowSearchWnd();

		//ツールチップ非表示
		ShowInfoToolTip();

		for(int i=0;i<g_iWindowCount;i++){
			DWORD dwExitCode=0;

			GetExitCodeThread(g_WindowInfo[i].Thread.hThread,&dwExitCode);
			if(dwExitCode==STILL_ACTIVE){
				PostThreadMessage(g_WindowInfo[i].Thread.dwThreadId,WM_QUIT,0,0);

				if(WaitForSingleObject(g_WindowInfo[i].Thread.hThread,THREAD_TIMEOUT)==WAIT_TIMEOUT){
					TerminateThread(g_WindowInfo[i].Thread.hThread,0);
				}
				CloseHandle(g_WindowInfo[i].Thread.hThread);
			}
		}

		if(g_Config.ListView.byOpacity!=0&&
		   g_Config.ListView.byOpacity!=100){
			//ウインドウを透明に
//			SetLayeredWindowAttributes(g_hMainWnd,0,0,LWA_ALPHA);
		}

		status::bHotKeyMode=false;
		status::byModKey=0;

		//アイコン解放
		for(int i=0;i<g_iWindowCount;++i){
			DestroyIcon(g_WindowInfo[i].hIcon);
			g_WindowInfo[i].hIcon=NULL;
			SAFE_RELEASE(g_WindowInfo[i].pD2DBitmap);
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

		for(UINT i=1;i<=MAX_WEBBROWSER;i++){
			g_pWebBrowserTab[i].Uninitialize();
		}

		if(g_Config.ListView.byOpacity!=0&&
		   g_Config.ListView.byOpacity!=100){
			//不透明度を元に戻す
//			SetLayeredWindowAttributes(g_hMainWnd,0,g_Config.ListView.byOpacity*255/100,LWA_ALPHA);
		}
		status::eWindowList=status::WINDOWLIST::HIDE;

		if(!g_Config.IncrementalSearch.bNoUnloadMigemo)migemo::Unload();

		if(g_pGdiplusBackgroundImage!=NULL){
			//Bitmapを削除
			g_pGdiplusBackgroundImage->ReleaseBitmap();
			g_pGdiplusBackgroundImage->ReleaseBitmapBackup();
		}

		InterlockedExchange(&lInterlockedFlag,FALSE);
	}
	return;
}

BOOL CALLBACK EnumOwnedWindowsProc(HWND hWnd,LPARAM lParam){
	if(GetWindow(hWnd,GW_OWNER)==*(HWND*)lParam&&
	   IsWindowAvailable(hWnd)){
		*(HWND*)lParam=hWnd;
		return FALSE;
	}
	return TRUE;
}

//ウインドウの切り替えスレッド
DWORD WINAPI TaskSwitchThread(LPVOID lvParam){
	HWND hWnd=*(HWND*)lvParam;

	//モーダル/モードレスダイアログを選択するため指定ウインドウがオーナーウインドウとなっているウインドウを検索
	EnumWindows(EnumOwnedWindowsProc,(LPARAM)&hWnd);

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

	status::WINDOWLIST eWindowListStatus=status::eWindowList;

	//WM_ACTIVATEAPP対策
	status::eWindowList=status::WINDOWLIST::SWITCHING;

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
	}else if(lpWindowInfo->bWebBrowserTabItem){
		//ブラウザタブ
		g_pWebBrowserTab[lpWindowInfo->iWebBrowserIndex].Activate(lpWindowInfo->szId);
	}else{
		//通常のウインドウ
		if(!IsHungAppWindow(lpWindowInfo->hWnd)){
			SetForegroundWindowEx(lpWindowInfo);
		}
	}
	status::eWindowList=eWindowListStatus;

	HideListView();
	return;
}

//リストビューを表示
void ShowListView(SCC_CORNERS eCorners=SCC_CENTER){
	if(status::eWindowList==status::WINDOWLIST::CREATING)return;

	if(status::eWindowList==status::WINDOWLIST::DISPLAYED){
		//表示済みであれば非表示に
		HideListView();
		return;
	}

	if(g_DisplayWindowInfo!=NULL||
	   g_WindowInfo!=NULL){
		return;
	}

	status::eCorners=eCorners;

	HWND hForegroundWnd=GetForegroundWindow();

	if((g_Config.ShowWindow.bMouseDisableInFullScreenMode&&!status::bHotKeyMode||
		g_Config.ShowWindow.bHotKeyDisableInFullScreenMode&&status::bHotKeyMode)&&
	   IsFullScreenWindow(GetForegroundWindow())){
		//フルスクリーン時は無効
		return;
	}

	//ウインドウの列挙
	HWND* phWindowList=AllocWindowListFromApplicationView();
	if(phWindowList){
		HWND* p=phWindowList;
		do{
			EnumWindowsProc(*p,true);
		}while(*++p);
		FreeWindowListFromApplicationView(&phWindowList);
		status::bEnumWindows=false;
	}else{
		status::bEnumWindows=true;
		//ApplicationViewから取得できなければ従来通りEnumWindows()で取得する
		EnumWindows((WNDENUMPROC)EnumWindowsProc,0);
	}

	if(g_iWindowCount){
		status::eWindowList=status::WINDOWLIST::CREATING;

		for(int i=1;i<MAX_WEBBROWSER;i++){
			if(lstrlen(g_Config.WebBrowser[i].szFilePath)||
			   g_Config.WebBrowser[i].iPort>=0){
				int iCount=0;
				if(g_pWebBrowserTab[i].Initialize(g_Config.WebBrowser[i].szFilePath,
											   (g_Config.WebBrowser[i].iPort>=0)?(DWORD*)&g_Config.WebBrowser[i].iPort:NULL)){
					DWORD dwProcessId=0;
					webbrowsertab::TAB_DATA* data=g_pWebBrowserTab[i].Alloc(&iCount,&dwProcessId);
					TCHAR szFilePath[MAX_PATH]={};
					GetProcessFileName(dwProcessId,szFilePath,MAX_PATH);
					for(int ii=0;ii<iCount;ii++){
						AddWebBrowserTabItem(&data[ii],szFilePath,i,dwProcessId);
					}
					g_pWebBrowserTab[i].Free(&data);
				}
			}
		}

		if(g_Config.ListView.bDesktopItem){
			//「デスクトップ」を追加
			AddDesktopItem();
		}

		if(g_Config.ListView.bCancelItem){
			//「キャンセル」を追加
			AddCancelItem();
		}

		//一時的な設定をリセット
		tempconf::Reset();

		for(int i=0;i<g_iWindowCount;++i){
			if(!g_WindowInfo[i].bDesktopItem&&
			   !g_WindowInfo[i].bCancelItem&&
			   !g_WindowInfo[i].bWebBrowserTabItem){
				HANDLE hInitEvent=CreateEvent(NULL,true,false,NULL);

				g_WindowInfo[i].Thread.hThread=CreateThread(NULL,0,GetWindowInfoThread,&hInitEvent,0,&g_WindowInfo[i].Thread.dwThreadId);
				WaitForSingleObject(hInitEvent,INFINITE);
				CloseHandle(hInitEvent);

				if(g_Config.ListView.iIcon<=LISTICON_NO_SMALL){
					//ウインドウタイトルのみ取得
					PostThreadMessage(g_WindowInfo[i].Thread.dwThreadId,WM_GETWINDOWINFO_TITLE,(WPARAM)&g_WindowInfo[i],0);
				}else{
					//アイコン+ウインドウタイトルを取得
					PostThreadMessage(g_WindowInfo[i].Thread.dwThreadId,WM_GETWINDOWINFO_ALL,(WPARAM)&g_WindowInfo[i],0);
				}
			}
		}

		//設定に合わせてウインドウ/ウェブタブブラウザをフィルタ
		FilterItemType();

		//アイテム数を設定
		ListView_SetItemCountEx(g_hListView,g_iDisplayWindowCount,LVSICF_NOINVALIDATEALL|LVSICF_NOSCROLL);

		//フィルタの結果アイテム数が0なら表示しない
		if(!g_iDisplayWindowCount&&
		   !g_Config.ListView.bDesktopItem&&
		   !g_Config.ListView.bCancelItem)return;

		ResizeListView();

		//最前面だったウインドウを選択する
		//bSelectSecondWindowが有効であれば、2番目のウインドウを選択する
		//EnumWindows()で取得した場合、WS_EX_TOPMOSTかつ最小化されているものはスキップする
		if(!g_Config.ListView.bSelectSecondWindow||g_iWindowCount==1){
			ListView_SelectItem(g_hListView,0);
		}else{
			for(int i=1;i<g_iWindowCount;++i){
				//「デスクトップ」か「キャンセル」であればスキップ
				if(g_WindowInfo[i].bDesktopItem||g_WindowInfo[i].bCancelItem)continue;
				if(status::bEnumWindows){
					//WS_EX_TOPMOST/SW_MINIMIZEであればスキップ
					if(g_WindowInfo[i].bTopMost&&g_WindowInfo[i].bIconic)continue;
					//フォアグラウンドウインドウであればスキップ
					if(g_WindowInfo[i].bForegorund)continue;
				}
				ListView_SelectItem(g_hListView,i);
				break;
			}
			if(ListView_GetSelectedItem(g_hListView)==-1){
				ListView_SelectItem(g_hListView,0);
			}
		}

		if(g_Config.IncrementalSearch.iMigemoMode!=MIGEMO_NO&&
		   !g_Config.IncrementalSearch.bNoUnloadMigemo)migemo::Load();

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

		RECT rcMainWnd={};

		GetWindowRect(g_hMainWnd,&rcMainWnd);

		if(g_Config.ListView.iHideCursorTime==0){
			//カーソルちらつき防止のため0設定ならここで処理
			SetCursor(g_hBlankCursor);
			SetClassLongPtr(g_hListView,GCLP_HCURSOR,(LONG_PTR)g_hBlankCursor);
		}

		for(int i=0;i<g_iWindowCount;i++){
			//ウインドウタイトル/アイコン取得用スレッド終了
			DWORD dwExitCode=0;

			GetExitCodeThread(g_WindowInfo[i].Thread.hThread,&dwExitCode);
			if(dwExitCode==STILL_ACTIVE){
				PostThreadMessage(g_WindowInfo[i].Thread.dwThreadId,WM_QUIT,0,0);

				if(WaitForSingleObject(g_WindowInfo[i].Thread.hThread,THREAD_TIMEOUT)==WAIT_TIMEOUT){
					TerminateThread(g_WindowInfo[i].Thread.hThread,0);
				}
				CloseHandle(g_WindowInfo[i].Thread.hThread);
			}
		}

		ShowWindow(g_hMainWnd,SW_SHOWNORMAL);

		//1000回チャレンジ！
		//システムメニューやコンテキストメニューが表示されている場合等は失敗するため、
		//SendInput(VK_ESCAPE)で対応する
		//エクスプローラのシステムメニューはキャンセルしても一度復活するため、
		//WM_SETFOCUSでのSendInput(MOUSEEVENTF_LEFTDOWN)で対応する
		//その他SetForegroundWindow()で切り替えできない場合もSendInput(MOUSEEVENTF_LEFTDOWN)対応となる
		for(int i=0;i<1000;i++){
			SetForegroundWindow(g_hMainWnd);
			if(GetForegroundWindow()==g_hMainWnd)break;
		}

		if(GetForegroundWindow()!=g_hMainWnd&&
		   !IsHungAppWindow(GetForegroundWindow())){

			HWND hMenuWnd=FindWindow(_T("#32768"),NULL);
			if(hMenuWnd){
				//メニュー表示をキャンセル
				SetForegroundWindow(hMenuWnd);

				INPUT data[]={
					{INPUT_KEYBOARD,0,VK_ESCAPE,0,0,0,0},
					{INPUT_KEYBOARD,0,VK_ESCAPE,0,KEYEVENTF_KEYUP,0,0},
				};

				for(int i=0;i<10;i++){
					if(IsWindowVisible(hMenuWnd)){
						SetForegroundWindow(hMenuWnd);
						SendInput(2,data,sizeof(INPUT));
					}

					if(!IsWindowVisible(hMenuWnd)){
						SetForegroundWindow(g_hMainWnd);
						break;
					}
				}
				for(int i=0;i<1000;i++){
					SetForegroundWindow(g_hMainWnd);
					if(GetForegroundWindow()==g_hMainWnd)break;
				}
			}
		}

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
									(!status::bHotKeyMode)?g_Config.ShowWindow.iMouseCursorMarginHorizontal:g_Config.ShowWindow.iHotKeyCursorMarginHorizontal,
									(!status::bHotKeyMode)?g_Config.ShowWindow.iMouseCursorMarginVertical:g_Config.ShowWindow.iHotKeyCursorMarginVertical);

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
		   (!status::bHotKeyMode&&g_Config.ShowWindow.bMouseMoveCursorSelectedWindow||
			status::bHotKeyMode&&g_Config.ShowWindow.bHotKeyMoveCursorSelectedWindow)&&
		   g_Config.ShowWindow.bMoveCursorAtStart){
			//「選択項目にカーソルを移動」が有効なら表示時に移動
			POINT ptOrig={};

			GetCursorPos(&ptOrig);
			RECT rc={};

			ListView_GetSubItemRect(g_hListView,ListView_GetSelectedItem(g_hListView),
									g_ListViewColumn_Table[0].bVisible&&g_ListViewColumn_Table[1].bVisible,
									LVIR_BOUNDS,&rc);

			int iListHeight=rc.bottom-rc.top;

			POINT pt={0,0};

			if(iListHeight<=((!status::bHotKeyMode)?g_Config.ShowWindow.iMouseCursorMarginVertical:g_Config.ShowWindow.iHotKeyCursorMarginVertical)){
				pt.x=rc.left;
				pt.y=rc.top;
				ClientToScreen(g_hListView,&pt);
				SetCursorPos(ptOrig.x,pt.y+iListHeight/2);
			}else{
				GetCursorCornerPos(&pt,
								   eCorners,
								   &rc,
								   0,
								   (!status::bHotKeyMode)?g_Config.ShowWindow.iMouseCursorMarginVertical:g_Config.ShowWindow.iHotKeyCursorMarginVertical);
				ClientToScreen(g_hListView,&pt);
				SetCursorPos(ptOrig.x,pt.y);
			}
		}

		status::eWindowList=status::WINDOWLIST::CREATED;

		//最前面表示
		SetWindowPos(g_hMainWnd,
					 HWND_TOPMOST,
					 0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		//親に送らないとWM_ACTIVATEAPPが使えない?
		SetFocus(g_hMainWnd);
	}
	return;
}

bool DrawItem(PAINTSTRUCT* pps){
	if(!g_iDisplayWindowCount)return false;
	if(status::eWindowList==status::WINDOWLIST::HIDE)return false;
	if(g_pDirectWriteListView==NULL)return false;
	if(g_pDirectWriteListView->GetRenderTarget()==NULL)return false;

	int iOrders[LISTITEM_NUM];
	ListView_GetColumnOrderArray(g_hListView,LISTITEM_NUM,&iOrders);

	float fDpiX=0,fDpiY=0;
	g_pDirectWriteListView->GetDesktopDpi(&fDpiX,&fDpiY);

	wic::Bitmap WicBitmap;

	//描画開始
	g_pDirectWriteListView->GetRenderTarget()->BeginDraw();

	for(int iItemID=ListView_GetTopIndex(g_hListView);iItemID<g_iDisplayWindowCount;++iItemID){
		if(iItemID+1>g_iDisplayWindowCount||
		   g_DisplayWindowInfo[iItemID]==NULL)break;

		bool bSelectedItem=ListView_GetSelectedItem(g_hListView)==iItemID;
		RECT rcItem={};
		ListView_GetSubItemRect(g_hListView,iItemID,0,LVIR_BOUNDS,&rcItem);

		if(pps!=NULL){
			//更新対象の領域がどうか
			RECT rcDummy={};

			if(rcItem.top<pps->rcPaint.top||
			   rcItem.bottom>pps->rcPaint.bottom)continue;
		}

		//背景色描画
		if(bSelectedItem){
			g_pDirectWriteListView->DrawSelectedTextBackground(D2D1::RectF((float)rcItem.left*(96.0f/fDpiX),(float)rcItem.top*(96.0f/fDpiY),(float)rcItem.right*(96.0f/fDpiX),(float)rcItem.bottom*(96.0f/fDpiY)));
		}else{
			g_pDirectWriteListView->DrawDefaultTextBackground(D2D1::RectF((float)rcItem.left*(96.0f/fDpiX),(float)rcItem.top*(96.0f/fDpiY),(float)rcItem.right*(96.0f/fDpiX),(float)rcItem.bottom*(96.0f/fDpiY)));
		}

		if(g_pDirectWriteListView->IsLoadedBackgroundImage()){
			//背景画像描画
			g_pDirectWriteListView->DrawBackground(D2D1::RectF((float)rcItem.left*(96.0f/fDpiX),(float)rcItem.top*(96.0f/fDpiY),(float)rcItem.right*(96.0f/fDpiX),(float)rcItem.bottom*(96.0f/fDpiY)),
											 g_Config.Background.iXOffset,g_Config.Background.iYOffset);
		}

		for(int iItem=0;iItem<Header_GetItemCount(ListView_GetHeader(g_hListView));iItem++){
			if(!ListView_GetColumnWidth(g_hListView,iItem))continue;

			//文字列表示領域のサイズを取得
			RECT rcTmp={};
			ListView_GetSubItemRect(g_hListView,iItemID,iItem,LVIR_LABEL,&rcTmp);

			D2D1_RECT_F rcItemLabel=D2D1::RectF((float)rcTmp.left*(96.0f/fDpiX),
												(float)rcTmp.top*(96.0f/fDpiY),
												(float)rcTmp.right*(96.0f/fDpiX),
												(float)rcTmp.bottom*(96.0f/fDpiY));

			switch(iItem){
				case LISTITEM_ICON:
					if(g_Config.ListView.iIcon<=LISTICON_NO_SMALL)break;

					if(g_DisplayWindowInfo[iItemID]->hIcon!=NULL&&
					   g_DisplayWindowInfo[iItemID]->pD2DBitmap==NULL){
						//未変換であればアイコンをID2D1Bitmapに変換
						g_DisplayWindowInfo[iItemID]->pD2DBitmap=WicBitmap.CreateID2D1BitmapFromHICON(g_pDirectWriteListView->GetRenderTarget(),g_DisplayWindowInfo[iItemID]->hIcon);
						DestroyIcon(g_DisplayWindowInfo[iItemID]->hIcon);
						g_DisplayWindowInfo[iItemID]->hIcon=NULL;
					}

					if(g_DisplayWindowInfo[iItemID]->pD2DBitmap){
						//アイコンを描写する
						RECT rcIconTmp={};

						ListView_GetItemRect(g_hListView,iItemID,&rcIconTmp,LVIR_ICON);
						OffsetRect(&rcIconTmp,-rcIconTmp.left,0);

						D2D1_RECT_F rcIcon=D2D1::RectF((float)rcIconTmp.left*(96.0f/fDpiX),
													   (float)rcIconTmp.top*(96.0f/fDpiY),
													   (float)rcIconTmp.right*(96.0f/fDpiX),
													   (float)rcIconTmp.bottom*(96.0f/fDpiY));

						if(g_DisplayWindowInfo[iItemID]->pD2DBitmap!=NULL){
							float fIconSize=(float)g_Config.GetListIconSize()*(96.0f/fDpiX);
							float padding=rcIcon.left+(rcIcon.right-rcIcon.left-fIconSize)/2;

							if(!g_Config.ListView.bUWPAppNoBackgroundColor&&
							   g_DisplayWindowInfo[iItemID]->bUWPAppBackground){
								//UWPアプリ背景描画
								ID2D1SolidColorBrush* pUWPAppNoBackgroundBrush=NULL;
								if(SUCCEEDED(g_pDirectWriteListView->GetRenderTarget()->CreateSolidColorBrush(D2D1::ColorF(GetRValue(g_DisplayWindowInfo[iItemID]->clrUWPAppBackground)/255.0f,GetGValue(g_DisplayWindowInfo[iItemID]->clrUWPAppBackground)/255.0f,GetBValue(g_DisplayWindowInfo[iItemID]->clrUWPAppBackground)/255.0f,1.0f),&pUWPAppNoBackgroundBrush))){
									g_pDirectWriteListView->GetRenderTarget()->PushAxisAlignedClip(D2D1::RectF(padding,rcIcon.top+(rcIcon.bottom-rcIcon.top-fIconSize)/2,fIconSize+padding,fIconSize+rcIcon.top+(rcIcon.bottom-rcIcon.top-fIconSize)/2),
																	   D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
									g_pDirectWriteListView->GetRenderTarget()->FillRectangle(D2D1::RectF(padding,rcIcon.top+(rcIcon.bottom-rcIcon.top-fIconSize)/2,fIconSize+padding,fIconSize+rcIcon.top+(rcIcon.bottom-rcIcon.top-fIconSize)/2),
																 pUWPAppNoBackgroundBrush);
									g_pDirectWriteListView->GetRenderTarget()->PopAxisAlignedClip();
								}
								SAFE_RELEASE(pUWPAppNoBackgroundBrush);
							}

							//アイコン描画
							//intでキャストしておかないと、Windows7環境でアイコンが一部ギザギザになってしまう
							//D2D1_BITMAP_INTERPOLATION_MODE_LINEAR指定でも回避できるが、ぼやけた描画になるため、キャストで回避する
							g_pDirectWriteListView->GetRenderTarget()->DrawBitmap(g_DisplayWindowInfo[iItemID]->pD2DBitmap,
													  D2D1::RectF(padding,rcIcon.top+(int)(rcIcon.bottom-rcIcon.top-fIconSize)/2,fIconSize+padding,fIconSize+rcIcon.top+(int)(rcIcon.bottom-rcIcon.top-fIconSize)/2),
													  1.0f,
													  D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
													  D2D1::RectF(0,0,g_DisplayWindowInfo[iItemID]->pD2DBitmap->GetSize().width,g_DisplayWindowInfo[iItemID]->pD2DBitmap->GetSize().height));
						}
					}
					break;

				case LISTITEM_FILENAME:
					if(bSelectedItem){
						g_pDirectWriteListView->DrawSelectedText(g_DisplayWindowInfo[iItemID]->szFileName,
															   lstrlen(g_DisplayWindowInfo[iItemID]->szFileName),
															   &rcItemLabel);
					}else{
						g_pDirectWriteListView->DrawDefaultText(g_DisplayWindowInfo[iItemID]->szFileName,
															  lstrlen(g_DisplayWindowInfo[iItemID]->szFileName),
															  &rcItemLabel);
					}
					break;

				case LISTITEM_WINDOWTITLE:
					if(bSelectedItem){
						g_pDirectWriteListView->DrawSelectedText(g_DisplayWindowInfo[iItemID]->szWindowTitle,
															   lstrlen(g_DisplayWindowInfo[iItemID]->szWindowTitle),
															   &rcItemLabel);
					}else{
						g_pDirectWriteListView->DrawDefaultText(g_DisplayWindowInfo[iItemID]->szWindowTitle,
															  lstrlen(g_DisplayWindowInfo[iItemID]->szWindowTitle),
															  &rcItemLabel);
					}
					break;

				default:
					break;
			}
		}
	}

	if(g_Config.ListView.bDialogFrame){
		RECT rcClient={};

		GetClientRect(g_hListView,&rcClient);
		g_pDirectWriteListView->DrawDialogFrame(D2D1::RectF((float)rcClient.left*(96.0f/fDpiX),(float)rcClient.top*(96.0f/fDpiY),(float)rcClient.right*(96.0f/fDpiX),(float)rcClient.bottom*(96.0f/fDpiY)),(float)2*(96.0f/fDpiX));
	}

	//描画終了
	if(g_pDirectWriteListView->GetRenderTarget()->EndDraw()==D2DERR_RECREATE_TARGET){
		//ターゲットをロストしたため再作成
		//※Windows7環境で解像度変更(フルスクリーンのゲームを起動等)することで発生を確認
		//発生するとウインドウリスト自体が表示されなくなるが、再表示で元に戻る
		g_pDirectWriteListView->CreateRenderTarget(g_hListView,0,0);
		ReleaseBrush();
		CreateBrush();

		//壁紙設定
		if(lstrlen(g_Config.Background.szImagePath)&&
		   g_Config.DirectWrite.bDirectWrite&&
		   g_pDirectWriteListView!=NULL&&
		   g_pDirectWriteListView->IsAvailable()){
			g_pDirectWriteListView->LoadBackgroundImage(g_Config.Background.szImagePath,
														g_Config.Background.byResizePercent,
														g_Config.Background.byOpacity);
			status::bBackground=g_pDirectWriteListView->IsLoadedBackgroundImage();
		}
	}
	return true;
}

//リストビューアイテムを描画
bool DrawItemGdiPlus(LPDRAWITEMSTRUCT lpDraw){
	HDC hDC=lpDraw->hDC;
	HWND hListView=lpDraw->hwndItem;
	UINT uItemID=lpDraw->itemID;

	if(lpDraw->CtlType!=ODT_LISTVIEW)return false;
	if(!g_iDisplayWindowCount||
	   int(uItemID+1)>g_iDisplayWindowCount||
	   g_DisplayWindowInfo[uItemID]==NULL)return false;

	int iOrders[LISTITEM_NUM];
	ListView_GetColumnOrderArray(hListView,LISTITEM_NUM,&iOrders);

	//背景を塗りつぶす
	RECT rcItem={};
	ListView_GetItemRect(hListView,uItemID,&rcItem,LVIR_BOUNDS);

	if(lpDraw->itemState&ODS_SELECTED){
		HorizontalGradient(hDC,rcItem,g_Config.SelectedItemDesign.clrTextBk,g_Config.SelectedItemDesign.clrTextBkGradientEnd);
	}else{
		HorizontalGradient(hDC,rcItem,g_Config.DefaultItemDesign.clrTextBk,g_Config.DefaultItemDesign.clrTextBkGradientEnd);
	}

	if(status::bBackground&&
	   g_pGdiplusBackgroundImage!=NULL){
		//背景を描画
		g_pGdiplusBackgroundImage->DrawImage(hDC,rcItem);
	}

	for(int iItem=0;iItem<Header_GetItemCount(ListView_GetHeader(hListView));iItem++){
		if(!ListView_GetColumnWidth(hListView,iItem))continue;

		HFONT hOldFont=NULL;

		if(lpDraw->itemState&ODS_SELECTED){
			//選択行
			//フォント
			hOldFont=(HFONT)SelectObject(hDC,g_hSelectedItemFont);
			//文字色
			SetTextColor(hDC,g_Config.SelectedItemDesign.clrText);
		}else{
			//通常行
			//フォント
			hOldFont=(HFONT)SelectObject(hDC,g_hDefaultItemFont);
			//文字色
			SetTextColor(hDC,g_Config.DefaultItemDesign.clrText);
		}

		//文字列表示領域のサイズを取得
		RECT rcItemLabel={};
		ListView_GetSubItemRect(hListView,uItemID,iItem,LVIR_LABEL,&rcItemLabel);

		switch(iItem){
			case LISTITEM_ICON:
				if(g_Config.ListView.iIcon<=LISTICON_NO_SMALL)break;

				if(g_DisplayWindowInfo[uItemID]->hIcon!=NULL){
					//アイコンを描写する
					RECT rcIcon={};
					ListView_GetItemRect(hListView,uItemID,&rcIcon,LVIR_ICON);
					OffsetRect(&rcIcon,-rcIcon.left,0);

					int iIconSize=g_Config.GetListIconSize();
					int padding=rcIcon.left+(rcIcon.right-rcIcon.left-iIconSize)/2;
					DrawIconEx(hDC,
							   padding,
							   rcIcon.top+(rcIcon.bottom-rcIcon.top-iIconSize)/2,
							   g_DisplayWindowInfo[uItemID]->hIcon,
							   iIconSize,
							   iIconSize,
							   0,0,
							   DI_IMAGE|DI_MASK);
				}

				break;

			case LISTITEM_FILENAME:
				DrawText(hDC,g_DisplayWindowInfo[uItemID]->szFileName,-1,&rcItemLabel,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
				break;

			case LISTITEM_WINDOWTITLE:
				DrawText(hDC,g_DisplayWindowInfo[uItemID]->szWindowTitle,-1,&rcItemLabel,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
				break;

			default:
				break;
		}
		//フォントの復元
		SelectObject(hDC,hOldFont);
	}

	if(g_Config.ListView.bDialogFrame){
		RECT rcFrame={};

		GetWindowRect(hListView,&rcFrame);
		HRGN hFrameRgn=CreateRectRgn(0,0,rcFrame.right-rcFrame.left,rcFrame.bottom-rcFrame.top);
		HBRUSH hFrameBrush=CreateSolidBrush(g_Config.ListView.clrDialogFrame);

		FrameRgn(hDC,hFrameRgn,hFrameBrush,1,1);
		DeleteObject(hFrameBrush);
		DeleteObject(hFrameRgn);
	}

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

	LPWINDOW_INFO* DisplayWindowInfoBak;
	int iDisplayWindowCount=g_iDisplayWindowCount;

	DisplayWindowInfoBak=(LPWINDOW_INFO*)HeapAlloc(g_hHeapWindowInfo,HEAP_ZERO_MEMORY,(iDisplayWindowCount)*sizeof(LPWINDOW_INFO));
	if(DisplayWindowInfoBak==NULL)return false;

	for(int i=0;i<iDisplayWindowCount;++i){
		DisplayWindowInfoBak[i]=g_DisplayWindowInfo[i];
	}

	if(szSearchString[0]!='\0'){
		LPWINDOW_INFO* DisplayWindowInfoTmp;

		DisplayWindowInfoTmp=(LPWINDOW_INFO*)HeapAlloc(g_hHeapWindowInfo,HEAP_ZERO_MEMORY,(iDisplayWindowCount)*sizeof(LPWINDOW_INFO));

		if(DisplayWindowInfoTmp==NULL)return false;

		for(int i=0;i<iDisplayWindowCount;++i){
			DisplayWindowInfoTmp[i]=g_DisplayWindowInfo[i];
		}

		//先頭のカラムを取得
		int iFirstItemColumn=g_ListViewColumn_Table[ListView_GetSubItemIndex(g_hListView,1)].bVisible;

		g_iDisplayWindowCount=0;

		bool bFullSearch=!bFirstColumnOnly&&
			g_ListViewColumn_Table[LISTITEM_FILENAME].bVisible&&
				g_ListViewColumn_Table[LISTITEM_WINDOWTITLE].bVisible;

		if(!tempconf::bMigemoSearch){
			//先頭のカラムを対象に絞り込む
			for(int i=0;i<iDisplayWindowCount;++i){
				bool bMatched=false;
				TCHAR* pszTargetString=(iFirstItemColumn==LISTITEM_FILENAME)?DisplayWindowInfoTmp[i]->szFileName:DisplayWindowInfoTmp[i]->szWindowTitle;
				TCHAR* pszTargetString2=(iFirstItemColumn==LISTITEM_FILENAME)?DisplayWindowInfoTmp[i]->szWindowTitle:DisplayWindowInfoTmp[i]->szFileName;

				if(tempconf::iMatchMode!=MATCH_FLEX){
					bMatched=(Match(iMatchMode,
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
					if(pszTargetStrings!=NULL){
						wsprintf(pszTargetStrings,_T("%s%s"),pszTargetString,(bFullSearch)?pszTargetString2:_T(""));

						bMatched=Match(iMatchMode,
									  pszTargetStrings,
									  szSearchString);

						HeapFree(GetProcessHeap(),0,pszTargetStrings);
						pszTargetStrings=NULL;
					}
				}

				if(bMatched){
					g_DisplayWindowInfo[g_iDisplayWindowCount]=DisplayWindowInfoTmp[i];
					DisplayWindowInfoTmp[i]=NULL;
					g_iDisplayWindowCount++;
				}
			}
		}else{
			int iRegexCount=0;
			//あいまい検索
			migemo::SetOperator((const unsigned char*)((iMatchMode!=MATCH_FLEX)?"":".*?"));

			//正規表現リストを作成
			TCHAR** ppszRegexes=migemo::AllocRegexes(szSearchString,&iRegexCount);

			if(ppszRegexes!=NULL){
				for(int i=0;i<iDisplayWindowCount;++i){
					bool bMatched=true;

					for(int iRegexIndex=0;bMatched&&iRegexIndex<iRegexCount;++iRegexIndex){
						migemo::CompiledBlock CompiledBlock;
						TCHAR* pszPattern=NULL;

						pszPattern=(TCHAR*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(lstrlen(ppszRegexes[iRegexIndex])+12)*sizeof(wchar_t));
						if(pszPattern==NULL)continue;
						wsprintf(pszPattern,_T("%s%s/i"),(iMatchMode==MATCH_FORWARD)?_T("m/^"):_T("m/"),ppszRegexes[iRegexIndex]);

						TCHAR* pszTargetString=(iFirstItemColumn==LISTITEM_FILENAME)?DisplayWindowInfoTmp[i]->szFileName:DisplayWindowInfoTmp[i]->szWindowTitle;
						TCHAR* pszTargetString2=(iFirstItemColumn==LISTITEM_FILENAME)?DisplayWindowInfoTmp[i]->szWindowTitle:DisplayWindowInfoTmp[i]->szFileName;

						if(tempconf::iMatchMode!=MATCH_FLEX){
							bMatched=(migemo::Match(CompiledBlock.Get(),
													pszPattern,
													pszTargetString,
													pszTargetString+lstrlen(pszTargetString))==1||
									  (bFullSearch&&
									   migemo::Match(CompiledBlock.Get(),
													 pszPattern,
													 pszTargetString2,
													 pszTargetString2+lstrlen(pszTargetString2))==1));
						}else{
							//あいまい検索であればカラムの文字列を結合して検索
							int iTargetStringsLength=(lstrlen(pszTargetString)+1);
							if(bFullSearch)iTargetStringsLength+=(lstrlen(pszTargetString2)+1);
							TCHAR* pszTargetStrings=(TCHAR*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(iTargetStringsLength+1)*sizeof(TCHAR));
							wsprintf(pszTargetStrings,_T("%s%s"),pszTargetString,(bFullSearch)?pszTargetString2:_T(""));

							bMatched=(migemo::Match(CompiledBlock.Get(),
													pszPattern,
													pszTargetStrings,
													pszTargetStrings+lstrlen(pszTargetStrings))==1);

							HeapFree(GetProcessHeap(),0,pszTargetStrings);
							pszTargetStrings=NULL;
						}
						HeapFree(GetProcessHeap(),0,pszPattern);
						pszPattern=NULL;
					}

					if(bMatched){
						g_DisplayWindowInfo[g_iDisplayWindowCount]=DisplayWindowInfoTmp[i];
						DisplayWindowInfoTmp[i]=NULL;
						g_iDisplayWindowCount++;
					}
				}
				//正規表現リストを解放
				migemo::FreeRegexes(&ppszRegexes);
			}
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
	status::bMigemoNomatch=false;

	if(!g_iDisplayWindowCount){
		status::bMigemoNomatch=true;
		g_iDisplayWindowCount=iDisplayWindowCount;

		for(int i=0;i<iDisplayWindowCount;++i){
			g_DisplayWindowInfo[i]=DisplayWindowInfoBak[i];
		}

		if(iSearchStringBack!=-1){
			if(!tempconf::bMigemoSearch){
				//入力を取り消す
				szSearchString[iSearchStringBack]='\0';
			}else{
				SendMessage(g_hListView,WM_SETREDRAW,false,0);

				//ウインドウ数確定
				ListView_SetItemCountEx(g_hListView,g_iDisplayWindowCount,LVSICF_NOINVALIDATEALL|LVSICF_NOSCROLL);
				//リサイズ
				ResizeListView();

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
		ResizeListView();

		SendMessage(g_hListView,WM_SETREDRAW,true,0);
		ListView_RedrawItems(g_hListView,0,g_iDisplayWindowCount);

		if(g_Config.ListView.bMouseOut){
			//カーソルがウインドウリスト外にあるなら右上へ移動させる
			if(!bMouseOut&&GetWindowThreadProcessId(ControlFromPoint(),&dwProcessId)!=0&&
			   dwProcessId!=GetCurrentProcessId()){
				RECT rc={};

				GetWindowRect(g_hListView,&rc);
				ClipCursor(&rc);
				ClipCursor(NULL);
			}
		}

		//候補のウインドウが1つなら確定
		if(g_iDisplayWindowCount==1&&
		   tempconf::bEnterUniqueWindow){
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

	ShowSearchWnd(szSearchString);
	//ツールチップ非表示
	ShowInfoToolTip();

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
			bResult=IncrementalSearch(szSearchString,tempconf::iMatchMode,tempconf::bFirstColumnOnly);
			if(!bResult)break;
		}
		if(bResult){
			ShowSearchWnd(g_szSearchString);
			return true;
		}
	}else{
		if(IncrementalSearch(g_szSearchString,tempconf::iMatchMode,tempconf::bFirstColumnOnly)){
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
	POINT ptMouse={};

	GetCursorPos(&ptMouse);

	RECT rc={};

	if(g_Config.ShowWindow.bMouseEachMonitor){
		MONITORINFOEX info={};

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

		FreeMonitorsList(&mi);
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
			if(status::eWindowList==status::WINDOWLIST::HIDE&&!status::bDisable&&!status::bShowSettingsDialog&&
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

	if(lpMsg->message==WM_KEYDOWN){
		if(lpMsg->wParam=='A'&&
		   GetAsyncKeyState(VK_CONTROL)&0x8000&&
		   GetAsyncKeyState(VK_SHIFT)>=0&&
		   GetAsyncKeyState(VK_MENU)>=0){
			//Ctrl+Aが押下されている場合
			HWND hWnd=GetFocus();
			if(hWnd!=NULL){
				TCHAR szClassName[128];

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

//検索文字列用ウインドウプロシージャ
LRESULT CALLBACK SearchWndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam){
	if(status::bDisable)return DefWindowProc(hWnd,uMsg,wParam,lParam);

	switch(uMsg){
		case WM_PAINT:{
			if(g_pszDisplaySearchString!=NULL){
				PAINTSTRUCT ps;
				HDC hDC=BeginPaint(hWnd,&ps);

				HFONT hOldFont=(HFONT)SelectObject(hDC,g_hSearchWindowFonts[GetCurrentIncrementalSearchState()]);

				SetTextColor(hDC,GetCurrentSearchWindowDesign()->clrText);

				SetBkMode(hDC,TRANSPARENT);

				HorizontalGradient(hDC,
								   ps.rcPaint,
								   GetCurrentSearchWindowDesign()->clrTextBk,
								   GetCurrentSearchWindowDesign()->clrTextBkGradientEnd);

				RECT rc=ps.rcPaint;

				rc.left+=g_Config.IncrementalSearch.iMargin;
				rc.top+=g_Config.IncrementalSearch.iMargin;

				DrawText(hDC,g_pszDisplaySearchString,-1,&rc,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);

				SelectObject(hDC,hOldFont);

				if(g_Config.ListView.bDialogFrame){
					RECT rcFrame={};
					GetWindowRect(hWnd,&rcFrame);
					HRGN hFrameRgn=CreateRectRgn(0,0,rcFrame.right-rcFrame.left,rcFrame.bottom-rcFrame.top);

					HBRUSH hFrameBrush=CreateSolidBrush(g_Config.ListView.clrDialogFrame);

					FrameRgn(hDC,hFrameRgn,hFrameBrush,1,1);
					DeleteObject(hFrameBrush);
					DeleteObject(hFrameRgn);
				}

				EndPaint(hWnd,&ps);
			}
			break;
		}
	}
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

//リストビュープロシージャ
LRESULT CALLBACK ListProc(HWND hListView,UINT uMsg,WPARAM wParam,LPARAM lParam){
	if(status::bDisable)return CallWindowProc((WNDPROC)GetWindowLongPtr(hListView,GWLP_USERDATA),hListView,uMsg,wParam,lParam);

	static bool bMenuLoop;
	static bool bMouseIn;
	static bool bMouseTracking;
	static bool bHideCursor;

	static bool bLButtonWheelTimer;
	static bool bRButtonWheelTimer;
	static bool bX1ButtonWheelTimer;
	static bool bX2ButtonWheelTimer;

	switch(uMsg){
		case WM_MOUSEWHEEL:
			//ホイール処理はMSG_MOUSEWHEELで行う
			return FALSE;

		case WM_ERASEBKGND:
			//ちらつき防止
			return FALSE;

		case WM_PAINT:{
			if(g_Config.DirectWrite.bDirectWrite&&
			   g_pDirectWriteListView!=NULL&&
			   g_pDirectWriteListView->IsAvailable()){
				PAINTSTRUCT ps={};
				BeginPaint(hListView,&ps);
				DrawItem(&ps);
				EndPaint(hListView,&ps);
				return false;
			}
			break;
		}

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

				if(migemo::IsAvailable()!=NULL&&
				   g_Config.IncrementalSearch.iMigemoMode!=MIGEMO_NO){
					if(g_szSearchString[0]=='\0'){
						tempconf::bMigemoSearch=(!bShift&&g_Config.IncrementalSearch.iMigemoMode==MIGEMO_LOWERCASE)||
							(bShift&&g_Config.IncrementalSearch.iMigemoMode==MIGEMO_UPPERCASE)||
								g_Config.IncrementalSearch.iMigemoMode==MIGEMO_ALWAYS;
					}

					if(!g_Config.IncrementalSearch.bMigemoCaseSensitive){
						//連文節検索を行わない場合であれば大文字小文字を統一
						if(c>='A'&&c<='Z'){
							c+=32;
						}
						if((tempconf::bMigemoSearch&&g_Config.IncrementalSearch.iMigemoMode==MIGEMO_UPPERCASE)||
						   (!tempconf::bMigemoSearch&&g_Config.IncrementalSearch.iMigemoMode==MIGEMO_LOWERCASE)){
							if(c>='a'&&c<='z'){
								c-=32;
							}
						}
					}
				}

				TCHAR key[2]={c};
				lstrcat(g_szSearchString,key);

				if(!tempconf::bMigemoSearch||g_Config.IncrementalSearch.iMigemoDelay==0){
					IncrementalSearch(g_szSearchString,tempconf::iMatchMode,tempconf::bFirstColumnOnly);
				}else{
					ShowSearchWnd(g_szSearchString);
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
			status::byModKey=0;
			if(GET_KEYSTATE_WPARAM(wParam)&MK_SHIFT)status::byModKey|=MODF_SHIFT;
			if(GET_KEYSTATE_WPARAM(wParam)&MK_CONTROL)status::byModKey|=MODF_CONTROL;

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
				status::byModKey=0;
				if(GET_KEYSTATE_WPARAM(wParam)&MK_SHIFT)status::byModKey|=MODF_SHIFT;
				if(GET_KEYSTATE_WPARAM(wParam)&MK_CONTROL)status::byModKey|=MODF_CONTROL;

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

			status::byModKey=0;
			if(GET_KEYSTATE_WPARAM(wParam)&MK_SHIFT)status::byModKey|=MODF_SHIFT;
			if(GET_KEYSTATE_WPARAM(wParam)&MK_CONTROL)status::byModKey|=MODF_CONTROL;

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
					//WM_SETFOCUS(status::CREATED時)の続き
					KillTimer(hListView,wParam);

					//一時的な設定をリセット
					tempconf::Reset();

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

					status::eWindowList=status::WINDOWLIST::DISPLAYED;

					//カーソルを閉じ込める
					if(status::eCorners!=SCC_NONE&&
					   (g_Config.ShowWindow.iMouseClipCursorMode!=CLIP_CURSOR_NO&&!status::bHotKeyMode||
						g_Config.ShowWindow.iHotKeyClipCursorMode!=CLIP_CURSOR_NO&&status::bHotKeyMode)){
						bool bMoveCursorAtStart=
							(!status::bHotKeyMode&&g_Config.ShowWindow.bMouseMoveCursorSelectedWindow||
							 status::bHotKeyMode&&g_Config.ShowWindow.bHotKeyMoveCursorSelectedWindow)&&
								g_Config.ShowWindow.bMoveCursorAtStart;
						int iClipCursorMode=(!status::bHotKeyMode)?g_Config.ShowWindow.iMouseClipCursorMode:g_Config.ShowWindow.iHotKeyClipCursorMode;
						int iClipCursorTime=(!status::bHotKeyMode)?g_Config.ShowWindow.iMouseClipCursorTime:g_Config.ShowWindow.iHotKeyClipCursorTime;
						RECT rcClip={};

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
					   status::eCorners!=SCC_NONE){
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
					if(status::eWindowList!=status::WINDOWLIST::DISPLAYED)break;
					if(GetAsyncKeyState(VK_MENU)&0x8000)break;
					if(bMenuLoop)break;
					if(IsWindowVisible(g_hInfoToolTip))break;

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

				case ID_TIMER_UNIQUE_WINDOW:{
					//候補が1つになった時の切り替え
					KillTimer(hListView,wParam);

					if(status::eWindowList!=status::WINDOWLIST::DISPLAYED)break;
					if(bMenuLoop)break;
					if(IsWindowVisible(g_hInfoToolTip))break;

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

					if(status::eWindowList==status::WINDOWLIST::DISPLAYED&&
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
			if(status::eWindowList!=status::WINDOWLIST::DISPLAYED)break;

			if(!g_Config.ListView.bMouseOut)break;

			if(GetAsyncKeyState(VK_MENU)&0x8000)break;

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
				if(status::eWindowList==status::WINDOWLIST::DISPLAYED){
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
						if(IsWindowVisible(g_hInfoToolTip))break;

						if(g_Config.ListView.bMouseHover||
						   g_Config.ListView.bDragMouseHover&&IsDragging()){
							int iIndex=ListView_GetHitItem(hListView);
							if(iIndex<0)break;
							ListView_SelectItem(hListView,iIndex);
						}
						break;
					}else if(g_Config.ListView.bMouseOut&&
							 bMouseIn){
						if(GetForegroundWindow()!=g_hMainWnd){
							SetForegroundWindow(g_hMainWnd);
						}
						SetFocus(hListView);
						if(!bMouseTracking){
							bMouseTracking=true;
							MouseLeaveEvent(hListView);
						}
					}
				}
			}else if((uMsg==MSG_MOUSEWHEEL||uMsg==WM_MOUSEHWHEEL)&&
					 status::eWindowList==status::WINDOWLIST::DISPLAYED&&
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
						if(uMsg==MSG_MOUSEWHEEL&&MouseType_Table[i].uMouseWheel==MOUSEWHEEL_UP&&
						   GET_WHEEL_DELTA_WPARAM(wParam)/WHEEL_DELTA>0||
						   uMsg==MSG_MOUSEWHEEL&&MouseType_Table[i].uMouseWheel==MOUSEWHEEL_DOWN&&
							 GET_WHEEL_DELTA_WPARAM(wParam)/WHEEL_DELTA<0||
						   uMsg==WM_MOUSEHWHEEL&&MouseType_Table[i].uMouseWheel==MOUSEHWHEEL_LEFT&&
							 GET_WHEEL_DELTA_WPARAM(wParam)/WHEEL_DELTA<0||
						   uMsg==WM_MOUSEHWHEEL&&MouseType_Table[i].uMouseWheel==MOUSEHWHEEL_RIGHT&&
							 GET_WHEEL_DELTA_WPARAM(wParam)/WHEEL_DELTA>0){
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
							status::bMouseCommand=true;
							SendMessage(g_hMainWnd,WM_COMMAND,MAKEWPARAM(g_Config.Mouse.sMouseTable[i].uCmdId,0),0);
							status::bMouseCommand=false;
							//breakして通常のCallWindowProcで処理する場合、チルトし続けるとWM_MOUSEHWHEELもその間飛んでくる
							//return falseするとチルトし続けても1回のみ
							return false;
						}
					}
				}
			}
			break;
		}
	}

	return CallWindowProc((WNDPROC)GetWindowLongPtr(hListView,GWLP_USERDATA),hListView,uMsg,wParam,lParam);
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
		case WM_XBUTTONDOWN:
		case WM_KEYDOWN:
		case WM_NCCALCSIZE:
		case WM_MOUSEWHEEL:
		case WM_MOUSEHWHEEL:
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
							dwm::UnregisterThumbnail(g_WindowInfo[i].hThumbnail);
							g_WindowInfo[i].hThumbnail=NULL;
						}
					}
				}

				int iIndex=ListView_GetSelectedItem(g_hListView);

				if(iIndex<0)break;

				if(g_hMenuWnd&&IsWindowVisible(g_hMenuWnd)||
				   g_hInfoToolTip&&IsWindowVisible(g_hInfoToolTip)){
					break;
				}

				if(g_DisplayWindowInfo[iIndex]->bCancelItem){
					break;
				}

				if(!g_DisplayWindowInfo[iIndex]->hThumbnail){
					dwm::RegisterThumbnail(g_hThumbnailWnd,
										  (!g_DisplayWindowInfo[iIndex]->bDesktopItem)?g_DisplayWindowInfo[iIndex]->hWnd:GetShellWindow(),
										  &g_DisplayWindowInfo[iIndex]->hThumbnail);
				}
				if(g_DisplayWindowInfo[iIndex]->hThumbnail){
					SIZE size={};

					dwm::QueryThumbnailSourceSize(g_DisplayWindowInfo[iIndex]->hThumbnail,&size);

					RECT rcDest={0,0,0,0};
					float rY=(float)g_Config.ListView.iThumbnailSize/size.cy;

					if(!tempconf::bThumbnailOriginalSize&&
					   g_Config.ListView.iThumbnailSize<size.cy){
						rcDest.bottom=(long)(size.cy*rY);
						rcDest.right=(long)(size.cx*rY);
					}else{
						rcDest.bottom=size.cy;
						rcDest.right=size.cx;
					}

					DWM_THUMBNAIL_PROPERTIES dtp={};
					dtp.dwFlags=DWM_TNP_RECTDESTINATION|DWM_TNP_VISIBLE|DWM_TNP_SOURCECLIENTAREAONLY;
					dtp.fSourceClientAreaOnly=FALSE;
					dtp.fVisible=TRUE;
					dtp.rcDestination=rcDest;
					dwm::UpdateThumbnailProperties(g_DisplayWindowInfo[iIndex]->hThumbnail,&dtp);

					int iSubItem=0;

					switch(g_Config.ListView.uThumbnailVerticalAlign){
						case THUMBNAIL_VERTICALALIGN_TOP:
							iSubItem=ListView_GetTopIndex(g_hListView);
							break;
						case THUMBNAIL_VERTICALALIGN_MIDDLE:
							iSubItem=ListView_GetTopIndex(g_hListView)+ListView_GetCountPerPage(g_hListView)/3;
							break;
						case THUMBNAIL_VERTICALALIGN_BOTTOM:
							iSubItem=ListView_GetTopIndex(g_hListView)+ListView_GetCountPerPage(g_hListView)-1;
							break;
						case THUMBNAIL_VERTICALALIGN_SELECTED_ITEM:
							iSubItem=iIndex;
							break;
						default:
							break;
					}

					RECT rc={};

					switch(tempconf::uThumbnail){
						case THUMBNAIL_FIRST_COLUMN:
							ListView_GetSubItemRect(g_hListView,
													iSubItem,
													ListView_GetSubItemIndex(g_hListView,1),
													LVIR_LABEL,
													&rc);
							break;
						case THUMBNAIL_SECOND_COLUMN:
						if(!g_ListViewColumn_Table[ListView_GetSubItemIndex(g_hListView,2)].bVisible){
							//2番目のカラムが存在しなければ1番目のカラム上に表示
							ListView_GetSubItemRect(g_hListView,
													iSubItem,
													ListView_GetSubItemIndex(g_hListView,1),
													LVIR_LABEL,
													&rc);
						}else{
							ListView_GetSubItemRect(g_hListView,
													iSubItem,
													ListView_GetSubItemIndex(g_hListView,2),
													LVIR_LABEL,
													&rc);
						}
							break;
						case THUMBNAIL_LEFT_SIDE:
						case THUMBNAIL_RIGHT_SIDE:
						default:
							ListView_GetSubItemRect(g_hListView,
													iSubItem,
													0,
													LVIR_BOUNDS,
													&rc);
							break;
					}

					POINT pt={rc.left,rc.top};

					if(tempconf::uThumbnail==THUMBNAIL_LEFT_SIDE){
						pt.x=-(rcDest.right-rcDest.left);
					}else if(tempconf::uThumbnail==THUMBNAIL_RIGHT_SIDE){
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
	//タスクトレイ用アイコン
	static HICON hTrayIcon;
	static HICON hDisableTrayIcon;

	//タスクトレイメニューハンドル
	static MENU mTray;
	//メニュー群
	static MENU mMainCmd;
	static MENU mSelectCmd;
	static MENU mCopyInfoCmd;
	static MENU mIncrementalSearchCmd;
	static MENU mWindowCmd;
	static MENU mItemTypeCmd;
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
			INITCOMMONCONTROLSEX ic={};
			ic.dwSize=sizeof(INITCOMMONCONTROLSEX);

			//リストビュー、ホットキー
			ic.dwICC=ICC_LISTVIEW_CLASSES|ICC_HOTKEY_CLASS;
			InitCommonControlsEx(&ic);

			//リストビューを作成
			g_hListView=CreateWindowEx(WS_EX_ACCEPTFILES,
									   WC_LISTVIEW,
									   _T(""),
									   WS_CHILD|WS_VISIBLE|LVS_REPORT|LVS_ICON|LVS_SINGLESEL|LVS_NOCOLUMNHEADER|LVS_OWNERDATA|LVS_OWNERDRAWFIXED,
									   0,0,0,0,
									   hWnd,
									   (HMENU)IDC_LISTVIEW,
									   GetModuleHandle(NULL),
									   NULL);

			//IMEを無効化
			{
				TCHAR szImm32DllPath[MAX_PATH]={};

				GetSystemDirectory(szImm32DllPath,ARRAY_SIZEOF(szImm32DllPath));
				lstrcat(szImm32DllPath,_T("\\imm32.dll"));
				HMODULE hImm32=LoadLibrary(szImm32DllPath);
				if(hImm32!=NULL){
					HIMC(WINAPI*pImmAssociateContext)(HWND,HIMC)=(HIMC(WINAPI*)(HWND,HIMC))GetProcAddress(hImm32,"ImmAssociateContext");
					if(pImmAssociateContext!=NULL)pImmAssociateContext(g_hListView,0);
					FreeLibrary(hImm32);
				}
			}

			//ダブルバッファ追加
			ListView_SetExtendedListViewStyle(g_hListView,
											  ListView_GetExtendedListViewStyle(g_hListView)|
											  LVS_EX_DOUBLEBUFFER);

			//サブクラス化
			SetWindowLongPtr(g_hListView,GWLP_USERDATA,GetWindowLongPtr(g_hListView,GWLP_WNDPROC));
			SetWindowLongPtr(g_hListView,GWLP_WNDPROC,(LONG_PTR)ListProc);

			//カラムを設定
			ListView_InitColumn(g_ListViewColumn_Table,&g_Config,g_hListView);

			LVCOLUMN lvColumn={};
			lvColumn.mask=LVCF_FMT|LVCF_TEXT|LVCF_SUBITEM;
			lvColumn.fmt=LVCFMT_LEFT;
			//アイコン
			lvColumn.pszText=(LPTSTR)_T("アイコン");
			lvColumn.iSubItem=LISTITEM_ICON;
			ListView_InsertColumn(g_hListView,LISTITEM_ICON,&lvColumn);
			//現在のファイル名
			lvColumn.pszText=(LPTSTR)_T("ファイル名");
			lvColumn.iSubItem=LISTITEM_FILENAME;
			ListView_InsertColumn(g_hListView,LISTITEM_FILENAME,&lvColumn);
			//新しいファイル名
			lvColumn.pszText=(LPTSTR)_T("ウインドウタイトル");
			lvColumn.iSubItem=LISTITEM_WINDOWTITLE;
			ListView_InsertColumn(g_hListView,LISTITEM_WINDOWTITLE,&lvColumn);

			//イメージリストの初期化
			InitializeImageList();

			//インクリメンタルサーチウインドウ作成
			TCHAR szSearchWindowClassName[]=_T("TascherSearchWindowClass");
			WNDCLASSEX wc={};

			wc.cbSize		=sizeof(WNDCLASSEX);
			wc.style		=0;
			wc.lpfnWndProc	=(WNDPROC)SearchWndProc;
			wc.cbClsExtra	=0;
			wc.cbWndExtra	=0;
			wc.hInstance	=GetModuleHandle(NULL);
			wc.hIcon		=NULL;
			wc.hCursor		=NULL;
			wc.hbrBackground=NULL;
			wc.lpszMenuName	=NULL;
			wc.lpszClassName=szSearchWindowClassName;
			RegisterClassEx(&wc);

			g_hSearchWnd=CreateWindowEx(WS_EX_LAYERED|WS_EX_NOACTIVATE|WS_EX_TRANSPARENT|WS_EX_TOPMOST,
										szSearchWindowClassName,
										NULL,
										WS_CHILD|WS_POPUP,
										0,0,0,0,
										g_hMainWnd,
										(HMENU)0,
										GetModuleHandle(NULL),
										NULL);
			if(g_Config.IncrementalSearch.byOpacity!=100){
				SetWindowLongPtr(g_hSearchWnd,GWL_EXSTYLE,GetWindowLongPtr(g_hSearchWnd,GWL_EXSTYLE)|WS_EX_LAYERED);
				SetLayeredWindowAttributes(g_hSearchWnd,0,g_Config.IncrementalSearch.byOpacity*255/100,LWA_ALPHA);
			}else{
				SetWindowLongPtr(g_hSearchWnd,GWL_EXSTYLE,GetWindowLongPtr(g_hSearchWnd,GWL_EXSTYLE)&~WS_EX_LAYERED);
			}

			//ツールチップを作成
			g_hInfoToolTip=CreateWindowEx(WS_EX_TOPMOST|WS_EX_TRANSPARENT,
									  TOOLTIPS_CLASS,
									  NULL,
									  WS_POPUP|TTS_NOFADE|TTS_NOANIMATE|TTS_NOPREFIX,
									  CW_USEDEFAULT,
									  CW_USEDEFAULT,
									  CW_USEDEFAULT,
									  CW_USEDEFAULT,
									  g_hListView,
									  NULL,
									  GetModuleHandle(NULL),
									  NULL);

			TOOLINFO ti={};
			ti.cbSize=sizeof(TOOLINFO);
			ti.uFlags=TTF_TRACK|TTF_SUBCLASS;
			ti.hwnd=g_hListView;
			ti.hinst=GetModuleHandle(NULL);
			ti.lpszText=NULL;
			ti.uId=ID_TOOLTIP_INFO;
			//ツールチップを登録
			SendMessage(g_hInfoToolTip,TTM_ADDTOOL,0,(LPARAM)&ti);

			//フォントを読み込む
			g_hDefaultItemFont=CreateGUIFont(g_Config.DefaultItemDesign.szFont,g_Config.DefaultItemDesign.iFontSize,g_Config.DefaultItemDesign.iFontStyle);
			g_hSelectedItemFont=CreateGUIFont(g_Config.SelectedItemDesign.szFont,g_Config.SelectedItemDesign.iFontSize,g_Config.SelectedItemDesign.iFontStyle);

			g_hSearchWindowFonts[INCREMENTALSEARCH_SEARCHWINDOWDESIGN_DEFAULT]=CreateGUIFont(g_Config.IncrementalSearch.DefaultSearchWindowDesign.szFont,g_Config.IncrementalSearch.DefaultSearchWindowDesign.iFontSize,g_Config.IncrementalSearch.DefaultSearchWindowDesign.iFontStyle);
			g_hSearchWindowFonts[INCREMENTALSEARCH_SEARCHWINDOWDESIGN_MIGEMOMATCH]=CreateGUIFont(g_Config.IncrementalSearch.MigemoMatchSearchWindowDesign.szFont,g_Config.IncrementalSearch.MigemoMatchSearchWindowDesign.iFontSize,g_Config.IncrementalSearch.MigemoMatchSearchWindowDesign.iFontStyle);
			g_hSearchWindowFonts[INCREMENTALSEARCH_SEARCHWINDOWDESIGN_MIGEMONOMATCH]=CreateGUIFont(g_Config.IncrementalSearch.MigemoNoMatchSearchWindowDesign.szFont,g_Config.IncrementalSearch.MigemoNoMatchSearchWindowDesign.iFontSize,g_Config.IncrementalSearch.MigemoNoMatchSearchWindowDesign.iFontStyle);

			if(g_Config.DirectWrite.bDirectWrite&&
			   g_pDirectWriteListView!=NULL&&
			   g_pDirectWriteListView->IsAvailable()){
				//ResizeListView()でResize()するため大きさは0,0指定
				g_pDirectWriteListView->CreateRenderTarget(g_hListView,0,0);
				CreateBrush();
			}

			//タスクトレイメニューを読み込む
			mTray.hMenu=LoadMenu(GetModuleHandle(NULL),MAKEINTRESOURCE(IDR_TRAYMENU));
			mTray.hSubMenu=GetSubMenu(mTray.hMenu,0);

			//タスクトレイアイコン読み込み
			hTrayIcon=(HICON)LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_ICON1),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
			hDisableTrayIcon=(HICON)LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_ICON_DISABLE),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);

			//タスクトレイに登録
			TaskTray(hWnd,hTrayIcon,NIM_ADD);

			//非表示用カーソル読み込み
			g_hBlankCursor=(HCURSOR)LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_CURSOR_BLANK),IMAGE_CURSOR,0,0,LR_DEFAULTSIZE);

			//壁紙設定
			if(lstrlen(g_Config.Background.szImagePath)){
				if(g_Config.DirectWrite.bDirectWrite&&
				   g_pDirectWriteListView!=NULL&&
				   g_pDirectWriteListView->IsAvailable()){
					g_pDirectWriteListView->LoadBackgroundImage(g_Config.Background.szImagePath,
															  g_Config.Background.byResizePercent,
															  g_Config.Background.byOpacity);
					status::bBackground=g_pDirectWriteListView->IsLoadedBackgroundImage();
				}else if(g_pGdiplusBackgroundImage!=NULL){
					status::bBackground=g_pGdiplusBackgroundImage->Load(g_Config.Background.szImagePath);
				}
			}else{
				status::bBackground=false;
			}

			//ホットキーを登録する
			if(g_Config.ShowWindow.wHotKey){
				RegistHotKey(hWnd,ID_HOTKEY,&g_Config.ShowWindow.wHotKey);
			}

			//メニュー群読み込み
			mMainCmd.Load(IDR_MAINMENU);
			mSelectCmd.Load(IDR_SELECTMENU);
			mCopyInfoCmd.Load(IDR_COPYINFOMENU);
			mIncrementalSearchCmd.Load(IDR_INCREMENTALSEARCHMENU);
			mWindowCmd.Load(IDR_WINDOWMENU);
			mProcessCmd.Load(IDR_PROCESSMENU);
			mOpacityCmd.Load(IDR_OPACITYMENU);
			mItemTypeCmd.Load(IDR_ITEMTYPEMENU);
			mCommand.Load(IDR_COMMANDMENU);

			//メインメニュー作成
			for(int i=0,iMenuItemCount=GetMenuItemCount(mMainCmd.hSubMenu);i<iMenuItemCount;++i){
				MENUITEMINFO mii={};
				mii.cbSize=sizeof(MENUITEMINFO);
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
						MENUITEMINFO miiWindowCmd={};
						miiWindowCmd.cbSize=sizeof(MENUITEMINFO);
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

					mii.hSubMenu=mItemTypeCmd.hSubMenu;
					mii.dwTypeData=_T("itemt&ype");
					InsertMenuItem(mMainCmd.hSubMenu,iMenuIndex++,true,&mii);

					mii.hSubMenu=mCommand.hSubMenu;
					mii.dwTypeData=_T("co&mmand");
					InsertMenuItem(mMainCmd.hSubMenu,iMenuIndex++,true,&mii);
					break;
				}
			}

			//DwmRegisterThumbnail()でサムネイルを表示するウインドウはトップレベルでなければならない
			TCHAR szThumbnailWindowClassName[]=_T("TascherThumbnailWindowClass");

			wc.style		=0;
			wc.lpfnWndProc	=(WNDPROC)ThumbnailWindowProc;
			wc.cbClsExtra	=0;
			wc.cbWndExtra	=0;
			wc.hInstance	=GetModuleHandle(NULL);
			wc.hIcon		=NULL;
			wc.hCursor		=NULL;
			wc.hbrBackground=NULL;
			wc.lpszMenuName	=NULL;
			wc.lpszClassName=szThumbnailWindowClassName;
			RegisterClassEx(&wc);

			g_hThumbnailWnd=CreateWindowEx(WS_EX_LAYERED|WS_EX_NOACTIVATE|WS_EX_TRANSPARENT,
										   szThumbnailWindowClassName,
										   NULL,
										   0,
										   0,0,0,0,
										   NULL,
										   (HMENU)0,
										   GetModuleHandle(NULL),
										   NULL);

			//デフォルトの不透明度(トグル用)
			tempconf::byOpacity=50;

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

			MENUITEMINFO mii={};
			mii.cbSize=sizeof(MENUITEMINFO);
			mii.fMask=MIIM_STATE;

			if((HMENU)wParam==mTray.hSubMenu){
				//「無効」メニューのチェックを設定
				mii.fState=(status::bDisable?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo(mTray.hSubMenu,IDM_MENU_DISABLE,false,&mii);
			}else if((HMENU)wParam==mMainCmd.hSubMenu||
					 (HMENU)wParam==mIncrementalSearchCmd.hSubMenu||
					 (HMENU)wParam==mWindowCmd.hSubMenu||
					 (HMENU)wParam==mProcessCmd.hSubMenu||
					 (HMENU)wParam==mOpacityCmd.hSubMenu||
					 (HMENU)wParam==mItemTypeCmd.hSubMenu||
					 (HMENU)wParam==mCommand.hSubMenu||
					 (HMENU)wParam==hChildProcessMenu){
				//thumbnail
				mii.fState=(tempconf::uThumbnail?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo((HMENU)wParam,IDM_KEY_THUMBNAIL,false,&mii);
				//forwardmatch
				mii.fState=(tempconf::iMatchMode==MATCH_FORWARD?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo((HMENU)wParam,IDM_KEY_FORWARD_MATCH,false,&mii);
				//partialmatch
				mii.fState=(tempconf::iMatchMode==MATCH_PARTICAL?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo((HMENU)wParam,IDM_KEY_PARTIAL_MATCH,false,&mii);
				//flexmatch
				mii.fState=(tempconf::iMatchMode==MATCH_FLEX?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo((HMENU)wParam,IDM_KEY_FLEX_MATCH,false,&mii);
				//searchscope
				mii.fState=(!tempconf::bFirstColumnOnly?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo((HMENU)wParam,IDM_KEY_SEARCHSCOPE,false,&mii);
				//uniquewindow
				mii.fState=(tempconf::bEnterUniqueWindow?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo((HMENU)wParam,IDM_KEY_UNIQUE_WINDOW,false,&mii);

				int iIndex=ListView_GetSelectedItem(g_hListView);

				if(iIndex<0)break;

				WINDOWPLACEMENT wndpl={};
				wndpl.length=sizeof(WINDOWPLACEMENT);

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

				//windowitem
				mii.fState=((tempconf::bWindowItem)?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo((HMENU)wParam,IDM_KEY_ITEMTYPE_WINDOW,false,&mii);

				//webbrowsertab
				mii.fState=((tempconf::bWebBrowserTabItem)?MFS_CHECKED:MFS_UNCHECKED);
				SetMenuItemInfo((HMENU)wParam,IDM_KEY_ITEMTYPE_WEBBROWSERTAB,false,&mii);


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

				if(!g_DisplayWindowInfo[iIndex]->bWebBrowserTabItem){
					GetWindowThreadProcessId((!g_DisplayWindowInfo[iIndex]->bUWPApp)?g_DisplayWindowInfo[iIndex]->hWnd:g_DisplayWindowInfo[iIndex]->hUICoreWnd,&dwProcessId);
				}else{
					dwProcessId=g_DisplayWindowInfo[iIndex]->dwProcessId;
				}

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
				MENUITEMINFO mii={};
				mii.cbSize=sizeof(MENUITEMINFO);
				mii.fMask=MIIM_DATA|MIIM_ID;

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
					if(status::eWindowList==status::WINDOWLIST::CREATING||status::bDisable||status::bShowSettingsDialog)break;
					if(status::eWindowList==status::WINDOWLIST::DISPLAYED){
						//表示されている場合
						if(!g_Config.ListView.bHotKey){
							//非表示
							SendMessage(g_hMainWnd,WM_COMMAND,MAKEWPARAM(IDM_KEY_CANCEL,0),0);
						}else{
							//切り替え
							SendMessage(g_hMainWnd,WM_COMMAND,MAKEWPARAM(IDM_KEY_SWITCH,0),0);
						}
					}else{
						status::bHotKeyMode=true;
						//表示
						ShowListView(g_Config.ShowWindow.eHotKeyCursorCorner);
					}
					break;

				default:
					break;
			}
			break;

		case WM_DRAWITEM:
			if(!g_Config.DirectWrite.bDirectWrite&&
			   g_pDirectWriteListView!=NULL&&
			   !g_pDirectWriteListView->IsAvailable()){
				DrawItemGdiPlus((LPDRAWITEMSTRUCT)lParam);
				return true;
			}
			break;

		case WM_SIZE:
			MoveWindow(g_hListView,0,0,LOWORD(lParam),HIWORD(lParam),true);
			break;

		case WM_SETFOCUS:
			SetFocus(g_hListView);

			if(status::eWindowList==status::WINDOWLIST::CREATED){
				//ShowListView()の続き
				if(status::eCorners!=SCC_NONE){
					RECT rcMainWnd={};

					GetWindowRect(g_hMainWnd,&rcMainWnd);
					ClipCursor(&rcMainWnd);
					POINT pt={};

					GetCursorPos(&pt);

					//Tascherにフォーカスを持ってくるためクリック
					//ドラッグがキャンセルされないようドラッグ中はクリックしない
					if(SetCursorCorner(g_hMainWnd,
									   status::eCorners,
									   (!status::bHotKeyMode)?g_Config.ShowWindow.iMouseCursorMarginHorizontal:g_Config.ShowWindow.iHotKeyCursorMarginHorizontal,
									   (!status::bHotKeyMode)?g_Config.ShowWindow.iMouseCursorMarginVertical:g_Config.ShowWindow.iHotKeyCursorMarginVertical)&&
					   IsCursorInWindow(g_hMainWnd)&&
					   (!(GetAsyncKeyState(VK_LBUTTON)&0x8000)&&
					   !(GetAsyncKeyState(VK_RBUTTON)&0x8000)||
					   FindWindow(_T("#32768"),NULL)!=NULL)){

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

				//タイマーで処理を分離しなければクリックの送信がstatus::DISPLAYEDの状態で送られてしまう
				SetTimer(g_hListView,ID_TIMER_READY,0,NULL);
			}
			break;

		case WM_ACTIVATEAPP:
			if(status::eWindowList!=status::WINDOWLIST::DISPLAYED)break;
			if(!(BOOL)wParam&&(DWORD)lParam!=GetWindowThreadProcessId(hWnd,NULL)){
				HideListView();
			}
			break;

		case WM_NOTIFY:{
			if(status::eWindowList!=status::WINDOWLIST::DISPLAYED)break;
			int iIndex=ListView_GetSelectedItem(g_hListView);

			if(iIndex!=-1){
				LPNMHDR lpNMHdr=(LPNMHDR)lParam;
				switch(lpNMHdr->code){
					case LVN_ITEMCHANGED:
						//アイテムの選択が変更された場合
						ClipCursor(NULL);

						while(IsWindowVisible(g_hMenuWnd)){
							//メニューを閉じる
							SendMessage(g_hMenuWnd,WM_KEYDOWN,VK_ESCAPE,0);
							Sleep(10);
						}

						//ツールチップ非表示
						ShowInfoToolTip();
//						ListView_RedrawItems(lpNMHdr->hwndFrom,((LPNMLISTVIEW)lParam)->iItem,((LPNMLISTVIEW)lParam)->iItem);
						if(g_Config.ListView.iTimeOut>0||
						   g_Config.ListView.bDragTimeOut&&IsCursorInWindow(g_hMainWnd)&&IsDragging()){
							//タイムアウトで確定するためタイマーを設定
							SetTimer(g_hListView,
									 MAKEWPARAM(IDM_KEY_SWITCH,ListView_GetSelectedItem(g_hListView)),
									 (g_Config.ListView.iTimeOut>0)?g_Config.ListView.iTimeOut:DEFAULT_SELECT_TIMEOUT,
									 NULL);
						}

						if(!status::bHotKeyMode&&g_Config.ShowWindow.bMouseMoveCursorSelectedWindow||
						   status::bHotKeyMode&&g_Config.ShowWindow.bHotKeyMoveCursorSelectedWindow){
							//選択項目にマウスを移動
							POINT ptOrig={};

							GetCursorPos(&ptOrig);

							if(WindowFromPoint(ptOrig)==g_hListView||
							   WindowFromPoint(ptOrig)==g_hMainWnd){
								RECT rc={};

								ListView_GetSubItemRect(g_hListView,ListView_GetSelectedItem(g_hListView),
														g_ListViewColumn_Table[0].bVisible&&g_ListViewColumn_Table[1].bVisible,
														LVIR_BOUNDS,&rc);

								POINT pt={rc.left,rc.top};
								ClientToScreen(g_hListView,&pt);

								SetCursorPos(ptOrig.x,pt.y+(rc.bottom-rc.top)/2);
							}
						}

						if(status::eWindowList==status::WINDOWLIST::DISPLAYED&&
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
			bool bButtonDown=false;
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

					if(status::eWindowList==status::WINDOWLIST::HIDE&&!status::bDisable&&!status::bShowSettingsDialog){
						for(int i=0;i<ARRAY_SIZEOF(SccTimerTable);i++){
							if(eCorners==SccTimerTable[i].eCorners&&
							   SccTimerTable[i].uId==wParam){
								ShowListView(g_Config.ShowWindow.ccCursorCorners[eCorners].eDestCorner);
								break;
							}
						}
					}
					return DefWindowProc(hWnd,uMsg,wParam,lParam);
				}

				case WM_CHAR:
					KillTimer(hWnd,wParam);
					IncrementalSearch(g_szSearchString,tempconf::iMatchMode,tempconf::bFirstColumnOnly);
					return DefWindowProc(hWnd,uMsg,wParam,lParam);

				case MAKEWPARAM(IDM_KEY_BS_STRING,1):
					//インクリメンタルサーチの文字列を1つ削る
					KillTimer(hWnd,wParam);
					//表示を元に戻す
					for(int i=0;i<g_iWindowCount;++i){
						g_DisplayWindowInfo[i]=&g_WindowInfo[i];
					}
					g_iDisplayWindowCount=g_iWindowCount;

					IncrementalSearch(g_szSearchString,tempconf::iMatchMode,tempconf::bFirstColumnOnly);

					if(hSelectedItemWnd){
						for(int i=0;i<g_iDisplayWindowCount;++i){
							if(g_DisplayWindowInfo[i]->hWnd==hSelectedItemWnd){
								ListView_SelectItem(g_hListView,i);
								break;
							}
						}
					}

					if(g_szSearchString[0]=='\0'){
						ShowSearchWnd();
						//ツールチップ非表示
						ShowInfoToolTip();
					}
					return DefWindowProc(hWnd,uMsg,wParam,lParam);

				//キー押下確認
				case WM_MBUTTONDOWN:
					bButtonDown=true;
					KillTimer(hWnd,wParam);
					//fall through
				case WM_MBUTTONDBLCLK:
					bButtonDown=true;
					byMod|=MODF_MBUTTON;
					break;
				case WM_LBUTTONDOWN:
					bButtonDown=true;
					KillTimer(hWnd,wParam);
					//fall through
				case WM_LBUTTONDBLCLK:
					bButtonDown=true;
					byMod|=MODF_LBUTTON;
					break;
				case WM_RBUTTONDOWN:
					bButtonDown=true;
					KillTimer(hWnd,wParam);
					//fall through
				case WM_RBUTTONDBLCLK:
					bButtonDown=true;
					byMod|=MODF_RBUTTON;
					break;
				case WM_X1BUTTONDOWN:
					bButtonDown=true;
					KillTimer(hWnd,wParam);
					//fall through
				case WM_X1BUTTONDBLCLK:
					bButtonDown=true;
					byMod|=MODF_X1BUTTON;
					break;
				case WM_X2BUTTONDOWN:
					bButtonDown=true;
					KillTimer(hWnd,wParam);
					//fall through
				case WM_X2BUTTONDBLCLK:
					bButtonDown=true;
					byMod|=MODF_X2BUTTON;
					break;
				default:
					return DefWindowProc(hWnd,uMsg,wParam,lParam);
			}

			if(bButtonDown){
				//保存していたShiftとCtrlの状態を適用
				byMod|=status::byModKey;
				status::byModKey=0;

				for(int i=0;i<SMOUSE_NUM;++i){
					if(!g_Config.Mouse.sMouseTable[i].uCmdId)continue;
					if(wParam==MouseType_Table[i].uMessageType&&
					   MouseType_Table[i].byMod==byMod){
						if(MouseType_Table[i].uMouseWheel==MOUSEWHEEL_UP&&
						   GET_WHEEL_DELTA_WPARAM(wParam)/WHEEL_DELTA>0){
							status::bMouseCommand=true;
							//マウスホイール上
							SendMessage(g_hMainWnd,WM_COMMAND,MAKEWPARAM(g_Config.Mouse.sMouseTable[i].uCmdId,0),0);
							status::bMouseCommand=false;
							break;
						}else if(MouseType_Table[i].uMouseWheel==MOUSEWHEEL_DOWN&&
							 GET_WHEEL_DELTA_WPARAM(wParam)/WHEEL_DELTA<0){
							status::bMouseCommand=true;
							//マウスホイール下
							SendMessage(g_hMainWnd,WM_COMMAND,MAKEWPARAM(g_Config.Mouse.sMouseTable[i].uCmdId,0),0);
							status::bMouseCommand=false;
							break;
						}else if(MouseType_Table[i].uMouseWheel==MOUSEHWHEEL_LEFT&&
							 GET_WHEEL_DELTA_WPARAM(wParam)/WHEEL_DELTA<0){
							status::bMouseCommand=true;
							//マウスホイール左
							SendMessage(g_hMainWnd,WM_COMMAND,MAKEWPARAM(g_Config.Mouse.sMouseTable[i].uCmdId,0),0);
							status::bMouseCommand=false;
							break;
						}else if(MouseType_Table[i].uMouseWheel==MOUSEHWHEEL_RIGHT&&
							 GET_WHEEL_DELTA_WPARAM(wParam)/WHEEL_DELTA>0){
							status::bMouseCommand=true;
							//マウスホイール右
							SendMessage(g_hMainWnd,WM_COMMAND,MAKEWPARAM(g_Config.Mouse.sMouseTable[i].uCmdId,0),0);
							status::bMouseCommand=false;
							break;
						}else if(MouseType_Table[i].uMouseWheel==MOUSEWHEEL_NO){
							status::bMouseCommand=true;
							//マウスホイールなし
							SendMessage(g_hMainWnd,WM_COMMAND,MAKEWPARAM(g_Config.Mouse.sMouseTable[i].uCmdId,0),0);
							status::bMouseCommand=false;
							break;
						}
					}
				}
			}
			break;
		}

		case WM_COMMAND:
			switch(LOWORD(wParam)){
				case IDM_KEY_SWITCH:{
					//切り替え
					if(status::eWindowList!=status::WINDOWLIST::DISPLAYED)break;

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
					if(status::eWindowList!=status::WINDOWLIST::DISPLAYED)break;
					//切り替えをキャンセル
					HideListView();
					break;

				case IDM_KEY_PREV:{
					//前のアイテムを選択
					int iIndex=ListView_GetSelectedItem(g_hListView);

					if((status::bBackground||g_Config.DirectWrite.bDirectWrite)&&
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

					if((status::bBackground||g_Config.DirectWrite.bDirectWrite)&&
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
					if((status::bBackground||g_Config.DirectWrite.bDirectWrite)&&
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
					if((status::bBackground||g_Config.DirectWrite.bDirectWrite)&&
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
					if(status::eWindowList!=status::WINDOWLIST::DISPLAYED){
						PostMessage(g_hMainWnd,WM_COMMAND,IDM_MENU_SETTINGS,0);
					}
					break;

				case IDM_KEY_QUIT:
					//終了
					status::bDisable=true;
					HideListView();
					PostMessage(g_hMainWnd,WM_CLOSE,0,0);
					break;

				case IDM_KEY_SWAP:
					//カラム入れ替え
					if(ListView_GetColumnCount(g_ListViewColumn_Table)!=LISTITEM_NUM)return false;
					ListView_ChangeColumnOrder(g_hListView);
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
					int iMatchMode=tempconf::iMatchMode;
					tempconf::iMatchMode=MATCH_FORWARD;
					//再びインクリメンタルサーチ
					if(!ReMatch()){
						//操作取り消し
						tempconf::iMatchMode=iMatchMode;
						ReMatch();
					}
					break;
				}

				case IDM_KEY_PARTIAL_MATCH:{
					//部分一致検索
					int iMatchMode=tempconf::iMatchMode;
					tempconf::iMatchMode=MATCH_PARTICAL;
					//再びインクリメンタルサーチ
					if(!ReMatch()){
						//操作取り消し
						tempconf::iMatchMode=iMatchMode;
						ReMatch();
					}
					break;
				}

				case IDM_KEY_FLEX_MATCH:{
					//あいまい検索
					int iMatchMode=tempconf::iMatchMode;
					tempconf::iMatchMode=MATCH_FLEX;
					//再びインクリメンタルサーチ
					if(!ReMatch()){
						//操作取り消し
						tempconf::iMatchMode=iMatchMode;
						ReMatch();
					}
					break;
				}

				case IDM_KEY_SEARCHSCOPE:
					//検索項目範囲切り替え
					tempconf::bFirstColumnOnly^=true;
					//再びインクリメンタルサーチ
					if(!ReMatch()){
						//操作取り消し
						tempconf::bFirstColumnOnly^=true;
						ReMatch();
					}
					break;

				case IDM_KEY_UNIQUE_WINDOW:
					//候補が1つなら切り替える
					tempconf::bEnterUniqueWindow^=true;

					if(tempconf::bEnterUniqueWindow){
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

				case IDM_KEY_RESET:{
					//先頭選択、インクリメンタルサーチ終了
					ListView_SelectNone(g_hListView);
					ShowSearchWnd();
					//ツールチップ非表示
					ShowInfoToolTip();
					//一時的な設定をリセット
					tempconf::Reset();
					//インクリメンタルサーチ終了
					for(int i=0;i<lstrlen(g_szSearchString);++i){
						g_szSearchString[i]='\0';
					}
					//表示を元に戻す
					for(int i=0;i<g_iWindowCount;++i){
						g_DisplayWindowInfo[i]=&g_WindowInfo[i];
					}
					g_iDisplayWindowCount=g_iWindowCount;

					FilterItemType();
					//リセット時にアイテムが1つであっても確定させない
					bool bEnterUniqueWindow=tempconf::bEnterUniqueWindow;
					tempconf::bEnterUniqueWindow=false;
					//再び絞り込む
					IncrementalSearch(g_szSearchString);
					tempconf::bEnterUniqueWindow=bEnterUniqueWindow;
					break;
				}

				case IDM_KEY_MINIMIZE:{
					//最小化
					int iIndex=ListView_GetSelectedItem(g_hListView);
					if(iIndex<0)break;
					if(g_DisplayWindowInfo[iIndex]->bWebBrowserTabItem)break;
					if(!IsHungAppWindow(g_DisplayWindowInfo[iIndex]->hWnd)/*&&
					   IsWindowEnabled(g_DisplayWindowInfo[iIndex]->hWnd)*/){
						HWND hWndBak=g_DisplayWindowInfo[iIndex]->hWnd;

						HideListView();
//						PostMessage(hWndBak,WM_SYSCOMMAND,SC_MINIMIZE,0);
						ShowWindow(hWndBak,SW_MINIMIZE);
					}
					HideListView();
					break;
				}

				case IDM_KEY_MAXIMIZE:{
					//最大化
					int iIndex=ListView_GetSelectedItem(g_hListView);
					if(iIndex<0)break;
					if(g_DisplayWindowInfo[iIndex]->bWebBrowserTabItem)break;
					if(!IsHungAppWindow(g_DisplayWindowInfo[iIndex]->hWnd)/*&&
					   IsWindowEnabled(g_DisplayWindowInfo[iIndex]->hWnd)*/){
						HWND hWndBak=g_DisplayWindowInfo[iIndex]->hWnd;

						HideListView();
//						PostMessage(hWndBak,WM_SYSCOMMAND,SC_MAXIMIZE,0);
						ShowWindow(hWndBak,SW_MAXIMIZE);
					}
					HideListView();
					break;
				}

				case IDM_KEY_RESTORE:{
					//大きさを元に戻す
					int iIndex=ListView_GetSelectedItem(g_hListView);
					if(iIndex<0)break;
					if(g_DisplayWindowInfo[iIndex]->bWebBrowserTabItem)break;
					if(!IsHungAppWindow(g_DisplayWindowInfo[iIndex]->hWnd)/*&&
					   IsWindowEnabled(g_DisplayWindowInfo[iIndex]->hWnd)*/){
						HWND hWndBak=g_DisplayWindowInfo[iIndex]->hWnd;

						HideListView();
//						PostMessage(hWndBak,WM_SYSCOMMAND,SC_RESTORE,0);
						ShowWindow(hWndBak,SW_RESTORE);
					}
					HideListView();
					break;
				}

				case IDM_KEY_MOVE:{
					//移動
					int iIndex=ListView_GetSelectedItem(g_hListView);
					if(iIndex<0)break;
					if(g_DisplayWindowInfo[iIndex]->bWebBrowserTabItem)break;
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
					if(g_DisplayWindowInfo[iIndex]->bWebBrowserTabItem)break;
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
					if(g_DisplayWindowInfo[iIndex]->bWebBrowserTabItem)break;
					if(!IsHungAppWindow(g_DisplayWindowInfo[iIndex]->hWnd)/*&&
					   IsWindowEnabled(g_DisplayWindowInfo[iIndex]->hWnd)*/){
						HWND hWndBak=g_DisplayWindowInfo[iIndex]->hWnd;
						bool bIsZoomed=IsZoomed(hWndBak)!=0;

						TaskSwitch(g_DisplayWindowInfo[iIndex]);
						HideListView();

						POINT pt={};

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
					if(g_DisplayWindowInfo[iIndex]->bWebBrowserTabItem)break;
					if(!IsHungAppWindow(g_DisplayWindowInfo[iIndex]->hWnd)/*&&
					   IsWindowEnabled(g_DisplayWindowInfo[iIndex]->hWnd)*/){
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
					if(g_DisplayWindowInfo[iIndex]->bWebBrowserTabItem)break;
					if(!IsHungAppWindow(g_DisplayWindowInfo[iIndex]->hWnd)/*&&
					   IsWindowEnabled(g_DisplayWindowInfo[iIndex]->hWnd)*/){
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
					if(g_DisplayWindowInfo[iIndex]->bWebBrowserTabItem){
						//ブラウザタブを閉じる
						g_pWebBrowserTab[g_DisplayWindowInfo[iIndex]->iWebBrowserIndex].Close(g_DisplayWindowInfo[iIndex]->szId);
					}else if(!IsHungAppWindow(g_DisplayWindowInfo[iIndex]->hWnd)/*&&
					   IsWindowEnabled(g_DisplayWindowInfo[iIndex]->hWnd)*/){
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
//					if(IsWindowEnabled(hSelectedWnd)){
						DWORD dwProcessId=0;

						if(g_DisplayWindowInfo[iIndex]->bWebBrowserTabItem){
							dwProcessId=g_DisplayWindowInfo[iIndex]->dwProcessId;
						}else{
							GetWindowThreadProcessId(hSelectedWnd,&dwProcessId);
						}

						if(dwProcessId){
							TerminateProcess(dwProcessId);
						}
//					}
					HideListView();
					break;
				}

				case IDM_KEY_TERMINATETREE:{
					//プロセスツリーを強制終了
					int iIndex=ListView_GetSelectedItem(g_hListView);
					if(iIndex<0)break;

					HWND hSelectedWnd=(!g_DisplayWindowInfo[iIndex]->bUWPApp)?g_DisplayWindowInfo[iIndex]->hWnd:g_DisplayWindowInfo[iIndex]->hUICoreWnd;
//					if(IsWindowEnabled(hSelectedWnd)){
						DWORD dwProcessId=0;

						if(g_DisplayWindowInfo[iIndex]->bWebBrowserTabItem){
							dwProcessId=g_DisplayWindowInfo[iIndex]->dwProcessId;
						}else{
							GetWindowThreadProcessId(hSelectedWnd,&dwProcessId);
						}

						if(dwProcessId){
							PROCESSENTRY32* pProcessEntry32=AllocProcessList();

							if(pProcessEntry32==NULL)break;
							TerminateProcessTree(pProcessEntry32,dwProcessId);
							FreeProcessList(&pProcessEntry32);
						}
//					}
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

						if(!tempconf::bMigemoSearch||g_Config.IncrementalSearch.iMigemoDelay==0){
							//表示を元に戻す
							for(int i=0;i<g_iWindowCount;++i){
								g_DisplayWindowInfo[i]=&g_WindowInfo[i];
							}
							g_iDisplayWindowCount=g_iWindowCount;

							//再び絞り込む
							IncrementalSearch(g_szSearchString,tempconf::iMatchMode,tempconf::bFirstColumnOnly);
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
							ShowSearchWnd(g_szSearchString);
							SetTimer(g_hMainWnd,wParam,g_Config.IncrementalSearch.iMigemoDelay,NULL);
							break;
						}
					}
					if(g_szSearchString[0]=='\0'){
						ShowSearchWnd();
						//ツールチップ非表示
						ShowInfoToolTip();
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
//					if(IsWindowEnabled(hSelectedWnd)){
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

						if(!g_DisplayWindowInfo[iIndex]->bWebBrowserTabItem){
							GetWindowThreadProcessId(hSelectedWnd,&dwProcessId);
						}else{
							dwProcessId=g_DisplayWindowInfo[iIndex]->dwProcessId;
						}
						SetPriorityClass(dwProcessId,dwPriorityClass);
//					}
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
//					   IsWindowEnabled(hSelectedWnd)&&
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
						FreeProcessList(&pProcessEntry32);

						//ツールチップ非表示
						ShowInfoToolTip();

						RECT rc={};
						ListView_GetSubItemRect(g_hListView,iIndex,ListView_GetSubItemIndex(g_hListView,1),LVIR_LABEL,&rc);

						POINT pt={rc.left,rc.top};
						if(g_Config.ListView.bPopupMenuCursorPos&&status::bMouseCommand){
							//カーソル位置にメニューを表示
							GetCursorPos(&pt);
						}else{
							ClientToScreen(g_hListView,&pt);
						}

						DWORD dwHoverProcessId=0;
						bool bMouseOut=(GetWindowThreadProcessId(ControlFromPoint(),&dwHoverProcessId)!=0&&
										dwHoverProcessId!=GetCurrentProcessId());

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
								RECT rcListView;

								GetWindowRect(g_hListView,&rcListView);
								ClipCursor(&rcListView);
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
									PROCESSENTRY32* pProcessTree=AllocProcessList();

									if(pProcessTree==NULL)break;
									TerminateProcessTree(pProcessTree,(DWORD)ulSelectedProcessId);
									FreeProcessList(&pProcessTree);

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
									FreeProcessCommandLine(&pszCommandLine);
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
									HWND hTargetWnd=GetTopWindow(NULL);

									do{
										if(!IsHungAppWindow(hTargetWnd)&&
											GetWindowLongPtr(hTargetWnd,GWLP_HWNDPARENT)==NULL&&
											IsWindowAvailable(hTargetWnd)&&
											IsWindowEnabled(hTargetWnd)){
											DWORD dwTargetProcessId=0;

											GetWindowThreadProcessId(hTargetWnd,&dwTargetProcessId);
											if(ulSelectedProcessId==dwTargetProcessId)break;
										}
									}while((hTargetWnd=GetNextWindow(hTargetWnd,GW_HWNDNEXT))!=NULL);
									if(hTargetWnd!=NULL&&
									   !IsHungAppWindow(hTargetWnd)){
										InternalGetWindowText(hTargetWnd,szWindowTitle,MAX_PATH);
									}else{
										lstrcpy(szWindowTitle, _T(""));
									}

									//コマンドライン
									TCHAR* pszCommandLine=AllocProcessCommandLine((DWORD)ulSelectedProcessId);

									WINDOWPLACEMENT wndpl={};
									wndpl.length=sizeof(WINDOWPLACEMENT);

									GetWindowPlacement(hTargetWnd,&wndpl);

									//最前面表示か否か
									if(IsWindowTopMost(hTargetWnd)){
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
														  24+//ulSelectedProcessId
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
											 //ulSelectedProcessId
											 _T("PID=%I64d\n")
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
											 ulSelectedProcessId,
											 szTopMost,
											 szShowCmd,
											 szPriority,
											 szElevated);

									FreeProcessCommandLine(&pszCommandLine);

									TOOLINFO ti={sizeof(TOOLINFO)};

									ti.lpszText=pszText;
									ti.hwnd=g_hListView;
									ti.uId=ID_TOOLTIP_INFO;

									RECT rcItem={};

									ListView_GetSubItemRect(g_hListView,iIndex,ListView_GetSubItemIndex(g_hListView,1),LVIR_LABEL,&rcItem);

									POINT ptToolTip={rcItem.left,rcItem.top};

									ClientToScreen(g_hListView,&ptToolTip);
									//ツールチップ文字列更新
									SendMessage(g_hInfoToolTip,TTM_UPDATETIPTEXT,0,(LPARAM)&ti);
									//ツールチップ表示位置
									SendMessage(g_hInfoToolTip,TTM_TRACKPOSITION,0,(LPARAM)MAKELONG(ptToolTip.x,ptToolTip.y));
									//改行できるように幅を設定
									SendMessage(g_hInfoToolTip,TTM_SETMAXTIPWIDTH,(WPARAM)0,(LPARAM)0);
									//ツールチップを表示
									SendMessage(g_hInfoToolTip,TTM_TRACKACTIVATE,true,(LPARAM)&ti);

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
//						IsWindowEnabled(hSelectedWnd)&&
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
//					   IsWindowEnabled(g_DisplayWindowInfo[iIndex]->hWnd)&&
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
//						IsWindowEnabled(hSelectedWnd)&&
						GetWindowThreadProcessId(hSelectedWnd,&dwProcessId)!=0){
						TCHAR* pszCommandLine=AllocProcessCommandLine(dwProcessId);
						if(pszCommandLine!=NULL){
							SetClipboardText(g_hMainWnd,pszCommandLine,lstrlen(pszCommandLine));
							FreeProcessCommandLine(&pszCommandLine);
						}
					}
					break;
				}

				case IDM_KEY_ITEMTYPE_WINDOW:{
					//リストにウインドウを表示/非表示
					tempconf::bWindowItem^=true;

					FilterItemType();

					if(g_iDisplayWindowCount==0){
						//フィルタの結果アイテム数が0ならウインドウアイテムを表示させる
						tempconf::bWindowItem=true;
						FilterItemType();
					}

					//再び絞り込む
					IncrementalSearch(g_szSearchString);

					break;
				}
				case IDM_KEY_ITEMTYPE_WEBBROWSERTAB:{
					//リストにウェブブラウザタブを表示/非表示
					tempconf::bWebBrowserTabItem^=true;

					FilterItemType();

					if(g_iDisplayWindowCount==0){
						//フィルタの結果アイテム数が0ならウインドウアイテムを表示させる
						tempconf::bWindowItem=true;
						FilterItemType();
					}

					//再び絞り込む
					IncrementalSearch(g_szSearchString);
					break;
				}

				case IDM_KEY_INFO:{
					//情報を表示
					int iIndex=ListView_GetSelectedItem(g_hListView);
					if(iIndex<0)break;
					DWORD dwProcessId=0;
					HWND hSelectedWnd=(!g_DisplayWindowInfo[iIndex]->bUWPApp)?g_DisplayWindowInfo[iIndex]->hWnd:g_DisplayWindowInfo[iIndex]->hUICoreWnd;

					if(g_DisplayWindowInfo[iIndex]->bWebBrowserTabItem){
						TCHAR* pszText=(TCHAR*)HeapAlloc(GetProcessHeap(),
														 HEAP_ZERO_MEMORY,
														 (
														  lstrlen(g_DisplayWindowInfo[iIndex]->szFilePath)+
														  lstrlen(g_DisplayWindowInfo[iIndex]->szWindowTitle)+
														  lstrlen(g_DisplayWindowInfo[iIndex]->szURL)+
														  24+//dwProcessId
														  lstrlen(g_DisplayWindowInfo[iIndex]->szId)+
														  12)*sizeof(TCHAR));
						wsprintf(pszText,
								 //g_DisplayWindowInfo[iIndex]->szFilePath,
								 _T("%s\n")
								 //g_DisplayWindowInfo[iIndex]->szWindowTitle,
								 _T("%s\n")
								 //g_DisplayWindowInfo[iIndex]->szURL,
								 _T("%s\n")
								 //g_DisplayWindowInfo[iIndex]->dwProcessId
								 _T("PID=%d\n")
								 //g_DisplayWindowInfo[iIndex]->szId,
								 _T("ID=%s"),
								 g_DisplayWindowInfo[iIndex]->szFilePath,
								 g_DisplayWindowInfo[iIndex]->szWindowTitle,
								 g_DisplayWindowInfo[iIndex]->szURL,
								 g_DisplayWindowInfo[iIndex]->dwProcessId,
								 g_DisplayWindowInfo[iIndex]->szId);

						TOOLINFO ti={};
						ti.cbSize=sizeof(TOOLINFO);
						ti.lpszText=pszText;
						ti.hwnd=g_hListView;
						ti.uId=ID_TOOLTIP_INFO;

						RECT rcItem={};

						ListView_GetSubItemRect(g_hListView,iIndex,ListView_GetSubItemIndex(g_hListView,1),LVIR_LABEL,&rcItem);

						POINT ptToolTip={rcItem.left,rcItem.top};

						ClientToScreen(g_hListView,&ptToolTip);
						//ツールチップ文字列更新
						SendMessage(g_hInfoToolTip,TTM_UPDATETIPTEXT,0,(LPARAM)&ti);
						//ツールチップ表示位置
						SendMessage(g_hInfoToolTip,TTM_TRACKPOSITION,0,(LPARAM)MAKELONG(ptToolTip.x,ptToolTip.y));
						//改行できるように幅を設定
						SendMessage(g_hInfoToolTip,TTM_SETMAXTIPWIDTH,(WPARAM)0,(LPARAM)0);
						//ツールチップを表示
						SendMessage(g_hInfoToolTip,TTM_TRACKACTIVATE,true,(LPARAM)&ti);

						HeapFree(GetProcessHeap(),0,pszText);
						pszText=NULL;
					}else if(!IsHungAppWindow(hSelectedWnd)&&
//						IsWindowEnabled(hSelectedWnd)&&
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

						WINDOWPLACEMENT wndpl={};
						wndpl.length=sizeof(WINDOWPLACEMENT);

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
														  24+//dwProcessId
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
								 //dwProcessId
								 _T("PID=%d\n")
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
								 dwProcessId,
								 szTopMost,
								 szShowCmd,
								 szPriority,
								 szElevated);

						FreeProcessCommandLine(&pszCommandLine);

						TOOLINFO ti={};
						ti.cbSize=sizeof(TOOLINFO);
						ti.lpszText=pszText;
						ti.hwnd=g_hListView;
						ti.uId=ID_TOOLTIP_INFO;

						RECT rcItem={};

						ListView_GetSubItemRect(g_hListView,iIndex,ListView_GetSubItemIndex(g_hListView,1),LVIR_LABEL,&rcItem);

						POINT ptToolTip={rcItem.left,rcItem.top};

						ClientToScreen(g_hListView,&ptToolTip);
						//ツールチップ文字列更新
						SendMessage(g_hInfoToolTip,TTM_UPDATETIPTEXT,0,(LPARAM)&ti);
						//ツールチップ表示位置
						SendMessage(g_hInfoToolTip,TTM_TRACKPOSITION,0,(LPARAM)MAKELONG(ptToolTip.x,ptToolTip.y));
						//改行できるように幅を設定
						SendMessage(g_hInfoToolTip,TTM_SETMAXTIPWIDTH,(WPARAM)0,(LPARAM)0);
						//ツールチップを表示
						SendMessage(g_hInfoToolTip,TTM_TRACKACTIVATE,true,(LPARAM)&ti);

						HeapFree(GetProcessHeap(),0,pszText);
						pszText=NULL;
					}
					break;
				}

				case IDM_KEY_THUMBNAIL:
					//サムネイルを表示
					if(tempconf::uThumbnail){
						tempconf::uThumbnailBak=tempconf::uThumbnail;
						tempconf::uThumbnail=THUMBNAIL_NO;
					}else{
						tempconf::uThumbnail=tempconf::uThumbnailBak;
					}
					if(tempconf::uThumbnail){
						int iIndex=ListView_GetSelectedItem(g_hListView);
						if(iIndex<0)break;

						ShowThumbnail();
					}else{
						ShowThumbnail(false);
					}
					break;

				case IDM_KEY_THUMBNAIL_ORIGINAL:
					tempconf::bThumbnailOriginalSize^=true;
					if(tempconf::uThumbnail){
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
					const UINT uOpacity[]={
						IDM_KEY_OPACITY0,
						IDM_KEY_OPACITY10,
						IDM_KEY_OPACITY20,
						IDM_KEY_OPACITY30,
						IDM_KEY_OPACITY40,
						IDM_KEY_OPACITY50,
						IDM_KEY_OPACITY60,
						IDM_KEY_OPACITY70,
						IDM_KEY_OPACITY80,
						IDM_KEY_OPACITY90,
						IDM_KEY_OPACITY100,
						IDM_KEY_OPACITYTOGGLE
					};

					int iOpacityIndex=1;
					for(;iOpacityIndex<ARRAY_SIZEOF(uOpacity);iOpacityIndex++){
						if(LOWORD(wParam)==uOpacity[iOpacityIndex]){
							break;
						}
					}
					//不透明度
					int iIndex=ListView_GetSelectedItem(g_hListView);
					if(iIndex<0)break;

					if(!IsHungAppWindow(g_DisplayWindowInfo[iIndex]->hWnd)/*&&
					   IsWindowEnabled(g_DisplayWindowInfo[iIndex]->hWnd)*/){
						if(uOpacity[iOpacityIndex]==IDM_KEY_OPACITYTOGGLE){
							BYTE byOpacity=255;

							GetLayeredWindowAttributes(g_DisplayWindowInfo[iIndex]->hWnd,NULL,&byOpacity,NULL);
							if(byOpacity!=255){
								byOpacity=100;
							}else{
								byOpacity=tempconf::byOpacity;
							}
							SendMessage(hWnd,WM_COMMAND,MAKEWPARAM(byOpacity/10+IDM_KEY_OPACITY0,0),0);
							break;
						}else if(uOpacity[iOpacityIndex]!=IDM_KEY_OPACITY100){
							SetWindowLongPtr(g_DisplayWindowInfo[iIndex]->hWnd,GWL_EXSTYLE,GetWindowLongPtr(g_DisplayWindowInfo[iIndex]->hWnd,GWL_EXSTYLE)|WS_EX_LAYERED);
							tempconf::byOpacity=(BYTE)(uOpacity[iOpacityIndex]-uOpacity[0])*10;
							SetLayeredWindowAttributes(g_DisplayWindowInfo[iIndex]->hWnd,0,tempconf::byOpacity*255/100,LWA_ALPHA);
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
					if(!IsHungAppWindow(g_DisplayWindowInfo[iIndex]->hWnd)/*&&
					   IsWindowEnabled(g_DisplayWindowInfo[iIndex]->hWnd)*/){
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
					if(g_DisplayWindowInfo[iIndex]->bWebBrowserTabItem)break;
					if(!IsHungAppWindow(g_DisplayWindowInfo[iIndex]->hWnd)/*&&
					   IsWindowEnabled(g_DisplayWindowInfo[iIndex]->hWnd)*/){
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
					if(g_DisplayWindowInfo[iIndex]->bWebBrowserTabItem)break;

					HWND hSelectedWnd=g_DisplayWindowInfo[iIndex]->hWnd;

					if(!IsHungAppWindow(hSelectedWnd)/*&&
					   IsWindowEnabled(hSelectedWnd)*/){
						HMONITOR hCurrentMonitor=MonitorFromWindow(hSelectedWnd,MONITOR_DEFAULTTONEAREST);
						MONITORINFO info={};
						info.cbSize=sizeof(info);
						GetMonitorInfo(hCurrentMonitor,&info);

						MONITOR_INFO* pMonitorInfo=AllocMonitorsList();

						if(pMonitorInfo==NULL)break;
						DestroyMenu(hMoveToMonitorMenu);
						hMoveToMonitorMenu=CreatePopupMenu();

						MENUITEMINFO mii={};
						mii.cbSize=sizeof(MENUITEMINFO);
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

						FreeMonitorsList(&pMonitorInfo);

						if(!GetMenuItemCount(hMoveToMonitorMenu))break;

						//ツールチップ非表示
						ShowInfoToolTip();

						RECT rc={};
						ListView_GetSubItemRect(g_hListView,iIndex,ListView_GetSubItemIndex(g_hListView,1),LVIR_LABEL,&rc);

						POINT pt={rc.left,rc.top};
						if(g_Config.ListView.bPopupMenuCursorPos&&status::bMouseCommand){
							//カーソル位置にメニューを表示
							GetCursorPos(&pt);
						}else{
							ClientToScreen(g_hListView,&pt);
						}

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
								RECT rcListView;

								GetWindowRect(g_hListView,&rcListView);
								ClipCursor(&rcListView);
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
					ToggleDesktop();
					break;
				}

				//メインメニュー
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

					//ツールチップ非表示
					ShowInfoToolTip();

					RECT rc={};
					ListView_GetSubItemRect(g_hListView,iIndex,ListView_GetSubItemIndex(g_hListView,1),LVIR_LABEL,&rc);

					POINT pt={rc.left,rc.top};
					if(g_Config.ListView.bPopupMenuCursorPos&&status::bMouseCommand){
						//カーソル位置にメニューを表示
						GetCursorPos(&pt);
					}else{
						ClientToScreen(g_hListView,&pt);
					}

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
						case IDM_KEY_ITEMTYPEMENU:
							phMenu=&mItemTypeCmd.hSubMenu;
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
						if(status::eWindowList==status::WINDOWLIST::DISPLAYED&&
						   g_Config.ListView.bMouseOut){
							//カーソルがウインドウリスト外にあるなら右上へ移動させる
							if(!bMouseOut&&GetWindowThreadProcessId(ControlFromPoint(),&dwProcessId)!=0&&
							   dwProcessId!=GetCurrentProcessId()){
								RECT rcListView;

								GetWindowRect(g_hListView,&rcListView);
								ClipCursor(&rcListView);
								ClipCursor(NULL);
							}
						}
					}
					break;
				}


				case IDM_MENU_SHOW:
					//表示
					if(status::eWindowList==status::WINDOWLIST::HIDE&&!status::bDisable&&!status::bShowSettingsDialog){
						ShowListView();
					}
					break;

				case IDM_MENU_DISABLE:
					//無効
					status::bDisable=!status::bDisable;

					if(!status::bDisable){
						//ホットキーを登録する
						RegistHotKey(hWnd,ID_HOTKEY,&g_Config.ShowWindow.wHotKey);
					}else{
						//ホットキーを解除する
						UnregistHotKey(hWnd,ID_HOTKEY,&g_Config.ShowWindow.wHotKey);
					}
					TaskTray(hWnd,(!status::bDisable)?hTrayIcon:hDisableTrayIcon,NIM_MODIFY);
					break;

				case IDM_MENU_SETTINGS:
				case IDM_MENU_VERSION:{
					//設定
					//バージョン情報
					if(!status::bShowSettingsDialog){
						//ホットキーを解除する
						if(g_Config.ShowWindow.wHotKey)UnregistHotKey(hWnd,ID_HOTKEY,&g_Config.ShowWindow.wHotKey);
						//WM_INPUT受け取り終了
						UnregisterRawInput();

						//Ctrl+Aフックインストール
						HHOOK hkCtrlA=SetWindowsHookEx(WH_MSGFILTER,CtrlAHookProc,NULL,GetCurrentThreadId());

						status::bShowSettingsDialog=true;
						//設定ダイアログを表示
						ShowSettingsDialog((LOWORD(wParam)==IDM_MENU_SETTINGS)?TAB_GENERAL:TAB_VERSION);
						status::bShowSettingsDialog=false;

						//Ctrl+Aフックアンインストール
						UnhookWindowsHookEx(hkCtrlA);

						//ホットキーを登録する
						if(g_Config.ShowWindow.wHotKey){
							RegistHotKey(hWnd,ID_HOTKEY,&g_Config.ShowWindow.wHotKey);
						}

						//一時的な設定をリセット
						tempconf::Reset();

						//フォントを再読み込み
						ReloadFont();

						//Bitmapを解放
						if(g_pDirectWriteListView!=NULL){
							g_pDirectWriteListView->ReleaseTextObject();
							g_pDirectWriteListView->ReleaseBackgroundImage();
						}
						if(g_pGdiplusBackgroundImage!=NULL){
							g_pGdiplusBackgroundImage->Release();
						}
						status::bBackground=false;

						if(g_Config.DirectWrite.bDirectWrite){
							if(g_pDirectWriteListView!=NULL&&
							   !g_pDirectWriteListView->IsAvailable()){
								direct2d::Load();
								if(!direct2d::IsLoaded()){
									g_Config.DirectWrite.bDirectWrite=false;
								}else{
									if(g_pDirectWriteListView!=NULL){
										g_pDirectWriteListView->Initialize();
									}
								}
							}
							if(g_pDirectWriteListView!=NULL&&
							   g_pDirectWriteListView->IsAvailable()){
								//ResizeListView()でResize()するため大きさは0,0指定
								g_pDirectWriteListView->CreateRenderTarget(g_hListView,0,0);
								ReleaseBrush();
								CreateBrush();
							}
						}else{
							if(g_pDirectWriteListView!=NULL){
								g_pDirectWriteListView->Uninitialize();
							}
							direct2d::Unload();
						}

						//Migemo読み込み/解放
						if(g_Config.IncrementalSearch.iMigemoMode==MIGEMO_NO){
							migemo::Unload();
						}else{
							if(g_Config.IncrementalSearch.bNoUnloadMigemo)migemo::Load();
						}

						//アクセラレータテーブルを読み込む
						g_hAccel=UpdateAcceleratorTable(g_hAccel,g_Config.ShortcutKey.sKeyTable);

						//不透明度を設定
						if(g_Config.ListView.byOpacity!=100){
							SetWindowLongPtr(hWnd,GWL_EXSTYLE,GetWindowLongPtr(hWnd,GWL_EXSTYLE)|WS_EX_LAYERED);
							SetLayeredWindowAttributes(hWnd,0,g_Config.ListView.byOpacity*255/100,LWA_ALPHA);
						}else{
							SetWindowLongPtr(hWnd,GWL_EXSTYLE,GetWindowLongPtr(hWnd,GWL_EXSTYLE)&~WS_EX_LAYERED);
						}

						if(g_Config.IncrementalSearch.byOpacity!=100){
							SetWindowLongPtr(g_hSearchWnd,GWL_EXSTYLE,GetWindowLongPtr(g_hSearchWnd,GWL_EXSTYLE)|WS_EX_LAYERED);
							SetLayeredWindowAttributes(g_hSearchWnd,0,g_Config.IncrementalSearch.byOpacity*255/100,LWA_ALPHA);
						}else{
							SetWindowLongPtr(g_hSearchWnd,GWL_EXSTYLE,GetWindowLongPtr(g_hSearchWnd,GWL_EXSTYLE)&~WS_EX_LAYERED);
						}

						//イメージリストの初期化
						InitializeImageList();

						//壁紙設定
						if(lstrlen(g_Config.Background.szImagePath)){
							if(g_Config.DirectWrite.bDirectWrite&&
							   g_pDirectWriteListView!=NULL&&
							   g_pDirectWriteListView->IsAvailable()){
								g_pDirectWriteListView->LoadBackgroundImage(g_Config.Background.szImagePath,
																		  g_Config.Background.byResizePercent,
																		  g_Config.Background.byOpacity);
								status::bBackground=g_pDirectWriteListView->IsLoadedBackgroundImage();
							}else if(g_pGdiplusBackgroundImage!=NULL){
								status::bBackground=g_pGdiplusBackgroundImage->Load(g_Config.Background.szImagePath);
							}
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

				default:{
					//コマンド
					bool bMatch=false;
					int iCommandIndex=1;
					for(;iCommandIndex<ARRAY_SIZEOF(uCommandIndex);iCommandIndex++){
						if(LOWORD(wParam)==uCommandIndex[iCommandIndex]){
							bMatch=true;
							break;
						}
					}
					if(bMatch){
						if(lstrlen(g_Config.Command[iCommandIndex].szCommandName)&&
						   lstrlen(g_Config.Command[iCommandIndex].szFilePath)){
							int iIndex=ListView_GetSelectedItem(g_hListView);
							TCHAR szParameters[MAX_PATH];

							lstrcpy(szParameters,g_Config.Command[iCommandIndex].szParameters);
							if(lstrlen(szParameters)&&iIndex>=0){
								DWORD dwProcessId=0;
								TCHAR szFilePath[MAX_PATH];
								HWND hSelectedWnd=(!g_DisplayWindowInfo[iIndex]->bUWPApp)?g_DisplayWindowInfo[iIndex]->hWnd:g_DisplayWindowInfo[iIndex]->hUICoreWnd;
								if(g_DisplayWindowInfo[iIndex]->bWebBrowserTabItem){
									dwProcessId=g_DisplayWindowInfo[iIndex]->dwProcessId;
								}else if(!IsHungAppWindow(hSelectedWnd)&&
//								   IsWindowEnabled(hSelectedWnd)&&
								   GetWindowThreadProcessId(hSelectedWnd,&dwProcessId)!=0){

									if(!GetProcessFileName(dwProcessId,szFilePath,MAX_PATH)){
										lstrcpy(szFilePath,_T(""));
									}
								}

								if(StrStr(szParameters,_T("%F"))){
									//フルパス
									if(lstrlen(szFilePath)){
										do{
											StrReplace(szParameters,_T("%F"),szFilePath);
										}while(StrStr(szParameters,_T("%F")));
									}
								}
								if(StrStr(szParameters,_T("%D"))){
									//ディレクトリパス
									TCHAR* p=NULL;

									if((p=StrRChr(szFilePath,&szFilePath[lstrlen(szFilePath)],'\\'))!=NULL){
										TCHAR szDirectory[MAX_PATH];

										lstrcpyn(szDirectory,szFilePath,(int)(p-szFilePath+1));
										do{
											StrReplace(szParameters,_T("%D"),szDirectory);
										}while(StrStr(szParameters,_T("%D")));
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
									do{
										StrReplace(szParameters,_T("%N"),szFileName);
									}while(StrStr(szParameters,_T("%N")));
								}
								if(StrStr(szParameters,_T("%T"))){
									//タイトル
									do{
										StrReplace(szParameters,_T("%T"),g_DisplayWindowInfo[iIndex]->szWindowTitle);
									}while(StrStr(szParameters,_T("%T")));
								}
								if(StrStr(szParameters,_T("%H"))){
									if(g_DisplayWindowInfo[iIndex]->hWnd!=NULL){
										//ハンドル(10進数)
										TCHAR szHandle[128];
										wsprintf(szHandle,_T("%d"),g_DisplayWindowInfo[iIndex]->hWnd);
										do{
											StrReplace(szParameters,_T("%H"),szHandle);
										}while(StrStr(szParameters,_T("%H")));
									}
								}
								if(StrStr(szParameters,_T("%X"))){
									if(g_DisplayWindowInfo[iIndex]->hWnd!=NULL){
										//ハンドル(16進数)
										TCHAR szHandle[128];
										wsprintf(szHandle,_T("%#x"),g_DisplayWindowInfo[iIndex]->hWnd);
										do{
											StrReplace(szParameters,_T("%X"),szHandle);
										}while(StrStr(szParameters,_T("%X")));
									}
								}
								if(StrStr(szParameters,_T("%P"))){
									//プロセスID
									if(dwProcessId){
										TCHAR szProcessID[128];
										wsprintf(szProcessID,_T("%10lu"),dwProcessId);
										do{
											StrReplace(szParameters,_T("%P"),szProcessID);
										}while(StrStr(szParameters,_T("%P")));
									}
								}
								if(StrStr(szParameters,_T("%C"))){
									//クラス名
									TCHAR szClassName[128];
									GetClassName(g_DisplayWindowInfo[iIndex]->hWnd,szClassName,ARRAY_SIZEOF(szClassName));
									if(lstrlen(szClassName)){
										do{
											StrReplace(szParameters,_T("%C"),szClassName);
										}while(StrStr(szParameters,_T("%C")));
									}
								}
								if(StrStr(szParameters,_T("%U"))){
									//URL(ウェブブラウザタブ選択時のみ)\t%U
									if(lstrlen(g_DisplayWindowInfo[iIndex]->szURL)){
										do{
											StrReplace(szParameters,_T("%U"),g_DisplayWindowInfo[iIndex]->szURL);
										}while(StrStr(szParameters,_T("%U")));
									}
								}
								if(StrStr(szParameters,_T("%%"))){
									//%
									do{
										StrReplace(szParameters,_T("%%"),_T("%"));
									}while(StrStr(szParameters,_T("%%")));
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
			}
			break;

		case WM_CLOSETHREAD:{
			DWORD dwExitCode=0;

			GetExitCodeThread((HANDLE)wParam,&dwExitCode);
			if(dwExitCode==STILL_ACTIVE){
				if(WaitForSingleObject((HANDLE)wParam,THREAD_TIMEOUT)==WAIT_TIMEOUT){
					TerminateThread((HANDLE)wParam,0);
				}
				CloseHandle((HANDLE)wParam);
			}
			break;
		}

		case WM_DPICHANGED:
			ReloadFont();
			if(g_pDirectWriteListView!=NULL&&
			   g_pDirectWriteListView->IsAvailable()){
				ReleaseBrush();
				CreateBrush();
			}
			InitializeImageList();
			break;

		case WM_CLOSE:{
			//タスクトレイアイコン削除
			TaskTray(hWnd,NULL,NIM_DELETE);
			DestroyIcon(hTrayIcon);
			DestroyIcon(hDisableTrayIcon);
			//ホットキーを解除する
			if(g_Config.ShowWindow.wHotKey)UnregistHotKey(hWnd,ID_HOTKEY,&g_Config.ShowWindow.wHotKey);

			//リストビューカラムの並び順を保存
			ListView_GetColumnOrderArray(g_hListView,LISTITEM_NUM,g_Config.ListView.iColumnOrder);

			//イメージリストを削除
			ImageList_Destroy(ListView_GetImageList(g_hListView,LVSIL_SMALL));

			//cfgファイルに書き込む
			WritePrivateProfile();

			//検索文字列解放
			ShowSearchWnd();
			DestroyWindow(g_hSearchWnd);

			//フォント削除
			for(int i=0;i<INCREMENTALSEARCH_SEARCHWINDOWDESIGN_NUM;i++){
				if(g_hSearchWindowFonts[i]){
					DeleteObject(g_hSearchWindowFonts[i]);
					g_hSearchWindowFonts[i]=NULL;
				}
			}

			//アイコン解放
			for(int i=0;i<g_iWindowCount;++i){
				if(g_WindowInfo[i].hIcon!=NULL){
					DestroyIcon(g_WindowInfo[i].hIcon);
					g_WindowInfo[i].hIcon=NULL;
				}
				SAFE_RELEASE(g_WindowInfo[i].pD2DBitmap);
			}

			//ブラシを解放
			ReleaseBrush();

			if(g_pGdiplusBackgroundImage!=NULL){
				//背景画像を削除
				g_pGdiplusBackgroundImage->Release();
			}

			if(g_pDirectWriteListView!=NULL){
				g_pDirectWriteListView->Uninitialize();
			}

			//ここでFreeLibrary(d2d1.dll)するとクラッシュするため呼び出さないこと
			//クラッシュ時の呼び出し履歴は
			// 	user32.dll!DispatchHookW()	不明
			// 	user32.dll!CallHookWithSEH(struct _GENERICHOOKHEADER *,void *,unsigned long *,unsigned __int64)	不明
			// 	user32.dll!__fnHkINLPMSG()	不明
			// 	ntdll.dll!KiUserCallbackDispatcherContinue()	不明
			// ...

			//カーソル解放
			DestroyCursor(g_hBlankCursor);

			ShowThumbnail(false);
			DestroyWindow(g_hThumbnailWnd);

			//メニュー解放
			DestroyMenu(mTray.hMenu);
			DestroyMenu(mMainCmd.hMenu);
			DestroyMenu(mSelectCmd.hMenu);
			DestroyMenu(mCopyInfoCmd.hMenu);
			DestroyMenu(mIncrementalSearchCmd.hMenu);
			DestroyMenu(mWindowCmd.hMenu);
			DestroyMenu(mProcessCmd.hMenu);
			DestroyMenu(mOpacityCmd.hMenu);
			DestroyMenu(mItemTypeCmd.hMenu);
			DestroyMenu(mCommand.hMenu);
			DestroyMenu(hChildProcessMenu);

			DestroyWindow(g_hListView);
			DestroyWindow(hWnd);
			break;
		}

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		default:
			if(uMsg==WM_INPUT){
				if(!status::bDisable&&!status::bShowSettingsDialog){
					if(status::eWindowList==status::WINDOWLIST::HIDE){
						PRAWINPUT pRawInput=NULL;
						UINT uSize=0;

						GetRawInputData((HRAWINPUT)lParam,RID_INPUT,NULL,&uSize,sizeof(RAWINPUTHEADER));

						if(!uSize)break;

						pRawInput=(PRAWINPUT)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,uSize);
						if(!pRawInput)break;
						if(GetRawInputData((HRAWINPUT)lParam,RID_INPUT,pRawInput,&uSize,sizeof(RAWINPUTHEADER))==uSize){
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
					}else if(status::eWindowList==status::WINDOWLIST::DISPLAYED){
						PRAWINPUT pRawInput;
						UINT uSize;

						GetRawInputData((HRAWINPUT)lParam,RID_INPUT,NULL,&uSize,sizeof(RAWINPUTHEADER));

						if(!uSize)break;

						pRawInput=(PRAWINPUT)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,uSize);
						if(!pRawInput)break;
						if(GetRawInputData((HRAWINPUT)lParam,RID_INPUT,pRawInput,&uSize,sizeof(RAWINPUTHEADER))==uSize){
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
						if(status::eWindowList==status::WINDOWLIST::HIDE&&!status::bDisable&&!status::bShowSettingsDialog){
							ShowListView();
						}
						return 0;
					}
					if(lParam==WM_RBUTTONDOWN){
						//タスクトレイアイコンが右クリックされた
						POINT pt={};

						GetCursorPos(&pt);
						SetForegroundWindow(hWnd);
						TrackPopupMenu(mTray.hSubMenu,TPM_BOTTOMALIGN,pt.x,pt.y,0,hWnd,NULL);
					}
				}
			}else if(uMsg==MSG_TRAYCREATED){
				//タスクバーが再作成されたら、トレイアイコンを再度追加
				TaskTray(g_hMainWnd,hTrayIcon,NIM_ADD);
			}
			break;
	}
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

//メッセージループを処理するための条件を追加
bool PreTranslateAccelerator(MSG *pMsg){
	//インクリメンタルサーチ中ならスペース挿入
	if(status::eWindowList==status::WINDOWLIST::DISPLAYED){
		if(pMsg->message==WM_KEYDOWN&&
		   pMsg->wParam==VK_SPACE&&
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
	if(status::eWindowList==status::WINDOWLIST::CREATED&&
	   status::eCorners!=SCC_NONE){
		if(pMsg->message==WM_LBUTTONDOWN||
		   pMsg->message==WM_LBUTTONUP||
		   pMsg->message==WM_RBUTTONDOWN||
		   pMsg->message==WM_RBUTTONUP){
			return false;
		}
	}
	return true;
}

#ifndef _DEBUG
void WinMainCRTStartup(){
#else
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInst,LPSTR lpszCmdLine,int nCmdShow){
#endif
	TCHAR szAppName[]=TASCHER_VERSION;
	TCHAR szMutexName[]=_T("{C0E49B9C-D5CB-4fa9-91F3-152C4A1317E6}");
	TCHAR szClassName[]=_T("TascherWindowClass");
	HWND hWnd=NULL;
	MSG msg={};
	HANDLE hMutex=NULL;
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

	bool(WINAPI*pChangeWindowMessageFilter)(UINT,DWORD)=(bool(WINAPI*)(UINT,DWORD))GetProcAddress(GetModuleHandle(_T("user32")),"ChangeWindowMessageFilter");
	if(pChangeWindowMessageFilter!=NULL){
		//MSGFLT_ADD
		pChangeWindowMessageFilter(MSG_TRAY,1);
	}

	//cfgファイルを読み込む
	ReadPrivateProfile();

	if(g_Config.bRunAsAdministrator&&
	   !IsAdministratorProcess(GetProcessId(GetCurrentProcess()))){
		if(!StrStr(GetCommandLine(),szMutexName)){
			//管理者権限で起動
			SHELLEXECUTEINFO sei={};
			sei.cbSize=sizeof(SHELLEXECUTEINFO);
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

	//タスクバー再作成用メッセージ登録
	MSG_TRAYCREATED=RegisterWindowMessage(_T("TaskbarCreated"));
	//WM_MOUSEMOVE用メッセージ登録
	MSG_MOUSEMOVE=RegisterWindowMessage(_T("MSG_MOUSEMOVE"));
	//WM_MMOUSEWHEEL/WM_MOUSEHWHEEL用メッセージ登録
	MSG_MOUSEWHEEL=RegisterWindowMessage(_T("MSG_MOUSEWHEEL"));

	com::Initialize ComInitialize;

	gdiplus::Initialize GdiplusInitialize;

	//Dwmapi.dll読み込み
	dwm::Load();

	g_pWebBrowserTab=new webbrowsertab::List[MAX_WEBBROWSER+1];
	g_pGdiplusBackgroundImage=new gdiplus::BackgroundImage;
	g_pDirectWriteListView=new direct2d::ListView;

	//d2d1.dll/DWrite.dll読み込み
	if(g_Config.DirectWrite.bDirectWrite){
		direct2d::Load();
		if(!direct2d::IsLoaded()){
			g_Config.DirectWrite.bDirectWrite=false;
		}else{
			if(g_pDirectWriteListView!=NULL){
				g_pDirectWriteListView->Initialize();
			}
		}
	}

	//Migemo,bregonig読み込み
	if(g_Config.IncrementalSearch.iMigemoMode!=MIGEMO_NO&&
	   g_Config.IncrementalSearch.bNoUnloadMigemo){
		migemo::Load();
	}

	//ウインドウ情報用ヒープ作成
	g_hHeapWindowInfo=HeapCreate(NULL,0,0);

	//ウインドウクラスを登録
	WNDCLASSEX wc={};

	wc.cbSize=sizeof(WNDCLASSEX);
	wc.style=0;
	wc.lpfnWndProc=(WNDPROC)WindowProc;
	wc.cbClsExtra=0;
	wc.cbWndExtra=0;
	wc.hInstance=GetModuleHandle(NULL);
	wc.hIcon=LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_ICON1));
	wc.hCursor=NULL;
	wc.hbrBackground=(HBRUSH)HOLLOW_BRUSH;
	wc.lpszMenuName=NULL;
	wc.lpszClassName=szClassName;

	if(!RegisterClassEx(&wc)){
		ExitProcess(0);
	}

	hWnd=CreateWindowEx(WS_EX_TOOLWINDOW|WS_EX_TOPMOST|WS_EX_ACCEPTFILES|WS_EX_WINDOWEDGE,
						szClassName,
						szAppName,
						WS_POPUP|WS_CLIPCHILDREN,
						CW_USEDEFAULT,CW_USEDEFAULT,
						CW_USEDEFAULT,CW_USEDEFAULT,
						NULL,
						NULL,
						GetModuleHandle(NULL),
						NULL);

	if(hWnd==NULL)ExitProcess(0);

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
	migemo::Unload();

	if(g_pDirectWriteListView!=NULL){
		delete g_pDirectWriteListView;
		g_pDirectWriteListView=NULL;
	}

	//d2d1.dll/DWrite.dll解放
	direct2d::Unload();

	if(g_pGdiplusBackgroundImage!=NULL){
		delete g_pGdiplusBackgroundImage;
		g_pGdiplusBackgroundImage=NULL;
	}

	if(g_pWebBrowserTab!=NULL){
		delete[] g_pWebBrowserTab;
	}

	//Dwmapi.dll解放
	dwm::Unload();

	//アクセラレータテーブルを破棄
	DestroyAcceleratorTable(g_hAccel);

	//WM_INPUT受け取り終了
	UnregisterRawInput();

	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
	ExitProcess((UINT)msg.wParam);
}
