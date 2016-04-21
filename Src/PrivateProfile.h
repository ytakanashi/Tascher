//PrivateProfile.h
//cfgファイル操作

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.61
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/


#ifndef PRIVATEPROFILE_H
#define PRIVATEPROFILE_H

#include"CommonSettings.h"


//インスタンスハンドル
extern HINSTANCE g_hInstance;

//cfgファイルパスを取得
void GetPrivateProfilePath(void);

//PrivateProfileファイルへ書き込む
void WritePrivateProfile(HWND hWnd);

//PrivateProfileファイルから読み込む
void ReadPrivateProfile(void);

#endif //PRIVATEPROFILE_H
