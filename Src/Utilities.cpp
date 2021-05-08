//Utilities.cpp
//様々な便利関数

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.64
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/


#include"StdAfx.h"
#include"Utilities.h"
#include"COM.h"
#include"CommonSettings.h"
#include"Path.h"
#include"resources/resource.h"
#include<objidl.h>
#include<olectl.h>
#include<shlobj.h>
#include<shlwapi.h>
#include<gdiplus.h>
#include<psapi.h>
#include<propvarutil.h>
#include<functiondiscoverykeys.h> //PKEY_AppUserModel_ID
#include<winternl.h>
#include<wincodec.h>

const CLSID CLSID_ImmersiveShell={0xC2F03A33,0x21F5,0x47FA,0xB4,0xBB,0x15,0x63,0x62,0xA2,0xF2,0x39};
const IID IID_IServiceProvider={0x6D5140C1,0x7436,0x11CE,0x80,0x34,0x00,0xAA,0x00,0x60,0x09,0xFA};

//ShObjIdl.h
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


enum APPLICATION_VIEW_CLOAK_TYPE{
	AVCT_NONE=0,
	AVCT_DEFAULT=1,
	AVCT_VIRTUAL_DESKTOP=2
};

enum APPLICATION_VIEW_COMPATIBILITY_POLICY{
	AVCP_NONE=0,
	AVCP_SMALL_SCREEN=1,
	AVCP_TABLET_SMALL_SCREEN=2,
	AVCP_VERY_SMALL_SCREEN=3,
	AVCP_HIGH_SCALE_FACTOR=4
};

#define IApplicationViewChangeListener UINT
#define IApplicationViewOperation UINT
#define IApplicationViewPosition UINT
#define IAsyncCallback UINT
#define IImmersiveApplication UINT
#define IImmersiveMonitor UINT
#define IImmersiveMonitor UINT
#define IShellPositionerPriority UINT
#define IShellPositionerPriority UINT



//hstring.h
typedef struct HSTRING__{
	int unused;
}HSTRING__;

typedef /* [v1_enum] */ 
enum TrustLevel{
	BaseTrust=0,
	PartialTrust=(BaseTrust+1),
	FullTrust=(PartialTrust+1)
}TrustLevel;

// Declare the HSTRING handle for C/C++
typedef __RPC_unique_pointer HSTRING__*HSTRING;

//inspectable.h
MIDL_INTERFACE("AF86E2E0-B12D-4c6a-9C5A-D7AA65101E90")
IInspectable:public IUnknown{
public:
	virtual HRESULT STDMETHODCALLTYPE GetIids(
		/* [out] */ __RPC__out ULONG *iidCount,
		/* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(*iidCount) IID** iids)=0;

	virtual HRESULT STDMETHODCALLTYPE GetRuntimeClassName(
		/* [out] */ __RPC__deref_out_opt HSTRING* className)=0;

	virtual HRESULT STDMETHODCALLTYPE GetTrustLevel(
		/* [out] */ __RPC__out TrustLevel* trustLevel)=0;

};

//一部の関数のみ動作確認
//DECLARE_INTERFACE_IID_(IApplicationView, IInspectable, "372E1D3B-38D3-42E4-A15B-8AB2B178F513")
DECLARE_INTERFACE_(IApplicationView,IInspectable){
	STDMETHOD(QueryInterface)(THIS_ REFIID riid,LPVOID FAR* ppvObject)PURE;
	STDMETHOD_(ULONG,AddRef)(THIS)PURE;
	STDMETHOD_(ULONG,Release)(THIS)PURE;
	STDMETHOD(GetIids)(__RPC__out ULONG *iidCount,__RPC__deref_out_ecount_full_opt(*iidCount) IID **iids)PURE;
	STDMETHOD(GetRuntimeClassName)(__RPC__deref_out_opt HSTRING *className)PURE;
	STDMETHOD(GetTrustLevel)(__RPC__out TrustLevel *trustLevel)PURE;
	STDMETHOD(SetFocus)(THIS)PURE;
	STDMETHOD(SwitchTo)(THIS)PURE;
	STDMETHOD(TryInvokeBack)(THIS_ IAsyncCallback*)PURE;
	STDMETHOD(GetThumbnailWindow)(THIS_ HWND*)PURE;
	STDMETHOD(GetMonitor)(THIS_ IImmersiveMonitor**)PURE;
	STDMETHOD(GetVisibility)(THIS_ int*)PURE;
	STDMETHOD(SetCloak)(THIS_ APPLICATION_VIEW_CLOAK_TYPE,int)PURE;
	STDMETHOD(GetPosition)(THIS_ REFIID,void**)PURE;
	STDMETHOD(SetPosition)(THIS_ IApplicationViewPosition*)PURE;
	STDMETHOD(InsertAfterWindow)(THIS_ HWND)PURE;
	STDMETHOD(GetExtendedFramePosition)(THIS_ RECT*)PURE;
	STDMETHOD(GetAppUserModelId)(THIS_ PWSTR*)PURE;
	STDMETHOD(SetAppUserModelId)(THIS_ PCWSTR)PURE;
	STDMETHOD(IsEqualByAppUserModelId)(THIS_ PCWSTR,int*)PURE;
	STDMETHOD(GetViewState)(THIS_ UINT*)PURE;
	STDMETHOD(SetViewState)(THIS_ UINT)PURE;
	STDMETHOD(GetNeediness)(THIS_ int*)PURE;
	STDMETHOD(GetLastActivationTimestamp)(THIS_ ULONGLONG*)PURE;
	STDMETHOD(SetLastActivationTimestamp)(THIS_ ULONGLONG)PURE;
	STDMETHOD(GetVirtualDesktopId)(THIS_ GUID*)PURE;
	STDMETHOD(SetVirtualDesktopId)(THIS_ REFGUID)PURE;
	STDMETHOD(GetShowInSwitchers)(THIS_ int*)PURE;
	STDMETHOD(SetShowInSwitchers)(THIS_ int)PURE;
	STDMETHOD(GetScaleFactor)(THIS_ int*)PURE;
	STDMETHOD(CanReceiveInput)(THIS_ BOOL*)PURE;
	STDMETHOD(GetCompatibilityPolicyType)(THIS_ APPLICATION_VIEW_COMPATIBILITY_POLICY*)PURE;
	STDMETHOD(SetCompatibilityPolicyType)(THIS_ APPLICATION_VIEW_COMPATIBILITY_POLICY)PURE;
	STDMETHOD(GetPositionPriority)(THIS_ IShellPositionerPriority**)PURE;
	STDMETHOD(SetPositionPriority)(THIS_ IShellPositionerPriority*)PURE;
	STDMETHOD(GetSizeConstraints)(THIS_ IImmersiveMonitor*,SIZE*,SIZE*)PURE;
	STDMETHOD(GetSizeConstraintsForDpi)(THIS_ UINT,SIZE*,SIZE*)PURE;
	STDMETHOD(SetSizeConstraintsForDpi)(THIS_ const UINT*,const SIZE*,const SIZE*)PURE;
	STDMETHOD(QuerySizeConstraintsFromApp)(THIS)PURE;
	STDMETHOD(OnMinSizePreferencesUpdated)(THIS_ HWND)PURE;
	STDMETHOD(ApplyOperation)(THIS_ IApplicationViewOperation*)PURE;
	STDMETHOD(IsTray)(THIS_ BOOL*)PURE;
	STDMETHOD(IsInHighZOrderBand)(THIS_ BOOL*)PURE;
	STDMETHOD(IsSplashScreenPresented)(THIS_ BOOL*)PURE;
	STDMETHOD(Flash)(THIS)PURE;
	STDMETHOD(GetRootSwitchableOwner)(THIS_ IApplicationView**)PURE;
	STDMETHOD(EnumerateOwnershipTree)(THIS_ IObjectArray**)PURE;
	STDMETHOD(GetEnterpriseId)(THIS_ PWSTR*)PURE;
	STDMETHOD(GetEnterpriseChromePreference)(THIS_ int*)PURE; 
	STDMETHOD(IsMirrored)(THIS_ BOOL*)PURE;
	//以下省略
};

//一部の関数のみ動作確認
//DECLARE_INTERFACE_IID_(IApplicationViewCollection, IUnknown, "1841C6D7-4F9D-42C0-AF41-8747538F10E5")
DECLARE_INTERFACE_(IApplicationViewCollection,IUnknown){
	STDMETHOD(QueryInterface)(THIS_ REFIID riid,LPVOID FAR * ppvObject)PURE;
	STDMETHOD_(ULONG,AddRef)(THIS)PURE;
	STDMETHOD_(ULONG,Release)(THIS)PURE;
	STDMETHOD(GetViews)(THIS_ IObjectArray**)PURE;
	STDMETHOD(GetViewsByZOrder)(THIS_ IObjectArray**)PURE;
	STDMETHOD(GetViewsByAppUserModelId)(THIS_ PCWSTR,IObjectArray**)PURE;
	STDMETHOD(GetViewForHwnd)(THIS_ HWND,IApplicationView**)PURE;
	STDMETHOD(GetViewForApplication)(THIS_ IImmersiveApplication*,IApplicationView**)PURE;
	STDMETHOD(GetViewForAppUserModelId)(THIS_ PCWSTR,IApplicationView**)PURE;
	STDMETHOD(GetViewInFocus)(THIS_ IApplicationView**)PURE;
	STDMETHOD(Proc10)(THIS_ IApplicationView**)PURE;
	STDMETHOD(RefreshCollection)(THIS) PURE;
	STDMETHOD(RegisterForApplicationViewChanges)(THIS_ IApplicationViewChangeListener*,DWORD*)PURE;
	STDMETHOD(UnregisterForApplicationViewChanges)(THIS_ DWORD)PURE;
};

