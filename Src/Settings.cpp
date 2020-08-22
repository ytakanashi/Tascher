//Settings.cpp
//設定ダイアログ

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.63
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/


#include"StdAfx.h"
#include"Settings.h"
#include"Function.h"
#include"ListView.h"
#include"Path.h"
#include"PrivateProfile.h"
#include"resources/resource.h"
#include<commdlg.h>
#include<richedit.h>
#include<objidl.h>
#include<shlobj.h>



//最初に表示するタブ
int g_iStartTab;

//ショートカットキーホットコントロールのウインドウハンドル
HWND g_hHotkey;

//表示用ホットコントロールのウインドウハンドル
HWND g_hDisplayHotkey;

//設定一時保存用
struct CONFIG g_ConfigTmp;

//設定項目デフォルト文字列
static TCHAR g_szDefaultString[]=_T(" デフォルト");

//コマンド追加/削除通知(リストビュー/コンボボックス更新)
#define WM_UPDATECOMMAND WM_APP+0x100
#define WM_SENDUPDATECOMMAND WM_APP+0x101
//マウスの移動ボタン用メニュー作成
#define WM_CREATEMENU WM_APP+0x102



//チェックボックスのチェックをつける/外す
LRESULT setCheck(HWND hDlg,UINT uId,int iCheck){
	return SendDlgItemMessage(hDlg,
							  uId,
							  BM_SETCHECK,
							  (WPARAM)(iCheck)?BST_CHECKED:BST_UNCHECKED,
							  (LPARAM)0
							  );
}

//チェックボックスがチェックされているか取得
bool getCheck(HWND hDlg,UINT uId){
	return SendDlgItemMessage(hDlg,uId,BM_GETCHECK,(WPARAM)0,(LPARAM)0)!=0;
}

//フォントを列挙する
int CALLBACK EnumFontsProc(ENUMLOGFONTEX *lplf,NEWTEXTMETRICEX *lptm,DWORD dwType,LPARAM lpData){
	//TruType以外は捨てる
	if(dwType!=TRUETYPE_FONTTYPE)return true;
	//縦書きは捨てる
	if(lplf->elfLogFont.lfFaceName[0]==_T('@')||lplf->elfLogFont.lfFaceName[0]==_T('$'))return true;
	//日本語でも欧文でもない
	if(lstrcmp(lplf->elfScript,_T("日本語"))&&lstrcmp(lplf->elfScript,_T("欧文")))return true;
	//追加されていない場合CB_ERRを返す(重複を防ぐため)
	if(SendMessage((HWND)lpData,CB_FINDSTRINGEXACT,(WPARAM)0,(LPARAM)lplf->elfLogFont.lfFaceName)==CB_ERR){
		SendMessage((HWND)lpData,CB_ADDSTRING,(WPARAM)0,(LPARAM)lplf->elfLogFont.lfFaceName);
	}
	return true;
}

//ホットキーコントロールのプロシージャ
LRESULT CALLBACK HotKeyProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam){
	switch(uMsg){
		case WM_KEYDOWN:
			switch(wParam){
				case VK_RETURN:
				case VK_TAB:
				case VK_SPACE:
				case VK_DELETE:
				case VK_ESCAPE:
				case VK_CAPITAL:
				case VK_BACK:{
					//エンターキーやスペースキーを入力可能に
					BYTE byKey=0;

					if(GetKeyState(VK_SHIFT)&0x8000)byKey|=HOTKEYF_SHIFT;
					if(GetKeyState(VK_CONTROL)&0x8000)byKey|=HOTKEYF_CONTROL;
					if(GetKeyState(VK_MENU)&0x8000)byKey|=HOTKEYF_ALT;
					if(lParam&0x01000000)byKey|=HOTKEYF_EXT;

					SendMessage(hWnd,HKM_SETHOTKEY,MAKEWORD(wParam,byKey),0);
					return TRUE;
				}

			default:
				break;
		}
		break;

		default:
			break;
	}
	return CallWindowProc((WNDPROC)GetWindowLongPtr(hWnd,GWLP_USERDATA),hWnd,uMsg,wParam,lParam);
}

//表示用ホットキーコントロールのプロシージャ
LRESULT CALLBACK DisplayHotKeyProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam){
	switch(uMsg){
		case WM_SYSKEYDOWN:
			switch(wParam){
				case VK_MENU:case VK_LMENU:case VK_RMENU:
					return true;
			}
		case WM_KEYDOWN:
			switch(wParam){
				case VK_SHIFT:case VK_LSHIFT:case VK_RSHIFT:
				case VK_CONTROL:case VK_LCONTROL:case VK_RCONTROL:
				case VK_LWIN:case VK_RWIN:
					return true;
				default:{
					EnableWindow(GetDlgItem(GetParent(hWnd),IDC_STATIC_HOTKEY_SHIFT),GetKeyState(VK_SHIFT)&0x8000);
					EnableWindow(GetDlgItem(GetParent(hWnd),IDC_STATIC_HOTKEY_CTRL),GetKeyState(VK_CONTROL)&0x8000);
					EnableWindow(GetDlgItem(GetParent(hWnd),IDC_STATIC_HOTKEY_ALT),GetKeyState(VK_MENU)&0x8000);
					EnableWindow(GetDlgItem(GetParent(hWnd),IDC_STATIC_HOTKEY_WIN),GetKeyState(VK_LWIN)&0x8000||GetKeyState(VK_RWIN)&0x8000);

					BYTE byKey=(BYTE)wParam;

					SendMessage(hWnd,HKM_SETHOTKEY,
								byKey|((byKey==VK_DELETE||
										byKey==VK_UP||
										byKey==VK_DOWN||
										byKey==VK_LEFT||
										byKey==VK_RIGHT||
										byKey==VK_HOME||
										byKey==VK_END||
										byKey==VK_PRIOR||
										byKey==VK_NEXT||
										byKey==VK_INSERT||
										byKey==VK_SNAPSHOT)?(HOTKEYF_EXT<<8):0),0);

					//親に送る
					SendMessage(GetParent(hWnd),WM_COMMAND,MAKEWPARAM(IDC_HOTKEY_DISPLAY,0),0);
					return TRUE;
				}
		}
		break;

		default:
			break;
	}
	return CallWindowProc((WNDPROC)GetWindowLongPtr(hWnd,GWLP_USERDATA),hWnd,uMsg,wParam,lParam);
}

//低レベルキーボードフックのプロシージャ
LRESULT CALLBACK LowLevelKeyboardHookProc(int nCode,WPARAM wParam,LPARAM lParam){
	if(nCode!=HC_ACTION)return CallNextHookEx(NULL/*This parameter is ignored.(by MSDN)*/,nCode,wParam,lParam);

	LPKBDLLHOOKSTRUCT lpKBDllHook=(LPKBDLLHOOKSTRUCT)lParam;

	//ホットキーコントロールでTabキーの入力を可能に
	if(lpKBDllHook->vkCode==VK_TAB){
		if(GetFocus()==g_hHotkey){
			return CallWindowProc(HotKeyProc,
								  g_hHotkey,
								  (UINT)wParam,lpKBDllHook->vkCode,0);
		}else if(GetFocus()==g_hDisplayHotkey){
			return CallWindowProc(DisplayHotKeyProc,
								  g_hDisplayHotkey,
								  (UINT)wParam,lpKBDllHook->vkCode,0);
		}
	}
	return CallNextHookEx(NULL/*This parameter is ignored.(by MSDN)*/,nCode,wParam,lParam);
}

//現在の設定をコントロールに適用
void SetCurrentSettings(HWND hDlg,struct CONFIG* pConfig,TAB iTab){
	switch(iTab){
		case TAB_GENERAL:
		default:{
			//「全般」タブ
			//動作
			//ダブルクリックを使用しない[動作]
			setCheck(hDlg,
					 IDC_CHECKBOX_DISABLEDOUBLECLICK,
					 pConfig->ListView.bDisableDoubleClick
					 );

			//マウスアウトで確定[動作]
			setCheck(hDlg,
					 IDC_CHECKBOX_MOUSEOUT,
					 pConfig->ListView.bMouseOut
					 );

			//ホットキーで確定[動作]
			setCheck(hDlg,
					 IDC_CHECKBOX_HOTKEY,
					 pConfig->ListView.bHotKey
					 );

			//マウスホバーで選択[動作]
			setCheck(hDlg,
					 IDC_CHECKBOX_MOUSEHOVER,
					 pConfig->ListView.bMouseHover
					 );

			//タイムアウトで確定[動作]
			setCheck(hDlg,
					 IDC_CHECKBOX_TIMEOUT,
					 pConfig->ListView.iTimeOut
					 );

			//2番目にアクティブなウインドウを選択[動作]
			setCheck(hDlg,
					 IDC_CHECKBOX_SELECTSECONDWINDOW,
					 pConfig->ListView.bSelectSecondWindow
					 );

			//サムネイルを表示[動作]
			setCheck(hDlg,
					 IDC_CHECKBOX_THUMBNAIL,
					 pConfig->ListView.uThumbnail
					 );



			//起動
			//自動起動[起動]
			setCheck(hDlg,
					 IDC_CHECKBOX_AUTOSTART,
					 pConfig->bAutoStart
					 );

			//管理者権限で起動する[起動]
			setCheck(hDlg,
					 IDC_CHECKBOX_RUN_AS_ADMINISTRATOR,
					 pConfig->bRunAsAdministrator
					 );


			//不透明度
			TCHAR szPosition[12]={};

			//範囲指定
			SendMessage(GetDlgItem(hDlg,IDC_SLIDER_ALPHA),TBM_SETRANGE,(WPARAM)true,MAKELPARAM(0,100));
			//メモリの間隔は1
			SendMessage(GetDlgItem(hDlg,IDC_SLIDER_ALPHA),TBM_SETTICFREQ,(WPARAM)1,(LPARAM)0);
			SendMessage(GetDlgItem(hDlg,IDC_SLIDER_ALPHA),TBM_SETPOS,(WPARAM)true,(LPARAM)pConfig->ListView.byOpacity);
			//1Stepは1
			SendMessage(GetDlgItem(hDlg,IDC_SLIDER_ALPHA),TBM_SETPAGESIZE,(WPARAM)0,(LPARAM)1);
			//不透明度表示
			wsprintf(szPosition,_T("%d%%"),pConfig->ListView.byOpacity);
			SetWindowText(GetDlgItem(hDlg,IDC_STATIC_ALPHA),(LPCTSTR)szPosition);


			//表示項目
			//アイコン
			SendDlgItemMessage(hDlg,
							   IDC_COMBO_ICON,
							   CB_SETCURSEL,
							   (WPARAM)pConfig->ListView.iIcon,
							   (LPARAM)0
							   );

			//ファイル名[表示項目]
			InitializeSpinEditControl(GetDlgItem(hDlg,IDC_SPIN_FILENAME),
									  GetDlgItem(hDlg,IDC_EDIT_FILENAME),
									  0,
									  GetSystemMetrics(SM_CXSCREEN),
									  pConfig->ListView.iFileNameWidth
									 );
			SetIntToEdit(GetDlgItem(hDlg,IDC_EDIT_FILENAME),pConfig->ListView.iFileNameWidth);

			//ウインドウタイトル[表示項目]
			InitializeSpinEditControl(GetDlgItem(hDlg,IDC_SPIN_WINDOWTITLE),
									  GetDlgItem(hDlg,IDC_EDIT_WINDOWTITLE),
									  0,
									  GetSystemMetrics(SM_CXSCREEN),
									  pConfig->ListView.iWindowTitleWidth
									 );
			SetIntToEdit(GetDlgItem(hDlg,IDC_EDIT_WINDOWTITLE),pConfig->ListView.iWindowTitleWidth);

			//デスクトップ[表示項目]
			setCheck(hDlg,
					 IDC_CHECKBOX_DESKTOPITEM,
					 pConfig->ListView.bDesktopItem
					 );

			//キャンセル[表示項目]
			setCheck(hDlg,
					 IDC_CHECKBOX_CANCELITEM,
					 pConfig->ListView.bCancelItem
					 );

			//インクリメンタルサーチ
			//検索方法[インクリメンタルサーチ]
			SendDlgItemMessage(hDlg,
							   IDC_COMBO_SEARCH,
							   CB_SETCURSEL,
							   (WPARAM)(pConfig->IncrementalSearch.iMatchMode),
							   (LPARAM)0
							   );

			//Migemoモード[インクリメンタルサーチ]
			SendDlgItemMessage(hDlg,
							   IDC_COMBO_MIGEMO,
							   CB_SETCURSEL,
							   (WPARAM)(pConfig->IncrementalSearch.iMigemoMode),
							   (LPARAM)0
							   );

			//左側の項目のみ検索[インクリメンタルサーチ]
			setCheck(hDlg,
					 IDC_CHECKBOX_FIRSTCOLUMNONLY,
					 pConfig->IncrementalSearch.bFirstColumnOnly
					 );

			//候補のウインドウが1つなら確定[インクリメンタルサーチ]
			setCheck(hDlg,
					 IDC_CHECKBOX_ENTERUNIQUEWINDOW,
					 pConfig->IncrementalSearch.bEnterUniqueWindow
					 );

			//連文節検索を行う[インクリメンタルサーチ]
			setCheck(hDlg,
					 IDC_CHECKBOX_MIGEMO_CASESENSITIVE,
					 pConfig->IncrementalSearch.bMigemoCaseSensitive
					 );
			break;
		}//case TAB_GENERAL


		case TAB_SHOWWINDOW:{
			//「表示方法」タブ
			//マウスの移動
			//左上[マウスの移動]
			setCheck(hDlg,
					 IDC_CHECKBOX_LEFTTOP,
					 pConfig->ShowWindow.ccCursorCorners[SCC_LEFTTOP].bValid
					 );

			//上[マウスの移動]
			setCheck(hDlg,
					 IDC_CHECKBOX_TOP,
					 pConfig->ShowWindow.ccCursorCorners[SCC_TOP].bValid
					 );

			//右上[マウスの移動]
			setCheck(hDlg,
					 IDC_CHECKBOX_RIGHTTOP,
					 pConfig->ShowWindow.ccCursorCorners[SCC_RIGHTTOP].bValid
					 );

			//左[マウスの移動]
			setCheck(hDlg,
					 IDC_CHECKBOX_LEFT,
					 pConfig->ShowWindow.ccCursorCorners[SCC_LEFT].bValid
					 );

			//右上[マウスの移動]
			setCheck(hDlg,
					 IDC_CHECKBOX_RIGHT,
					 pConfig->ShowWindow.ccCursorCorners[SCC_RIGHT].bValid
					 );

			//左下[マウスの移動]
			setCheck(hDlg,
					 IDC_CHECKBOX_LEFTBOTTOM,
					 pConfig->ShowWindow.ccCursorCorners[SCC_LEFTBOTTOM].bValid
					 );

			//下[マウスの移動]
			setCheck(hDlg,
					 IDC_CHECKBOX_BOTTOM,
					 pConfig->ShowWindow.ccCursorCorners[SCC_BOTTOM].bValid
					 );

			//右下[マウスの移動]
			setCheck(hDlg,
					 IDC_CHECKBOX_RIGHTBOTTOM,
					 pConfig->ShowWindow.ccCursorCorners[SCC_RIGHTBOTTOM].bValid
					 );

			//モニタごとに画面端を判定
			setCheck(hDlg,
					 IDC_CHECKBOX_MOUSE_EACH_MONITOR,
					 pConfig->ShowWindow.bMouseEachMonitor
					 );

			//マウスホイール回転時のみ[マウスの移動]
			setCheck(hDlg,
					 IDC_CHECKBOX_SHOW_MOUSEWHEEL,
					 pConfig->ShowWindow.bMouseWheel
					 );

			//カーソル移動の判定時間[マウスの移動]
			SetIntToEdit(GetDlgItem(hDlg,IDC_EDIT_SHOWWINDOW_CURSORCORNER_DELAY),pConfig->ShowWindow.iCursorCornerDelay);

			//選択項目にカーソルを移動[マウスの移動]
			setCheck(hDlg,
					 IDC_CHECKBOX_MOUSE_MOVECURSORSELECTEDWINDOW,
					 pConfig->ShowWindow.bMouseMoveCursorSelectedWindow
					 );

			//選択項目にカーソルを移動[ホットキー]
			setCheck(hDlg,
					 IDC_CHECKBOX_HOTKEY_MOVECURSORSELECTEDWINDOW,
					 pConfig->ShowWindow.bHotKeyMoveCursorSelectedWindow
					 );

			//「選択項目にカーソルを移動」が有効なら表示時に移動[共通]
			setCheck(hDlg,
					 IDC_CHECKBOX_SHOW_MOVECURSORATSTART,
					 pConfig->ShowWindow.bMoveCursorAtStart
					 );

			//カーソルを移動[ホットキー]
			SendDlgItemMessage(hDlg,
							   IDC_COMBO_HOTKEY_CURSOR,
							   CB_SETCURSEL,
							   (WPARAM)pConfig->ShowWindow.eHotKeyCursorCorner,
							   (LPARAM)0
							   );

			//[ホットキー]
			EnableWindow(GetDlgItem(hDlg,IDC_STATIC_HOTKEY_SHIFT),LOBYTE(pConfig->ShowWindow.wHotKey)&MOD_SHIFT);
			EnableWindow(GetDlgItem(hDlg,IDC_STATIC_HOTKEY_CTRL),LOBYTE(pConfig->ShowWindow.wHotKey)&MOD_CONTROL);
			EnableWindow(GetDlgItem(hDlg,IDC_STATIC_HOTKEY_ALT),LOBYTE(pConfig->ShowWindow.wHotKey)&MOD_ALT);
			EnableWindow(GetDlgItem(hDlg,IDC_STATIC_HOTKEY_WIN),LOBYTE(pConfig->ShowWindow.wHotKey)&MOD_WIN);
			SendDlgItemMessage(hDlg,
							   IDC_HOTKEY_DISPLAY,
							   HKM_SETHOTKEY,
							   HIBYTE(pConfig->ShowWindow.wHotKey),
							   (LPARAM)0
							  );

			break;
		}//TAB_SHOWWINDOW

		case TAB_SHORTCUTKEY:{
			//「ショートカットキー」タブ
			break;
		}//TAB_SHORTCUTKEY

		case TAB_MOUSE:{
			//「マウス」タブ
			break;
		}//TAB_MOUSE

		case TAB_COMMAND:{
			//「コマンド」タブ
			break;
		}//TAB_COMMAND

		case TAB_DESIGN:{
			//「デザイン」タブ
			TCHAR szTmp[12]={};

			//文字色
			COLORREFToHex(szTmp,pConfig->DefaultItemDesign.clrText);
			SetWindowText(GetDlgItem(hDlg,IDC_EDIT_TEXTCOLOR),szTmp);

			//背景色
			COLORREFToHex(szTmp,pConfig->DefaultItemDesign.clrTextBk);
			SetWindowText(GetDlgItem(hDlg,IDC_EDIT_TEXTBKCOLOR),szTmp);

			//フォント名
			int iIndex=(int)SendDlgItemMessage(hDlg,IDC_COMBO_FONT,CB_FINDSTRINGEXACT,(WPARAM)0,(LPARAM)pConfig->DefaultItemDesign.szFont);
			if(iIndex==CB_ERR)iIndex=(int)SendDlgItemMessage(hDlg,IDC_COMBO_FONT,CB_FINDSTRINGEXACT,(WPARAM)0,(LPARAM)g_szDefaultString);//デフォルト
			SendDlgItemMessage(hDlg,IDC_COMBO_FONT,CB_SETCURSEL,(WPARAM)iIndex,(LPARAM)0);

			//フォントサイズ
			wsprintf(szTmp,_T("%d"),pConfig->DefaultItemDesign.iFontSize);
			iIndex=(int)SendDlgItemMessage(hDlg,IDC_COMBO_FONTSIZE,CB_FINDSTRINGEXACT,(WPARAM)0,(LPARAM)szTmp);
			if(iIndex==CB_ERR)iIndex=(int)SendDlgItemMessage(hDlg,IDC_COMBO_FONTSIZE,CB_FINDSTRINGEXACT,(WPARAM)0,(LPARAM)g_szDefaultString);//デフォルト
			SendDlgItemMessage(hDlg,IDC_COMBO_FONTSIZE,CB_SETCURSEL,(WPARAM)iIndex,(LPARAM)0);

			//フォントスタイル
			//太字[フォント]
			setCheck(hDlg,
					 IDC_CHECKBOX_BOLD,
					 pConfig->DefaultItemDesign.iFontStyle&SFONT_BOLD
					 );

			//斜体[フォント]
			setCheck(hDlg,
					 IDC_CHECKBOX_ITALIC,
					 pConfig->DefaultItemDesign.iFontStyle&SFONT_ITALIC
					 );

			//下線[フォント]
			setCheck(hDlg,
					 IDC_CHECKBOX_UNDERLINE,
					 pConfig->DefaultItemDesign.iFontStyle&SFONT_UNDERLINE
					 );

			//打ち消し線[フォント]
			setCheck(hDlg,
					 IDC_CHECKBOX_STRIKEOUT,
					 pConfig->DefaultItemDesign.iFontStyle&SFONT_STRIKEOUT
					 );

			//背景画像
			SetWindowText(GetDlgItem(hDlg,IDC_EDIT_BACKGROUND),pConfig->Background.szImagePath);

			TCHAR szPosition[12]={};

			//背景画像のオフセット
			InitializeSpinEditControl(GetDlgItem(hDlg,IDC_SPIN_XOFFSET),
									  GetDlgItem(hDlg,IDC_EDIT_XOFFSET),
									  -GetSystemMetrics(SM_CXSCREEN),
									  GetSystemMetrics(SM_CXSCREEN),
									  pConfig->Background.iXOffset
									 );
			SetIntToEdit(GetDlgItem(hDlg,IDC_EDIT_XOFFSET),pConfig->Background.iXOffset);
			InitializeSpinEditControl(GetDlgItem(hDlg,IDC_SPIN_YOFFSET),
									  GetDlgItem(hDlg,IDC_EDIT_YOFFSET),
									  -GetSystemMetrics(SM_CYSCREEN),
									  GetSystemMetrics(SM_CYSCREEN),
									  pConfig->Background.iYOffset
									 );
			SetIntToEdit(GetDlgItem(hDlg,IDC_EDIT_YOFFSET),pConfig->Background.iYOffset);


			//大きさ
			//範囲指定
			SendMessage(GetDlgItem(hDlg,IDC_SLIDER_RESIZEPERCENT),TBM_SETRANGE,(WPARAM)true,MAKELPARAM(0,100));
			//メモリの間隔は1
			SendMessage(GetDlgItem(hDlg,IDC_SLIDER_RESIZEPERCENT),TBM_SETTICFREQ,(WPARAM)1,(LPARAM)0);
			SendMessage(GetDlgItem(hDlg,IDC_SLIDER_RESIZEPERCENT),TBM_SETPOS,(WPARAM)true,(LPARAM)pConfig->Background.byResizePercent);
			//1Stepは1
			SendMessage(GetDlgItem(hDlg,IDC_SLIDER_RESIZEPERCENT),TBM_SETPAGESIZE,(WPARAM)0,(LPARAM)1);
			//数値表示
			wsprintf(szPosition,_T("%d%%"),pConfig->Background.byResizePercent);
			SetWindowText(GetDlgItem(hDlg,IDC_STATIC_RESIZEPERCENT),(LPCTSTR)szPosition);

			//不透明度
			//範囲指定
			SendMessage(GetDlgItem(hDlg,IDC_SLIDER_BG_ALPHA),TBM_SETRANGE,(WPARAM)true,MAKELPARAM(0,100));
			//メモリの間隔は1
			SendMessage(GetDlgItem(hDlg,IDC_SLIDER_BG_ALPHA),TBM_SETTICFREQ,(WPARAM)1,(LPARAM)0);
			SendMessage(GetDlgItem(hDlg,IDC_SLIDER_BG_ALPHA),TBM_SETPOS,(WPARAM)true,(LPARAM)pConfig->Background.byOpacity);
			//1Stepは1
			SendMessage(GetDlgItem(hDlg,IDC_SLIDER_BG_ALPHA),TBM_SETPAGESIZE,(WPARAM)0,(LPARAM)1);
			//数値表示
			wsprintf(szPosition,_T("%d%%"),pConfig->Background.byOpacity);
			SetWindowText(GetDlgItem(hDlg,IDC_STATIC_BG_ALPHA),(LPCTSTR)szPosition);

			break;
		}//TAB_DESIGN

		case TAB_ADVANCED:{
			//「高度な設定」タブ
			//動作
			//ドラッグ中はマウスホバーで選択[動作]
			setCheck(hDlg,
					 IDC_CHECKBOX_DRAG_MOUSEHOVER,
					 pConfig->ListView.bDragMouseHover
					 );

			//ドラッグ中はタイムアウトで確定[動作]
			setCheck(hDlg,
					 IDC_CHECKBOX_DRAG_TIMEOUT,
					 pConfig->ListView.bDragTimeOut
					 );

			//タイムアウト[動作]
			SetIntToEdit(GetDlgItem(hDlg,IDC_EDIT_TIMEOUT),pConfig->ListView.iTimeOut);

			//サムネイル
			//表示位置
			SendDlgItemMessage(hDlg,
							   IDC_COMBO_THUMBNAIL,
							   CB_SETCURSEL,
							   (WPARAM)pConfig->ListView.uThumbnail,
							   (LPARAM)0
							   );
			SendDlgItemMessage(hDlg,
							   IDC_COMBO_THUMBNAIL_VERTICALALIGN,
							   CB_SETCURSEL,
							   (WPARAM)pConfig->ListView.uThumbnailVerticalAlign,
							   (LPARAM)0
							   );
			EnableWindow(GetDlgItem(hDlg,IDC_COMBO_THUMBNAIL_VERTICALALIGN),g_ConfigTmp.ListView.uThumbnail!=THUMBNAIL_NO);
			//サムネイルの大きさ
			SetIntToEdit(GetDlgItem(hDlg,IDC_EDIT_THUMBNAIL_SIZE),pConfig->ListView.iThumbnailSize);
			EnableWindow(GetDlgItem(hDlg,IDC_EDIT_THUMBNAIL_SIZE),g_ConfigTmp.ListView.uThumbnail!=THUMBNAIL_NO);
			//サムネイルを元の大きさで表示
			setCheck(hDlg,
					 IDC_CHECKBOX_THUMBNAIL_ORIGINAL,
					 pConfig->ListView.bThumbnailOriginalSize
					 );
			EnableWindow(GetDlgItem(hDlg,IDC_CHECKBOX_THUMBNAIL_ORIGINAL),g_ConfigTmp.ListView.uThumbnail!=THUMBNAIL_NO);
			//サムネイル表示までの時間
			SetIntToEdit(GetDlgItem(hDlg,IDC_EDIT_THUMBNAIL_DELAY),pConfig->ListView.iThumbnailDelay);
			EnableWindow(GetDlgItem(hDlg,IDC_EDIT_THUMBNAIL_DELAY),g_ConfigTmp.ListView.uThumbnail!=THUMBNAIL_NO);

			//フィルタ
			//除外するファイル名[フィルタ]
			SetWindowText(GetDlgItem(hDlg,IDC_EDIT_EXCLUDE_FILENAME),pConfig->Exclude.szFileName);

			//ウインドウリストを表示するモニタ[マルチモニタ]
			SendDlgItemMessage(hDlg,
							   IDC_COMBO_MULTIMONITOR_MODE,
							   CB_SETCURSEL,
							   (WPARAM)pConfig->MultiMonitor.iActiveMonitorMode,
							   (LPARAM)0
							   );
			if(g_ConfigTmp.MultiMonitor.iActiveMonitorMode==ACTIVEMONITOR_BY_NAME&&
			   lstrlen(pConfig->MultiMonitor.szMonitorName)){
				if(CB_ERR==SendDlgItemMessage(hDlg,
											  IDC_COMBO_MULTIMONITOR_MODE,
											  CB_SELECTSTRING,
											  (WPARAM)-1,
											  (LPARAM)pConfig->MultiMonitor.szMonitorName)){
					SendDlgItemMessage(hDlg,
									   IDC_COMBO_MULTIMONITOR_MODE,
									   CB_SETCURSEL,
									   (WPARAM)0,
									   (LPARAM)0
									   );
				}
			}

			//アクティブモニタのウインドウのみ表示[マルチモニタ]
			setCheck(hDlg,
					 IDC_CHECKBOX_ONLY_ACTIVEMONITOR,
					 pConfig->MultiMonitor.bOnlyActiveMonitor
					 );

			//アクティブな仮想デスクトップのウインドウのみ表示[仮想デスクトップ]
			setCheck(hDlg,
					 IDC_CHECKBOX_ONLY_ACTIVEVIRTUALDESKTOP,
					 pConfig->VirtualDesktop.bOnlyActiveVirtualDesktop
					 );
			break;
		}//case TAB_ADVANCED
	}
	return;
}

