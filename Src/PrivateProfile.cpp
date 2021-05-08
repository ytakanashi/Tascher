//PrivateProfile.cpp
//cfgファイル操作

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.64
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/


#include"StdAfx.h"
#include"PrivateProfile.h"
#include"CommonSettings.h"
#include"Utilities.h"
#include"Path.h"
#include"resources/resource.h"
#include<dwrite.h>


TCHAR g_szPrivateProfile[MAX_PATH]={};

typedef struct{
	LPCTSTR szKeyName;
	LPCTSTR szKeyNameCursor;
	SCC_CORNERS eDefault;
}CURSORCORNER_TABLE;

static CURSORCORNER_TABLE CursorCorner_Table[]={
	{_T(""),_T(""),SCC_NONE},
	{
		_T("LeftTop"),
		_T("LeftTopCursor"),
		SCC_RIGHTBOTTOM,
	},
	{
		_T("Top"),
		_T("TopCursor"),
		SCC_BOTTOM,
	},
	{
		_T("RightTop"),
		_T("RightTopCursor"),
		SCC_LEFTBOTTOM,
	},
	{
		_T("Left"),
		_T("LeftCursor"),
		SCC_RIGHT,
	},
	{
		_T("Right"),
		_T("RightCursor"),
		SCC_LEFT,
	},
	{
		_T("LeftBottom"),
		_T("LeftBottomCursor"),
		SCC_RIGHTTOP,
	},
	{
		_T("Bottom"),
		_T("BottomCursor"),
		SCC_TOP,
	},
	{
		_T("RightBottom"),
		_T("RightBottomCursor"),
		SCC_LEFTTOP,
	},
};


//cfgファイルパスを取得
void GetPrivateProfilePath(){
	TCHAR szFileName[32]={};

	path::GetExeDirectory(g_szPrivateProfile,MAX_PATH);
	LoadString(GetModuleHandle(NULL),IDS_FILENAME,(LPTSTR)&szFileName,sizeof(szFileName)-1);
	wsprintf(g_szPrivateProfile,_T("%s\\%s.cfg"),g_szPrivateProfile,szFileName);
	return;
}

//整数を文字列に変換してPrivateProfileファイルに書き込む
bool WINAPI WritePrivateProfileInt(LPCTSTR lpAppName,LPCTSTR lpKeyName,LONG_PTR lData,LPCTSTR lpFileName){
	TCHAR szValue[256];
	wsprintf(szValue,_T("%ld"),lData);
	return WritePrivateProfileString(lpAppName,lpKeyName,szValue,lpFileName)!=0;
}

