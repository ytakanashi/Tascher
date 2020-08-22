//Function.cpp
//様々な便利関数

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.63
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
#include<propvarutil.h>
#include<functiondiscoverykeys.h> //PKEY_AppUserModel_ID
#include<winternl.h>

#if COMMANDLINE_WMI_WIN32_PROCESS
#include<wbemidl.h>
#endif

const CLSID CLSID_ImmersiveShell={0xC2F03A33,0x21F5,0x47FA,0xB4,0xBB,0x15,0x63,0x62,0xA2,0xF2,0x39};
#ifndef __IVirtualDesktopManager_INTERFACE_DEFINED__
EXTERN_C const IID IID_IVirtualDesktopManager;
MIDL_INTERFACE("a5cd92ff-29be-454c-8d04-d82879fb3f1b")
IVirtualDesktopManager : public IUnknown{
public:
	virtual HRESULT STDMETHODCALLTYPE IsWindowOnCurrentVirtualDesktop(
		/* [in] */ __RPC__in HWND topLevelWindow,
		/* [out] */ __RPC__out BOOL *onCurrentDesktop) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetWindowDesktopId(
		/* [in] */ __RPC__in HWND topLevelWindow,
		/* [out] */ __RPC__out GUID *desktopId) = 0;

	virtual HRESULT STDMETHODCALLTYPE MoveWindowToDesktop(
		/* [in] */ __RPC__in HWND topLevelWindow,
		/* [in] */ __RPC__in REFGUID desktopId) = 0;
};
#endif


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

struct MONITORS_INFO{
	MONITOR_INFO* pInfo;
	int iInfoCount;
};


//文字列を置換
bool StrReplace(TCHAR* pszStr,const TCHAR* pszTarget,const TCHAR* pszReplacement){
	bool bResult=false;
	TCHAR* p=NULL;
	TCHAR* tmp=(TCHAR*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(lstrlen(pszStr)-lstrlen(pszTarget)+lstrlen(pszReplacement)+1)*sizeof(TCHAR));

	p=pszStr;
	if(p=StrStr(p,pszTarget)){
		lstrcpy(tmp,p+lstrlen(pszTarget));
		*p='\0';
		lstrcat(pszStr,pszReplacement);
		lstrcat(pszStr,tmp);
		bResult=true;
	}
	HeapFree(GetProcessHeap(),0,tmp);
	return bResult;
}

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

int CALLBACK BrowseForFolderCallbackProc(HWND hWnd,UINT uMsg,LPARAM lParam,LPARAM lpData){
	static HWND hEdit;

	switch(uMsg){
		case BFFM_INITIALIZED:{
			if(lpData){
				//初期ディレクトリ設定
				SendMessage(hWnd,BFFM_SETSELECTION,(WPARAM)true,(LPARAM)lpData);
				hEdit=FindWindowEx(hWnd,NULL,_T("Edit"),NULL);

				if(hEdit){
					//エディットコントロールにオートコンプリート機能を実装
					SHAutoComplete(hEdit,SHACF_FILESYSTEM|SHACF_URLALL|SHACF_FILESYS_ONLY|SHACF_USETAB);
				}
			}
			break;
		}

		case BFFM_SELCHANGED:{
			LPITEMIDLIST lpItemIDList=(LPITEMIDLIST)lParam;
			TCHAR szDirectory[MAX_PATH]={};

			//TODO:マイコンピュータを素通りしてしまう
			if(SHGetPathFromIDList(lpItemIDList,szDirectory)){
				if(hEdit){
					lstrcat(szDirectory,_T("\\"));
					SendMessage(hEdit,WM_SETTEXT,(WPARAM)0,(LPARAM)szDirectory);
				}
			}
			break;
		}

		//無効なディレクトリ名であった場合
		case BFFM_VALIDATEFAILED:return 1;

		default:
			break;
	}
	return 0;
}

//ディレクトリを選択
bool SelectDirectory(HWND hWnd,TCHAR* pResult,const TCHAR* pszTitle,const TCHAR* pszDefaultDirectory){
	bool bResult=false;

	CoInitialize(NULL);
	LPTSTR lpBuffer;
	LPITEMIDLIST pidlRoot;
	LPITEMIDLIST lpItemIDList=NULL;
	LPMALLOC lpMalloc=NULL;
	if(FAILED(SHGetMalloc(&lpMalloc)))return false;
	if((lpBuffer=(LPTSTR)lpMalloc->Alloc(MAX_PATH))==NULL)return false;
	if(!SUCCEEDED(SHGetSpecialFolderLocation(NULL,CSIDL_DESKTOP,&pidlRoot))){
		lpMalloc->Free(lpBuffer);
		CoUninitialize();
		return false;
	}

	//デスクトップのパスを取得
	SHFILEINFO shFileInfo={};
	SHGetFileInfo((LPCTSTR)pidlRoot,0,&shFileInfo,sizeof(SHFILEINFO),SHGFI_DISPLAYNAME|SHGFI_PIDL);

	BROWSEINFO BrowseInfo={};
	BrowseInfo.hwndOwner=hWnd;
	BrowseInfo.pidlRoot=pidlRoot;
	BrowseInfo.pszDisplayName=lpBuffer;
	BrowseInfo.lpszTitle=(pszTitle)?pszTitle:_T("フォルダを選択してください");
	//BIF_DONTGOBELOWDOMAINを有効にすると、デスクトップ上のファイルが列挙されてしまう
	BrowseInfo.ulFlags=BIF_USENEWUI|BIF_NONEWFOLDERBUTTON|BIF_RETURNONLYFSDIRS/*|BIF_DONTGOBELOWDOMAIN*/|BIF_VALIDATE;
	BrowseInfo.lpfn=BrowseForFolderCallbackProc;
	BrowseInfo.lParam=reinterpret_cast<LPARAM>((pszDefaultDirectory)?pszDefaultDirectory:shFileInfo.szDisplayName);//szDesktopPath;
	BrowseInfo.iImage=0;

	lpItemIDList=SHBrowseForFolder(&BrowseInfo);
	if(lpItemIDList){
		if(SHGetPathFromIDList(lpItemIDList,pResult)){
			bResult=true;
		}
		lpMalloc->Free(lpItemIDList);
	}
	lpMalloc->Free(pidlRoot);
	lpMalloc->Free(lpBuffer);
	lpMalloc->Release();
	CoUninitialize();
	return bResult;
}

//モニター列挙のコールバック
BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor,HDC hdcMonitor,LPRECT lprcMonitor,LPARAM dwData){
	MONITORINFOEX info;

	info.cbSize=sizeof(MONITORINFOEX);

	GetMonitorInfo(hMonitor,&info);

	MONITOR_INFO* pMonitorInfo=(MONITOR_INFO*)dwData;
	MONITOR_INFO mi={hMonitor,*lprcMonitor,info};

	for(size_t i=0,iSize=HeapSize(GetProcessHeap(),0,pMonitorInfo)/sizeof(MONITOR_INFO);
		i<iSize;
		++i){
		if(pMonitorInfo[i].hMonitor!=NULL)continue;
		pMonitorInfo[i]=mi;
		break;
	}

	return TRUE;
}

