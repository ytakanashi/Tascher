//WebBrowserTab,cpp
//ブラウザタブ取得

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.64
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/


#include"StdAfx.h"
#include"WebBrowserTab.h"
#include"Utilities.h"
#include<iphlpapi.h>

namespace webbrowsertab{

//winsock.h
#define AF_INET         2               /* internetwork: UDP, TCP, etc. */
bool GetTargetAddress(TCHAR* pszAddress,DWORD* pdwPort,DWORD* pdwProcessId,TCHAR* pszTargetProcessPath,DWORD* pdwTargetPort){
	if(pszAddress==NULL||pdwPort==NULL)return false;

	bool bResult=false;
	DWORD dwSize=0;
	if(GetExtendedTcpTable(NULL,&dwSize,true,AF_INET,TCP_TABLE_OWNER_PID_ALL,0)!=ERROR_INSUFFICIENT_BUFFER){
		return false;
	}

	PMIB_TCPTABLE_OWNER_PID pTcpTable=(PMIB_TCPTABLE_OWNER_PID)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,dwSize);
	if(pTcpTable==NULL)return false;

	if(GetExtendedTcpTable(pTcpTable,&dwSize,true,AF_INET,TCP_TABLE_OWNER_PID_ALL,0)!=NO_ERROR){
		HeapFree(GetProcessHeap(),0,pTcpTable);
		return false;
	}

	for(UINT i=0;i<pTcpTable->dwNumEntries;i++){
		if(pTcpTable->table[i].dwState==MIB_TCP_STATE_LISTEN){
			DWORD dwPort=((pTcpTable->table[i].dwLocalPort>>8)&0x00ff)|((pTcpTable->table[i].dwLocalPort<<8)&0xff00);

			//指定したプロセスのパスか
			TCHAR szProcessName[MAX_PATH]={};
			if(pszTargetProcessPath!=NULL&&
			   lstrlen(pszTargetProcessPath)){
				if(!GetProcessFileName(pTcpTable->table[i].dwOwningPid,szProcessName,ARRAY_SIZEOF(szProcessName)))continue;
				//含まれていればOK
				if(StrRStrI(szProcessName,NULL,pszTargetProcessPath)==NULL||
				   lstrcmpi(PathFindFileName(szProcessName),PathFindFileName(pszTargetProcessPath))!=0)continue;
			}

			//指定したポートと一致するか
			if(pdwTargetPort!=NULL&&
			   *pdwTargetPort!=dwPort)continue;

			TCHAR szAddress[24]={};
			wsprintf(szAddress,_T("%d.%d.%d.%d"),(pTcpTable->table[i].dwLocalAddr)&0xFF,(pTcpTable->table[i].dwLocalAddr>>8)&0xFF,(pTcpTable->table[i].dwLocalAddr>>16)&0xFF,(pTcpTable->table[i].dwLocalAddr>>24)&0xFF);
			lstrcpy(pszAddress,szAddress);

			*pdwPort=dwPort;

			if(pdwProcessId!=NULL){
				*pdwProcessId=pTcpTable->table[i].dwOwningPid;
			}
			bResult=true;
			break;
		}
	}
	HeapFree(GetProcessHeap(),0,pTcpTable);
	return bResult;
}

