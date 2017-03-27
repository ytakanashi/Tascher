//Function.cpp
//様々な便利関数

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.62
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/


#include"StdAfx.h"
#include"Function.h"
#include"CommonSettings.h"
#include"Path.h"
#include"resources/resource.h"
#include<objidl.h>
#include<olectl.h>
#include<shlobj.h>
#include<shlwapi.h>
#include<psapi.h>
#include<wbemidl.h>

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

typedef struct PACKAGE_VERSION{
	union{
		UINT64 Version;
		struct{
			USHORT Revision;
			USHORT Build;
			USHORT Minor;
			USHORT Major;
		}DUMMYSTRUCTNAME;
	}DUMMYUNIONNAME;
}PACKAGE_VERSION;

typedef struct PACKAGE_ID{
	UINT32          reserved;
	UINT32          processorArchitecture;
	PACKAGE_VERSION version;
	PWSTR           name;
	PWSTR           publisher;
	PWSTR           resourceId;
	PWSTR           publisherId;
}PACKAGE_ID;



//一つのファイルを選択
bool OpenSingleFileDialog(HWND hWnd,TCHAR* pszResult,int iLength,const TCHAR* pszFilter,const TCHAR* pszTitle){
	TCHAR szDlgErr[128]={};
	OPENFILENAME ofnOpen={sizeof(OPENFILENAME)};

	ofnOpen.hwndOwner=hWnd;
	ofnOpen.lpstrFilter=pszFilter;
	ofnOpen.lpstrFile=pszResult;
	ofnOpen.nMaxFile=iLength;
	ofnOpen.lpstrTitle=pszTitle;
	ofnOpen.Flags=OFN_EXPLORER|OFN_HIDEREADONLY|OFN_FILEMUSTEXIST;
	if(!GetOpenFileName(&ofnOpen)){
		DWORD dwDlgErr=CommDlgExtendedError();

		if(dwDlgErr!=0){
			wsprintf(szDlgErr,_T("ファイルの選択に失敗しました。\nErrCode:0x%08X"),dwDlgErr);
			MessageBox(hWnd,szDlgErr,NULL,MB_ICONWARNING);
			pszResult[0]=_T('\0');
		}
		return false;
	}
	return true;
}

//画面中央にウインドウを表示
bool SetCenterWindow(HWND hWnd){
	RECT rc;
	GetWindowRect(hWnd,&rc);

	return SetWindowPos(hWnd,
				NULL,
				(GetSystemMetrics(SM_CXSCREEN)-(rc.right-rc.left))>>1,
				(GetSystemMetrics(SM_CYSCREEN)-(rc.bottom-rc.top))>>1,
				-1,
				-1,
				SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE
	)!=0;
}

//カーソルを四隅に移動
bool SetCornerCursor(HWND hWnd,SCC_CORNERS scwCorner,int iMargin){
	RECT rc;
	POINT pt;

	GetWindowRect(hWnd,&rc);

	switch(scwCorner){
		case SCC_LEFTTOP:
			//左上
			pt.x=rc.left+iMargin;
			pt.y=rc.top+iMargin;
			break;

		case SCC_TOP:
			//上
			pt.x=rc.left+((rc.right-rc.left)>>1);
			pt.y=rc.top+iMargin;
			break;

		case SCC_RIGHTTOP:
			//右上
			pt.x=rc.right-iMargin;
			pt.y=rc.top+iMargin;
			break;

		case SCC_LEFT:
			//左
			pt.x=rc.left+iMargin;
			pt.y=rc.top+((rc.bottom-rc.top)>>1);
			break;

		case SCC_RIGHT:
			//右
			pt.x=rc.right-iMargin;
			pt.y=rc.top+((rc.bottom-rc.top)>>1);
			break;

		case SCC_LEFTBOTTOM:
			//左下
			pt.x=rc.left+iMargin;
			pt.y=rc.bottom-iMargin;
			break;

		case SCC_BOTTOM:
			//下
			pt.x=rc.left+((rc.right-rc.left)>>1);
			pt.y=rc.bottom-iMargin;
			break;

		case SCC_RIGHTBOTTOM:
			//右下
			pt.x=rc.right-iMargin;
			pt.y=rc.bottom-iMargin;
			break;

		case SCC_CENTER:
		default:
			//中央
			pt.x=rc.left+((rc.right-rc.left)>>1);
			pt.y=rc.top+((rc.bottom-rc.top)>>1);
			break;
	}
	return SetCursorPos(pt.x,pt.y)!=0;
}

//カーソルをウインドウの中心に移動
bool SetCenterCursor(HWND hWnd){
	POINT pt;
	RECT rc;

	if(hWnd!=NULL){
		//ウインドウの中心座標を取得
		GetClientRect(hWnd,&rc);
		pt.x=(rc.right-rc.left)>>1;
		pt.y=(rc.bottom-rc.top)>>1;
		ClientToScreen(hWnd,&pt);
	}else{
		//ウインドウハンドルがNULLならば、マウスカーソルを画面中央へ移動
		//ウインドウの中心座標を取得
		pt.x=(GetSystemMetrics(SM_CXSCREEN)>>1);
		pt.y=(GetSystemMetrics(SM_CYSCREEN)>>1);
	}
	return SetCursorPos(pt.x,pt.y)!=0;
}