//「全般」タブ
BOOL CALLBACK GeneralTabProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam){
	static struct LISTVIEWCOLUMN_TABLE ListViewColumn_Table[LISTITEM_NUM];

	static LPCTSTR lpszIcon[]={
		_T("アイコンなし(大)"),
		_T("アイコンなし(小)"),
		_T("小さなアイコン"),
		_T("大きなアイコン"),
	};

	static struct SPINEDIT_TABLE ListViewItemWidth_Table[LISTITEM_NUM];

	static LPCTSTR lpszMatchMode[]={
		_T("先頭一致"),
		_T("部分一致"),
		_T("あいまい")
	};

	static LPCTSTR lpszMigemoMode[]={
		_T("使用しない"),
		_T("小文字入力で有効"),
		_T("大文字入力で有効"),
		_T("常に有効"),
	};

	//Slider+Staticコントロール
	typedef struct{
		UINT uSliderId;
		UINT uStaticId;
		BYTE* pbyValue;
	}SLIDER_TABLE;

	static SLIDER_TABLE Slider_Table[]={
		//不透明度
		{
			IDC_SLIDER_ALPHA,
			IDC_STATIC_ALPHA,
			&g_ConfigTmp.ListView.byOpacity,
		},
	};


	//CreateShortcut()用
	static TCHAR szExecute[MAX_PATH];
	static TCHAR szWorkingDirectory[MAX_PATH];
	static TCHAR szStartupLinkPath[MAX_PATH];

	switch(uMsg){
		case WM_INITDIALOG:{
			//アイコン用コンボボックスの設定
			for(UINT i=0;i<ARRAY_SIZEOF(lpszIcon);i++){
				SendDlgItemMessage(hDlg,IDC_COMBO_ICON,CB_ADDSTRING,(WPARAM)0,(LPARAM)lpszIcon[i]);
			}
			//検索方法用コンボボックスの設定
			for(UINT i=0;i<ARRAY_SIZEOF(lpszMatchMode);i++){
				SendDlgItemMessage(hDlg,IDC_COMBO_SEARCH,CB_ADDSTRING,(WPARAM)0,(LPARAM)lpszMatchMode[i]);
			}
			//Migemoモード用コンボボックスの設定
			for(UINT i=0;i<ARRAY_SIZEOF(lpszMigemoMode);i++){
				SendDlgItemMessage(hDlg,IDC_COMBO_MIGEMO,CB_ADDSTRING,(WPARAM)0,(LPARAM)lpszMigemoMode[i]);
			}
			//カラムを設定
			LisView_InitColumn(ListViewColumn_Table,&g_ConfigTmp,g_hListView);

			{
				SPINEDIT_TABLE FileName_Table={IDC_SPIN_FILENAME,IDC_EDIT_FILENAME,ListViewColumn_Table[LISTITEM_FILENAME].piWidth,0,GetSystemMetrics(SM_CXSCREEN)};
				SPINEDIT_TABLE WindowTitle_Table={IDC_SPIN_WINDOWTITLE,IDC_EDIT_WINDOWTITLE,ListViewColumn_Table[LISTITEM_WINDOWTITLE].piWidth,0,GetSystemMetrics(SM_CXSCREEN)};
				ListViewItemWidth_Table[LISTITEM_FILENAME]=FileName_Table;
				ListViewItemWidth_Table[LISTITEM_WINDOWTITLE]=WindowTitle_Table;
			}

			//スタートアップディレクトリ内のショートカットファイルの存在を設定に合わせる
			GetModuleFileName(NULL,szExecute,MAX_PATH);

			lstrcpy(szWorkingDirectory,szExecute);
			//ファイル名を取り除く
			PathRemoveFileSpec(szWorkingDirectory);

			//Startupパスを取得
			SHGetSpecialFolderPath(NULL,szStartupLinkPath,CSIDL_STARTUP,false);
			//ファイル名を結合
			TCHAR szBuffer[32]={};
			LoadString(g_hInstance,IDS_FILENAME,(LPTSTR)&szBuffer,sizeof(szBuffer)-1);
			lstrcat(szStartupLinkPath,_T("\\"));
			lstrcat(szStartupLinkPath,szBuffer);
			lstrcat(szStartupLinkPath,_T(".lnk"));

			if(g_ConfigTmp.bAutoStart){//スタートアップ(自動起動)
				//ショートカット作成
				g_ConfigTmp.bAutoStart=CreateShortcut(szStartupLinkPath,szExecute,NULL,NULL,szWorkingDirectory);
				//レジストリ有効化
				EnableStartupApproved(szStartupLinkPath,true);
			}else{
				//無効な場合削除
				DeleteFile(szStartupLinkPath);
				//レジストリ削除
				RemoveStartupApproved(szStartupLinkPath);
			}

			//現在の設定をコントロールに適用
			SetCurrentSettings(hDlg,&g_ConfigTmp,TAB_GENERAL);
			return true;
		}

		case WM_CTLCOLORSTATIC:{
			//テキストの背景色を透過させる
			HDC hDC=(HDC)wParam;
			SetBkMode(hDC,TRANSPARENT);
			break;
		}

		case WM_SHOWWINDOW:
			//タイムアウト設定を同期
			if(wParam)SetCurrentSettings(hDlg,&g_ConfigTmp,TAB_GENERAL);
			break;

		case WM_HSCROLL:{
			TCHAR szPosition[12]={};

			for(UINT i=0;i<ARRAY_SIZEOF(Slider_Table);i++){
				if((HWND)lParam==GetDlgItem(hDlg,Slider_Table[i].uSliderId)){
					*Slider_Table[i].pbyValue=(int)SendMessage(GetDlgItem(hDlg,Slider_Table[i].uSliderId),TBM_GETPOS,(WPARAM)0,(LPARAM)0);
					//数値表示
					wsprintf(szPosition,_T("%d%%"),*Slider_Table[i].pbyValue);
					SetWindowText(GetDlgItem(hDlg,Slider_Table[i].uStaticId),(LPCTSTR)szPosition);
					break;
				}
			}
			break;
		}

		case WM_COMMAND:{
			switch(LOWORD(wParam)){
				//[動作]
				case IDC_CHECKBOX_DISABLEDOUBLECLICK://ダブルクリックを使用しない[動作]
					g_ConfigTmp.ListView.bDisableDoubleClick=getCheck(hDlg,LOWORD(wParam));
					break;

				case IDC_CHECKBOX_MOUSEOUT://マウスアウトで確定[動作]
					g_ConfigTmp.ListView.bMouseOut=getCheck(hDlg,LOWORD(wParam));
					break;

				case IDC_CHECKBOX_HOTKEY://ホットキーで確定[動作]
					g_ConfigTmp.ListView.bHotKey=getCheck(hDlg,LOWORD(wParam));
					break;

				case IDC_CHECKBOX_MOUSEHOVER://マウスホバーで選択[動作]
					g_ConfigTmp.ListView.bMouseHover=getCheck(hDlg,LOWORD(wParam));
					break;

				case IDC_CHECKBOX_TIMEOUT://タイムアウトで確定[動作]
					g_ConfigTmp.ListView.iTimeOut=(getCheck(hDlg,LOWORD(wParam)))?DEFAULT_SELECT_TIMEOUT:0;
					break;

				case IDC_CHECKBOX_SELECTSECONDWINDOW://2番目にアクティブなウインドウを選択[動作]
					g_ConfigTmp.ListView.bSelectSecondWindow=getCheck(hDlg,LOWORD(wParam));
					break;

				case IDC_CHECKBOX_THUMBNAIL://サムネイルを表示[動作]
					g_ConfigTmp.ListView.uThumbnail=getCheck(hDlg,LOWORD(wParam));
					break;


				case IDC_CHECKBOX_AUTOSTART://自動起動[起動]
					g_ConfigTmp.bAutoStart=getCheck(hDlg,LOWORD(wParam));
					break;

				case IDC_CHECKBOX_RUN_AS_ADMINISTRATOR://管理者権限で起動する[起動]
					g_ConfigTmp.bRunAsAdministrator=getCheck(hDlg,LOWORD(wParam));
					break;


				//[表示項目]
				case IDC_CHECKBOX_DESKTOPITEM://デスクトップ[表示項目]
					g_ConfigTmp.ListView.bDesktopItem=getCheck(hDlg,LOWORD(wParam));
					break;

				case IDC_CHECKBOX_CANCELITEM://キャンセル[表示項目]
					g_ConfigTmp.ListView.bCancelItem=getCheck(hDlg,LOWORD(wParam));
					break;

				//[インクリメンタルサーチ]
				case IDC_CHECKBOX_FIRSTCOLUMNONLY://左側の項目のみ検索[インクリメンタルサーチ]
					g_ConfigTmp.IncrementalSearch.bFirstColumnOnly=getCheck(hDlg,LOWORD(wParam));
					break;

				case IDC_CHECKBOX_ENTERUNIQUEWINDOW://候補のウインドウが1つなら確定[インクリメンタルサーチ]
					g_ConfigTmp.IncrementalSearch.bEnterUniqueWindow=getCheck(hDlg,LOWORD(wParam));
					break;

				case IDC_CHECKBOX_MIGEMO_CASESENSITIVE://連文節検索を行う[インクリメンタルサーチ]
					g_ConfigTmp.IncrementalSearch.bMigemoCaseSensitive=getCheck(hDlg,LOWORD(wParam));
					break;

				default:
					break;
			}

			switch(HIWORD(wParam)){
				case CBN_SELCHANGE://コンボボックスの選択内容が変更された
					switch(LOWORD(wParam)){
						case IDC_COMBO_ICON://アイコン
							g_ConfigTmp.ListView.iIcon=(int)SendDlgItemMessage(hDlg,LOWORD(wParam),CB_GETCURSEL,(WPARAM)0,(LPARAM)0);
							if(g_ConfigTmp.ListView.iIcon==CB_ERR)g_ConfigTmp.ListView.iIcon=0;
							break;
						case IDC_COMBO_SEARCH://検索方法
							g_ConfigTmp.IncrementalSearch.iMatchMode=(int)SendDlgItemMessage(hDlg,LOWORD(wParam),CB_GETCURSEL,(WPARAM)0,(LPARAM)0);
							if(g_ConfigTmp.IncrementalSearch.iMatchMode==CB_ERR)g_ConfigTmp.IncrementalSearch.iMatchMode=0;
							break;
						case IDC_COMBO_MIGEMO://Migemoモード
							g_ConfigTmp.IncrementalSearch.iMigemoMode=(int)SendDlgItemMessage(hDlg,LOWORD(wParam),CB_GETCURSEL,(WPARAM)0,(LPARAM)0);
							if(g_ConfigTmp.IncrementalSearch.iMigemoMode==CB_ERR)g_ConfigTmp.IncrementalSearch.iMigemoMode=0;
							break;
						default:
							break;
					}
					break;
				case EN_CHANGE://エディットボックスの内容が変更される直前
					//ファイル名、ウインドウタイトル[表示項目]
					for(int i=0;i<static_cast<int>(ARRAY_SIZEOF(ListViewItemWidth_Table));i++){
						if(LOWORD(wParam)==ListViewItemWidth_Table[i].uEditId){
							*ListViewItemWidth_Table[i].piValue=GetIntFromEdit(GetDlgItem(hDlg,ListViewItemWidth_Table[i].uEditId),
																			   ListViewItemWidth_Table[i].iMinimum,
																			   ListViewItemWidth_Table[i].iMaximum);

							if(*ListViewItemWidth_Table[i].piValue){
								ListViewColumn_Table[i].bVisible=true;
							}else{
								ListViewColumn_Table[i].bVisible=false;
							}
						}
					}
					break;
				default:
					break;
			}

			break;
		}

		case WM_NOTIFY:
			//ファイル名、ウインドウタイトル[表示項目]
			for(int i=0;i<static_cast<int>(ARRAY_SIZEOF(ListViewItemWidth_Table));i++){
				if(wParam==ListViewItemWidth_Table[i].uSpinId){
					LPNMUPDOWN lpNMUpDown=(LPNMUPDOWN)lParam;
					if(lpNMUpDown->hdr.code==UDN_DELTAPOS){
						*ListViewItemWidth_Table[i].piValue=GetIntFromEdit(GetDlgItem(hDlg,ListViewItemWidth_Table[i].uEditId),ListViewItemWidth_Table[i].iMinimum,ListViewItemWidth_Table[i].iMaximum);
						if((lpNMUpDown->iDelta)>0&&*ListViewItemWidth_Table[i].piValue<ListViewItemWidth_Table[i].iMaximum){
							//上が押された
							*ListViewItemWidth_Table[i].piValue+=1;
						}else if((lpNMUpDown->iDelta)<0&&*ListViewItemWidth_Table[i].piValue>ListViewItemWidth_Table[i].iMinimum){
							//下が押された
							*ListViewItemWidth_Table[i].piValue-=1;
						}

						if(*ListViewItemWidth_Table[i].piValue!=0){
							ListViewColumn_Table[i].bVisible=true;
						}else{
							ListViewColumn_Table[i].bVisible=false;
						}
						SetIntToEdit(GetDlgItem(hDlg,ListViewItemWidth_Table[i].uEditId),*ListViewItemWidth_Table[i].piValue);
					}
				}
			}
			break;

		case WM_CLOSETAB:{
			if(!ListViewColumn_Table[LISTITEM_FILENAME].bVisible
			   &&!ListViewColumn_Table[LISTITEM_WINDOWTITLE].bVisible){
				//全てのカラムが無効な場合、すべて表示する
				ListViewColumn_Table[LISTITEM_FILENAME].bVisible=
					ListViewColumn_Table[LISTITEM_WINDOWTITLE].bVisible=
						true;
				*ListViewColumn_Table[LISTITEM_FILENAME].piWidth=ListViewColumn_Table[LISTITEM_FILENAME].iDefaultWidth;
				*ListViewColumn_Table[LISTITEM_WINDOWTITLE].piWidth=ListViewColumn_Table[LISTITEM_WINDOWTITLE].iDefaultWidth;
			}
			if(g_ConfigTmp.bAutoStart){//自動起動[起動]
				//ショートカット作成
				g_ConfigTmp.bAutoStart=CreateShortcut(szStartupLinkPath,szExecute,NULL,NULL,szWorkingDirectory);
				//レジストリ有効化
				EnableStartupApproved(szStartupLinkPath,true);
			}else{
				//無効な場合削除
				DeleteFile(szStartupLinkPath);
				//レジストリ削除
				RemoveStartupApproved(szStartupLinkPath);
			}
			return true;
		}

		default:
			break;
	}
	return false;
}

