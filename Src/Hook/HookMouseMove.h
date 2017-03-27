//HookMouseMove.h


/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.62
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/


#ifndef _HOOKMOUSEMOVE_H_E9EB4B70_B490_45BE_BF59_CF333996E66E
#define _HOOKMOUSEMOVE_H_E9EB4B70_B490_45BE_BF59_CF333996E66E



#ifdef BUILD_DLL
	#define DLL_EXPORT __declspec(dllexport)
#else
	#define DLL_EXPORT //__declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C"{
#endif
//WH_MOUSE_LLフックをインストール
BOOL DLL_EXPORT instalMouseLLlHook(const DWORD hook_process_id,HWND post_wnd);
//フックをアンインストール
BOOL DLL_EXPORT uninstallMouseLLHook();

#ifdef __cplusplus
}
#endif

#endif //_HOOKMOUSEMOVE_H_E9EB4B70_B490_45BE_BF59_CF333996E66E
