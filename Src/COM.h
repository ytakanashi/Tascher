//COM.h
//COM初期化

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.64
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/


#ifndef TASCHER_COM_H
#define TASCHER_COM_H


namespace com{
class Initialize{
public:
	explicit Initialize(DWORD dwCoInit=COINIT_APARTMENTTHREADED|COINIT_DISABLE_OLE1DDE):
	m_hResult(CoInitializeEx(NULL,dwCoInit)){}
	~Initialize(){
		if(SUCCEEDED(m_hResult)){
			CoUninitialize();
		}
	}
	operator HRESULT()const{return m_hResult;}
private:
	HRESULT m_hResult;
	Initialize(const Initialize&);
	Initialize& operator=(const Initialize&);
};
}//namespace com

#endif //TASCHER_COM_H
