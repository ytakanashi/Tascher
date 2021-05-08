//WIC.cpp
//WIC操作

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.64
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/


#include"StdAfx.h"
#include"WIC.h"

namespace wic{
IWICBitmapSource* Bitmap::FromFileName(const TCHAR* szImagePath){
	if(pWICImagingFactory==NULL)return false;

	HRESULT hResult=S_OK;
	IWICBitmapDecoder* pWICBitmapDecoder=NULL;
	IWICBitmapFrameDecode* pWICBitmapFrameDecode=NULL;
	IWICFormatConverter* pWICFormatConverter=NULL;
	IWICBitmapSource* pWICBitmapSource=NULL;

	if(SUCCEEDED(hResult)){
		hResult=pWICImagingFactory->CreateDecoderFromFilename(szImagePath,
															  NULL,
															  GENERIC_READ,
															  WICDecodeMetadataCacheOnLoad,
															  &pWICBitmapDecoder);
	}
	if(SUCCEEDED(hResult)){
		hResult=pWICBitmapDecoder->GetFrame(0,&pWICBitmapFrameDecode);
	}
	if(SUCCEEDED(hResult)){
		hResult=pWICImagingFactory->CreateFormatConverter(&pWICFormatConverter);
	}
	if(SUCCEEDED(hResult)){
		hResult=pWICFormatConverter->Initialize(pWICBitmapFrameDecode,
											 GUID_WICPixelFormat32bppPBGRA,
											  WICBitmapDitherTypeNone,
											  NULL,
											  1.0f,
											  WICBitmapPaletteTypeMedianCut);
	}
	if(SUCCEEDED(hResult)){
		hResult=WICConvertBitmapSource(GUID_WICPixelFormat32bppPBGRA,pWICBitmapFrameDecode,&pWICBitmapSource);
	}

	SAFE_RELEASE(pWICFormatConverter);
	SAFE_RELEASE(pWICBitmapFrameDecode);
	SAFE_RELEASE(pWICBitmapDecoder);
	return SUCCEEDED(hResult)?pWICBitmapSource:NULL;
}

ID2D1Bitmap* Bitmap::CreateID2D1BitmapFromHICON(ID2D1RenderTarget* pRenderTarget,HICON hIcon){
	if(pWICImagingFactory==NULL)return NULL;
	if(pRenderTarget==NULL||hIcon==NULL)return NULL;

	HRESULT hResult=S_OK;
	IWICFormatConverter* pWICFormatConverter=NULL;
	IWICBitmap* pWICBitmap=NULL;
	ID2D1Bitmap* pD2DBitmap=NULL;

	if(SUCCEEDED(hResult)){
		hResult=pWICImagingFactory->CreateBitmapFromHICON(hIcon,&pWICBitmap);
	}
	if(SUCCEEDED(hResult)){
		hResult=pWICImagingFactory->CreateFormatConverter(&pWICFormatConverter);
	}
	if(SUCCEEDED(hResult)){
		hResult=pWICFormatConverter->Initialize(pWICBitmap,GUID_WICPixelFormat32bppPBGRA,WICBitmapDitherTypeNone,NULL,0.0f,WICBitmapPaletteTypeMedianCut);
	}
	if(SUCCEEDED(hResult)){
		hResult=pRenderTarget->CreateBitmapFromWicBitmap(pWICFormatConverter,NULL,&pD2DBitmap);
	}

	SAFE_RELEASE(pWICBitmap);
	SAFE_RELEASE(pWICFormatConverter);

	return SUCCEEDED(hResult)?pD2DBitmap:NULL;
}
}//namespace wic