//MsXml6.h
class DECLSPEC_UUID("88d96a0a-f192-11d4-a65f-0040963251e5")XMLHTTP60;
MIDL_INTERFACE("ED8C108D-4349-11D2-91A4-00C04F7969E8")
IXMLHTTPRequest:public IDispatch{
public:
	virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE open(
		/* [in] */ __RPC__in BSTR bstrMethod,
		/* [in] */ __RPC__in BSTR bstrUrl,
		/* [optional][in] */ VARIANT varAsync,
		/* [optional][in] */ VARIANT bstrUser,
		/* [optional][in] */ VARIANT bstrPassword)=0;

	virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE setRequestHeader(
		/* [in] */ __RPC__in BSTR bstrHeader,
		/* [in] */ __RPC__in BSTR bstrValue)=0;

	virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE getResponseHeader(
		/* [in] */ __RPC__in BSTR bstrHeader,
		/* [retval][out] */ __RPC__deref_out_opt BSTR* pbstrValue)=0;

	virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE getAllResponseHeaders(
		/* [retval][out] */ __RPC__deref_out_opt BSTR* pbstrHeaders)=0;

	virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE send(
		/* [optional][in] */ VARIANT varBody)=0;

	virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE abort(void) = 0;

	virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_status(
		/* [retval][out] */ __RPC__out long* plStatus)=0;

	virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_statusText(
		/* [retval][out] */ __RPC__deref_out_opt BSTR* pbstrStatus)=0;

	virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_responseXML(
		/* [retval][out] */ __RPC__deref_out_opt IDispatch** ppBody)=0;

	virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_responseText(
		/* [retval][out] */ __RPC__deref_out_opt BSTR* pbstrBody)=0;

	virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_responseBody(
		/* [retval][out] */ __RPC__out VARIANT* pvarBody)=0;

	virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_responseStream(
		/* [retval][out] */ __RPC__out VARIANT* pvarBody)=0;

	virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_readyState(
		/* [retval][out] */ __RPC__out long* plState)=0;

	virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_onreadystatechange(
		/* [in] */ __RPC__in_opt IDispatch* pReadyStateSink)=0;

};

class DECLSPEC_UUID("88d96a0b-f192-11d4-a65f-0040963251e5")ServerXMLHTTP60;
MIDL_INTERFACE("2e9196bf-13ba-4dd4-91ca-6c571f281495")
IServerXMLHTTPRequest:public IXMLHTTPRequest{
public:
	enum SERVERXMLHTTP_OPTION{
		SXH_OPTION_URL=-1,
		SXH_OPTION_URL_CODEPAGE=(SXH_OPTION_URL+1),
		SXH_OPTION_ESCAPE_PERCENT_IN_URL=(SXH_OPTION_URL_CODEPAGE+1),
		SXH_OPTION_IGNORE_SERVER_SSL_CERT_ERROR_FLAGS=(SXH_OPTION_ESCAPE_PERCENT_IN_URL+1),
		SXH_OPTION_SELECT_CLIENT_SSL_CERT=(SXH_OPTION_IGNORE_SERVER_SSL_CERT_ERROR_FLAGS+1)
	};

	virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE setTimeouts(
		/* [in] */ long resolveTimeout,
		/* [in] */ long connectTimeout,
		/* [in] */ long sendTimeout,
		/* [in] */ long receiveTimeout)=0;

	virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE waitForResponse(
		/* [optional][in] */ VARIANT timeoutInSeconds,
		/* [retval][out] */ __RPC__out VARIANT_BOOL *isSuccessful)=0;

	virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE getOption(
		/* [in] */ SERVERXMLHTTP_OPTION option,
		/* [retval][out] */ __RPC__out VARIANT *value)=0;

	virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE setOption(
		/* [in] */ SERVERXMLHTTP_OPTION option,
		/* [in] */ VARIANT value)=0;
};

class ChromeDevToolsProtocol:public Allocator{
public:
	ChromeDevToolsProtocol():
		pIXMLHTTPRequest(NULL),
		ComInitialize(),
		bAvailable(false),
		szAddress(),
		dwPort(0),
		dwProcessId(0){
		//キャッシュを防ぐためXMLHTTP60ではなくServerXMLHTTP60を使用
		CoCreateInstance(__uuidof(ServerXMLHTTP60),NULL,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&pIXMLHTTPRequest));
	}
	~ChromeDevToolsProtocol(){
		SAFE_RELEASE(pIXMLHTTPRequest);
	}
	const bool Initialize(TCHAR* pszTargetProcessPath,DWORD* pdwTargetPort){
		if(pszTargetProcessPath==NULL&&pdwTargetPort==NULL)return false;
		if(pIXMLHTTPRequest!=NULL){
			if(GetTargetAddress(szAddress,&dwPort,&dwProcessId,pszTargetProcessPath,pdwTargetPort)){
				TCHAR szVersion[256]={};
				wsprintf(szVersion,_T("http://%s:%d/json/version"),szAddress,dwPort);
				bAvailable=true;
				bAvailable=Send(szVersion);
			}
		}
		return bAvailable;
	}
	const bool IsResponseOk(long lStatus)const{return (lStatus>=200&&lStatus<=299);}