//現在のカーソルの位置からコントロールのハンドルを取得
HWND ControlFromPoint(){
	POINT pt;

	GetCursorPos(&pt);
	HWND hResult=WindowFromPoint(pt);

	if(hResult!=NULL){
		POINT ptClient=pt;

		ScreenToClient(hResult,&ptClient);
		{
			HWND hChild=ChildWindowFromPoint(hResult,ptClient);
			if(hChild!=NULL&&IsWindowVisible(hChild)){
				hResult=hChild;
			}
		}
	}

	DWORD dwWnd=MAXWORD;

	HWND hParent=GetParent(hResult);
	if(hParent!=NULL){
		//ポップアップウインドウではない
		if((GetWindowLongPtr(hResult,GWL_STYLE)&WS_POPUP)==0){
			//子ウインドウ取得
			HWND hChild=GetWindow(hParent,GW_CHILD);

			while(hChild!=NULL){
				//非表示の子ウインドウを無視
				if(IsWindowVisible(hChild)){
					RECT rc;

					GetWindowRect(hChild,&rc);
					if(PtInRect(&rc,pt)!=0){
						DWORD dwWndChild=(rc.right-rc.left)*(rc.bottom-rc.top);

						if(dwWndChild<dwWnd){
							dwWnd=dwWndChild;
							hResult=hChild;
						}
					}
				}
				hChild=GetWindow(hChild,GW_HWNDNEXT);
			}
		}
	}
	return hResult;
}

//ショートカットを作成
//C++ではなく、Cの場合メンバlpVtblを介さなければならない
//又、型REFCLSIDの宣言が異なる為、CoCreateInstance()の引数にも注意
//例えば、C++の場合はCLSID_ShellLink
//Cの場合は&CLSID_ShellLink
bool CreateShortcut(LPCTSTR lpszShortcutFile,LPCTSTR lpszTargetFile,LPCTSTR lpszArgs,LPCTSTR lpszDescription,LPCTSTR lpszWorkingDirectory){
	bool bResult=false;

	//COMの初期化を行う
	CoInitialize(NULL);
	//シェルリンク用インターフェイス
	IShellLink *pShellLink=NULL;
	//ファイル保存用インターフェイス
	IPersistFile *pPersistFile=NULL;

	//IShellLinkのポインタを取得する
	if(SUCCEEDED(CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLink,(LPVOID*)&pShellLink))){
		//IPersistFileのポインタを取得する
		if(SUCCEEDED(pShellLink->QueryInterface(IID_IPersistFile,(LPVOID*)&pPersistFile))){
			//リンクパス
			if(SUCCEEDED(pShellLink->SetPath(lpszTargetFile))){
				//実行時の引数
				if(lpszArgs){
					pShellLink->SetArguments(lpszArgs);
				}
				//説明文
				if(lpszDescription){
					pShellLink->SetDescription(lpszDescription);
				}
				//作業ディレクトリ
				if(lpszWorkingDirectory){
					pShellLink->SetWorkingDirectory(lpszWorkingDirectory);
				}

#ifdef _UNICODE
				//ショートカットファイルの保存
				if(SUCCEEDED(pPersistFile->Save(lpszShortcutFile,true)))bResult=true;
#else
				WCHAR szShortcutFileW[MAX_PATH];
				if(MultiByteToWideChar(CP_ACP,0,lpszShortcutFile,-1,szShortcutFileW,MAX_PATH)>0){
					//ショートカットファイルの保存
					if(SUCCEEDED(pPersistFile->Save(szShortcutFileW,true)))bResult=true;
				}
#endif //_UNICODE

			}
			pPersistFile->Release();
		}
		pShellLink->Release();
	}
	//COMを解放
	CoUninitialize();
	return bResult;
}

//スピン+エディットコントロールの初期設定
bool InitializeSpinEditControl(HWND hSpin,HWND hEdit,int iMinimum,int iMaximum,int iCurrent){
	if(hSpin==NULL||hEdit==NULL)return false;

	//エディットコントロールを関連付ける
	SendMessage(hSpin,UDM_SETBUDDY,(WPARAM)hEdit,(LPARAM)0);
	//最大値と最小値を設定
	SendMessage(hSpin,UDM_SETRANGE32,(WPARAM)iMinimum,(LPARAM)iMaximum);
	//現在サイズの位置に移動
	SendMessage(hSpin,UDM_SETPOS,(WPARAM)0,(LPARAM)MAKELONG((short)iCurrent,0));
	return true;
}

//エディットボックスから数値を取得
int GetIntFromEdit(HWND hWnd,const int iMinimum,const int iMaximum){
	TCHAR szNumber[256]={};
	int iResult=0;

	GetWindowText(hWnd,szNumber,ARRAY_SIZEOF(szNumber));
	iResult=StrToInt(szNumber);
	//最小値
	if(iResult<iMinimum)iResult=iMinimum;
	//最大値
	if(iResult>iMaximum)iResult=iMaximum;
	return iResult;
}

//数値を文字列に変換してエディットボックスに入力
bool SetIntToEdit(HWND hEdit,const int iNumber){
	TCHAR szNumber[256]={};

	wsprintf(szNumber,_T("%d"),iNumber);
	SetWindowText(hEdit,szNumber);
	return true;
}

//数字の桁数を取得
int GetIntLength(int iNumber){
	int iResult=0;

	for(;iNumber!=0;iResult++)iNumber/=10;
	return iResult;
}

//16進数またはRGB表記の文字列をCOLORREF型に変換
COLORREF StringToCOLORREF(TCHAR* pszString){
	if(((pszString[0]=='#'&&lstrlen(pszString)==7)||
		(lstrlen(pszString)==6&&StrChr(pszString,',')==NULL))){
		//16進数値
		int iResult=0;
		TCHAR szHex[12]=_T("0x");
		lstrcat(szHex,(pszString[0]==_T('#'))?pszString+1:pszString);
		StrToIntEx(szHex,STIF_SUPPORT_HEX,&iResult);
		return RGB(GetBValue(iResult),GetGValue(iResult),GetRValue(iResult));
	}else if((pszString[0]!='#'&&path::CountCharacter(pszString,_T(','))==2)){
		//RGB値
		int iR,iG,iB;
		iR=StrToInt(pszString);//r,g,bからrを取り出す
		pszString+=StrChr(pszString,',')-pszString+1;//rとgの間の','+1に移動
		iG=StrToInt(pszString);//g,bからgを取り出す
		pszString+=StrChr(pszString,',')-pszString+1;//gとbの間の','+1に移動
		iB=StrToInt(pszString);//bからbを取り出す

		return RGB(iR,iG,iB);
	}
	return (COLORREF)StrToInt(pszString);
}