LRESULT CALLBACK CursorCornerButtonProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam){
	typedef struct{
		HMENU hMenu;
		UINT uButtonId;
		SCC_CORNERS eDefault;
		CURSORCORNER* pCorner;
	}BUTTON_TABLE;

	static BUTTON_TABLE Button_Table[]={
		{
			NULL,
			IDC_CHECKBOX_LEFTTOP,
			SCC_RIGHTBOTTOM,
			&g_ConfigTmp.ShowWindow.ccCursorCorners[SCC_LEFTTOP],
		},
		{
			NULL,
			IDC_CHECKBOX_TOP,
			SCC_BOTTOM,
			&g_ConfigTmp.ShowWindow.ccCursorCorners[SCC_TOP],
		},
		{
			NULL,
			IDC_CHECKBOX_RIGHTTOP,
			SCC_LEFTBOTTOM,
			&g_ConfigTmp.ShowWindow.ccCursorCorners[SCC_RIGHTTOP],
		},
		{
			NULL,
			IDC_CHECKBOX_LEFT,
			SCC_RIGHT,
			&g_ConfigTmp.ShowWindow.ccCursorCorners[SCC_LEFT],
		},
		{
			NULL,
			IDC_CHECKBOX_RIGHT,
			SCC_LEFT,
			&g_ConfigTmp.ShowWindow.ccCursorCorners[SCC_RIGHT],
		},
		{
			NULL,
			IDC_CHECKBOX_LEFTBOTTOM,
			SCC_RIGHTTOP,
			&g_ConfigTmp.ShowWindow.ccCursorCorners[SCC_LEFTBOTTOM],
		},
		{
			NULL,
			IDC_CHECKBOX_BOTTOM,
			SCC_TOP,
			&g_ConfigTmp.ShowWindow.ccCursorCorners[SCC_BOTTOM],
		},
		{
			NULL,
			IDC_CHECKBOX_RIGHTBOTTOM,
			SCC_LEFTTOP,
			&g_ConfigTmp.ShowWindow.ccCursorCorners[SCC_RIGHTBOTTOM],
		},
	};

	static LPCTSTR lpszHotkeyCursor[]={
		_T("カーソルを移動しない"),
		_T("カーソルを左上に移動"),
		_T("カーソルを上へ移動"),
		_T("カーソルを右上へ移動"),
		_T("カーソルを左へ移動"),
		_T("カーソルを右へ移動"),
		_T("カーソルを左下へ移動"),
		_T("カーソルを下へ移動"),
		_T("カーソルを右下へ移動"),
		_T("カーソルを中央へ移動"),
	};

	switch(uMsg){
		case WM_CREATEMENU:{
			for(UINT i=0;i<ARRAY_SIZEOF(Button_Table);i++){
				if(GetDlgItem(GetParent(hWnd),Button_Table[i].uButtonId)==hWnd){
					Button_Table[i].hMenu=CreatePopupMenu();
					MENUITEMINFO mii={sizeof(MENUITEMINFO),MIIM_STATE};

					mii.fMask=MIIM_ID|MIIM_TYPE;
					mii.fType=MFT_STRING;
					for(UINT ii=0;ii<ARRAY_SIZEOF(lpszHotkeyCursor);ii++){
						mii.dwTypeData=(LPTSTR)lpszHotkeyCursor[ii];
						mii.wID=ii+1;
						InsertMenuItem(Button_Table[i].hMenu,ii,true,&mii);
					}
				}
			}
			break;
		}

		case WM_INITMENU:{
			MENUITEMINFO mii={sizeof(MENUITEMINFO),MIIM_STATE};

			for(UINT i=0;i<ARRAY_SIZEOF(Button_Table);i++){
				mii.fState=MFS_UNCHECKED;
				SetMenuItemInfo(Button_Table[i].hMenu,i+1,false,&mii);
				if(GetDlgItem(GetParent(hWnd),Button_Table[i].uButtonId)==hWnd){
					mii.fState=MFS_CHECKED;
					SetMenuItemInfo((HMENU)wParam,Button_Table[i].pCorner->eDestCorner+1,false,&mii);
				}
			}
			break;
		}

		case WM_LBUTTONDOWN:{
			//左クリック処理は親で行う
			break;
		}
		case WM_RBUTTONDOWN:{
			for(UINT i=0;i<ARRAY_SIZEOF(Button_Table);i++){
				if(GetDlgItem(GetParent(hWnd),Button_Table[i].uButtonId)==hWnd){
					if(!SendMessage(hWnd,BM_GETCHECK,(WPARAM)0,(LPARAM)0)){
						SendMessage(hWnd,BM_SETCHECK,(WPARAM)BST_CHECKED,(LPARAM)0);
					}
					POINT pt;

					GetCursorPos(&pt);
					int iIndex=TrackPopupMenu(Button_Table[i].hMenu,
											  TPM_RETURNCMD|TPM_TOPALIGN,
											  pt.x,pt.y,
											  0,
											  hWnd,
											  NULL);

					if(iIndex){
						Button_Table[i].pCorner->eDestCorner=(SCC_CORNERS)(iIndex-1);
						break;
					}
				}
			}
			return true;
		}

		case WM_DESTROY:
			for(UINT i=0;i<ARRAY_SIZEOF(Button_Table);i++){
				if(GetDlgItem(GetParent(hWnd),Button_Table[i].uButtonId)==hWnd){
					DestroyMenu(Button_Table[i].hMenu);
				}
			}

		default:
			break;
	}
	return CallWindowProc((WNDPROC)GetWindowLongPtr(hWnd,GWLP_USERDATA),hWnd,uMsg,wParam,lParam);
}

//「表示方法」タブ
BOOL CALLBACK ShowWindowTabProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam){
	static HICON hDesktopIcon;

	static LPCTSTR lpszHotkeyCursor[]={
		_T("カーソルを移動しない"),
		_T("カーソルを左上に移動"),
		_T("カーソルを上へ移動"),
		_T("カーソルを右上へ移動"),
		_T("カーソルを左へ移動"),
		_T("カーソルを右へ移動"),
		_T("カーソルを左下へ移動"),
		_T("カーソルを下へ移動"),
		_T("カーソルを右下へ移動"),
		_T("カーソルを中央へ移動"),
	};

	static UINT uButtonIds[]={
		IDC_CHECKBOX_LEFTTOP,
		IDC_CHECKBOX_TOP,
		IDC_CHECKBOX_RIGHTTOP,
		IDC_CHECKBOX_LEFT,
		IDC_CHECKBOX_RIGHT,
		IDC_CHECKBOX_LEFTBOTTOM,
		IDC_CHECKBOX_BOTTOM,
		IDC_CHECKBOX_RIGHTBOTTOM,
	};


	switch(uMsg){
		case WM_INITDIALOG:{
			g_hDisplayHotkey=GetDlgItem(hDlg,IDC_HOTKEY_DISPLAY);
			//サブクラス化
			SetWindowLongPtr(g_hDisplayHotkey,GWLP_USERDATA,GetWindowLongPtr(g_hDisplayHotkey,GWLP_WNDPROC));
			SetWindowLongPtr(g_hDisplayHotkey,GWLP_WNDPROC,(LONG_PTR)DisplayHotKeyProc);

			for(UINT i=0;i<ARRAY_SIZEOF(uButtonIds);i++){
				SetWindowLongPtr(GetDlgItem(hDlg,uButtonIds[i]),GWLP_USERDATA,GetWindowLongPtr(GetDlgItem(hDlg,uButtonIds[i]),GWLP_WNDPROC));
				SetWindowLongPtr(GetDlgItem(hDlg,uButtonIds[i]),GWLP_WNDPROC,(LONG_PTR)CursorCornerButtonProc);
				PostMessage(GetDlgItem(hDlg,uButtonIds[i]),WM_CREATEMENU,(WPARAM)0,(LPARAM)0);
			}

			//カーソル移動項目を追加
			for(UINT i=0;i<ARRAY_SIZEOF(lpszHotkeyCursor);i++){
				SendDlgItemMessage(hDlg,IDC_COMBO_HOTKEY_CURSOR,CB_ADDSTRING,(WPARAM)0,(LPARAM)lpszHotkeyCursor[i]);
			}

			//アイコン読み込み
			hDesktopIcon=GetDesktopIcon(false);

			//アイコン設定
			SendMessage(GetDlgItem(hDlg,IDC_PICTURE_DESKTOP),STM_SETIMAGE,IMAGE_ICON,(LPARAM)hDesktopIcon);

			//現在の設定をコントロールに適用
			SetCurrentSettings(hDlg,&g_ConfigTmp,TAB_SHOWWINDOW);
			return true;
		}

		case WM_CTLCOLORSTATIC:{//テキストの背景色を透過させる
			HDC hDC=(HDC)wParam;
			SetBkMode(hDC,TRANSPARENT);
			break;
		}

		case WM_COMMAND:{
			switch(LOWORD(wParam)){
				case IDC_CHECKBOX_LEFTTOP://左上[マウスの移動]
				case IDC_CHECKBOX_TOP://上[マウスの移動]
				case IDC_CHECKBOX_RIGHTTOP://右上[マウスの移動]
				case IDC_CHECKBOX_LEFT://左[マウスの移動]
				case IDC_CHECKBOX_RIGHT://右[マウスの移動]
				case IDC_CHECKBOX_LEFTBOTTOM://左下[マウスの移動]
				case IDC_CHECKBOX_BOTTOM://下[マウスの移動]
				case IDC_CHECKBOX_RIGHTBOTTOM://右下[マウスの移動]
					for(UINT i=0;i<ARRAY_SIZEOF(uButtonIds);i++){
						if(LOWORD(wParam)==uButtonIds[i]){
							g_ConfigTmp.ShowWindow.ccCursorCorners[i+1].bValid=getCheck(hDlg,LOWORD(wParam));
							break;
						}
					}
					break;

				case IDC_CHECKBOX_SHOW_MOUSEWHEEL://マウスホイール回転時のみ[マウスの移動]
					g_ConfigTmp.ShowWindow.bMouseWheel=getCheck(hDlg,LOWORD(wParam));
					break;

				case IDC_CHECKBOX_MOUSE_EACH_MONITOR://モニタごとに画面端を判定
					g_ConfigTmp.ShowWindow.bMouseEachMonitor=getCheck(hDlg,LOWORD(wParam));
					break;

				case IDC_CHECKBOX_MOUSE_MOVECURSORSELECTEDWINDOW://選択項目にカーソルを移動[マウスの移動]
					g_ConfigTmp.ShowWindow.bMouseMoveCursorSelectedWindow=getCheck(hDlg,LOWORD(wParam));
					break;

				case IDC_CHECKBOX_HOTKEY_MOVECURSORSELECTEDWINDOW://選択項目にカーソルを移動[ホットキー]
					g_ConfigTmp.ShowWindow.bHotKeyMoveCursorSelectedWindow=getCheck(hDlg,LOWORD(wParam));
					break;

				case IDC_CHECKBOX_SHOW_MOVECURSORATSTART://「選択項目にカーソルを移動」が有効なら表示時に移動[マウスの移動]
					g_ConfigTmp.ShowWindow.bMoveCursorAtStart=getCheck(hDlg,LOWORD(wParam));
					break;

				case IDC_HOTKEY_DISPLAY:{//ホットキー
					WORD wModifiers=
						(IsWindowEnabled(GetDlgItem(hDlg,IDC_STATIC_HOTKEY_SHIFT))?MOD_SHIFT:0)|
							(IsWindowEnabled(GetDlgItem(hDlg,IDC_STATIC_HOTKEY_CTRL))?MOD_CONTROL:0)|
								(IsWindowEnabled(GetDlgItem(hDlg,IDC_STATIC_HOTKEY_ALT))?MOD_ALT:0)|
									(IsWindowEnabled(GetDlgItem(hDlg,IDC_STATIC_HOTKEY_WIN))?MOD_WIN:0);
					WORD wVKey=(WORD)SendDlgItemMessage(hDlg,LOWORD(wParam),HKM_GETHOTKEY,(WPARAM)0,(LPARAM)0);

					g_ConfigTmp.ShowWindow.wHotKey=MAKEWORD(wModifiers,wVKey);
					break;
				}

				case IDC_BUTTON_UNREGHOTKEY://解除[ホットキー]
					g_ConfigTmp.ShowWindow.wHotKey=0;
					EnableWindow(GetDlgItem(hDlg,IDC_STATIC_HOTKEY_SHIFT),false);
					EnableWindow(GetDlgItem(hDlg,IDC_STATIC_HOTKEY_CTRL),false);
					EnableWindow(GetDlgItem(hDlg,IDC_STATIC_HOTKEY_ALT),false);
					EnableWindow(GetDlgItem(hDlg,IDC_STATIC_HOTKEY_WIN),false);
					SendDlgItemMessage(hDlg,IDC_HOTKEY_DISPLAY,HKM_SETHOTKEY,(WPARAM)0,(LPARAM)0);
					break;

				default:
					break;

			}

			switch(HIWORD(wParam)){
				case EN_CHANGE:
					switch(LOWORD(wParam)){
						case IDC_EDIT_SHOWWINDOW_CURSORCORNER_DELAY:
							//カーソル移動の判定時間[動作]
							g_ConfigTmp.ShowWindow.iCursorCornerDelay=GetIntFromEdit(GetDlgItem(hDlg,LOWORD(wParam)),0,10000);
							break;

						default:
							break;
					}
					break;

				case CBN_SELCHANGE:{//コンボボックスの選択内容が変更された
					switch(LOWORD(wParam)){
						case IDC_COMBO_HOTKEY_CURSOR:{
							g_ConfigTmp.ShowWindow.eHotKeyCursorCorner=(SCC_CORNERS)SendDlgItemMessage(hDlg,LOWORD(wParam),CB_GETCURSEL,(WPARAM)0,(LPARAM)0);
							if(g_ConfigTmp.ShowWindow.eHotKeyCursorCorner==CB_ERR)g_ConfigTmp.ShowWindow.eHotKeyCursorCorner=SCC_NONE;
							break;
						}
					default:
						break;
					}
					break;
				}

				default:
					break;
			}

			break;
		}

		case WM_CLOSETAB:
			DestroyIcon(hDesktopIcon);
			return true;

		default:
			break;
	}

	return false;
}