//モニタ名からHMONITORを取得
HMONITOR MonitorFromName(LPCTSTR lpszMonitorName){
	if(lpszMonitorName==NULL)return false;

	HMONITOR hResult=NULL;
	int iMonitorCount=GetSystemMetrics(SM_CMONITORS);

	MONITOR_INFO* pMonitorInfo;

	pMonitorInfo=(MONITOR_INFO*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(iMonitorCount)*sizeof(MONITOR_INFO));
	EnumDisplayMonitors(NULL,NULL,MonitorEnumProc,(LPARAM)pMonitorInfo);

	for(size_t i=0,iSize=HeapSize(GetProcessHeap(),0,pMonitorInfo)/sizeof(MONITOR_INFO);
		i<iSize;
		++i){
			if(lstrcmp(pMonitorInfo[i].info.szDevice,lpszMonitorName)==0){
			hResult=pMonitorInfo[i].hMonitor;
			break;
		}
	}

	HeapFree(GetProcessHeap(),0,pMonitorInfo);
	pMonitorInfo=NULL;

	return hResult;
}

//モニタリストを取得
MONITOR_INFO* AllocMonitorsList(){
	MONITOR_INFO* pMonitorInfo;

	int iMonitorCount=GetSystemMetrics(SM_CMONITORS);

	pMonitorInfo=(MONITOR_INFO*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(iMonitorCount)*sizeof(MONITOR_INFO));
	EnumDisplayMonitors(NULL,NULL,MonitorEnumProc,(LPARAM)pMonitorInfo);

	return pMonitorInfo;
}

//モニタリストを解放
void FreeMonitorsList(MONITOR_INFO* pMonitorInfo){
	HeapFree(GetProcessHeap(),0,pMonitorInfo);
	pMonitorInfo=NULL;
}

//指定したモニタにウインドウを移動
bool MoveToMonitor(HWND hWnd,HMONITOR hDestMonitor){
	HMONITOR hCurrentMonitor=MonitorFromWindow(hWnd,MONITOR_DEFAULTTONEAREST);
	if(hDestMonitor==NULL||hCurrentMonitor==hDestMonitor)return false;

	WINDOWPLACEMENT wndpl;
	wndpl.length=sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(hWnd,&wndpl);

	MONITORINFO info;
	info.cbSize=sizeof(info);
	GetMonitorInfo(hCurrentMonitor,&info);
	RECT rcSrcMonitor=info.rcMonitor;

	GetMonitorInfo(hDestMonitor,&info);

	RECT rcDestMonitor=info.rcMonitor;
	RECT rcDestWindow=wndpl.rcNormalPosition;

	if(wndpl.showCmd!=SW_SHOWMAXIMIZED&&
	   wndpl.showCmd!=SW_SHOWMINIMIZED){
		GetWindowRect(hWnd,&rcDestWindow);
	}

	rcDestWindow.left-=rcSrcMonitor.left;
	rcDestWindow.right-=rcSrcMonitor.left;
	rcDestWindow.top-=rcSrcMonitor.top;
	rcDestWindow.bottom-=rcSrcMonitor.top;

	int iSrcMonitorWidth=rcSrcMonitor.right-rcSrcMonitor.left;
	int iSrcMonitorHeight=rcSrcMonitor.bottom-rcSrcMonitor.top;

	int iDestMonitorWidth=rcDestMonitor.right-rcDestMonitor.left;
	int iDestMonitorHeight=rcDestMonitor.bottom-rcDestMonitor.top;

	int iDestWindowWidth=rcDestWindow.right-rcDestWindow.left;
	int iDestWindowHeight=rcDestWindow.bottom-rcDestWindow.top;

	if(iDestWindowWidth>iDestMonitorWidth||
	   iDestWindowHeight>iDestMonitorHeight||
	   !((GetWindowLongPtr(hWnd,GWL_STYLE)&DS_MODALFRAME)||
		 (GetWindowLongPtr(hWnd,GWL_EXSTYLE)&WS_EX_DLGMODALFRAME))){
		//モニタからはみだす場合、モーダルの場合
		rcDestWindow.left=rcDestWindow.left*iDestMonitorWidth/iSrcMonitorWidth;
		rcDestWindow.top=rcDestWindow.top*iDestMonitorHeight/iSrcMonitorHeight;
		rcDestWindow.right=rcDestWindow.right*iDestMonitorWidth/iSrcMonitorWidth;
		rcDestWindow.bottom=rcDestWindow.bottom*iDestMonitorHeight/iSrcMonitorHeight;
	}else{
		int iX=(iSrcMonitorWidth-iDestWindowWidth!=0)?(rcDestWindow.left*(iDestMonitorWidth-iDestWindowWidth)/(iSrcMonitorWidth-iDestWindowWidth)):0;
		rcDestWindow.right=iDestWindowWidth+iX;
		rcDestWindow.left=iX;

		int iY=(iSrcMonitorHeight-iDestWindowHeight!=0)?(rcDestWindow.top*(iDestMonitorHeight-iDestWindowHeight)/(iSrcMonitorHeight-iDestWindowHeight)):0;
		rcDestWindow.bottom=iDestWindowHeight+iY;
		rcDestWindow.top=iY;
	}

	rcDestWindow.left+=rcDestMonitor.left;
	rcDestWindow.right+=rcDestMonitor.left;
	rcDestWindow.top+=rcDestMonitor.top;
	rcDestWindow.bottom+=rcDestMonitor.top;

	wndpl.rcNormalPosition=rcDestWindow;
	SetWindowPlacement(hWnd,&wndpl);

	if(wndpl.showCmd==SW_SHOWMAXIMIZED){
		ShowWindow(hWnd,SW_RESTORE);
		ShowWindow(hWnd,SW_MAXIMIZE);
	}else{
		SetWindowPos(hWnd,0,rcDestWindow.left,rcDestWindow.top,(rcDestWindow.right-rcDestWindow.left),(rcDestWindow.bottom-rcDestWindow.top),SWP_NOZORDER);
	}

	if(wndpl.showCmd==SW_SHOWMINIMIZED){
		ShowWindow(hWnd,SW_RESTORE);
	}
	return true;
}

