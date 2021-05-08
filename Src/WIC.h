//WIC.h
//WIC操作

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.64
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/


#ifndef TASCHER_WIC_H
#define TASCHER_WIC_H
#include<wincodec.h>
#include<d2d1.h>
#include"COM.h"


namespace wic{
struct Bitmap{
	Bitmap():ComInitialize(),pWICImagingFactory(NULL){
		CoCreateInstance(CLSID_WICImagingFactory,NULL,CLSCTX_INPROC_SERVER,IID_IWICImagingFactory,(void**)(&pWICImagingFactory));
	}
	~Bitmap(){
		SAFE_RELEASE(pWICImagingFactory);
	}
	IWICBitmapSource* FromFileName(const TCHAR* szImagePath);
	ID2D1Bitmap* CreateID2D1BitmapFromHICON(ID2D1RenderTarget* pRenderTarget,HICON hIcon);
private:
	com::Initialize ComInitialize;
	IWICImagingFactory* pWICImagingFactory;
};
}//namespace wic

#endif //TASCHER_WIC_H