//COLORREF型を16進数表記の文字列に変換
bool COLORREFToHex(TCHAR* pszResult,COLORREF color){
	wsprintf(pszResult,_T("#%02X%02X%02X"),GetRValue(color),GetGValue(color),GetBValue(color));
	return true;
}

//COLORREF型をRGB表記の文字列に変換
bool COLORREFToRGB(TCHAR* pszResult,COLORREF color){
	wsprintf(pszResult,_T("%d,%d,%d"),GetRValue(color),GetGValue(color),GetBValue(color));
	return true;
}

//標準のGUIフォントを作成
HFONT CreateGUIFont(LPCTSTR lpszFontName,int iSize,int iStyle){
	NONCLIENTMETRICS ncMetrics={sizeof(NONCLIENTMETRICS)};
	LOGFONT lgFont={};

	SystemParametersInfo(SPI_GETNONCLIENTMETRICS,sizeof(NONCLIENTMETRICS),&ncMetrics,0);
	lgFont=ncMetrics.lfMessageFont;

	if(lstrlen(lpszFontName))lstrcpy(lgFont.lfFaceName,lpszFontName);
	if(iSize>0)lgFont.lfHeight=iSize;
	if(iStyle&SFONT_BOLD)lgFont.lfWeight=FW_BOLD;
	if(iStyle&SFONT_ITALIC)lgFont.lfItalic=true;
	if(iStyle&SFONT_UNDERLINE)lgFont.lfUnderline=true;
	if(iStyle&SFONT_STRIKEOUT)lgFont.lfStrikeOut=true;

	return CreateFontIndirect(&lgFont);
}

//論理フォントをコントロールに適用
void SetFont(HWND hWnd,LPCTSTR lpszFontName,int iSize,int iStyle){
	HFONT hFont;
	HDC hDC=GetDC(hWnd);

	hFont=CreateGUIFont(lpszFontName,iSize,iStyle);
	SetBkMode(hDC,OPAQUE);
	SendMessage(hWnd,WM_SETFONT,(WPARAM)hFont,true);
	ReleaseDC(hWnd,hDC);
	return;
}

//ホットキー登録
bool RegistHotKey(HWND hWnd,int iItemId,WORD& pwHotkey){
	bool bResult=false;

	if(pwHotkey){
		UINT uModifiers=0,uVirtKey=0;
		WORD wModify=(WORD)(pwHotkey>>8);

		if(wModify&HOTKEYF_SHIFT){
			uModifiers|=MOD_SHIFT;
		}
		if(wModify&HOTKEYF_ALT){
			uModifiers|=MOD_ALT;
		}
		if(wModify&HOTKEYF_CONTROL){
			uModifiers|=MOD_CONTROL;
		}
		uVirtKey=(UINT)LOBYTE(pwHotkey);

		if(RegisterHotKey(hWnd,iItemId,uModifiers,uVirtKey)){
			bResult=true;
		}else{
			//ホットキー登録に失敗
			MessageBox(hWnd,_T("ホットキーの登録に失敗しました"),NULL,MB_OK|MB_ICONERROR);
			pwHotkey=0;
			bResult=false;
		}
	}
	return bResult;
}

//ホットキー登録解除
bool UnregistHotKey(HWND hWnd,int iItemId,WORD& pwHotkey){
	if(pwHotkey){
		return UnregisterHotKey(hWnd,iItemId)!=0;
	}
	return false;
}

