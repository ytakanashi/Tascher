//PrivateProfile.cpp
//cfgファイル操作

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.62
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/


#include"StdAfx.h"
#include"PrivateProfile.h"
#include"Function.h"
#include"Path.h"
#include"resources/resource.h"

TCHAR g_szPrivateProfile[MAX_PATH]={};



//cfgファイルパスを取得
void GetPrivateProfilePath(){
	TCHAR szBuffer[32]={};

	path::GetExeDirectory(g_szPrivateProfile,MAX_PATH);
	LoadString(g_hInstance,IDS_FILENAME,(LPTSTR)&szBuffer,sizeof(szBuffer)-1);
	lstrcat(g_szPrivateProfile,_T("\\"));
	lstrcat(g_szPrivateProfile,szBuffer);
	lstrcat(g_szPrivateProfile,_T(".cfg"));
	return;
}

//整数を文字列に変換してPrivateProfileファイルに書き込む
bool WINAPI WritePrivateProfileInt(LPCTSTR lpAppName,LPCTSTR lpKeyName,LONG_PTR lData,LPCTSTR lpFileName){
	TCHAR szValue[256];
	wsprintf(szValue,_T("%d"),lData);
	return WritePrivateProfileString(lpAppName,lpKeyName,szValue,lpFileName)!=0;
}

