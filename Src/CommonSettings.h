//CommonSettings.h
//設定関係共通ヘッダファイル

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.60
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/


#ifndef COMMONSETTINGS_H
#define COMMONSETTINGS_H

#include"ListView.h"
#include"resources/resource.h"

//ウインドウ幅
#define DEFAULT_WINDOW_WIDTH 900
//ウインドウ高さ
#define DEFAULT_WINDOW_HEIGHT 450

#define DEFAULT_FILENAME_WIDTH 100
#define DEFAULT_WINDOWTITLE_WIDTH 400

//タイムアウト
#define DEFAULT_SELECT_TIMEOUT 1000

//フォント最小サイズ
//0=フォントサイズ変更なし
#define MINIMUM_FONTSIZE 0
//フォント最大サイズ
#define MAXIMUM_FONTSIZE 30

//ショートカットキー数
#define MAX_SHORTCUTKEY 64


enum{
	LISTITEM_FILENAME,//ファイル名
	LISTITEM_WINDOWTITLE,//ウインドウタイトル
	LISTITEM_NUM//要素数
};

enum{
	LISTICON_NO=0,//アイコンなし(大)
	LISTICON_NO_SMALL,//アイコンなし(小)
	LISTICON_SMALL,//小さなアイコン
	LISTICON_BIG,//大きなアイコン
};

enum{
	MATCH_FORWARD=0,//前方一致
	MATCH_PARTICAL,//部分一致
	MATCH_FLEX,//あいまい
};

enum{
	MIGEMO_NO=0,//使用しない
	MIGEMO_DEFAULT,//デフォルトで有効
	MIGEMO_NODEFAULT,//大文字入力で有効
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
	SMOUSE_NUM,
};

typedef struct{
	UINT uMouseType;
	UINT uCmdId;
}SMOUSE;

typedef struct{
	//関連するWM_*
	UINT uMessageType;
	BYTE fVirt;
	LPCTSTR szDescript;
}SMOUSETYPE_TABLE;

static SMOUSETYPE_TABLE MouseType_Table[]={
	{
		WM_LBUTTONDOWN,
		0,
		_T("左クリック"),
	},
	{
		WM_LBUTTONDOWN,
		0,
		_T("左ダブルクリック"),
	},
	{
		WM_RBUTTONDOWN,
		0,
		_T("右クリック"),
	},
	{
		WM_RBUTTONDOWN,
		0,
		_T("右ダブルクリック"),
	},
	{
		WM_LBUTTONDOWN,
		FSHIFT,
		_T("Shift + 左クリック"),
	},
	{
		WM_LBUTTONDOWN,
		FSHIFT,
		_T("Shift + 左ダブルクリック"),
	},
	{
		WM_RBUTTONDOWN,
		FSHIFT,
		_T("Shift + 右クリック"),
	},
	{
		WM_RBUTTONDOWN,
		FSHIFT,
		_T("Shift + 右ダブルクリック"),
	},
	{
		WM_LBUTTONDOWN,
		FCONTROL,
		_T("Ctrl + 左クリック"),
	},
	{
		WM_LBUTTONDOWN,
		FCONTROL,
		_T("Ctrl + 左ダブルクリック"),
	},
	{
		WM_RBUTTONDOWN,
		FCONTROL,
		_T("Ctrl + 右クリック"),
	},
	{
		WM_RBUTTONDOWN,
		FCONTROL,
		_T("Ctrl + 右ダブルクリック"),
	},
	{
		WM_LBUTTONDOWN,
		FSHIFT|FCONTROL,
		_T("Shift + Ctrl + 左クリック"),
	},
	{
		WM_LBUTTONDOWN,
		FSHIFT|FCONTROL,
		_T("Shift + Ctrl + 左ダブルクリック"),
	},
	{
		WM_LBUTTONDOWN,
		FSHIFT|FCONTROL,
		_T("Shift + Ctrl + 右クリック"),
	},
	{
		WM_RBUTTONDOWN,
		FSHIFT|FCONTROL,
		_T("Shift + Ctrl + 右ダブルクリック"),
	},
	{
		WM_MOUSEWHEEL,
		0,
		_T("マウスホイール上"),
	},
	{
		WM_MOUSEWHEEL,
		0,
		_T("マウスホイール下"),
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
		IDM_KEY_TOPMOST,
		_T("topmost"),
		_T("最前面表示する/しない"),
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
		IDM_KEY_AND,
		_T("and"),
		_T("AND検索(検索中以外は切り替え)"),
	},
	{
		IDM_KEY_BS_STRING,
		_T("bsstring"),
		_T("インクリメンタルサーチの文字列を一つ削る"),
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
		IDM_KEY_MENU,
		_T("menu"),
		_T("コマンドメニュー"),
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

	//タスクバー相当の表示[動作]
	bool bTaskBarEquivalent;

	//2番目にアクティブなウインドウを選択[動作]
	bool bSelectSecondWindow;

	//不透明度
	BYTE byAlpha;

	//表示項目
	//アイコン[表示項目]
	int iIcon;

	//「デスクトップ」を表示する[表示項目]
	bool bDesktopItem;

	//「キャンセル」を表示する[表示項目]
	bool bCancelItem;

	//リストビューカラムの並び順
	int iColumnOrder[LISTITEM_NUM];

	//ファイル名カラムの幅
	int iFileNameWidth;

	//ウインドウタイトルカラムの幅
	int iWindowTitleWidth;
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
};

struct SHOWWINDOW{
	//四隅に移動
	//左上[マウスの移動]
	bool bLeftTop;
	//右上[マウスの移動]
	bool bRightTop;
	//左下[マウスの移動]
	bool bLeftBottom;
	//右下[マウスの移動]
	bool bRightBottom;

	//ホットキー
	WORD wHotKey;
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
	BYTE byAlpha;
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

	//インクリメンタルサーチ
	struct INCREMENTALSEARCH IncrementalSearch;

	//背景画像
	struct BACKGROUND Background;

	//スタートアップに追加する(自動起動)
	bool bAutoStart;
};

extern struct CONFIG g_Config;

#endif //COMMONSETTINGS_H