//ショートカットキーリストのヘッダーのプロシージャ
LRESULT CALLBACK ShortcutKeyListHeaderProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam){
	switch(uMsg){
		//ヘッダーサイズの固定
		case WM_SETCURSOR:
			return TRUE;
		case WM_LBUTTONDBLCLK:
			return FALSE;

		default:
			break;
	}
	return CallWindowProc((WNDPROC)GetWindowLongPtr(hWnd,GWLP_USERDATA),hWnd,uMsg,wParam,lParam);
}

//ショートカットキーリストのプロシージャ
LRESULT CALLBACK ShortcutKeyListProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam){
	switch(uMsg){
		//ヘッダーサイズの固定
		case WM_NOTIFY:
			if(((LPNMHEADER)lParam)){
				switch(((LPNMHEADER)lParam)->hdr.code){
					case HDN_ENDTRACKA:
					case HDN_BEGINTRACKA:
					case HDN_BEGINTRACKW:
					case HDN_ENDTRACKW:
						return TRUE;
				}
			}
			break;

		default:
			break;
	}
	return CallWindowProc((WNDPROC)GetWindowLongPtr(hWnd,GWLP_USERDATA),hWnd,uMsg,wParam,lParam);
}

//「ショートカットキー」タブ
BOOL CALLBACK ShortcutKeyTabProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam){
	static HWND hListView;

	switch(uMsg){
		case WM_INITDIALOG:{
			hListView=GetDlgItem(hDlg,IDC_LISTVIEW_SHORTCUTKEY);

			//リストビュースタイル変更
			ListView_SetExtendedListViewStyle(hListView,
											  (ListView_GetExtendedListViewStyle(hListView)|
											  //LVS_EX_FULLROWSELECT(行全体選択)
											  LVS_EX_FULLROWSELECT|
											  //LVS_EX_GRIDLINES(罫線表示)
											  LVS_EX_GRIDLINES));


			//リストビューにカラムを追加
			LVCOLUMN LVColumn={};
			LVColumn.mask=LVCF_FMT|LVCF_TEXT|LVCF_SUBITEM;
			LVColumn.fmt=LVCFMT_LEFT;
			LVColumn.pszText=_T("ショートカットキー");
			LVColumn.iSubItem=0;
			ListView_InsertColumn(hListView,0,&LVColumn);
			LVColumn.pszText=_T("コマンド");
			LVColumn.iSubItem=1;
			ListView_InsertColumn(hListView,1,&LVColumn);

			//リストビューにアイテムを追加
			for(int i=0;i<ARRAY_SIZEOF(g_ConfigTmp.ShortcutKey.sKeyTable)&&g_ConfigTmp.ShortcutKey.sKeyTable[i].wKey;i++){
				for(int ii=0;ii<ARRAY_SIZEOF(ShortcutKeyCmd_Table);ii++){
					if(ShortcutKeyCmd_Table[ii].uCmdId==g_ConfigTmp.ShortcutKey.sKeyTable[i].uCmdId){
						TCHAR szKey[32]={};
						LVITEM LVItem={};
						LVItem.mask=LVIF_TEXT;

						GetHotKeyName(g_ConfigTmp.ShortcutKey.sKeyTable[i].wKey,szKey,ARRAY_SIZEOF(szKey));

						LVItem.pszText=szKey;
						LVItem.iItem=i;
						LVItem.iSubItem=0;
						ListView_InsertItem(hListView,&LVItem);

						if(ShortcutKeyCmd_Table[ii].uCmdId<IDM_KEY_COMMAND1){
							LVItem.pszText=(LPTSTR)ShortcutKeyCmd_Table[ii].szDescript;
						}else{
							int iCommandIndex=ShortcutKeyCmd_Table[ii].uCmdId-IDM_KEY_COMMAND1+1;
							if(lstrlen(g_ConfigTmp.Command[iCommandIndex].szCommandName)){
								LVItem.pszText=(LPTSTR)g_ConfigTmp.Command[iCommandIndex].szCommandName;
							}else{
								LVItem.pszText=(LPTSTR)ShortcutKeyCmd_Table[ii].szDescript;
							}
						}
						LVItem.iSubItem=1;
						ListView_SetItem(hListView,&LVItem);
					}
				}
			}

			//カラム幅の調整
			RECT rc={};
			GetClientRect(hListView,&rc);
			ListView_SetColumnWidth(hListView,0,LVSCW_AUTOSIZE_USEHEADER);
			ListView_SetColumnWidth(hListView,1,rc.right-rc.left-ListView_GetColumnWidth(hListView,0));

			//サブクラス化
			//リストビュー
			SetWindowLongPtr(hListView,GWLP_USERDATA,GetWindowLongPtr(hListView,GWLP_WNDPROC));
			SetWindowLongPtr(hListView,GWLP_WNDPROC,(LONG_PTR)ShortcutKeyListProc);
			//リストビューヘッダー
			HWND hListViewHeader=ListView_GetHeader(hListView);
			SetWindowLongPtr(hListViewHeader,GWLP_USERDATA,GetWindowLongPtr(hListViewHeader,GWLP_WNDPROC));
			SetWindowLongPtr(hListViewHeader,GWLP_WNDPROC,(LONG_PTR)ShortcutKeyListHeaderProc);
			//ホットキー
			g_hHotkey=GetDlgItem(hDlg,IDC_HOTKEY_SHORTCUTKEY);
			SetWindowLongPtr(g_hHotkey,GWLP_USERDATA,GetWindowLongPtr(g_hHotkey,GWLP_WNDPROC));
			SetWindowLongPtr(g_hHotkey,GWLP_WNDPROC,(LONG_PTR)HotKeyProc);

			//コマンド名をリストビュー、コンボボックスに反映
			SendMessage(hDlg,WM_UPDATECOMMAND,0,0);
			SendDlgItemMessage(hDlg,IDC_COMBO_SHORTCUTKEY,CB_SETCURSEL,0,0);

			//現在の設定をコントロールに適用
			SetCurrentSettings(hDlg,&g_ConfigTmp,TAB_SHORTCUTKEY);
			return true;
		}

		case WM_CTLCOLORSTATIC:{//テキストの背景色を透過させる
			HDC hDC=(HDC)wParam;
			SetBkMode(hDC,TRANSPARENT);
			break;
		}

		case WM_COMMAND:{
			switch(LOWORD(wParam)){
				case IDC_BUTTON_SAVEKEY:{//保存
					if(g_ConfigTmp.ShortcutKey.iSize>=MAX_SHORTCUTKEY)break;

					SKEY sKey={(WORD)SendMessage(g_hHotkey,HKM_GETHOTKEY,(WPARAM)0,(LPARAM)0)};

					if(!sKey.wKey)break;

					int iListIndex=g_ConfigTmp.ShortcutKey.iSize;
					bool bDup=false;

					for(int i=0;i<ARRAY_SIZEOF(g_ConfigTmp.ShortcutKey.sKeyTable)&&g_ConfigTmp.ShortcutKey.sKeyTable[i].wKey;i++){
						if(g_ConfigTmp.ShortcutKey.sKeyTable[i].wKey==sKey.wKey){
							bDup=true;
							iListIndex=i;
							break;
						}
					}

					int iComboIndex=(int)SendDlgItemMessage(hDlg,IDC_COMBO_SHORTCUTKEY,CB_GETCURSEL,(WPARAM)0,(LPARAM)0);

					if(iComboIndex==CB_ERR)iComboIndex=0;

					sKey.uCmdId=ShortcutKeyCmd_Table[iComboIndex].uCmdId;

					//ショートカットキーテーブルに追加
					g_ConfigTmp.ShortcutKey.sKeyTable[iListIndex]=sKey;

					//リストビューに追加
					TCHAR szKey[32]={};
					LVITEM LVItem={};
					LVItem.mask=LVIF_TEXT;

					GetHotKeyName(sKey.wKey,szKey,ARRAY_SIZEOF(szKey));

					LVItem.pszText=szKey;
					LVItem.iItem=iListIndex;
					LVItem.iSubItem=0;
					if(!bDup){
						ListView_InsertItem(hListView,&LVItem);
					}

					if(ShortcutKeyCmd_Table[iComboIndex].uCmdId<IDM_KEY_COMMAND1){
						LVItem.pszText=(LPTSTR)ShortcutKeyCmd_Table[iComboIndex].szDescript;
					}else{
						int iCommandIndex=ShortcutKeyCmd_Table[iComboIndex].uCmdId-IDM_KEY_COMMAND1+1;
						if(lstrlen(g_ConfigTmp.Command[iCommandIndex].szCommandName)){
							LVItem.pszText=(LPTSTR)g_ConfigTmp.Command[iCommandIndex].szCommandName;
						}else{
							LVItem.pszText=(LPTSTR)ShortcutKeyCmd_Table[iComboIndex].szDescript;
						}
					}
					LVItem.iSubItem=1;
					ListView_SetItem(hListView,&LVItem);

					g_ConfigTmp.ShortcutKey.iSize++;

					break;
				}

				case IDC_BUTTON_REMOVEKEY:{//削除
					int iIndex=ListView_GetNextItem(hListView,-1,LVIS_SELECTED);

					if(iIndex==-1)break;

					if(ListView_DeleteItem(hListView,iIndex)){
						if(g_ConfigTmp.ShortcutKey.iSize>1){
							while((UINT)(iIndex+1)<g_ConfigTmp.ShortcutKey.iSize){
								g_ConfigTmp.ShortcutKey.sKeyTable[iIndex]=g_ConfigTmp.ShortcutKey.sKeyTable[iIndex+1];
								iIndex++;
							}
						}

						SKEY sKey={};

						g_ConfigTmp.ShortcutKey.iSize--;
						g_ConfigTmp.ShortcutKey.sKeyTable[g_ConfigTmp.ShortcutKey.iSize]=sKey;
					}
					break;
				}

				default:
					break;

			}
			break;
		}

		case WM_NOTIFY:
			if(((LPNMHDR)lParam)->idFrom==IDC_LISTVIEW_SHORTCUTKEY){
				switch(((LPNMLISTVIEW)lParam)->hdr.code){
					case LVN_ITEMCHANGED:{//アイテムを選択
						if(((LPNMLISTVIEW)lParam)->iItem==-1||
						   ((LPNMLISTVIEW)lParam)->uNewState!=(LVIS_SELECTED|LVIS_FOCUSED)){
							return 0;
						}

						int iIndex=((LPNMLISTVIEW)lParam)->iItem;

						if(iIndex!=-1){
							SendMessage(g_hHotkey,HKM_SETHOTKEY,(WPARAM)g_ConfigTmp.ShortcutKey.sKeyTable[iIndex].wKey,(LPARAM)0);

							//コンボボックス選択
							for(int i=0;i<ARRAY_SIZEOF(ShortcutKeyCmd_Table);i++){
								if(ShortcutKeyCmd_Table[i].uCmdId==g_ConfigTmp.ShortcutKey.sKeyTable[iIndex].uCmdId){
									SendDlgItemMessage(hDlg,IDC_COMBO_SHORTCUTKEY,CB_SETCURSEL,i,0);
									break;
								}
							}
						}else{
							SendMessage(g_hHotkey,HKM_SETHOTKEY,(WPARAM)0,(LPARAM)0);
							SendDlgItemMessage(hDlg,IDC_COMBO_SHORTCUTKEY,CB_SETCURSEL,0,0);
						}
						break;
					}
					default:
						break;
				}
			}
			break;

		case WM_UPDATECOMMAND:{
			for(int i=0;i<ARRAY_SIZEOF(g_ConfigTmp.ShortcutKey.sKeyTable)&&g_ConfigTmp.ShortcutKey.sKeyTable[i].wKey;i++){
				for(int ii=0;ii<ARRAY_SIZEOF(ShortcutKeyCmd_Table);ii++){
					if(ShortcutKeyCmd_Table[ii].uCmdId==g_ConfigTmp.ShortcutKey.sKeyTable[i].uCmdId){
						TCHAR szKey[32]={};
						LVITEM LVItem={};
						LVItem.mask=LVIF_TEXT;
						LVItem.iItem=i;

						if(ShortcutKeyCmd_Table[ii].uCmdId<IDM_KEY_COMMAND1){
							LVItem.pszText=(LPTSTR)ShortcutKeyCmd_Table[ii].szDescript;
						}else{
							int iCommandIndex=ShortcutKeyCmd_Table[ii].uCmdId-IDM_KEY_COMMAND1+1;
							if(lstrlen(g_ConfigTmp.Command[iCommandIndex].szCommandName)){
								LVItem.pszText=(LPTSTR)g_ConfigTmp.Command[iCommandIndex].szCommandName;
							}else{
								LVItem.pszText=(LPTSTR)ShortcutKeyCmd_Table[ii].szDescript;
							}
						}
						LVItem.iSubItem=1;
						ListView_SetItem(hListView,&LVItem);
					}
				}
			}

			int iComboIndex=(int)SendDlgItemMessage(hDlg,IDC_COMBO_SHORTCUTKEY,CB_GETCURSEL,(WPARAM)0,(LPARAM)0);

			//全て削除
			SendDlgItemMessage(hDlg,IDC_COMBO_SHORTCUTKEY,CB_RESETCONTENT,0,0);

			for(UINT i=0;i<ARRAY_SIZEOF(ShortcutKeyCmd_Table);i++){
				if(ShortcutKeyCmd_Table[i].uCmdId<IDM_KEY_COMMAND1){
					SendDlgItemMessage(hDlg,IDC_COMBO_SHORTCUTKEY,CB_ADDSTRING,(WPARAM)0,(LPARAM)ShortcutKeyCmd_Table[i].szDescript);
				}else{
					int iCommandIndex=ShortcutKeyCmd_Table[i].uCmdId-IDM_KEY_COMMAND1+1;
					if(lstrlen(g_ConfigTmp.Command[iCommandIndex].szCommandName)){
						SendDlgItemMessage(hDlg,IDC_COMBO_SHORTCUTKEY,CB_ADDSTRING,(WPARAM)0,(LPARAM)g_ConfigTmp.Command[iCommandIndex].szCommandName);
					}else{
						SendDlgItemMessage(hDlg,IDC_COMBO_SHORTCUTKEY,CB_ADDSTRING,(WPARAM)0,(LPARAM)ShortcutKeyCmd_Table[i].szDescript);
					}
				}
			}

			if(iComboIndex!=CB_ERR){
				SendDlgItemMessage(hDlg,IDC_COMBO_SHORTCUTKEY,CB_SETCURSEL,iComboIndex,0);
			}
			break;
		}

		case WM_CLOSETAB:
			return true;

		default:
			break;
	}

	return false;
}

