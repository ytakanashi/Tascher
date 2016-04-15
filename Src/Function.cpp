//Function.cpp
//様々な便利関数

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.60
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/


#include"StdAfx.h"
#include"Function.h"
#include"Path.h"
#include"resources/resource.h"
#include<objidl.h>
#include<olectl.h>
#include<shlobj.h>
#include<shlwapi.h>


//一つのファイルを選択
bool OpenSingleFileDialog(HWND hWnd,TCHAR* pszResult,int iLength,const TCHAR* pszFilter,const TCHAR* pszTitle){
	TCHAR szDlgErr[128]={};//CommDlgExtendedError()用

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
bool SetCornerCursor(HWND hWnd,SCC_CORNERS scwCorner){
	RECT rc;
	POINT pt;

	GetWindowRect(hWnd,&rc);

	switch(scwCorner){
		case SCC_LEFTTOP:
			//左上
			pt.x=rc.left+20;
			pt.y=rc.top+20;
			break;

		case SCC_RIGHTTOP:
			//右上
			pt.x=rc.right-20;
			pt.y=rc.top+20;
			break;

		case SCC_LEFTBOTTOM:
			//左下
			pt.x=rc.left+20;
			pt.y=rc.bottom-20;
			break;

		case SCC_RIGHTBOTTOM:
		default:
			//右下
			pt.x=rc.right-20;
			pt.y=rc.bottom-20;
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

#if 0
//ウインドウは応答なしである
//IsHungAppWindow()[_WIN32_WINNT >= 0x0500]を後で発見...
bool IsWindowHung(HWND hWnd){
	return (!SendMessageTimeout(hWnd,WM_NULL,0,0,SMTO_ABORTIFHUNG,1000,NULL))?true:false;
}
#endif

//ウインドウをフォアグラウンドに持ってくる
bool SetForegroundWindowEx(HWND hWnd){
	bool bResult=false;
	bool bHung=IsHungAppWindow(hWnd)!=0;
	int iForegroundThreadId=0,iTargetThreadId=0;
	DWORD dwTimeout=0;

	//最小化されていたらもとに戻す
	if(IsIconic(hWnd)){
		ShowWindowAsync(hWnd,SW_RESTORE);
	}

	iForegroundThreadId=GetWindowThreadProcessId(GetForegroundWindow(),NULL);
	iTargetThreadId=GetWindowThreadProcessId(hWnd,NULL);

	if(iForegroundThreadId==iTargetThreadId||bHung){
		BringWindowToTop(hWnd);
		bResult=SetForegroundWindow(hWnd)!=0;
	}else{
		//対象のウインドウが応答なしの場合、AttachThreadInput()とSetWindowPos()を実行すると巻き添えに
		AttachThreadInput(iTargetThreadId,iForegroundThreadId,true);
		SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT,0,&dwTimeout,0);
		SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT,0,(LPVOID)0,0);
		LockSetForegroundWindow(LSFW_UNLOCK);
		SetWindowPos(hWnd,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		BringWindowToTop(hWnd);
		bResult=SetForegroundWindow(hWnd)!=0;
		SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT,0,&dwTimeout,0);
		AttachThreadInput(iTargetThreadId,iForegroundThreadId,false);
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
	int iBufferSize=0;

	if(!OpenClipboard(hWnd))return bResult;

	iBufferSize=(iLength+sizeof(TCHAR))*sizeof(TCHAR);
	HGLOBAL hGlobal=GlobalAlloc(GMEM_MOVEABLE,iBufferSize);
	if(hGlobal!=NULL){
		LPVOID lpMemory=(LPVOID)GlobalLock(hGlobal);
		lstrcpy((LPTSTR)lpMemory,pszText);
		GlobalUnlock(hGlobal);
		EmptyClipboard();
#ifdef UNICODE
		bResult=SetClipboardData(CF_UNICODETEXT,hGlobal)!=NULL;
#else
		bResult=SetClipboardData(CF_TEXT,hGlobal)!=NULL;
#endif
	}
	CloseClipboard();
	return bResult;
}
