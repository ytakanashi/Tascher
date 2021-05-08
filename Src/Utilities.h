//Utilities.h
//様々な便利関数

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.64
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/


#ifndef TASCHER_UTILITIES_H
#define TASCHER_UTILITIES_H

#include"CommonSettings.h"
#include<tlhelp32.h>
#include<d2d1.h>


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

struct MONITOR_INFO{
	HMONITOR hMonitor;
	RECT rc;
	MONITORINFOEX info;
};

//SetCursorCorner()
enum SCC_CORNERS{
	SCC_NONE=0,
	SCC_LEFTTOP=1,//左上
	SCC_TOP,//上
	SCC_RIGHTTOP,//右上
	SCC_LEFT,//左
	SCC_RIGHT,//右
	SCC_LEFTBOTTOM,//左下
	SCC_BOTTOM,//下
	SCC_RIGHTBOTTOM,//右下
	SCC_CENTER,//中央
};

//CreateGUIFont()
enum SFONT_TYPE{
	SFONT_BOLD=1<<0,
	SFONT_ITALIC=1<<1,
	SFONT_UNDERLINE=1<<2,
	SFONT_STRIKEOUT=1<<3
};


inline bool IsSpace(TCHAR c){return c==' '||c=='\t'||c=='\n'||c=='v'||c=='f'||c=='r';}

//マウス入力の監視を開始
bool RegisterRawInput(HWND hWnd);

//マウス入力の監視を終了
bool UnregisterRawInput();

//文字列を置換
bool StrReplace(TCHAR* pszStr,const TCHAR* pszTarget,const TCHAR* pszReplacement);

//一つのファイルを選択
bool OpenSingleFileDialog(HWND hWnd,TCHAR* pszResult,int iLength,const TCHAR* pszFilter,const TCHAR* pszTitle);

//ディレクトリを選択
bool SelectDirectory(HWND hWnd,TCHAR* pResult,const TCHAR* pszTitle=NULL,const TCHAR* pszDefaultDirectory=NULL);

//画面中央にウインドウを表示
bool SetCenterWindow(HWND hWnd);
bool SetCenterWindow(HWND hWnd,HWND hForegroundWnd);
bool SetCenterWindow(HWND hWnd,LPCTSTR lpszMonitorName);

//カーソル移動先の四隅の座標を取得
bool GetCursorCornerPos(POINT* ppt,SCC_CORNERS eCorner,RECT* prc,int iMarginHorizontal=20,int iMarginVertical=20);

//カーソルを四隅に移動
bool SetCursorCorner(HWND hWnd,SCC_CORNERS eCorner,int iMarginHorizontal=20,int iMarginVertical=20);

bool GetDpiForMonitor(HMONITOR hMonitor,UINT*pdpiX,UINT*pdpiY);

//モニタ名からHMONITOR取得
HMONITOR MonitorFromName(LPCTSTR lpszMonitorName);

//モニタリストを取得
MONITOR_INFO* AllocMonitorsList();

//モニタリストを解放
void FreeMonitorsList(MONITOR_INFO** ppMonitorInfo);

//モニタのDCを作成(DeleteDC()で削除すること)
HDC CreateMonitorDC(HMONITOR hMonitor);

//指定したモニタにウインドウを移動
bool MoveToMonitor(HWND hWnd,HMONITOR hDestMonitor);

//画面中央にウインドウを表示(マルチモニタの場合カーソルがあるモニタ)
bool SetCenterWindow(HWND hWnd);

//画面中央にウインドウを表示(マルチモニタの場合指定したウインドウがあるモニタ)
bool SetCenterWindow(HWND hWnd,HWND hForegroundWnd);

//画面中央にウインドウを表示(マルチモニタの場合指定した名前のモニタ)
bool SetCenterWindow(HWND hWnd,LPCTSTR lpszMonitorName);

//カーソルをウインドウの中心に移動
bool SetCenterCursor(HWND hWnd=NULL);

//現在のカーソルの位置からコントロールのハンドルを取得
HWND ControlFromPoint();

//ウインドウがフルスクリーンかどうか
bool IsFullScreenWindow(HWND hWnd);

//ウインドウが現在の仮想デスクトップ上に存在するか
bool IsWindowOnCurrentVirtualDesktop(HWND hWnd);

//ウインドウリストを取得
HWND* AllocWindowListFromApplicationView();

//ウインドウリストを解放
void FreeWindowListFromApplicationView(HWND** pphWnd);

//ApplicationFrameWindowからWindows.UI.Core.CoreWindowのハンドルを取得
HWND GetCoreWindow(HWND hWnd);

//ショートカットを作成
bool CreateShortcut(LPCTSTR lpszShortcutFile,LPCTSTR lpszTargetFile,LPCTSTR lpszArgs,LPCTSTR lpszDescription,LPCTSTR lpszWorkingDirectory);

//スタートアップ登録ファイルの有効/無効設定する
bool EnableStartupApproved(LPCTSTR lpszShortcutFile,bool bEnable);

//スタートアップ登録ファイルの有効/無効設定を削除
bool RemoveStartupApproved(LPCTSTR lpszShortcutFile);

//スピン+エディットコントロールの初期設定
bool InitializeSpinEditControl(HWND hSpin,HWND hEdit,int iMinimum,int iMaximum,int iCurrent);

