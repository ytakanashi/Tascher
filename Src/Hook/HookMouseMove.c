//HookMouseMove.c
//WM_MOUSEMOVEをフック

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.60
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/

#ifndef _DEBUG
#ifdef _MSC_VER
#pragma comment(linker,"/merge:.data=.text")
#pragma comment(linker,"/section:.text,erw")
#endif
#endif

#include"../StdAfx.h"
#include"HookMouseMove.h"


UINT WM_HOOKDIALOG;
HHOOK g_hook_handle;
HINSTANCE g_instance_handle;
DWORD g_hook_process_id;
HWND g_post_wnd;

//WH_MOUSE_LLのコールバック
LRESULT CALLBACK LowLevelMouseHookProc(int code,WPARAM wParam,LPARAM lParam){
	if(g_hook_process_id==GetCurrentProcessId()&&
	   code==HC_ACTION){
		if(wParam==WM_MOUSEMOVE){
			MSLLHOOKSTRUCT* pMLLHS=(MSLLHOOKSTRUCT*)lParam;
			PostMessage(g_post_wnd,WM_MOUSEMOVE,pMLLHS->pt.x,pMLLHS->pt.y);
		}
	}

	return CallNextHookEx(NULL/*This parameter is ignored.*/,code,wParam,lParam);
}

//WH_MOUSE_LLフックをインストール
BOOL DLL_EXPORT installMouseLLHook(const DWORD hook_process_id,HWND post_wnd){
	if(post_wnd==0||hook_process_id==0)return FALSE;

	if((g_hook_handle=SetWindowsHookEx(WH_MOUSE_LL,LowLevelMouseHookProc,g_instance_handle,0))!=NULL){
		g_hook_process_id=hook_process_id;
		g_post_wnd=post_wnd;
		return TRUE;
	}
	return FALSE;
}

//フックをアンインストール
BOOL DLL_EXPORT uninstallMouseLLHook(){
	if(g_hook_handle!=NULL&&
	   UnhookWindowsHookEx(g_hook_handle)!=0){
		//WM_NULLを投げると7-zip32.dllが処理中断してしまうので注意
//		PostMessage(HWND_BROADCAST,WM_NULL,0,0);
		g_hook_handle=NULL;
		return TRUE;
	}
	return FALSE;
}

#ifdef __cplusplus
//extern "C"必須
extern "C"{
#endif

BOOL WINAPI DllMain(HINSTANCE instance_handle, DWORD reason, LPVOID lpvReserved){
	switch(reason){
		case DLL_PROCESS_ATTACH:
			// attach to process
			// return FALSE to fail DLL load
			//Dllのハンドルを保存
			g_instance_handle=instance_handle;
			//DLL_THREAD_ATTACHとDLL_THREAD_DETACHを無効にする
			DisableThreadLibraryCalls(instance_handle);
			break;

		case DLL_PROCESS_DETACH:
			// detach from process
			break;

		case DLL_THREAD_ATTACH:
			// attach to thread
			break;

		case DLL_THREAD_DETACH:
			// detach from thread
			break;
	}
	return TRUE;// succesful
}

#ifdef __cplusplus
}
#endif