//ウインドウをフォアグラウンドに持ってくる
bool SetForegroundWindowEx(HWND hWnd){
	bool bResult=false;
	bool bHung=IsHungAppWindow(hWnd)!=0;
	DWORD dwCurrentThreadId=0,dwTargetThreadId=0;
	DWORD dwTimeout=0;

	dwCurrentThreadId=GetCurrentThreadId();
	dwTargetThreadId=GetWindowThreadProcessId(hWnd,NULL);

	if(IsIconic(hWnd)){
		//最小化されている場合まず元に戻す
		//管理者権限で動作しているウインドウに対してはWM_SYSCOMMANDでないと処理されない?
//		ShowWindow(hWnd,SW_RESTORE);
		SendMessage(hWnd,WM_SYSCOMMAND,SC_RESTORE,0);
	}

	if(!bHung){
		for(int i=0;i<10&&hWnd!=GetForegroundWindow();i++){
			//あの手この手
			dwCurrentThreadId=GetCurrentThreadId();
			dwTargetThreadId=GetWindowThreadProcessId(GetForegroundWindow(),NULL);
			AttachThreadInput(dwCurrentThreadId,dwTargetThreadId,true);
			SetWindowPos(hWnd,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
			BringWindowToTop(hWnd);
			AllowSetForegroundWindow(ASFW_ANY);
			bResult=SetForegroundWindow(hWnd)!=0;
			AttachThreadInput(dwCurrentThreadId,dwTargetThreadId,false);
			Sleep(10);
		}
	}else{
		//対象のウインドウが応答なしの場合、AttachThreadInput()とSetWindowPos()を実行すると巻き添えに
		BringWindowToTop(hWnd);
		bResult=SetForegroundWindow(hWnd)!=0;
	}
	return bResult;
}

//最前面ウインドウかどうか
bool IsWindowTopMost(HWND hWnd){
	return (GetWindowLongPtr(hWnd,GWL_EXSTYLE)&WS_EX_TOPMOST)!=0;
}

#if BACKGROUND_NO_GDIPLUS
//画像を読み込みHBITMAPを返す
HBITMAP ReadPicture(TCHAR*pszFile){
	if(pszFile==NULL)return NULL;

	HBITMAP hBitmap=NULL;
	Gdiplus::Bitmap bBitmap(pszFile);

	bBitmap.GetHBITMAP(0,&hBitmap);
	return hBitmap;
#if 0
	if(pszFile==NULL)return NULL;

	HBITMAP hBitmap=NULL;
	HANDLE hFile=NULL;
	HGLOBAL hGlobal=NULL;
	DWORD dwSize=0,dwRead=0;

	hFile=CreateFile(pszFile,GENERIC_READ,0,0,OPEN_EXISTING,0,0);
	if(hFile==INVALID_HANDLE_VALUE)return NULL;
	dwSize=GetFileSize(hFile,0);
	hGlobal=GlobalAlloc(GHND,dwSize);
	if(hGlobal==0){
		CloseHandle(hFile);
		return NULL;
	}
	if(!ReadFile(hFile,GlobalLock(hGlobal),dwSize,&dwRead,0)){
		CloseHandle(hFile);
		GlobalFree(hGlobal);
		return NULL;
	}
	CloseHandle(hFile);
	GlobalUnlock(hGlobal);

	IPicture* pPicture;
	IStream* pStream;

	CreateStreamOnHGlobal(hGlobal,true,&pStream);
	if(OleLoadPicture(pStream,dwSize,0,IID_IPicture,(void**)&pPicture)==S_OK){
		pPicture->get_Handle((OLE_HANDLE*)&hBitmap);
		hBitmap=(HBITMAP)CopyImage(hBitmap,IMAGE_BITMAP,0,0,0);
	}
	if(pStream)pStream->Release();
	if(pPicture)pPicture->Release();
	GlobalFree(hGlobal);
	return hBitmap;
#endif
}
#endif

//Gdiplus::Bitmap*を解放
void FreeBitmap(Gdiplus::Bitmap*& pBitmap){
	if(pBitmap==NULL)return;
	HBITMAP hBitmap=NULL;

	pBitmap->GetHBITMAP(0,&hBitmap);
	DeleteObject(hBitmap);
	delete pBitmap;
	pBitmap=NULL;
}

//キーの名前を取得
bool GetKeyName(WORD wKey,BOOL bExtended,TCHAR* pszKey,UINT uBufferSize){
	LONG lScanCode=MapVirtualKey(wKey,0)<<16;

	if(bExtended)lScanCode|=0x01000000L;

	return GetKeyNameText(lScanCode,pszKey,uBufferSize)!=0;
}

//ホットキーを文字列に変換
void GetHotKeyName(WORD wKey,TCHAR* pszKey,UINT uBufferSize){
	TCHAR szKey[12]={};

	if(HIBYTE(wKey)&HOTKEYF_CONTROL){
		GetKeyName(VK_CONTROL,false,szKey,ARRAY_SIZEOF(szKey));
		StrNCat(pszKey,szKey,uBufferSize);
		StrNCat(pszKey,_T(" + "),uBufferSize);
	}
	if(HIBYTE(wKey)&HOTKEYF_SHIFT){
		GetKeyName(VK_SHIFT,false,szKey,ARRAY_SIZEOF(szKey));
		StrNCat(pszKey,szKey,uBufferSize);
		StrNCat(pszKey,_T(" + "),uBufferSize);
	}
	if(HIBYTE(wKey)&HOTKEYF_ALT){
		GetKeyName(VK_MENU,false,szKey,ARRAY_SIZEOF(szKey));
		StrNCat(pszKey,szKey,uBufferSize);
		StrNCat(pszKey,_T(" + "),uBufferSize);
	}
	GetKeyName(LOBYTE(wKey),HIBYTE(wKey)&HOTKEYF_EXT,szKey,ARRAY_SIZEOF(szKey));
	StrNCat(pszKey,szKey,uBufferSize);
	return;
}

#if 0
//デスクトップを表示する
void ToggleDesktop(){
	CoInitialize(NULL);
	IShellDispatch4* pShellDispatch=NULL;
	if(S_OK==CoCreateInstance(CLSID_Shell,NULL,CLSCTX_SERVER,IID_IDispatch,(LPVOID*)&pShellDispatch)){
		pShellDispatch->ToggleDesktop();
	}
	pShellDispatch->Release();
	CoUninitialize();
#if 0
	//419=MIN_ALL
	PostMessage(FindWindow(_T("Shell_TrayWnd"),NULL),WM_COMMAND,419,0);
#endif
}
#endif

//文字列をクリップボードにコピー
bool SetClipboardText(HWND hWnd,const TCHAR* pszText,int iLength){
	bool bResult=false;

	if(!OpenClipboard(hWnd))return bResult;

	HGLOBAL hGlobal=GlobalAlloc(GMEM_MOVEABLE,(iLength+1)*sizeof(TCHAR));
	if(hGlobal!=NULL){
		lstrcpy((TCHAR*)GlobalLock(hGlobal),pszText);
		GlobalUnlock(hGlobal);
		EmptyClipboard();
		bResult=SetClipboardData(CF_UNICODETEXT,hGlobal)!=NULL;
	}
	CloseClipboard();
	return bResult;
}

//アクセラレータテーブルを更新
HACCEL UpdateAcceleratorTable(HACCEL hAccel,SKEY sKeyTable[]){
	ACCEL aKeyTable[MAX_SHORTCUTKEY]={};

	for(int i=0;i<MAX_SHORTCUTKEY;++i){
		if(!sKeyTable[i].wKey)break;

		ACCEL aKey={FVIRTKEY,0,0};

		if(HIBYTE(sKeyTable[i].wKey)&HOTKEYF_ALT){
			aKey.fVirt|=FALT;
		}
		if(HIBYTE(sKeyTable[i].wKey)&HOTKEYF_CONTROL){
			aKey.fVirt|=FCONTROL;
		}
		if(HIBYTE(sKeyTable[i].wKey)&HOTKEYF_SHIFT){
			aKey.fVirt|=FSHIFT;
		}
		aKey.key=LOBYTE(sKeyTable[i].wKey);
		aKey.cmd=sKeyTable[i].uCmdId;
		aKeyTable[i]=aKey;
	}

	if(hAccel!=NULL)DestroyAcceleratorTable(hAccel);
	hAccel=CreateAcceleratorTable(aKeyTable,MAX_SHORTCUTKEY);
	return hAccel;
}

//プロセスのファイル名を取得
bool GetProcessFileName(DWORD dwProcessId,LPTSTR pszFileName,DWORD dwFileNameLength){
	bool bResult=false;
	HANDLE hProcess=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,false,dwProcessId);

	static BOOL(WINAPI*pQueryFullProcessImageName)(HANDLE,DWORD,LPTSTR,PDWORD);

	if(!pQueryFullProcessImageName)pQueryFullProcessImageName=(BOOL(WINAPI*)(HANDLE,DWORD,LPTSTR,PDWORD))
		GetProcAddress(GetModuleHandle(_T("kernel32")),
#ifdef UNICODE
					   "QueryFullProcessImageNameW"
#else
					   "QueryFullProcessImageNameA"
#endif
					   );
	if(pQueryFullProcessImageName!=NULL){
		bResult=pQueryFullProcessImageName(hProcess,0,pszFileName,&dwFileNameLength)!=0;
	}
	if(!bResult){
		bResult=GetProcessImageFileName(hProcess,pszFileName,dwFileNameLength)!=0;
	}
	CloseHandle(hProcess);
	return bResult;
}