//エディットボックスから数値を取得
int GetIntFromEdit(HWND hWnd,const int iMinimum,const int iMaximum);

//数値を文字列に変換してエディットボックスに入力
bool SetIntToEdit(HWND hEdit,const int iNumber);

//エディットボックスから数値を取得
int GetIntFromEdit(HWND hWnd,const int iMinimum,const int iMaximum);

//16進数またはRGB表記の文字列をCOLORREF型に変換
COLORREF StringToCOLORREF(TCHAR* pszString);

//COLORREF型を16進数表記の文字列に変換
bool COLORREFToHex(TCHAR* pszResult,COLORREF color);

//COLORREF型をRGB表記の文字列に変換
//bool COLORREFToRGB(TCHAR* pszResult,COLORREF color);

//メッセージボックスのLOGFONTを取得
LOGFONT GetMesssageLOGFONT();

//標準のGUIフォントを作成
HFONT CreateGUIFont(LPCTSTR lpszFontName=NULL,int iSize=0,int iStyle=0);

//論理フォントをコントロールに適用
void SetFont(HWND hWnd,LPCTSTR lpszFontName=NULL,int iSize=0,int iStyle=0);

//子孫ウインドウへのSendMessage()
void SendMessageToDescendants(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam,bool bDeep=true);

//ホットキー登録
bool RegistHotKey(HWND hWnd,int iItemId,WORD* pwHotKey);

//ホットキー登録解除
bool UnregistHotKey(HWND hWnd,int iItemId,WORD* pwHotKey);

//ウインドウをフォアグラウンドに持ってくる
bool SetForegroundWindowEx(HWND hWnd);

//最前面ウインドウかどうか
bool IsWindowTopMost(HWND hWNd);

//キーの名前を取得
bool GetKeyName(WORD wKey,BOOL bExtended,TCHAR* pszKey,UINT uBufferSize);

//ホットキーを文字列に変換
void GetHotKeyName(WORD wKey,TCHAR* pszKey,UINT uBufferSize);

//デスクトップを表示する
void ToggleDesktop();

//文字列をクリップボードにコピー
bool SetClipboardText(HWND hWnd,const TCHAR* pszText,int iLength);

//アクセラレータテーブルを更新
HACCEL UpdateAcceleratorTable(HACCEL hAccel,SKEY sKeyTable[]);

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
void FreeProcessList(PROCESSENTRY32** ppProcessEntry32);

//特定のプロセスのコマンドラインを取得
LPTSTR AllocProcessCommandLine(DWORD dwProcessId);

//AllocProcessCommandLine()で取得したコマンドラインを解放
void FreeProcessCommandLine(TCHAR** ppszCommandLine);

//管理者権限で実行中かどうか
bool IsAdministratorProcess(DWORD dwProcessId);

//プロセスからApplicationUserModelIdを取得
bool GetAppUserModelIdFromProcess(DWORD dwProcessId,LPTSTR pszAppUserModelId,UINT uiBufferLength);

//ウインドウからApplicationUserModelIdを取得
bool GetAppUserModelIdFromWindow(HWND hWnd,LPTSTR pszAppUserModelId,UINT uiBufferLength);

//キー入力を登録する
void SetKeybrd(LPINPUT lpKey,WORD wVKey,bool KeyDown);

//キー入力を行う
void ExecKey(WORD wKey,WORD wVKey);

//マウスアウトを検出する
void MouseLeaveEvent(HWND hWnd);

//デスクトップアイコンを取得する
HICON GetDesktopIcon(bool bSmall);

bool GetUWPAppIconD2D1(ID2D1RenderTarget* pRenderTarget,HWND hWnd,ID2D1Bitmap** ppResultBitmap,bool* pbClrBackground=NULL,COLORREF* pclrBackground=NULL);

//実行ファイルからアイコンを取得
HICON GetIcon(HWND hWnd,UINT uIconSize,bool bUWPApp,bool bUWPAppNoBackgroundColor);

//あの手この手でアイコンを取得[小さい]
HICON GetSmallIconFromWindow(HWND hWnd);

//あの手この手でアイコンを取得[大きい]
HICON GetLargeIconFromWindow(HWND hWnd);

//降格ShellExecute()
bool ShellExecuteNonElevated(PCTSTR lpszFile,PCTSTR lpszParameters,PCTSTR lpszWorkingDirectory,int nShowCmd);

//ウインドウハンドルからファイル名を取得する
bool GetFileNameFromWindowHandle(HWND hWnd,LPTSTR lpFileName,DWORD dwFileNameLength);

//ドラッグ中かどうか
bool IsDragging();

//カーソルがウインドウ内にあるか
bool IsCursorInWindow(HWND hWnd);

//OSバージョンを取得
bool GetWindowsVersion(PRTL_OSVERSIONINFOW info);

//グラデーション描画(水平方向)
void HorizontalGradient(HDC hDC,const RECT& GradientFill,COLORREF rgbTop,COLORREF rgbBottom);
//グラデーション描画(垂直方向)
//void VerticalGradient(HDC hDC,const RECT& GradientFill,COLORREF rgbTop,COLORREF rgbBottom);

#endif //TASCHER_UTILITIES_H
