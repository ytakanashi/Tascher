//Direct2D.h
//Direct2D操作関数

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.64
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/


#ifndef TASCHER_DIRECT2D_H
#define TASCHER_DIRECT2D_H

#include"Allocator.h"
#include"Utilities.h"
#include<d2d1.h>
#include<dwrite.h>

namespace direct2d{

//d2d1.dll/DWrite.dll読み込み
bool Load();
//d2d1.dll/DWrite.dll解放
bool Unload();

bool IsLoaded();


struct Direct2DBase:public Allocator{
	ID2D1Factory* pD2d1Factory;
	IDWriteFactory* pDWriteFactory;
	ID2D1HwndRenderTarget* pRenderTarget;
	bool bAvailable;

	Direct2DBase():
		pD2d1Factory(NULL),
		pDWriteFactory(NULL),
		pRenderTarget(NULL),
		bAvailable(false){}
	~Direct2DBase(){}

	bool IsAvailable(){return bAvailable;}

	bool Initialize();

	void Uninitialize(){
		SAFE_RELEASE(pRenderTarget);
		SAFE_RELEASE(pDWriteFactory);
		SAFE_RELEASE(pD2d1Factory);
		bAvailable=false;
	}

	ID2D1HwndRenderTarget* GetRenderTarget()const{return pRenderTarget;}

	//RenderTarget作成
	bool CreateRenderTarget(HWND hTarget,UINT uWidth,UINT uHeight);

	//DPIを取得
	void GetDesktopDpi(float* fDpiX,float* fDpiY){
		if(!IsLoaded()&&!Load())return;

		if(pD2d1Factory!=NULL){
			pD2d1Factory->ReloadSystemMetrics();
			pD2d1Factory->GetDesktopDpi(fDpiX,fDpiY);
		}
	}

};

struct ListView:public Direct2DBase{
	ListView():
		pDefaultTextBrush(NULL),
		pSelectedTextBrush(NULL),
		pDefaultTextBkBrush(NULL),
		pSelectedTextBkBrush(NULL),
		pDialogFrameBrush(NULL),
		pDefaultTextFormat(NULL),
		pSelectedTextFormat(NULL),
		pBitmapBackground(NULL){}
private:

	//テキスト文字色、背景色ブラシ
	ID2D1SolidColorBrush* pDefaultTextBrush=NULL;
	ID2D1SolidColorBrush* pSelectedTextBrush=NULL;
	ID2D1LinearGradientBrush* pDefaultTextBkBrush=NULL;
	ID2D1LinearGradientBrush* pSelectedTextBkBrush=NULL;
	ID2D1SolidColorBrush* pDialogFrameBrush=NULL;

	//テキストフォーマット
	IDWriteTextFormat* pDefaultTextFormat=NULL;
	IDWriteTextFormat* pSelectedTextFormat=NULL;

	//背景画像
	ID2D1Bitmap* pBitmapBackground=NULL;

	//テキストフォーマット作成
	bool CreateTextFormat(IDWriteTextFormat** ppTextFormat,HFONT hFont);
	//背景色ブラシ作成
	bool CreateTextBkBrush(ID2D1LinearGradientBrush** ppTextBkBrush,COLORREF clr,COLORREF clrGradientEndColor);
public:

	bool IsLoadedBackgroundImage(){
		return pBitmapBackground!=NULL;
	}

	void ReleaseTextFormat(){
		SAFE_RELEASE(pDefaultTextFormat);
		SAFE_RELEASE(pSelectedTextFormat);
	}

	//通常行テキストフォーマット作成
	bool CreateDefaultTextFormat(HFONT hFont){
		return CreateTextFormat(&pDefaultTextFormat,hFont);
	}

	//選択行テキストフォーマット作成
	bool CreateSelectedTextFormat(HFONT hFont){
		return CreateTextFormat(&pSelectedTextFormat,hFont);
	}

	void SetTextRenderingParams(UINT uGamma,UINT uEnhancedContrast,UINT uClearTypeLevel,UINT uRenderingMode);

	void ReleaseTextBrush(){
		SAFE_RELEASE(pDefaultTextBrush);
		SAFE_RELEASE(pSelectedTextBrush);
	}

	void ReleaseTextBkBrush(){
		SAFE_RELEASE(pDefaultTextBkBrush);
		SAFE_RELEASE(pSelectedTextBkBrush);
	}

	//通常行文字色ブラシ作成
	bool CreateDefaultTextBrush(COLORREF clrText){
		if(pRenderTarget==NULL)return false;
		SAFE_RELEASE(pDefaultTextBrush);
		return SUCCEEDED(pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(clrText),&pDefaultTextBrush));
	}

