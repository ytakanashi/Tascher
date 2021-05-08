//CommonSettings.h
//設定関係共通ヘッダファイル

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.64
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/


#ifndef TASCHER_COMMONSETTINGS_H
#define TASCHER_COMMONSETTINGS_H

#include"ListView.h"
#include"resources/resource.h"

HMONITOR MonitorFromName(LPCTSTR lpszMonitorName);

#ifndef WM_MOUSEHWHEEL
	//#if (_WIN32_WINNT >= 0x0600)
	#define WM_MOUSEHWHEEL 0x020E
#endif

#ifndef RI_MOUSE_HWHEEL
	//#if(WINVER >= 0x0600)
	#define RI_MOUSE_HWHEEL 0x0800
#endif

//UWPApp判別に使用
#ifndef WS_EX_NOREDIRECTIONBITMAP
	#define WS_EX_NOREDIRECTIONBITMAP 0x00200000L
#endif

#ifndef WM_DPICHANGED
	//#if(WINVER >= 0x0601)
	#define WM_DPICHANGED       0x02E0
#endif



enum SCC_CORNERS;

const int DEFAULT_FILENAME_WIDTH=150;
const int DEFAULT_WINDOWTITLE_WIDTH=600;

const int MINIMUM_ICONMARGIN=0;
const int DEFAULT_ICONMARGIN=10;
const int MAXIMUM_ICONMARGIN=128;

const int MINIMUM_ICONMARGIN_LEFTRIGHT=0;
const int DEFAULT_ICONMARGIN_LEFTRIGHT=10;
const int MAXIMUM_ICONMARGIN_LEFTRIGHT=128;

//タイムアウト
const int DEFAULT_SELECT_TIMEOUT=1000;

//フォント最小サイズ
//0=フォントサイズ変更なし
const int MINIMUM_FONTSIZE=0;
//フォント最大サイズ
const int MAXIMUM_FONTSIZE=30;

//最大ショートカットキー数
const int MAX_SHORTCUTKEY=256;

//最大コマンド数
const int MAX_COMMAND=50;

//DirectWriteパラメータ
const int MINIMUM_GAMMA=1000;
const int DEFAULT_GAMMA=1900;
const int MAXIMUM_GAMMA=3000;

const int MINIMUM_ENHANCEDCONTRAST=0;
const int MAXIMUM_ENHANCEDCONTRAST=200;

const int MINIMUM_CLEARTYPELEVEL=0;
const int MAXIMUM_CLEARTYPELEVEL=100;

const int MAX_WEBBROWSER=5;


enum{
	LISTITEM_ICON,//アイコン
	LISTITEM_FILENAME,//ファイル名
	LISTITEM_WINDOWTITLE,//ウインドウタイトル
	LISTITEM_NUM//要素数
};

enum{
	LISTICON_NO=0,//アイコンなし(大)
	LISTICON_NO_SMALL,//アイコンなし(小)
	LISTICON_SMALL,//小さなアイコン(16x16)
	LISTICON_BIG,//大きなアイコン(32x32)
	LISTICON_BIG48,//大きなアイコン(48x48)
};

enum{
	MATCH_FORWARD=0,//前方一致
	MATCH_PARTICAL,//部分一致
	MATCH_FLEX,//あいまい
};

enum{
	MIGEMO_NO=0,//使用しない
	MIGEMO_LOWERCASE,//小文字入力で有効
	MIGEMO_UPPERCASE,//大文字入力で有効
	MIGEMO_ALWAYS,
};

enum{
	INCREMENTALSEARCH_SEARCHWINDOWDESIGN_DEFAULT=0,//デフォルト
	INCREMENTALSEARCH_SEARCHWINDOWDESIGN_MIGEMOMATCH,//Migemo検索一致
	INCREMENTALSEARCH_SEARCHWINDOWDESIGN_MIGEMONOMATCH,//Migemo検索不一致
	INCREMENTALSEARCH_SEARCHWINDOWDESIGN_NUM,
};

enum{
	THUMBNAIL_NO=0,//表示しない
	THUMBNAIL_FIRST_COLUMN,//1番目のカラム上に表示
	THUMBNAIL_SECOND_COLUMN,//2番目のカラム上に表示
	THUMBNAIL_LEFT_SIDE,//画面外左側に表示
	THUMBNAIL_RIGHT_SIDE,//画面外右側に表示
};

enum{
	THUMBNAIL_VERTICALALIGN_TOP,//上揃え
	THUMBNAIL_VERTICALALIGN_MIDDLE,//真ん中くらい
	THUMBNAIL_VERTICALALIGN_BOTTOM,//下揃え
	THUMBNAIL_VERTICALALIGN_SELECTED_ITEM,//選択項目
};

enum{
	CLIP_CURSOR_NO=0,
	CLIP_CURSOR_TOP_BOTTOM,
	CLIP_CURSOR_ALL
};

enum{
	ACTIVEMONITOR_BY_CURSOR=0,
	ACTIVEMONITOR_BY_WINDOW,
	ACTIVEMONITOR_BY_NAME
};


typedef struct{
	WORD wKey;
	UINT uCmdId;
}SKEY;