private:
	long Send(TCHAR* pszURL){
		if(!IsAvailable()||pszURL==NULL)return false;
		BSTR bstrGet=SysAllocString(_T("GET"));
		BSTR bstrURL=SysAllocString(pszURL);
		BSTR bstrCacheControl=SysAllocString(_T("Cache-Control"));
		BSTR bstrNoCache=SysAllocString(_T("no-store"));
		VARIANT vAsync={};
		VariantInit(&vAsync);
		V_VT(&vAsync)=VT_BOOL;
		V_BOOL(&vAsync)=VARIANT_FALSE;

		BSTR bstrNullString=SysAllocString(_T(""));
		VARIANT vNullString={};
		VariantInit(&vNullString);
		V_VT(&vNullString)=VT_BSTR;
		V_BSTR(&vNullString)=bstrNullString;
		pIXMLHTTPRequest->open(
			bstrGet,
			bstrURL,
			vAsync,
			vNullString,
			vNullString);
		pIXMLHTTPRequest->setRequestHeader(bstrCacheControl,bstrNoCache);
		SysFreeString(bstrGet);
		SysFreeString(bstrURL);
		SysFreeString(bstrCacheControl);
		SysFreeString(bstrNoCache);
		VariantClear(&vAsync);

		pIXMLHTTPRequest->send(vNullString);
		long lStatus;
		pIXMLHTTPRequest->get_status(&lStatus);

		VariantClear(&vNullString);
		SysFreeString(bstrNullString);
		return lStatus;
	}
public:
	const bool IsAvailable()const{return bAvailable;}
	//HeapFree()で解放すること
	TCHAR* AllocJsonList(){
		if(!IsAvailable())return NULL;
		TCHAR szGetList[128]={};
		wsprintf(szGetList,_T("http://%s:%d/json/list"),szAddress,dwPort);

		if(!IsResponseOk(Send(szGetList)))return NULL;
		BSTR bstrText;
		if(FAILED(pIXMLHTTPRequest->get_responseText(&bstrText)))return NULL;

		TCHAR* pszJsonList=NULL;
		pszJsonList=(TCHAR*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(SysStringLen(bstrText)+1)*sizeof(TCHAR));
		if(pszJsonList!=NULL){
			lstrcpy(pszJsonList,(TCHAR*)bstrText);
		}
		SysFreeString(bstrText);
		return pszJsonList;
	}
	void FreeJsonList(TCHAR**ppszJsonList){
		if(*ppszJsonList!=NULL){
			HeapFree(GetProcessHeap(),0,*ppszJsonList);
			*ppszJsonList=NULL;
		}
	}
	bool Activate(TCHAR* pszId){
		if(!IsAvailable()||pszId==NULL)return false;
		TCHAR szActivate[256]={};
		wsprintf(szActivate,_T("http://%s:%d/json/activate/%s"),szAddress,dwPort,pszId);
		//AllowSetForegroundWindow()しないと点滅するだけで切り替えができない
		AllowSetForegroundWindow(dwProcessId);
		return IsResponseOk(Send(szActivate));
	}

	bool Close(TCHAR* pszId){
		if(!IsAvailable()||pszId==NULL)return false;
		TCHAR szClose[256]={};
		wsprintf(szClose,_T("http://%s:%d/json/close/%s"),szAddress,dwPort,pszId);
		return IsResponseOk(Send(szClose));
	}
	const DWORD GetProcessId()const{return dwProcessId;}
private:
	com::Initialize ComInitialize;
	IServerXMLHTTPRequest* pIXMLHTTPRequest;
	bool bAvailable;
	TCHAR szAddress[128];
	DWORD dwPort;
	DWORD dwProcessId;
};