const int MAX_KEY_LENGTH=255;
const int MAX_VALUE_NAME=16383;


//マウス入力の監視を開始
bool RegisterRawInput(HWND hWnd){
	UINT uDevices=0;

	if(GetRawInputDeviceList(NULL,&uDevices,sizeof(RAWINPUTDEVICELIST))!=0){
		return false;
	}

	RAWINPUTDEVICE Rid[1];
	Rid[0].usUsagePage=0x01;
	Rid[0].usUsage=0x02;
	Rid[0].dwFlags=RIDEV_INPUTSINK;
	Rid[0].hwndTarget=hWnd;

	return RegisterRawInputDevices(Rid,1,sizeof(Rid[0]))!=0;
}

//マウス入力の監視を終了
bool UnregisterRawInput(){
	RAWINPUTDEVICE Rid[1];
	Rid[0].usUsagePage=0x01;
	Rid[0].usUsage=0x02;
	Rid[0].dwFlags=RIDEV_REMOVE;
	Rid[0].hwndTarget=NULL;

	return RegisterRawInputDevices(Rid,1,sizeof(Rid[0]))!=0;
}

//文字列を置換
bool StrReplace(TCHAR* pszStr,const TCHAR* pszTarget,const TCHAR* pszReplacement){
	bool bResult=false;
	TCHAR* p=NULL;
	TCHAR* tmp=(TCHAR*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(lstrlen(pszStr)-lstrlen(pszTarget)+lstrlen(pszReplacement)+1)*sizeof(TCHAR));
	if(tmp==NULL)return false;

	p=pszStr;
	if((p=StrStr(p,pszTarget))!=NULL){
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
			TCHAR szDlgErr[128];

			wsprintf(szDlgErr,_T("ファイルの選択に失敗しました。\nErrCode:0x%08X"),(unsigned int)dwDlgErr);
			MessageBox(hWnd,szDlgErr,NULL,MB_ICONWARNING);
			pszResult=NULL;
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
		case BFFM_VALIDATEFAILED:
			return 1;

		default:
			break;
	}
	return 0;
}

//ディレクトリを選択
bool SelectDirectory(HWND hWnd,TCHAR* pResult,const TCHAR* pszTitle,const TCHAR* pszDefaultDirectory){
	bool bResult=false;
	LPITEMIDLIST lpidlDesktop=NULL;
	LPITEMIDLIST lpidlSelectDirectory=NULL;

	com::Initialize ComInitialize;

	if(SUCCEEDED(SHGetSpecialFolderLocation(NULL,CSIDL_DESKTOP,&lpidlDesktop))){
		//デスクトップのパスを取得
		SHFILEINFO shFileInfo={};
		SHGetFileInfo((LPCTSTR)lpidlDesktop,0,&shFileInfo,sizeof(SHFILEINFO),SHGFI_DISPLAYNAME|SHGFI_PIDL);

		BROWSEINFO BrowseInfo={};
		BrowseInfo.hwndOwner=hWnd;
		BrowseInfo.pidlRoot=lpidlDesktop;
		BrowseInfo.pszDisplayName=NULL;
		BrowseInfo.lpszTitle=(pszTitle)?pszTitle:_T("フォルダを選択してください");
		//BIF_DONTGOBELOWDOMAINを有効にすると、デスクトップ上のファイルが列挙されてしまう
		BrowseInfo.ulFlags=BIF_USENEWUI|BIF_NONEWFOLDERBUTTON|BIF_RETURNONLYFSDIRS/*|BIF_DONTGOBELOWDOMAIN*/|BIF_VALIDATE;
		BrowseInfo.lpfn=BrowseForFolderCallbackProc;
		BrowseInfo.lParam=(LPARAM)((pszDefaultDirectory)?pszDefaultDirectory:shFileInfo.szDisplayName);

		lpidlSelectDirectory=SHBrowseForFolder(&BrowseInfo);
		if(lpidlSelectDirectory!=NULL){
			bResult=SHGetPathFromIDList(lpidlSelectDirectory,pResult)!=0;
			CoTaskMemFree(lpidlSelectDirectory);
		}
		CoTaskMemFree(lpidlDesktop);
	}
	return bResult;
}

bool GetDpiForMonitor(HMONITOR hMonitor,UINT*pdpiX,UINT*pdpiY){
	if(hMonitor==NULL)return false;

	bool bResult=false;
	TCHAR szShCoreDllPath[MAX_PATH]={};

	GetSystemDirectory(szShCoreDllPath,ARRAY_SIZEOF(szShCoreDllPath));
	lstrcat(szShCoreDllPath,_T("\\Shcore.dll"));
	HMODULE hShcore=LoadLibrary(szShCoreDllPath);

	if(hShcore!=NULL){
		//ShellScalingApi.h
		typedef enum MONITOR_DPI_TYPE{
			MDT_EFFECTIVE_DPI=0,
			MDT_ANGULAR_DPI=1,
			MDT_RAW_DPI=2,
			MDT_DEFAULT=MDT_EFFECTIVE_DPI
		}MONITOR_DPI_TYPE;

		HRESULT(WINAPI*pGetDpiForMonitor)(HMONITOR,MONITOR_DPI_TYPE,UINT*,UINT*)=(HRESULT(WINAPI*)(HMONITOR,MONITOR_DPI_TYPE,UINT*,UINT*))GetProcAddress(hShcore,"GetDpiForMonitor");
		if(pGetDpiForMonitor)bResult=SUCCEEDED(pGetDpiForMonitor(hMonitor,MDT_EFFECTIVE_DPI,pdpiX,pdpiY));
		FreeLibrary(hShcore);
	}
	return bResult;
}

//モニター列挙のコールバック
BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor,HDC hdcMonitor,LPRECT lprcMonitor,LPARAM dwData){
	MONITOR_INFO* pMonitorsInfo=(MONITOR_INFO*)dwData;

	for(size_t i=0,iSize=HeapSize(GetProcessHeap(),0,pMonitorsInfo)/sizeof(MONITOR_INFO);
		i<iSize;
		++i){
		if(pMonitorsInfo[i].hMonitor!=NULL)continue;

		MONITORINFOEX miex={};

		miex.cbSize=sizeof(MONITORINFOEX);
		GetMonitorInfo(hMonitor,&miex);

		pMonitorsInfo[i]=MONITOR_INFO{hMonitor,*lprcMonitor,miex};
		break;
	}

	return TRUE;
}