enum{
	//左クリック
	SMOUSE_LCLICK=0,
	//左ダブルクリック
	SMOUSE_LDCLICK,
	//右クリック
	SMOUSE_RCLICK,
	//右ダブルクリック
	SMOUSE_RDCLICK,
	//Shift + 左クリック
	SMOUSE_LSCLICK,
	//Shift + 左ダブルクリック
	SMOUSE_LSDCLICK,
	//Shift + 右クリック
	SMOUSE_RSCLICK,
	//Shift + 右ダブルクリック
	SMOUSE_RSDCLICK,
	//Ctrl + 左クリック
	SMOUSE_LCCLICK,
	//Ctrl + 左ダブルクリック
	SMOUSE_LCDCLICK,
	//Ctrl + 右クリック
	SMOUSE_RCCLICK,
	//Ctrl + 右ダブルクリック
	SMOUSE_RCDCLICK,
	//Shift + Ctrl + 左クリック
	SMOUSE_LSCCLICK,
	//Shift + Ctrl + 左ダブルクリック
	SMOUSE_LSCDCLICK,
	//Shift + Ctrl + 右クリック
	SMOUSE_RSCCLICK,
	//Shift + Ctrl + 右ダブルクリック
	SMOUSE_RSCDCLICK,
	//マウスホイール上
	SMOUSE_UWHEEL,
	//マウスホイール下
	SMOUSE_DWHEEL,
	//マウスホイール左
	SMOUSE_LHWHEEL,
	//マウスホイール右
	SMOUSE_RHWHEEL,
	//左クリック + マウスホイール上
	SMOUSE_LCLICKUWHEEL,
	//左クリック + マウスホイール下
	SMOUSE_LCLICKDWHEEL,
	//左クリック + マウスホイール左
	SMOUSE_LCLICKLHWHEEL,
	//左クリック + マウスホイール右
	SMOUSE_LCLICKRHWHEEL,
	//右クリック + マウスホイール上
	SMOUSE_RCLICKUWHEEL,
	//右クリック + マウスホイール下
	SMOUSE_RCLICKDWHEEL,
	//右クリック + マウスホイール左
	SMOUSE_RCLICKLHWHEEL,
	//右クリック + マウスホイール右
	SMOUSE_RCLICKRHWHEEL,
	//Shift + マウスホイール上
	SMOUSE_USWHEEL,
	//Shift + マウスホイール下
	SMOUSE_DSWHEEL,
	//Shift + マウスホイール左
	SMOUSE_LSHWHEEL,
	//Shift + マウスホイール右
	SMOUSE_RSHWHEEL,
	//Ctrl + マウスホイール上
	SMOUSE_UCWHEEL,
	//Ctrl + マウスホイール下
	SMOUSE_DCWHEEL,
	//Ctrl + マウスホイール左
	SMOUSE_LCHWHEEL,
	//Ctrl + マウスホイール右
	SMOUSE_RCHWHEEL,
	//Shift + Ctrl + マウスホイール上
	SMOUSE_USCWHEEL,
	//Shift + Ctrl + マウスホイール下
	SMOUSE_DSCWHEEL,
	//Shift + Ctrl + マウスホイール左
	SMOUSE_LSCHWHEEL,
	//Shift + Ctrl + マウスホイール右
	SMOUSE_RSCHWHEEL,
	//中央クリック
	SMOUSE_MCLICK,
	//中央ダブルクリック
	SMOUSE_MDCLICK,
	//Shift + 中央クリック
	SMOUSE_MSCLICK,
	//Shift + 中央ダブルクリック
	SMOUSE_MSDCLICK,
	//Ctrl + 中央クリック
	SMOUSE_MCCLICK,
	//Ctrl + 中央ダブルクリック
	SMOUSE_MCDCLICK,
	//Shift + Ctrl + 中央クリック
	SMOUSE_MSCCLICK,
	//Shift + Ctrl + 中央ダブルクリック
	SMOUSE_MSCDCLICK,
	//X1クリック
	SMOUSE_X1CLICK,
	//X1ダブルクリック
	SMOUSE_X1DCLICK,
	//X2クリック
	SMOUSE_X2CLICK,
	//X2ダブルクリック
	SMOUSE_X2DCLICK,
	//Shift + X1クリック
	SMOUSE_X1SCLICK,
	//Shift + X1ダブルクリック
	SMOUSE_X1SDCLICK,
	//Shift + X2クリック
	SMOUSE_X2SCLICK,
	//Shift + X2ダブルクリック
	SMOUSE_X2SDCLICK,
	//Ctrl + X1クリック
	SMOUSE_X1CCLICK,
	//Ctrl + X1ダブルクリック
	SMOUSE_X1CDCLICK,
	//Ctrl + X2クリック
	SMOUSE_X2CCLICK,
	//Ctrl + X2ダブルクリック
	SMOUSE_X2CDCLICK,
	//Shift + Ctrl + X1クリック
	SMOUSE_X1SCCLICK,
	//Shift + Ctrl + X1ダブルクリック
	SMOUSE_X1SCDCLICK,
	//Shift + Ctrl + X2クリック
	SMOUSE_X2SCCLICK,
	//Shift + Ctrl + X2ダブルクリック
	SMOUSE_X2SCDCLICK,
	//X1クリック + マウスホイール上
	SMOUSE_X1CLICKUWHEEL,
	//X1クリック + マウスホイール下
	SMOUSE_X1CLICKDWHEEL,
	//X1クリック + マウスホイール左
	SMOUSE_X1CLICKLHWHEEL,
	//X1クリック + マウスホイール右
	SMOUSE_X1CLICKRHWHEEL,
	//X2クリック + マウスホイール上
	SMOUSE_X2CLICKUWHEEL,
	//X2クリック + マウスホイール下
	SMOUSE_X2CLICKDWHEEL,
	//X2クリック + マウスホイール左
	SMOUSE_X2CLICKLHWHEEL,
	//X2クリック + マウスホイール右
	SMOUSE_X2CLICKRHWHEEL,

	SMOUSE_NUM,
};

typedef struct{
	UINT uMouseType;
	UINT uCmdId;
}SMOUSE;

enum{
	MOUSEWHEEL_NO=0,
	MOUSEWHEEL_UP,
	MOUSEWHEEL_DOWN,
	MOUSEHWHEEL_LEFT,
	MOUSEHWHEEL_RIGHT,
};

enum{
	MODF_SHIFT=1<<0,
	MODF_CONTROL=1<<1,
	MODF_LBUTTON=1<<2,
	MODF_RBUTTON=1<<3,
	MODF_MBUTTON=1<<4,
	MODF_X1BUTTON=1<<5,
	MODF_X2BUTTON=1<<6
};

typedef struct{
	//関連するWM_*(ダブルクリックはクリックメッセージ)
	UINT uMessageType;
	UINT uMouseWheel;
	BYTE byMod;
	LPCTSTR szDescript;
}SMOUSETYPE_TABLE;

