//Direct2D.cpp
//Direct2D操作関数

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.64
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/


#include"StdAfx.h"
#include"Direct2D.h"
#include"WIC.h"


namespace direct2d{

HMODULE g_hD2D1Dll;
HMODULE g_hDWriteDll;


HRESULT(WINAPI*pD2D1CreateFactory)(D2D1_FACTORY_TYPE,REFIID,const D2D1_FACTORY_OPTIONS*,void**);
HRESULT(WINAPI*pDWriteCreateFactory)(DWRITE_FACTORY_TYPE,REFIID,IUnknown**);

bool g_bLoaded;



bool IsLoaded(){
	return g_bLoaded;
}

//d2d1.dll/DWrite.dll読み込み
bool Load(){
	if(g_hD2D1Dll!=NULL&&
	   g_hDWriteDll!=NULL)return true;

	g_bLoaded=false;

	TCHAR szD2D1DllPath[MAX_PATH]={};
	TCHAR szDWriteDllPath[MAX_PATH]={};

	GetSystemDirectory(szD2D1DllPath,ARRAY_SIZEOF(szD2D1DllPath));
	lstrcpy(szDWriteDllPath,szD2D1DllPath);
	lstrcat(szD2D1DllPath,_T("\\d2d1.dll"));
	lstrcat(szDWriteDllPath,_T("\\DWrite.dll"));
	if((g_hD2D1Dll=LoadLibrary(szD2D1DllPath))!=NULL&&
	   (g_hDWriteDll=LoadLibrary(szDWriteDllPath))!=NULL){
		pD2D1CreateFactory=(HRESULT(WINAPI*)(D2D1_FACTORY_TYPE,REFIID,const D2D1_FACTORY_OPTIONS*,void**))
			GetProcAddress(g_hD2D1Dll,"D2D1CreateFactory");
		pDWriteCreateFactory=(HRESULT(WINAPI*)(DWRITE_FACTORY_TYPE,REFIID,IUnknown**))
			GetProcAddress(g_hDWriteDll,"DWriteCreateFactory");
		g_bLoaded=pD2D1CreateFactory&&pDWriteCreateFactory;
	}

	return g_bLoaded;
}

//d2d1.dll/DWrite.dll解放
bool Unload(){
	if(g_hD2D1Dll!=NULL){
		FreeLibrary(g_hD2D1Dll);
		g_hD2D1Dll=NULL;
	}
	if(g_hDWriteDll!=NULL){
		FreeLibrary(g_hDWriteDll);
		g_hDWriteDll=NULL;
	}
	g_bLoaded=false;
	return true;
}

bool Direct2DBase::Initialize(){
	if(IsAvailable())Uninitialize();
	if(!IsLoaded()&&!Load())return false;
	pD2d1Factory=NULL;
	pDWriteFactory=NULL;
	pRenderTarget=NULL;
	bAvailable=false;
	HRESULT hResult=S_OK;

	if(SUCCEEDED(hResult)){
		hResult=pD2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED,__uuidof(ID2D1Factory),NULL,(void**)&pD2d1Factory);
	}
	if(SUCCEEDED(hResult)){
		hResult=pDWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,__uuidof(IDWriteFactory),(IUnknown**)&pDWriteFactory);
	}
	bAvailable=SUCCEEDED(hResult);
	return bAvailable;
}

//RenderTarget作成
bool Direct2DBase::CreateRenderTarget(HWND hTarget,UINT uWidth,UINT uHeight){
	if(!IsLoaded()&&!Load())return false;
	SAFE_RELEASE(pRenderTarget);

	bAvailable=SUCCEEDED(pD2d1Factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),
															  D2D1::HwndRenderTargetProperties(hTarget,D2D1::SizeU(uWidth,uHeight)),
															  &pRenderTarget));
	return bAvailable;
}

