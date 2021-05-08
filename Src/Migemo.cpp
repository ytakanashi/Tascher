//Migemo.cpp
//Migemoを使用した正規表現

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.64
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/


#include"StdAfx.h"
#include"Migemo.h"
#include"Path.h"

namespace migemo{

//Migemo,bregexp
HMODULE g_hMigemo;
HMODULE g_hBregonig;

//Migemo
#define MIGEMO_OPINDEX_NEWLINE 5
typedef struct _migemo migemo;
typedef migemo*(__stdcall*MIGEMO_OPEN)(const char* dict);
typedef void(__stdcall*MIGEMO_CLOSE)(migemo* object);
typedef unsigned char*(__stdcall*MIGEMO_QUERY)(migemo* object,const unsigned char* query);
typedef void(__stdcall*MIGEMO_RELEASE)(migemo* object,unsigned char* string);
typedef int(__stdcall*MIGEMO_LOAD)(migemo* obj,int dict_id,const char* dict_file);
typedef int(__stdcall*MIGEMO_IS_ENABLE)(migemo* obj);
typedef int(__stdcall*MIGEMO_SET_OPERATOR)(migemo* object,int index,const unsigned char* op);
typedef int (*MIGEMO_PROC_INT2CHAR)(unsigned int, unsigned char*);
typedef void(__stdcall*MIGEMO_SETPROC_INT2CHAR)(migemo* object,MIGEMO_PROC_INT2CHAR proc);
MIGEMO_OPEN migemo_open;
MIGEMO_CLOSE migemo_close;
MIGEMO_QUERY migemo_query;
MIGEMO_RELEASE migemo_release;
MIGEMO_LOAD migemo_load;
MIGEMO_IS_ENABLE migemo_is_enable;
MIGEMO_SET_OPERATOR migemo_set_operator;
MIGEMO_SETPROC_INT2CHAR migemo_setproc_int2char;

#define BREGEXP_MAX_ERROR_MESSAGE_LEN 80

int(__stdcall*BMatch)(TCHAR* str,TCHAR* target,TCHAR* targetendp,BREGEXP** rxp,TCHAR* msg);
void(__stdcall*BRegfree)(BREGEXP* rx);

//Migemoオブジェクト
migemo* g_pmigemo;

int int2char(unsigned int in,unsigned char* out){
	if(in<0x80||!out){
		switch(in){
			//デフォルトではエスケープされない文字を処理
			case '[':
			case ']':
			case '~':
				if(out){
					out[0]='\\';
					out[1]=in&0xFF;
				}
				return 2;
		}
		//default_int2charに投げる
		return 0;
	}

	//UTF-8向け処理
	if(in<0x800){
		out[0]=0xc0+(in>>6);
		out[1]=0x80+(in&0x3f);
		return 2;
	}else if(in<0x10000){
		out[0]=0xe0+(in>>12);
		out[1]=0x80+((in>>6)&0x3f);
		out[2]=0x80+(in&0x3f);
		return 3;
	}else if(in<0x200000){
		out[0]=0xf0+(in>>18);
		out[1]=0x80+((in>>12)&0x3f);
		out[2]=0x80+((in>>6)&0x3f);
		out[3]=0x80+(in&0x3f);
		return 4;
	}else if(in<0x4000000){
		out[0]=0xf8+(in>>24);
		out[1]=0x80+((in>>18)&0x3f);
		out[2]=0x80+((in>>12)&0x3f);
		out[3]=0x80+((in>>6)&0x3f);
		out[4]=0x80+(in&0x3f);
		return 5;
	}else{
		out[0]=0xfc+(in>>30);
		out[1]=0x80+((in>>24)&0x3f);
		out[2]=0x80+((in>>18)&0x3f);
		out[3]=0x80+((in>>12)&0x3f);
		out[4]=0x80+((in>>6)&0x3f);
		out[5]=0x80+(in&0x3f);
		return 6;
	}
}

bool IsAvailable(){
	return g_pmigemo!=NULL;
}

//Migemo/bregonig読み込み
bool Load(){
	if(g_pmigemo!=NULL)return true;

	g_pmigemo=NULL;
	g_hMigemo=LoadLibrary(_T("migemo.dll"));

	if(g_hMigemo!=NULL){
		migemo_open=(MIGEMO_OPEN)GetProcAddress(g_hMigemo,"migemo_open");
		migemo_close=(MIGEMO_CLOSE)GetProcAddress(g_hMigemo,"migemo_close");
		migemo_query=(MIGEMO_QUERY)GetProcAddress(g_hMigemo,"migemo_query");
		migemo_release=(MIGEMO_RELEASE)GetProcAddress(g_hMigemo,"migemo_release");
		migemo_load=(MIGEMO_LOAD)GetProcAddress(g_hMigemo,"migemo_load");
		migemo_is_enable=(MIGEMO_IS_ENABLE)GetProcAddress(g_hMigemo,"migemo_is_enable");
		migemo_set_operator=(MIGEMO_SET_OPERATOR)GetProcAddress(g_hMigemo,"migemo_set_operator");
		migemo_setproc_int2char=(MIGEMO_SETPROC_INT2CHAR)GetProcAddress(g_hMigemo,"migemo_setproc_int2char");

		if(migemo_open!=NULL){
			//migemo.dllがあるディレクトリから取得
			TCHAR szMigemoDirectory[MAX_PATH]={};

			if(GetModuleFileName(g_hMigemo,szMigemoDirectory,ARRAY_SIZEOF(szMigemoDirectory))){
				TCHAR szCurrentDirectory[MAX_PATH]={};

				PathRemoveFileSpec(szMigemoDirectory);
				GetCurrentDirectory(ARRAY_SIZEOF(szCurrentDirectory),szCurrentDirectory);
				SetCurrentDirectory(szMigemoDirectory);
				g_pmigemo=migemo_open("./dict/utf-8/" "migemo-dict");
				if(g_pmigemo==NULL||!migemo_is_enable(g_pmigemo)){
					g_pmigemo=migemo_open("./dict/" "migemo-dict");
				}
				if(g_pmigemo==NULL||!migemo_is_enable(g_pmigemo)){
					migemo_close(g_pmigemo);
					g_pmigemo=NULL;
				}
				SetCurrentDirectory(szCurrentDirectory);
			}
		}
		//'[',']','~'をエスケープするため自前で処理
		migemo_setproc_int2char(g_pmigemo,int2char);

		g_hBregonig=LoadLibrary(_T("bregonig.dll"));

		if(g_pmigemo!=NULL&&
		   g_hBregonig!=NULL){
			BMatch=(int(WINAPI*)(TCHAR*,TCHAR*,TCHAR*,BREGEXP**,TCHAR*))GetProcAddress(g_hBregonig,"BMatchW");
			BRegfree=(void(WINAPI*)(BREGEXP*))GetProcAddress(g_hBregonig,"BRegfreeW");
		}
	}

	if(g_pmigemo==NULL||BMatch==NULL){
		MessageBox(NULL,_T("migemo/dict/bregonig読み込み失敗"),_T("Tascher"),MB_OK|MB_ICONEXCLAMATION);
		Unload();
	}
	return g_pmigemo!=NULL;
}

//Migemo/bregonig解放
bool Unload(){
	if(migemo_close!=NULL)migemo_close(g_pmigemo);
	g_pmigemo=NULL;
	migemo_open=NULL;
	migemo_close=NULL;
	migemo_query=NULL;
	migemo_release=NULL;
	migemo_load=NULL;
	migemo_is_enable=NULL;
	migemo_set_operator=NULL;
	FreeLibrary(g_hMigemo);
	g_hMigemo=NULL;
	BMatch=NULL;
	BRegfree=NULL;
	FreeLibrary(g_hBregonig);
	g_hBregonig=NULL;
	return true;
}

//Migemoを用い正規表現リストを作成
TCHAR** AllocRegexes(TCHAR* szQuery,int* piRegexCount){
	if(!lstrlen(szQuery))return NULL;
	*piRegexCount=path::CountCharacter(szQuery,' ');
	*piRegexCount=(*piRegexCount==0)?1:*piRegexCount+1;
	if(szQuery[lstrlen(szQuery)-1]==' ')--*piRegexCount;

	//FreeRegexes()で解放すること!
	TCHAR** ppszRegexes=(TCHAR**)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(TCHAR*)**piRegexCount);
	if(ppszRegexes==NULL)return NULL;