//「マウス」タブ
BOOL CALLBACK MouseTabProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam){
	static HWND hListView;

	switch(uMsg){
		case WM_INITDIALOG:{
			hListView=GetDlgItem(hDlg,IDC_LISTVIEW_MOUSE);

			//リストビュースタイル変更
			ListView_SetExtendedListViewStyle(hListView,
											  (ListView_GetExtendedListViewStyle(hListView)|
											  //LVS_EX_FULLROWSELECT(行全体選択)
											  LVS_EX_FULLROWSELECT|
											  //LVS_EX_GRIDLINES(罫線表示)
											  LVS_EX_GRIDLINES));


			//リストビューにカラムを追加
			LVCOLUMN LVColumn={};
			LVColumn.mask=LVCF_FMT|LVCF_TEXT|LVCF_SUBITEM;
			LVColumn.fmt=LVCFMT_LEFT;
			LVColumn.pszText=_T("マウス");
			LVColumn.iSubItem=0;
			ListView_InsertColumn(hListView,0,&LVColumn);
			LVColumn.pszText=_T("コマンド");
			LVColumn.iSubItem=1;
			ListView_InsertColumn(hListView,1,&LVColumn);

			//リストビューにアイテムを追加
			for(int i=0,iCount=ListView_GetItemCount(hListView);i<SMOUSE_NUM;i++){
				if(!g_ConfigTmp.Mouse.sMouseTable[i].uCmdId)continue;

				for(int ii=0;ii<ARRAY_SIZEOF(ShortcutKeyCmd_Table);ii++){
					if(ShortcutKeyCmd_Table[ii].uCmdId==g_ConfigTmp.Mouse.sMouseTable[i].uCmdId){
						LVITEM LVItem={};
						LVItem.mask=LVIF_TEXT;

						LVItem.pszText=(LPTSTR)MouseType_Table[i].szDescript;
						LVItem.iItem=iCount++;
						LVItem.iSubItem=0;
						ListView_InsertItem(hListView,&LVItem);

						if(ShortcutKeyCmd_Table[ii].uCmdId<IDM_KEY_COMMAND1){
							LVItem.pszText=(LPTSTR)ShortcutKeyCmd_Table[ii].szDescript;
						}else{
							int iCommandIndex=ShortcutKeyCmd_Table[ii].uCmdId-IDM_KEY_COMMAND1+1;
							if(lstrlen(g_ConfigTmp.Command[iCommandIndex].szCommandName)){
								LVItem.pszText=(LPTSTR)g_ConfigTmp.Command[iCommandIndex].szCommandName;
							}else{
								LVItem.pszText=(LPTSTR)ShortcutKeyCmd_Table[ii].szDescript;
							}
						}
						LVItem.iSubItem=1;
						ListView_SetItem(hListView,&LVItem);
					}
				}
			}

			//カラム幅の調整
			RECT rc={};
			GetClientRect(hListView,&rc);
			ListView_SetColumnWidth(hListView,0,LVSCW_AUTOSIZE_USEHEADER);
			ListView_SetColumnWidth(hListView,1,rc.right-rc.left-ListView_GetColumnWidth(hListView,0));

			//サブクラス化
			//リストビュー
			SetWindowLongPtr(hListView,GWLP_USERDATA,GetWindowLongPtr(hListView,GWLP_WNDPROC));
			SetWindowLongPtr(hListView,GWLP_WNDPROC,(LONG_PTR)ShortcutKeyListProc);
			//リストビューヘッダー
			HWND hListViewHeader=ListView_GetHeader(hListView);
			SetWindowLongPtr(hListViewHeader,GWLP_USERDATA,GetWindowLongPtr(hListViewHeader,GWLP_WNDPROC));
			SetWindowLongPtr(hListViewHeader,GWLP_WNDPROC,(LONG_PTR)ShortcutKeyListHeaderProc);

			for(int i=0;i<SMOUSE_NUM;i++){
				SendDlgItemMessage(hDlg,IDC_COMBO_MOUSE,CB_ADDSTRING,(WPARAM)0,(LPARAM)MouseType_Table[i].szDescript);
			}
			SendDlgItemMessage(hDlg,IDC_COMBO_MOUSE,CB_SETCURSEL,0,0);

			//コマンド名をリストビュー、コンボボックスに反映
			SendMessage(hDlg,WM_UPDATECOMMAND,0,0);
			SendDlgItemMessage(hDlg,IDC_COMBO_MOUSE_CMD,CB_SETCURSEL,0,0);

			//現在の設定をコントロールに適用
			SetCurrentSettings(hDlg,&g_ConfigTmp,TAB_MOUSE);
			return true;
		}

		case WM_CTLCOLORSTATIC:{//テキストの背景色を透過させる
			HDC hDC=(HDC)wParam;
			SetBkMode(hDC,TRANSPARENT);
			break;
		}

		case WM_COMMAND:{
			switch(LOWORD(wParam)){
				case IDC_BUTTON_SAVEMOUSE:{//保存
					int iComboIndex=(int)SendDlgItemMessage(hDlg,IDC_COMBO_MOUSE,CB_GETCURSEL,(WPARAM)0,(LPARAM)0);
					int iComboCmdIndex=(int)SendDlgItemMessage(hDlg,IDC_COMBO_MOUSE_CMD,CB_GETCURSEL,(WPARAM)0,(LPARAM)0);
					int iCount=ListView_GetItemCount(hListView);
					int iListIndex=iCount;

					if(iComboIndex==CB_ERR)iComboIndex=0;
					if(iComboCmdIndex==CB_ERR)iComboCmdIndex=0;

					bool bDup=g_ConfigTmp.Mouse.sMouseTable[iComboIndex].uCmdId!=0;

					if(bDup){
						for(int i=0;i<iCount;i++){
							TCHAR szMouseType[48]={};
							ListView_GetItemText(hListView,i,0,szMouseType,ARRAY_SIZEOF(szMouseType));

							if(lstrcmp(MouseType_Table[iComboIndex].szDescript,szMouseType)==0){
								iListIndex=i;
								break;
							}
						}
					}


					//マウステーブルに追加
					g_ConfigTmp.Mouse.sMouseTable[iComboIndex].uCmdId=ShortcutKeyCmd_Table[iComboCmdIndex].uCmdId;

					//リストビューに追加
					LVITEM LVItem={};
					LVItem.mask=LVIF_TEXT;

					LVItem.pszText=(LPTSTR)MouseType_Table[iComboIndex].szDescript;
					LVItem.iItem=iListIndex;
					LVItem.iSubItem=0;
					if(!bDup){
						ListView_InsertItem(hListView,&LVItem);
					}

					if(ShortcutKeyCmd_Table[iComboCmdIndex].uCmdId<IDM_KEY_COMMAND1){
						LVItem.pszText=(LPTSTR)ShortcutKeyCmd_Table[iComboCmdIndex].szDescript;
					}else{
						int iCommandIndex=ShortcutKeyCmd_Table[iComboCmdIndex].uCmdId-IDM_KEY_COMMAND1+1;
						if(lstrlen(g_ConfigTmp.Command[iCommandIndex].szCommandName)){
							LVItem.pszText=(LPTSTR)g_ConfigTmp.Command[iCommandIndex].szCommandName;
						}else{
							LVItem.pszText=(LPTSTR)ShortcutKeyCmd_Table[iComboCmdIndex].szDescript;
						}
					}
					LVItem.iSubItem=1;
					ListView_SetItem(hListView,&LVItem);

					break;
				}

				case IDC_BUTTON_REMOVEMOUSE:{//削除
					int iIndex=ListView_GetNextItem(hListView,-1,LVIS_SELECTED);

					if(iIndex==-1)break;

					TCHAR szMouseType[48]={};
					ListView_GetItemText(hListView,iIndex,0,szMouseType,ARRAY_SIZEOF(szMouseType));

					if(ListView_DeleteItem(hListView,iIndex)){
						for(int i=0;i<SMOUSE_NUM;i++){
							if(lstrcmp(MouseType_Table[i].szDescript,szMouseType)==0)
							g_ConfigTmp.Mouse.sMouseTable[i].uCmdId=0;
						}
					}
					break;
				}

				default:
					break;

			}
			break;
		}

		case WM_NOTIFY:
			if(((LPNMHDR)lParam)->idFrom==IDC_LISTVIEW_MOUSE){
				switch(((LPNMLISTVIEW)lParam)->hdr.code){
					case LVN_ITEMCHANGED:{//アイテムを選択
						if(((LPNMLISTVIEW)lParam)->iItem==-1||
						   ((LPNMLISTVIEW)lParam)->uNewState!=(LVIS_SELECTED|LVIS_FOCUSED)){
							return 0;
						}
						
						int iIndex=((LPNMLISTVIEW)lParam)->iItem;
						if(iIndex==-1)break;

						if(iIndex!=-1){
							TCHAR szMouseType[48]={};
							TCHAR szCmd[64]={};
							ListView_GetItemText(hListView,iIndex,0,szMouseType,ARRAY_SIZEOF(szMouseType));
							int iComboIndex=(int)SendDlgItemMessage(hDlg,IDC_COMBO_MOUSE,CB_FINDSTRINGEXACT,(WPARAM)0,(LPARAM)szMouseType);
							SendDlgItemMessage(hDlg,IDC_COMBO_MOUSE,CB_SETCURSEL,
											   iComboIndex,0);
							for(int i=0;i<ARRAY_SIZEOF(ShortcutKeyCmd_Table);i++){
								if(ShortcutKeyCmd_Table[i].uCmdId==g_ConfigTmp.Mouse.sMouseTable[iComboIndex].uCmdId){
									SendDlgItemMessage(hDlg,IDC_COMBO_MOUSE_CMD,CB_SETCURSEL,i,0);
									break;
								}
							}
						}else{
							SendMessage(g_hHotkey,HKM_SETHOTKEY,(WPARAM)0,(LPARAM)0);
							SendDlgItemMessage(hDlg,IDC_COMBO_SHORTCUTKEY,CB_SETCURSEL,0,0);
						}
						break;
					}
					default:
						break;
				}
			}
			break;

		case WM_UPDATECOMMAND:{
			for(int i=0,iItem=0;i<SMOUSE_NUM;i++){
				if(!g_ConfigTmp.Mouse.sMouseTable[i].uCmdId)continue;

				for(int ii=0;ii<ARRAY_SIZEOF(ShortcutKeyCmd_Table);ii++){
					if(ShortcutKeyCmd_Table[ii].uCmdId==g_ConfigTmp.Mouse.sMouseTable[i].uCmdId){
						LVITEM LVItem={};
						LVItem.mask=LVIF_TEXT;

						LVItem.iItem=iItem;

						if(ShortcutKeyCmd_Table[ii].uCmdId<IDM_KEY_COMMAND1){
							LVItem.pszText=(LPTSTR)ShortcutKeyCmd_Table[ii].szDescript;
						}else{
							int iCommandIndex=ShortcutKeyCmd_Table[ii].uCmdId-IDM_KEY_COMMAND1+1;
							if(lstrlen(g_ConfigTmp.Command[iCommandIndex].szCommandName)){
								LVItem.pszText=(LPTSTR)g_ConfigTmp.Command[iCommandIndex].szCommandName;
							}else{
								LVItem.pszText=(LPTSTR)ShortcutKeyCmd_Table[ii].szDescript;
							}
						}
						LVItem.iSubItem=1;
						ListView_SetItem(hListView,&LVItem);
						iItem++;
					}
				}
			}

			int iComboIndex=(int)SendDlgItemMessage(hDlg,IDC_COMBO_MOUSE_CMD,CB_GETCURSEL,(WPARAM)0,(LPARAM)0);

			//全て削除
			SendDlgItemMessage(hDlg,IDC_COMBO_MOUSE_CMD,CB_RESETCONTENT,0,0);

			for(UINT i=0;i<ARRAY_SIZEOF(ShortcutKeyCmd_Table);i++){
				if(ShortcutKeyCmd_Table[i].uCmdId<IDM_KEY_COMMAND1){
					SendDlgItemMessage(hDlg,IDC_COMBO_MOUSE_CMD,CB_ADDSTRING,(WPARAM)0,(LPARAM)ShortcutKeyCmd_Table[i].szDescript);
				}else{
					int iCommandIndex=ShortcutKeyCmd_Table[i].uCmdId-IDM_KEY_COMMAND1+1;
					if(lstrlen(g_ConfigTmp.Command[iCommandIndex].szCommandName)){
						SendDlgItemMessage(hDlg,IDC_COMBO_MOUSE_CMD,CB_ADDSTRING,(WPARAM)0,(LPARAM)g_ConfigTmp.Command[iCommandIndex].szCommandName);
					}else{
						SendDlgItemMessage(hDlg,IDC_COMBO_MOUSE_CMD,CB_ADDSTRING,(WPARAM)0,(LPARAM)ShortcutKeyCmd_Table[i].szDescript);
					}
				}
			}

			if(iComboIndex!=CB_ERR){
				SendDlgItemMessage(hDlg,IDC_COMBO_MOUSE_CMD,CB_SETCURSEL,iComboIndex,0);
			}
			break;
		}

		case WM_CLOSETAB:
			return true;

		default:
			break;
	}

	return false;
}

//「コマンド」タブ
BOOL CALLBACK CommandTabProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam){
	static HWND hListView;

	//「>」(引数)メニュー
	static HMENU hParametersMenu;

	static LPCTSTR lpszCommandOption[]={
		_T("何もせずに実行"),
		_T("切り替えてから実行"),
		_T("切り替えをキャンセルしてから実行"),
	};

	typedef struct{
		UINT uCmdShow;
		LPCTSTR szDescript;
	}CMDSHOW_TABLE;

	static CMDSHOW_TABLE CmdShow_Table[]={
		{
			SW_SHOWNORMAL,
			_T("通常のウインドウ"),
		},
		{
			SW_MAXIMIZE,
			_T("最大化"),
		},
		{
			SW_MINIMIZE,
			_T("最小化"),
		},
/*
		{
			SW_HIDE,
			_T("非表示"),
		},
*/
	};

	switch(uMsg){
		case WM_INITDIALOG:{
			hListView=GetDlgItem(hDlg,IDC_LISTVIEW_COMMAND);

			//リストビュースタイル変更
			ListView_SetExtendedListViewStyle(hListView,
											  (ListView_GetExtendedListViewStyle(hListView)|
											  //LVS_EX_FULLROWSELECT(行全体選択)
											  LVS_EX_FULLROWSELECT|
											  //LVS_EX_GRIDLINES(罫線表示)
											  LVS_EX_GRIDLINES));


			//リストビューにカラムを追加
			LVCOLUMN LVColumn={};
			LVColumn.mask=LVCF_FMT|LVCF_TEXT|LVCF_SUBITEM;
			LVColumn.fmt=LVCFMT_LEFT;
			LVColumn.pszText=_T("No.");
			LVColumn.iSubItem=0;
			ListView_InsertColumn(hListView,0,&LVColumn);
			LVColumn.pszText=_T("コマンド名");
			LVColumn.iSubItem=1;
			ListView_InsertColumn(hListView,1,&LVColumn);

			//リストビューにアイテムを追加
			for(int i=1;i<=MAX_COMMAND;i++){
				TCHAR szNo[16]={};
				LVITEM LVItem={};
				LVItem.mask=LVIF_TEXT;

				wsprintf(szNo,_T("%d"),i);

				LVItem.pszText=szNo;
				LVItem.iItem=i-1;
				LVItem.iSubItem=0;
				ListView_InsertItem(hListView,&LVItem);

				LVItem.pszText=(LPTSTR)g_ConfigTmp.Command[i].szCommandName;
				LVItem.iSubItem=1;
				ListView_SetItem(hListView,&LVItem);
			}

			//カラム幅の調整
			RECT rc={};
			GetClientRect(hListView,&rc);
			ListView_SetColumnWidth(hListView,0,LVSCW_AUTOSIZE_USEHEADER);
			ListView_SetColumnWidth(hListView,1,rc.right-rc.left-ListView_GetColumnWidth(hListView,0));

			//サブクラス化
			//リストビュー
			SetWindowLongPtr(hListView,GWLP_USERDATA,GetWindowLongPtr(hListView,GWLP_WNDPROC));
			SetWindowLongPtr(hListView,GWLP_WNDPROC,(LONG_PTR)ShortcutKeyListProc);
			//リストビューヘッダー
			HWND hListViewHeader=ListView_GetHeader(hListView);
			SetWindowLongPtr(hListViewHeader,GWLP_USERDATA,GetWindowLongPtr(hListViewHeader,GWLP_WNDPROC));
			SetWindowLongPtr(hListViewHeader,GWLP_WNDPROC,(LONG_PTR)ShortcutKeyListHeaderProc);

			//コマンドNo.
			for(UINT i=1;i<=MAX_COMMAND;i++){
				TCHAR szNo[16]={};

				wsprintf(szNo,_T("%d"),i);
				SendDlgItemMessage(hDlg,IDC_COMBO_CMDNO,CB_ADDSTRING,(WPARAM)0,(LPARAM)szNo);
			}

			//コマンド種別
			for(UINT i=0;i<ARRAY_SIZEOF(CommandMode_Table);i++){
				SendDlgItemMessage(hDlg,IDC_COMBO_CMDMODE,CB_ADDSTRING,(WPARAM)0,(LPARAM)CommandMode_Table[i].szDescript);
			}

			//「>」(引数)メニューを読み込む
			hParametersMenu=GetSubMenu(LoadMenu(g_hInstance,MAKEINTRESOURCE(IDR_CMDPARAMETERSMENU)),0);

			//コマンド実行前の動作
			for(UINT i=0;i<ARRAY_SIZEOF(lpszCommandOption);i++){
				SendDlgItemMessage(hDlg,IDC_COMBO_CMDOPTION,CB_ADDSTRING,(WPARAM)0,(LPARAM)lpszCommandOption[i]);
			}

			//表示方法
			for(UINT i=0;i<ARRAY_SIZEOF(CmdShow_Table);i++){
				SendDlgItemMessage(hDlg,IDC_COMBO_CMDSHOW,CB_ADDSTRING,(WPARAM)0,(LPARAM)CmdShow_Table[i].szDescript);
			}

			//No.1を選択
			SendDlgItemMessage(hDlg,IDC_COMBO_CMDNO,CB_SETCURSEL,0,0);
			PostMessage(hDlg,WM_COMMAND,MAKEWPARAM(IDC_COMBO_CMDNO,CBN_SELCHANGE),(LPARAM)GetDlgItem(hDlg,IDC_COMBO_CMDNO));

			//現在の設定をコントロールに適用
			SetCurrentSettings(hDlg,&g_ConfigTmp,TAB_COMMAND);
			return true;
		}

		case WM_CTLCOLORSTATIC:{//テキストの背景色を透過させる
			HDC hDC=(HDC)wParam;
			SetBkMode(hDC,TRANSPARENT);
			break;
		}

		case WM_COMMAND:{
			switch(LOWORD(wParam)){
				case IDC_BUTTON_CMDSAVE:{//保存
					int iIndex=(int)SendDlgItemMessage(hDlg,IDC_COMBO_CMDNO,CB_GETCURSEL,(WPARAM)0,(LPARAM)0);

					if(iIndex==CB_ERR)iIndex=0;

					iIndex++;

					{
						TCHAR szCommandName[MAX_PATH];
						TCHAR szFilePath[MAX_PATH];

						GetWindowText(GetDlgItem(hDlg,IDC_EDIT_CMDNAME),szCommandName,ARRAY_SIZEOF(szCommandName));
						GetWindowText(GetDlgItem(hDlg,IDC_EDIT_CMDFILEPATH),szFilePath,ARRAY_SIZEOF(szFilePath));

						if(!lstrlen(szCommandName)||
						   !lstrlen(szFilePath)){
							MessageBox(hDlg,_T("コマンド名/実行ファイルを指定してください。"),_T("Tascher"),MB_OK|MB_ICONEXCLAMATION);
							break;
						}
					}

					//コマンド名
					GetWindowText(GetDlgItem(hDlg,IDC_EDIT_CMDNAME),g_ConfigTmp.Command[iIndex].szCommandName,ARRAY_SIZEOF(g_ConfigTmp.Command[iIndex].szCommandName));
					//実行ファイル
					GetWindowText(GetDlgItem(hDlg,IDC_EDIT_CMDFILEPATH),g_ConfigTmp.Command[iIndex].szFilePath,ARRAY_SIZEOF(g_ConfigTmp.Command[iIndex].szFilePath));
					//引数
					GetWindowText(GetDlgItem(hDlg,IDC_EDIT_CMDPARAMETERS),g_ConfigTmp.Command[iIndex].szParameters,ARRAY_SIZEOF(g_ConfigTmp.Command[iIndex].szParameters));
					//作業ディレクトリ
					GetWindowText(GetDlgItem(hDlg,IDC_EDIT_CMDWORKINGDIRECTORY),g_ConfigTmp.Command[iIndex].szWorkingDirectory,ARRAY_SIZEOF(g_ConfigTmp.Command[iIndex].szWorkingDirectory));

					//コマンド種別
					int iComboIndex=(int)SendDlgItemMessage(hDlg,IDC_COMBO_CMDMODE,CB_GETCURSEL,(WPARAM)0,(LPARAM)0);

					if(iComboIndex==CB_ERR)iComboIndex=0;
					g_ConfigTmp.Command[iIndex].uCommandMode=iComboIndex;

					//コマンド実行前の動作
					iComboIndex=(int)SendDlgItemMessage(hDlg,IDC_COMBO_CMDOPTION,CB_GETCURSEL,(WPARAM)0,(LPARAM)0);

					if(iComboIndex==CB_ERR)iComboIndex=0;
					g_ConfigTmp.Command[iIndex].uCommandOption=iComboIndex;

					//表示方法
					iComboIndex=(int)SendDlgItemMessage(hDlg,IDC_COMBO_CMDSHOW,CB_GETCURSEL,(WPARAM)0,(LPARAM)0);

					if(iComboIndex==CB_ERR)iComboIndex=0;
					g_ConfigTmp.Command[iIndex].uCmdShow=CmdShow_Table[iComboIndex].uCmdShow;

					//リストビューの内容を変更
					TCHAR szNo[16]={};
					LVITEM LVItem={};
					LVItem.mask=LVIF_TEXT;

					wsprintf(szNo,_T("%d"),iIndex);

					LVItem.pszText=szNo;
					LVItem.iItem=iIndex-1;
					LVItem.iSubItem=0;

					LVItem.pszText=(LPTSTR)g_ConfigTmp.Command[iIndex].szCommandName;
					LVItem.iSubItem=1;
					ListView_SetItem(hListView,&LVItem);

					//他のタブのコマンド名更新
					SendMessage(hDlg,WM_SENDUPDATECOMMAND,0,0);
					break;
				}

				case IDC_BUTTON_CMDREMOVE:{//削除
					int iIndex=ListView_GetSelectedItem(hListView);

					if(iIndex==-1)break;

					iIndex++;

					//コマンド名
					lstrcpy(g_ConfigTmp.Command[iIndex].szCommandName,_T(""));
					SetWindowText(GetDlgItem(hDlg,IDC_EDIT_CMDNAME),g_ConfigTmp.Command[iIndex].szCommandName);
					//実行ファイル
					lstrcpy(g_ConfigTmp.Command[iIndex].szFilePath,_T(""));
					SetWindowText(GetDlgItem(hDlg,IDC_EDIT_CMDFILEPATH),g_ConfigTmp.Command[iIndex].szFilePath);
					//引数
					lstrcpy(g_ConfigTmp.Command[iIndex].szParameters,_T(""));
					SetWindowText(GetDlgItem(hDlg,IDC_EDIT_CMDPARAMETERS),g_ConfigTmp.Command[iIndex].szParameters);
					//作業ディレクトリ
					lstrcpy(g_ConfigTmp.Command[iIndex].szWorkingDirectory,_T(""));
					SetWindowText(GetDlgItem(hDlg,IDC_EDIT_CMDWORKINGDIRECTORY),g_ConfigTmp.Command[iIndex].szWorkingDirectory);
					//コマンド種別
					SendDlgItemMessage(hDlg,IDC_COMBO_CMDMODE,CB_SETCURSEL,0,0);
					g_ConfigTmp.Command[iIndex].uCommandMode=CMDMODE_OPEN;
					//コマンド実行前の動作
					SendDlgItemMessage(hDlg,IDC_COMBO_CMDOPTION,CB_SETCURSEL,0,0);
					g_ConfigTmp.Command[iIndex].uCommandOption=CMDOPT_NONE;
					//表示方法
					SendDlgItemMessage(hDlg,IDC_COMBO_CMDSHOW,CB_SETCURSEL,0,0);
					g_ConfigTmp.Command[iIndex].uCmdShow=SW_SHOWNORMAL;


					//リストビューの内容を変更
					TCHAR szNo[16]={};
					LVITEM LVItem={};
					LVItem.mask=LVIF_TEXT;

					wsprintf(szNo,_T("%d"),iIndex);

					LVItem.pszText=szNo;
					LVItem.iItem=iIndex-1;
					LVItem.iSubItem=0;

					LVItem.pszText=(LPTSTR)g_ConfigTmp.Command[iIndex].szCommandName;
					LVItem.iSubItem=1;
					ListView_SetItem(hListView,&LVItem);

					//他のタブのコマンド名更新
					SendMessage(hDlg,WM_SENDUPDATECOMMAND,0,0);
					break;
				}

				case IDC_BUTTON_CMDFILEPATH:{//実行ファイル
					TCHAR szFilePath[MAX_PATH]={};

					if(!OpenSingleFileDialog(hDlg,
											 szFilePath,
											 MAX_PATH,
											 _T("実行ファイル (*.*)\0*.*\0\0"),
											 _T("実行するファイルを選択してください"))){
						break;
					}
					SetWindowText(GetDlgItem(hDlg,IDC_EDIT_CMDFILEPATH),szFilePath);
					SetFocus(GetDlgItem(hDlg,IDC_EDIT_CMDFILEPATH));
					break;
				}

				case IDC_BUTTON_CMDPARAMETERS:{//「>」(引数)
					RECT rc;

					GetWindowRect(GetDlgItem(hDlg,LOWORD(wParam)),&rc);
					if(int iId=TrackPopupMenu(hParametersMenu,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_VERTICAL|TPM_RETURNCMD,rc.left,rc.bottom,0,hDlg,NULL)){
						TCHAR szString[128]={};
						MENUITEMINFO mi={sizeof(MENUITEMINFO)};

						mi.fMask=MIIM_STRING;
						mi.dwTypeData=szString;
						mi.cch=ARRAY_SIZEOF(szString);
						GetMenuItemInfo(hParametersMenu,iId,false,&mi);

						TCHAR szAddString[128];

						lstrcpy(szAddString,(StrChr(szString,'\t'))+1);

						SendDlgItemMessage(hDlg,
										   IDC_EDIT_CMDPARAMETERS,
										   EM_REPLACESEL,
										   1,
										   (LPARAM)szAddString);
						SetFocus(GetDlgItem(hDlg,IDC_EDIT_CMDPARAMETERS));
					}
					break;
				}

				case IDC_BUTTON_CMDWORKINGDIRECTORY:{//作業ディレクトリ
					TCHAR szWorkingDirectory[MAX_PATH]={};

					if(!SelectDirectory(hDlg,
										szWorkingDirectory,
										_T("作業ディレクトリを選択してください"))){
						
						break;
					}
					SetWindowText(GetDlgItem(hDlg,IDC_EDIT_CMDWORKINGDIRECTORY),szWorkingDirectory);
					SetFocus(GetDlgItem(hDlg,IDC_EDIT_CMDWORKINGDIRECTORY));
					break;
				}

				default:
					break;

			}

			switch(HIWORD(wParam)){
				case CBN_SELCHANGE://コンボボックスの選択内容が変更された
					switch(LOWORD(wParam)){
						case IDC_COMBO_CMDNO:{//コマンドNo.
							int iComboIndex=(int)SendDlgItemMessage(hDlg,LOWORD(wParam),CB_GETCURSEL,(WPARAM)0,(LPARAM)0);
							if(iComboIndex==CB_ERR)iComboIndex=0;

							//リストビューを選択
							ListView_SelectItem(hListView,iComboIndex);

							int iIndex=iComboIndex;

							iIndex++;

							//コマンド名
							SetWindowText(GetDlgItem(hDlg,IDC_EDIT_CMDNAME),g_ConfigTmp.Command[iIndex].szCommandName);
							//実行ファイル
							SetWindowText(GetDlgItem(hDlg,IDC_EDIT_CMDFILEPATH),g_ConfigTmp.Command[iIndex].szFilePath);
							//引数
							SetWindowText(GetDlgItem(hDlg,IDC_EDIT_CMDPARAMETERS),g_ConfigTmp.Command[iIndex].szParameters);
							//作業ディレクトリ
							SetWindowText(GetDlgItem(hDlg,IDC_EDIT_CMDWORKINGDIRECTORY),g_ConfigTmp.Command[iIndex].szWorkingDirectory);
							//コマンド種別
							SendDlgItemMessage(hDlg,IDC_COMBO_CMDMODE,CB_SETCURSEL,g_ConfigTmp.Command[iIndex].uCommandMode,0);
							//コマンド実行前の動作
							SendDlgItemMessage(hDlg,IDC_COMBO_CMDOPTION,CB_SETCURSEL,g_ConfigTmp.Command[iIndex].uCommandOption,0);
							for(UINT i=0;i<ARRAY_SIZEOF(CmdShow_Table);i++){
								//表示方法
								if(g_ConfigTmp.Command[iIndex].uCmdShow==CmdShow_Table[i].uCmdShow){
									SendDlgItemMessage(hDlg,IDC_COMBO_CMDSHOW,CB_SETCURSEL,i,0);
									break;
								}
							}
							break;
						}
						default:
							break;
					}
			}
			break;
		}

		case WM_NOTIFY:
			if(((LPNMHDR)lParam)->idFrom==IDC_LISTVIEW_COMMAND){
				switch(((LPNMLISTVIEW)lParam)->hdr.code){
					case LVN_ITEMCHANGED:{//アイテムを選択
						if(((LPNMLISTVIEW)lParam)->iItem==-1||
						   ((LPNMLISTVIEW)lParam)->uNewState!=(LVIS_SELECTED|LVIS_FOCUSED)){
							return 0;
						}

						int iIndex=((LPNMLISTVIEW)lParam)->iItem;

						if(iIndex!=-1){
							int iComboIndex=(int)SendDlgItemMessage(hDlg,IDC_COMBO_CMDNO,CB_GETCURSEL,(WPARAM)0,(LPARAM)0);

							if(iIndex==iComboIndex)break;
							SendDlgItemMessage(hDlg,IDC_COMBO_CMDNO,CB_SETCURSEL,iIndex,0);
							PostMessage(hDlg,WM_COMMAND,MAKEWPARAM(IDC_COMBO_CMDNO,CBN_SELCHANGE),(LPARAM)GetDlgItem(hDlg,IDC_COMBO_CMDNO));
						}
						break;
					}
					default:
						break;
				}
			}
			break;

		case WM_SENDUPDATECOMMAND:{
			//他のタブにコマンド名更新メッセージを送信
			TCHAR szClassName[128];
			HWND hChild=GetWindow(GetParent(hDlg),GW_CHILD);
			while(hChild){
				GetClassName(hChild,szClassName,ARRAY_SIZEOF(szClassName));
				if(hChild!=hDlg&&
				   lstrcmp(szClassName,_T("#32770"))==0){
					SendMessage(hChild,WM_UPDATECOMMAND,0,0);
				}
				hChild=GetWindow(hChild,GW_HWNDNEXT);
			}
			break;
		}

		case WM_CLOSETAB:
			DestroyMenu(hParametersMenu);
			return true;

		default:
			break;
	}

	return false;
}