//画面中央にウインドウを表示(マルチモニタの場合カーソルがあるモニタ)
bool SetCenterWindow(HWND hWnd){
	RECT rc;
	GetWindowRect(hWnd,&rc);

	int iX=0,iY=0;

	POINT pt;

	GetCursorPos(&pt);

	HMONITOR hMonitor=MonitorFromPoint(pt,MONITOR_DEFAULTTONEAREST);

	if(hMonitor){
		MONITORINFO info;

		info.cbSize=sizeof(MONITORINFO);
		if(GetMonitorInfo(hMonitor,&info)){
			iX=info.rcWork.left+((info.rcWork.right-info.rcWork.left-(rc.right-rc.left))>>1);
			iY=info.rcWork.top+((info.rcWork.bottom-info.rcWork.top-(rc.bottom-rc.top))>>1);
		}
	}

	if(iX==0&&iY==0){
		iX=(GetSystemMetrics(SM_CXSCREEN)-(rc.right-rc.left))>>1;
		iY=(GetSystemMetrics(SM_CYSCREEN)-(rc.bottom-rc.top))>>1;
	}

	return SetWindowPos(hWnd,
						NULL,
						iX,
						iY,
						-1,
						-1,
						SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE
						)!=0;
}

//画面中央にウインドウを表示(マルチモニタの場合指定したウインドウがあるモニタ)
bool SetCenterWindow(HWND hWnd,HWND hForegroundWnd){
	RECT rc;
	GetWindowRect(hWnd,&rc);

	int iX=0,iY=0;

	POINT pt;

	GetCursorPos(&pt);

	HMONITOR hMonitor=MonitorFromWindow(hForegroundWnd,MONITOR_DEFAULTTONEAREST);

	if(hMonitor){
		MONITORINFO info;

		info.cbSize=sizeof(MONITORINFO);

		if(GetMonitorInfo(hMonitor,&info)){
			iX=info.rcWork.left+((info.rcWork.right-info.rcWork.left-(rc.right-rc.left))>>1);
			iY=info.rcWork.top+((info.rcWork.bottom-info.rcWork.top-(rc.bottom-rc.top))>>1);
		}
	}

	if(iX==0&&iY==0){
		iX=(GetSystemMetrics(SM_CXSCREEN)-(rc.right-rc.left))>>1;
		iY=(GetSystemMetrics(SM_CYSCREEN)-(rc.bottom-rc.top))>>1;
	}

	return SetWindowPos(hWnd,
						NULL,
						iX,
						iY,
						-1,
						-1,
						SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE
						)!=0;
}

//画面中央にウインドウを表示(マルチモニタの場合指定した名前のモニタ)
bool SetCenterWindow(HWND hWnd,LPCTSTR lpszMonitorName){
	RECT rc;
	GetWindowRect(hWnd,&rc);

	int iX=0,iY=0;

	POINT pt;

	GetCursorPos(&pt);

	HMONITOR hMonitor=MonitorFromName(lpszMonitorName);

	if(hMonitor){
		MONITORINFO info;

		info.cbSize=sizeof(MONITORINFO);

		if(GetMonitorInfo(hMonitor,&info)){
			iX=info.rcWork.left+((info.rcWork.right-info.rcWork.left-(rc.right-rc.left))>>1);
			iY=info.rcWork.top+((info.rcWork.bottom-info.rcWork.top-(rc.bottom-rc.top))>>1);
		}
	}

	if(iX==0&&iY==0){
		iX=(GetSystemMetrics(SM_CXSCREEN)-(rc.right-rc.left))>>1;
		iY=(GetSystemMetrics(SM_CYSCREEN)-(rc.bottom-rc.top))>>1;
	}

	return SetWindowPos(hWnd,
						NULL,
						iX,
						iY,
						-1,
						-1,
						SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE
						)!=0;
}

//カーソル移動先の四隅の座標を取得
bool GetCursorCornerPos(POINT* ppt,SCC_CORNERS eCorner,RECT* prc,int iMarginHorizontal,int iMarginVertical){
	if(!prc||!ppt)return false;

	switch(eCorner){
		case SCC_LEFTTOP:
			//左上
			ppt->x=prc->left+iMarginHorizontal;
			ppt->y=prc->top+iMarginVertical;
			break;

		case SCC_TOP:
			//上
			ppt->x=prc->left+((prc->right-prc->left)>>1);
			ppt->y=prc->top+iMarginVertical;
			break;

		case SCC_RIGHTTOP:
			//右上
			ppt->x=prc->right-iMarginHorizontal;
			ppt->y=prc->top+iMarginVertical;
			break;

		case SCC_LEFT:
			//左
			ppt->x=prc->left+iMarginHorizontal;
			ppt->y=prc->top+((prc->bottom-prc->top)>>1);
			break;

		case SCC_RIGHT:
			//右
			ppt->x=prc->right-iMarginHorizontal;
			ppt->y=prc->top+((prc->bottom-prc->top)>>1);
			break;

		case SCC_LEFTBOTTOM:
			//左下
			ppt->x=prc->left+iMarginHorizontal;
			ppt->y=prc->bottom-iMarginVertical;
			break;

		case SCC_BOTTOM:
			//下
			ppt->x=prc->left+((prc->right-prc->left)>>1);
			ppt->y=prc->bottom-iMarginVertical;
			break;

		case SCC_RIGHTBOTTOM:
			//右下
			ppt->x=prc->right-iMarginHorizontal;
			ppt->y=prc->bottom-iMarginVertical;
			break;

		case SCC_CENTER:
			//中央
			ppt->x=prc->left+((prc->right-prc->left)>>1);
			ppt->y=prc->top+((prc->bottom-prc->top)>>1);
			break;

		case SCC_NONE:
		default:
			break;
	}
	return true;
}