//モニタ名からHMONITORを取得
HMONITOR MonitorFromName(LPCTSTR lpszMonitorName){
	if(lpszMonitorName==NULL)return NULL;

	HMONITOR hResult=NULL;
	int iMonitorCount=GetSystemMetrics(SM_CMONITORS);

	MONITOR_INFO* pMonitorsInfo=NULL;

	pMonitorsInfo=(MONITOR_INFO*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(iMonitorCount)*sizeof(MONITOR_INFO));
	if(pMonitorsInfo==NULL)return NULL;
	EnumDisplayMonitors(NULL,NULL,MonitorEnumProc,(LPARAM)pMonitorsInfo);

	for(size_t i=0;i<iMonitorCount;i++){
		if(lstrcmp(pMonitorsInfo[i].info.szDevice,lpszMonitorName)==0){
			hResult=pMonitorsInfo[i].hMonitor;
			break;
		}
	}

	HeapFree(GetProcessHeap(),0,pMonitorsInfo);
	pMonitorsInfo=NULL;

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
void FreeMonitorsList(MONITOR_INFO** ppMonitorInfo){
	if(*ppMonitorInfo!=NULL){
		HeapFree(GetProcessHeap(),0,*ppMonitorInfo);
		*ppMonitorInfo=NULL;
	}
}

//モニタのDCを作成(DeleteDC()で削除すること)
HDC CreateMonitorDC(HMONITOR hMonitor){
	if(hMonitor==NULL)return NULL;

	MONITORINFOEX info={};
	info.cbSize=sizeof(MONITORINFOEX);
	if(GetMonitorInfo(hMonitor,&info)==0)return NULL;

	return CreateDC(info.szDevice,NULL,NULL,NULL);
}

//指定したモニタにウインドウを移動
bool MoveToMonitor(HWND hWnd,HMONITOR hDestMonitor){
	if(hWnd==NULL)return false;
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

bool SetCenterWindow(HWND hWnd,HMONITOR hMonitor){
	RECT rc={};
	GetWindowRect(hWnd,&rc);

	int iX=0,iY=0;

	if(hMonitor){
		MONITORINFO info={};

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

//画面中央にウインドウを表示(マルチモニタの場合カーソルがあるモニタ)
bool SetCenterWindow(HWND hWnd){
	POINT pt={};

	GetCursorPos(&pt);

	return SetCenterWindow(hWnd,MonitorFromPoint(pt,MONITOR_DEFAULTTONEAREST));
}

//画面中央にウインドウを表示(マルチモニタの場合指定したウインドウがあるモニタ)
bool SetCenterWindow(HWND hWnd,HWND hForegroundWnd){
	return SetCenterWindow(hWnd,MonitorFromWindow(hForegroundWnd,MONITOR_DEFAULTTONEAREST));
}

//画面中央にウインドウを表示(マルチモニタの場合指定した名前のモニタ)
bool SetCenterWindow(HWND hWnd,LPCTSTR lpszMonitorName){
	return SetCenterWindow(hWnd,MonitorFromName(lpszMonitorName));
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
	RECT rc={};
	POINT pt={};

	GetWindowRect(hWnd,&rc);

	GetCursorCornerPos(&pt,eCorner,&rc,iMarginHorizontal,iMarginVertical);
	return SetCursorPos(pt.x,pt.y)!=0;
}

//カーソルをウインドウの中心に移動
bool SetCenterCursor(HWND hWnd){
	POINT pt={};
	RECT rc={};

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
	POINT pt={};

	GetCursorPos(&pt);
	HWND hResultWnd=WindowFromPoint(pt);

	if(hResultWnd==NULL)return NULL;

	POINT ptClient=pt;

	ScreenToClient(hResultWnd,&ptClient);
	{
		HWND hChild=ChildWindowFromPoint(hResultWnd,ptClient);
		if(hChild!=NULL&&IsWindowVisible(hChild)){
			hResultWnd=hChild;
		}
	}

	DWORD dwWnd=MAXWORD;

	HWND hParent=GetParent(hResultWnd);
	if(hParent!=NULL){
		//ポップアップウインドウではない
		if((GetWindowLongPtr(hResultWnd,GWL_STYLE)&WS_POPUP)==0){
			//子ウインドウ取得
			for(HWND hChild=GetWindow(hParent,GW_CHILD);hChild!=NULL;hChild=GetWindow(hChild,GW_HWNDNEXT)){
				//非表示の子ウインドウを無視
				if(IsWindowVisible(hChild)){
					RECT rc={};

					GetWindowRect(hChild,&rc);
					if(PtInRect(&rc,pt)!=0){
						DWORD dwWndChild=(rc.right-rc.left)*(rc.bottom-rc.top);

						if(dwWndChild<dwWnd){
							dwWnd=dwWndChild;
							hResultWnd=hChild;
						}
					}
				}
			}
		}
	}
	return hResultWnd;
}

bool IsDesktopWindow(HWND hWnd){
	TCHAR szClassName[128]={};

	GetClassName(hWnd,szClassName,ARRAY_SIZEOF(szClassName));

	if(lstrcmp(szClassName,_T("Progman"))!=0&&
	   lstrcmp(szClassName,_T("WorkerW"))!=0){
		return false;
	}

	HWND hSHELLDLL_DefView=FindWindowEx(hWnd,NULL,_T("SHELLDLL_DefView"),NULL);
	HWND hSysListView32=FindWindowEx(hSHELLDLL_DefView,NULL,_T("SysListView32"),_T("FolderView"));

	return hSHELLDLL_DefView&&hSysListView32;
}

//ウインドウがフルスクリーンかどうか
bool IsFullScreenWindow(HWND hWnd){
	if(IsDesktopWindow(hWnd))return false;

	HMONITOR hMonitor=MonitorFromWindow(hWnd,MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO info={};

	info.cbSize=sizeof(MONITORINFO);
	GetMonitorInfo(hMonitor,&info);

	RECT rcClient={};
	GetClientRect(hWnd,&rcClient);

	return rcClient.left==0&&rcClient.top==0&&
		rcClient.right==(info.rcMonitor.right-info.rcMonitor.left)&&rcClient.bottom==(info.rcMonitor.bottom-info.rcMonitor.top);
}

//レジストリからIIDを取得
bool GetIIDFromRegistry(TCHAR* pszResult,LPCTSTR lpszIIDName){
	bool bResult=false;
	TCHAR szInterfaceKey[]=_T("SOFTWARE\\Classes\\Interface");
	TCHAR szSubKey[MAX_PATH]={};
	HKEY hInterfaceKey=NULL;

	if(pszResult==NULL)return bResult;

	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,szInterfaceKey,0,KEY_ENUMERATE_SUB_KEYS,&hInterfaceKey)!=ERROR_SUCCESS){
		return bResult;
	}

	for(DWORD dwIndex=0;!bResult;dwIndex++){
		DWORD dwSize=ARRAY_SIZEOF(szSubKey);

		if(RegEnumKeyEx(hInterfaceKey,
						 dwIndex,
						 szSubKey,
						 &dwSize,
						 NULL,
						 NULL,
						 NULL,
						 NULL)==ERROR_SUCCESS){
			TCHAR szIIDKey[MAX_PATH]={};
			HKEY hIIDKey=NULL;
			wsprintf(szIIDKey,_T("%s\\%s"),szInterfaceKey,szSubKey);

			if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,szIIDKey,0,KEY_READ,&hIIDKey)!=ERROR_SUCCESS){
				continue;
			}

			DWORD type=0;
			BYTE data[MAX_PATH]={};
			DWORD cbData=sizeof(data);

			if(RegQueryValueEx(hIIDKey,_T(""),NULL,&type,data,&cbData)==ERROR_SUCCESS){
				if(type==REG_SZ&&
				   lstrcmp((TCHAR*)data,lpszIIDName)==0){
					lstrcpy(pszResult,szSubKey);
					bResult=true;
					break;
				}
			}
			RegCloseKey(hIIDKey);
		}else{
			break;
		}
	}
	RegCloseKey(hInterfaceKey);
	return bResult;
}

//ウインドウが現在の仮想デスクトップ上に存在するか
bool IsWindowOnCurrentVirtualDesktop(HWND hWnd){
	bool bResult=true;
	RTL_OSVERSIONINFOW rovi={};

	rovi.dwOSVersionInfoSize=sizeof(RTL_OSVERSIONINFOW);

	if(GetWindowsVersion(&rovi)&&rovi.dwMajorVersion<10)return false;

	com::Initialize ComInitialize;

	IServiceProvider* pServiceProvider=NULL;
	IID iidIVirtualDesktopManager={};
	static TCHAR szIVirtualDesktopManager[MAX_PATH]={};
	IVirtualDesktopManager* pVirtualDesktopManager=NULL;

	if(!lstrlen(szIVirtualDesktopManager)){
		GetIIDFromRegistry(szIVirtualDesktopManager,_T("IVirtualDesktopManager"));
	}
	if(FAILED(IIDFromString(szIVirtualDesktopManager,&iidIVirtualDesktopManager)))return false;

	if(SUCCEEDED(CoCreateInstance(CLSID_ImmersiveShell,NULL,CLSCTX_LOCAL_SERVER,__uuidof(IServiceProvider),(PVOID*)&pServiceProvider))){
		if(SUCCEEDED(pServiceProvider->QueryService(__uuidof(IVirtualDesktopManager),&pVirtualDesktopManager))){
			BOOL bIsWindowOnCurrentVirtualDesktop=FALSE;
			GUID guid={};

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
					RECT rc={};

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

//ウインドウリストを取得
HWND* AllocWindowListFromApplicationView(){
	HWND* phResult=NULL;
	RTL_OSVERSIONINFOW rovi={sizeof(RTL_OSVERSIONINFOW)};

	if(GetWindowsVersion(&rovi)&&rovi.dwMajorVersion<10)return NULL;

	com::Initialize ComInitialize;

	IServiceProvider* pServiceProvider=NULL;

	if(SUCCEEDED(CoCreateInstance(CLSID_ImmersiveShell,NULL,CLSCTX_LOCAL_SERVER,__uuidof(IServiceProvider),(PVOID*)&pServiceProvider))){
		IID iidIApplicationViewCollection={};
		static TCHAR szIApplicationViewCollection[MAX_PATH]={};
		IApplicationViewCollection* pApplicationViewCollection;

		IID iidIApplicationView={};
		static TCHAR szIApplicationView[MAX_PATH]={};

		if(!lstrlen(szIApplicationView)){
			GetIIDFromRegistry(szIApplicationView,_T("IApplicationView"));
		}

		if(!lstrlen(szIApplicationViewCollection)){
			GetIIDFromRegistry(szIApplicationViewCollection,_T("IApplicationViewCollection"));
		}

		if(SUCCEEDED(IIDFromString(szIApplicationViewCollection,&iidIApplicationViewCollection))&&
		   SUCCEEDED(IIDFromString(szIApplicationView,&iidIApplicationView))&&
		   SUCCEEDED(pServiceProvider->QueryService(iidIApplicationViewCollection,iidIApplicationViewCollection,(void**)&pApplicationViewCollection))){
			IObjectArray* pApplicationViewArray=NULL;
			if(pApplicationViewCollection&&
			   SUCCEEDED(pApplicationViewCollection->GetViewsByZOrder(&pApplicationViewArray))){
				UINT uCount=0;

				if(SUCCEEDED(pApplicationViewArray->GetCount(&uCount))){
					struct WINDOW_LIST{
						HWND hWnd;
						ULONGLONG ullLastActivationTimestamp;
					};

					WINDOW_LIST* pWindowList=NULL;
					int iWindowCount=0;
					HANDLE hHeapWindowList=HeapCreate(NULL,0,0);

					if(hHeapWindowList!=NULL){
						for(UINT i=0;i<uCount;i++){
							IApplicationView* pApplicationView=NULL;
							BOOL bShowInSwitchers=FALSE;
							HWND hWnd=NULL;

							if(FAILED(pApplicationViewArray->GetAt(i,iidIApplicationView,(void**)&pApplicationView)||!pApplicationView))continue;
							if(FAILED(pApplicationView->GetShowInSwitchers(&bShowInSwitchers))||!bShowInSwitchers)continue;
							if(FAILED(pApplicationView->GetThumbnailWindow(&hWnd)))continue;
							ULONGLONG ullLastActivationTimestamp=0;

							if(pApplicationView->GetLastActivationTimestamp(&ullLastActivationTimestamp)==S_OK){
								WINDOW_LIST wl={hWnd,ullLastActivationTimestamp};
								WINDOW_LIST* pTmp=NULL;

								if(iWindowCount==0)pTmp=(WINDOW_LIST*)HeapAlloc(hHeapWindowList,HEAP_ZERO_MEMORY,(++iWindowCount)*sizeof(WINDOW_LIST));
								else pTmp=(WINDOW_LIST*)HeapReAlloc(hHeapWindowList,HEAP_ZERO_MEMORY,pWindowList,(++iWindowCount)*sizeof(WINDOW_LIST));

								pWindowList=pTmp;
								pWindowList[iWindowCount-1]=wl;
							}
						}

						for(int iIndex=0;iIndex<iWindowCount;iIndex++){
							int iCount=iIndex;
							bool bBreakFalg=true;

							for(int i=0;i<iWindowCount-iCount-1;i++){
								if(pWindowList[i].ullLastActivationTimestamp<pWindowList[i+1].ullLastActivationTimestamp){
									WINDOW_LIST tmp=pWindowList[i];
									pWindowList[i]=pWindowList[i+1];
									pWindowList[i+1]=tmp;
									bBreakFalg=false;
								}
							}
							if(bBreakFalg)break;
						}

						//ウインドウリスト+終端分を確保する
						phResult=(HWND*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(iWindowCount+1)*sizeof(HWND));

						if(phResult!=NULL){
							for(int i=0;i<iWindowCount;i++){
								phResult[i]=pWindowList[i].hWnd;
							}
						}
						HeapFree(hHeapWindowList,0,pWindowList);
						hHeapWindowList=NULL;
					}
				}
			}
			pApplicationViewCollection->Release();
		}
		pServiceProvider->Release();
	}
	return phResult;
}

//ウインドウリストを解放
void FreeWindowListFromApplicationView(HWND** pphWnd){
	if(*pphWnd!=NULL){
		HeapFree(GetProcessHeap(),0,*pphWnd);
		*pphWnd=NULL;
	}
}

//ApplicationFrameWindowからWindows.UI.Core.CoreWindowのハンドルを取得
HWND GetCoreWindow(HWND hWnd){
	HWND hUICoreWnd=FindWindowEx(hWnd,NULL,_T("Windows.UI.Core.CoreWindow"),NULL);
	if(!hUICoreWnd){
		//最小化されている場合、子にWindows.UI.Core.CoreWindowがない
		//その場合、トップレベルのWindows.UI.Core.CoreWindowを列挙、そのプロセスIdから取得したAppUserModelIdと
		//ApplicationFrameWindowのウインドウハンドルから取得したAppUserModelIdとを比べ、一致するかで判断する
		HWND hCurrent=GetTopWindow(NULL);
		TCHAR szApplicationUserModelIdFromWindow[256]={};

		//ApplicationFrameWindowのウインドウハンドルからAppUserModelIdを取得
		if(GetAppUserModelIdFromWindow(hWnd,szApplicationUserModelIdFromWindow,ARRAY_SIZEOF(szApplicationUserModelIdFromWindow))){
			do{
				if(!IsHungAppWindow(hCurrent)&&
				   GetWindowLongPtr(hCurrent,GWLP_HWNDPARENT)==NULL&&
				   GetWindowLongPtr(hCurrent,GWL_EXSTYLE)&(WS_EX_NOREDIRECTIONBITMAP)/*&&
						   IsWindowEnabled(hCurrent)*/){//IsWindowEnabled(hCurrent)だとタスクバーにあるが中断しているウインドウを弾いてしまう
					TCHAR szClassName[128]={};

					GetClassName(hCurrent,szClassName,ARRAY_SIZEOF(szClassName));
					if(lstrcmp(szClassName,_T("Windows.UI.Core.CoreWindow"))==0){
						DWORD dwProcessId=0;
						TCHAR szApplicationUserModelIdFromProcess[256]={};

						GetWindowThreadProcessId(hCurrent,&dwProcessId);
						//Windows.UI.Core.CoreWindowのプロセスIdからAppUserModelIdを取得
						GetAppUserModelIdFromProcess(dwProcessId,szApplicationUserModelIdFromProcess,ARRAY_SIZEOF(szApplicationUserModelIdFromProcess));

						if(lstrcmp(szApplicationUserModelIdFromWindow,szApplicationUserModelIdFromProcess)==0){
							hUICoreWnd=hCurrent;
							break;
						}
					}
				}
			}while((hCurrent=GetNextWindow(hCurrent,GW_HWNDNEXT))!=NULL);
		}
	}
	return hUICoreWnd;
}

//ショートカットを作成
bool CreateShortcut(LPCTSTR lpszShortcutFile,LPCTSTR lpszTargetFile,LPCTSTR lpszArgs,LPCTSTR lpszDescription,LPCTSTR lpszWorkingDirectory){
	bool bResult=false;

	com::Initialize ComInitialize;
	IShellLink *pShellLink=NULL;
	IPersistFile *pPersistFile=NULL;

	if(SUCCEEDED(CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLink,(LPVOID*)&pShellLink))){
		if(SUCCEEDED(pShellLink->QueryInterface(IID_IPersistFile,(LPVOID*)&pPersistFile))){
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

				//ショートカットファイルの保存
				bResult=SUCCEEDED(pPersistFile->Save(lpszShortcutFile,true));
			}
			pPersistFile->Release();
		}
		pShellLink->Release();
	}
	return bResult;
}

//スタートアップ登録ファイルの有効/無効設定する
//Windows8,10環境ではスタートアップディレクトリにショートカットがあってもレジストリで無効になっていれば起動しない
bool EnableStartupApproved(LPCTSTR lpszShortcutFile,bool bEnable){
	bool bResult=false;
	TCHAR szFileName[MAX_PATH];

	lstrcpy(szFileName,lpszShortcutFile);
	PathStripPath(szFileName);

	HKEY hKey=NULL;

	if(ERROR_SUCCESS!=RegOpenKeyEx(
		HKEY_CURRENT_USER,
		_T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolder"),
		0,
		KEY_READ|KEY_WRITE,
		&hKey))return false;

	BYTE pCurrrentData[12]={};
	DWORD dwDataSize=sizeof(pCurrrentData)/sizeof(pCurrrentData[0]);

	if(RegQueryValueEx(hKey,
					   szFileName,
					   0,
					   NULL,
					   (LPBYTE)pCurrrentData,
					   &dwDataSize)==ERROR_SUCCESS){
		bool bCurrentState=(pCurrrentData[0]==2);

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
			SYSTEMTIME st={};
			GetLocalTime(&st);

			FILETIME ft={};
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

/*
//COLORREF型をRGB表記の文字列に変換
bool COLORREFToRGB(TCHAR* pszResult,COLORREF color){
	wsprintf(pszResult,_T("%d,%d,%d"),GetRValue(color),GetGValue(color),GetBValue(color));
	return true;
}
*/

//メッセージボックスのLOGFONTを取得
LOGFONT GetMesssageLOGFONT(){
	HMONITOR hMonitor=g_Config.GetActiveMonitor();
	UINT dpiX=0,dpiY=0;
	GetDpiForMonitor(hMonitor,&dpiX,&dpiY);

	LOGFONT lgFont={};

	BOOL(WINAPI*pSystemParametersInfoForDpi)(UINT,UINT,PVOID,UINT,UINT)=
		(BOOL(WINAPI*)(UINT,UINT,PVOID,UINT,UINT))GetProcAddress(GetModuleHandle(_T("user32")),"SystemParametersInfoForDpi");
	bool bResult=false;
	if(pSystemParametersInfoForDpi!=NULL&&
	   dpiY){
		//iPaddedBorderWidthを含むNONCLIENTMETRICSでないと失敗する
	typedef struct{
		UINT cbSize;
		int iBorderWidth;
		int iScrollWidth;
		int iScrollHeight;
		int iCaptionWidth;
		int iCaptionHeight;
		LOGFONTW lfCaptionFont;
		int iSmCaptionWidth;
		int iSmCaptionHeight;
		LOGFONTW lfSmCaptionFont;
		int iMenuWidth;
		int iMenuHeight;
		LOGFONTW lfMenuFont;
		LOGFONTW lfStatusFont;
		LOGFONTW lfMessageFont;
		int iPaddedBorderWidth;//#if(WINVER >= 0x0600)
	}NONCLIENTMETRICSW_FOR_VISTA;
		NONCLIENTMETRICSW_FOR_VISTA ncm={};
		ncm.cbSize=sizeof(NONCLIENTMETRICSW_FOR_VISTA);

		bResult=pSystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS,sizeof(NONCLIENTMETRICSW_FOR_VISTA),&ncm,0,dpiY);
		lgFont=ncm.lfMessageFont;
	}
	if(!bResult){
		NONCLIENTMETRICS ncm={};
		ncm.cbSize=sizeof(NONCLIENTMETRICS);

		SystemParametersInfo(SPI_GETNONCLIENTMETRICS,sizeof(NONCLIENTMETRICS),&ncm,0);
		lgFont=ncm.lfMessageFont;
	}
	return lgFont;
}

//標準のGUIフォントを作成
HFONT CreateGUIFont(LPCTSTR lpszFontName,int iSize,int iStyle){
	HMONITOR hMonitor=g_Config.GetActiveMonitor();
	UINT dpiX=0,dpiY=0;
	GetDpiForMonitor(hMonitor,&dpiX,&dpiY);

	if(!dpiY){
		HDC hActiveMonitorDC=CreateMonitorDC(hMonitor);
		dpiY=GetDeviceCaps(hActiveMonitorDC,LOGPIXELSY);
		DeleteDC(hActiveMonitorDC);
	}

	LOGFONT lgFont=GetMesssageLOGFONT();
//	if(lgFont.lfHeight<=0){

	if(iSize==0){iSize=(lgFont.lfHeight>0)?lgFont.lfHeight:0-lgFont.lfHeight;}

//abs
lgFont.lfHeight=
-MulDiv(iSize,
		    dpiY,72);
//	}

	if(lstrlen(lpszFontName))lstrcpy(lgFont.lfFaceName,lpszFontName);
//	if(iSize>0)lgFont.lfHeight=iSize;
	if(iStyle&SFONT_BOLD)lgFont.lfWeight=FW_BOLD;
	if(iStyle&SFONT_ITALIC)lgFont.lfItalic=true;
	if(iStyle&SFONT_UNDERLINE)lgFont.lfUnderline=true;
	if(iStyle&SFONT_STRIKEOUT)lgFont.lfStrikeOut=true;

	return CreateFontIndirect(&lgFont);
}

//論理フォントをコントロールに適用
void SetFont(HWND hWnd,LPCTSTR lpszFontName,int iSize,int iStyle){
	HFONT hFont=NULL;
	HDC hDC=GetDC(hWnd);

	hFont=CreateGUIFont(lpszFontName,iSize,iStyle);
	SetBkMode(hDC,OPAQUE);
	SendMessage(hWnd,WM_SETFONT,(WPARAM)hFont,true);
	ReleaseDC(hWnd,hDC);
	return;
}

//子孫ウインドウへのSendMessage()
void SendMessageToDescendants(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam,bool bDeep){
	for(HWND hChild=GetTopWindow(hWnd);hChild!=NULL;hChild=GetNextWindow(hChild,GW_HWNDNEXT)){
		SendMessage(hChild,uMsg,wParam,lParam);
		if(bDeep&&GetTopWindow(hChild)!=NULL){
			SendMessageToDescendants(hChild,uMsg,wParam,lParam,bDeep);
		}
	}
}

//ホットキー登録
bool RegistHotKey(HWND hWnd,int iItemId,WORD* pwHotKey){
	bool bResult=false;

	if(*pwHotKey){
		if(RegisterHotKey(hWnd,iItemId,LOBYTE(*pwHotKey),HIBYTE(*pwHotKey))){
			bResult=true;
		}else{
			//ホットキー登録に失敗
			MessageBox(hWnd,_T("ホットキーの登録に失敗しました"),NULL,MB_OK|MB_ICONERROR);
			*pwHotKey=0;
			bResult=false;
		}
	}
	return bResult;
}

//ホットキー登録解除
bool UnregistHotKey(HWND hWnd,int iItemId,WORD* pwHotKey){
	if(*pwHotKey){
		return UnregisterHotKey(hWnd,iItemId)!=0;
	}
	return false;
}

//ウインドウをフォアグラウンドに持ってくる
bool SetForegroundWindowEx(HWND hWnd){
	bool bResult=false;
	if(IsHungAppWindow(hWnd))return false;

	//最大化→最小化したウインドウに対して
	//SendMessage(hWnd,WM_SYSCOMMAND,SC_RESTORE,0);
	//SwitchToThisWindow(hWnd,true);
	//すると最大化が解除されて切り替わるので注意する

	//Tascher本体をアクティブにする時はウインドウをクリックする処理を併用しています
	SwitchToThisWindow(hWnd,true);

	return bResult;
}

//最前面ウインドウかどうか
bool IsWindowTopMost(HWND hWnd){
	return (GetWindowLongPtr(hWnd,GWL_EXSTYLE)&WS_EX_TOPMOST)!=0;
}

bool IsStoreApp(HWND hWnd){
	bool bResut=false;
	DWORD dwProcessId=0;

	GetWindowThreadProcessId(hWnd,&dwProcessId);
	HANDLE hProcess=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,dwProcessId);

	if(hProcess==NULL)return false;

	LONG(WINAPI*pGetPackageFullName)(HANDLE,UINT32*,PWSTR)=
		(LONG(WINAPI*)(HANDLE,UINT32*,PWSTR))GetProcAddress(GetModuleHandle(_T("kernel32")),"GetPackageFullName");

	if(pGetPackageFullName==NULL){
		CloseHandle(hProcess);
		return false;
	}

	UINT32 uiFullPackageNameLength=0;

#ifndef APPMODEL_ERROR_NO_PACKAGE
	#define APPMODEL_ERROR_NO_PACKAGE 15700L
#endif

	bResut=pGetPackageFullName(hProcess,&uiFullPackageNameLength,NULL)!=APPMODEL_ERROR_NO_PACKAGE&&
		uiFullPackageNameLength;

	CloseHandle(hProcess);
	return bResut;
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
	com::Initialize ComInitialize;
	IShellDispatch4* pShellDispatch=NULL;
	if(SUCCEEDED(CoCreateInstance(CLSID_Shell,NULL,CLSCTX_SERVER,IID_IDispatch,(LPVOID*)&pShellDispatch))){
		pShellDispatch->ToggleDesktop();
	}
	pShellDispatch->Release();
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
		aKey.cmd=(WORD)sKeyTable[i].uCmdId;
		aKeyTable[i]=aKey;
	}

	if(hAccel!=NULL)DestroyAcceleratorTable(hAccel);
	hAccel=CreateAcceleratorTable(aKeyTable,MAX_SHORTCUTKEY);
	return hAccel;
}

//プロセスのファイル名を取得
bool GetProcessFileName(DWORD dwProcessId,LPTSTR pszFileName,DWORD dwFileNameLength){
	bool bResult=false;

	HANDLE hProcess=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,false,dwProcessId);
	if(hProcess==NULL&&
	   GetLastError()==ERROR_ACCESS_DENIED){
		hProcess=OpenProcess(PROCESS_QUERY_INFORMATION,false,dwProcessId);
	}

	if(hProcess==NULL)return false;

	BOOL(WINAPI*pQueryFullProcessImageName)(HANDLE,DWORD,LPTSTR,PDWORD)=
		(BOOL(WINAPI*)(HANDLE,DWORD,LPTSTR,PDWORD))GetProcAddress(GetModuleHandle(_T("kernel32")),"QueryFullProcessImageNameW");

	if(pQueryFullProcessImageName!=NULL){
		bResult=pQueryFullProcessImageName(hProcess,0,pszFileName,&dwFileNameLength)!=0;
	}
	if(!bResult){
		//GetProcessImageFileName()はドライブレターではなくデバイス名で取得するため、変換用テーブルを作成
		struct DEVICE_TABLE{
			TCHAR szDrive[4];
			TCHAR szDevice[MAX_PATH];
		};
		DEVICE_TABLE* pDeviceTable=(DEVICE_TABLE*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,27*sizeof(DEVICE_TABLE));
		if(pDeviceTable==NULL){
			CloseHandle(hProcess);
			return false;
		}

		for(int i=0,iDriveCount=L'A';iDriveCount<='Z';iDriveCount++,i++){
			TCHAR szDrive[4]={(TCHAR)iDriveCount,':','\0','\0'};

			if(QueryDosDevice(szDrive,pDeviceTable[i].szDevice,ARRAY_SIZEOF(pDeviceTable[i].szDevice))){
				lstrcpy(pDeviceTable[i].szDrive,szDrive);
			}
		}

		bResult=GetProcessImageFileName(hProcess,pszFileName,dwFileNameLength)!=0;

		for(int i=0;i<27;i++){
			TCHAR* p=pszFileName;
			if(p&&StrStr(p,pDeviceTable[i].szDevice)==p){
				StrReplace(pszFileName,pDeviceTable[i].szDevice,pDeviceTable[i].szDrive);
			}
		}
		HeapFree(GetProcessHeap(),0,pDeviceTable);
		pDeviceTable=NULL;
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
	HANDLE hSnapshot=NULL;
	PROCESSENTRY32 ProcessEntry32={sizeof(PROCESSENTRY32)};

	if((hSnapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0))!=INVALID_HANDLE_VALUE){
		if(Process32First(hSnapshot,&ProcessEntry32)){
			int iNumber=0;

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
void FreeProcessList(PROCESSENTRY32** ppProcessEntry32){
	if(*ppProcessEntry32!=NULL){
		HeapFree(GetProcessHeap(),0,*ppProcessEntry32);
		*ppProcessEntry32=NULL;
	}
}

#ifndef NT_ERROR
#define NT_ERROR(Status) ((((ULONG)(Status))>>30)==3)
#endif

//特定のプロセスのコマンドラインを取得
LPTSTR AllocProcessCommandLine(DWORD dwProcessId){
/*
	//winternl.h
	typedef struct _RTL_USER_PROCESS_PARAMETERS{
		BYTE Reserved1[16];
		PVOID Reserved2[10];
		UNICODE_STRING ImagePathName;
		UNICODE_STRING CommandLine;
	}RTL_USER_PROCESS_PARAMETERS,*PRTL_USER_PROCESS_PARAMETERS;

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
*/
	TCHAR* pszCommandLine=NULL;

	HANDLE hProcess=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,false,dwProcessId);
	if(hProcess==NULL)return pszCommandLine;

	NTSTATUS(NTAPI*pNtQueryInformationProcess)(
		HANDLE ProcessHandle,
		PROCESSINFOCLASS ProcessInformationClass,
		PVOID ProcessInformation,
		ULONG ProcessInformationLength,
		PULONG ReturnLength)=
		(NTSTATUS(NTAPI*)(HANDLE,PROCESSINFOCLASS,PVOID,ULONG,PULONG))GetProcAddress(GetModuleHandle(_T("ntdll")),"NtQueryInformationProcess");


	ULONG(NTAPI*pRtlNtStatusToDosError)(NTSTATUS)=
		(ULONG(NTAPI*)(NTSTATUS))GetProcAddress(GetModuleHandle(_T("ntdll")),"RtlNtStatusToDosError");

	if(pNtQueryInformationProcess==NULL||pRtlNtStatusToDosError==NULL){
		CloseHandle(hProcess);
		return pszCommandLine;
	}

	PROCESS_BASIC_INFORMATION pbi={};
	ULONG uLength=0;
	NTSTATUS status=pNtQueryInformationProcess(hProcess,ProcessBasicInformation,&pbi,sizeof(pbi),&uLength);

	SetLastError(pRtlNtStatusToDosError(status));
	if(NT_ERROR(status)||!pbi.PebBaseAddress){
		CloseHandle(hProcess);
		return pszCommandLine;
	}

	SIZE_T nNumberOfBytesRead=0;
	_PEB peb={};

	if(!ReadProcessMemory(hProcess,pbi.PebBaseAddress,&peb,sizeof(peb),&nNumberOfBytesRead)){
		CloseHandle(hProcess);
		return pszCommandLine;
	}

	_RTL_USER_PROCESS_PARAMETERS upp={};

	if(ReadProcessMemory(hProcess,peb.ProcessParameters,&upp,sizeof(upp),&nNumberOfBytesRead)&&
	   upp.CommandLine.Length){
		pszCommandLine=(TCHAR*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(upp.CommandLine.Length+1)*sizeof(TCHAR));

		if(pszCommandLine){
			ReadProcessMemory(hProcess,upp.CommandLine.Buffer,pszCommandLine,upp.CommandLine.Length,&nNumberOfBytesRead);
		}
	}
	CloseHandle(hProcess);
	return pszCommandLine;
}

//AllocProcessCommandLine()で取得したコマンドラインを解放
void FreeProcessCommandLine(TCHAR** ppszCommandLine){
	if(*ppszCommandLine!=NULL){
		HeapFree(GetProcessHeap(),0,*ppszCommandLine);
		*ppszCommandLine=NULL;
	}
}

//管理者権限で実行中かどうか
bool IsAdministratorProcess(DWORD dwProcessId){
	bool bAdministrator=false;
	HANDLE hToken=NULL;
	TOKEN_ELEVATION te={};

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
	HANDLE hProcess=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,dwProcessId);

	LONG(WINAPI*pGetApplicationUserModelId)(HANDLE,UINT32*,PWSTR)=
		(LONG(WINAPI*)(HANDLE,UINT32*,PWSTR))GetProcAddress(GetModuleHandle(_T("kernel32")),"GetApplicationUserModelId");

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
	com::Initialize ComInitialize;
	IPropertyStore *pPropertyStore=NULL;

	HRESULT(WINAPI*pSHGetPropertyStoreForWindow)(HWND,REFIID,void**)=
		(HRESULT(WINAPI*)(HWND,REFIID,void**))GetProcAddress(GetModuleHandle(_T("shell32")),"SHGetPropertyStoreForWindow");

	if(pSHGetPropertyStoreForWindow&&
	   SUCCEEDED(pSHGetPropertyStoreForWindow(hWnd,IID_PPV_ARGS(&pPropertyStore)))){
		PROPVARIANT pvAppUserModelId={};

		PropVariantInit(&pvAppUserModelId);
		if(SUCCEEDED(pPropertyStore->GetValue(PKEY_AppUserModel_ID,&pvAppUserModelId))){
			HMODULE hPropsys=LoadLibrary(_T("Propsys.dll"));

			if(hPropsys!=NULL){
				HRESULT(WINAPI*pPropVariantToString)(REFPROPVARIANT,PWSTR,UINT)=
					(HRESULT(WINAPI*)(REFPROPVARIANT,PWSTR,UINT))GetProcAddress(hPropsys,"PropVariantToString");
				if(pPropVariantToString&&
				   SUCCEEDED(pPropVariantToString(pvAppUserModelId,pszAppUserModelId,uiBufferLength))){
					bResult=true;
				}
				FreeLibrary(hPropsys);
			}
		}
		PropVariantClear(&pvAppUserModelId);

		pPropertyStore->Release();
	}
	return bResult;
}

//キー入力を登録する
void SetKeybrd(LPINPUT lpKey,WORD wVKey,bool KeyDown){
	if(lpKey!=NULL){
		lpKey->type=INPUT_KEYBOARD;
		lpKey->ki.wVk=wVKey;
		lpKey->ki.wScan=(WORD)MapVirtualKey(wVKey,0);
		lpKey->ki.dwFlags=(KeyDown?0:KEYEVENTF_KEYUP);
		lpKey->ki.dwExtraInfo=0;
		lpKey->ki.time=0;
	}
}

//キー入力を行う
void ExecKey(WORD wKey,WORD wVKey){
	INPUT Key[6]={};
	LPINPUT lpKey=Key;
	SetKeybrd(lpKey++,wKey,false);
	if(wVKey){
		SetKeybrd(lpKey++,wVKey,false);
		SetKeybrd(lpKey++,wVKey,true);
	}
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
	com::Initialize ComInitialize;
	LPITEMIDLIST lpidlDesktop=NULL;
	IShellFolder* pIShellFolder=NULL;
	IExtractIcon* pIExtractIcon=NULL;

	if(FAILED(SHGetSpecialFolderLocation(NULL,CSIDL_DESKTOP,&lpidlDesktop)))return NULL;
	if(SUCCEEDED(SHGetDesktopFolder(&pIShellFolder))){
		if(SUCCEEDED(pIShellFolder->GetUIObjectOf(NULL,1,(LPCITEMIDLIST*)&lpidlDesktop,IID_IExtractIcon,0,(void**)&pIExtractIcon))){
			int iIndex=0;
			UINT uFlags=0;
			TCHAR szLocation[MAX_PATH]={};

			if(SUCCEEDED(pIExtractIcon->GetIconLocation(GIL_FORSHELL,szLocation,MAX_PATH,&iIndex,&uFlags))){
				pIExtractIcon->Extract(szLocation,iIndex,(!bSmall)?&hIconLarge:NULL,(bSmall)?&hIconSmall:NULL,MAKELONG(32,16));
			}
		}
		pIExtractIcon->Release();
		pIShellFolder->Release();
		CoTaskMemFree(lpidlDesktop);
	}
	return (!bSmall)?hIconLarge:hIconSmall;
}

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

//UWPAppのアイコンを取得する
bool GetUWPAppIconInfo(HWND hWnd,TCHAR* szLogoPath,bool* pbClrBackground,COLORREF* pclrBackground=NULL){
	if(hWnd==NULL||szLogoPath==NULL)return false;

	//MsXml6.h
	class DECLSPEC_UUID("88d96a05-f192-11d4-a65f-0040963251e5")DOMDocument60;

	//appmodel.h
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
		UINT32 reserved;
		UINT32 processorArchitecture;
		PACKAGE_VERSION version;
		PWSTR name;
		PWSTR publisher;
		PWSTR resourceId;
		PWSTR publisherId;
	}PACKAGE_ID;

	com::Initialize ComInitialize;
	DWORD dwProcessId=0;

	GetWindowThreadProcessId(hWnd,&dwProcessId);
	HANDLE hProcess=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,dwProcessId);

	if(hProcess==NULL)return false;

	PTSTR pszFullPackageName=NULL;
	PTSTR pszPackagePath=NULL;
	BYTE* pbyPackageId=NULL;
	UINT32 uiFullPackageNameLength=0,uiPackagePathLength=0,uiPackageIdLength=0;

	LONG(WINAPI*pGetPackageFullName)(HANDLE,UINT32*,PWSTR)=
		(LONG(WINAPI*)(HANDLE,UINT32*,PWSTR))GetProcAddress(GetModuleHandle(_T("kernel32")),"GetPackageFullName");
	LONG(WINAPI*pGetPackagePathByFullName)(PCWSTR,UINT32*,PWSTR)=
		(LONG(WINAPI*)(PCWSTR,UINT32*,PWSTR))GetProcAddress(GetModuleHandle(_T("kernel32")),"GetPackagePathByFullName");
	LONG(WINAPI*pPackageIdFromFullName)(PCWSTR,const UINT32,UINT32*,BYTE*)=
		(LONG(WINAPI*)(PCWSTR,const UINT32,UINT32*,BYTE*))GetProcAddress(GetModuleHandle(_T("kernel32")),"PackageIdFromFullName");

	if(pGetPackageFullName==NULL||pGetPackagePathByFullName==NULL||pPackageIdFromFullName==NULL){
		CloseHandle(hProcess);
		return false;
	}
	LONG lResult=pGetPackageFullName(hProcess,&uiFullPackageNameLength,NULL);
	if(lResult==ERROR_INSUFFICIENT_BUFFER&&uiFullPackageNameLength){
		pszFullPackageName=(PTSTR)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,uiFullPackageNameLength*sizeof(TCHAR));
	}
	if(pszFullPackageName==NULL){
		CloseHandle(hProcess);
		return false;
	}
	if(pGetPackageFullName(hProcess,&uiFullPackageNameLength,pszFullPackageName)!=ERROR_SUCCESS||
	   pszFullPackageName==NULL||
	   !uiFullPackageNameLength){
		HeapFree(GetProcessHeap(),0,pszFullPackageName);
		CloseHandle(hProcess);
		return false;
	}

	lResult=pGetPackagePathByFullName(pszFullPackageName,&uiPackagePathLength,NULL);
	if(lResult==ERROR_INSUFFICIENT_BUFFER&&uiPackagePathLength){
		pszPackagePath=(PTSTR)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,uiPackagePathLength*sizeof(TCHAR));
	}
	if(pszPackagePath==NULL){
		HeapFree(GetProcessHeap(),0,pszFullPackageName);
		CloseHandle(hProcess);
		return false;
	}
	if(pGetPackagePathByFullName(pszFullPackageName,&uiPackagePathLength,pszPackagePath)!=ERROR_SUCCESS||
	   pszPackagePath==NULL||
	   !uiPackagePathLength){
		HeapFree(GetProcessHeap(),0,pszFullPackageName);
		HeapFree(GetProcessHeap(),0,pszPackagePath);
		CloseHandle(hProcess);
		return false;
	}
	lResult=pPackageIdFromFullName(pszFullPackageName,0,&uiPackageIdLength,NULL);
	if(lResult==ERROR_INSUFFICIENT_BUFFER&&uiPackageIdLength){
		pbyPackageId=(BYTE*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,uiPackageIdLength);
	}
	if(pbyPackageId==NULL){
		HeapFree(GetProcessHeap(),0,pszFullPackageName);
		HeapFree(GetProcessHeap(),0,pszPackagePath);
		CloseHandle(hProcess);
		return false;
	}
	lResult=pPackageIdFromFullName(pszFullPackageName,0,&uiPackageIdLength,pbyPackageId);

	bool bResult=false;
	HRESULT hResult=E_FAIL;
	IXMLDOMDocument* pXMLDocument=NULL;

	if(lResult==ERROR_SUCCESS&&pbyPackageId&&uiPackageIdLength){
		hResult=CoCreateInstance(__uuidof(DOMDocument60),NULL,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&pXMLDocument));
	}
	if(SUCCEEDED(hResult)&&
	   pXMLDocument!=NULL){
		pXMLDocument->put_async(VARIANT_FALSE);
		pXMLDocument->put_validateOnParse(VARIANT_FALSE);
		pXMLDocument->put_resolveExternals(VARIANT_FALSE);

		TCHAR szAppxManifestPath[MAX_PATH]={};
		VARIANT_BOOL varStatus=FALSE;
		VARIANT varFileName={};

		lstrcpy(szAppxManifestPath,pszPackagePath);
		if(szAppxManifestPath[lstrlen(szAppxManifestPath)-1]!='\\')lstrcat(szAppxManifestPath,_T("\\"));
		lstrcat(szAppxManifestPath,_T("AppxManifest.xml"));
		VariantInit(&varFileName);

		hResult=VariantFromString(szAppxManifestPath,varFileName);
		if(SUCCEEDED(hResult)){
			hResult=pXMLDocument->load(varFileName,&varStatus);
		}

		IXMLDOMNode* pNode=NULL;
		BSTR bstrNodeName=SysAllocString(_T("//*[local-name()='VisualElements']"));

		if(SUCCEEDED(hResult)&&
			varStatus==VARIANT_TRUE){

			hResult=pXMLDocument->selectSingleNode(bstrNodeName,&pNode);
		}

		IXMLDOMNamedNodeMap* pXMLNamedNodeMap=NULL;

		if(SUCCEEDED(hResult)&&pNode){
			hResult=pNode->get_attributes(&pXMLNamedNodeMap);
		}

		if(SUCCEEDED(hResult)&&pXMLNamedNodeMap){
			BSTR bstrLogos[]={SysAllocString(_T("Square44x44Logo")),SysAllocString(_T("Square30x30Logo")),SysAllocString(_T("SmallLogo"))};

			for(int i=0;i<ARRAY_SIZEOF(bstrLogos);i++){
				IXMLDOMNode* pLogoNode=NULL;

				if(FAILED(pXMLNamedNodeMap->getNamedItem(bstrLogos[i],&pLogoNode))||pLogoNode==NULL)continue;

				VARIANT varLogo={};
				VariantInit(&varLogo);
				if(SUCCEEDED(pLogoNode->get_nodeValue(&varLogo))){
					wsprintf(szLogoPath,_T("@{%s?ms-resource:///Files/%s}"),pszFullPackageName,varLogo.bstrVal);
//					for(int ii=0,length=lstrlen(szLogoPath);ii<length;ii++)if(szLogoPath[ii]=='\\')szLogoPath[ii]='/';
					hResult=SHLoadIndirectString(szLogoPath,szLogoPath,MAX_PATH,NULL);

					if(pclrBackground!=NULL){
						IXMLDOMNode* pBackgroundColorNode=NULL;
						BSTR bstrBackgroundColor=SysAllocString(_T("BackgroundColor"));

						hResult=pXMLNamedNodeMap->getNamedItem(bstrBackgroundColor,&pBackgroundColorNode);
						if(SUCCEEDED(hResult)&&pBackgroundColorNode){
							VARIANT varBackgroundColor={};
							VariantInit(&varBackgroundColor);

							hResult=pBackgroundColorNode->get_nodeValue(&varBackgroundColor);
							if(SUCCEEDED(hResult)&&varBackgroundColor.bstrVal[0]=='#'){
								*pclrBackground=StringToCOLORREF(varBackgroundColor.bstrVal);
								if(pbClrBackground!=NULL)*pbClrBackground=true;
							}
							VariantClear(&varBackgroundColor);
						}
						SAFE_RELEASE(pBackgroundColorNode);
						SysFreeString(bstrBackgroundColor);
					}
					bResult=SUCCEEDED(hResult);
					break;
				}
				VariantClear(&varLogo);
			}
			for(int i=0;i<ARRAY_SIZEOF(bstrLogos);i++)SysFreeString(bstrLogos[i]);
		}
		SAFE_RELEASE(pXMLNamedNodeMap);
		SAFE_RELEASE(pNode);
		SysFreeString(bstrNodeName);
		VariantClear(&varFileName);
	}
	SAFE_RELEASE(pXMLDocument);

	HeapFree(GetProcessHeap(),0,pbyPackageId);
	HeapFree(GetProcessHeap(),0,pszPackagePath);
	HeapFree(GetProcessHeap(),0,pszFullPackageName);
	CloseHandle(hProcess);
	return bResult;
}