//プレビューリストのプロシージャ
LRESULT CALLBACK PreviewListViewProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam){
	switch(uMsg){
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
			SetFocus(hWnd);
			return false;

		default:
			break;
	}
	return CallWindowProc((WNDPROC)GetWindowLongPtr(hWnd,GWLP_USERDATA),hWnd,uMsg,wParam,lParam);
}

//「デザイン」タブ
BOOL CALLBACK DesignTabProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam){
	static HWND hPreviewList;

	static TCHAR szColor[12];

	//色選択ダイアログ
	static CHOOSECOLOR ccTextColor={sizeof(CHOOSECOLOR)};
	static COLORREF clrCustColors[16];
	ccTextColor.hwndOwner=hDlg;
	ccTextColor.lpCustColors=clrCustColors;
	ccTextColor.Flags=CC_FULLOPEN|CC_RGBINIT;

	//スポイトアイコン
	static HICON hiconSpoit;

	//スポイトドラッグ中
	static bool bTextSpoitDrag;
	static bool bTextBkSpoitDrag;

	typedef struct{
		UINT uButtonId;
		HFONT* hFont;
		TCHAR* pszFont;
		int* piFontSize;
		int* piFontStyle;
		COLORREF* pclrText;
		COLORREF* pclrTextBk;
	}ITEMDESIGN_TABLE;

	static ITEMDESIGN_TABLE ItemDesign_Table[]={
		//デフォルト(通常行)
		{
			IDC_BUTTON_DEFAULT,
			&g_hDefaultItemFont,
			g_ConfigTmp.DefaultItemDesign.szFont,
			&g_ConfigTmp.DefaultItemDesign.iFontSize,
			&g_ConfigTmp.DefaultItemDesign.iFontStyle,
			&g_ConfigTmp.DefaultItemDesign.clrText,
			&g_ConfigTmp.DefaultItemDesign.clrTextBk,
		},

		//選択行
		{
			IDC_BUTTON_SELECTED,
			&g_hSelectedItemFont,
			g_ConfigTmp.SelectedItemDesign.szFont,
			&g_ConfigTmp.SelectedItemDesign.iFontSize,
			&g_ConfigTmp.SelectedItemDesign.iFontStyle,
			&g_ConfigTmp.SelectedItemDesign.clrText,
			&g_ConfigTmp.SelectedItemDesign.clrTextBk,
		},
	};

	static LPCTSTR lpszFontSize[]={
		g_szDefaultString,
		_T("8"),
		_T("9"),
		_T("10"),
		_T("11"),
		_T("12"),
		_T("14"),
		_T("16"),
		_T("18"),
		_T("20"),
		_T("22"),
		_T("24"),
		_T("26"),
		_T("28"),
		_T("30"),
	};

	typedef struct{
		UINT uCheckBoxId;
		int iStyle;
	}FONTSTYLE_TABLE;

	static FONTSTYLE_TABLE FontStyle_Table[]={
		{IDC_CHECKBOX_BOLD,SFONT_BOLD},//太字
		{IDC_CHECKBOX_ITALIC,SFONT_ITALIC},//斜体
		{IDC_CHECKBOX_UNDERLINE,SFONT_UNDERLINE},//下線
		{IDC_CHECKBOX_STRIKEOUT,SFONT_STRIKEOUT},//打ち消し線
	};

	static SPINEDIT_TABLE BackgroundOffset_Table[]={
		//X座標
		{
			IDC_SPIN_XOFFSET,
			IDC_EDIT_XOFFSET,
			&g_ConfigTmp.Background.iXOffset,
			//WM_INITDIALOGで代入
			0,
			0,
		},
		//Y座標
		{
			IDC_SPIN_YOFFSET,
			IDC_EDIT_YOFFSET,
			&g_ConfigTmp.Background.iYOffset,
			0,
			0,
		},
	};

	//Slider+Staticコントロール
	typedef struct{
		UINT uSliderId;
		UINT uStaticId;
		BYTE* pbyValue;
	}SLIDER_TABLE;

	static SLIDER_TABLE Slider_Table[]={
		//大きさ
		{
			IDC_SLIDER_RESIZEPERCENT,
			IDC_STATIC_RESIZEPERCENT,
			&g_ConfigTmp.Background.byResizePercent,
		},
		//不透明度
		{
			IDC_SLIDER_BG_ALPHA,
			IDC_STATIC_BG_ALPHA,
			&g_ConfigTmp.Background.byOpacity,
		},
	};

	switch(uMsg){
		case WM_INITDIALOG:{
			//アイコン読み込み
			hiconSpoit=(HICON)LoadImage(g_hInstance,MAKEINTRESOURCE(IDI_ICON_SPOIT),IMAGE_ICON,16,16,LR_SHARED);

			//アイコン設定
			SendMessage(GetDlgItem(hDlg,IDC_PICTURE_TEXTCOLOR),STM_SETIMAGE,IMAGE_ICON,(LPARAM)hiconSpoit);
			SendMessage(GetDlgItem(hDlg,IDC_PICTURE_TEXTBKCOLOR),STM_SETIMAGE,IMAGE_ICON,(LPARAM)hiconSpoit);

			//デフォルトボタンを押下する
			setCheck(hDlg,IDC_BUTTON_DEFAULT,true);

			//フォント名
			//フォントの列挙
			HDC hDC=GetDC(hDlg);
			LOGFONT lgFont={};
			lgFont.lfCharSet=DEFAULT_CHARSET;//全文字セット全フォントを列挙する
			//「 デフォルト」を追加
			SendDlgItemMessage(hDlg,IDC_COMBO_FONT,CB_ADDSTRING,(WPARAM)0,(LPARAM)g_szDefaultString);
			EnumFontFamiliesEx(hDC,&lgFont,(FONTENUMPROC)EnumFontsProc,(LPARAM)GetDlgItem(hDlg,IDC_COMBO_FONT),0);
			ReleaseDC(hDlg,hDC);

			//フォントサイズ
			for(UINT i=0;i<ARRAY_SIZEOF(lpszFontSize);i++){
				SendDlgItemMessage(hDlg,IDC_COMBO_FONTSIZE,CB_ADDSTRING,(WPARAM)0,(LPARAM)lpszFontSize[i]);
			}

			//エディットコントロールの最大文字数を設定
			Edit_LimitText(GetDlgItem(hDlg,IDC_EDIT_TEXTCOLOR),11);
			Edit_LimitText(GetDlgItem(hDlg,IDC_EDIT_TEXTBKCOLOR),11);

			//プレビューにアイテム追加
			hPreviewList=GetDlgItem(hDlg,IDC_LIST_PREVIEW);
			LVCOLUMN LVColumn;
			LVITEM LVItem;
			RECT rc;

			GetWindowRect(hPreviewList,&rc);
			//リストビューにカラムを追加
			LVColumn.mask=LVCF_FMT|LVCF_WIDTH|LVCF_TEXT;
			LVColumn.fmt=LVCFMT_LEFT;
			LVColumn.cx=rc.right-rc.left-10;
			LVColumn.pszText=NULL;
			ListView_InsertColumn(hPreviewList,0,&LVColumn);

			LVItem.mask=LVIF_TEXT;
			LVItem.pszText=(LPTSTR)_T("通常行");
			LVItem.iItem=0;
			LVItem.iSubItem=0;
			ListView_InsertItem(hPreviewList,&LVItem);

			LVItem.pszText=(LPTSTR)_T("選択行");
			LVItem.iItem=1;
			ListView_InsertItem(hPreviewList,&LVItem);

			//プレビューのフォントを設定
			for(UINT i=0;i<ARRAY_SIZEOF(ItemDesign_Table);i++){
				*ItemDesign_Table[i].hFont=CreateGUIFont(ItemDesign_Table[i].pszFont,
														*ItemDesign_Table[i].piFontSize,
														*ItemDesign_Table[i].piFontStyle);
			}

			SendMessage(hPreviewList,WM_SETFONT,(WPARAM)*ItemDesign_Table[0].hFont,MAKELPARAM(true,0));

			//サブクラス化
			SetWindowLongPtr(hPreviewList,GWLP_USERDATA,GetWindowLongPtr(hPreviewList,GWLP_WNDPROC));
			SetWindowLongPtr(hPreviewList,GWLP_WNDPROC,(LONG_PTR)PreviewListViewProc);

			//背景画像のオフセットテーブルに最小値最大値を設定
			BackgroundOffset_Table[0].iMinimum=-GetSystemMetrics(SM_CXSCREEN);
			BackgroundOffset_Table[0].iMaximum=GetSystemMetrics(SM_CXSCREEN);
			BackgroundOffset_Table[1].iMinimum=-GetSystemMetrics(SM_CYSCREEN);
			BackgroundOffset_Table[1].iMaximum=GetSystemMetrics(SM_CYSCREEN);


			//現在の設定をコントロールに適用
			SetCurrentSettings(hDlg,&g_ConfigTmp,TAB_DESIGN);

			return true;
		}

		case WM_CTLCOLORSTATIC:{//テキストの背景色を透過させる
			HWND hWnd=(HWND)lParam;
			if(hWnd==GetDlgItem(hDlg,IDC_COMBO_FONT)||hWnd==GetDlgItem(hDlg,IDC_COMBO_FONTSIZE))break;
			HDC hDC=(HDC)wParam;
			SetBkMode(hDC,TRANSPARENT);
			break;
		}

		case WM_LBUTTONDOWN:{
			//文字色のスポイト
			if(ControlFromPoint()==GetDlgItem(hDlg,IDC_PICTURE_TEXTCOLOR)){
				bTextSpoitDrag=true;
				SetCapture(hDlg);
				SetCursor(LoadCursor(NULL,IDC_CROSS));
			}

			//背景色のスポイト
			if(ControlFromPoint()==GetDlgItem(hDlg,IDC_PICTURE_TEXTBKCOLOR)){
				bTextBkSpoitDrag=true;
				SetCapture(hDlg);
				SetCursor(LoadCursor(NULL,IDC_CROSS));
			}
			break;
		}

		case WM_LBUTTONUP:{
			if(!bTextSpoitDrag&&!bTextBkSpoitDrag)break;
			POINT ptScreen;
			HDC hScreenDC;
			COLORREF clrScreen;

			GetCursorPos(&ptScreen);

			//カーソルがスポイトアイコン上にある場合。スポイト動作をキャンセル
			//文字色のスポイト
			if(ControlFromPoint()==GetDlgItem(hDlg,IDC_PICTURE_TEXTCOLOR)){
				bTextSpoitDrag=false;
				ReleaseCapture();
				SetCursor(LoadCursor(NULL,IDC_ARROW));
				break;
			}

			//背景色のスポイト
			if(ControlFromPoint()==GetDlgItem(hDlg,IDC_PICTURE_TEXTBKCOLOR)){
				bTextBkSpoitDrag=false;
				ReleaseCapture();
				SetCursor(LoadCursor(NULL,IDC_ARROW));
				break;
			}

			//スクリーンのデバイスコンテキストを取得
			hScreenDC=GetDC(NULL);
			clrScreen=GetPixel(hScreenDC,ptScreen.x,ptScreen.y);
			ReleaseDC(hDlg,hScreenDC);
			for(UINT i=0;i<ARRAY_SIZEOF(ItemDesign_Table);i++){
				if(SendDlgItemMessage(hDlg,ItemDesign_Table[i].uButtonId,BM_GETCHECK,(WPARAM)0,(LPARAM)0)){
					if(bTextSpoitDrag){
						*ItemDesign_Table[i].pclrText=clrScreen;
						COLORREFToHex(szColor,*ItemDesign_Table[i].pclrText);
						SetWindowText(GetDlgItem(hDlg,IDC_EDIT_TEXTCOLOR),szColor);

						bTextSpoitDrag=false;
					}
					if(bTextBkSpoitDrag){
						*ItemDesign_Table[i].pclrTextBk=clrScreen;
						COLORREFToHex(szColor,*ItemDesign_Table[i].pclrTextBk);
						SetWindowText(GetDlgItem(hDlg,IDC_EDIT_TEXTBKCOLOR),szColor);

						bTextBkSpoitDrag=false;
					}
				}
			}
			ReleaseCapture();
			SetCursor(LoadCursor(NULL,IDC_ARROW));
			break;
		}

		case WM_HSCROLL:{
			TCHAR szPosition[12]={};

			for(UINT i=0;i<ARRAY_SIZEOF(Slider_Table);i++){
				if((HWND)lParam==GetDlgItem(hDlg,Slider_Table[i].uSliderId)){
					*Slider_Table[i].pbyValue=(int)SendMessage(GetDlgItem(hDlg,Slider_Table[i].uSliderId),TBM_GETPOS,(WPARAM)0,(LPARAM)0);
					//数値表示
					wsprintf(szPosition,_T("%d%%"),*Slider_Table[i].pbyValue);
					SetWindowText(GetDlgItem(hDlg,Slider_Table[i].uStaticId),(LPCTSTR)szPosition);
					break;
				}
			}
			break;
		}

		case WM_COMMAND:{
			switch(HIWORD(wParam)){
				case EN_CHANGE:{//エディットボックスの内容が変更される直前
					switch(LOWORD(wParam)){
						case IDC_EDIT_TEXTCOLOR://文字色
							GetWindowText(GetDlgItem(hDlg,LOWORD(wParam)),szColor,sizeof(szColor));
							if(lstrlen(szColor)==6||lstrlen(szColor)==7||path::CountCharacter(szColor,_T(','))==2){
								for(UINT i=0;i<ARRAY_SIZEOF(ItemDesign_Table);i++){
									if(SendDlgItemMessage(hDlg,ItemDesign_Table[i].uButtonId,BM_GETCHECK,(WPARAM)0,(LPARAM)0)){
										*ItemDesign_Table[i].pclrText=StringToCOLORREF(szColor);
									}
								}
								PostMessage(hDlg,WM_COMMAND,MAKEWPARAM(IDC_COMBO_FONT,CBN_SELCHANGE),(LPARAM)0);
							}
							break;

						case IDC_EDIT_TEXTBKCOLOR://背景色
							GetWindowText(GetDlgItem(hDlg,LOWORD(wParam)),szColor,sizeof(szColor));
							if(lstrlen(szColor)==6||lstrlen(szColor)==7||path::CountCharacter(szColor,_T(','))==2){
								for(UINT i=0;i<ARRAY_SIZEOF(ItemDesign_Table);i++){
									if(SendDlgItemMessage(hDlg,ItemDesign_Table[i].uButtonId,BM_GETCHECK,(WPARAM)0,(LPARAM)0)){
										*ItemDesign_Table[i].pclrTextBk=StringToCOLORREF(szColor);
									}
								}
								PostMessage(hDlg,WM_COMMAND,MAKEWPARAM(IDC_COMBO_FONT,CBN_SELCHANGE),(LPARAM)0);
							}
							break;

						default:
							//背景画像のオフセット
							for(int i=0;i<static_cast<int>(ARRAY_SIZEOF(BackgroundOffset_Table));i++){
								if(LOWORD(wParam)==BackgroundOffset_Table[i].uEditId){
									*BackgroundOffset_Table[i].piValue=GetIntFromEdit(GetDlgItem(hDlg,BackgroundOffset_Table[i].uEditId),
																			   BackgroundOffset_Table[i].iMinimum,
																			   BackgroundOffset_Table[i].iMaximum);
								}
							}
							break;
					}
					break;
				}

				case CBN_SELCHANGE:{//コンボボックスの選択内容が変更された
					switch(LOWORD(wParam)){
						case IDC_COMBO_FONTSIZE:{//フォントサイズ
							TCHAR szSize[12]={};
							int iResult=0;

							//選択されたフォントサイズを取得
							int iIndex=(int)SendDlgItemMessage(hDlg,LOWORD(wParam),CB_GETCURSEL,(WPARAM)0,(LPARAM)0);
							if(iIndex==CB_ERR)iIndex=0;
							SendDlgItemMessage(hDlg,LOWORD(wParam),CB_GETLBTEXT,(WPARAM)iIndex,(LPARAM)szSize);
							if(lstrcmp(szSize,g_szDefaultString)==0)iResult=0;
							else iResult=StrToInt(szSize);

							for(UINT i=0;i<ARRAY_SIZEOF(ItemDesign_Table);i++){
								if(SendDlgItemMessage(hDlg,ItemDesign_Table[i].uButtonId,BM_GETCHECK,(WPARAM)0,(LPARAM)0)){
									*ItemDesign_Table[i].piFontSize=(INRANGE(MINIMUM_FONTSIZE,iResult,MAXIMUM_FONTSIZE))?iResult:MINIMUM_FONTSIZE;

									if(*ItemDesign_Table[i].hFont)DeleteObject(*ItemDesign_Table[i].hFont);
									*ItemDesign_Table[i].hFont=CreateGUIFont(ItemDesign_Table[i].pszFont,
																			*ItemDesign_Table[i].piFontSize,
																			*ItemDesign_Table[i].piFontStyle);
								}
							}

							SendMessage(hPreviewList,WM_SETFONT,(WPARAM)*ItemDesign_Table[0].hFont,MAKELPARAM(true,0));

							ListView_RedrawItems(hPreviewList,0,ARRAY_SIZEOF(ItemDesign_Table));
							break;
						}

						case IDC_COMBO_FONT:{
							TCHAR szFont[128]={};

							//選択されたフォントを取得
							int iIndex=(int)SendDlgItemMessage(hDlg,LOWORD(wParam),CB_GETCURSEL,(WPARAM)0,(LPARAM)0);
							if(iIndex==CB_ERR)iIndex=0;
							SendDlgItemMessage(hDlg,LOWORD(wParam),CB_GETLBTEXT,(WPARAM)iIndex,(LPARAM)szFont);

							for(UINT i=0;i<ARRAY_SIZEOF(ItemDesign_Table);i++){
								if(SendDlgItemMessage(hDlg,ItemDesign_Table[i].uButtonId,BM_GETCHECK,(WPARAM)0,(LPARAM)0)){
									if(lstrcmp(szFont,g_szDefaultString)==0)lstrcpy((ItemDesign_Table[i].pszFont),_T(""));
									else lstrcpy(ItemDesign_Table[i].pszFont,szFont);

									if(*ItemDesign_Table[i].hFont)DeleteObject(*ItemDesign_Table[i].hFont);
									*ItemDesign_Table[i].hFont=CreateGUIFont(ItemDesign_Table[i].pszFont,
																			*ItemDesign_Table[i].piFontSize,
																			*ItemDesign_Table[i].piFontStyle);
									break;
								}
							}

							ListView_RedrawItems(hPreviewList,0,ARRAY_SIZEOF(ItemDesign_Table));
							break;
						}
					}
				}

				default:
					break;
			}

			switch(LOWORD(wParam)){
				case IDC_CHECKBOX_BOLD://太字
				case IDC_CHECKBOX_ITALIC://斜体
				case IDC_CHECKBOX_UNDERLINE://下線
				case IDC_CHECKBOX_STRIKEOUT://打ち消し線
					for(UINT i=0;i<ARRAY_SIZEOF(ItemDesign_Table);i++){
						if(SendDlgItemMessage(hDlg,ItemDesign_Table[i].uButtonId,BM_GETCHECK,(WPARAM)0,(LPARAM)0)){
							for(UINT ii=0;ii<ARRAY_SIZEOF(FontStyle_Table);ii++){
								if(SendDlgItemMessage(hDlg,FontStyle_Table[ii].uCheckBoxId,BM_GETCHECK,(WPARAM)0,(LPARAM)0)){
									*ItemDesign_Table[i].piFontStyle|=FontStyle_Table[ii].iStyle;
								}else{
									*ItemDesign_Table[i].piFontStyle&=~FontStyle_Table[ii].iStyle;
								}
							}
							break;
						}
					}
					PostMessage(hDlg,WM_COMMAND,MAKEWPARAM(IDC_COMBO_FONT,CBN_SELCHANGE),(LPARAM)0);
					break;

				case IDC_BUTTON_DEFAULT://通常行
				case IDC_BUTTON_SELECTED://選択行
					//ボタン処理
					if(!getCheck(hDlg,LOWORD(wParam)))break;

					for(UINT i=0;i<ARRAY_SIZEOF(ItemDesign_Table);i++){
						if(LOWORD(wParam)==ItemDesign_Table[i].uButtonId){
							//フォント名
							int iIndex=(int)SendDlgItemMessage(hDlg,IDC_COMBO_FONT,CB_FINDSTRINGEXACT,(WPARAM)0,(LPARAM)ItemDesign_Table[i].pszFont);
							if(iIndex==CB_ERR)iIndex=(int)SendDlgItemMessage(hDlg,IDC_COMBO_FONT,CB_FINDSTRINGEXACT,(WPARAM)0,(LPARAM)g_szDefaultString);
							SendDlgItemMessage(hDlg,IDC_COMBO_FONT,CB_SETCURSEL,(WPARAM)iIndex,(LPARAM)0);

							//フォントサイズ
							TCHAR szTmp[12]={};
							wsprintf(szTmp,_T("%d"),*ItemDesign_Table[i].piFontSize);
							iIndex=(int)SendDlgItemMessage(hDlg,IDC_COMBO_FONTSIZE,CB_FINDSTRINGEXACT,(WPARAM)0,(LPARAM)szTmp);
							if(iIndex==CB_ERR)iIndex=(int)SendDlgItemMessage(hDlg,IDC_COMBO_FONTSIZE,CB_FINDSTRINGEXACT,(WPARAM)0,(LPARAM)g_szDefaultString);
							SendDlgItemMessage(hDlg,IDC_COMBO_FONTSIZE,CB_SETCURSEL,(WPARAM)iIndex,(LPARAM)0);

							//フォントスタイル
							for(UINT ii=0;ii<ARRAY_SIZEOF(FontStyle_Table);ii++){
								setCheck(hDlg,
										 FontStyle_Table[ii].uCheckBoxId,
										 *ItemDesign_Table[i].piFontStyle&FontStyle_Table[ii].iStyle
										 );
							}

							//文字色
							COLORREFToHex(szColor,*ItemDesign_Table[i].pclrText);
							SetWindowText(GetDlgItem(hDlg,IDC_EDIT_TEXTCOLOR),szColor);

							//背景色
							COLORREFToHex(szColor,*ItemDesign_Table[i].pclrTextBk);
							SetWindowText(GetDlgItem(hDlg,IDC_EDIT_TEXTBKCOLOR),szColor);

							break;
						}
					}
					break;

				case IDC_BUTTON_TEXTCOLOR://文字色選択ボタン
				case IDC_BUTTON_TEXTBKCOLOR://背景色選択ボタン
					for(UINT i=0;i<ARRAY_SIZEOF(ItemDesign_Table);i++){
						if(SendDlgItemMessage(hDlg,ItemDesign_Table[i].uButtonId,BM_GETCHECK,(WPARAM)0,(LPARAM)0)){
							if(LOWORD(wParam)==IDC_BUTTON_TEXTCOLOR){
								ccTextColor.rgbResult=*ItemDesign_Table[i].pclrText;
							}else if(LOWORD(wParam)==IDC_BUTTON_TEXTBKCOLOR){
								ccTextColor.rgbResult=*ItemDesign_Table[i].pclrTextBk;
							}

							if(ChooseColor(&ccTextColor)){
								if(LOWORD(wParam)==IDC_BUTTON_TEXTCOLOR){
									*ItemDesign_Table[i].pclrText=ccTextColor.rgbResult;
									COLORREFToHex(szColor,*ItemDesign_Table[i].pclrText);
									SetWindowText(GetDlgItem(hDlg,IDC_EDIT_TEXTCOLOR),szColor);
								}else if(LOWORD(wParam)==IDC_BUTTON_TEXTBKCOLOR){
									*ItemDesign_Table[i].pclrTextBk=ccTextColor.rgbResult;
									COLORREFToHex(szColor,*ItemDesign_Table[i].pclrTextBk);
									SetWindowText(GetDlgItem(hDlg,IDC_EDIT_TEXTBKCOLOR),szColor);
								}
							}
						}
					}
					break;

				case IDC_BUTTON_BACKGROUND:{//参照[背景画像]
					TCHAR szImagePath[MAX_PATH]={};

					if(!OpenSingleFileDialog(hDlg,
											 szImagePath,
											 MAX_PATH,
											 _T("背景画像 (*.*)\0*.*\0\0"),
											 _T("背景画像を選択してください"))){
						break;
					}
					SetWindowText(GetDlgItem(hDlg,IDC_EDIT_BACKGROUND),szImagePath);
					SetFocus(GetDlgItem(hDlg,IDC_EDIT_BACKGROUND));
					break;
				}

				default:
					break;
			}

			break;
		}

		case WM_NOTIFY:{
			switch(((LPNMHDR)lParam)->idFrom){
				case IDC_LIST_PREVIEW:{//フォントプレビュー
					switch(((LPNMHDR)lParam)->code){
						case NM_CUSTOMDRAW:{
							LPNMLVCUSTOMDRAW lpNMLVCD=(LPNMLVCUSTOMDRAW)lParam;

							if(lpNMLVCD->nmcd.dwDrawStage==CDDS_PREPAINT){
								SetWindowLongPtr(hDlg,DWLP_MSGRESULT,CDRF_NOTIFYITEMDRAW);
								return true;
							}
							if(lpNMLVCD->nmcd.dwDrawStage==CDDS_ITEMPREPAINT){
								if((int)lpNMLVCD->nmcd.dwItemSpec>=ListView_GetItemCount(((LPNMHDR)lParam)->hwndFrom)||lpNMLVCD->nmcd.dwItemSpec<0)break;

								lpNMLVCD->clrText=*ItemDesign_Table[lpNMLVCD->nmcd.dwItemSpec].pclrText;
								lpNMLVCD->clrTextBk=*ItemDesign_Table[lpNMLVCD->nmcd.dwItemSpec].pclrTextBk;
								SelectObject(lpNMLVCD->nmcd.hdc,*ItemDesign_Table[lpNMLVCD->nmcd.dwItemSpec].hFont);

								SetWindowLongPtr(hDlg,DWLP_MSGRESULT,CDRF_NEWFONT);
								return true;
							}else{
								SetWindowLongPtr(hDlg,DWLP_MSGRESULT,CDRF_DODEFAULT);
								return true;
							}
							break;
						}
					}
				}//case IDC_LIST_PREVIEW

				default:{
					//背景画像のオフセット
					for(int i=0;i<static_cast<int>(ARRAY_SIZEOF(BackgroundOffset_Table));i++){
						if(wParam==BackgroundOffset_Table[i].uSpinId){
							LPNMUPDOWN lpNMUpDown=(LPNMUPDOWN)lParam;
							if(lpNMUpDown->hdr.code==UDN_DELTAPOS){
								*BackgroundOffset_Table[i].piValue=GetIntFromEdit(GetDlgItem(hDlg,BackgroundOffset_Table[i].uEditId),
																				  BackgroundOffset_Table[i].iMinimum,
																				  BackgroundOffset_Table[i].iMaximum);
								if((lpNMUpDown->iDelta)>0&&*BackgroundOffset_Table[i].piValue<BackgroundOffset_Table[i].iMaximum){
									//上が押された
									*BackgroundOffset_Table[i].piValue+=1;
								}else if((lpNMUpDown->iDelta)<0&&*BackgroundOffset_Table[i].piValue>BackgroundOffset_Table[i].iMinimum){
									//下が押された
									*BackgroundOffset_Table[i].piValue-=1;
								}

								SetIntToEdit(GetDlgItem(hDlg,BackgroundOffset_Table[i].uEditId),*BackgroundOffset_Table[i].piValue);
							}
						}
					}
					break;
				}


			}//switch(((LPNMHDR)lParam)->idFrom)
			break;
		}//case WM_NOTIFY

		case WM_CLOSETAB:
			//背景画像
			GetWindowText(GetDlgItem(hDlg,IDC_EDIT_BACKGROUND),g_ConfigTmp.Background.szImagePath,MAX_PATH);
			return true;

		default:
			break;
	}
	return false;
}