//プロセスを強制終了
bool TerminateProcess(DWORD dwProcessId){
	bool bResult=false;
	HANDLE hProcess=OpenProcess(PROCESS_TERMINATE,false,dwProcessId);

	if(hProcess!=NULL)bResult=TerminateProcess(hProcess,0)!=0;
	CloseHandle(hProcess);
	return bResult;
}

//プロセスの優先度を取得
DWORD GetPriorityClass(DWORD dwProcessId){
	DWORD dwResult=0;

	HANDLE hProcess=OpenProcess(PROCESS_QUERY_INFORMATION,false,dwProcessId);

	if(hProcess!=NULL)dwResult=GetPriorityClass(hProcess);
	CloseHandle(hProcess);
	return dwResult;
}

//プロセスの優先度を設定
bool SetPriorityClass(DWORD dwProcessId,DWORD dwPriorityClass){
	bool bResult=false;

	HANDLE hProcess=OpenProcess(PROCESS_SET_INFORMATION,false,dwProcessId);

	if(hProcess!=NULL)bResult=SetPriorityClass(hProcess,dwPriorityClass)!=0;
	CloseHandle(hProcess);
	return bResult;
}

//プロセスツリーを強制終了
void TerminateProcessTree(PROCESSENTRY32* pProcessEntry32,DWORD dwParentProcessId){
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

			//プロセス終了
			TerminateProcessTree(pProcessEntry32,pProcessEntry32[i].th32ProcessID);
			TerminateProcess(pProcessEntry32[i].th32ProcessID);
		}
	}
	TerminateProcess(dwParentProcessId);
	return;
}

//プロセスリストを作成
PROCESSENTRY32* AllocProcessList(){
	PROCESSENTRY32* pProcessEntry32=NULL;
	int iNumber=0;
	HANDLE hSnapshot=NULL;
	PROCESSENTRY32 ProcessEntry32={sizeof(PROCESSENTRY32)};

	if(INVALID_HANDLE_VALUE!=(hSnapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0))){
		if(Process32First(hSnapshot,&ProcessEntry32)){
			do{
				PROCESSENTRY32* pTmp=NULL;
				if(iNumber==0)pTmp=(PROCESSENTRY32*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(++iNumber)*sizeof(PROCESSENTRY32));
				else pTmp=(PROCESSENTRY32*)HeapReAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,pProcessEntry32,(++iNumber)*sizeof(PROCESSENTRY32));
				if(pTmp==NULL)break;
				pProcessEntry32=pTmp;
				pProcessEntry32[iNumber-1]=ProcessEntry32;
			}while(Process32Next(hSnapshot,&ProcessEntry32));
		}

		CloseHandle(hSnapshot);
	}
	return pProcessEntry32;
}

//プロセスリストを解放
void FreeProcessList(PROCESSENTRY32* pProcessEntry32){
	HeapFree(GetProcessHeap(),0,pProcessEntry32);
	pProcessEntry32=NULL;
}

//特定のプロセスのコマンドラインを取得
LPTSTR AllocProcessCommandLine(DWORD dwProcessId){
	TCHAR* pszCommandLine=NULL;

	CoInitializeEx(0,COINIT_MULTITHREADED);

	if(SUCCEEDED(CoInitializeSecurity(NULL,-1,NULL,NULL,RPC_C_AUTHN_LEVEL_DEFAULT,RPC_C_IMP_LEVEL_IMPERSONATE,NULL,EOAC_NONE,NULL))){
		IWbemLocator* pWbemLocator=NULL;

		if(SUCCEEDED(CoCreateInstance(CLSID_WbemLocator,0,CLSCTX_INPROC_SERVER,IID_IWbemLocator,(LPVOID*)&pWbemLocator))){
			IWbemServices* pWbemServices=NULL;

			if(SUCCEEDED(pWbemLocator->ConnectServer(_T("ROOT\\CIMV2"),NULL,NULL,NULL,0,NULL,NULL,&pWbemServices))){
				if(SUCCEEDED(CoSetProxyBlanket(pWbemServices,RPC_C_AUTHN_WINNT,RPC_C_AUTHZ_NONE,NULL,RPC_C_AUTHN_LEVEL_CALL,RPC_C_IMP_LEVEL_IMPERSONATE,NULL,EOAC_NONE))){
					IEnumWbemClassObject* pEnum=NULL;
					TCHAR szQuery[128];

					wsprintf(szQuery,_T("SELECT ProcessId,CommandLine FROM Win32_Process WHERE ProcessID = %d"),dwProcessId);
					if(SUCCEEDED(pWbemServices->ExecQuery(_T("WQL"),szQuery,WBEM_FLAG_FORWARD_ONLY,NULL,&pEnum))){
						IWbemClassObject* result=NULL;
						ULONG uReturned=0;

						if(SUCCEEDED(pEnum->Next(WBEM_INFINITE,1,&result,&uReturned))){
							VARIANT CommandLine;

							if(SUCCEEDED(result->Get(_T("CommandLine"),0,&CommandLine,0,0))&&
							   !(CommandLine.vt==VT_NULL)){
								int iLength=SysStringLen(CommandLine.bstrVal);
								pszCommandLine=(TCHAR*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(iLength+1)*sizeof(TCHAR));
								lstrcpy(pszCommandLine,CommandLine.bstrVal);
							}
							result->Release();
						}
						pEnum->Release();
					}
					pWbemServices->Release();
				}
			}
			pWbemLocator->Release();
		}
	}

	CoUninitialize();
	return pszCommandLine;
}