	TCHAR* pszQuery=(TCHAR*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(lstrlen(szQuery)+1)*sizeof(TCHAR));
	if(pszQuery==NULL)return NULL;

	lstrcpy(pszQuery,szQuery);

	int iIndex=0;

	for(int i=0,iRegexIndex=0;iRegexIndex<*piRegexCount;++i){
		if(szQuery[i]==' '||
		   (szQuery[i]=='\0'&&lstrlen(pszQuery+iIndex))){
			pszQuery[i]='\0';

			int iQueryLength=WideCharToMultiByte(CP_UTF8,0,pszQuery+iIndex,-1,NULL,0,NULL,NULL);
			char* pszQueryA=(char*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,iQueryLength);
			WideCharToMultiByte(CP_UTF8,0,pszQuery+iIndex,-1,pszQueryA,iQueryLength,NULL,NULL);
			unsigned char*pszRegexA=migemo_query(g_pmigemo,(const unsigned char*)pszQueryA);
			HeapFree(GetProcessHeap(),0,pszQueryA);
			pszQueryA=NULL;
			int iRegexLength=MultiByteToWideChar(CP_UTF8,0,(LPCSTR)pszRegexA,-1,NULL,0);
			ppszRegexes[iRegexIndex]=(TCHAR*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,iRegexLength*sizeof(TCHAR));
			MultiByteToWideChar(CP_UTF8,0,(LPCSTR)pszRegexA,-1,ppszRegexes[iRegexIndex],iRegexLength);
			migemo_release(g_pmigemo,pszRegexA);

			iIndex=i+1;
			iRegexIndex++;
			if(szQuery[i]=='\0')break;
		}
	}

	HeapFree(GetProcessHeap(),0,pszQuery);
	pszQuery=NULL;
	return ppszRegexes;
}

//正規表現リストを解放
void FreeRegexes(TCHAR*** pppszRegexes){
	HeapFree(GetProcessHeap(),0,*pppszRegexes);
	*pppszRegexes=NULL;
}

//演算子を指定
int SetOperator(const unsigned char* szOption){
	return migemo_set_operator(g_pmigemo,MIGEMO_OPINDEX_NEWLINE,szOption);
}

//正規表現検索
int Match(BREGEXP* pBregExp,TCHAR* str,TCHAR* target,TCHAR* targetendp){
	TCHAR msg[BREGEXP_MAX_ERROR_MESSAGE_LEN]={};
	int iResult=BMatch(str,target,targetendp,&pBregExp,msg);
	return iResult;
}

CompiledBlock::~CompiledBlock(){
	if(pBregExp!=NULL)BRegfree(pBregExp);
}

}//namespace migemo
