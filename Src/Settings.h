//Settings.h
//設定ダイアログ

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.64
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/


#ifndef TASCHER_SETTINGS_H
#define TASCHER_SETTINGS_H

#include"CommonSettings.h"


//タブ
enum TAB{
	TAB_GENERAL=0,
	TAB_SHOWWINDOW,
	TAB_SHORTCUTKEY,
	TAB_MOUSE,
	TAB_COMMAND,
	TAB_DESIGN,
	TAB_INCREMENTALSEARCH,
	TAB_WEBBROWSER,
	TAB_ADVANCED,
	TAB_VERSION,
};

//設定ウインドウを表示する
INT_PTR ShowSettingsDialog(int iStartTab);

#endif //TASCHER_SETTINGS_H