class AllocTabDataList{
public:
	AllocTabDataList():pTabData(NULL),iTabDataCount(0){}
	~AllocTabDataList(){
		//Freeは各自で行うこと
	}
	bool AddData(TAB_DATA *pData){
		TAB_DATA* pTmp=NULL;

		if(iTabDataCount==0)pTmp=(TAB_DATA*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(++iTabDataCount)*sizeof(TAB_DATA));
		else pTmp=(TAB_DATA*)HeapReAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,pTabData,(++iTabDataCount)*sizeof(TAB_DATA));
		if(pTmp==NULL){
			--iTabDataCount;
			return false;
		}

		pTabData=pTmp;
		pTabData[iTabDataCount-1]=*pData;
		return true;
	}

	TAB_DATA* GetData()const{
		return pTabData;
	}

	const int GetCount()const{
		return iTabDataCount;
	}

private:
	TAB_DATA* pTabData;
	int iTabDataCount;
};

const TCHAR szHtmlDecodeTable[][2][12]={
	{_T("&lt;"),_T("<")},
	{_T("&gt;"),_T(">")},
	{_T("&amp;"),_T("&")},
	{_T("&quot;"),_T("\"")},
	{_T("&#39;"),_T("'")},
};

class ParseJson{
public:
	ParseJson(AllocTabDataList& List_):List(List_),TabData(){}
	~ParseJson(){}

TCHAR* GetStringValue(TCHAR* ptr,const TCHAR* search,bool bNest=false){
	if(ptr==NULL||search==NULL)return NULL;
	if(ptr!=StrStr(ptr,search))return NULL;
	ptr=StrStr(ptr,search);
	if(ptr==NULL){
		return NULL;
	}
	ptr=StrChr(ptr,':');
	if(ptr==NULL){
		return NULL;
	}
	ptr++;//skip ':'
	for(;IsSpace(*ptr);ptr++);
	if(ptr==NULL){
		return NULL;
	}
	if(*ptr=='\"'&&*(ptr+1)!='\0'){
		ptr++;//skip "
		if(*ptr=='\"')return ptr;//空文字列
		TCHAR* p2=NULL;
		for(p2=ptr+1;*p2&&*p2!='\"';p2++);
		if(p2==NULL)return NULL;
		unsigned int diff=(unsigned int)(p2-ptr);
		TCHAR* str=(TCHAR*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(diff+1)*sizeof(TCHAR));
		if(str){
			lstrcpyn(str,ptr,diff+1);
			if(lstrcmp(search,_T("\"type\""))==0){
				TabData.pszType=str;
			}else if(lstrcmp(search,_T("\"id\""))==0){
				TabData.pszId=str;
			}else if(lstrcmp(search,_T("\"title\""))==0){
				TabData.pszTitle=str;
				//特殊文字(&lt;等)を変換
				for(int i=0;i<ARRAY_SIZEOF(szHtmlDecodeTable);i++){
					do{
						StrReplace(TabData.pszTitle,szHtmlDecodeTable[i][0],szHtmlDecodeTable[i][1]);
					}while(StrStr(TabData.pszTitle,szHtmlDecodeTable[i][0]));
				}
			}else if(lstrcmp(search,_T("\"url\""))==0){
				TabData.pszURL=str;
			}else if(lstrcmp(search,_T("\"faviconUrl\""))==0){
				TabData.pszFaviconURL=str;
			}else{
				HeapFree(GetProcessHeap(),0,str);
			}
		}
		ptr=p2;
		ptr++;//skip "
	}else{
		if(*ptr=='t'||//true
			*ptr=='f'||//false
			*ptr=='n'||//null
			*ptr=='-'||
			*ptr>='0'&&*ptr<='9'){
			TCHAR* p2=NULL;
			for(p2=ptr+1;*p2&&!IsSpace(*p2);p2++);
			if(p2==NULL)return NULL;

			unsigned int diff=(unsigned int)(p2-ptr);
			TCHAR* str=(TCHAR*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(diff+1)*sizeof(TCHAR));
			if(str){
				lstrcpyn(str,ptr,diff+1);
				HeapFree(GetProcessHeap(),0,str);
			}
			ptr=p2;
		}else if(*ptr=='{'){
			//ネスト
			return Search(ptr,true);
		}else if(*ptr=='['){
			//配列
			TCHAR* p2=NULL;
			bool bIsString=false;
			bool bArrayEnd=false;
			for(p2=ptr+1;*p2;p2++){
				if(*(p2-1)!='\\'&&*p2=='\"')bIsString^=true;
				if(*p2==']'&&!bIsString){bArrayEnd=true;break;}
			}
			if(!bArrayEnd)return NULL;
			p2++;
			if(p2==NULL)return NULL;
			unsigned int diff=(unsigned int)(p2-ptr);
			TCHAR* str=(TCHAR*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(diff+1)*sizeof(TCHAR));
			if(str){
				lstrcpyn(str,ptr,diff+1);
				HeapFree(GetProcessHeap(),0,str);
			}
			ptr=p2;
		}
	}
	return ptr;
}

TCHAR* Search(TCHAR* ptr,bool bNest=false){
	for(;*ptr!='\0';ptr++){
		switch(*ptr){
		case '\"':{
			TCHAR* coron=StrChr(ptr,':');
			if(coron==NULL)break;
			TCHAR* bak=coron;
			for(--bak;bak&&IsSpace(*bak);bak--);
			if(ptr==bak||*bak!='\"')break;

			TCHAR* prop=(TCHAR*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(bak-ptr+2)*sizeof(TCHAR));
			if(prop){
				lstrcpyn(prop,ptr,(int)(bak-ptr+2));
				ptr=GetStringValue(ptr,prop,bNest);
				HeapFree(GetProcessHeap(),0,prop);
			}
			break;
		}
		case '{':{
			//オブジェクト開始
			TabData.init();
			break;
		}
		case '}':{
			//オブジェクト終了
			if(lstrcmp(TabData.pszType,_T("page"))==0){
				//pageなら追加
				List.AddData(&TabData);
			}else{
				TabData.init();
			}
			break;
		}
		}
	}
	return ptr;
}
private:
	AllocTabDataList& List;
	TAB_DATA TabData;
};