static SMOUSETYPE_TABLE MouseType_Table[]={
	{
		WM_LBUTTONDOWN,
		MOUSEWHEEL_NO,
		MODF_LBUTTON,
		_T("左クリック"),
	},
	{
		WM_LBUTTONDBLCLK,
		MOUSEWHEEL_NO,
		MODF_LBUTTON,
		_T("左ダブルクリック"),
	},
	{
		WM_RBUTTONDOWN,
		MOUSEWHEEL_NO,
		MODF_RBUTTON,
		_T("右クリック"),
	},
	{
		WM_RBUTTONDBLCLK,
		MOUSEWHEEL_NO,
		MODF_RBUTTON,
		_T("右ダブルクリック"),
	},
	{
		WM_LBUTTONDOWN,
		MOUSEWHEEL_NO,
		MODF_SHIFT|MODF_LBUTTON,
		_T("Shift + 左クリック"),
	},
	{
		WM_LBUTTONDBLCLK,
		MOUSEWHEEL_NO,
		MODF_SHIFT|MODF_LBUTTON,
		_T("Shift + 左ダブルクリック"),
	},
	{
		WM_RBUTTONDOWN,
		MOUSEWHEEL_NO,
		MODF_SHIFT|MODF_RBUTTON,
		_T("Shift + 右クリック"),
	},
	{
		WM_RBUTTONDBLCLK,
		MOUSEWHEEL_NO,
		MODF_SHIFT|MODF_RBUTTON,
		_T("Shift + 右ダブルクリック"),
	},
	{
		WM_LBUTTONDOWN,
		MOUSEWHEEL_NO,
		MODF_CONTROL|MODF_LBUTTON,
		_T("Ctrl + 左クリック"),
	},
	{
		WM_LBUTTONDBLCLK,
		MOUSEWHEEL_NO,
		MODF_CONTROL|MODF_LBUTTON,
		_T("Ctrl + 左ダブルクリック"),
	},
	{
		WM_RBUTTONDOWN,
		MOUSEWHEEL_NO,
		MODF_CONTROL|MODF_RBUTTON,
		_T("Ctrl + 右クリック"),
	},
	{
		WM_RBUTTONDBLCLK,
		MOUSEWHEEL_NO,
		MODF_CONTROL|MODF_RBUTTON,
		_T("Ctrl + 右ダブルクリック"),
	},
	{
		WM_LBUTTONDOWN,
		MOUSEWHEEL_NO,
		MODF_SHIFT|MODF_CONTROL|MODF_LBUTTON,
		_T("Shift + Ctrl + 左クリック"),
	},
	{
		WM_LBUTTONDBLCLK,
		MOUSEWHEEL_NO,
		MODF_SHIFT|MODF_CONTROL|MODF_LBUTTON,
		_T("Shift + Ctrl + 左ダブルクリック"),
	},
	{
		WM_RBUTTONDOWN,
		MOUSEWHEEL_NO,
		MODF_SHIFT|MODF_CONTROL|MODF_RBUTTON,
		_T("Shift + Ctrl + 右クリック"),
	},
	{
		WM_RBUTTONDBLCLK,
		MOUSEWHEEL_NO,
		MODF_SHIFT|MODF_CONTROL|MODF_RBUTTON,
		_T("Shift + Ctrl + 右ダブルクリック"),
	},
	{
		WM_MOUSEWHEEL,
		MOUSEWHEEL_UP,
		0,
		_T("マウスホイール上"),
	},
	{
		WM_MOUSEWHEEL,
		MOUSEWHEEL_DOWN,
		0,
		_T("マウスホイール下"),
	},
	{
		WM_MOUSEHWHEEL,
		MOUSEHWHEEL_LEFT,
		0,
		_T("マウスホイール左"),
	},
	{
		WM_MOUSEHWHEEL,
		MOUSEHWHEEL_RIGHT,
		0,
		_T("マウスホイール右"),
	},
	{
		WM_LBUTTONDOWN,
		MOUSEWHEEL_UP,
		MODF_LBUTTON,
		_T("左クリック + マウスホイール上"),
	},
	{
		WM_LBUTTONDOWN,
		MOUSEWHEEL_DOWN,
		MODF_LBUTTON,
		_T("左クリック + マウスホイール下"),
	},
	{
		WM_LBUTTONDOWN,
		MOUSEHWHEEL_LEFT,
		MODF_LBUTTON,
		_T("左クリック + マウスホイール左"),
	},
	{
		WM_LBUTTONDOWN,
		MOUSEHWHEEL_RIGHT,
		MODF_LBUTTON,
		_T("左クリック + マウスホイール右"),
	},
	{
		WM_RBUTTONDOWN,
		MOUSEWHEEL_UP,
		MODF_RBUTTON,
		_T("右クリック + マウスホイール上"),
	},
	{
		WM_RBUTTONDOWN,
		MOUSEWHEEL_DOWN,
		MODF_RBUTTON,
		_T("右クリック + マウスホイール下"),
	},
	{
		WM_RBUTTONDOWN,
		MOUSEHWHEEL_LEFT,
		MODF_RBUTTON,
		_T("右クリック + マウスホイール左"),
	},
	{
		WM_RBUTTONDOWN,
		MOUSEHWHEEL_RIGHT,
		MODF_RBUTTON,
		_T("右クリック + マウスホイール右"),
	},
	{
		WM_MOUSEWHEEL,
		MOUSEWHEEL_UP,
		MODF_SHIFT,
		_T("Shift + マウスホイール上"),
	},
	{
		WM_MOUSEWHEEL,
		MOUSEWHEEL_DOWN,
		MODF_SHIFT,
		_T("Shift + マウスホイール下"),
	},
	{
		WM_MOUSEHWHEEL,
		MOUSEHWHEEL_LEFT,
		MODF_SHIFT,
		_T("Shift + マウスホイール左"),
	},
	{
		WM_MOUSEHWHEEL,
		MOUSEHWHEEL_RIGHT,
		MODF_SHIFT,
		_T("Shift + マウスホイール右"),
	},
	{
		WM_MOUSEWHEEL,
		MOUSEWHEEL_UP,
		MODF_CONTROL,
		_T("Ctrl + マウスホイール上"),
	},
	{
		WM_MOUSEWHEEL,
		MOUSEWHEEL_DOWN,
		MODF_CONTROL,
		_T("Ctrl + マウスホイール下"),
	},
	{
		WM_MOUSEHWHEEL,
		MOUSEHWHEEL_LEFT,
		MODF_CONTROL,
		_T("Ctrl + マウスホイール左"),
	},
	{
		WM_MOUSEHWHEEL,
		MOUSEHWHEEL_RIGHT,
		MODF_CONTROL,
		_T("Ctrl + マウスホイール右"),
	},
	{
		WM_MOUSEWHEEL,
		MOUSEWHEEL_UP,
		MODF_SHIFT|MODF_CONTROL,
		_T("Shift + Ctrl + マウスホイール上"),
	},
	{
		WM_MOUSEWHEEL,
		MOUSEWHEEL_DOWN,
		MODF_SHIFT|MODF_CONTROL,
		_T("Shift + Ctrl + マウスホイール下"),
	},
	{
		WM_MOUSEHWHEEL,
		MOUSEHWHEEL_LEFT,
		MODF_SHIFT|MODF_CONTROL,
		_T("Shift + Ctrl + マウスホイール左"),
	},
	{
		WM_MOUSEHWHEEL,
		MOUSEHWHEEL_RIGHT,
		MODF_SHIFT|MODF_CONTROL,
		_T("Shift + Ctrl + マウスホイール右"),
	},
	{
		WM_MBUTTONDOWN,
		MOUSEWHEEL_NO,
		MODF_MBUTTON,
		_T("中央クリック"),
	},
	{
		WM_MBUTTONDBLCLK,
		MOUSEWHEEL_NO,
		MODF_MBUTTON,
		_T("中央ダブルクリック"),
	},
	{
		WM_MBUTTONDOWN,
		MOUSEWHEEL_NO,
		MODF_SHIFT|MODF_MBUTTON,
		_T("Shift + 中央クリック"),
	},
	{
		WM_MBUTTONDBLCLK,
		MOUSEWHEEL_NO,
		MODF_SHIFT|MODF_MBUTTON,
		_T("Shift + 中央ダブルクリック"),
	},
	{
		WM_MBUTTONDOWN,
		MOUSEWHEEL_NO,
		MODF_CONTROL|MODF_MBUTTON,
		_T("Ctrl + 中央クリック"),
	},
	{
		WM_MBUTTONDBLCLK,
		MOUSEWHEEL_NO,
		MODF_CONTROL|MODF_MBUTTON,
		_T("Ctrl + 中央ダブルクリック"),
	},
	{
		WM_MBUTTONDOWN,
		MOUSEWHEEL_NO,
		MODF_SHIFT|MODF_CONTROL|MODF_MBUTTON,
		_T("Shift + Ctrl + 中央クリック"),
	},
	{
		WM_MBUTTONDBLCLK,
		MOUSEWHEEL_NO,
		MODF_SHIFT|MODF_CONTROL|MODF_MBUTTON,
		_T("Shift + Ctrl + 中央ダブルクリック"),
	},
	{
		WM_XBUTTONDOWN,
		MOUSEWHEEL_NO,
		MODF_X1BUTTON,
		_T("X1クリック"),
	},
	{
		WM_XBUTTONDBLCLK,
		MOUSEWHEEL_NO,
		MODF_X1BUTTON,
		_T("X1ダブルクリック"),
	},
	{
		WM_XBUTTONDOWN,
		MOUSEWHEEL_NO,
		MODF_X2BUTTON,
		_T("X2クリック"),
	},
	{
		WM_XBUTTONDBLCLK,
		MOUSEWHEEL_NO,
		MODF_X2BUTTON,
		_T("X2ダブルクリック"),
	},
	{
		WM_XBUTTONDOWN,
		MOUSEWHEEL_NO,
		MODF_SHIFT|MODF_X1BUTTON,
		_T("Shift + X1クリック"),
	},
	{
		WM_XBUTTONDBLCLK,
		MOUSEWHEEL_NO,
		MODF_SHIFT|MODF_X1BUTTON,
		_T("Shift + X1ダブルクリック"),
	},
	{
		WM_XBUTTONDOWN,
		MOUSEWHEEL_NO,
		MODF_SHIFT,
		_T("Shift + X2クリック"),
	},
	{
		WM_XBUTTONDBLCLK,
		MOUSEWHEEL_NO,
		MODF_SHIFT|MODF_X2BUTTON,
		_T("Shift + X2ダブルクリック"),
	},
	{
		WM_XBUTTONDOWN,
		MOUSEWHEEL_NO,
		MODF_CONTROL|MODF_X1BUTTON,
		_T("Ctrl + X1クリック"),
	},
	{
		WM_XBUTTONDBLCLK,
		MOUSEWHEEL_NO,
		MODF_CONTROL|MODF_X1BUTTON,
		_T("Ctrl + X1ダブルクリック"),
	},
	{
		WM_XBUTTONDOWN,
		MOUSEWHEEL_NO,
		MODF_CONTROL|MODF_X2BUTTON,
		_T("Ctrl + X2クリック"),
	},
	{
		WM_XBUTTONDBLCLK,
		MOUSEWHEEL_NO,
		MODF_CONTROL|MODF_X2BUTTON,
		_T("Ctrl + X2ダブルクリック"),
	},
	{
		WM_XBUTTONDOWN,
		MOUSEWHEEL_NO,
		MODF_SHIFT|MODF_CONTROL|MODF_X1BUTTON,
		_T("Shift + Ctrl + X1クリック"),
	},
	{
		WM_XBUTTONDBLCLK,
		MOUSEWHEEL_NO,
		MODF_SHIFT|MODF_CONTROL|MODF_X1BUTTON,
		_T("Shift + Ctrl + X1ダブルクリック"),
	},
	{
		WM_XBUTTONDOWN,
		MOUSEWHEEL_NO,
		MODF_SHIFT|MODF_CONTROL|MODF_X2BUTTON,
		_T("Shift + Ctrl + X2クリック"),
	},
	{
		WM_XBUTTONDBLCLK,
		MOUSEWHEEL_NO,
		MODF_SHIFT|MODF_CONTROL|MODF_X2BUTTON,
		_T("Shift + Ctrl + X2ダブルクリック"),
	},
	{
		WM_XBUTTONDOWN,
		MOUSEWHEEL_UP,
		MODF_X1BUTTON,
		_T("X1クリック + マウスホイール上"),
	},
	{
		WM_XBUTTONDOWN,
		MOUSEWHEEL_DOWN,
		MODF_X1BUTTON,
		_T("X1クリック + マウスホイール下"),
	},
	{
		WM_XBUTTONDOWN,
		MOUSEHWHEEL_LEFT,
		MODF_X1BUTTON,
		_T("X1クリック + マウスホイール左"),
	},
	{
		WM_XBUTTONDOWN,
		MOUSEHWHEEL_RIGHT,
		MODF_X1BUTTON,
		_T("X1クリック + マウスホイール右"),
	},
	{
		WM_XBUTTONDOWN,
		MOUSEWHEEL_UP,
		MODF_X2BUTTON,
		_T("X2クリック + マウスホイール上"),
	},
	{
		WM_XBUTTONDOWN,
		MOUSEWHEEL_DOWN,
		MODF_X2BUTTON,
		_T("X2クリック + マウスホイール下"),
	},
	{
		WM_XBUTTONDOWN,
		MOUSEHWHEEL_LEFT,
		MODF_X2BUTTON,
		_T("X2クリック + マウスホイール左"),
	},
	{
		WM_XBUTTONDOWN,
		MOUSEHWHEEL_RIGHT,
		MODF_X2BUTTON,
		_T("X2クリック + マウスホイール右"),
	},
};

