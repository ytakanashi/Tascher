//WebBrowserTab.h
//ブラウザタブ取得

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.64
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/


#ifndef TASCHER_WEBBROWSERTAB_H
#define TASCHER_WEBBROWSERTAB_H

#include"Allocator.h"
#include"COM.h"


namespace webbrowsertab{
class ChromeDevToolsProtocol;

struct TAB_DATA{
	TCHAR* pszType;
	TCHAR* pszId;
	TCHAR* pszTitle;
	TCHAR* pszURL;
	TCHAR* pszFaviconURL;
	TAB_DATA():pszType(NULL),pszId(NULL),pszTitle(NULL),pszURL(NULL),pszFaviconURL(NULL){}
	void init(){pszType=NULL;pszId=NULL;pszTitle=NULL;pszURL=NULL;pszFaviconURL=NULL;}
};

class List:public Allocator{
public:
	List();
	~List();
	bool Initialize(TCHAR* pszTargetProcessPath,DWORD* pdwTargetPort=NULL);
	void Uninitialize();
	TAB_DATA* Alloc(int* piCount,DWORD* pdwProcessId=NULL);
	void Free(TAB_DATA** ppList);
	bool Activate(TCHAR* pszId);
	bool Close(TCHAR* pszId);
private:
	ChromeDevToolsProtocol* pChromeDevToolsProtocol;
};


}//namespace webbrowsertab

#endif //TASCHER_WEBBROWSERTAB_H