	//選択行文字色ブラシ作成
	bool CreateSelectedTextBrush(COLORREF clrText){
		if(pRenderTarget==NULL)return false;
		SAFE_RELEASE(pSelectedTextBrush);
		return SUCCEEDED(pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(clrText),&pSelectedTextBrush));
	}

	//通常行背景色ブラシ作成
	bool CreateDefaultTextBkBrush(COLORREF clr,COLORREF clrGradientEndColor);

	//選択行背景色ブラシ作成
	bool CreateSelectedTextBkBrush(COLORREF clr,COLORREF clrGradientEndColor);

	void ReleaseDialogFrameBrush(){
		SAFE_RELEASE(pDialogFrameBrush);
	}

	//フレームブラシ作成
	bool CreateDialogFrameBrush(COLORREF clrDialogFrame){
		if(pRenderTarget==NULL)return false;
		SAFE_RELEASE(pDialogFrameBrush);

		return SUCCEEDED(pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(GetRValue(clrDialogFrame)/255.0f,GetGValue(clrDialogFrame)/255.0f,GetBValue(clrDialogFrame)/255.0f,1.0f),&pDialogFrameBrush));
	}

	//通常行背景を描画
	void DrawDefaultTextBackground(const D2D1_RECT_F& rect){
		if(pRenderTarget==NULL||pDefaultTextBkBrush==NULL)return;
		pRenderTarget->PushAxisAlignedClip(rect,D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
		pRenderTarget->FillRectangle(rect,pDefaultTextBkBrush);
		pRenderTarget->PopAxisAlignedClip();
	}

	//選択行背景を描画
	void DrawSelectedTextBackground(const D2D1_RECT_F& rect){
		if(pRenderTarget==NULL||pSelectedTextBkBrush==NULL)return;
		pRenderTarget->PushAxisAlignedClip(rect,D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
		pRenderTarget->FillRectangle(rect,pSelectedTextBkBrush);
		pRenderTarget->PopAxisAlignedClip();
	}

	//通常行文字を描画
	void DrawSelectedText(const TCHAR* pszText,UINT uLength,const D2D1_RECT_F* rect){
		if(pRenderTarget==NULL||pSelectedTextFormat==NULL||pSelectedTextBrush==NULL)return;

		D2D1_DRAW_TEXT_OPTIONS TextOptions=D2D1_DRAW_TEXT_OPTIONS_CLIP;

		RTL_OSVERSIONINFOW rovi={};

		rovi.dwOSVersionInfoSize=sizeof(RTL_OSVERSIONINFOW);

		if(GetWindowsVersion(&rovi)&&
		   (rovi.dwMajorVersion>=10||
			rovi.dwMajorVersion==6&&rovi.dwMinorVersion==3)){
			//v141_xp対応のため数値表記
			//また、絵文字非対応環境(Windows7等)でD2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONTを指定すると、
			//それまで描画していた背景色を含め消えてしまうため、対応環境(8.1以降)でのみ追加する
			TextOptions|=/*D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT*/(D2D1_DRAW_TEXT_OPTIONS)0x00000004;
		}

		pRenderTarget->DrawText(pszText,
								uLength,
								pSelectedTextFormat,
								rect,
								pSelectedTextBrush,
								TextOptions);
	}

	//選択行文字を描画
	void DrawDefaultText(const TCHAR* pszText,UINT uLength,const D2D1_RECT_F* rect){
		if(pRenderTarget==NULL||pDefaultTextFormat==NULL||pDefaultTextBrush==NULL)return;

		D2D1_DRAW_TEXT_OPTIONS TextOptions=D2D1_DRAW_TEXT_OPTIONS_CLIP;

		RTL_OSVERSIONINFOW rovi={};

		rovi.dwOSVersionInfoSize=sizeof(RTL_OSVERSIONINFOW);

		if(GetWindowsVersion(&rovi)&&
		   (rovi.dwMajorVersion>=10||
			rovi.dwMajorVersion==6&&rovi.dwMinorVersion==3)){
			//v141_xp対応のため数値表記
			//また、絵文字非対応環境(Windows7等)でD2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONTを指定すると、
			//それまで描画していた背景色を含め消えてしまうため、対応環境(8.1以降)でのみ追加する
			TextOptions|=/*D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT*/(D2D1_DRAW_TEXT_OPTIONS)0x00000004;
		}

		pRenderTarget->DrawText(pszText,
								uLength,
								pDefaultTextFormat,
								rect,
								pDefaultTextBrush,
								TextOptions);
	}

	//フレームを描画
	void DrawDialogFrame(const D2D1_RECT_F& rect,float fStrokeWidth){
		if(pRenderTarget==NULL||pDialogFrameBrush==NULL)return;
		pRenderTarget->DrawRectangle(rect,pDialogFrameBrush,fStrokeWidth);
	}

	//背景画像読み込み
	bool LoadBackgroundImage(const TCHAR* szImagePath,BYTE byResizePercent,BYTE byOpacity);

	void ReleaseBackgroundImage(){
		SAFE_RELEASE(pBitmapBackground);
	}

	//背景画像を描画
	void DrawBackground(const D2D1_RECT_F& rect,int iXOffset,int iYOffset);

	void ReleaseTextObject(){
		ReleaseTextFormat();
		ReleaseTextBrush();
		ReleaseTextBkBrush();
		ReleaseDialogFrameBrush();
	}

	void Uninitialize(){
		ReleaseTextObject();
		ReleaseBackgroundImage();
		Direct2DBase::Uninitialize();
	}
};




}//namespace direct2d

#endif //TASCHER_DIRECT2D_H