typedef struct{
	UINT uCmdId;
	LPCTSTR szCmd;
	LPCTSTR szDescript;
}SHORTCUTKEYCMD_TABLE;

static SHORTCUTKEYCMD_TABLE ShortcutKeyCmd_Table[]={
	{
		IDM_KEY_SWITCH,
		_T("switch"),
		_T("切り替え"),
	},
	{
		IDM_KEY_CANCEL,
		_T("cancel"),
		_T("キャンセル"),
	},
	{
		IDM_KEY_RESET,
		_T("reset"),
		_T("操作リセット"),
	},
	{
		IDM_KEY_PREV,
		_T("prev"),
		_T("前のアイテムを選択"),
	},
	{
		IDM_KEY_NEXT,
		_T("next"),
		_T("次のアイテムを選択"),
	},
	{
		IDM_KEY_FIRST,
		_T("first"),
		_T("先頭のアイテムを選択"),
	},
	{
		IDM_KEY_LAST,
		_T("last"),
		_T("末尾のアイテムを選択"),
	},
	{
		IDM_KEY_SETTINGS,
		_T("settings"),
		_T("設定"),
	},
	{
		IDM_KEY_QUIT,
		_T("quit"),
		_T("終了"),
	},
	{
		IDM_KEY_SWAP,
		_T("swap"),
		_T("カラム入れ替え"),
	},
	{
		IDM_KEY_FORWARD_MATCH,
		_T("forwardmatch"),
		_T("先頭一致検索"),
	},
	{
		IDM_KEY_PARTIAL_MATCH,
		_T("partialmatch"),
		_T("部分一致検索"),
	},
	{
		IDM_KEY_FLEX_MATCH,
		_T("flexmatch"),
		_T("あいまい検索"),
	},
	{
		IDM_KEY_SEARCHSCOPE,
		_T("searchscope"),
		_T("検索範囲切り替え"),
	},
	{
		IDM_KEY_UNIQUE_WINDOW,
		_T("uniquewindow"),
		_T("候補が1つなら切り替えする/しない"),
	},
	{
		IDM_KEY_MINIMIZE,
		_T("minimize"),
		_T("最小化"),
	},
	{
		IDM_KEY_MAXIMIZE,
		_T("maximize"),
		_T("最大化"),
	},
	{
		IDM_KEY_RESTORE,
		_T("restore"),
		_T("大きさを元に戻す"),
	},
	{
		IDM_KEY_MOVE,
		_T("move"),
		_T("移動"),
	},
	{
		IDM_KEY_SIZE,
		_T("size"),
		_T("サイズ変更"),
	},
	{
		IDM_KEY_CENTER,
		_T("center"),
		_T("画面中央に移動"),
	},
	{
		IDM_KEY_TOPMOST,
		_T("topmost"),
		_T("最前面表示する/しない"),
	},
	{
		IDM_KEY_BACKGROUND,
		_T("background"),
		_T("最背面表示する"),
	},
	{
		IDM_KEY_CLOSE,
		_T("close"),
		_T("閉じる"),
	},
	{
		IDM_KEY_TERMINATE,
		_T("terminate"),
		_T("プロセスを強制終了"),
	},
	{
		IDM_KEY_TERMINATETREE,
		_T("terminatetree"),
		_T("プロセスツリーを強制終了"),
	},
	{
		IDM_KEY_BS_STRING,
		_T("bsstring"),
		_T("インクリメンタルサーチの文字列を1つ削る"),
	},
	{
		IDM_KEY_IDLE_PRIORITY,
		_T("idlepriority"),
		_T("優先度をアイドルに設定"),
	},
	{
		IDM_KEY_BELOW_NORMAL_PRIORITY,
		_T("belownormalpriority"),
		_T("優先度を通常以下に設定"),
	},
	{
		IDM_KEY_NORMAL_PRIORITY,
		_T("normalpriority"),
		_T("優先度を通常に設定"),
	},
	{
		IDM_KEY_ABOVE_NORMAL_PRIORITY,
		_T("abovenormalpriority"),
		_T("優先度を通常以上に設定"),
	},
	{
		IDM_KEY_HIGH_PRIORITY,
		_T("highpriority"),
		_T("優先度を高に設定"),
	},
	{
		IDM_KEY_REALTIME_PRIORITY,
		_T("realtimepriority"),
		_T("優先度をリアルタイムに設定"),
	},
	{
		IDM_KEY_CHILDPROCESS,
		_T("childprocess"),
		_T("子プロセスを表示"),
	},
	{
		IDM_KEY_THUMBNAIL,
		_T("thumbnail"),
		_T("サムネイルを表示"),
	},
	{
		IDM_KEY_THUMBNAIL_ORIGINAL,
		_T("thumbnailoriginal"),
		_T("元の大きさでサムネイルを表示"),
	},
	{
		IDM_KEY_OPACITY0,
		_T("opacity0%"),
		_T("不透明度0%"),
	},
	{
		IDM_KEY_OPACITY10,
		_T("opacity10%"),
		_T("不透明度10%"),
	},
	{
		IDM_KEY_OPACITY20,
		_T("opacity20%"),
		_T("不透明度20%"),
	},
	{
		IDM_KEY_OPACITY30,
		_T("opacity30%"),
		_T("不透明度30%"),
	},
	{
		IDM_KEY_OPACITY40,
		_T("opacity40%"),
		_T("不透明度40%"),
	},
	{
		IDM_KEY_OPACITY50,
		_T("opacity50%"),
		_T("不透明度50%"),
	},
	{
		IDM_KEY_OPACITY60,
		_T("opacity60%"),
		_T("不透明度60%"),
	},
	{
		IDM_KEY_OPACITY70,
		_T("opacity70%"),
		_T("不透明度70%"),
	},
	{
		IDM_KEY_OPACITY80,
		_T("opacity80%"),
		_T("不透明度80%"),
	},
	{
		IDM_KEY_OPACITY90,
		_T("opacity90%"),
		_T("不透明度90%"),
	},
	{
		IDM_KEY_OPACITY100,
		_T("opacity100%"),
		_T("不透明度100%"),
	},
	{
		IDM_KEY_OPACITYTOGGLE,
		_T("opacitytoggle"),
		_T("不透明度トグル"),
	},
	{
		IDM_KEY_COPY_NAME,
		_T("copyname"),
		_T("ファイル名をコピー"),
	},
	{
		IDM_KEY_COPY_TITLE,
		_T("copytitle"),
		_T("ウインドウタイトルをコピー"),
	},
	{
		IDM_KEY_COPY_COMMANDLINE,
		_T("copycommandline"),
		_T("コマンドラインをコピー"),
	},
	{
		IDM_KEY_ITEMTYPE_WINDOW,
		_T("windowitem"),
		_T("リストにウインドウを表示/非表示"),
	},
	{
		IDM_KEY_ITEMTYPE_WEBBROWSERTAB,
		_T("webbrowsertabitem"),
		_T("リストにウェブブラウザタブを表示/非表示"),
	},
	{
		IDM_KEY_INFO,
		_T("info"),
		_T("情報を表示"),
	},
	{
		IDM_KEY_DESKTOP,
		_T("desktop"),
		_T("デスクトップを表示"),
	},
	{
		IDM_KEY_CENTERCURSOR,
		_T("centercursor"),
		_T("切り替えてカーソルをウインドウ中央へ移動"),
	},
	{
		IDM_KEY_MOVETOACTIVEMONITOR,
		_T("movetoactivemonitor"),
		_T("アクティブなモニタへウインドウを移動"),
	},
	{
		IDM_KEY_MOVETOMONITOR,
		_T("movetomonitor"),
		_T("指定モニタへウインドウを移動"),
	},
	{
		IDM_KEY_MENU,
		_T("menu"),
		_T("メインメニュー"),
	},
	{
		IDM_KEY_COPYINFOMENU,
		_T("copyinfomenu"),
		_T("copyinfoメニュー"),
	},
	{
		IDM_KEY_SELECTMENU,
		_T("selectmenu"),
		_T("selectメニュー"),
	},
	{
		IDM_KEY_INCREMENTALSEARCHMENU,
		_T("incrementalsearchmenu"),
		_T("incrementalsearchメニュー"),
	},
	{
		IDM_KEY_WINDOWMENU,
		_T("windowmenu"),
		_T("windowメニュー"),
	},
	{
		IDM_KEY_PROCESSMENU,
		_T("processmenu"),
		_T("processメニュー"),
	},
	{
		IDM_KEY_OPACITYMENU,
		_T("opacitymenu"),
		_T("opacityメニュー"),
	},
	{
		IDM_KEY_ITEMTYPEMENU,
		_T("itemtypemenu"),
		_T("itemtypeメニュー"),
	},
	{
		IDM_KEY_COMMANDMENU,
		_T("commandmenu"),
		_T("commandメニュー"),
	},
	{
		IDM_KEY_COMMAND1,
		_T("command1"),
		_T("コマンド1"),
	},
	{
		IDM_KEY_COMMAND2,
		_T("command2"),
		_T("コマンド2"),
	},
	{
		IDM_KEY_COMMAND3,
		_T("command3"),
		_T("コマンド3"),
	},
	{
		IDM_KEY_COMMAND4,
		_T("command4"),
		_T("コマンド4"),
	},
	{
		IDM_KEY_COMMAND5,
		_T("command5"),
		_T("コマンド5"),
	},
	{
		IDM_KEY_COMMAND6,
		_T("command6"),
		_T("コマンド6"),
	},
	{
		IDM_KEY_COMMAND7,
		_T("command7"),
		_T("コマンド7"),
	},
	{
		IDM_KEY_COMMAND8,
		_T("command8"),
		_T("コマンド8"),
	},
	{
		IDM_KEY_COMMAND9,
		_T("command9"),
		_T("コマンド9"),
	},
	{
		IDM_KEY_COMMAND10,
		_T("command10"),
		_T("コマンド10"),
	},
	{
		IDM_KEY_COMMAND11,
		_T("command11"),
		_T("コマンド11"),
	},
	{
		IDM_KEY_COMMAND12,
		_T("command12"),
		_T("コマンド12"),
	},
	{
		IDM_KEY_COMMAND13,
		_T("command13"),
		_T("コマンド13"),
	},
	{
		IDM_KEY_COMMAND14,
		_T("command14"),
		_T("コマンド14"),
	},
	{
		IDM_KEY_COMMAND15,
		_T("command15"),
		_T("コマンド15"),
	},
	{
		IDM_KEY_COMMAND16,
		_T("command16"),
		_T("コマンド16"),
	},
	{
		IDM_KEY_COMMAND17,
		_T("command17"),
		_T("コマンド17"),
	},
	{
		IDM_KEY_COMMAND18,
		_T("command18"),
		_T("コマンド18"),
	},
	{
		IDM_KEY_COMMAND19,
		_T("command19"),
		_T("コマンド19"),
	},
	{
		IDM_KEY_COMMAND20,
		_T("command20"),
		_T("コマンド20"),
	},
	{
		IDM_KEY_COMMAND21,
		_T("command21"),
		_T("コマンド21"),
	},
	{
		IDM_KEY_COMMAND22,
		_T("command22"),
		_T("コマンド22"),
	},
	{
		IDM_KEY_COMMAND23,
		_T("command23"),
		_T("コマンド23"),
	},
	{
		IDM_KEY_COMMAND24,
		_T("command24"),
		_T("コマンド24"),
	},
	{
		IDM_KEY_COMMAND25,
		_T("command25"),
		_T("コマンド25"),
	},
	{
		IDM_KEY_COMMAND26,
		_T("command26"),
		_T("コマンド26"),
	},
	{
		IDM_KEY_COMMAND27,
		_T("command27"),
		_T("コマンド27"),
	},
	{
		IDM_KEY_COMMAND28,
		_T("command28"),
		_T("コマンド28"),
	},
	{
		IDM_KEY_COMMAND29,
		_T("command29"),
		_T("コマンド29"),
	},
	{
		IDM_KEY_COMMAND30,
		_T("command30"),
		_T("コマンド30"),
	},
	{
		IDM_KEY_COMMAND31,
		_T("command31"),
		_T("コマンド31"),
	},
	{
		IDM_KEY_COMMAND32,
		_T("command32"),
		_T("コマンド32"),
	},
	{
		IDM_KEY_COMMAND33,
		_T("command33"),
		_T("コマンド33"),
	},
	{
		IDM_KEY_COMMAND34,
		_T("command34"),
		_T("コマンド34"),
	},
	{
		IDM_KEY_COMMAND35,
		_T("command35"),
		_T("コマンド35"),
	},
	{
		IDM_KEY_COMMAND36,
		_T("command36"),
		_T("コマンド36"),
	},
	{
		IDM_KEY_COMMAND37,
		_T("command37"),
		_T("コマンド37"),
	},
	{
		IDM_KEY_COMMAND38,
		_T("command38"),
		_T("コマンド38"),
	},
	{
		IDM_KEY_COMMAND39,
		_T("command39"),
		_T("コマンド39"),
	},
	{
		IDM_KEY_COMMAND40,
		_T("command40"),
		_T("コマンド40"),
	},
	{
		IDM_KEY_COMMAND41,
		_T("command41"),
		_T("コマンド41"),
	},
	{
		IDM_KEY_COMMAND42,
		_T("command42"),
		_T("コマンド42"),
	},
	{
		IDM_KEY_COMMAND43,
		_T("command43"),
		_T("コマンド43"),
	},
	{
		IDM_KEY_COMMAND44,
		_T("command44"),
		_T("コマンド44"),
	},
	{
		IDM_KEY_COMMAND45,
		_T("command45"),
		_T("コマンド45"),
	},
	{
		IDM_KEY_COMMAND46,
		_T("command46"),
		_T("コマンド46"),
	},
	{
		IDM_KEY_COMMAND47,
		_T("command47"),
		_T("コマンド47"),
	},
	{
		IDM_KEY_COMMAND48,
		_T("command48"),
		_T("コマンド48"),
	},
	{
		IDM_KEY_COMMAND49,
		_T("command49"),
		_T("コマンド49"),
	},
	{
		IDM_KEY_COMMAND50,
		_T("command50"),
		_T("コマンド50"),
	},
};