//テキストフォーマット作成
bool ListView::CreateTextFormat(IDWriteTextFormat** ppTextFormat,HFONT hFont){
	if(pDWriteFactory==NULL)return false;
	SAFE_RELEASE(*ppTextFormat);

	LOGFONT lgFont={};
	GetObject(hFont,sizeof(LOGFONT),&lgFont);

	float fFontSize=(float)lgFont.lfHeight;
	if(fFontSize<0)fFontSize=0-fFontSize;

	float fDpiX=0,fDpiY=0;
	GetDesktopDpi(&fDpiX,&fDpiY);
	fFontSize*=(96.0f/fDpiX);

	DWRITE_FONT_WEIGHT FontWeight=DWRITE_FONT_WEIGHT_NORMAL;
	DWRITE_FONT_STYLE FontStyle=DWRITE_FONT_STYLE_NORMAL;

	if(lgFont.lfWeight==FW_BOLD){
		//太字
		FontWeight=DWRITE_FONT_WEIGHT_BOLD;
	}

	if(lgFont.lfItalic){
		//斜体
		FontStyle=DWRITE_FONT_STYLE_ITALIC;
	}

	if(FAILED(pDWriteFactory->CreateTextFormat(lgFont.lfFaceName,
											   NULL,
											   FontWeight,
											   FontStyle,
											   DWRITE_FONT_STRETCH_NORMAL,
											   fFontSize,
											   _T(""),
											   ppTextFormat)))return false;

	//単語中は改行しないようにする
	//SetTrimming()との組み合わせにより単語中であっても行の幅を超える場合は改行するようになる
	(*ppTextFormat)->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);


	//描画位置を行中央に設定
	(*ppTextFormat)->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	IDWriteInlineObject* pInlineObject=NULL;

	//省略記号設定
	if(SUCCEEDED(pDWriteFactory->CreateEllipsisTrimmingSign(*ppTextFormat,&pInlineObject))){
		DWRITE_TRIMMING dt={DWRITE_TRIMMING_GRANULARITY_CHARACTER,0,0};

		(*ppTextFormat)->SetTrimming(&dt,pInlineObject);
	}
	SAFE_RELEASE(pInlineObject);
	return true;
}

//背景色ブラシ作成
bool ListView::CreateTextBkBrush(ID2D1LinearGradientBrush** ppTextBkBrush,COLORREF clr,COLORREF clrGradientEndColor){
	if(pRenderTarget==NULL)return false;
	bool bResult=false;
	HRESULT hResult=S_OK;

	SAFE_RELEASE(*ppTextBkBrush);

	ID2D1GradientStopCollection* pGradientStops=NULL;
	D2D1_GRADIENT_STOP gradientStops[2]={};

	gradientStops[0].color=D2D1::ColorF(GetRValue(clr)/255.0f,GetGValue(clr)/255.0f,GetBValue(clr)/255.0f,1.0f);
	gradientStops[0].position=0.0f;
	gradientStops[1].color=D2D1::ColorF(GetRValue(clrGradientEndColor)/255.0f,GetGValue(clrGradientEndColor)/255.0f,GetBValue(clrGradientEndColor)/255.0f,1.0f);
	gradientStops[1].position=1.0f;

	if(SUCCEEDED(hResult)){
		hResult=pRenderTarget->CreateGradientStopCollection(gradientStops,
															ARRAY_SIZEOF(gradientStops),
															D2D1_GAMMA_2_2,
															D2D1_EXTEND_MODE_CLAMP,
															&pGradientStops);
	}

	if(SUCCEEDED(hResult)){
		RECT rcItem={};

		ListView_GetItemRect(pRenderTarget->GetHwnd(),0,&rcItem,LVIR_BOUNDS);
		float fDpiX=0,fDpiY=0;
		GetDesktopDpi(&fDpiX,&fDpiY);

		D2D1_RECT_F rcBrush=D2D1::RectF(0,0,
										(float)rcItem.right*(96.0f/fDpiY)-(float)rcItem.left*(96.0f/fDpiY),
										(float)rcItem.bottom*(96.0f/fDpiY)-(float)rcItem.top*(96.0f/fDpiY));
		bResult=SUCCEEDED(pRenderTarget->CreateLinearGradientBrush(D2D1::LinearGradientBrushProperties(D2D1::Point2F(rcBrush.left,rcBrush.top),
																									   D2D1::Point2F(rcBrush.right,rcBrush.top)),
																   pGradientStops,ppTextBkBrush));
	}
	SAFE_RELEASE(pGradientStops);
	return bResult;
}

void ListView::SetTextRenderingParams(UINT uGamma,UINT uEnhancedContrast,UINT uClearTypeLevel,UINT uRenderingMode){
	if(pDWriteFactory==NULL||pRenderTarget==NULL)return;
	IDWriteRenderingParams* pRenderParams=NULL;

	//ガンマ値等を設定
	if(SUCCEEDED(pDWriteFactory->CreateCustomRenderingParams((float)(uGamma/1000),
											(float)(uEnhancedContrast/100),
											(float)(uClearTypeLevel/100),
											DWRITE_PIXEL_GEOMETRY_RGB,
											(DWRITE_RENDERING_MODE)uRenderingMode,
											&pRenderParams))&&
	   pRenderParams){
		pRenderTarget->SetTextRenderingParams(pRenderParams);
	}

	SAFE_RELEASE(pRenderParams);

	//DWRITE_RENDERING_MODE_ALIASEDならD2D1_TEXT_ANTIALIAS_MODE_ALIASED
	pRenderTarget->SetTextAntialiasMode((uRenderingMode==DWRITE_RENDERING_MODE_ALIASED)?
										D2D1_TEXT_ANTIALIAS_MODE_ALIASED:
										D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);
	return;
}