//カーソルを四隅に移動
bool SetCursorCorner(HWND hWnd,SCC_CORNERS eCorner,int iMarginHorizontal,int iMarginVertical){
	RECT rc;
	POINT pt={0,0};

	GetWindowRect(hWnd,&rc);

	GetCursorCornerPos(&pt,eCorner,&rc,iMarginHorizontal,iMarginVertical);
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

//ウインドウが現在の仮想デスクトップ上に存在するか
bool IsWindowOnCurrentVirtualDesktop(HWND hWnd){
	bool bResult=true;
	RTL_OSVERSIONINFOW info={sizeof(RTL_OSVERSIONINFOW)};

	if(GetWindowsVersion(&info)&&info.dwMajorVersion<10)return true;

		IServiceProvider* pServiceProvider=NULL;
		IVirtualDesktopManager* pVirtualDesktopManager=NULL;

		if(SUCCEEDED(CoCreateInstance(CLSID_ImmersiveShell,NULL,CLSCTX_LOCAL_SERVER,__uuidof(IServiceProvider),(PVOID*)&pServiceProvider))){
			if(SUCCEEDED(pServiceProvider->QueryService(__uuidof(IVirtualDesktopManager),&pVirtualDesktopManager))){
				BOOL bIsWindowOnCurrentVirtualDesktop;
				GUID guid;

				if(((GetWindowLongPtr(hWnd,GWL_STYLE)&DS_MODALFRAME)||
				   (GetWindowLongPtr(hWnd,GWL_EXSTYLE)&WS_EX_DLGMODALFRAME))&&
				   GetParent(hWnd)&&
				   GetWindowLongPtr(GetParent(hWnd),GWL_STYLE)&WS_VISIBLE){
					hWnd=GetParent(hWnd);
				}

				if(GetWindowLongPtr(hWnd,GWL_EXSTYLE)&WS_EX_CONTROLPARENT){
					//Delphi対策
					HWND hOwner=GetWindow(hWnd,GW_OWNER);

					if(hOwner){
						RECT rc;

						GetWindowRect(hOwner,&rc);
						if((rc.right-rc.left)==0&&
						   (rc.bottom-rc.top)==0){
							hWnd=hOwner;
						}
					}
				}

				if(SUCCEEDED(pVirtualDesktopManager->IsWindowOnCurrentVirtualDesktop(hWnd,&bIsWindowOnCurrentVirtualDesktop))){
					if(bIsWindowOnCurrentVirtualDesktop){
						if(SUCCEEDED(pVirtualDesktopManager->GetWindowDesktopId(hWnd,&guid))&&
						   guid==GUID_NULL){
							bResult=false;
						}
					}else{
						bResult=false;
					}
				}
				pVirtualDesktopManager->Release();
			}
			pServiceProvider->Release();
		}
	return bResult;
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

//スタートアップ登録ファイルの有効/無効設定する
//Windows8,10環境ではスタートアップディレクトリにショートカットがあってもレジストリで無効になっていれば起動しない
bool EnableStartupApproved(LPCTSTR lpszShortcutFile,bool bEnable){
	bool bResult=false;
	TCHAR szFileName[MAX_PATH];

	lstrcpy(szFileName,lpszShortcutFile);
	PathStripPath(szFileName);

	HKEY hKey;

	if(ERROR_SUCCESS!=RegOpenKeyEx(
		HKEY_CURRENT_USER,
		_T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolder"),
		0,
		KEY_READ|KEY_WRITE,
		&hKey))return false;

	bool bCurrentState=false;
	BYTE pCurrrentData[12]={};
	DWORD dwDataSize=sizeof(pCurrrentData)/sizeof(pCurrrentData[0]);

	if(RegQueryValueEx(hKey,
					   szFileName,
					   0,
					   NULL,
					   (LPBYTE)pCurrrentData,
					   &dwDataSize)==ERROR_SUCCESS){
		bCurrentState=pCurrrentData[0]==2;

		if(bCurrentState&&bEnable||
		   !bCurrentState&&!bEnable){
			RegCloseKey(hKey);
			return true;
		}

		//HKCUでは先頭1バイト目が2だと有効、3は無効と思われる
		//※WindowsDefenderは6で有効になっていたりする
		//3～4バイト目が0で、5バイト目以降は無効にした日のFILETIME
		BYTE pNewData[]={0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

		if(bEnable){
			bResult=(RegSetValueEx(hKey,
								   szFileName,
								   0,
								   REG_BINARY,
								   (LPBYTE)pNewData,
								   ARRAY_SIZEOF(pNewData))==ERROR_SUCCESS);
		}else{
			SYSTEMTIME st;
			GetLocalTime(&st);

			FILETIME ft;
			SystemTimeToFileTime(&st,&ft);
			LocalFileTimeToFileTime(&ft,&ft);

			pNewData[0]=0x03;
			pNewData[4]=ft.dwLowDateTime&0xff;
			pNewData[5]=(ft.dwLowDateTime>>8)&0xff;
			pNewData[6]=(ft.dwLowDateTime>>16)&0xff;
			pNewData[7]=(ft.dwLowDateTime>>24)&0xff;
			pNewData[8]=ft.dwHighDateTime&0xff;
			pNewData[9]=(ft.dwHighDateTime>>8)&0xff;
			pNewData[10]=(ft.dwHighDateTime>>16)&0xff;
			pNewData[11]=(ft.dwHighDateTime>>24)&0xff;

			bResult=(RegSetValueEx(hKey,
								   szFileName,
								   0,
								   REG_BINARY,
								   (LPBYTE)pNewData,
								   ARRAY_SIZEOF(pNewData))==ERROR_SUCCESS);
		}
	}

	RegCloseKey(hKey);
	return bResult;
}

//スタートアップ登録ファイルの有効/無効設定を削除
bool RemoveStartupApproved(LPCTSTR lpszShortcutFile){
	bool bResult=false;
	TCHAR szFileName[MAX_PATH];

	lstrcpy(szFileName,lpszShortcutFile);
	PathStripPath(szFileName);

	HKEY hKey;

	if(RegOpenKeyEx(HKEY_CURRENT_USER,
					_T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolder"),
					0,
					KEY_SET_VALUE,
					&hKey)!=ERROR_SUCCESS)return false;

	bResult=RegDeleteValue(hKey,szFileName)==ERROR_SUCCESS;
	RegCloseKey(hKey);
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
bool RegistHotKey(HWND hWnd,int iItemId,WORD& pwHotKey){
	bool bResult=false;

	if(pwHotKey){
		if(RegisterHotKey(hWnd,iItemId,LOBYTE(pwHotKey),HIBYTE(pwHotKey))){
			bResult=true;
		}else{
			//ホットキー登録に失敗
			MessageBox(hWnd,_T("ホットキーの登録に失敗しました"),NULL,MB_OK|MB_ICONERROR);
			pwHotKey=0;
			bResult=false;
		}
	}
	return bResult;
}

//ホットキー登録解除
bool UnregistHotKey(HWND hWnd,int iItemId,WORD& pwHotKey){
	if(pwHotKey){
		return UnregisterHotKey(hWnd,iItemId)!=0;
	}
	return false;
}

//ウインドウをフォアグラウンドに持ってくる
bool SetForegroundWindowEx(HWND hWnd){
	bool bResult=false;
	if(IsHungAppWindow(hWnd))return false;

	if(IsIconic(hWnd)){
		//最小化されている場合まず元に戻す
		//管理者権限で動作しているウインドウに対してはWM_SYSCOMMANDでないと処理されない?
//		ShowWindow(hWnd,SW_RESTORE);
		SendMessage(hWnd,WM_SYSCOMMAND,SC_RESTORE,0);
	}
/*
	if(!bHung){
		bool bTopMost=IsWindowTopMost(hWnd);
		DWORD dwTimeout=200000;
		SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT,0,&dwTimeout,0);

		//あの手この手
		DWORD dwThreadID=GetWindowThreadProcessId(hWnd,NULL);

		AttachThreadInput(dwThreadID,GetCurrentThreadId(),true);

		SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT,0,0,0);

		BringWindowToTop(hWnd);
		AllowSetForegroundWindow(ASFW_ANY);
		bResult=SetForegroundWindow(hWnd)!=0;
		SetWindowPos(hWnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		SetWindowPos(hWnd,(bTopMost)?HWND_TOPMOST:HWND_NOTOPMOST,0,0,0,0,SWP_SHOWWINDOW|SWP_NOMOVE|SWP_NOSIZE);

		SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT,0,(PVOID)dwTimeout,0);

		AttachThreadInput(dwThreadID,GetCurrentThreadId(),false);
	}else{
		//対象のウインドウが応答なしの場合、AttachThreadInput()とSetWindowPos()を実行すると巻き添えに
		bool bTopMost=IsWindowTopMost(hWnd);

		AllowSetForegroundWindow(GetWindowThreadProcessId(hWnd,NULL));
		BringWindowToTop(hWnd);
		bResult=SetForegroundWindow(hWnd)!=0;
		SetWindowPos(hWnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		SetWindowPos(hWnd,(bTopMost)?HWND_TOPMOST:HWND_NOTOPMOST,0,0,0,0,SWP_SHOWWINDOW|SWP_NOMOVE|SWP_NOSIZE);
	}
*/

	//Tascher本体をアクティブにする時はウインドウをクリックする処理を併用しています
	ShowWindow(hWnd,SW_SHOW);
	BringWindowToTop(hWnd);
	SetForegroundWindow(hWnd);

	bool bTopMost=IsWindowTopMost(hWnd);
	SetWindowPos(hWnd,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
	SetWindowPos(hWnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
	SetWindowPos(hWnd,(bTopMost)?HWND_TOPMOST:HWND_NOTOPMOST,0,0,0,0,SWP_SHOWWINDOW|SWP_NOMOVE|SWP_NOSIZE);

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

#if COMMANDLINE_WMI_WIN32_PROCESS
//特定のプロセスのコマンドラインを取得
//一度目の呼び出しだとCoInitializeSecurity()が失敗したり、
//成功するようになるとIVirtualDesktopManager::IsWindowOnCurrentVirtualDesktop()が上手く動かなかったりするので
//NtQueryInformationProcess()を使うように。
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

#else

#ifndef NT_ERROR
#define NT_ERROR(Status) ((((ULONG)(Status)) >> 30) == 3)
#endif

//特定のプロセスのコマンドラインを取得
LPTSTR AllocProcessCommandLine(DWORD dwProcessId){
typedef struct _RTL_USER_PROCESS_PARAMETERS{
	BYTE Reserved1[16];
	PVOID Reserved2[10];
	UNICODE_STRING ImagePathName;
	UNICODE_STRING CommandLine;
}RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;

typedef struct _PEB{
	BYTE Reserved1[2];
	BYTE BeingDebugged;
	BYTE Reserved2[1];
	PVOID Reserved3[2];
	PPEB_LDR_DATA Ldr;
	PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
	BYTE Reserved4[104];
	PVOID Reserved5[52];
	PPS_POST_PROCESS_INIT_ROUTINE PostProcessInitRoutine;
	BYTE Reserved6[128];
	PVOID Reserved7[1];
	ULONG SessionId;
}PEB,*PPEB;

	TCHAR* pszCommandLine=NULL;

	HANDLE hProcess=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,false,dwProcessId);
	if(hProcess==NULL)return pszCommandLine;

	static NTSTATUS(NTAPI*pNtQueryInformationProcess)(
		HANDLE ProcessHandle,
		PROCESSINFOCLASS ProcessInformationClass,
		PVOID ProcessInformation,
		ULONG ProcessInformationLength,
		PULONG ReturnLength);

	static ULONG(NTAPI*pRtlNtStatusToDosError)(NTSTATUS);

	if(!pNtQueryInformationProcess)pNtQueryInformationProcess=(NTSTATUS(NTAPI*)(HANDLE,PROCESSINFOCLASS,PVOID,ULONG,PULONG))
		GetProcAddress(GetModuleHandle(_T("ntdll")),"NtQueryInformationProcess");

	if(!pRtlNtStatusToDosError)pRtlNtStatusToDosError=(ULONG(NTAPI*)(NTSTATUS))
		GetProcAddress(GetModuleHandle(_T("ntdll")),"RtlNtStatusToDosError");

	if(pNtQueryInformationProcess==NULL||pRtlNtStatusToDosError==NULL){
		return pszCommandLine;
	}

	PROCESS_BASIC_INFORMATION pbi;
	ULONG uLength=0;
	NTSTATUS status=pNtQueryInformationProcess(hProcess,ProcessBasicInformation,&pbi,sizeof(pbi),&uLength);

	SetLastError(pRtlNtStatusToDosError(status));
	if(NT_ERROR(status)||!pbi.PebBaseAddress){
		return pszCommandLine;
	}

	SIZE_T nNumberOfBytesRead=0;
	_PEB peb;

	if(!ReadProcessMemory(hProcess,pbi.PebBaseAddress,&peb,sizeof(peb),&nNumberOfBytesRead)){
		return pszCommandLine;
	}

	_RTL_USER_PROCESS_PARAMETERS upp;

	if(ReadProcessMemory(hProcess,peb.ProcessParameters,&upp,sizeof(upp),&nNumberOfBytesRead)&&
	   upp.CommandLine.Length){
		pszCommandLine=(TCHAR*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(upp.CommandLine.Length+1)*sizeof(TCHAR));

		if(pszCommandLine){
			ReadProcessMemory(hProcess,upp.CommandLine.Buffer,pszCommandLine,upp.CommandLine.Length,&nNumberOfBytesRead);
		}
	}
	return pszCommandLine;
}
#endif

//AllocProcessCommandLine()で取得したコマンドラインを解放
void FreeProcessCommandLine(TCHAR* pszCommandLine){
	HeapFree(GetProcessHeap(),0,pszCommandLine);
	pszCommandLine=NULL;
}

//管理者権限で実行中かどうか
bool IsAdministratorProcess(DWORD dwProcessId){
	bool bAdministrator=false;
	HANDLE hToken=NULL;
	TOKEN_ELEVATION te;

	HANDLE hProcess=OpenProcess(MAXIMUM_ALLOWED,FALSE,dwProcessId);

	if(hProcess==NULL)return false;

	if(OpenProcessToken(hProcess,GENERIC_READ,&hToken)){
		DWORD dwLength=0;

		if(GetTokenInformation(hToken,TokenElevation,&te,sizeof(te),&dwLength)){
			bAdministrator=te.TokenIsElevated!=0;
		}
		CloseHandle(hToken);
	}
	CloseHandle(hProcess);

	return bAdministrator;
}

//プロセスからApplicationUserModelIdを取得
bool GetAppUserModelIdFromProcess(DWORD dwProcessId,LPTSTR pszAppUserModelId,UINT uiBufferLength){
	bool bResult=false;
	PTSTR pszFullPackageName=NULL;
	PTSTR pszPackagePath=NULL;
	UINT32 uiLength=0;

	HANDLE hProcess=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,dwProcessId);

	static LONG(WINAPI*pGetApplicationUserModelId)(HANDLE,UINT32*,PWSTR);

	if(!pGetApplicationUserModelId)pGetApplicationUserModelId=(LONG(WINAPI*)(HANDLE,UINT32*,PWSTR))
		GetProcAddress(GetModuleHandle(_T("kernel32")),"GetApplicationUserModelId");

	if(pGetApplicationUserModelId&&
	   ERROR_SUCCESS==pGetApplicationUserModelId(hProcess,&uiBufferLength,pszAppUserModelId)){
		bResult=true;
	}

	CloseHandle(hProcess);
	return bResult;
}

//ウインドウからApplicationUserModelIdを取得
bool GetAppUserModelIdFromWindow(HWND hWnd,LPTSTR pszAppUserModelId,UINT uiBufferLength){
	bool bResult=false;
	CoInitializeEx(0,COINIT_MULTITHREADED);

	IPropertyStore *pPropertyStore;

	static HRESULT(WINAPI*pSHGetPropertyStoreForWindow)(HWND,REFIID,void**);

	if(!pSHGetPropertyStoreForWindow)pSHGetPropertyStoreForWindow=(HRESULT(WINAPI*)(HWND,REFIID,void**))
		GetProcAddress(GetModuleHandle(_T("shell32")),"SHGetPropertyStoreForWindow");

	if(pSHGetPropertyStoreForWindow&&
	   SUCCEEDED(pSHGetPropertyStoreForWindow(hWnd,IID_PPV_ARGS(&pPropertyStore)))){
		PROPVARIANT pvAppUserModelId;

		PropVariantInit(&pvAppUserModelId);
		if(SUCCEEDED(pPropertyStore->GetValue(PKEY_AppUserModel_ID,&pvAppUserModelId))){
			if(SUCCEEDED(PropVariantToString(pvAppUserModelId,pszAppUserModelId,uiBufferLength))){
				bResult=true;
			}
		}
		PropVariantClear(&pvAppUserModelId);

		pPropertyStore->Release();
	}

	CoUninitialize();
	return bResult;
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
	IShellFolder* pIShellFolder=NULL;
	IExtractIcon* pIExtractIcon=NULL;
	IMalloc* pMalloc;
	SHGetMalloc(&pMalloc);

	CoInitialize(NULL);
	if(SUCCEEDED(SHGetSpecialFolderLocation(NULL,CSIDL_DESKTOP,&lvItemList))){
		SHGetDesktopFolder(&pIShellFolder);
		if(SUCCEEDED(pIShellFolder->GetUIObjectOf(NULL,1,(LPCITEMIDLIST*)&lvItemList,IID_IExtractIcon,0,(void**)&pIExtractIcon))){
			int iIndex=0;
			UINT uFlags;
			TCHAR szLocation[MAX_PATH]={};

			if(SUCCEEDED(pIExtractIcon->GetIconLocation(GIL_FORSHELL,szLocation,MAX_PATH,&iIndex,&uFlags))){
				pIExtractIcon->Extract(szLocation,iIndex,(!bSmall)?&hIconLarge:NULL,(bSmall)?&hIconSmall:NULL,MAKELONG(32,16));
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

HRESULT VariantFromString(PCTSTR wszValue,VARIANT& Variant){
	HRESULT hResult=S_OK;
	BSTR bstr=SysAllocString(wszValue);

	if(bstr){
		V_VT(&Variant)=VT_BSTR;
		V_BSTR(&Variant)=bstr;
	}else{
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

HRESULT GetShellViewForDesktop(REFIID riid,void** ppResult){
	*ppResult=NULL;

	IShellWindows* pIShellWindows;
	HRESULT hResult=CoCreateInstance(CLSID_ShellWindows,NULL,CLSCTX_LOCAL_SERVER,IID_PPV_ARGS(&pIShellWindows));
	if(SUCCEEDED(hResult)){
		HWND hWnd;
		IDispatch* pIDispatch;
		VARIANT vEmpty={};

		if(S_OK==pIShellWindows->FindWindowSW(&vEmpty,&vEmpty,SWC_DESKTOP,(long*)&hWnd,SWFO_NEEDDISPATCH,&pIDispatch)){
			IShellBrowser* pIShellBrowser;

			hResult=IUnknown_QueryService(pIDispatch,SID_STopLevelBrowser,IID_PPV_ARGS(&pIShellBrowser));
			if(SUCCEEDED(hResult)){
				IShellView* pIShellView;

				hResult=pIShellBrowser->QueryActiveShellView(&pIShellView);
				if(SUCCEEDED(hResult)){
					hResult=pIShellView->QueryInterface(riid,ppResult);
					pIShellView->Release();
				}
				pIShellBrowser->Release();
			}
			pIDispatch->Release();
		}else{
			hResult=E_FAIL;
		}
		pIShellWindows->Release();
	}
	return hResult;
}

HRESULT GetShellDispatchFromView(IShellView* pIShellView,REFIID riid,void** ppResult){
	*ppResult=NULL;

	IDispatch* pIDispatchBackground;
	HRESULT hResult=pIShellView->GetItemObject(SVGIO_BACKGROUND,IID_PPV_ARGS(&pIDispatchBackground));
	if(SUCCEEDED(hResult)){
		IShellFolderViewDual* pIShellFolderViewDual;
		hResult=pIDispatchBackground->QueryInterface(IID_PPV_ARGS(&pIShellFolderViewDual));

		if(SUCCEEDED(hResult)){
			IDispatch* pIDispatch;
			hResult=pIShellFolderViewDual->get_Application(&pIDispatch);

			if(SUCCEEDED(hResult)){
				hResult=pIDispatch->QueryInterface(riid,ppResult);
				pIDispatch->Release();
			}
			pIShellFolderViewDual->Release();
		}
		pIDispatchBackground->Release();
	}
	return hResult;
}

//降格ShellExecute()
bool ShellExecuteNonElevated(LPCTSTR lpszFile,LPCTSTR lpszParameters,LPCTSTR lpszWorkingDirectory,int nShowCmd){
	IShellView *pIShellView;
	HRESULT hResult=GetShellViewForDesktop(IID_PPV_ARGS(&pIShellView));
	if(SUCCEEDED(hResult)){
		IShellDispatch2 *pIShellDispatch2;
		hResult=GetShellDispatchFromView(pIShellView,IID_PPV_ARGS(&pIShellDispatch2));

		if(SUCCEEDED(hResult)){
			BSTR bstrFile=SysAllocString(lpszFile);
			hResult=bstrFile?S_OK:E_OUTOFMEMORY;

			if(SUCCEEDED(hResult)){
				VARIANT varParameters;
				VARIANT varWorkingDirectory;
				VARIANT varEmpty={};
				VARIANT varShowCmd;

				VariantInit(&varParameters);
				VariantInit(&varWorkingDirectory);

				VariantInit(&varShowCmd);
				varShowCmd.vt=VT_I4;
				varShowCmd.intVal=nShowCmd;

				if(lpszParameters)VariantFromString(lpszParameters,varParameters);
				if(lpszWorkingDirectory)VariantFromString(lpszWorkingDirectory,varWorkingDirectory);

				AllowSetForegroundWindow(ASFW_ANY);
				hResult=pIShellDispatch2->ShellExecute(bstrFile,varParameters,varWorkingDirectory,varEmpty,varShowCmd);

				SysFreeString(bstrFile);
				if(lpszParameters)VariantClear(&varParameters);
				if(lpszWorkingDirectory)VariantClear(&varWorkingDirectory);
			}
			pIShellDispatch2->Release();
		}
		pIShellView->Release();
	}
	return SUCCEEDED(hResult);
}

/*
#if (_WIN32_WINNT < 0x0600)
typedef struct _STARTUPINFOEXA{
	STARTUPINFOA StartupInfo;
	LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList;
}STARTUPINFOEXA,*LPSTARTUPINFOEXA;
typedef struct _STARTUPINFOEXW{
	STARTUPINFOW StartupInfo;
	LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList;
}STARTUPINFOEXW,*LPSTARTUPINFOEXW;
#ifdef UNICODE
typedef STARTUPINFOEXW STARTUPINFOEX;
typedef LPSTARTUPINFOEXW LPSTARTUPINFOEX;
#else
typedef STARTUPINFOEXA STARTUPINFOEX;
typedef LPSTARTUPINFOEXA LPSTARTUPINFOEX;
#endif // UNICODE

#define PROC_THREAD_ATTRIBUTE_NUMBER    0x0000FFFF
#define PROC_THREAD_ATTRIBUTE_THREAD    0x00010000  // Attribute may be used with thread creation
#define PROC_THREAD_ATTRIBUTE_INPUT     0x00020000  // Attribute is input only
#define PROC_THREAD_ATTRIBUTE_ADDITIVE  0x00040000  // Attribute may be "accumulated," e.g. bitmasks, counters, etc.

#ifndef _USE_FULL_PROC_THREAD_ATTRIBUTE
typedef enum _PROC_THREAD_ATTRIBUTE_NUM {
    ProcThreadAttributeParentProcess        = 0,
    ProcThreadAttributeHandleList           = 2,
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN7)
    ProcThreadAttributeGroupAffinity        = 3,
    ProcThreadAttributePreferredNode        = 4,
    ProcThreadAttributeIdealProcessor       = 5,
    ProcThreadAttributeUmsThread            = 6,
    ProcThreadAttributeMitigationPolicy     = 7,
#endif
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
    ProcThreadAttributeSecurityCapabilities = 9,
#endif
    ProcThreadAttributeProtectionLevel      = 11,
#if (_WIN32_WINNT >= _WIN32_WINNT_WINBLUE)
#endif
#if (_WIN32_WINNT >= _WIN32_WINNT_WINTHRESHOLD)
    ProcThreadAttributeJobList              = 13,
#endif
} PROC_THREAD_ATTRIBUTE_NUM;
#endif

#define ProcThreadAttributeValue(Number, Thread, Input, Additive) \
    (((Number) & PROC_THREAD_ATTRIBUTE_NUMBER) | \
     ((Thread != FALSE) ? PROC_THREAD_ATTRIBUTE_THREAD : 0) | \
     ((Input != FALSE) ? PROC_THREAD_ATTRIBUTE_INPUT : 0) | \
     ((Additive != FALSE) ? PROC_THREAD_ATTRIBUTE_ADDITIVE : 0))

#define PROC_THREAD_ATTRIBUTE_PARENT_PROCESS \
    ProcThreadAttributeValue (ProcThreadAttributeParentProcess, FALSE, TRUE, FALSE)
#define PROC_THREAD_ATTRIBUTE_HANDLE_LIST \
    ProcThreadAttributeValue (ProcThreadAttributeHandleList, FALSE, TRUE, FALSE)

#endif

bool ShellExecuteNonElevated2(PCTSTR lpszFile,PCTSTR lpszParameters,PCTSTR lpszWorkingDirectory,int nShowCmd){
	if(lpszFile==NULL)return false;

	static BOOL(WINAPI*pInitializeProcThreadAttributeList)(LPPROC_THREAD_ATTRIBUTE_LIST,DWORD,DWORD,PSIZE_T);
	static BOOL(WINAPI*pUpdateProcThreadAttribute)(LPPROC_THREAD_ATTRIBUTE_LIST,DWORD,DWORD_PTR,PVOID,SIZE_T,PVOID,PSIZE_T);

	if(!pInitializeProcThreadAttributeList)pInitializeProcThreadAttributeList=(BOOL(WINAPI*)(LPPROC_THREAD_ATTRIBUTE_LIST,DWORD,DWORD,PSIZE_T))
		GetProcAddress(GetModuleHandle(_T("kernel32")),"InitializeProcThreadAttributeList");

	if(!pUpdateProcThreadAttribute)pUpdateProcThreadAttribute=(BOOL(WINAPI*)(LPPROC_THREAD_ATTRIBUTE_LIST,DWORD,DWORD_PTR,PVOID,SIZE_T,PVOID,PSIZE_T))
		GetProcAddress(GetModuleHandle(_T("kernel32")),"UpdateProcThreadAttribute");

	if(!pInitializeProcThreadAttributeList||
	   !pUpdateProcThreadAttribute)return false;

	HWND hWnd=GetShellWindow();

	DWORD dwProcessId;
	GetWindowThreadProcessId(hWnd,&dwProcessId);

	HANDLE hProcess=OpenProcess(PROCESS_CREATE_PROCESS,FALSE,dwProcessId);

	SIZE_T size;
	pInitializeProcThreadAttributeList(NULL,1,0,&size);
	PPROC_THREAD_ATTRIBUTE_LIST pAttributeList=(PPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,size);

	pInitializeProcThreadAttributeList(pAttributeList,1,0,&size);
	pUpdateProcThreadAttribute(pAttributeList,0,PROC_THREAD_ATTRIBUTE_PARENT_PROCESS,&hProcess,sizeof(hProcess),NULL,NULL);

	TCHAR* pszCommand=(TCHAR*)HeapAlloc(GetProcessHeap(),
										HEAP_ZERO_MEMORY,
										(lstrlen(lpszFile)+(lpszParameters?(lstrlen(lpszParameters)):0)+12)*sizeof(TCHAR));

	STARTUPINFOEX StartupInfoEx={};
	StartupInfoEx.lpAttributeList=pAttributeList;
	StartupInfoEx.StartupInfo.cb=sizeof(STARTUPINFOEX);
	StartupInfoEx.StartupInfo.wShowWindow=SW_MAXIMIZE;
	StartupInfoEx.StartupInfo.dwFlags=STARTF_USESHOWWINDOW;
	PROCESS_INFORMATION ProcessInfo;

	if(StrChr(lpszFile,' ')){
		wsprintf(pszCommand,_T("\"%s\""),lpszFile);
	}else{
		lstrcpy(pszCommand,lpszFile);
	}
	if(lpszParameters){
		wsprintf(pszCommand,_T("%s %s"),pszCommand,lpszParameters);
	}

	bool bResult=CreateProcessW(NULL,pszCommand,NULL,NULL,FALSE,
								CREATE_NEW_CONSOLE|EXTENDED_STARTUPINFO_PRESENT,
								NULL,lpszWorkingDirectory,&StartupInfoEx.StartupInfo,&ProcessInfo);
	CloseHandle(ProcessInfo.hProcess);
	CloseHandle(ProcessInfo.hThread);
	CloseHandle(hProcess);
	HeapFree(GetProcessHeap(),0,pAttributeList);

	return bResult;
}
*/

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

/*
//ウインドウハンドルからファイル名を取得する
//処理速度が遅いので採用しないことに
bool GetFileNameFromWindowHandle2(HWND hWnd,LPTSTR lpFileName,DWORD dwFileNameLength){
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
*/

//HeapFree(GetProcessHeap(),0)で解放すること
UINT HICON2DIB(LPBYTE* lpBuffer,HICON hIcon){
	BITMAP bm;
	ICONINFO iconInfo;
	HBITMAP hbm=NULL;

	GetIconInfo(hIcon,&iconInfo);
	hbm=iconInfo.hbmColor;
	if(!GetObject(hbm,sizeof(BITMAP),&bm)){
		hbm=iconInfo.hbmMask;
		if(!GetObject(hbm,sizeof(BITMAP),&bm)){
			DeleteObject(iconInfo.hbmColor);
			DeleteObject(iconInfo.hbmMask);
			return 0;
		}
		bm.bmHeight>>=1;
	}

	BITMAPINFO bi={0};
	bi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth=bm.bmWidth;
	bi.bmiHeader.biHeight=bm.bmHeight;
	bi.bmiHeader.biPlanes=1;
	bi.bmiHeader.biBitCount=24;
	bi.bmiHeader.biCompression=BI_RGB;
	bi.bmiHeader.biSizeImage=((bm.bmWidth*3+3)&0xFFFFFFFC)*bm.bmHeight;

	UINT uBufferSize=bm.bmWidth*bm.bmHeight*4;

	*lpBuffer=(LPBYTE)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,uBufferSize*sizeof(BYTE));

	if(*lpBuffer==NULL){
		return 0;
	}

	HDC hDC=CreateCompatibleDC(NULL);

	if(hDC==NULL){
		HeapFree(GetProcessHeap(),0,*lpBuffer);
		return 0;
	}

	HBITMAP hBmpOld=(HBITMAP)SelectObject(hDC,(HGDIOBJ)hbm/*iconInfo.hbmColor*/);

	if(GetDIBits(hDC,hbm/*iconInfo.hbmColor*/,0,bm.bmHeight,(LPVOID)*lpBuffer,&bi,DIB_RGB_COLORS)==0){
		HeapFree(GetProcessHeap(),0,*lpBuffer);
		uBufferSize=0;
	}

	SelectObject(hDC,hBmpOld);

	DeleteObject(iconInfo.hbmColor);
	DeleteObject(iconInfo.hbmMask);

	DeleteDC(hDC);

	return uBufferSize;
}

bool IsSameCursor(HCURSOR hCursor1,HCURSOR hCursor2){
	bool bSame=true;

	LPBYTE lpCursor1=NULL,lpCursor2=NULL;
	UINT uBufferSize1=HICON2DIB(&lpCursor1,hCursor1);
	UINT uBufferSize2=HICON2DIB(&lpCursor2,hCursor2);

	if(!uBufferSize1||!uBufferSize2||uBufferSize1!=uBufferSize2){
		bSame=false;
	}else{
		for(UINT i=0;i<uBufferSize1;i++){
			if(lpCursor1[i]!=lpCursor2[i]){bSame=false;break;}
		}
	}

	HeapFree(GetProcessHeap(),0,lpCursor1);
	HeapFree(GetProcessHeap(),0,lpCursor2);

	return bSame;
}

//ドラッグ中かどうか
bool IsDragging(){
	if(!(GetAsyncKeyState(VK_LBUTTON)&0x8000)&&
	   !(GetAsyncKeyState(VK_RBUTTON)&0x8000))return false;

	static HCURSOR hBlockCursor=NULL;
	if(!hBlockCursor){hBlockCursor=(HCURSOR)LoadImage(GetModuleHandle(_T("ole32")),MAKEINTRESOURCE(1),IMAGE_CURSOR,0,0,LR_DEFAULTSIZE|LR_SHARED);}
	static HCURSOR hAcceptCursor=NULL;
	if(!hAcceptCursor)hAcceptCursor=(HCURSOR)LoadImage(GetModuleHandle(_T("ole32")),MAKEINTRESOURCE(3),IMAGE_CURSOR,0,0,LR_DEFAULTSIZE|LR_SHARED);

	CURSORINFO CursorInfo={sizeof(CURSORINFO)};

	GetCursorInfo(&CursorInfo);

/*
	//ウインドウに紐付けられたカーソルと同じか
	if(hWnd&&IsSameCursor(CursorInfo.hCursor,
						  (HCURSOR)GetClassLongPtr(hWnd,GCLP_HCURSOR))){
		return false;
	}
*/

	//ドラッグ中カーソルかどうか
	//ドロップ禁止カーソル
	if(IsSameCursor(CursorInfo.hCursor,hBlockCursor)){
		return true;
	}

	//ドロップ許可カーソル
	if(IsSameCursor(CursorInfo.hCursor,hAcceptCursor)){
		return true;
	}

	//通常の矢印カーソルと同じか
/*
	if(IsSameCursor(CursorInfo.hCursor,
					(HCURSOR)LoadImage(NULL,IDC_ARROW,IMAGE_CURSOR,0,0,LR_DEFAULTSIZE|LR_SHARED))){
		return false;
	}
*/

/*
	//通常の矢印+砂時計カーソルと同じか
	if(IsSameCursor(CursorInfo.hCursor,
					(HCURSOR)LoadImage(NULL,IDC_APPSTARTING,IMAGE_CURSOR,0,0,LR_DEFAULTSIZE|LR_SHARED))){
		return false;
	}
*/


	return false;
}

//カーソルがウインドウ内にあるか
bool IsCursorInWindow(HWND hWnd){
	POINT pt={};

	GetCursorPos(&pt);
	RECT rc;

	GetWindowRect(hWnd,&rc);

	return PtInRect(&rc,pt)!=0;
}

#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)

//OSバージョンを取得
bool GetWindowsVersion(PRTL_OSVERSIONINFOW info){
	static NTSTATUS(WINAPI*pRtlGetVersion)(PRTL_OSVERSIONINFOW);

	if(!pRtlGetVersion)pRtlGetVersion=(NTSTATUS(WINAPI*)(PRTL_OSVERSIONINFOW))
		GetProcAddress(GetModuleHandle(_T("ntdll")),"RtlGetVersion");

	return (pRtlGetVersion!=NULL&&
			STATUS_SUCCESS==pRtlGetVersion(info));
}