struct LISTVIEW{
	//動作
	//ダブルクリックを使用しない[動作]
	bool bDisableDoubleClick;

	//マウスアウトで確定[動作]
	bool bMouseOut;

	//ホットキーで確定[動作]
	bool bHotKey;

	//マウスホバーで選択[動作]
	bool bMouseHover;

	//タイムアウトで確定[動作]
	int iTimeOut;

	//ドラッグ中であればマウスホバーで選択
	bool bDragMouseHover;

	//ドラッグ中であればタイムアウトで確定
	bool bDragTimeOut;

	//タスクバー相当の表示[動作]
//	bool bTaskBarEquivalent;

	//2番目にアクティブなウインドウを選択[動作]
	bool bSelectSecondWindow;

	//サムネイルを表示(及び平行方向揃え位置)
	UINT uThumbnail;

	//サムネイルの垂直方向揃え位置
	UINT uThumbnailVerticalAlign;

	//サムネイルの大きさ
	int iThumbnailSize;

	//サムネイルを表示するまでの時間
	int iThumbnailDelay;

	//サムネイルを元の大きさで表示
	bool bThumbnailOriginalSize;


	//不透明度
	BYTE byOpacity;

	//表示項目
	//アイコン[表示項目]
	int iIcon;

	//ウインドウを表示する[表示項目]
	bool bWindowItem;