List::List():pChromeDevToolsProtocol(NULL){
}

List::~List(){
	Uninitialize();
}

bool List::Initialize(TCHAR* pszTargetProcessPath,DWORD* pdwTargetPort){
	Uninitialize();
	pChromeDevToolsProtocol=new ChromeDevToolsProtocol;
	if(pChromeDevToolsProtocol==NULL)return false;
	return pChromeDevToolsProtocol->Initialize(pszTargetProcessPath,pdwTargetPort);
}

void List::Uninitialize(){
	if(pChromeDevToolsProtocol!=NULL){
		delete pChromeDevToolsProtocol;
		pChromeDevToolsProtocol=NULL;
	}
}

TAB_DATA* List::Alloc(int* piCount,DWORD* pdwProcessId){
	if(pChromeDevToolsProtocol==NULL)return NULL;
	TCHAR* pszJsonList=pChromeDevToolsProtocol->AllocJsonList();
	if(pszJsonList==NULL)return NULL;

	AllocTabDataList TabDataList;
	ParseJson parse(TabDataList);
	parse.Search(pszJsonList);
	if(piCount!=NULL)*piCount=TabDataList.GetCount();
	if(pdwProcessId!=NULL)*pdwProcessId=pChromeDevToolsProtocol->GetProcessId();
	pChromeDevToolsProtocol->FreeJsonList(&pszJsonList);

	return TabDataList.GetData();
}

void List::Free(TAB_DATA** ppList){
	if(*ppList!=NULL){
		HeapFree(GetProcessHeap(),0,*ppList);
		*ppList=NULL;
	}
}

bool List::Activate(TCHAR* pszId){
	if(pChromeDevToolsProtocol==NULL||pszId==NULL)return false;
	return pChromeDevToolsProtocol->Activate(pszId);
}

bool List::Close(TCHAR* pszId){
	if(pChromeDevToolsProtocol==NULL||pszId==NULL)return false;
	return pChromeDevToolsProtocol->Close(pszId);
}

}//namespace webbrowsertab