//UWPAppのBitmapを取得する
bool GetUWPAppIconD2D1(ID2D1RenderTarget* pRenderTarget,HWND hWnd,ID2D1Bitmap** ppResultBitmap,bool* pbClrBackground,COLORREF* pclrBackground){
	if(hWnd==NULL||pRenderTarget==NULL||ppResultBitmap==NULL)return false;

	TCHAR szLogoPath[MAX_PATH]={};

	if(!GetUWPAppIconInfo(hWnd,szLogoPath,pbClrBackground,pclrBackground))return false;

	HRESULT hResult=E_FAIL;
	IWICImagingFactory* pWICImagingFactory=NULL;

	hResult=CoCreateInstance(CLSID_WICImagingFactory,NULL,CLSCTX_INPROC_SERVER,IID_IWICImagingFactory,(void**)(&pWICImagingFactory));

	IWICBitmapDecoder* pWICBitmapDecoder=NULL;

	if(SUCCEEDED(hResult)){
		hResult=pWICImagingFactory->CreateDecoderFromFilename(szLogoPath,NULL,GENERIC_READ,WICDecodeMetadataCacheOnLoad,&pWICBitmapDecoder);
	}

	IWICBitmapFrameDecode* pWICBitmapFrame=NULL;

	if(SUCCEEDED(hResult)){
		hResult=pWICBitmapDecoder->GetFrame(0,&pWICBitmapFrame);
	}

	IWICFormatConverter* pWICFormatConverter=NULL;

	if(SUCCEEDED(hResult)){
		hResult=pWICImagingFactory->CreateFormatConverter(&pWICFormatConverter);
	}
	if(SUCCEEDED(hResult)){
		hResult=pWICFormatConverter->Initialize(pWICBitmapFrame,GUID_WICPixelFormat32bppPBGRA,WICBitmapDitherTypeNone,NULL,0.0f,WICBitmapPaletteTypeMedianCut);
	}
	if(SUCCEEDED(hResult)){
		hResult=pRenderTarget->CreateBitmapFromWicBitmap(pWICFormatConverter,NULL,ppResultBitmap);
	}

	SAFE_RELEASE(pWICFormatConverter);
	SAFE_RELEASE(pWICBitmapFrame);
	SAFE_RELEASE(pWICBitmapDecoder);
	SAFE_RELEASE(pWICImagingFactory);
	return SUCCEEDED(hResult);
}