	//ウェブブラウザタブを表示する[表示項目]
	bool bWebBrowserTabItem;

	//「デスクトップ」を表示する[表示項目]
	bool bDesktopItem;

	//「キャンセル」を表示する[表示項目]
	bool bCancelItem;

	//リストビューカラムの並び順
	int iColumnOrder[LISTITEM_NUM];

	//アイコンカラムの幅(内部使用のため、IconSize+iIconMargin*2)
	int iIconWidth;

	//ファイル名カラムの幅
	int iFileNameWidth;

	//ウインドウタイトルカラムの幅
	int iWindowTitleWidth;

	//アイコンの余白
	int iIconMargin;

	//UWPアプリの背景色を描画しない
	bool bUWPAppNoBackgroundColor;

	//ウインドウからアイコンを取得
	bool bIconFromWindow;

	//フレームを表示
	bool bDialogFrame;

	//フレーム色
	COLORREF clrDialogFrame;

	//ダブルクリックとして判定する時間
	int iDoubleClickTime;

	//クリック+マウスホイール上下として判定する時間
	int iMouseWheelClickTime;

	//カーソルを非表示にするまでの時間
	int iHideCursorTime;

	//カーソル位置にメニューを表示(マウス操作時のみ)
	bool bPopupMenuCursorPos;
};

