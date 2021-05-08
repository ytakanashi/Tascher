//Path.cpp
//パス操作関数

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.64
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/


#include"StdAfx.h"
#include"Path.h"
#include<shlwapi.h>

#ifdef __cplusplus
namespace path{
#endif

//文字列から文字を検索し、その個数を返す
int CountCharacter(const TCHAR* pszStr,int iChar){
	int i=0,iResult=0;

	for(i=0;pszStr[i]!=_T('\0');++i){
		#ifndef UNICODE
			if(IsDBCSLeadByte(pszStr[i])){
				++i;
				continue;
			}
		#endif
		if(pszStr[i]==iChar)iResult++;
	}
	return iResult;
}

//実行ファイルのディレクトリを取得
//C:\foo\bar\baz.exe->C:\foo\bar
bool GetExeDirectory(TCHAR* pszResult,const int iSize){
	bool bResult=false;

	if(GetModuleFileName(NULL,pszResult,iSize)){
		if(PathRemoveFileSpec(pszResult))bResult=true;
	}
	return bResult;
}

}//namespace