//AllocProcessCommandLine()で取得したコマンドラインを解放
void FreeProcessCommandLine(TCHAR* pszCommandLine){
	HeapFree(GetProcessHeap(),0,pszCommandLine);
	pszCommandLine=NULL;
}

//キー入力を登録する
void SetKeybrd(LPINPUT lpKey,WORD wVKey,bool KeyDown){
	lpKey->type=INPUT_KEYBOARD;
	lpKey->ki.wVk=wVKey;
	lpKey->ki.wScan=(WORD)MapVirtualKey(wVKey,0);
	lpKey->ki.dwFlags=(KeyDown?0:KEYEVENTF_KEYUP);
	lpKey->ki.dwExtraInfo=0;
	lpKey->ki.time=0;
}

//キー入力を行う
void ExecKey(WORD wKey,WORD wVKey){
	INPUT Key[6];
	LPINPUT lpKey=Key;
	SetKeybrd(lpKey++,wKey,false);
	if(wVKey)SetKeybrd(lpKey++,wVKey,false);
	if(wVKey)SetKeybrd(lpKey++,wVKey,true);
	SetKeybrd(lpKey++,wKey,true);
	SetKeybrd(lpKey++,wKey,false);
	if(wVKey)SetKeybrd(lpKey++,wVKey,false);
	SendInput((UINT)(lpKey-Key),Key,sizeof(INPUT));
	return;
}

//マウスアウトを検出する
void MouseLeaveEvent(HWND hWnd){
	TRACKMOUSEEVENT tme={sizeof(TRACKMOUSEEVENT),TME_LEAVE,hWnd};

	TrackMouseEvent(&tme);
	return;
}

//デスクトップアイコンを取得する
HICON GetDesktopIcon(bool bSmall){
	HICON hIconLarge=NULL,hIconSmall=NULL;
	LPITEMIDLIST lvItemList=NULL;
	IShellFolder *pIShellFolder=NULL;
	IExtractIcon *pIExtractIcon=NULL;
	IMalloc *pMalloc;
	SHGetMalloc(&pMalloc);

	CoInitialize(NULL);
	if(SUCCEEDED(SHGetSpecialFolderLocation(NULL,CSIDL_DESKTOP,&lvItemList))){
		SHGetDesktopFolder(&pIShellFolder);
		if(SUCCEEDED(pIShellFolder->GetUIObjectOf(NULL,1,(LPCITEMIDLIST*)&lvItemList,IID_IExtractIcon,0,(void**)&pIExtractIcon))){
			int iIndex=0;
			UINT uFlags;
			TCHAR szLocation[MAX_PATH]={};

			if(SUCCEEDED(pIExtractIcon->GetIconLocation(GIL_FORSHELL,szLocation,MAX_PATH,&iIndex,&uFlags))){
				pIExtractIcon->Extract(szLocation,iIndex,&hIconLarge,&hIconSmall,MAKELONG(32,16));
			}
		}
		pIExtractIcon->Release();
		pIShellFolder->Release();
		pMalloc->Free(lvItemList);
	}
	pMalloc->Release();
	CoUninitialize();
	return (!bSmall)?hIconLarge:hIconSmall;
}

#import <msxml6.dll> raw_interfaces_only

HRESULT VariantFromString(PCWSTR wszValue,VARIANT& Variant){
	HRESULT hResult=S_OK;
	BSTR bstr=SysAllocString(wszValue);

	if(bstr){
		V_VT(&Variant)=VT_BSTR;
		V_BSTR(&Variant)=bstr;
	}
	else{
		hResult=E_OUTOFMEMORY;
	}
	return hResult;
}

HRESULT CreateAndInitDOM(IXMLDOMDocument** ppXMLDocument){
	HRESULT hResult=CoCreateInstance(__uuidof(MSXML2::DOMDocument60),NULL,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(ppXMLDocument));
	if(SUCCEEDED(hResult)){
		(*ppXMLDocument)->put_async(VARIANT_FALSE);
		(*ppXMLDocument)->put_validateOnParse(VARIANT_FALSE);
		(*ppXMLDocument)->put_resolveExternals(VARIANT_FALSE);
	}
	return hResult;
}

