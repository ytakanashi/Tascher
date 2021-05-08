//Gdiplus.cpp
//Gdiplus操作関数

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.64
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/


#include"StdAfx.h"
#include"Gdiplus.h"

namespace gdiplus{
bool Initialize::bInitialized;

//画像を読み込み
bool BackgroundImage::Load(const TCHAR* szImagePath){
	if(pBitmapOriginal!=NULL){
		delete pBitmapOriginal;
		pBitmapOriginal=NULL;
	}
	pBitmapOriginal=Gdiplus::Bitmap::FromFile(szImagePath);
	return pBitmapOriginal!=NULL;
}

//背景描画用のGdiplus::Bitmapを作成
bool BackgroundImage::CreateBitmap(int iWidth,int iHeight,int iXOffset,int iYOffset,BYTE byResizePercent,BYTE byOpacity){
	if(pBitmapOriginal==NULL)return false;

	if(pBitmapBackup==NULL){
		if(pBitmap!=NULL){
			delete pBitmap;
			pBitmap=NULL;
		}
		//GdipAlloc()
		pBitmap=new Gdiplus::Bitmap(iWidth,iHeight);
		Gdiplus::Graphics graphics(pBitmap);
		Gdiplus::Rect rcDest(iXOffset,iYOffset,
							pBitmapOriginal->GetWidth()*byResizePercent/100,
							pBitmapOriginal->GetHeight()*byResizePercent/100);
		//不透明度設定
		Gdiplus::ImageAttributes imageattr;
		Gdiplus::ColorMatrix colormatrix={
			1.0f,0.0f,0.0f,0.0f,0.0f,
			0.0f,1.0f,0.0f,0.0f,0.0f,
			0.0f,0.0f,1.0f,0.0f,0.0f,
			0.0f,0.0f,0.0f,(Gdiplus::REAL)byOpacity/100,0.0f,
			0.0f,0.0f,0.0f,0.0f,1.0f
		};
		imageattr.SetColorMatrix(&colormatrix);

		graphics.DrawImage(pBitmapOriginal,
						   rcDest,
						   0,0,
						   pBitmapOriginal->GetWidth(),pBitmapOriginal->GetHeight(),
						   Gdiplus::UnitPixel,&imageattr);
		//バックアップ
		pBitmapBackup=pBitmap->Clone(0,0,iWidth,iHeight,PixelFormatDontCare);
	}else{
		delete pBitmap;
		pBitmap=NULL;
		pBitmap=new Gdiplus::Bitmap(iWidth,iHeight);
		Gdiplus::Graphics graphics(pBitmap);
		Gdiplus::Rect rcDest(0,0,iWidth,iHeight);
		//バックアップから描画
		graphics.DrawImage(pBitmapBackup,
						   rcDest,0,0,iWidth,iHeight,Gdiplus::UnitPixel);
	}
	return true;
}

//背景画像を描画
void BackgroundImage::DrawImage(HDC hDC,RECT rc){
	if(pBitmap==NULL)return;

	Gdiplus::Graphics graphics(hDC);

	//背景を描画
	graphics.DrawImage(pBitmap,
					   rc.left,rc.top,
					   rc.left,rc.top,
					   rc.right-rc.left,rc.bottom-rc.top,
					   Gdiplus::UnitPixel);
}

}//namespace gdiplus