//PrivateProfileファイルへ書き込む
void WritePrivateProfile(HWND hWnd){
	//リストビュー全般
	//動作
	//ダブルクリックを使用しない[動作]
	WritePrivateProfileInt(_T("ListView"),_T("DisableDoubleClick"),g_Config.ListView.bDisableDoubleClick,g_szPrivateProfile);
	//マウスアウトで確定[動作]
	WritePrivateProfileInt(_T("ListView"),_T("MouseOut"),g_Config.ListView.bMouseOut,g_szPrivateProfile);
	//ホットキーで確定[動作]
	WritePrivateProfileInt(_T("ListView"),_T("HotKey"),g_Config.ListView.bHotKey,g_szPrivateProfile);
	//マウスホバーで選択[動作]
	WritePrivateProfileInt(_T("ListView"),_T("MouseHover"),g_Config.ListView.bMouseHover,g_szPrivateProfile);
	//タイムアウトで確定[動作]
	WritePrivateProfileInt(_T("ListView"),_T("TimeOut"),g_Config.ListView.iTimeOut,g_szPrivateProfile);
	//ドラッグ中であればマウスホバーで選択
	WritePrivateProfileInt(_T("ListView"),_T("DragMouseHover"),g_Config.ListView.bDragMouseHover,g_szPrivateProfile);
	//ドラッグ中であればタイムアウトで確定
	WritePrivateProfileInt(_T("ListView"),_T("DragTimeOut"),g_Config.ListView.bDragTimeOut,g_szPrivateProfile);
	//タスクバー相当の表示[動作]
	WritePrivateProfileInt(_T("ListView"),_T("TaskBarEquivalent"),g_Config.ListView.bTaskBarEquivalent,g_szPrivateProfile);
	//2番目にアクティブなウインドウを選択[動作]
	WritePrivateProfileInt(_T("ListView"),_T("SelectSecondWindow"),g_Config.ListView.bSelectSecondWindow,g_szPrivateProfile);
	//サムネイルを表示[動作]
	WritePrivateProfileInt(_T("ListView"),_T("Thumbnail"),g_Config.ListView.bThumbnail,g_szPrivateProfile);
	//サムネイルの大きさ
	WritePrivateProfileInt(_T("ListView"),_T("ThumbnailSize"),g_Config.ListView.iThumbnailSize,g_szPrivateProfile);
	//サムネイルを表示するまでの時間
	WritePrivateProfileInt(_T("ListView"),_T("ThumbnailDelay"),g_Config.ListView.iThumbnailDelay,g_szPrivateProfile);
	//[不透明度]
	WritePrivateProfileInt(_T("ListView"),_T("Alpha"),g_Config.ListView.byAlpha,g_szPrivateProfile);
	//アイコンなし(大)、アイコンなし(小)、小さなアイコン、大きなアイコン[表示項目]
	WritePrivateProfileInt(_T("ListView"),_T("Icon"),g_Config.ListView.iIcon,g_szPrivateProfile);
	//リスト作成時のアイコンを使用する
	WritePrivateProfileInt(_T("ListView"),_T("CacheIcon"),g_Config.ListView.bCacheIcon,g_szPrivateProfile);
	//デスクトップ[表示項目]
	WritePrivateProfileInt(_T("ListView"),_T("DesktopItem"),g_Config.ListView.bDesktopItem,g_szPrivateProfile);
	//キャンセル[表示項目]
	WritePrivateProfileInt(_T("ListView"),_T("CancelItem"),g_Config.ListView.bCancelItem,g_szPrivateProfile);

	//フレームを表示
	WritePrivateProfileInt(_T("ListView"),_T("DialogFrame"),g_Config.ListView.bDialogFrame,g_szPrivateProfile);

	//表示項目(カラム幅で決定)
	//並び順
	TCHAR szColumnOrder[12]={};

	for(int i=0;i<LISTITEM_NUM;i++){
		TCHAR sz[3]={};

		wsprintf(sz,_T("%d"),g_Config.ListView.iColumnOrder[i]);
		lstrcat(szColumnOrder,sz);
	}
	WritePrivateProfileString(_T("ListView"),_T("ColumnOrder"),szColumnOrder,g_szPrivateProfile);

	//カラムの幅
	WritePrivateProfileInt(_T("ListView"),_T("FileNameWidth"),g_Config.ListView.iFileNameWidth,g_szPrivateProfile);
	WritePrivateProfileInt(_T("ListView"),_T("WindowTitleWidth"),g_Config.ListView.iWindowTitleWidth,g_szPrivateProfile);

	//検索方法[インクリメンタルサーチ]
	WritePrivateProfileInt(_T("IncrementalSearch"),_T("MatchMode"),g_Config.IncrementalSearch.iMatchMode,g_szPrivateProfile);

	//Migemoモード[インクリメンタルサーチ]
	WritePrivateProfileInt(_T("IncrementalSearch"),_T("MigemoMode"),g_Config.IncrementalSearch.iMigemoMode,g_szPrivateProfile);

	//左側の項目のみ検索[インクリメンタルサーチ]
	WritePrivateProfileInt(_T("IncrementalSearch"),_T("FirstColumnOnly"),g_Config.IncrementalSearch.bFirstColumnOnly,g_szPrivateProfile);

	//候補のウインドウが1つなら確定[インクリメンタルサーチ]
	WritePrivateProfileInt(_T("IncrementalSearch"),_T("EnterUniqueWindow"),g_Config.IncrementalSearch.bEnterUniqueWindow,g_szPrivateProfile);

	//候補が1つになった時切り替えるまでの時間
	WritePrivateProfileInt(_T("IncrementalSearch"),_T("UniqueWindowDelay"),g_Config.IncrementalSearch.iUniqueWindowDelay,g_szPrivateProfile);

	//連文節検索を行う[インクリメンタルサーチ]
	WritePrivateProfileInt(_T("IncrementalSearch"),_T("MigemoCaseSensitive"),g_Config.IncrementalSearch.bMigemoCaseSensitive,g_szPrivateProfile);

	//Migemo検索を開始するまでの時間
	WritePrivateProfileInt(_T("IncrementalSearch"),_T("MigemoDelay"),g_Config.IncrementalSearch.iMigemoDelay,g_szPrivateProfile);


	//スタートアップに追加する(自動起動)
	WritePrivateProfileInt(_T("General"),_T("AutoStart"),g_Config.bAutoStart,g_szPrivateProfile);

	TCHAR szTmp[32]={};

	//背景画像
	WritePrivateProfileString(_T("Background"),_T("Path"),g_Config.Background.szImagePath,g_szPrivateProfile);
	WritePrivateProfileInt(_T("Background"),_T("xOffset"),g_Config.Background.iXOffset,g_szPrivateProfile);
	WritePrivateProfileInt(_T("Background"),_T("yOffset"),g_Config.Background.iYOffset,g_szPrivateProfile);
	WritePrivateProfileInt(_T("Background"),_T("ResizePercent"),g_Config.Background.byResizePercent,g_szPrivateProfile);
	WritePrivateProfileInt(_T("Background"),_T("Alpha"),g_Config.Background.byAlpha,g_szPrivateProfile);


	//表示操作
	WritePrivateProfileInt(_T("ShowWindow"),_T("LeftTop"),g_Config.ShowWindow.bLeftTop,g_szPrivateProfile);
	WritePrivateProfileInt(_T("ShowWindow"),_T("Top"),g_Config.ShowWindow.bTop,g_szPrivateProfile);
	WritePrivateProfileInt(_T("ShowWindow"),_T("RightTop"),g_Config.ShowWindow.bRightTop,g_szPrivateProfile);
	WritePrivateProfileInt(_T("ShowWindow"),_T("Left"),g_Config.ShowWindow.bLeft,g_szPrivateProfile);
	WritePrivateProfileInt(_T("ShowWindow"),_T("Right"),g_Config.ShowWindow.bRight,g_szPrivateProfile);
	WritePrivateProfileInt(_T("ShowWindow"),_T("LeftBottom"),g_Config.ShowWindow.bLeftBottom,g_szPrivateProfile);
	WritePrivateProfileInt(_T("ShowWindow"),_T("Bottom"),g_Config.ShowWindow.bBottom,g_szPrivateProfile);
	WritePrivateProfileInt(_T("ShowWindow"),_T("RightBottom"),g_Config.ShowWindow.bRightBottom,g_szPrivateProfile);
	WritePrivateProfileInt(_T("ShowWindow"),_T("MouseWheel"),g_Config.ShowWindow.bMouseWheel,g_szPrivateProfile);
	WritePrivateProfileInt(_T("ShowWindow"),_T("HotKey"),g_Config.ShowWindow.wHotKey,g_szPrivateProfile);


	//ショートカットキー
	//セクション内のキーをすべて削除
	WritePrivateProfileString(_T("ShortcutKey"),NULL,NULL,g_szPrivateProfile);
	for(UINT i=0;i<MAX_SHORTCUTKEY;i++){
		wsprintf(szTmp,_T("%d"),g_Config.ShortcutKey.sKeyTable[i].wKey);

		for(UINT ii=0;ii<ARRAY_SIZEOF(ShortcutKeyCmd_Table);ii++){
			if(ShortcutKeyCmd_Table[ii].uCmdId==g_Config.ShortcutKey.sKeyTable[i].uCmdId){
				WritePrivateProfileString(_T("ShortcutKey"),szTmp,ShortcutKeyCmd_Table[ii].szCmd,g_szPrivateProfile);
				break;
			}
		}
	}


	//マウス
	//セクション内のキーをすべて削除
	WritePrivateProfileString(_T("Mouse"),NULL,NULL,g_szPrivateProfile);
	for(UINT i=0;i<SMOUSE_NUM;i++){
		if(!g_Config.Mouse.sMouseTable[i].uCmdId)continue;

		wsprintf(szTmp,_T("%d"),g_Config.Mouse.sMouseTable[i].uMouseType);

		for(UINT ii=0;ii<ARRAY_SIZEOF(ShortcutKeyCmd_Table);ii++){
			if(ShortcutKeyCmd_Table[ii].uCmdId==g_Config.Mouse.sMouseTable[i].uCmdId){
				WritePrivateProfileString(_T("Mouse"),szTmp,ShortcutKeyCmd_Table[ii].szCmd,g_szPrivateProfile);
				break;
			}
		}
	}


	//リストビューアイテムのデザイン
	//デフォルト(通常行)
	WritePrivateProfileString(_T("DefaultItemDesign"),_T("Font"),g_Config.DefaultItemDesign.szFont,g_szPrivateProfile);
	WritePrivateProfileInt(_T("DefaultItemDesign"),_T("FontSize"),g_Config.DefaultItemDesign.iFontSize,g_szPrivateProfile);
	WritePrivateProfileInt(_T("DefaultItemDesign"),_T("FontStyle"),g_Config.DefaultItemDesign.iFontStyle,g_szPrivateProfile);

	COLORREFToHex(szTmp,g_Config.DefaultItemDesign.clrText);
	WritePrivateProfileString(_T("DefaultItemDesign"),_T("TextColor"),szTmp,g_szPrivateProfile);

	COLORREFToHex(szTmp,g_Config.DefaultItemDesign.clrTextBk);
	WritePrivateProfileString(_T("DefaultItemDesign"),_T("BackColor"),szTmp,g_szPrivateProfile);

	//選択行
	WritePrivateProfileString(_T("SelectedItemDesign"),_T("Font"),g_Config.SelectedItemDesign.szFont,g_szPrivateProfile);
	WritePrivateProfileInt(_T("SelectedItemDesign"),_T("FontSize"),g_Config.SelectedItemDesign.iFontSize,g_szPrivateProfile);
	WritePrivateProfileInt(_T("SelectedItemDesign"),_T("FontStyle"),g_Config.SelectedItemDesign.iFontStyle,g_szPrivateProfile);

	COLORREFToHex(szTmp,g_Config.SelectedItemDesign.clrText);
	WritePrivateProfileString(_T("SelectedItemDesign"),_T("TextColor"),szTmp,g_szPrivateProfile);

	COLORREFToHex(szTmp,g_Config.SelectedItemDesign.clrTextBk);
	WritePrivateProfileString(_T("SelectedItemDesign"),_T("BackColor"),szTmp,g_szPrivateProfile);


	//除外するファイル名たち(;区切り)
	WritePrivateProfileString(_T("Exclude"),_T("FileName"),g_Config.Exclude.szFileName,g_szPrivateProfile);
	return;
}