//「高度な設定」タブ
BOOL CALLBACK AdvancedTabProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam){
	static LPCTSTR lpszThumbnail[]={
		_T("表示しない"),
		_T("1番目のカラム上に表示"),
		_T("2番目のカラム上に表示"),
		_T("画面外左側に表示"),
		_T("画面外右側に表示"),
	};

	static LPCTSTR lpszThumbnailVerticalAlign[]={
		_T("上揃えで表示"),
		_T("真ん中くらいに表示"),
		_T("下揃えで表示"),
		_T("選択項目に揃えて表示"),
	};

	static LPCTSTR lpszActiveMonitorMode[]={
		_T("カーソルのあるモニタ"),
		_T("アクティブなウインドウのあるモニタ"),
	};

	switch(uMsg){
		case WM_INITDIALOG:{
			//サムネイル表示位置コンボボックスの設定
			for(UINT i=0;i<ARRAY_SIZEOF(lpszThumbnail);i++){
				SendDlgItemMessage(hDlg,IDC_COMBO_THUMBNAIL,CB_ADDSTRING,(WPARAM)0,(LPARAM)lpszThumbnail[i]);
			}
			for(UINT i=0;i<ARRAY_SIZEOF(lpszThumbnailVerticalAlign);i++){
				SendDlgItemMessage(hDlg,IDC_COMBO_THUMBNAIL_VERTICALALIGN,CB_ADDSTRING,(WPARAM)0,(LPARAM)lpszThumbnailVerticalAlign[i]);
			}

			//マルチモニタコンボボックスの設定
			for(UINT i=0;i<ARRAY_SIZEOF(lpszActiveMonitorMode);i++){
				SendDlgItemMessage(hDlg,IDC_COMBO_MULTIMONITOR_MODE,CB_ADDSTRING,(WPARAM)0,(LPARAM)lpszActiveMonitorMode[i]);
			}

			MONITOR_INFO* pMonitorInfo=AllocMonitorsList();

			for(size_t i=0,iSize=HeapSize(GetProcessHeap(),0,pMonitorInfo)/sizeof(MONITOR_INFO);
				i<iSize;
				++i){
				SendDlgItemMessage(hDlg,IDC_COMBO_MULTIMONITOR_MODE,CB_ADDSTRING,(WPARAM)0,(LPARAM)pMonitorInfo[i].info.szDevice);
			}

			//現在の設定をコントロールに適用
			SetCurrentSettings(hDlg,&g_ConfigTmp,TAB_ADVANCED);
			return true;
		}

		case WM_CTLCOLORSTATIC:{//テキストの背景色を透過させる
			HDC hDC=(HDC)wParam;
			SetBkMode(hDC,TRANSPARENT);
			break;
		}

		case WM_SHOWWINDOW:
			//タイムアウト設定を同期
			if(wParam)SetCurrentSettings(hDlg,&g_ConfigTmp,TAB_ADVANCED);
			break;

		case WM_COMMAND:
			switch(LOWORD(wParam)){
				//[動作]
				case IDC_CHECKBOX_DRAG_MOUSEHOVER://ドラッグ中はマウスホバーで選択
					g_ConfigTmp.ListView.bDragMouseHover=getCheck(hDlg,LOWORD(wParam));
					break;

				case IDC_CHECKBOX_DRAG_TIMEOUT://ドラッグ中はタイムアウトで確定
					g_ConfigTmp.ListView.bDragTimeOut=getCheck(hDlg,LOWORD(wParam));
					break;

				case IDC_CHECKBOX_THUMBNAIL_ORIGINAL://元の大きさで表示
					g_ConfigTmp.ListView.bThumbnailOriginalSize=getCheck(hDlg,LOWORD(wParam));
					EnableWindow(GetDlgItem(hDlg,IDC_EDIT_THUMBNAIL_SIZE),!g_ConfigTmp.ListView.bThumbnailOriginalSize);
					break;

				case IDC_CHECKBOX_ONLY_ACTIVEMONITOR://アクティブモニタのウインドウのみ表示
					g_ConfigTmp.MultiMonitor.bOnlyActiveMonitor=getCheck(hDlg,LOWORD(wParam));
					break;

				case IDC_CHECKBOX_ONLY_ACTIVEVIRTUALDESKTOP://アクティブな仮想デスクトップのウインドウのみ表示
					g_ConfigTmp.VirtualDesktop.bOnlyActiveVirtualDesktop=getCheck(hDlg,LOWORD(wParam));
					break;

				default:
					break;
			}

			switch(HIWORD(wParam)){
				case CBN_SELCHANGE://コンボボックスの選択内容が変更された
					switch(LOWORD(wParam)){
						case IDC_COMBO_THUMBNAIL://表示位置
							g_ConfigTmp.ListView.uThumbnail=(UINT)SendDlgItemMessage(hDlg,LOWORD(wParam),CB_GETCURSEL,(WPARAM)0,(LPARAM)0);
							if(g_ConfigTmp.ListView.uThumbnail==CB_ERR)g_ConfigTmp.ListView.uThumbnail=THUMBNAIL_NO;
							EnableWindow(GetDlgItem(hDlg,IDC_COMBO_THUMBNAIL_VERTICALALIGN),g_ConfigTmp.ListView.uThumbnail!=THUMBNAIL_NO);
							EnableWindow(GetDlgItem(hDlg,IDC_EDIT_THUMBNAIL_SIZE),g_ConfigTmp.ListView.uThumbnail!=THUMBNAIL_NO);
							EnableWindow(GetDlgItem(hDlg,IDC_CHECKBOX_THUMBNAIL_ORIGINAL),g_ConfigTmp.ListView.uThumbnail!=THUMBNAIL_NO);
							EnableWindow(GetDlgItem(hDlg,IDC_EDIT_THUMBNAIL_DELAY),g_ConfigTmp.ListView.uThumbnail!=THUMBNAIL_NO);
							break;

						case IDC_COMBO_THUMBNAIL_VERTICALALIGN://表示位置(上揃え)
							g_ConfigTmp.ListView.uThumbnailVerticalAlign=(UINT)SendDlgItemMessage(hDlg,LOWORD(wParam),CB_GETCURSEL,(WPARAM)0,(LPARAM)0);
							if(g_ConfigTmp.ListView.uThumbnailVerticalAlign==CB_ERR)g_ConfigTmp.ListView.uThumbnailVerticalAlign=THUMBNAIL_VERTICALALIGN_TOP;
							break;

						case IDC_COMBO_MULTIMONITOR_MODE://ウインドウリストを表示するモニタ
							g_ConfigTmp.MultiMonitor.iActiveMonitorMode=(UINT)SendDlgItemMessage(hDlg,LOWORD(wParam),CB_GETCURSEL,(WPARAM)0,(LPARAM)0);
							if(g_ConfigTmp.MultiMonitor.iActiveMonitorMode==CB_ERR)g_ConfigTmp.MultiMonitor.iActiveMonitorMode=ACTIVEMONITOR_BY_CURSOR;
							if(g_ConfigTmp.MultiMonitor.iActiveMonitorMode>=ACTIVEMONITOR_BY_NAME){
								SendDlgItemMessage(hDlg,LOWORD(wParam),CB_GETLBTEXT,(WPARAM)g_ConfigTmp.MultiMonitor.iActiveMonitorMode,(LPARAM)g_ConfigTmp.MultiMonitor.szMonitorName);
								g_ConfigTmp.MultiMonitor.iActiveMonitorMode=ACTIVEMONITOR_BY_NAME;
							}
							break;

						default:
							break;
					}
					break;

				case EN_CHANGE://エディットボックスの内容が変更される直前
					switch(LOWORD(wParam)){
						case IDC_EDIT_TIMEOUT:
							//タイムアウト[動作]
							g_ConfigTmp.ListView.iTimeOut=GetIntFromEdit(GetDlgItem(hDlg,LOWORD(wParam)),0,10000);
							break;

						case IDC_EDIT_THUMBNAIL_SIZE:
							//サムネイルの大きさ[動作]
							g_ConfigTmp.ListView.iThumbnailSize=GetIntFromEdit(GetDlgItem(hDlg,LOWORD(wParam)),0,10000);
							break;

						case IDC_EDIT_THUMBNAIL_DELAY:
							//サムネイル表示までの時間[動作]
							g_ConfigTmp.ListView.iThumbnailDelay=GetIntFromEdit(GetDlgItem(hDlg,LOWORD(wParam)),0,10000);
							break;

						default:
							break;
					}
					break;

				default:
					break;
			}
			break;

		case WM_CLOSETAB:
			//除外するファイル名[フィルタ]
			if(lstrlen(g_ConfigTmp.Exclude.szFileName)&&
			   g_ConfigTmp.Exclude.szFileName[lstrlen(g_ConfigTmp.Exclude.szFileName)-1]!=';'){
				lstrcat(g_ConfigTmp.Exclude.szFileName,_T(";"));
			}
			GetWindowText(GetDlgItem(hDlg,IDC_EDIT_EXCLUDE_FILENAME),g_ConfigTmp.Exclude.szFileName,MAX_PATH);
			return true;

		default:
			break;
	}
	return false;
}

