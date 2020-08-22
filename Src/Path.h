//Path.h
//パス操作関数

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.63
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/


#ifndef PATH_H
#define PATH_H

#ifdef __cplusplus
namespace path{
#endif

//文字列から文字を検索し、その個数を返す
int CountCharacter(const TCHAR* pszStr,int iChar);
//実行ファイルのディレクトリを取得
bool GetExeDirectory(TCHAR* pszResult,const int iSize);
//ディレクトリであるかどうか
bool IsDirectory(const TCHAR* pszPath);

}//namespace

#endif //PATH_H