//UWPAppのアイコンを取得する
HICON GetUWPAppIcon(HWND hWnd,UINT uiIconSize=32,bool bNoBackgroundColor=false){
	if(hWnd==NULL)return NULL;

	HICON hIcon=NULL;
	TCHAR szLogoPath[MAX_PATH]={};
	COLORREF clrBackground;

	if(!GetUWPAppIconInfo(hWnd,szLogoPath,NULL,(!bNoBackgroundColor)?&clrBackground:NULL))return false;

	Gdiplus::Bitmap bBitmapOrig(szLogoPath);
	Gdiplus::Bitmap bBitmap(uiIconSize,uiIconSize);
	Gdiplus::Graphics gGraphics(&bBitmap);

	if(!bNoBackgroundColor){
		gGraphics.FillRectangle(new Gdiplus::SolidBrush(Gdiplus::Color(GetRValue(clrBackground),GetGValue(clrBackground),GetBValue(clrBackground))),0,0,bBitmap.GetWidth(),bBitmap.GetHeight());
	}
	gGraphics.DrawImage(&bBitmapOrig,0,0,uiIconSize,uiIconSize);
	bBitmap.GetHICON(&hIcon);

	return hIcon;
}

//実行ファイルからアイコンを取得
HICON GetIcon(HWND hWnd,UINT uIconSize,bool bUWPApp,bool bUWPAppNoBackgroundColor){
	HICON hIcon=NULL;

	if(hWnd==NULL||IsHungAppWindow(hWnd))return NULL;

	if(bUWPApp||IsStoreApp(hWnd)){
		hIcon=GetUWPAppIcon(hWnd,uIconSize,bUWPAppNoBackgroundColor);
	}
	if(hIcon==NULL){
		TCHAR szFilePath[MAX_PATH]={};

		if(GetFileNameFromWindowHandle(hWnd,szFilePath,ARRAY_SIZEOF(szFilePath))){
			PrivateExtractIcons(szFilePath,0,uIconSize,uIconSize,&hIcon,NULL,1,0);
		}
	}

	return hIcon;
}