struct SHORTCUTKEY{
	//アクセラレータテーブル
	ACCEL aKeyTable[MAX_SHORTCUTKEY];
	SKEY sKeyTable[MAX_SHORTCUTKEY];
	UINT iSize;
};

struct MOUSE{
	//マウス
	SMOUSE sMouseTable[SMOUSE_NUM];
};

struct SEARCHWINDOWDESIGN{
	//フォント名
	TCHAR szFont[128];
	//サイズ
	int iFontSize;
	//スタイル
	int iFontStyle;

	//文字色
	COLORREF clrText;
	//背景色
	COLORREF clrTextBk;
	//背景色(グラデーション)
	COLORREF clrTextBkGradientEnd;

	//検索文字列
	TCHAR szTextFormat[32];
};

struct ITEMDESIGN{
	//フォント名
	TCHAR szFont[128];
	//サイズ
	int iFontSize;
	//スタイル
	int iFontStyle;

	//文字色
	COLORREF clrText;
	//背景色
	COLORREF clrTextBk;
	//背景色(グラデーション)
	COLORREF clrTextBkGradientEnd;
};

struct DIRECTWRITE{
	bool bDirectWrite;
	UINT uRenderingMode;
	UINT uGamma;//1-3 *1000
	UINT uEnhancedContrast;//1-2 *100
	UINT uClearTypeLevel;//The degree of ClearType level, from 0.0f (no ClearType) to 1.0f (full ClearType). *100
};

struct CURSORCORNER{
	bool bValid;
	SCC_CORNERS eDestCorner;
};

struct SHOWWINDOW{
	//[SCC_NONE]はダミー
	//[マウスの移動]
	CURSORCORNER ccCursorCorners[9];
	//マウスホイール回転時のみ[マウスの移動]
	bool bMouseWheel;

	//選択項目にカーソルを移動[マウスの移動]
	bool bMouseMoveCursorSelectedWindow;

	//カーソルを移動[ホットキー]
	SCC_CORNERS eHotKeyCursorCorner;

	//ホットキー
	WORD wHotKey;

	//選択項目にカーソルを移動[ホットキー]
	bool bHotKeyMoveCursorSelectedWindow;

	//「選択項目にカーソルを移動」が有効なら表示時に移動
	bool bMoveCursorAtStart;

	//カーソル移動の判定時間
	int iCursorCornerDelay;

	//モニタごとに画面端を判定
	bool bMouseEachMonitor;

	//カーソル四隅移動時のマージンX座標[マウスの移動]
	int iMouseCursorMarginHorizontal;

	//カーソル四隅移動時のマージンY座標[マウスの移動]
	int iMouseCursorMarginVertical;

	//カーソル四隅移動時のマージンX座標[ホットキー]
	int iHotKeyCursorMarginHorizontal;

	//カーソル四隅移動時のマージンY座標[ホットキー]
	int iHotKeyCursorMarginVertical;

	//カーソルを閉じ込める[マウスの移動]
	int iMouseClipCursorMode;

	//カーソルを閉じ込める時間[マウスの移動]
	int iMouseClipCursorTime;

	//カーソルを閉じ込める[ホットキー]
	int iHotKeyClipCursorMode;

	//カーソルを閉じ込める時間[ホットキー]
	int iHotKeyClipCursorTime;

	//画面端と判断する端からの距離
	int iLeftCornerDistance;
	int iRightCornerDistance;
	int iTopCornerDistance;
	int iBottomCornerDistance;

	//フルスクリーン時は無効[マウスの移動]
	bool bMouseDisableInFullScreenMode;
	//フルスクリーン時は無効[ホットキー]
	bool bHotKeyDisableInFullScreenMode;
};