//UWPAppのアイコンを取得する
HICON GetUWPAppIcon(HWND hWnd,UINT uiIconSize=32){
	HICON hIcon=NULL;
	DWORD dwProcessId=0;
	IXMLDOMDocument* pXMLDocument=NULL;
	TCHAR* pszUWPAppPath=NULL;

	GetWindowThreadProcessId(hWnd,&dwProcessId);
	HANDLE hProcess=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,dwProcessId);

	if(hProcess==NULL)return hIcon;

	PTSTR pszFullPackageName=NULL;
	PTSTR pszPackagePath=NULL;
	BYTE* byPackageId=NULL;
	UINT32 uiFullPackageNameLength=0,uiPackagePathLength=0,uiPackageIdLength=0;

	static LONG(WINAPI*pGetPackageFullName)(HANDLE,UINT32*,PWSTR);
	static LONG(WINAPI*pGetPackagePathByFullName)(PCWSTR,UINT32*,PWSTR);
	static LONG(WINAPI*pPackageIdFromFullName)(PCWSTR,const UINT32,UINT32*,BYTE*);

	if(!pGetPackageFullName)pGetPackageFullName=(LONG(WINAPI*)(HANDLE,UINT32*,PWSTR))
		GetProcAddress(GetModuleHandle(_T("kernel32")),"GetPackageFullName");
	if(!pGetPackagePathByFullName)pGetPackagePathByFullName=(LONG(WINAPI*)(PCWSTR,UINT32*,PWSTR))
		GetProcAddress(GetModuleHandle(_T("kernel32")),"GetPackagePathByFullName");
	if(!pPackageIdFromFullName)pPackageIdFromFullName=(LONG(WINAPI*)(PCWSTR,const UINT32,UINT32*,BYTE*))
		GetProcAddress(GetModuleHandle(_T("kernel32")),"PackageIdFromFullName");


	if(pGetPackageFullName==NULL||pGetPackagePathByFullName==NULL||pPackageIdFromFullName==NULL){
		CloseHandle(hProcess);
		return hIcon;
	}
	pGetPackageFullName(hProcess,&uiFullPackageNameLength,NULL);
	pszFullPackageName=(PTSTR)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,uiFullPackageNameLength*sizeof(TCHAR));
	pGetPackageFullName(hProcess,&uiFullPackageNameLength,pszFullPackageName);
	if(pszFullPackageName==NULL){
		CloseHandle(hProcess);
		return hIcon;
	}

	pGetPackagePathByFullName(pszFullPackageName,&uiPackagePathLength,NULL);
	pszPackagePath=(PTSTR)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,uiPackagePathLength*sizeof(TCHAR));
	pGetPackagePathByFullName(pszFullPackageName,&uiPackagePathLength,pszPackagePath);
	pPackageIdFromFullName(pszFullPackageName,0,&uiPackageIdLength,NULL);
	byPackageId=(BYTE*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,uiPackageIdLength);
	pPackageIdFromFullName(pszFullPackageName,0,&uiPackageIdLength,byPackageId);

	CoInitialize(NULL);

	if(pszPackagePath&&byPackageId&&
		SUCCEEDED(CreateAndInitDOM(&pXMLDocument))){
		TCHAR szAppxManifestPath[MAX_PATH]={};
		VARIANT_BOOL varStatus;
		VARIANT varFileName;

		lstrcpy(szAppxManifestPath,pszPackagePath);
		if(szAppxManifestPath[lstrlen(szAppxManifestPath)-1]!='\\')lstrcat(szAppxManifestPath,_T("\\"));
		lstrcat(szAppxManifestPath,_T("AppxManifest.xml"));
		VariantInit(&varFileName);

		if(SUCCEEDED(VariantFromString(szAppxManifestPath,varFileName))){
			if(SUCCEEDED(pXMLDocument->load(varFileName,&varStatus))&&
				varStatus==VARIANT_TRUE){
				IXMLDOMNode* pNode=NULL;
				BSTR bstrNodeName=SysAllocString(_T("//*[local-name()='VisualElements']"));

				if(SUCCEEDED(pXMLDocument->selectSingleNode(bstrNodeName,&pNode))&&pNode){
					IXMLDOMNode *pLogoNode=NULL;
					IXMLDOMNode *pBackgroundColorNode=NULL;
					IXMLDOMNamedNodeMap* pXMLNamedNodeMap=NULL;
					if(SUCCEEDED(pNode->get_attributes(&pXMLNamedNodeMap))&&pXMLNamedNodeMap){
						BSTR bstrLogos[]={SysAllocString(_T("Square44x44Logo")),SysAllocString(_T("Square30x30Logo")),SysAllocString(_T("SmallLogo"))};
						BSTR bstrBackgroundColor=SysAllocString(_T("BackgroundColor"));

						for(int i=0;i<ARRAY_SIZEOF(bstrLogos);i++){
							if(SUCCEEDED(pXMLNamedNodeMap->getNamedItem(bstrLogos[i],&pLogoNode))&&pLogoNode){
								VARIANT varLogo;
								VariantInit(&varLogo);
								if(SUCCEEDED(pLogoNode->get_nodeValue(&varLogo))){
									TCHAR szLogoPath[MAX_PATH];

									wsprintf(szLogoPath,_T("@{%s?ms-resource://%s/Files/%s}"),pszFullPackageName,((PACKAGE_ID*)byPackageId)->name,varLogo.bstrVal);
									for(int ii=0,length=lstrlen(szLogoPath);ii<length;ii++)if(szLogoPath[ii]=='\\')szLogoPath[ii]='/';
									SHLoadIndirectString(szLogoPath,szLogoPath,MAX_PATH,NULL);
									if(SUCCEEDED(pXMLNamedNodeMap->getNamedItem(bstrBackgroundColor,&pBackgroundColorNode))&&pBackgroundColorNode){
										VARIANT varBackgroundColor;
										VariantInit(&varBackgroundColor);
										if(SUCCEEDED(pBackgroundColorNode->get_nodeValue(&varBackgroundColor))){
											Gdiplus::Bitmap bBitmapOrig(szLogoPath);
											Gdiplus::Bitmap bBitmap(uiIconSize,uiIconSize);
											COLORREF clrBackground=StringToCOLORREF(varBackgroundColor.bstrVal);
											Gdiplus::Graphics gGraphics(&bBitmap);

											if(varBackgroundColor.bstrVal[0]=='#'){
												gGraphics.FillRectangle(new Gdiplus::SolidBrush(Gdiplus::Color(GetRValue(clrBackground),GetGValue(clrBackground),GetBValue(clrBackground))),0,0,bBitmap.GetWidth(),bBitmap.GetHeight());
											}
											gGraphics.DrawImage(&bBitmapOrig,0,0,uiIconSize,uiIconSize);
											bBitmap.GetHICON(&hIcon);

										}
										VariantClear(&varBackgroundColor);
										pBackgroundColorNode->Release();;
									}
								}
								break;
							}
						}
						for(int i=0;i<ARRAY_SIZEOF(bstrLogos);i++)SysFreeString(bstrLogos[i]);
						SysFreeString(bstrBackgroundColor);
						pXMLNamedNodeMap->Release();

					}
					pNode->Release();
				}
				SysFreeString(bstrNodeName);
			}
			VariantClear(&varFileName);
		}
		pXMLDocument->Release();
	}
	CoUninitialize();
	HeapFree(GetProcessHeap(),0,byPackageId);
	HeapFree(GetProcessHeap(),0,pszPackagePath);
	HeapFree(GetProcessHeap(),0,pszFullPackageName);
	CloseHandle(hProcess);
	return hIcon;
}

