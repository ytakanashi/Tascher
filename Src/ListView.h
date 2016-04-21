//ListView.h
//リストビューのカラム

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.61
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/


#ifndef LISTVIEW_H
#define LISTVIEW_H

#include"CommonSettings.h"


struct LISTVIEWCOLUMN_TABLE{
	int iListItem;
	bool bVisible;
	int* piWidth;
	int iDefaultWidth;
};



//カラムの位置を取得
int ListView_GetColumnIndex(struct LISTVIEWCOLUMN_TABLE* pListViewColumn_Table,int iListItem);

//全てのカラムの幅を取得
int ListView_GetAllColumnWidth(struct LISTVIEWCOLUMN_TABLE* pListViewColumn_Table);

//カラムの数を取得
int ListView_GetColumnCount(struct LISTVIEWCOLUMN_TABLE* pListViewColumn_Table);

//カラムの情報を初期化
void LisView_InitColumn(struct LISTVIEWCOLUMN_TABLE* pListViewColumn_Table,struct CONFIG* pConfig,HWND hListView);

//カラムの並び順を変更
bool ListView_ChangeColumnOrder(struct LISTVIEWCOLUMN_TABLE* pListViewColumn_Table,HWND hListView);

//リストビューで使用するイメージリストを登録
bool ListView_InitImageList(HWND hListView,bool bSmallImage);

//先頭を選択する
void ListView_SelectFirstItem(HWND hListView);

//末尾を選択する
void ListView_SelectLastItem(HWND hListView);

//一つ上を選択する
void ListView_SelectPrevItem(HWND hListView);

//一つ下を選択する
void ListView_SelectNextItem(HWND hListView);

//指定したIndexのアイテムを選択する
void ListView_SelectItem(HWND hListView,int iIndex);

//選択を解除する
void ListView_SelectNone(HWND hListView);

//選択しているアイテムのIndexを取得する
int ListView_GetSelectedItem(HWND hListView);

//カーソル下にあるアイテムのIndexを取得する
int ListView_GetHitItem(HWND hListView);

//背景を設定する
//bool ListView_SetImage(HWND hListView,LPCTSTR lpszImagePath,int ixOffsetPercent=0,int iyOffsetPercent=0);

#endif //LISTVIEW_H