struct INCREMENTALSEARCH{
	//検索方法[インクリメンタルサーチ]
	int iMatchMode;

	//Migemoモード[インクリメンタルサーチ]
	int iMigemoMode;

	//左側の項目のみ検索[インクリメンタルサーチ]
	bool bFirstColumnOnly;

	//候補のウインドウが1つなら確定[インクリメンタルサーチ]
	bool bEnterUniqueWindow;

	//候補が1つになった時切り替えるまでの時間
	int iUniqueWindowDelay;

	//連文節検索を行う(Migemo)[インクリメンタルサーチ]
	bool bMigemoCaseSensitive;

	//Migemo検索を開始するまでの時間
	int iMigemoDelay;

	//Migemoを読み込んだままにする
	bool bNoUnloadMigemo;

	//ツールチップデザイン
	struct SEARCHWINDOWDESIGN DefaultSearchWindowDesign;

	//Migemo検索一致
	struct SEARCHWINDOWDESIGN MigemoMatchSearchWindowDesign;

	//Migemo検索不一致
	struct SEARCHWINDOWDESIGN MigemoNoMatchSearchWindowDesign;

	//文字列表示位置マージン
	int iMargin;

	//不透明度
	BYTE byOpacity;
};

struct BACKGROUND{
	//背景画像のパス
	TCHAR szImagePath[MAX_PATH];
	//背景画像のオフセット
	int iXOffset;
	int iYOffset;
	//大きさ
	BYTE byResizePercent;
	//不透明度
	BYTE byOpacity;
};

struct EXCLUDE{
	//除外するファイル名たち(;区切り)
	TCHAR szFileName[MAX_PATH];
};

struct MULTIMONITOR{
	//ウインドウリストを表示するモニタ
	int iActiveMonitorMode;

	//ウインドウリストを表示するモニタ名
	TCHAR szMonitorName[CCHDEVICENAME];

	//アクティブなモニタのウインドウのみ表示
	bool bOnlyActiveMonitor;
};

struct VIRTUALDESKTOP{
	//アクティブな仮想デスクトップのウインドウのみ表示
	bool bOnlyActiveVirtualDesktop;
};

//ShellExecuteのlpVerb
enum{
	//実行/開く(open)
	CMDMODE_OPEN=0,
	//管理者として実行(runas)
	CMDMODE_ADMIN,
	//エクスプローラで開く(explorer)
//	CMDMODE_EXPLORER,
};

typedef struct{
	UINT uCommandMode;
	LPCTSTR szVerb;
	LPCTSTR szDescript;
}COMMANDMODE_TABLE;

static COMMANDMODE_TABLE CommandMode_Table[]={
	{
		CMDMODE_OPEN,
		_T("open"),
		_T("実行/開く"),
	},
	{
		CMDMODE_ADMIN,
		_T("runas"),
		_T("管理者として実行"),
	},
/*
	{
		CMDMODE_EXPLORER,
		_T("explorer"),
		_T("エクスプローラで開く")
	},
*/
};

enum{
	//何もせずに実行
	CMDOPT_NONE=0,
	//切り替えてから実行
	CMDOPT_SWITCH,
	//切り替えをキャンセルしてから実行
	CMDOPT_CANCEL,
};

struct COMMAND{
	//コマンドの名前
	TCHAR szCommandName[256];
	//コマンド種別
	UINT uCommandMode;
	//コマンド実行前の動作
	UINT uCommandOption;
	//実行ファイル
	TCHAR szFilePath[MAX_PATH];
	//引数
	TCHAR szParameters[MAX_PATH];
	//作業ディレクトリ
	TCHAR szWorkingDirectory[MAX_PATH];
	//表示方法
	UINT uCmdShow;
};

struct WEBBROWSER{
	//対象ファイル名
	TCHAR szFilePath[MAX_PATH];
	//ポート番号(省略する場合は-1)
	int iPort;
};

struct CONFIG{
	//リストビュー全般
	struct LISTVIEW ListView;

	//表示操作
	struct SHOWWINDOW ShowWindow;

	//ショートカットキー
	struct SHORTCUTKEY ShortcutKey;

	//マウス
	struct MOUSE Mouse;

	//リストビューアイテムのデザイン
	struct ITEMDESIGN DefaultItemDesign;
	struct ITEMDESIGN SelectedItemDesign;

	//DirectWrite
	struct DIRECTWRITE DirectWrite;

	//インクリメンタルサーチ
	struct INCREMENTALSEARCH IncrementalSearch;

	//背景画像
	struct BACKGROUND Background;

	//除外アイテム
	struct EXCLUDE Exclude;

	//マルチモニタ設定
	struct MULTIMONITOR MultiMonitor;

	//仮想デスクトップ
	struct VIRTUALDESKTOP VirtualDesktop;

	//自動起動する
	bool bAutoStart;

	//管理者権限で起動する
	bool bRunAsAdministrator;

	//独自コマンド(1～50)
	struct COMMAND Command[MAX_COMMAND+1];

	//ウェブブラウザ
	struct WEBBROWSER WebBrowser[MAX_WEBBROWSER+1];


	HMONITOR GetActiveMonitor(){
		HMONITOR hMonitor=NULL;

		switch(MultiMonitor.iActiveMonitorMode){
			case ACTIVEMONITOR_BY_WINDOW:
				hMonitor=MonitorFromWindow(GetForegroundWindow(),MONITOR_DEFAULTTONEAREST);
				break;
			case ACTIVEMONITOR_BY_NAME:
				hMonitor=MonitorFromName(MultiMonitor.szMonitorName);
				if(hMonitor!=NULL)break;
				//fall through
			case ACTIVEMONITOR_BY_CURSOR:{
				POINT pt={};

				GetCursorPos(&pt);
				hMonitor=MonitorFromPoint(pt,MONITOR_DEFAULTTONEAREST);
				break;
			}
		}
		return hMonitor;
	}

	UINT GetListIconSize(){
		switch(ListView.iIcon){
			case LISTICON_NO_SMALL:
			case LISTICON_NO:
				return 0;
			case LISTICON_SMALL:
				return 16;
			case LISTICON_BIG:
			default:
				return 32;
			case LISTICON_BIG48:
				return 48;
		}
	}

	UINT GetListIconHeight(){
		switch(ListView.iIcon){
			case LISTICON_NO_SMALL:
				return 16;
			case LISTICON_NO:
				return 32;
			case LISTICON_SMALL:
				return 16;
			case LISTICON_BIG:
			default:
				return 32;
			case LISTICON_BIG48:
				return 48;
		}
	}

};

extern struct CONFIG g_Config;

#endif //TASCHER_COMMONSETTINGS_H
