//Funciton.h
//様々な便利関数

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.60
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/


#ifndef FUNCTION_H
#define FUNCTION_H

#include<gdiplus.h>


//数値が範囲内にあるかどうか
#define INRANGE(Low,Num,High) (((Low)<=(Num))&&((Num)<=(High)))


//Spin+Editコントロール
struct SPINEDIT_TABLE{
	UINT uSpinId;
	UINT uEditId;
	int* piValue;
	int iMinimum;
	int iMaximum;
};

//SetCornerCursor()
enum SCC_CORNERS{
	SCC_NONE=0,
	SCC_LEFTTOP=1,//左上
	SCC_RIGHTTOP,//右上
	SCC_LEFTBOTTOM,//左下
	SCC_RIGHTBOTTOM,//右下
	SCC_CENTER,//中央
};
//CreateGUIFont()
typedef enum{
	SFONT_BOLD=1<<0,
	SFONT_ITALIC=1<<1,
	SFONT_UNDERLINE=1<<2,
	SFONT_STRIKEOUT=1<<3
}SFONT_TYPE;


//一つのファイルを選択
bool OpenSingleFileDialog(HWND hWnd,TCHAR* pszResult,int iLength,const TCHAR* pszFilter,const TCHAR* pszTitle);

//画面中央にウインドウを表示
bool SetCenterWindow(HWND hWnd);

//カーソルを四隅に移動
bool SetCornerCursor(HWND hWnd,SCC_CORNERS scwCorner);

//カーソルをウインドウの中心に移動
bool SetCenterCursor(HWND hWnd=NULL);

//現在のカーソルの位置からコントロールのハンドルを取得
HWND ControlFromPoint();

//ショートカットを作成
bool CreateShortcut(LPCTSTR lpszShortcutFile,LPCTSTR lpszTargetFile,LPCTSTR lpszArgs,LPCTSTR lpszDescription,LPCTSTR lpszWorkingDirectory);

//スピン+エディットコントロールの初期設定
bool InitializeSpinEditControl(HWND hSpin,HWND hEdit,int iMinimum,int iMaximum,int iCurrent);

//エディットボックスから数値を取得
int GetIntFromEdit(HWND hWnd,const int iMinimum,const int iMaximum);

//数値を文字列に変換してエディットボックスに入力
bool SetIntToEdit(HWND hEdit,const int iNumber);

//数字の桁数を取得
int GetIntLength(int iNumber);

//エディットボックスから数値を取得
int GetIntFromEdit(HWND hWnd,const int iMinimum,const int iMaximum);

//16進数またはRGB表記の文字列をCOLORREF型に変換
COLORREF StringToCOLORREF(TCHAR* pszString);

//COLORREF型を16進数表記の文字列に変換
bool COLORREFToHex(TCHAR* pszResult,COLORREF color);

//COLORREF型をRGB表記の文字列に変換
bool COLORREFToRGB(TCHAR* pszResult,COLORREF color);

//標準のGUIフォントを作成
HFONT CreateGUIFont(LPCTSTR lpszFontName=NULL,int iSize=0,int iStyle=0);

//論理フォントをコントロールに適用
void SetFont(HWND hWnd,LPCTSTR lpszFontName=NULL,int iSize=0,int iStyle=0);

//不動明度を設定
bool _SetLayeredWindowAttributes(HWND hWnd,COLORREF crKey,BYTE byAlpha,DWORD dwFlags);

//ホットキー登録
bool RegistHotKey(HWND hWnd,int iItemId,WORD& pwHotkey);

//ホットキー登録解除
bool UnregistHotKey(HWND hWnd,int iItemId,WORD& pwHotkey);

#if 0
//ウインドウは応答なしである
bool IsWindowHung(HWND hWnd);
#endif

//ウインドウをフォアグラウンドに持ってくる
bool SetForegroundWindowEx(HWND hWnd);

//最前面ウインドウかどうか
bool IsWindowTopMost(HWND hWNd);

//画像を読み込みHBITMAPを返す
HBITMAP ReadPicture(TCHAR*pszFile);

//Gdiplus::Bitmap*を解放
void FreeBitmap(Gdiplus::Bitmap*& pBitmap);

//キーの名前を取得
bool GetKeyName(WORD wKey,BOOL bExtended,TCHAR* pszKey,UINT uBufferSize);

//ホットキーを文字列に変換
void GetHotKeyName(WORD wKey,TCHAR* pszKey,UINT uBufferSize);

#if 0
//デスクトップを表示する
void ToggleDesktop();
#endif

//文字列をクリップボードにコピー
bool SetClipboardText(HWND hWnd,const TCHAR* pszText,int iLength);

#endif //FUNCTION_H
