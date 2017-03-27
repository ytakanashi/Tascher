//Settings.h
//設定ダイアログ

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.62
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/


#ifndef SETTINGS_H
#define SETTINGS_H

#include"CommonSettings.h"


//インスタンスハンドル
extern HINSTANCE g_hInstance;
//リストビューハンドル
extern HWND g_hListView;

//設定ダイアログが表示されている
extern bool g_bSettingsDialog;

//リストビュー用フォント
extern HFONT g_hDefaultItemFont;
extern HFONT g_hSelectedItemFont;

//タブ
enum TAB{
	TAB_GENERAL=0,
	TAB_SHOWWINDOW,
	TAB_SHORTCUTKEY,
	TAB_MOUSE,
	TAB_DESIGN,
	TAB_ADVANCED,
	TAB_VERSION,
};

#define WM_CLOSETAB (WM_APP+100)

//設定ウインドウを表示する
INT_PTR ShowSettingsDialog(HWND hWnd,int iStartTab);

#endif //SETTINGS_H