//PrivateProfileファイルへ書き込む
void WritePrivateProfile(){
	TCHAR szTmp[32]={};

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
	//2番目にアクティブなウインドウを選択[動作]
	WritePrivateProfileInt(_T("ListView"),_T("SelectSecondWindow"),g_Config.ListView.bSelectSecondWindow,g_szPrivateProfile);
	//サムネイルを表示[動作]
	WritePrivateProfileInt(_T("ListView"),_T("Thumbnail"),g_Config.ListView.uThumbnail,g_szPrivateProfile);
	//サムネイルの大きさ
	WritePrivateProfileInt(_T("ListView"),_T("ThumbnailSize"),g_Config.ListView.iThumbnailSize,g_szPrivateProfile);
	//サムネイルを元の大きさで表示
	WritePrivateProfileInt(_T("ListView"),_T("ThumbnailOriginalSize"),g_Config.ListView.bThumbnailOriginalSize,g_szPrivateProfile);
	//サムネイルを表示するまでの時間
	WritePrivateProfileInt(_T("ListView"),_T("ThumbnailDelay"),g_Config.ListView.iThumbnailDelay,g_szPrivateProfile);
	//サムネイルの垂直方向揃え位置
	WritePrivateProfileInt(_T("ListView"),_T("ThumbnailVerticalAlign"),g_Config.ListView.uThumbnailVerticalAlign,g_szPrivateProfile);

	//[不透明度]
	WritePrivateProfileInt(_T("ListView"),_T("Opacity"),g_Config.ListView.byOpacity,g_szPrivateProfile);
	//アイコンなし(大)、アイコンなし(小)、小さなアイコン、大きなアイコン[表示項目]
	WritePrivateProfileInt(_T("ListView"),_T("Icon"),g_Config.ListView.iIcon,g_szPrivateProfile);
	//ウインドウ[表示項目]
	WritePrivateProfileInt(_T("ListView"),_T("WindowItem"),g_Config.ListView.bWindowItem,g_szPrivateProfile);
	//ウェブブラウザタブ[表示項目]
	WritePrivateProfileInt(_T("ListView"),_T("WebBrowserTabItem"),g_Config.ListView.bWebBrowserTabItem,g_szPrivateProfile);
	//デスクトップ[表示項目]
	WritePrivateProfileInt(_T("ListView"),_T("DesktopItem"),g_Config.ListView.bDesktopItem,g_szPrivateProfile);
	//キャンセル[表示項目]
	WritePrivateProfileInt(_T("ListView"),_T("CancelItem"),g_Config.ListView.bCancelItem,g_szPrivateProfile);

	//UWPアプリの背景色を描画しない
	WritePrivateProfileInt(_T("ListView"),_T("UWPAppNoBackgroundColor"),g_Config.ListView.bUWPAppNoBackgroundColor,g_szPrivateProfile);

	//ウインドウからアイコンを取得
	WritePrivateProfileInt(_T("ListView"),_T("IconFromWindow"),g_Config.ListView.bIconFromWindow,g_szPrivateProfile);

	//フレームを表示
	WritePrivateProfileInt(_T("ListView"),_T("DialogFrame"),g_Config.ListView.bDialogFrame,g_szPrivateProfile);
	//フレーム色
	COLORREFToHex(szTmp,g_Config.ListView.clrDialogFrame);
	WritePrivateProfileString(_T("ListView"),_T("DialogFrameColor"),szTmp,g_szPrivateProfile);

	//ダブルクリックとして判定する時間
	WritePrivateProfileInt(_T("ListView"),_T("DoubleClickTime"),g_Config.ListView.iDoubleClickTime,g_szPrivateProfile);
	//クリック+マウスホイール上下として判定する時間
	WritePrivateProfileInt(_T("ListView"),_T("MouseWheelClickTime"),g_Config.ListView.iMouseWheelClickTime,g_szPrivateProfile);

	//カーソルを非表示にするまでの時間
	WritePrivateProfileInt(_T("ListView"),_T("HideCursorTime"),g_Config.ListView.iHideCursorTime,g_szPrivateProfile);

	//カーソル位置にメニューを表示(マウス操作時のみ)
	WritePrivateProfileInt(_T("ListView"),_T("PopupMenuCursorPos"),g_Config.ListView.bPopupMenuCursorPos,g_szPrivateProfile);

	//表示項目(カラム幅で決定)
	//並び順
	TCHAR szColumnOrder[12]={};

	for(int i=0;i<LISTITEM_NUM;i++){
		TCHAR szOrder[12]={};

		wsprintf(szOrder,_T("%d"),g_Config.ListView.iColumnOrder[i]);
		lstrcat(szColumnOrder,szOrder);
	}
	WritePrivateProfileString(_T("ListView"),_T("ColumnOrder"),szColumnOrder,g_szPrivateProfile);

	//カラムの幅
	WritePrivateProfileInt(_T("ListView"),_T("FileNameWidth"),g_Config.ListView.iFileNameWidth,g_szPrivateProfile);
	WritePrivateProfileInt(_T("ListView"),_T("WindowTitleWidth"),g_Config.ListView.iWindowTitleWidth,g_szPrivateProfile);

	//アイコンの余白
	WritePrivateProfileInt(_T("ListView"),_T("IconMargin"),g_Config.ListView.iIconMargin,g_szPrivateProfile);

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

	//Migemoを読み込んだままにする
	WritePrivateProfileInt(_T("IncrementalSearch"),_T("NoUnloadMigemo"),g_Config.IncrementalSearch.bNoUnloadMigemo,g_szPrivateProfile);

	//インクリメンタルサーチウインドウデザイン
	WritePrivateProfileString(_T("DefaultSearchWindowDesign"),_T("Font"),g_Config.IncrementalSearch.DefaultSearchWindowDesign.szFont,g_szPrivateProfile);
	WritePrivateProfileInt(_T("DefaultSearchWindowDesign"),_T("FontSize"),g_Config.IncrementalSearch.DefaultSearchWindowDesign.iFontSize,g_szPrivateProfile);

	COLORREFToHex(szTmp,g_Config.IncrementalSearch.DefaultSearchWindowDesign.clrText);
	WritePrivateProfileString(_T("DefaultSearchWindowDesign"),_T("TextColor"),szTmp,g_szPrivateProfile);

	COLORREFToHex(szTmp,g_Config.IncrementalSearch.DefaultSearchWindowDesign.clrTextBk);
	WritePrivateProfileString(_T("DefaultSearchWindowDesign"),_T("BackColor"),szTmp,g_szPrivateProfile);

	COLORREFToHex(szTmp,g_Config.IncrementalSearch.DefaultSearchWindowDesign.clrTextBkGradientEnd);
	WritePrivateProfileString(_T("DefaultSearchWindowDesign"),_T("BackGradientEndColor"),szTmp,g_szPrivateProfile);

	WritePrivateProfileString(_T("DefaultSearchWindowDesign"),_T("TextFormat"),g_Config.IncrementalSearch.DefaultSearchWindowDesign.szTextFormat,g_szPrivateProfile);

	//Migemo検索一致
	WritePrivateProfileString(_T("MigemoMatchSearchWindowDesign"),_T("Font"),g_Config.IncrementalSearch.MigemoMatchSearchWindowDesign.szFont,g_szPrivateProfile);
	WritePrivateProfileInt(_T("MigemoMatchSearchWindowDesign"),_T("FontSize"),g_Config.IncrementalSearch.MigemoMatchSearchWindowDesign.iFontSize,g_szPrivateProfile);

	COLORREFToHex(szTmp,g_Config.IncrementalSearch.MigemoMatchSearchWindowDesign.clrText);
	WritePrivateProfileString(_T("MigemoMatchSearchWindowDesign"),_T("TextColor"),szTmp,g_szPrivateProfile);

	COLORREFToHex(szTmp,g_Config.IncrementalSearch.MigemoMatchSearchWindowDesign.clrTextBk);
	WritePrivateProfileString(_T("MigemoMatchSearchWindowDesign"),_T("BackColor"),szTmp,g_szPrivateProfile);

	COLORREFToHex(szTmp,g_Config.IncrementalSearch.MigemoMatchSearchWindowDesign.clrTextBkGradientEnd);
	WritePrivateProfileString(_T("MigemoMatchSearchWindowDesign"),_T("BackGradientEndColor"),szTmp,g_szPrivateProfile);

	WritePrivateProfileString(_T("MigemoMatchSearchWindowDesign"),_T("TextFormat"),g_Config.IncrementalSearch.MigemoMatchSearchWindowDesign.szTextFormat,g_szPrivateProfile);

	//Migemo検索不一致
	WritePrivateProfileString(_T("MigemoNoMatchSearchWindowDesign"),_T("Font"),g_Config.IncrementalSearch.MigemoNoMatchSearchWindowDesign.szFont,g_szPrivateProfile);
	WritePrivateProfileInt(_T("MigemoNoMatchSearchWindowDesign"),_T("FontSize"),g_Config.IncrementalSearch.MigemoNoMatchSearchWindowDesign.iFontSize,g_szPrivateProfile);

	COLORREFToHex(szTmp,g_Config.IncrementalSearch.MigemoNoMatchSearchWindowDesign.clrText);
	WritePrivateProfileString(_T("MigemoNoMatchSearchWindowDesign"),_T("TextColor"),szTmp,g_szPrivateProfile);

	COLORREFToHex(szTmp,g_Config.IncrementalSearch.MigemoNoMatchSearchWindowDesign.clrTextBk);
	WritePrivateProfileString(_T("MigemoNoMatchSearchWindowDesign"),_T("BackColor"),szTmp,g_szPrivateProfile);

	COLORREFToHex(szTmp,g_Config.IncrementalSearch.MigemoNoMatchSearchWindowDesign.clrTextBkGradientEnd);
	WritePrivateProfileString(_T("MigemoNoMatchSearchWindowDesign"),_T("BackGradientEndColor"),szTmp,g_szPrivateProfile);

	WritePrivateProfileString(_T("MigemoNoMatchSearchWindowDesign"),_T("TextFormat"),g_Config.IncrementalSearch.MigemoNoMatchSearchWindowDesign.szTextFormat,g_szPrivateProfile);

	WritePrivateProfileInt(_T("IncrementalSearch"),_T("Margin"),g_Config.IncrementalSearch.iMargin,g_szPrivateProfile);
	WritePrivateProfileInt(_T("IncrementalSearch"),_T("Opacity"),g_Config.IncrementalSearch.byOpacity,g_szPrivateProfile);


	//自動起動する
	WritePrivateProfileInt(_T("General"),_T("AutoStart"),g_Config.bAutoStart,g_szPrivateProfile);

	//管理者権限で起動する
	WritePrivateProfileInt(_T("General"),_T("RunAsAdministrator"),g_Config.bRunAsAdministrator,g_szPrivateProfile);

	//背景画像
	WritePrivateProfileString(_T("Background"),_T("Path"),g_Config.Background.szImagePath,g_szPrivateProfile);
	WritePrivateProfileInt(_T("Background"),_T("xOffset"),g_Config.Background.iXOffset,g_szPrivateProfile);
	WritePrivateProfileInt(_T("Background"),_T("yOffset"),g_Config.Background.iYOffset,g_szPrivateProfile);
	WritePrivateProfileInt(_T("Background"),_T("ResizePercent"),g_Config.Background.byResizePercent,g_szPrivateProfile);
	WritePrivateProfileInt(_T("Background"),_T("Opacity"),g_Config.Background.byOpacity,g_szPrivateProfile);


	//表示操作
	for(UINT i=1;i<SCC_CENTER;i++){
		WritePrivateProfileInt(_T("ShowWindow"),CursorCorner_Table[i].szKeyName,g_Config.ShowWindow.ccCursorCorners[i].bValid,g_szPrivateProfile);
		WritePrivateProfileInt(_T("ShowWindow"),CursorCorner_Table[i].szKeyNameCursor,g_Config.ShowWindow.ccCursorCorners[i].eDestCorner,g_szPrivateProfile);
	}
	//選択項目にカーソルを移動[マウスの移動]
	WritePrivateProfileInt(_T("ShowWindow"),_T("MouseMoveCursorSelectedWindow"),g_Config.ShowWindow.bMouseMoveCursorSelectedWindow,g_szPrivateProfile);
	//選択項目にカーソルを移動[ホットキー]
	WritePrivateProfileInt(_T("ShowWindow"),_T("HotKeyCursorMouseSelectedWindow"),g_Config.ShowWindow.bHotKeyMoveCursorSelectedWindow,g_szPrivateProfile);
	WritePrivateProfileInt(_T("ShowWindow"),_T("MouseWheel"),g_Config.ShowWindow.bMouseWheel,g_szPrivateProfile);
	WritePrivateProfileInt(_T("ShowWindow"),_T("HotKeyCursorCorner"),g_Config.ShowWindow.eHotKeyCursorCorner,g_szPrivateProfile);
	WritePrivateProfileInt(_T("ShowWindow"),_T("HotKey"),g_Config.ShowWindow.wHotKey,g_szPrivateProfile);
	//「選択項目にカーソルを移動」が有効なら表示時に移動
	WritePrivateProfileInt(_T("ShowWindow"),_T("MoveCursorAtStart"),g_Config.ShowWindow.bMoveCursorAtStart,g_szPrivateProfile);
	WritePrivateProfileInt(_T("ShowWindow"),_T("CursorCornerDelay"),g_Config.ShowWindow.iCursorCornerDelay,g_szPrivateProfile);
	WritePrivateProfileInt(_T("ShowWindow"),_T("MouseEachMonitor"),g_Config.ShowWindow.bMouseEachMonitor,g_szPrivateProfile);
	WritePrivateProfileInt(_T("ShowWindow"),_T("MouseCursorMarginHorizontal"),g_Config.ShowWindow.iMouseCursorMarginHorizontal,g_szPrivateProfile);
	WritePrivateProfileInt(_T("ShowWindow"),_T("MouseCursorMarginVertical"),g_Config.ShowWindow.iMouseCursorMarginVertical,g_szPrivateProfile);
	WritePrivateProfileInt(_T("ShowWindow"),_T("HotKeyCursorMarginHorizontal"),g_Config.ShowWindow.iHotKeyCursorMarginHorizontal,g_szPrivateProfile);
	WritePrivateProfileInt(_T("ShowWindow"),_T("HotKeyCursorMarginVertical"),g_Config.ShowWindow.iHotKeyCursorMarginVertical,g_szPrivateProfile);
	//カーソルを閉じ込める
	WritePrivateProfileInt(_T("ShowWindow"),_T("MouseClipCursorMode"),g_Config.ShowWindow.iMouseClipCursorMode,g_szPrivateProfile);
	WritePrivateProfileInt(_T("ShowWindow"),_T("MouseClipCursorTime"),g_Config.ShowWindow.iMouseClipCursorTime,g_szPrivateProfile);
	WritePrivateProfileInt(_T("ShowWindow"),_T("HotKeyClipCursorMode"),g_Config.ShowWindow.iHotKeyClipCursorMode,g_szPrivateProfile);
	WritePrivateProfileInt(_T("ShowWindow"),_T("HotKeyClipCursorTime"),g_Config.ShowWindow.iHotKeyClipCursorTime,g_szPrivateProfile);
	//画面端と判断する端からの距離
	WritePrivateProfileInt(_T("ShowWindow"),_T("LeftCornerDistance"),g_Config.ShowWindow.iLeftCornerDistance,g_szPrivateProfile);
	WritePrivateProfileInt(_T("ShowWindow"),_T("RightCornerDistance"),g_Config.ShowWindow.iRightCornerDistance,g_szPrivateProfile);
	WritePrivateProfileInt(_T("ShowWindow"),_T("TopCornerDistance"),g_Config.ShowWindow.iTopCornerDistance,g_szPrivateProfile);
	WritePrivateProfileInt(_T("ShowWindow"),_T("BottomCornerDistance"),g_Config.ShowWindow.iBottomCornerDistance,g_szPrivateProfile);
	//フルスクリーン時は無効
	WritePrivateProfileInt(_T("ShowWindow"),_T("MouseDisableInFullScreenMode"),g_Config.ShowWindow.bMouseDisableInFullScreenMode,g_szPrivateProfile);
	WritePrivateProfileInt(_T("ShowWindow"),_T("HotKeyDisableInFullScreenMode"),g_Config.ShowWindow.bHotKeyDisableInFullScreenMode,g_szPrivateProfile);


	//ショートカットキー
	//セクション内のキーを一旦すべて削除
	WritePrivateProfileString(_T("ShortcutKey"),NULL,NULL,g_szPrivateProfile);
	//追加
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
	//セクション内のキーを一旦すべて削除
	WritePrivateProfileString(_T("Mouse"),NULL,NULL,g_szPrivateProfile);
	//追加
	for(UINT i=0;i<SMOUSE_NUM;i++){
		if(!g_Config.Mouse.sMouseTable[i].uCmdId)continue;

		wsprintf(szTmp,_T("%u"),g_Config.Mouse.sMouseTable[i].uMouseType);

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

	COLORREFToHex(szTmp,g_Config.DefaultItemDesign.clrTextBkGradientEnd);
	WritePrivateProfileString(_T("DefaultItemDesign"),_T("BackGradientEndColor"),szTmp,g_szPrivateProfile);

	//選択行
	WritePrivateProfileString(_T("SelectedItemDesign"),_T("Font"),g_Config.SelectedItemDesign.szFont,g_szPrivateProfile);
	WritePrivateProfileInt(_T("SelectedItemDesign"),_T("FontSize"),g_Config.SelectedItemDesign.iFontSize,g_szPrivateProfile);
	WritePrivateProfileInt(_T("SelectedItemDesign"),_T("FontStyle"),g_Config.SelectedItemDesign.iFontStyle,g_szPrivateProfile);

	COLORREFToHex(szTmp,g_Config.SelectedItemDesign.clrText);
	WritePrivateProfileString(_T("SelectedItemDesign"),_T("TextColor"),szTmp,g_szPrivateProfile);

	COLORREFToHex(szTmp,g_Config.SelectedItemDesign.clrTextBk);
	WritePrivateProfileString(_T("SelectedItemDesign"),_T("BackColor"),szTmp,g_szPrivateProfile);

	COLORREFToHex(szTmp,g_Config.SelectedItemDesign.clrTextBkGradientEnd);
	WritePrivateProfileString(_T("SelectedItemDesign"),_T("BackGradientEndColor"),szTmp,g_szPrivateProfile);


	//DirectWrite
	WritePrivateProfileInt(_T("DirectWrite"),_T("DirectWrite"),g_Config.DirectWrite.bDirectWrite,g_szPrivateProfile);
	WritePrivateProfileInt(_T("DirectWrite"),_T("RenderingMode"),g_Config.DirectWrite.uRenderingMode,g_szPrivateProfile);
	WritePrivateProfileInt(_T("DirectWrite"),_T("Gamma"),g_Config.DirectWrite.uGamma,g_szPrivateProfile);
	WritePrivateProfileInt(_T("DirectWrite"),_T("EnhancedContrast"),g_Config.DirectWrite.uEnhancedContrast,g_szPrivateProfile);
	WritePrivateProfileInt(_T("DirectWrite"),_T("ClearTypeLevel"),g_Config.DirectWrite.uClearTypeLevel,g_szPrivateProfile);


	//除外するファイル名たち(;区切り)
	WritePrivateProfileString(_T("Exclude"),_T("FileName"),g_Config.Exclude.szFileName,g_szPrivateProfile);

	//ウインドウリストを表示するモニタ
	WritePrivateProfileInt(_T("MultiMonitor"),_T("ActiveMonitorMode"),g_Config.MultiMonitor.iActiveMonitorMode,g_szPrivateProfile);
	//ウインドウリストを表示するモニタ名
	WritePrivateProfileString(_T("MultiMonitor"),_T("MonitorName"),g_Config.MultiMonitor.szMonitorName,g_szPrivateProfile);
	//アクティブなモニタのウインドウのみ表示
	WritePrivateProfileInt(_T("MultiMonitor"),_T("OnlyActiveMonitor"),g_Config.MultiMonitor.bOnlyActiveMonitor,g_szPrivateProfile);

	//アクティブな仮想デスクトップのウインドウのみ表示
	WritePrivateProfileInt(_T("VirtualDesktop"),_T("OnlyActiveVirtualDesktop"),g_Config.VirtualDesktop.bOnlyActiveVirtualDesktop,g_szPrivateProfile);


	for(UINT i=1;i<=MAX_COMMAND;i++){
		TCHAR szKey[24];

		wsprintf(szKey,_T("Command%u"),i);
		if(lstrlen(g_Config.Command[i].szCommandName)){
			WritePrivateProfileString(szKey,_T("CommandName"),g_Config.Command[i].szCommandName,g_szPrivateProfile);
			WritePrivateProfileInt(szKey,_T("CommandMode"),g_Config.Command[i].uCommandMode,g_szPrivateProfile);
			WritePrivateProfileInt(szKey,_T("CommandOption"),g_Config.Command[i].uCommandOption,g_szPrivateProfile);
			WritePrivateProfileString(szKey,_T("FilePath"),g_Config.Command[i].szFilePath,g_szPrivateProfile);
			WritePrivateProfileString(szKey,_T("Parameters"),g_Config.Command[i].szParameters,g_szPrivateProfile);
			WritePrivateProfileString(szKey,_T("WorkingDirectory"),g_Config.Command[i].szWorkingDirectory,g_szPrivateProfile);
			WritePrivateProfileInt(szKey,_T("CommandShow"),g_Config.Command[i].uCmdShow,g_szPrivateProfile);
		}else{
			//キーを削除
			WritePrivateProfileString(szKey,NULL,NULL,g_szPrivateProfile);
		}
	}

	//ウェブブラウザタブ
	for(UINT i=1;i<=MAX_WEBBROWSER;i++){
		TCHAR szKey[24];

		wsprintf(szKey,_T("WebBrowser%u"),i);
		WritePrivateProfileString(szKey,_T("FilePath"),g_Config.WebBrowser[i].szFilePath,g_szPrivateProfile);
		WritePrivateProfileInt(szKey,_T("Port"),g_Config.WebBrowser[i].iPort,g_szPrivateProfile);
	}

	return;
}

//PrivateProfileファイルから読み込む
void ReadPrivateProfile(){
	TCHAR szTmp[32]={};

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
	//2番目にアクティブなウインドウを選択[動作]
	g_Config.ListView.bSelectSecondWindow=GetPrivateProfileInt(_T("ListView"),_T("SelectSecondWindow"),1,g_szPrivateProfile)!=0;
	//サムネイルを表示[動作] 表示位置[高度な設定]
	g_Config.ListView.uThumbnail=GetPrivateProfileInt(_T("ListView"),_T("Thumbnail"),THUMBNAIL_RIGHT_SIDE,g_szPrivateProfile);
	g_Config.ListView.uThumbnail=(INRANGE(THUMBNAIL_NO,g_Config.ListView.uThumbnail,THUMBNAIL_RIGHT_SIDE))?g_Config.ListView.uThumbnail:THUMBNAIL_RIGHT_SIDE;
	//サムネイルの大きさ
	g_Config.ListView.iThumbnailSize=GetPrivateProfileInt(_T("ListView"),_T("ThumbnailSize"),400,g_szPrivateProfile);
	//サムネイルを元の大きさで表示
	g_Config.ListView.bThumbnailOriginalSize=GetPrivateProfileInt(_T("ListView"),_T("ThumbnailOriginalSize"),0,g_szPrivateProfile)!=0;
	//サムネイルを表示するまでの時間
	g_Config.ListView.iThumbnailDelay=GetPrivateProfileInt(_T("ListView"),_T("ThumbnailDelay"),0,g_szPrivateProfile);
	//サムネイルの垂直方向揃え位置
	g_Config.ListView.uThumbnailVerticalAlign=GetPrivateProfileInt(_T("ListView"),_T("ThumbnailVerticalAlign"),0,g_szPrivateProfile);
	g_Config.ListView.uThumbnailVerticalAlign=(INRANGE(THUMBNAIL_VERTICALALIGN_TOP,g_Config.ListView.uThumbnailVerticalAlign,THUMBNAIL_VERTICALALIGN_SELECTED_ITEM))?g_Config.ListView.uThumbnailVerticalAlign:THUMBNAIL_VERTICALALIGN_TOP;

	//[不透明度]
	g_Config.ListView.byOpacity=(BYTE)GetPrivateProfileInt(_T("ListView"),_T("Opacity"),100,g_szPrivateProfile);
	g_Config.ListView.byOpacity=(INRANGE(0,g_Config.ListView.byOpacity,100))?g_Config.ListView.byOpacity:100;

	//アイコンなし(大)、アイコンなし(小)、小さなアイコン(16x16)、大きなアイコン(32x32)、大きなアイコン(48x48)[表示項目]
	g_Config.ListView.iIcon=GetPrivateProfileInt(_T("ListView"),_T("Icon"),LISTICON_BIG,g_szPrivateProfile);
	g_Config.ListView.iIcon=(INRANGE(LISTICON_NO,g_Config.ListView.iIcon,LISTICON_BIG48))?g_Config.ListView.iIcon:LISTICON_BIG;
	//ウインドウ[表示項目]
	g_Config.ListView.bWindowItem=GetPrivateProfileInt(_T("ListView"),_T("WindowItem"),1,g_szPrivateProfile)!=0;
	//ウェブブラウザタブ[表示項目]
	g_Config.ListView.bWebBrowserTabItem=GetPrivateProfileInt(_T("ListView"),_T("WebBrowserTabItem"),0,g_szPrivateProfile)!=0;
	if(!g_Config.ListView.bWindowItem&&
	   !g_Config.ListView.bWebBrowserTabItem)g_Config.ListView.bWindowItem=true;
	//デスクトップ[表示項目]
	g_Config.ListView.bDesktopItem=GetPrivateProfileInt(_T("ListView"),_T("DesktopItem"),0,g_szPrivateProfile)!=0;
	//キャンセル[表示項目]
	g_Config.ListView.bCancelItem=GetPrivateProfileInt(_T("ListView"),_T("CancelItem"),0,g_szPrivateProfile)!=0;

	//UWPアプリの背景色を描画しない
	g_Config.ListView.bUWPAppNoBackgroundColor=GetPrivateProfileInt(_T("ListView"),_T("UWPAppNoBackgroundColor"),1,g_szPrivateProfile)!=0;

	//ウインドウからアイコンを取得
	g_Config.ListView.bIconFromWindow=GetPrivateProfileInt(_T("ListView"),_T("IconFromWindow"),0,g_szPrivateProfile)!=0;

	//フレームを表示
	g_Config.ListView.bDialogFrame=GetPrivateProfileInt(_T("ListView"),_T("DialogFrame"),1,g_szPrivateProfile)!=0;
	//フレーム色
	GetPrivateProfileString(_T("ListView"),_T("DialogFrameColor"),_T("#007ACC"),szTmp,ARRAY_SIZEOF(szTmp),g_szPrivateProfile);
	g_Config.ListView.clrDialogFrame=StringToCOLORREF(szTmp);

	//ダブルクリックとして判定する時間
	g_Config.ListView.iDoubleClickTime=GetPrivateProfileInt(_T("ListView"),_T("DoubleClickTime"),GetDoubleClickTime(),g_szPrivateProfile);
	g_Config.ListView.iDoubleClickTime=(INRANGE(10,g_Config.ListView.iDoubleClickTime,5000))?g_Config.ListView.iDoubleClickTime:GetDoubleClickTime();
	//クリック+マウスホイール上下として判定する時間
	g_Config.ListView.iMouseWheelClickTime=GetPrivateProfileInt(_T("ListView"),_T("MouseWheelClickTime"),g_Config.ListView.iDoubleClickTime-1000,g_szPrivateProfile);
	g_Config.ListView.iMouseWheelClickTime=(INRANGE(10,g_Config.ListView.iMouseWheelClickTime,4900))?g_Config.ListView.iMouseWheelClickTime:GetDoubleClickTime()-100;

	//カーソルを非表示にするまでの時間
	g_Config.ListView.iHideCursorTime=GetPrivateProfileInt(_T("ListView"),_T("HideCursorTime"),-1,g_szPrivateProfile);

	//カーソル位置にメニューを表示(マウス操作時のみ)
	g_Config.ListView.bPopupMenuCursorPos=GetPrivateProfileInt(_T("ListView"),_T("PopupMenuCursorPos"),0,g_szPrivateProfile)!=0;


	//ファイル名、ウインドウタイトル[表示項目]
	//並び順
	TCHAR szColumnOrder[12]={};

	GetPrivateProfileString(_T("ListView"),_T("ColumnOrder"),_T("012"),szColumnOrder,ARRAY_SIZEOF(szColumnOrder),g_szPrivateProfile);

	//Orderが正しいが確認
	TCHAR* pszColumnOrder=szColumnOrder;
	TCHAR szCheck[3]={};//1234567の場合8に、2345678の場合9に[桁数+最小数値]
	int iCount=LISTITEM_ICON;

	while(*pszColumnOrder){
		if((*pszColumnOrder<(LISTITEM_ICON+_T('0')))||
		   (*pszColumnOrder>((LISTITEM_NUM+1)+_T('0')))||
		   (szCheck[*pszColumnOrder-_T('0')]++)){
			break;
		}
		pszColumnOrder++;
		iCount--;
	}

	if(iCount||*pszColumnOrder){
		//Orderが不正であればデフォルトに
		lstrcpy(szColumnOrder,_T("012"));
	}

	if(szColumnOrder[0]!=_T('0')){
		//LISTITEM_ICONを先頭に
		TCHAR szOrderTmp[12]={};
		lstrcpy(szOrderTmp,szColumnOrder);
		TCHAR* pszOrderTmp=szOrderTmp;
		int iIndex=1;
		szColumnOrder[0]=_T('0');
		while(*pszOrderTmp){
			if(*pszOrderTmp!=_T('0'))szColumnOrder[iIndex++]=*pszOrderTmp;
			pszOrderTmp++;
		}
	}

	for(int i=0;i<LISTITEM_NUM;i++){
		g_Config.ListView.iColumnOrder[i]=szColumnOrder[i]-_T('0');
	}

	//カラムの幅
	g_Config.ListView.iFileNameWidth=GetPrivateProfileInt(_T("ListView"),_T("FileNameWidth"),DEFAULT_FILENAME_WIDTH,g_szPrivateProfile);
	g_Config.ListView.iFileNameWidth=abs(g_Config.ListView.iFileNameWidth);
	g_Config.ListView.iFileNameWidth=(INRANGE(0,g_Config.ListView.iFileNameWidth,GetSystemMetrics(SM_CXSCREEN)))?g_Config.ListView.iFileNameWidth:0;
	g_Config.ListView.iWindowTitleWidth=GetPrivateProfileInt(_T("ListView"),_T("WindowTitleWidth"),DEFAULT_WINDOWTITLE_WIDTH,g_szPrivateProfile);
	g_Config.ListView.iWindowTitleWidth=abs(g_Config.ListView.iWindowTitleWidth);
	g_Config.ListView.iWindowTitleWidth=(INRANGE(0,g_Config.ListView.iWindowTitleWidth,GetSystemMetrics(SM_CXSCREEN)))?g_Config.ListView.iWindowTitleWidth:0;

	//すべてのカラム幅が0の場合、すべて有効に
	if(g_Config.ListView.iFileNameWidth==0&&
	   g_Config.ListView.iWindowTitleWidth==0){
		g_Config.ListView.iFileNameWidth=DEFAULT_FILENAME_WIDTH;
		g_Config.ListView.iWindowTitleWidth=DEFAULT_WINDOWTITLE_WIDTH;
	}

	//アイコンの余白
	g_Config.ListView.iIconMargin=GetPrivateProfileInt(_T("ListView"),_T("IconMargin"),DEFAULT_ICONMARGIN,g_szPrivateProfile);
	g_Config.ListView.iIconMargin=abs(g_Config.ListView.iIconMargin);
	g_Config.ListView.iIconMargin=(INRANGE(MINIMUM_ICONMARGIN,g_Config.ListView.iIconMargin,MAXIMUM_ICONMARGIN))?g_Config.ListView.iIconMargin:DEFAULT_ICONMARGIN;


	//検索方法[インクリメンタルサーチ]
	g_Config.IncrementalSearch.iMatchMode=GetPrivateProfileInt(_T("IncrementalSearch"),_T("MatchMode"),MATCH_PARTICAL,g_szPrivateProfile);
	g_Config.IncrementalSearch.iMatchMode=(INRANGE(MATCH_FLEX,g_Config.IncrementalSearch.iMatchMode,MATCH_FLEX))?g_Config.IncrementalSearch.iMatchMode:MATCH_PARTICAL;

	//Migemoモード[インクリメンタルサーチ]
	g_Config.IncrementalSearch.iMigemoMode=GetPrivateProfileInt(_T("IncrementalSearch"),_T("MigemoMode"),MIGEMO_NO,g_szPrivateProfile);
	g_Config.IncrementalSearch.iMigemoMode=(INRANGE(MIGEMO_NO,g_Config.IncrementalSearch.iMatchMode,MIGEMO_ALWAYS))?g_Config.IncrementalSearch.iMigemoMode:MIGEMO_NO;

	//左側の項目のみ検索[インクリメンタルサーチ]
	g_Config.IncrementalSearch.bFirstColumnOnly=GetPrivateProfileInt(_T("IncrementalSearch"),_T("FirstColumnOnly"),0,g_szPrivateProfile)!=0;

	//候補のウインドウが1つなら確定[インクリメンタルサーチ]
	g_Config.IncrementalSearch.bEnterUniqueWindow=GetPrivateProfileInt(_T("IncrementalSearch"),_T("EnterUniqueWindow"),1,g_szPrivateProfile)!=0;

	//候補が1つになった時切り替えるまでの時間
	g_Config.IncrementalSearch.iUniqueWindowDelay=GetPrivateProfileInt(_T("IncrementalSearch"),_T("UniqueWindowDelay"),300,g_szPrivateProfile);

	//連文節検索を行う[インクリメンタルサーチ]
	g_Config.IncrementalSearch.bMigemoCaseSensitive=GetPrivateProfileInt(_T("IncrementalSearch"),_T("MigemoCaseSensitive"),0,g_szPrivateProfile)!=0;

	//Migemo検索を開始するまでの時間
	g_Config.IncrementalSearch.iMigemoDelay=GetPrivateProfileInt(_T("IncrementalSearch"),_T("MigemoDelay"),30,g_szPrivateProfile);

	//Migemoを読み込んだままにする
	g_Config.IncrementalSearch.bNoUnloadMigemo=GetPrivateProfileInt(_T("IncrementalSearch"),_T("NoUnloadMigemo"),1,g_szPrivateProfile)!=0;

	//インクリメンタルサーチウインドウデザイン
	GetPrivateProfileString(_T("DefaultSearchWindowDesign"),_T("Font"),NULL,g_Config.IncrementalSearch.DefaultSearchWindowDesign.szFont,ARRAY_SIZEOF(g_Config.DefaultItemDesign.szFont),g_szPrivateProfile);
	g_Config.IncrementalSearch.DefaultSearchWindowDesign.iFontSize=GetPrivateProfileInt(_T("DefaultSearchWindowDesign"),_T("FontSize"),MINIMUM_FONTSIZE,g_szPrivateProfile);
	g_Config.IncrementalSearch.DefaultSearchWindowDesign.iFontSize=(INRANGE(MINIMUM_FONTSIZE,g_Config.IncrementalSearch.DefaultSearchWindowDesign.iFontSize,MAXIMUM_FONTSIZE))?g_Config.IncrementalSearch.DefaultSearchWindowDesign.iFontSize:MINIMUM_FONTSIZE;
	g_Config.IncrementalSearch.DefaultSearchWindowDesign.iFontStyle=GetPrivateProfileInt(_T("DefaultSearchWindowDesign"),_T("FontStyle"),0,g_szPrivateProfile);

	GetPrivateProfileString(_T("DefaultSearchWindowDesign"),_T("TextColor"),_T("#F0F0F0"),szTmp,ARRAY_SIZEOF(szTmp),g_szPrivateProfile);
	g_Config.IncrementalSearch.DefaultSearchWindowDesign.clrText=StringToCOLORREF(szTmp);
	GetPrivateProfileString(_T("DefaultSearchWindowDesign"),_T("BackColor"),_T("#141414"),szTmp,ARRAY_SIZEOF(szTmp),g_szPrivateProfile);
	g_Config.IncrementalSearch.DefaultSearchWindowDesign.clrTextBk=StringToCOLORREF(szTmp);
	GetPrivateProfileString(_T("DefaultSearchWindowDesign"),_T("BackGradientEndColor"),_T("#141414"),szTmp,ARRAY_SIZEOF(szTmp),g_szPrivateProfile);
	g_Config.IncrementalSearch.DefaultSearchWindowDesign.clrTextBkGradientEnd=StringToCOLORREF(szTmp);

	GetPrivateProfileString(_T("DefaultSearchWindowDesign"),_T("TextFormat"),_T("%S"),g_Config.IncrementalSearch.DefaultSearchWindowDesign.szTextFormat,ARRAY_SIZEOF(g_Config.IncrementalSearch.DefaultSearchWindowDesign.szTextFormat),g_szPrivateProfile);

	//Migemo検索一致
	GetPrivateProfileString(_T("MigemoMatchSearchWindowDesign"),_T("Font"),NULL,g_Config.IncrementalSearch.MigemoMatchSearchWindowDesign.szFont,ARRAY_SIZEOF(g_Config.DefaultItemDesign.szFont),g_szPrivateProfile);
	g_Config.IncrementalSearch.MigemoMatchSearchWindowDesign.iFontSize=GetPrivateProfileInt(_T("MigemoMatchSearchWindowDesign"),_T("FontSize"),MINIMUM_FONTSIZE,g_szPrivateProfile);
	g_Config.IncrementalSearch.MigemoMatchSearchWindowDesign.iFontSize=(INRANGE(MINIMUM_FONTSIZE,g_Config.IncrementalSearch.MigemoMatchSearchWindowDesign.iFontSize,MAXIMUM_FONTSIZE))?g_Config.IncrementalSearch.MigemoMatchSearchWindowDesign.iFontSize:MINIMUM_FONTSIZE;
	g_Config.IncrementalSearch.MigemoMatchSearchWindowDesign.iFontStyle=GetPrivateProfileInt(_T("MigemoMatchSearchWindowDesign"),_T("FontStyle"),SFONT_BOLD,g_szPrivateProfile);

	GetPrivateProfileString(_T("MigemoMatchSearchWindowDesign"),_T("TextColor"),_T("#F0F0F0"),szTmp,ARRAY_SIZEOF(szTmp),g_szPrivateProfile);
	g_Config.IncrementalSearch.MigemoMatchSearchWindowDesign.clrText=StringToCOLORREF(szTmp);
	GetPrivateProfileString(_T("MigemoMatchSearchWindowDesign"),_T("BackColor"),_T("#141414"),szTmp,ARRAY_SIZEOF(szTmp),g_szPrivateProfile);
	g_Config.IncrementalSearch.MigemoMatchSearchWindowDesign.clrTextBk=StringToCOLORREF(szTmp);
	GetPrivateProfileString(_T("MigemoMatchSearchWindowDesign"),_T("BackGradientEndColor"),_T("#141414"),szTmp,ARRAY_SIZEOF(szTmp),g_szPrivateProfile);
	g_Config.IncrementalSearch.MigemoMatchSearchWindowDesign.clrTextBkGradientEnd=StringToCOLORREF(szTmp);

	GetPrivateProfileString(_T("MigemoMatchSearchWindowDesign"),_T("TextFormat"),_T("[M] %S"),g_Config.IncrementalSearch.MigemoMatchSearchWindowDesign.szTextFormat,ARRAY_SIZEOF(g_Config.IncrementalSearch.MigemoMatchSearchWindowDesign.szTextFormat),g_szPrivateProfile);

	//Migemo検索不一致
	GetPrivateProfileString(_T("MigemoNoMatchSearchWindowDesign"),_T("Font"),NULL,g_Config.IncrementalSearch.MigemoNoMatchSearchWindowDesign.szFont,ARRAY_SIZEOF(g_Config.DefaultItemDesign.szFont),g_szPrivateProfile);
	g_Config.IncrementalSearch.MigemoNoMatchSearchWindowDesign.iFontSize=GetPrivateProfileInt(_T("MigemoNoMatchSearchWindowDesign"),_T("FontSize"),MINIMUM_FONTSIZE,g_szPrivateProfile);
	g_Config.IncrementalSearch.MigemoNoMatchSearchWindowDesign.iFontSize=(INRANGE(MINIMUM_FONTSIZE,g_Config.IncrementalSearch.MigemoNoMatchSearchWindowDesign.iFontSize,MAXIMUM_FONTSIZE))?g_Config.IncrementalSearch.MigemoNoMatchSearchWindowDesign.iFontSize:MINIMUM_FONTSIZE;
	g_Config.IncrementalSearch.MigemoNoMatchSearchWindowDesign.iFontStyle=GetPrivateProfileInt(_T("MigemoNoMatchSearchWindowDesign"),_T("FontStyle"),0,g_szPrivateProfile);

	GetPrivateProfileString(_T("MigemoNoMatchSearchWindowDesign"),_T("TextColor"),_T("#868686"),szTmp,ARRAY_SIZEOF(szTmp),g_szPrivateProfile);
	g_Config.IncrementalSearch.MigemoNoMatchSearchWindowDesign.clrText=StringToCOLORREF(szTmp);

	GetPrivateProfileString(_T("MigemoNoMatchSearchWindowDesign"),_T("BackColor"),_T("#141414"),szTmp,ARRAY_SIZEOF(szTmp),g_szPrivateProfile);
	g_Config.IncrementalSearch.MigemoNoMatchSearchWindowDesign.clrTextBk=StringToCOLORREF(szTmp);
	GetPrivateProfileString(_T("MigemoNoMatchSearchWindowDesign"),_T("BackGradientEndColor"),_T("#141414"),szTmp,ARRAY_SIZEOF(szTmp),g_szPrivateProfile);
	g_Config.IncrementalSearch.MigemoNoMatchSearchWindowDesign.clrTextBkGradientEnd=StringToCOLORREF(szTmp);

	GetPrivateProfileString(_T("MigemoNoMatchSearchWindowDesign"),_T("TextFormat"),_T("[m] %S"),g_Config.IncrementalSearch.MigemoNoMatchSearchWindowDesign.szTextFormat,ARRAY_SIZEOF(g_Config.IncrementalSearch.MigemoNoMatchSearchWindowDesign.szTextFormat),g_szPrivateProfile);

	g_Config.IncrementalSearch.iMargin=GetPrivateProfileInt(_T("IncrementalSearch"),_T("Margin"),3,g_szPrivateProfile);
	g_Config.IncrementalSearch.byOpacity=(BYTE)GetPrivateProfileInt(_T("IncrementalSearch"),_T("Opacity"),80,g_szPrivateProfile);
	g_Config.IncrementalSearch.byOpacity=(INRANGE(0,g_Config.IncrementalSearch.byOpacity,100))?g_Config.IncrementalSearch.byOpacity:100;


	//自動起動する
	g_Config.bAutoStart=GetPrivateProfileInt(_T("General"),_T("AutoStart"),0,g_szPrivateProfile)!=0;

	//管理者権限で起動する
	g_Config.bRunAsAdministrator=GetPrivateProfileInt(_T("General"),_T("RunAsAdministrator"),0,g_szPrivateProfile)!=0;

	//背景画像
	GetPrivateProfileString(_T("Background"),_T("Path"),NULL,g_Config.Background.szImagePath,ARRAY_SIZEOF(g_Config.Background.szImagePath),g_szPrivateProfile);
	g_Config.Background.iXOffset=GetPrivateProfileInt(_T("Background"),_T("xOffset"),0,g_szPrivateProfile);
	g_Config.Background.iYOffset=GetPrivateProfileInt(_T("Background"),_T("yOffset"),0,g_szPrivateProfile);
	g_Config.Background.byResizePercent=(BYTE)GetPrivateProfileInt(_T("Background"),_T("ResizePercent"),100,g_szPrivateProfile);
	g_Config.Background.byResizePercent=(INRANGE(0,g_Config.Background.byResizePercent,100))?g_Config.Background.byResizePercent:100;
	g_Config.Background.byOpacity=(BYTE)GetPrivateProfileInt(_T("Background"),_T("Opacity"),100,g_szPrivateProfile);
	g_Config.Background.byOpacity=(INRANGE(0,g_Config.Background.byOpacity,100))?g_Config.Background.byOpacity:100;


	//表示操作
	for(UINT i=1;i<SCC_CENTER;i++){
		g_Config.ShowWindow.ccCursorCorners[i].bValid=GetPrivateProfileInt(_T("ShowWindow"),CursorCorner_Table[i].szKeyName,(i==SCC_RIGHTBOTTOM)?1:0,g_szPrivateProfile)!=0;
		g_Config.ShowWindow.ccCursorCorners[i].eDestCorner=(SCC_CORNERS)GetPrivateProfileInt(_T("ShowWindow"),CursorCorner_Table[i].szKeyNameCursor,CursorCorner_Table[i].eDefault,g_szPrivateProfile);
	}
	//選択項目にカーソルを移動[マウスの移動]
	g_Config.ShowWindow.bMouseMoveCursorSelectedWindow=GetPrivateProfileInt(_T("ShowWindow"),_T("MouseMoveCursorSelectedWindow"),0,g_szPrivateProfile)!=0;
	//選択項目にカーソルを移動[ホットキー]
	g_Config.ShowWindow.bHotKeyMoveCursorSelectedWindow=GetPrivateProfileInt(_T("ShowWindow"),_T("HotKeyCursorMouseSelectedWindow"),0,g_szPrivateProfile)!=0;
	g_Config.ShowWindow.bMouseWheel=GetPrivateProfileInt(_T("ShowWindow"),_T("MouseWheel"),0,g_szPrivateProfile)!=0;
	g_Config.ShowWindow.eHotKeyCursorCorner=(SCC_CORNERS)GetPrivateProfileInt(_T("ShowWindow"),_T("HotKeyCursorCorner"),0,g_szPrivateProfile);
	g_Config.ShowWindow.wHotKey=(WORD)GetPrivateProfileInt(_T("ShowWindow"),_T("HotKey"),23046,g_szPrivateProfile);
	g_Config.ShowWindow.bMoveCursorAtStart=GetPrivateProfileInt(_T("ShowWindow"),_T("MoveCursorAtStart"),0,g_szPrivateProfile)!=0;
	g_Config.ShowWindow.iCursorCornerDelay=GetPrivateProfileInt(_T("ShowWindow"),_T("CursorCornerDelay"),0,g_szPrivateProfile);
	g_Config.ShowWindow.bMouseEachMonitor=GetPrivateProfileInt(_T("ShowWindow"),_T("MouseEachMonitor"),true,g_szPrivateProfile)!=0;
	g_Config.ShowWindow.iMouseCursorMarginHorizontal=GetPrivateProfileInt(_T("ShowWindow"),_T("MouseCursorMarginHorizontal"),20,g_szPrivateProfile);
	g_Config.ShowWindow.iMouseCursorMarginVertical=GetPrivateProfileInt(_T("ShowWindow"),_T("MouseCursorMarginVertical"),20,g_szPrivateProfile);
	g_Config.ShowWindow.iHotKeyCursorMarginHorizontal=GetPrivateProfileInt(_T("ShowWindow"),_T("HotKeyCursorMarginHorizontal"),20,g_szPrivateProfile);
	g_Config.ShowWindow.iHotKeyCursorMarginVertical=GetPrivateProfileInt(_T("ShowWindow"),_T("HotKeyCursorMarginVertical"),20,g_szPrivateProfile);
	//カーソルを閉じ込める
	g_Config.ShowWindow.iMouseClipCursorMode=GetPrivateProfileInt(_T("ShowWindow"),_T("MouseClipCursorMode"),CLIP_CURSOR_NO,g_szPrivateProfile);
	g_Config.ShowWindow.iMouseClipCursorMode=(INRANGE(CLIP_CURSOR_NO,g_Config.ShowWindow.iMouseClipCursorMode,CLIP_CURSOR_ALL))?g_Config.ShowWindow.iMouseClipCursorMode:CLIP_CURSOR_NO;
	g_Config.ShowWindow.iMouseClipCursorTime=GetPrivateProfileInt(_T("ShowWindow"),_T("MouseClipCursorTime"),0,g_szPrivateProfile);
	g_Config.ShowWindow.iHotKeyClipCursorMode=GetPrivateProfileInt(_T("ShowWindow"),_T("HotKeyClipCursorMode"),CLIP_CURSOR_NO,g_szPrivateProfile);
	g_Config.ShowWindow.iHotKeyClipCursorMode=(INRANGE(CLIP_CURSOR_NO,g_Config.ShowWindow.iHotKeyClipCursorMode,CLIP_CURSOR_ALL))?g_Config.ShowWindow.iHotKeyClipCursorMode:CLIP_CURSOR_NO;
	g_Config.ShowWindow.iHotKeyClipCursorTime=GetPrivateProfileInt(_T("ShowWindow"),_T("HotKeyClipCursorTime"),0,g_szPrivateProfile);
	//画面端と判断する端からの距離
	g_Config.ShowWindow.iLeftCornerDistance=GetPrivateProfileInt(_T("ShowWindow"),_T("LeftCornerDistance"),3,g_szPrivateProfile);
	g_Config.ShowWindow.iLeftCornerDistance=(INRANGE(0,g_Config.ShowWindow.iLeftCornerDistance,1024))?g_Config.ShowWindow.iLeftCornerDistance:3;
	g_Config.ShowWindow.iRightCornerDistance=GetPrivateProfileInt(_T("ShowWindow"),_T("RightCornerDistance"),3,g_szPrivateProfile);
	g_Config.ShowWindow.iRightCornerDistance=(INRANGE(0,g_Config.ShowWindow.iRightCornerDistance,1024))?g_Config.ShowWindow.iRightCornerDistance:3;
	g_Config.ShowWindow.iTopCornerDistance=GetPrivateProfileInt(_T("ShowWindow"),_T("TopCornerDistance"),3,g_szPrivateProfile);
	g_Config.ShowWindow.iTopCornerDistance=(INRANGE(0,g_Config.ShowWindow.iTopCornerDistance,1024))?g_Config.ShowWindow.iTopCornerDistance:3;
	g_Config.ShowWindow.iBottomCornerDistance=GetPrivateProfileInt(_T("ShowWindow"),_T("BottomCornerDistance"),3,g_szPrivateProfile);
	g_Config.ShowWindow.iBottomCornerDistance=(INRANGE(0,g_Config.ShowWindow.iBottomCornerDistance,1024))?g_Config.ShowWindow.iBottomCornerDistance:3;
	//フルスクリーン時は無効
	g_Config.ShowWindow.bMouseDisableInFullScreenMode=GetPrivateProfileInt(_T("ShowWindow"),_T("MouseDisableInFullScreenMode"),false,g_szPrivateProfile)!=0;
	g_Config.ShowWindow.bHotKeyDisableInFullScreenMode=GetPrivateProfileInt(_T("ShowWindow"),_T("HotKeyDisableInFullScreenMode"),false,g_szPrivateProfile)!=0;


	//ショートカットキー
	//必ず初期化
	g_Config.ShortcutKey.iSize=0;

	if(GetPrivateProfileInt(_T("ShortcutKey"),NULL,0,g_szPrivateProfile)!=
	   GetPrivateProfileInt(_T("ShortcutKey"),NULL,1,g_szPrivateProfile)){
		SKEY sKey[]={
			{VK_ESCAPE,IDM_KEY_CANCEL},
			{VK_SPACE,IDM_KEY_CENTERCURSOR},
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
	g_Config.DefaultItemDesign.iFontSize=GetPrivateProfileInt(_T("DefaultItemDesign"),_T("FontSize"),MINIMUM_FONTSIZE,g_szPrivateProfile);
	g_Config.DefaultItemDesign.iFontSize=(INRANGE(MINIMUM_FONTSIZE,g_Config.DefaultItemDesign.iFontSize,MAXIMUM_FONTSIZE))?g_Config.DefaultItemDesign.iFontSize:MINIMUM_FONTSIZE;
	g_Config.DefaultItemDesign.iFontStyle=GetPrivateProfileInt(_T("DefaultItemDesign"),_T("FontStyle"),0,g_szPrivateProfile);

	GetPrivateProfileString(_T("DefaultItemDesign"),_T("TextColor"),_T("#FFFFFF"),szTmp,ARRAY_SIZEOF(szTmp),g_szPrivateProfile);
	g_Config.DefaultItemDesign.clrText=StringToCOLORREF(szTmp);

	GetPrivateProfileString(_T("DefaultItemDesign"),_T("BackColor"),_T("#454545"),szTmp,ARRAY_SIZEOF(szTmp),g_szPrivateProfile);
	g_Config.DefaultItemDesign.clrTextBk=StringToCOLORREF(szTmp);

	GetPrivateProfileString(_T("DefaultItemDesign"),_T("BackGradientEndColor"),_T("#6C6C6C"),szTmp,ARRAY_SIZEOF(szTmp),g_szPrivateProfile);
	g_Config.DefaultItemDesign.clrTextBkGradientEnd=StringToCOLORREF(szTmp);


	//選択行
	GetPrivateProfileString(_T("SelectedItemDesign"),_T("Font"),NULL,g_Config.SelectedItemDesign.szFont,ARRAY_SIZEOF(g_Config.SelectedItemDesign.szFont),g_szPrivateProfile);
	g_Config.SelectedItemDesign.iFontSize=GetPrivateProfileInt(_T("SelectedItemDesign"),_T("FontSize"),MINIMUM_FONTSIZE,g_szPrivateProfile);
	g_Config.SelectedItemDesign.iFontSize=(INRANGE(MINIMUM_FONTSIZE,g_Config.SelectedItemDesign.iFontSize,MAXIMUM_FONTSIZE))?g_Config.SelectedItemDesign.iFontSize:MINIMUM_FONTSIZE;
	g_Config.SelectedItemDesign.iFontStyle=GetPrivateProfileInt(_T("SelectedItemDesign"),_T("FontStyle"),SFONT_BOLD,g_szPrivateProfile);

	GetPrivateProfileString(_T("SelectedItemDesign"),_T("TextColor"),_T("#FFFFFF"),szTmp,ARRAY_SIZEOF(szTmp),g_szPrivateProfile);
	g_Config.SelectedItemDesign.clrText=StringToCOLORREF(szTmp);

	GetPrivateProfileString(_T("SelectedItemDesign"),_T("BackColor"),_T("#347395"),szTmp,ARRAY_SIZEOF(szTmp),g_szPrivateProfile);
	g_Config.SelectedItemDesign.clrTextBk=StringToCOLORREF(szTmp);

	GetPrivateProfileString(_T("SelectedItemDesign"),_T("BackGradientEndColor"),_T("#89BBD6"),szTmp,ARRAY_SIZEOF(szTmp),g_szPrivateProfile);
	g_Config.SelectedItemDesign.clrTextBkGradientEnd=StringToCOLORREF(szTmp);


	//DirectWrite
	g_Config.DirectWrite.bDirectWrite=GetPrivateProfileInt(_T("DirectWrite"),_T("DirectWrite"),true,g_szPrivateProfile)!=0;
	g_Config.DirectWrite.uRenderingMode=GetPrivateProfileInt(_T("DirectWrite"),_T("RenderingMode"),DWRITE_RENDERING_MODE_DEFAULT,g_szPrivateProfile);
	//v141_xp対応のためDWRITE_RENDERING_MODE_NATURAL_SYMMETRIC=5
	g_Config.DirectWrite.uRenderingMode=(INRANGE(DWRITE_RENDERING_MODE_DEFAULT,g_Config.DirectWrite.uRenderingMode,/*DWRITE_RENDERING_MODE_NATURAL_SYMMETRIC*/5))?g_Config.DirectWrite.uRenderingMode:/*DWRITE_RENDERING_MODE_NATURAL_SYMMETRIC*/5;
	g_Config.DirectWrite.uGamma=GetPrivateProfileInt(_T("DirectWrite"),_T("Gamma"),DEFAULT_GAMMA,g_szPrivateProfile);
	g_Config.DirectWrite.uGamma=(INRANGE(MINIMUM_GAMMA,g_Config.DirectWrite.uGamma,MAXIMUM_GAMMA))?g_Config.DirectWrite.uGamma:1900;
	g_Config.DirectWrite.uEnhancedContrast=GetPrivateProfileInt(_T("DirectWrite"),_T("EnhancedContrast"),MINIMUM_ENHANCEDCONTRAST,g_szPrivateProfile);
	g_Config.DirectWrite.uEnhancedContrast=(INRANGE(MINIMUM_ENHANCEDCONTRAST,g_Config.DirectWrite.uEnhancedContrast,MAXIMUM_ENHANCEDCONTRAST))?g_Config.DirectWrite.uEnhancedContrast:MINIMUM_ENHANCEDCONTRAST;
	g_Config.DirectWrite.uClearTypeLevel=GetPrivateProfileInt(_T("DirectWrite"),_T("ClearTypeLevel"),MAXIMUM_CLEARTYPELEVEL,g_szPrivateProfile);
	g_Config.DirectWrite.uClearTypeLevel=(INRANGE(MINIMUM_CLEARTYPELEVEL,g_Config.DirectWrite.uClearTypeLevel,MAXIMUM_CLEARTYPELEVEL))?g_Config.DirectWrite.uClearTypeLevel:MAXIMUM_CLEARTYPELEVEL;


	//除外するファイル名たち(;区切り)
	GetPrivateProfileString(_T("Exclude"),_T("FileName"),NULL,g_Config.Exclude.szFileName,ARRAY_SIZEOF(g_Config.Exclude.szFileName),g_szPrivateProfile);
	//末尾に';'を追加
	if(lstrlen(g_Config.Exclude.szFileName)&&
	   g_Config.Exclude.szFileName[lstrlen(g_Config.Exclude.szFileName)-1]!=';'){
		lstrcat(g_Config.Exclude.szFileName,_T(";"));
	}


	//ウインドウリストを表示するモニタ
	g_Config.MultiMonitor.iActiveMonitorMode=GetPrivateProfileInt(_T("MultiMonitor"),_T("ActiveMonitorMode"),ACTIVEMONITOR_BY_CURSOR,g_szPrivateProfile);
	g_Config.MultiMonitor.iActiveMonitorMode=(INRANGE(ACTIVEMONITOR_BY_CURSOR,g_Config.MultiMonitor.iActiveMonitorMode,ACTIVEMONITOR_BY_NAME))?g_Config.MultiMonitor.iActiveMonitorMode:ACTIVEMONITOR_BY_CURSOR;
	//ウインドウリストを表示するモニタ名
	GetPrivateProfileString(_T("MultiMonitor"),_T("MonitorName"),NULL,g_Config.MultiMonitor.szMonitorName,ARRAY_SIZEOF(g_Config.MultiMonitor.szMonitorName),g_szPrivateProfile);
	//アクティブなモニタのウインドウのみ表示
	g_Config.MultiMonitor.bOnlyActiveMonitor=GetPrivateProfileInt(_T("MultiMonitor"),_T("OnlyActiveMonitor"),false,g_szPrivateProfile)!=0;

	//アクティブな仮想デスクトップのウインドウのみ表示
	g_Config.VirtualDesktop.bOnlyActiveVirtualDesktop=GetPrivateProfileInt(_T("VirtualDesktop"),_T("OnlyActiveVirtualDesktop"),false,g_szPrivateProfile)!=0;

	//コマンド
	bool bCmdExist=false;

	for(UINT i=1;i<=MAX_COMMAND;i++){
		TCHAR szKey[24];

		wsprintf(szKey,_T("Command%u"),i);
		if(GetPrivateProfileInt(szKey,NULL,0,g_szPrivateProfile)==
		   GetPrivateProfileInt(szKey,NULL,1,g_szPrivateProfile)){
			bCmdExist=true;
			break;
		}
	}

	for(UINT i=1;i<=MAX_COMMAND;i++){
		TCHAR szKey[24];

		wsprintf(szKey,_T("Command%u"),i);

		if(!bCmdExist&&i==1){
			lstrcpy(g_Config.Command[i].szCommandName,_T("Open file location(&F)"));
			g_Config.Command[i].uCommandMode=CMDMODE_OPEN;
			g_Config.Command[i].uCommandOption=CMDOPT_CANCEL;
			lstrcpy(g_Config.Command[i].szFilePath,_T("explorer"));
			lstrcpy(g_Config.Command[i].szParameters,_T("%D"));
			lstrcpy(g_Config.Command[i].szWorkingDirectory,_T(""));
			g_Config.Command[i].uCmdShow=SW_SHOWNORMAL;
		}else{
			GetPrivateProfileString(szKey,_T("CommandName"),_T(""),g_Config.Command[i].szCommandName,ARRAY_SIZEOF(g_Config.Command[i].szCommandName),g_szPrivateProfile);

			g_Config.Command[i].uCommandMode=GetPrivateProfileInt(szKey,_T("CommandMode"),CMDMODE_OPEN,g_szPrivateProfile);
			g_Config.Command[i].uCommandMode=(INRANGE(CMDMODE_OPEN,g_Config.Command[i].uCommandMode,CMDMODE_ADMIN))?g_Config.Command[i].uCommandMode:CMDMODE_OPEN;

			g_Config.Command[i].uCommandOption=GetPrivateProfileInt(szKey,_T("CommandOption"),CMDOPT_NONE,g_szPrivateProfile);
			g_Config.Command[i].uCommandOption=(INRANGE(CMDOPT_NONE,g_Config.Command[i].uCommandOption,CMDOPT_CANCEL))?g_Config.Command[i].uCommandOption:CMDOPT_NONE;

			GetPrivateProfileString(szKey,_T("FilePath"),_T(""),g_Config.Command[i].szFilePath,ARRAY_SIZEOF(g_Config.Command[i].szFilePath),g_szPrivateProfile);
			GetPrivateProfileString(szKey,_T("Parameters"),_T(""),g_Config.Command[i].szParameters,ARRAY_SIZEOF(g_Config.Command[i].szParameters),g_szPrivateProfile);
			GetPrivateProfileString(szKey,_T("WorkingDirectory"),_T(""),g_Config.Command[i].szWorkingDirectory,ARRAY_SIZEOF(g_Config.Command[i].szWorkingDirectory),g_szPrivateProfile);
			g_Config.Command[i].uCmdShow=GetPrivateProfileInt(szKey,_T("CommandShow"),SW_SHOWNORMAL,g_szPrivateProfile);
		}
	}

	//ウェブブラウザタブ
	for(UINT i=1;i<=MAX_WEBBROWSER;i++){
		TCHAR szKey[24];

		wsprintf(szKey,_T("WebBrowser%u"),i);
		GetPrivateProfileString(szKey,_T("FilePath"),_T(""),g_Config.WebBrowser[i].szFilePath,ARRAY_SIZEOF(g_Config.WebBrowser[i].szFilePath),g_szPrivateProfile);
		g_Config.WebBrowser[i].iPort=GetPrivateProfileInt(szKey,_T("Port"),-1,g_szPrivateProfile);
	}

	return;
}