//ハイパーリンクのプロシージャ
LRESULT CALLBACK HyperLinkProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam){
	switch(uMsg){
		case WM_SETCURSOR:
			SetCursor(LoadCursor(NULL,IDC_HAND));
			return true;

		default:
			break;
	}
	return CallWindowProc((WNDPROC)GetWindowLongPtr(hWnd,GWLP_USERDATA),hWnd,uMsg,wParam,lParam);
}

//「バージョン情報」タブ
BOOL CALLBACK VersionTabProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam){
	static HICON hIcon;

	switch(uMsg){
		case WM_INITDIALOG:{
			//バージョン情報入力
			SetDlgItemText(hDlg,IDC_STATIC_VERSION,TASCHER_VERSION);

			//フォントを設定
			HWND hURL=GetDlgItem(hDlg,IDC_STATIC_URL);
			SetFont(hURL,NULL,0,SFONT_UNDERLINE);

			//アイコン読み込み
			hIcon=(HICON)LoadImage(g_hInstance,MAKEINTRESOURCE(IDI_ICON1),IMAGE_ICON,48,48,LR_SHARED);

			//アイコン設定
			SendMessage(GetDlgItem(hDlg,IDC_PICTURE_ICON),STM_SETIMAGE,IMAGE_ICON,(LPARAM)hIcon);

			//サブクラス化
			SetWindowLongPtr(hURL,GWLP_USERDATA,GetWindowLongPtr(hURL,GWLP_WNDPROC));
			SetWindowLongPtr(hURL,GWLP_WNDPROC,(LONG_PTR)HyperLinkProc);
			return true;
		}

		case WM_CTLCOLORSTATIC:{//テキストの背景色を透過させる
			HDC hDC=(HDC)wParam;
			SetBkMode(hDC,TRANSPARENT);
			break;
		}

		case WM_COMMAND:
			switch(LOWORD(wParam)){
				case IDC_STATIC_URL:
					if(HIWORD(wParam)==STN_CLICKED){
						//ホームページへ
						TCHAR szURL[256]={};

						GetWindowText(GetDlgItem(hDlg,IDC_STATIC_URL),szURL,ARRAY_SIZEOF(szURL));
						ShellExecute(NULL,_T("open"),szURL,NULL,NULL,SW_SHOWNORMAL);
						return true;
					}
					break;
				default:
					break;
			}
			break;

		case WM_CLOSETAB:
			return true;

		default:
			break;
	}
	return false;
}

//設定ダイアログのプロシージャ
BOOL CALLBACK SettingsDialogProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam){
	static HWND hTab;
	static HWND hTabGeneral;//「全般」タブ
	static HWND hTabShowWindow;//「表示方法」タブ
	static HWND hTabShortcutKey;//「ショートカットキー」タブ
	static HWND hTabMouse;//「マウス」タブ
	static HWND hTabCommand;//「コマンド」タブ
	static HWND hTabDesign;//「デザイン」タブ
	static HWND hTabAdvanced;//「高度な設定」タブ
	static HWND hTabVersion;//「バージョン情報」タブ
	static HHOOK hHook;

	typedef struct{
		HWND* hTab;
		UINT uItem;
	}TAB_TABLE;

	static TAB_TABLE Tab_Table[]={
		{
			&hTabGeneral,
			TAB_GENERAL
		},
		{
			&hTabShowWindow,
			TAB_SHOWWINDOW,
		},
		{
			&hTabShortcutKey,
			TAB_SHORTCUTKEY,
		},
		{
			&hTabMouse,
			TAB_MOUSE,
		},
		{
			&hTabCommand,
			TAB_COMMAND,
		},
		{
			&hTabDesign,
			TAB_DESIGN,
		},
		{
			&hTabAdvanced,
			TAB_ADVANCED,
		},
		{
			&hTabVersion,
			TAB_VERSION,
		},
	};

	switch (uMsg){
		case WM_INITDIALOG:{
			//設定一時保存
			g_ConfigTmp=g_Config;

			//画面中央に表示
			switch(g_Config.MultiMonitor.iActiveMonitorMode){
				case ACTIVEMONITOR_BY_CURSOR:
				SetCenterWindow(hDlg);
				break;
			case ACTIVEMONITOR_BY_WINDOW:
				SetCenterWindow(hDlg,GetForegroundWindow());
				break;
			case ACTIVEMONITOR_BY_NAME:
				SetCenterWindow(hDlg,g_Config.MultiMonitor.szMonitorName);
				break;
			}
			hTab=GetDlgItem(hDlg,IDC_TAB1);

			//タブ追加
			TC_ITEM TabItem;

			TabItem.mask=TCIF_TEXT;
			TabItem.pszText=(LPTSTR)_T("全般");
			TabCtrl_InsertItem(hTab,TAB_GENERAL,&TabItem);

			TabItem.pszText=(LPTSTR)_T("表示方法");
			TabCtrl_InsertItem(hTab,TAB_SHOWWINDOW,&TabItem);

			TabItem.pszText=(LPTSTR)_T("ショートカットキー");
			TabCtrl_InsertItem(hTab,TAB_SHORTCUTKEY,&TabItem);

			TabItem.pszText=(LPTSTR)_T("マウス");
			TabCtrl_InsertItem(hTab,TAB_MOUSE,&TabItem);

			TabItem.pszText=(LPTSTR)_T("コマンド");
			TabCtrl_InsertItem(hTab,TAB_COMMAND,&TabItem);

			TabItem.pszText=(LPTSTR)_T("デザイン");
			TabCtrl_InsertItem(hTab,TAB_DESIGN,&TabItem);

			TabItem.pszText=(LPTSTR)_T("高度な設定");
			TabCtrl_InsertItem(hTab,TAB_ADVANCED,&TabItem);

			TabItem.pszText=(LPTSTR)_T("バージョン情報");
			TabCtrl_InsertItem(hTab,TAB_VERSION,&TabItem);

			//タブ(ダイアログ)作成
			hTabGeneral=CreateDialog(g_hInstance,MAKEINTRESOURCE(IDD_GENERALTAB),hDlg,(DLGPROC)GeneralTabProc);
			hTabShowWindow=CreateDialog(g_hInstance,MAKEINTRESOURCE(IDD_SHOWWINDOWTAB),hDlg,(DLGPROC)ShowWindowTabProc);
			hTabShortcutKey=CreateDialog(g_hInstance,MAKEINTRESOURCE(IDD_SHORTCUTKEYTAB),hDlg,(DLGPROC)ShortcutKeyTabProc);
			hTabMouse=CreateDialog(g_hInstance,MAKEINTRESOURCE(IDD_MOUSETAB),hDlg,(DLGPROC)MouseTabProc);
			hTabCommand=CreateDialog(g_hInstance,MAKEINTRESOURCE(IDD_COMMANDTAB),hDlg,(DLGPROC)CommandTabProc);
			hTabDesign=CreateDialog(g_hInstance,MAKEINTRESOURCE(IDD_DESIGNTAB),hDlg,(DLGPROC)DesignTabProc);
			hTabAdvanced=CreateDialog(g_hInstance,MAKEINTRESOURCE(IDD_ADVANCEDTAB),hDlg,(DLGPROC)AdvancedTabProc);
			hTabVersion=CreateDialog(g_hInstance,MAKEINTRESOURCE(IDD_VERSIONTAB),hDlg,(DLGPROC)VersionTabProc);

			//タブの座標取得
			RECT rc;
			LPPOINT lpt=(LPPOINT)&rc;

			GetClientRect(hTab,&rc);
			TabCtrl_AdjustRect(hTab,false,&rc);
			//座標変換
			MapWindowPoints(hTab,hDlg,lpt,2);

			//タブの位置とサイズの調整
			HDWP hdwp=BeginDeferWindowPos(TAB_VERSION);
			for(int i=0;i<ARRAY_SIZEOF(Tab_Table);i++){
				hdwp=DeferWindowPos(hdwp,
									*Tab_Table[i].hTab,
									NULL,
									rc.left,
									rc.top,
									rc.right-rc.left,
									rc.bottom-rc.top,
									0);
			}
			EndDeferWindowPos(hdwp);

			//タブを表示
			for(int i=0;i<ARRAY_SIZEOF(Tab_Table);i++){
				if(g_iStartTab==Tab_Table[i].uItem){
					ShowWindow(*Tab_Table[i].hTab,SW_SHOW);
					TabCtrl_SetCurFocus(hTab,Tab_Table[i].uItem);
				}else{
					ShowWindow(*Tab_Table[i].hTab,SW_HIDE);
				}
			}

			//ホットキーコントロールでTabキーの入力を可能に
			hHook=SetWindowsHookEx(WH_KEYBOARD_LL,(HOOKPROC)LowLevelKeyboardHookProc,g_hInstance,0);

			SetForegroundWindowEx(hDlg);

			return true;
		}

		case WM_NOTIFY:{
			switch(((LPNMHDR)lParam)->idFrom){
				case IDC_TAB1:
					switch(((LPNMHDR)lParam)->code){
						case TCN_SELCHANGE:
							for(int i=0,iCurSel=TabCtrl_GetCurSel(hTab);i<ARRAY_SIZEOF(Tab_Table);i++){
								if(iCurSel==Tab_Table[i].uItem){
									//タブを切り替え
									ShowWindow(*Tab_Table[i].hTab,SW_SHOW);
								}else{
									ShowWindow(*Tab_Table[i].hTab,SW_HIDE);
								}
							}
							break;
						default:
							break;
					}
			}
			break;
		}

		case WM_COMMAND:
			switch(LOWORD(wParam)){
				case IDOK:
					if(GetDlgItem(hTabShortcutKey,IDC_HOTKEY_SHORTCUTKEY)==GetFocus()){
						//ホットキーコントロールにEnterキーを投げる
						return (BOOL)CallWindowProc(HotKeyProc,
													GetDlgItem(hTabShortcutKey,IDC_HOTKEY_SHORTCUTKEY),
													WM_KEYDOWN,VK_RETURN,0);
					}

					//タブを閉じる
					for(int i=0;i<ARRAY_SIZEOF(Tab_Table);i++){
						SendMessage(*Tab_Table[i].hTab,WM_CLOSETAB,(WPARAM)0,(LPARAM)0);
					}

					//設定を保存
					g_Config=g_ConfigTmp;

					//cfgファイルに書き込む
					WritePrivateProfile(GetParent(hDlg));

					EndDialog(hDlg,IDOK);
					return true;

				case IDCANCEL:
					if(GetDlgItem(hTabShortcutKey,IDC_HOTKEY_SHORTCUTKEY)==GetFocus()){
						//ホットキーコントロールにEscキーを投げる
						return (BOOL)CallWindowProc(HotKeyProc,
													GetDlgItem(hTabShortcutKey,IDC_HOTKEY_SHORTCUTKEY),
													WM_KEYDOWN,VK_ESCAPE,0);
					}
					//fall through
				default:
					EndDialog(hDlg,IDCANCEL);
					return true;
			}
			break;

		case WM_CLOSE:
			UnhookWindowsHookEx(hHook);
			PostMessage(hDlg,WM_COMMAND,(WPARAM)IDCANCEL,(LPARAM)0);
			return true;

		default:
			return false;
	}
	return false;
}

//設定ウインドウを表示する
INT_PTR ShowSettingsDialog(HWND hWnd,int iStartTab){
	INT_PTR iResult=IDCANCEL;
	g_bSettingsDialog=true;
	g_iStartTab=iStartTab;
	iResult=DialogBox(g_hInstance,MAKEINTRESOURCE(IDD_SETTINGSDIALOG),NULL/*hWnd*/,(DLGPROC)SettingsDialogProc);
	g_bSettingsDialog=false;
	return iResult;
}
