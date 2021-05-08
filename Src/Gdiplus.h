//Gdiplus.h
//Gdiplus操作関数

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.64
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/


#ifndef TASCHER_GDIPLUS_H
#define TASCHER_GDIPLUS_H

#include<gdiplus.h>
#include"Allocator.h"

namespace gdiplus{

//GdiplusStartup()とGdiplusShutdown()を繰り返すとメモリリークするらしいので静的メンバで対策
struct Initialize{
	Initialize():ulToken(0){
		if(!bInitialized){
			Gdiplus::GdiplusStartupInput gdiplusStartupInput;
			bInitialized=(Gdiplus::GdiplusStartup(&ulToken,&gdiplusStartupInput,NULL)==Gdiplus::Ok);
		}
	}
	~Initialize(){
		if(ulToken){
			Gdiplus::GdiplusShutdown(ulToken);
		}
	}

private:
	static bool bInitialized;
	Initialize(const Initialize&);
	Initialize& operator=(const Initialize&);
	ULONG_PTR ulToken;
};

struct BackgroundImage:public Allocator{
	//表示用Bitmap
	Gdiplus::Bitmap* pBitmap;
	//バックップ用Bitmap
	Gdiplus::Bitmap* pBitmapBackup;
	//オリジナルのBitmap
	Gdiplus::Bitmap* pBitmapOriginal;

	BackgroundImage():
		pBitmap(NULL),
		pBitmapBackup(NULL),
		pBitmapOriginal(NULL){}
	~BackgroundImage(){}

	bool IsLoaded(){
		return pBitmapOriginal!=NULL;
	}

	//画像を読み込み
	bool Load(const TCHAR* szImagePath);

	//背景描画用のGdiplus::Bitmapを作成
	bool CreateBitmap(int iWidth,int iHeight,int iXOffset,int iYOffset,BYTE byResizePercent,BYTE byOpacity);

	void ReleaseBitmapOriginal(){
		delete pBitmapOriginal;
		pBitmapOriginal=NULL;
	}

	void ReleaseBitmapBackup(){
		if(pBitmapBackup!=NULL){
			delete pBitmapBackup;
			pBitmapBackup=NULL;
		}
	}

	void ReleaseBitmap(){
		if(pBitmap!=NULL){
			delete pBitmap;
			pBitmap=NULL;
		}
	}

	void Release(){
		ReleaseBitmapOriginal();
		ReleaseBitmapBackup();
		pBitmapBackup=NULL;
		if(pBitmap!=NULL){
			delete pBitmap;
			pBitmap=NULL;
		}
	}

	//背景画像を描画
	void DrawImage(HDC hDC,RECT rc);
};

}//namespace gdiplus

#endif //TASCHER_GDIPLUS_H