//あの手この手でアイコンを取得[小さい]
HICON GetSmallIcon(HWND hWnd,bool bUWPApp){
	HICON hIcon=NULL;

	if(hWnd==NULL||IsHungAppWindow(hWnd))return NULL;

	if(bUWPApp){
		hIcon=GetUWPAppIcon(hWnd,16);
	}

	//250msで支度しな!
	if(hIcon==NULL&&!SendMessageTimeout(hWnd,WM_GETICON,ICON_SMALL,0,SMTO_ABORTIFHUNG,250,(PDWORD_PTR)&hIcon)){
		DWORD dwError=GetLastError();

		if(dwError==ERROR_SUCCESS||dwError==ERROR_TIMEOUT){
			return LoadIcon(NULL,IDI_APPLICATION);
		}
	}
	if(hIcon==NULL){
		hIcon=(HICON)GetClassLongPtr(hWnd,GCLP_HICONSM);
	}
	if(hIcon==NULL){
		HWND hParent=GetParent(hWnd);
		if(IsWindowVisible(hParent))return GetSmallIcon(hParent);
	}
	return hIcon;
}

//あの手この手でアイコンを取得[大きい]
HICON GetLargeIcon(HWND hWnd,bool bUWPApp){
	HICON hIcon=NULL;

	if(hWnd==NULL||IsHungAppWindow(hWnd))return NULL;

	if(bUWPApp){
		hIcon=GetUWPAppIcon(hWnd);
	}

	//250msで支度しな!
	if(hIcon==NULL&&!SendMessageTimeout(hWnd,WM_GETICON,ICON_BIG,0,SMTO_ABORTIFHUNG,250,(PDWORD_PTR)&hIcon)){
		DWORD dwError=GetLastError();

		if(dwError==ERROR_SUCCESS||dwError==ERROR_TIMEOUT){
			return LoadIcon(NULL,IDI_APPLICATION);
		}
	}
	if(hIcon==NULL){
		hIcon=(HICON)GetClassLongPtr(hWnd,GCLP_HICON);
	}
	if(hIcon==NULL){
		HWND hParent=GetParent(hWnd);
		if(IsWindowVisible(hParent))return GetLargeIcon(hParent);
	}
	return hIcon;
}

//ウインドウハンドルからファイル名を取得する
bool GetFileNameFromWindowHandle(HWND hWnd,LPTSTR lpFileName,DWORD dwFileNameLength){
	bool bResult=false;
	DWORD dwProcessId=0;

	if(GetWindowThreadProcessId(hWnd,&dwProcessId)!=0){
		bResult=GetProcessFileName(dwProcessId,lpFileName,dwFileNameLength);
#if 0
		HANDLE hProcess=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,false,dwProcessId);

		if(hProcess){
			//EnumProcessModules()は不要、環境によっては遅くなる
//			HMODULE hModule=NULL;

//			if(EnumProcessModules(hProcess,&hModule,sizeof(hModule),&dwRet)){

				/*if(GetModuleFileNameEx(hProcess,hModule,lpFileName,dwFileNameLength)!=0){
					bResult=true;
				}else */if(GetProcessImageFileName(hProcess,lpFileName,dwFileNameLength)!=0){
					//GetProcessImageFileName()は32bit->64bitが可能
					bResult=true;
				}

//			}
		}
		CloseHandle(hProcess);
#endif
	}
	return bResult;
}

#if 0
//ウインドウハンドルからファイル名を取得する
//処理速度が遅いので採用しないことに
bool GetFileNameFromWindowHandle2(HWND hWnd,LPTSTR lpFileName,DWORD dwFileNameLength/*NULL文字を含めた文字数*/){
	bool bResult=false;
	DWORD dwProcessId=0;
	HANDLE hSnapshot=NULL;
	PROCESSENTRY32 ProcessEntry32={sizeof(PROCESSENTRY32)};

	//ウインドウハンドルからプロセスIDを取得
	if(GetWindowThreadProcessId(hWnd,&dwProcessId)!=0){
		if(INVALID_HANDLE_VALUE!=(hSnapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0))){
			bool bResultProcess32=Process32First(hSnapshot,&ProcessEntry32)!=0;

			while(bResultProcess32){
				if(dwProcessId==ProcessEntry32.th32ProcessID){
					//ファイル名をコピー
					lstrcpyn(lpFileName,ProcessEntry32.szExeFile,dwFileNameLength);
					bResult=true;
				}
				bResultProcess32=Process32Next(hSnapshot,&ProcessEntry32)!=0;
			}

			CloseHandle(hSnapshot);
		}
	}
	return bResult;
}
#endif