//PrivateProfileファイルから読み込む
void ReadPrivateProfile(){
	//cfgファイルパス取得
	GetPrivateProfilePath();

	//リストビュー全般
	//ダブルクリックを使用しない[動作]
	g_Config.ListView.bDisableDoubleClick=GetPrivateProfileInt(_T("ListView"),_T("DisableDoubleClick"),1,g_szPrivateProfile)!=0;
	//マウスアウトで確定[動作]
	g_Config.ListView.bMouseOut=GetPrivateProfileInt(_T("ListView"),_T("MouseOut"),1,g_szPrivateProfile)!=0;
	//ホットキーで確定[動作]
	g_Config.ListView.bHotKey=GetPrivateProfileInt(_T("ListView"),_T("HotKey"),0,g_szPrivateProfile)!=0;
	//マウスホバーで選択[動作]
	g_Config.ListView.bMouseHover=GetPrivateProfileInt(_T("ListView"),_T("MouseHover"),0,g_szPrivateProfile)!=0;
	//タイムアウトで選択[動作]
	g_Config.ListView.iTimeOut=GetPrivateProfileInt(_T("ListView"),_T("TimeOut"),0,g_szPrivateProfile);
	//ドラッグ中であればマウスホバーで選択
	g_Config.ListView.bDragMouseHover=GetPrivateProfileInt(_T("ListView"),_T("DragMouseHover"),1,g_szPrivateProfile)!=0;
	//ドラッグ中であればタイムアウトで確定
	g_Config.ListView.bDragTimeOut=GetPrivateProfileInt(_T("ListView"),_T("DragTimeOut"),1,g_szPrivateProfile)!=0;
	//タスクバー相当の表示[動作]
	g_Config.ListView.bTaskBarEquivalent=GetPrivateProfileInt(_T("ListView"),_T("TaskBarEquivalent"),1,g_szPrivateProfile)!=0;
	//2番目にアクティブなウインドウを選択[動作]
	g_Config.ListView.bSelectSecondWindow=GetPrivateProfileInt(_T("ListView"),_T("SelectSecondWindow"),1,g_szPrivateProfile)!=0;
	//サムネイルを表示[動作]
	g_Config.ListView.bThumbnail=GetPrivateProfileInt(_T("ListView"),_T("Thumbnail"),1,g_szPrivateProfile)!=0;
	//サムネイルの大きさ
	g_Config.ListView.iThumbnailSize=GetPrivateProfileInt(_T("ListView"),_T("ThumbnailSize"),300,g_szPrivateProfile);
	//サムネイルを表示するまでの時間
	g_Config.ListView.iThumbnailDelay=GetPrivateProfileInt(_T("ListView"),_T("ThumbnailDelay"),1000,g_szPrivateProfile);

	//[不透明度]
	g_Config.ListView.byAlpha=GetPrivateProfileInt(_T("ListView"),_T("Alpha"),255,g_szPrivateProfile);

	//アイコンなし(大)、アイコンなし(小)、小さなアイコン、大きなアイコン[表示項目]
	g_Config.ListView.iIcon=GetPrivateProfileInt(_T("ListView"),_T("Icon"),LISTICON_BIG,g_szPrivateProfile);
	//リスト作成時のアイコンを使用する
	g_Config.ListView.bCacheIcon=GetPrivateProfileInt(_T("ListView"),_T("CacheIcon"),1,g_szPrivateProfile)!=0;
	//デスクトップ[表示項目]
	g_Config.ListView.bDesktopItem=GetPrivateProfileInt(_T("ListView"),_T("DesktopItem"),0,g_szPrivateProfile)!=0;
	//キャンセル[表示項目]
	g_Config.ListView.bCancelItem=GetPrivateProfileInt(_T("ListView"),_T("CancelItem"),0,g_szPrivateProfile)!=0;

	//フレームを表示
	g_Config.ListView.bDialogFrame=GetPrivateProfileInt(_T("ListView"),_T("DialogFrame"),1,g_szPrivateProfile)!=0;

	//アイコン指定を正しい値に収める
	g_Config.ListView.iIcon=(INRANGE(LISTICON_NO,g_Config.ListView.iIcon,LISTICON_BIG))?g_Config.ListView.iIcon:LISTICON_BIG;
	//不透明度を範囲内に収める
	g_Config.ListView.byAlpha=(INRANGE(0,g_Config.ListView.byAlpha,255))?g_Config.ListView.byAlpha:255;


	//ファイル名、ウインドウタイトル[表示項目]
	//並び順
	TCHAR szColumnOrder[12]={};

	GetPrivateProfileString(_T("ListView"),_T("ColumnOrder"),_T("01"),szColumnOrder,ARRAY_SIZEOF(szColumnOrder),g_szPrivateProfile);

	//Orderが正しいが確認
	TCHAR* pszColumnOrder=szColumnOrder;
	TCHAR szCheck[2]={};//1234567の場合8に、2345678の場合9に[桁数+最小数値]
	int iCount=2;

	while(*pszColumnOrder){
		if((*pszColumnOrder<(LISTITEM_FILENAME+_T('0')))||
		   (*pszColumnOrder>(LISTITEM_WINDOWTITLE+_T('0')))||
		   (szCheck[*pszColumnOrder-_T('0')]++)){
			break;
		}
		pszColumnOrder++;
		iCount--;
	}

	if(iCount||*pszColumnOrder){
		lstrcpy(szColumnOrder,_T("01"));
	}

	for(int i=0;i<LISTITEM_NUM;i++){
		g_Config.ListView.iColumnOrder[i]=szColumnOrder[i]-_T('0');
	}

	//カラムの幅
	g_Config.ListView.iFileNameWidth=GetPrivateProfileInt(_T("ListView"),_T("FileNameWidth"),DEFAULT_FILENAME_WIDTH,g_szPrivateProfile);
	g_Config.ListView.iWindowTitleWidth=GetPrivateProfileInt(_T("ListView"),_T("WindowTitleWidth"),DEFAULT_WINDOWTITLE_WIDTH,g_szPrivateProfile);

	//絶対値を取る
	g_Config.ListView.iFileNameWidth=abs(g_Config.ListView.iFileNameWidth);
	g_Config.ListView.iWindowTitleWidth=abs(g_Config.ListView.iWindowTitleWidth);

	g_Config.ListView.iFileNameWidth=(INRANGE(0,g_Config.ListView.iFileNameWidth,GetSystemMetrics(SM_CXSCREEN)))?g_Config.ListView.iFileNameWidth:0;
	g_Config.ListView.iWindowTitleWidth=(INRANGE(0,g_Config.ListView.iWindowTitleWidth,GetSystemMetrics(SM_CXSCREEN)))?g_Config.ListView.iWindowTitleWidth:0;

	//すべてのカラム幅が0の場合、すべて有効に
	if(g_Config.ListView.iFileNameWidth==0&&
	   g_Config.ListView.iWindowTitleWidth==0){
		g_Config.ListView.iFileNameWidth=DEFAULT_FILENAME_WIDTH;
		g_Config.ListView.iWindowTitleWidth=DEFAULT_WINDOWTITLE_WIDTH;
	}

	//検索方法[インクリメンタルサーチ]
	g_Config.IncrementalSearch.iMatchMode=GetPrivateProfileInt(_T("IncrementalSearch"),_T("MatchMode"),MATCH_PARTICAL,g_szPrivateProfile);

	//Migemoモード[インクリメンタルサーチ]
	g_Config.IncrementalSearch.iMigemoMode=GetPrivateProfileInt(_T("IncrementalSearch"),_T("MigemoMode"),MIGEMO_NO,g_szPrivateProfile);

	//左側の項目のみ検索[インクリメンタルサーチ]
	g_Config.IncrementalSearch.bFirstColumnOnly=GetPrivateProfileInt(_T("IncrementalSearch"),_T("FirstColumnOnly"),0,g_szPrivateProfile)!=0;

	//候補のウインドウが1つなら確定[インクリメンタルサーチ]
	g_Config.IncrementalSearch.bEnterUniqueWindow=GetPrivateProfileInt(_T("IncrementalSearch"),_T("EnterUniqueWindow"),1,g_szPrivateProfile)!=0;

	//候補が1つになった時切り替えるまでの時間
	g_Config.IncrementalSearch.iUniqueWindowDelay=GetPrivateProfileInt(_T("IncrementalSearch"),_T("UniqueWindowDelay"),300,g_szPrivateProfile);

	//連文節検索を行う[インクリメンタルサーチ]
	g_Config.IncrementalSearch.bMigemoCaseSensitive=GetPrivateProfileInt(_T("IncrementalSearch"),_T("MigemoCaseSensitive"),0,g_szPrivateProfile)!=0;

	//Migemo検索を開始するまでの時間
	g_Config.IncrementalSearch.iMigemoDelay=GetPrivateProfileInt(_T("IncrementalSearch"),_T("MigemoDelay"),90,g_szPrivateProfile);

	//Migemoモードを範囲内に収める
	g_Config.IncrementalSearch.iMigemoMode=(INRANGE(MIGEMO_NO,g_Config.IncrementalSearch.iMigemoMode,MIGEMO_ALWAYS))?g_Config.IncrementalSearch.iMigemoMode:MIGEMO_ALWAYS;


	//スタートアップに追加する(自動起動)
	g_Config.bAutoStart=GetPrivateProfileInt(_T("General"),_T("AutoStart"),0,g_szPrivateProfile)!=0;

	TCHAR szTmp[32]={};

	//背景画像
	GetPrivateProfileString(_T("Background"),_T("Path"),NULL,g_Config.Background.szImagePath,ARRAY_SIZEOF(g_Config.Background.szImagePath),g_szPrivateProfile);
	g_Config.Background.iXOffset=GetPrivateProfileInt(_T("Background"),_T("xOffset"),0,g_szPrivateProfile);
	g_Config.Background.iYOffset=GetPrivateProfileInt(_T("Background"),_T("yOffset"),0,g_szPrivateProfile);
	g_Config.Background.byResizePercent=GetPrivateProfileInt(_T("Background"),_T("ResizePercent"),100,g_szPrivateProfile);
	g_Config.Background.byAlpha=GetPrivateProfileInt(_T("Background"),_T("Alpha"),100,g_szPrivateProfile);

	//大きさを範囲内に収める
	g_Config.Background.byResizePercent=(INRANGE(0,g_Config.Background.byResizePercent,100))?g_Config.Background.byResizePercent:100;
	//不透明度を範囲内に収める
	g_Config.Background.byAlpha=(INRANGE(0,g_Config.Background.byAlpha,100))?g_Config.Background.byAlpha:100;


	//表示操作
	g_Config.ShowWindow.bLeftTop=GetPrivateProfileInt(_T("ShowWindow"),_T("LeftTop"),0,g_szPrivateProfile)!=0;
	g_Config.ShowWindow.bTop=GetPrivateProfileInt(_T("ShowWindow"),_T("Top"),0,g_szPrivateProfile)!=0;
	g_Config.ShowWindow.bRightTop=GetPrivateProfileInt(_T("ShowWindow"),_T("RightTop"),0,g_szPrivateProfile)!=0;
	g_Config.ShowWindow.bLeft=GetPrivateProfileInt(_T("ShowWindow"),_T("Left"),0,g_szPrivateProfile)!=0;
	g_Config.ShowWindow.bRight=GetPrivateProfileInt(_T("ShowWindow"),_T("Right"),0,g_szPrivateProfile)!=0;
	g_Config.ShowWindow.bLeftBottom=GetPrivateProfileInt(_T("ShowWindow"),_T("LeftBottom"),0,g_szPrivateProfile)!=0;
	g_Config.ShowWindow.bBottom=GetPrivateProfileInt(_T("ShowWindow"),_T("Bottom"),0,g_szPrivateProfile)!=0;
	g_Config.ShowWindow.bRightBottom=GetPrivateProfileInt(_T("ShowWindow"),_T("RightBottom"),1,g_szPrivateProfile)!=0;
	g_Config.ShowWindow.bMouseWheel=GetPrivateProfileInt(_T("ShowWindow"),_T("MouseWheel"),0,g_szPrivateProfile)!=0;
	g_Config.ShowWindow.wHotKey=GetPrivateProfileInt(_T("ShowWindow"),_T("HotKey"),858,g_szPrivateProfile);



	//ショートカットキー
	//必ず初期化
	g_Config.ShortcutKey.iSize=0;

	if(GetPrivateProfileInt(_T("ShortcutKey"),NULL,0,g_szPrivateProfile)!=
	   GetPrivateProfileInt(_T("ShortcutKey"),NULL,1,g_szPrivateProfile)){
		SKEY sKey[]={
			{VK_ESCAPE,IDM_KEY_CANCEL},
			{VK_SPACE,IDM_KEY_AND},
			{VK_RETURN,IDM_KEY_SWITCH},
			{MAKEWORD('M',HOTKEYF_CONTROL),IDM_KEY_SWITCH},
			{MAKEWORD('G',HOTKEYF_CONTROL),IDM_KEY_RESET},
			{MAKEWORD('P',HOTKEYF_CONTROL),IDM_KEY_PREV},
			{MAKEWORD(VK_UP,HOTKEYF_EXT),IDM_KEY_PREV},
			{MAKEWORD('N',HOTKEYF_CONTROL),IDM_KEY_NEXT},
			{MAKEWORD(VK_DOWN,HOTKEYF_EXT),IDM_KEY_NEXT},
			{MAKEWORD('A',HOTKEYF_CONTROL),IDM_KEY_FIRST},
			{MAKEWORD('E',HOTKEYF_CONTROL),IDM_KEY_LAST},
			{MAKEWORD('S',HOTKEYF_CONTROL),IDM_KEY_SETTINGS},
			{MAKEWORD('Q',HOTKEYF_CONTROL),IDM_KEY_QUIT},
			{MAKEWORD('I',HOTKEYF_CONTROL),IDM_KEY_MINIMIZE},
			{MAKEWORD('X',HOTKEYF_CONTROL),IDM_KEY_MAXIMIZE},
			{MAKEWORD('W',HOTKEYF_CONTROL),IDM_KEY_CLOSE},
			{MAKEWORD('R',HOTKEYF_CONTROL),IDM_KEY_RESTORE},
			{MAKEWORD('T',HOTKEYF_CONTROL),IDM_KEY_TOPMOST},
			{VK_BACK,IDM_KEY_BS_STRING},
			{MAKEWORD('H',HOTKEYF_CONTROL),IDM_KEY_BS_STRING},
			{MAKEWORD(VK_LEFT,HOTKEYF_EXT),IDM_KEY_SWAP},
			{MAKEWORD(VK_RIGHT,HOTKEYF_EXT),IDM_KEY_SWAP},
			{MAKEWORD('K',HOTKEYF_CONTROL),IDM_KEY_FORWARD_MATCH},
			{MAKEWORD('L',HOTKEYF_CONTROL),IDM_KEY_PARTIAL_MATCH},
			{MAKEWORD('F',HOTKEYF_CONTROL),IDM_KEY_FLEX_MATCH},
			{MAKEWORD('O',HOTKEYF_CONTROL),IDM_KEY_SEARCHSCOPE},
			{MAKEWORD('U',HOTKEYF_CONTROL),IDM_KEY_UNIQUE_WINDOW},
			{MAKEWORD('D',HOTKEYF_CONTROL),IDM_KEY_DESKTOP},
			{VK_OEM_102,IDM_KEY_MENU},
			{MAKEWORD(VK_OEM_102,HOTKEYF_CONTROL),IDM_KEY_CHILDPROCESS},
		};

		for(int i=0;i<ARRAY_SIZEOF(sKey);i++){
			g_Config.ShortcutKey.sKeyTable[g_Config.ShortcutKey.iSize++]=sKey[i];
		}
	}

	TCHAR buff[512]={};

	GetPrivateProfileString(_T("ShortcutKey"),NULL,_T(""),buff,ARRAY_SIZEOF(buff),g_szPrivateProfile);

	for(TCHAR*ptr=buff;*ptr!=NULL;ptr+=lstrlen(ptr)+1){
		TCHAR szCmd[32]={};
		SKEY sKey={(WORD)StrToInt(ptr)};

		if(g_Config.ShortcutKey.iSize>=MAX_SHORTCUTKEY)break;
		if(!sKey.wKey)continue;

		//重複するキーは追加しない
		bool bDup=false;

		for(UINT i=0;i<g_Config.ShortcutKey.iSize;i++){
			if(g_Config.ShortcutKey.sKeyTable[i].wKey==sKey.wKey){
				bDup=true;
				break;
			}
		}
		if(bDup)continue;

		GetPrivateProfileString(_T("ShortcutKey"),ptr,_T(""),szCmd,ARRAY_SIZEOF(szCmd),g_szPrivateProfile);

		for(UINT i=0;i<ARRAY_SIZEOF(ShortcutKeyCmd_Table);i++){
			if(lstrcmpi(ShortcutKeyCmd_Table[i].szCmd,szCmd)==0){
				sKey.uCmdId=ShortcutKeyCmd_Table[i].uCmdId;
				g_Config.ShortcutKey.sKeyTable[g_Config.ShortcutKey.iSize++]=sKey;
				break;
			}
		}
	}

	//マウス
	//初期化
	for(UINT i=0;i<SMOUSE_NUM;i++){
		SMOUSE sMouse={i,0};
		g_Config.Mouse.sMouseTable[i]=sMouse;
	}

	if(GetPrivateProfileInt(_T("Mouse"),NULL,0,g_szPrivateProfile)!=
	   GetPrivateProfileInt(_T("Mouse"),NULL,1,g_szPrivateProfile)){
		SMOUSE sMouse[]={
			{SMOUSE_LCLICK,IDM_KEY_SWITCH},
			{SMOUSE_RCLICK,IDM_KEY_MENU},
			{SMOUSE_RSCLICK,IDM_KEY_WINDOWMENU},
			{SMOUSE_RCCLICK,IDM_KEY_CANCEL},
			{SMOUSE_UWHEEL,IDM_KEY_PREV},
			{SMOUSE_DWHEEL,IDM_KEY_NEXT},
		};

		for(int i=0;i<ARRAY_SIZEOF(sMouse);i++){
			g_Config.Mouse.sMouseTable[sMouse[i].uMouseType]=sMouse[i];
		}
	}

	GetPrivateProfileString(_T("Mouse"),NULL,_T(""),buff,ARRAY_SIZEOF(buff),g_szPrivateProfile);

	for(TCHAR*ptr=buff;*ptr!=NULL;ptr+=lstrlen(ptr)+1){
		TCHAR szCmd[32]={};
		SMOUSE sMouse={(WORD)StrToInt(ptr)};

		if(sMouse.uMouseType>=SMOUSE_NUM)continue;

		GetPrivateProfileString(_T("Mouse"),ptr,_T(""),szCmd,ARRAY_SIZEOF(szCmd),g_szPrivateProfile);

		for(UINT i=0;i<ARRAY_SIZEOF(ShortcutKeyCmd_Table);i++){
			if(lstrcmpi(ShortcutKeyCmd_Table[i].szCmd,szCmd)==0){
				sMouse.uCmdId=ShortcutKeyCmd_Table[i].uCmdId;
				g_Config.Mouse.sMouseTable[sMouse.uMouseType]=sMouse;
				break;
			}
		}
	}

	//リストビューアイテムのデザイン
	//デフォルト(通常行)
	GetPrivateProfileString(_T("DefaultItemDesign"),_T("Font"),NULL,g_Config.DefaultItemDesign.szFont,ARRAY_SIZEOF(g_Config.DefaultItemDesign.szFont),g_szPrivateProfile);
	g_Config.DefaultItemDesign.iFontSize=GetPrivateProfileInt(_T("DefaultItemDesign"),_T("FontSize"),0,g_szPrivateProfile);
	g_Config.DefaultItemDesign.iFontStyle=GetPrivateProfileInt(_T("DefaultItemDesign"),_T("FontStyle"),0,g_szPrivateProfile);

	GetPrivateProfileString(_T("DefaultItemDesign"),_T("TextColor"),_T("#000000"),szTmp,ARRAY_SIZEOF(szTmp),g_szPrivateProfile);
	g_Config.DefaultItemDesign.clrText=StringToCOLORREF(szTmp);

	GetPrivateProfileString(_T("DefaultItemDesign"),_T("BackColor"),_T("#FFFFFF"),szTmp,ARRAY_SIZEOF(szTmp),g_szPrivateProfile);
	g_Config.DefaultItemDesign.clrTextBk=StringToCOLORREF(szTmp);

	//フォントサイズを範囲内に収める
	//範囲外であればMINIMUM_FONTSIZE(==0、フォントサイズ変更なし)
	g_Config.DefaultItemDesign.iFontSize=(INRANGE(MINIMUM_FONTSIZE,g_Config.DefaultItemDesign.iFontSize,MAXIMUM_FONTSIZE))?g_Config.DefaultItemDesign.iFontSize:MINIMUM_FONTSIZE;

	//選択行
	GetPrivateProfileString(_T("SelectedItemDesign"),_T("Font"),NULL,g_Config.SelectedItemDesign.szFont,ARRAY_SIZEOF(g_Config.SelectedItemDesign.szFont),g_szPrivateProfile);
	g_Config.SelectedItemDesign.iFontSize=GetPrivateProfileInt(_T("SelectedItemDesign"),_T("FontSize"),0,g_szPrivateProfile);
	g_Config.SelectedItemDesign.iFontStyle=GetPrivateProfileInt(_T("SelectedItemDesign"),_T("FontStyle"),1,g_szPrivateProfile);

	GetPrivateProfileString(_T("SelectedItemDesign"),_T("TextColor"),_T("#7BB2BD"),szTmp,ARRAY_SIZEOF(szTmp),g_szPrivateProfile);
	g_Config.SelectedItemDesign.clrText=StringToCOLORREF(szTmp);

	GetPrivateProfileString(_T("SelectedItemDesign"),_T("BackColor"),_T("#003873"),szTmp,ARRAY_SIZEOF(szTmp),g_szPrivateProfile);
	g_Config.SelectedItemDesign.clrTextBk=StringToCOLORREF(szTmp);

	//フォントサイズを範囲内に収める
	//範囲外であればMINIMUM_FONTSIZE(==0、フォントサイズ変更なし)
	g_Config.SelectedItemDesign.iFontSize=(INRANGE(MINIMUM_FONTSIZE,g_Config.SelectedItemDesign.iFontSize,MAXIMUM_FONTSIZE))?g_Config.SelectedItemDesign.iFontSize:MINIMUM_FONTSIZE;


	//除外するファイル名たち(;区切り)
	GetPrivateProfileString(_T("Exclude"),_T("FileName"),NULL,g_Config.Exclude.szFileName,ARRAY_SIZEOF(g_Config.Exclude.szFileName),g_szPrivateProfile);
	//末尾に';'を追加
	if(lstrlen(g_Config.Exclude.szFileName)&&
	   g_Config.Exclude.szFileName[lstrlen(g_Config.Exclude.szFileName)-1]!=';'){
		lstrcat(g_Config.Exclude.szFileName,_T(";"));
	}
	return;
}