//あの手この手でアイコンを取得[小さい]
HICON GetSmallIconFromWindow(HWND hWnd){
	HICON hIcon=NULL;

	if(hWnd==NULL||IsHungAppWindow(hWnd))return NULL;

	if(!SendMessageTimeout(hWnd,WM_GETICON,ICON_SMALL,0,SMTO_ABORTIFHUNG,500,(PDWORD_PTR)&hIcon)){
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
		if(IsWindowVisible(hParent))return GetSmallIconFromWindow(hParent);
	}
	return hIcon;
}

//あの手この手でアイコンを取得[大きい]
HICON GetLargeIconFromWindow(HWND hWnd){
	HICON hIcon=NULL;

	if(hWnd==NULL||IsHungAppWindow(hWnd))return NULL;

	if(!SendMessageTimeout(hWnd,WM_GETICON,ICON_BIG,0,SMTO_ABORTIFHUNG,500,(PDWORD_PTR)&hIcon)){
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
		if(IsWindowVisible(hParent))return GetLargeIconFromWindow(hParent);
	}
	return hIcon;
}

HRESULT GetShellViewForDesktop(REFIID riid,void** ppResult){
	*ppResult=NULL;

	IShellWindows* pIShellWindows=NULL;
	HRESULT hResult=CoCreateInstance(CLSID_ShellWindows,NULL,CLSCTX_LOCAL_SERVER,IID_PPV_ARGS(&pIShellWindows));
	if(SUCCEEDED(hResult)){
		HWND hWnd=NULL;
		IDispatch* pIDispatch=NULL;
		VARIANT vEmpty={};

		if(S_OK==pIShellWindows->FindWindowSW(&vEmpty,&vEmpty,SWC_DESKTOP,(long*)&hWnd,SWFO_NEEDDISPATCH,&pIDispatch)){
			IShellBrowser* pIShellBrowser=NULL;

			hResult=IUnknown_QueryService(pIDispatch,SID_STopLevelBrowser,IID_PPV_ARGS(&pIShellBrowser));
			if(SUCCEEDED(hResult)){
				IShellView* pIShellView=NULL;

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

	IDispatch* pIDispatchBackground=NULL;
	HRESULT hResult=pIShellView->GetItemObject(SVGIO_BACKGROUND,IID_PPV_ARGS(&pIDispatchBackground));
	if(SUCCEEDED(hResult)){
		IShellFolderViewDual* pIShellFolderViewDual=NULL;
		hResult=pIDispatchBackground->QueryInterface(IID_PPV_ARGS(&pIShellFolderViewDual));

		if(SUCCEEDED(hResult)){
			IDispatch* pIDispatch=NULL;
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
	com::Initialize ComInitialize;
	IShellView *pIShellView=NULL;

	HRESULT hResult=GetShellViewForDesktop(IID_PPV_ARGS(&pIShellView));
	if(SUCCEEDED(hResult)){
		IShellDispatch2 *pIShellDispatch2=NULL;
		hResult=GetShellDispatchFromView(pIShellView,IID_PPV_ARGS(&pIShellDispatch2));

		if(SUCCEEDED(hResult)){
			BSTR bstrFile=SysAllocString(lpszFile);
			hResult=bstrFile?S_OK:E_OUTOFMEMORY;

			if(SUCCEEDED(hResult)){
				VARIANT varParameters={};
				VARIANT varWorkingDirectory={};
				VARIANT varEmpty={};
				VARIANT varShowCmd={};

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

//ウインドウハンドルからファイル名を取得する
bool GetFileNameFromWindowHandle(HWND hWnd,LPTSTR lpFileName,DWORD dwFileNameLength){
	bool bResult=false;
	DWORD dwProcessId=0;

	if(GetWindowThreadProcessId(hWnd,&dwProcessId)!=0){
		bResult=GetProcessFileName(dwProcessId,lpFileName,dwFileNameLength);
	}
	return bResult;
}

//HeapFree(GetProcessHeap(),0)で解放すること
UINT HICON2DIB(LPBYTE* lpBuffer,HICON hIcon){
	BITMAP bm={};
	ICONINFO iconInfo={};
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
	RECT rc={};

	GetWindowRect(hWnd,&rc);

	return PtInRect(&rc,pt)!=0;
}

#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)

//OSバージョンを取得
bool GetWindowsVersion(PRTL_OSVERSIONINFOW info){
	NTSTATUS(WINAPI*pRtlGetVersion)(PRTL_OSVERSIONINFOW)=
		(NTSTATUS(WINAPI*)(PRTL_OSVERSIONINFOW))GetProcAddress(GetModuleHandle(_T("ntdll")),"RtlGetVersion");

	return (pRtlGetVersion!=NULL&&
			STATUS_SUCCESS==pRtlGetVersion(info));
}

//グラデーション描画(水平方向)
void HorizontalGradient(HDC hDC,const RECT& GradientFill,COLORREF rgbTop,COLORREF rgbBottom){
	GRADIENT_RECT gradientRect={0,1};
	TRIVERTEX triVertext[2]={
		GradientFill.left,
		GradientFill.top,
		(COLOR16)(GetRValue(rgbTop)<<8),
		(COLOR16)(GetGValue(rgbTop)<<8),
		(COLOR16)(GetBValue(rgbTop)<<8),
		0x0000,
		GradientFill.right,
		GradientFill.bottom,
		COLOR16(GetRValue(rgbBottom)<<8),
		COLOR16(GetGValue(rgbBottom)<<8),
		COLOR16(GetBValue(rgbBottom)<<8),
		0x0000
	};
	GdiGradientFill(hDC,triVertext,2,&gradientRect,1,GRADIENT_FILL_RECT_H);
	return;
}

/*
//グラデーション描画(垂直方向)
void VerticalGradient(HDC hDC,const RECT& GradientFill,COLORREF rgbTop,COLORREF rgbBottom){
	GRADIENT_RECT gradientRect={0,1};
	TRIVERTEX triVertext[2]={
		GradientFill.left,
		GradientFill.top,
		(COLOR16)(GetRValue(rgbTop)<<8),
		(COLOR16)(GetGValue(rgbTop)<<8),
		(COLOR16)(GetBValue(rgbTop)<<8),
		0x0000,
		GradientFill.right,
		GradientFill.bottom,
		COLOR16(GetRValue(rgbBottom)<<8),
		COLOR16(GetGValue(rgbBottom)<<8),
		COLOR16(GetBValue(rgbBottom)<<8),
		0x0000
	};
	GdiGradientFill(hDC,triVertext,2,&gradientRect,1,GRADIENT_FILL_RECT_V);
}
*/