//通常行背景色ブラシ作成
bool ListView::CreateDefaultTextBkBrush(COLORREF clr,COLORREF clrGradientEndColor){
	return CreateTextBkBrush(&pDefaultTextBkBrush,clr,clrGradientEndColor);
}

//選択行背景色ブラシ作成
bool ListView::CreateSelectedTextBkBrush(COLORREF clr,COLORREF clrGradientEndColor){
	return CreateTextBkBrush(&pSelectedTextBkBrush,clr,clrGradientEndColor);
}

//背景画像読み込み
bool ListView::LoadBackgroundImage(const TCHAR* szImagePath,BYTE byResizePercent,BYTE byOpacity){
	if(pRenderTarget==NULL)return false;
	ReleaseBackgroundImage();

	HRESULT hResult=S_OK;
	wic::Bitmap WicBitmap;
	IWICBitmapSource* pWICBitmapSource=WicBitmap.FromFileName(szImagePath);

	if(pWICBitmapSource!=NULL){
		//RenderTargetを作成し、画像縮小処理を行い、GetBitmap()でID2D1Bitmapを取得
		UINT uOriginalWidth=0,uOriginalHeight=0;
		ID2D1BitmapRenderTarget* pCompatibleRenderTarget=NULL;

		if(SUCCEEDED(hResult)){
			hResult=pWICBitmapSource->GetSize(&uOriginalWidth,&uOriginalHeight);
		}
		if(SUCCEEDED(hResult)){
			hResult=pRenderTarget->CreateCompatibleRenderTarget(D2D1::SizeF((float)uOriginalWidth,(float)uOriginalHeight),
																		 &pCompatibleRenderTarget);
		}

		ID2D1Bitmap* pBitmapTmp=NULL;

		if(SUCCEEDED(hResult)){
			if(SUCCEEDED(pCompatibleRenderTarget->CreateBitmapFromWicBitmap(pWICBitmapSource,&pBitmapTmp))){
				if(byResizePercent!=100){
					pCompatibleRenderTarget->SetTransform(D2D1::Matrix3x2F::Scale((float)byResizePercent/100,
																				  (float)byResizePercent/100,
																				  D2D1::Point2F(0,0)));
				}

				float fDpiX=0,fDpiY=0;
				GetDesktopDpi(&fDpiX,&fDpiY);

				pCompatibleRenderTarget->BeginDraw();

				pCompatibleRenderTarget->DrawBitmap(pBitmapTmp,
													D2D1::RectF(0,0,uOriginalWidth*(96.0f/fDpiX),uOriginalHeight*(96.0f/fDpiX)),
													(float)byOpacity/100,
													D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
													D2D1::RectF(0,0,(float)uOriginalWidth,(float)uOriginalHeight));

				pCompatibleRenderTarget->EndDraw();
				pCompatibleRenderTarget->GetBitmap(&pBitmapBackground);
			}
		}
		SAFE_RELEASE(pBitmapTmp);
		SAFE_RELEASE(pCompatibleRenderTarget);
	}
	SAFE_RELEASE(pWICBitmapSource);
	return pBitmapBackground!=NULL;
}

//背景画像を描画
void ListView::DrawBackground(const D2D1_RECT_F& rect,int iXOffset,int iYOffset){
	if(!IsLoadedBackgroundImage())return;
	D2D1_SIZE_F sizeBitmap=pBitmapBackground->GetSize();
	pRenderTarget->PushAxisAlignedClip(rect,D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
	pRenderTarget->DrawBitmap(pBitmapBackground,
							  D2D1::RectF((float)iXOffset,
										  (float)iYOffset,
										  sizeBitmap.width+iXOffset,
										  sizeBitmap.height+iYOffset),
							  1.0f,
							  D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
							  D2D1::RectF(0,0,sizeBitmap.width,sizeBitmap.height));
	pRenderTarget->PopAxisAlignedClip();
}

}//namespace direct2d
