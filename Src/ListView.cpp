//ListView.cpp
//リストビュー操作

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.64
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/


#include"StdAfx.h"
#include"ListView.h"

//現在のカラム位置からカラム番号を取得
int ListView_GetSubItemIndex(HWND hListView,int iIndex){
	if(iIndex<0||iIndex>=LISTITEM_NUM)return -1;
	int iOrders[LISTITEM_NUM]={};
	ListView_GetColumnOrderArray(hListView,LISTITEM_NUM,&iOrders);
	return iOrders[iIndex];
}

//全てのカラムの幅を取得
int ListView_GetAllColumnWidth(struct LISTVIEWCOLUMN_TABLE* pListViewColumn_Table){
	int iWidth=0;

	for(int i=0;i<LISTITEM_NUM;i++){
		if(pListViewColumn_Table[i].bVisible){iWidth+=*pListViewColumn_Table[i].piWidth;
		}
	}
	return iWidth;
}

//カラムの数を取得
int ListView_GetColumnCount(struct LISTVIEWCOLUMN_TABLE* pListViewColumn_Table){
	int iCount=0;

	for(int i=0;i<LISTITEM_NUM;i++){
		if(pListViewColumn_Table[i].bVisible)iCount++;
	}
	return iCount;
}

//カラムの情報を初期化
void ListView_InitColumn(struct LISTVIEWCOLUMN_TABLE* pListViewColumn_Table,struct CONFIG* pConfig,HWND hListView){
	pListViewColumn_Table[LISTITEM_ICON].iListItem=LISTITEM_ICON;
	//アイコンカラムは非表示にしない
	pListViewColumn_Table[LISTITEM_ICON].bVisible=true;
	pListViewColumn_Table[LISTITEM_ICON].piWidth=&pConfig->ListView.iIconWidth;
	pListViewColumn_Table[LISTITEM_ICON].iDefaultWidth=pConfig->GetListIconSize()+g_Config.ListView.iIconMargin*2;

	pListViewColumn_Table[LISTITEM_FILENAME].iListItem=LISTITEM_FILENAME;
	pListViewColumn_Table[LISTITEM_FILENAME].bVisible=pConfig->ListView.iFileNameWidth!=0;
	pListViewColumn_Table[LISTITEM_FILENAME].piWidth=&pConfig->ListView.iFileNameWidth;
	pListViewColumn_Table[LISTITEM_FILENAME].iDefaultWidth=DEFAULT_FILENAME_WIDTH;

	pListViewColumn_Table[LISTITEM_WINDOWTITLE].iListItem=LISTITEM_WINDOWTITLE;
	pListViewColumn_Table[LISTITEM_WINDOWTITLE].bVisible=pConfig->ListView.iWindowTitleWidth!=0;
	pListViewColumn_Table[LISTITEM_WINDOWTITLE].piWidth=&pConfig->ListView.iWindowTitleWidth;
	pListViewColumn_Table[LISTITEM_WINDOWTITLE].iDefaultWidth=DEFAULT_WINDOWTITLE_WIDTH;

	if(hListView==NULL)return;
	for(int i=0;i<LISTITEM_NUM;i++){
		ListView_SetColumnWidth(hListView,i,*pListViewColumn_Table[i].piWidth);
	}
	return;
}

//カラムの並び順を変更
bool ListView_ChangeColumnOrder(HWND hListView){
	int iOrders[][LISTITEM_NUM]={
		{LISTITEM_ICON,LISTITEM_FILENAME,LISTITEM_WINDOWTITLE},
		{LISTITEM_ICON,LISTITEM_WINDOWTITLE,LISTITEM_FILENAME},
	};
	int iCurrentOrders[LISTITEM_NUM];

	ListView_GetColumnOrderArray(hListView,LISTITEM_NUM,&iCurrentOrders);

	return ListView_SetColumnOrderArray(hListView,LISTITEM_NUM,(iCurrentOrders[1]==iOrders[0][1])?iOrders[1]:iOrders[0])!=0;
}

//先頭を選択する
void ListView_SelectFirstItem(HWND hListView){
	ListView_EnsureVisible(hListView,0,false);
	ListView_SetItemState(hListView,0,LVIS_FOCUSED|LVIS_SELECTED,LVIS_FOCUSED|LVIS_SELECTED);
	return;
}

//末尾を選択する
void ListView_SelectLastItem(HWND hListView){
	ListView_EnsureVisible(hListView,ListView_GetItemCount(hListView)-1,false);
	ListView_SetItemState(hListView,ListView_GetItemCount(hListView)-1,LVIS_FOCUSED|LVIS_SELECTED,LVIS_FOCUSED|LVIS_SELECTED);
	return;
}

//一つ上を選択する
void ListView_SelectPrevItem(HWND hListView){
	int iIndex=ListView_GetSelectedItem(hListView);

	if(!iIndex){
		ListView_SelectLastItem(hListView);
	}else{
		iIndex--;
		ListView_EnsureVisible(hListView,iIndex,false);
		ListView_SetItemState(hListView,iIndex,LVIS_FOCUSED|LVIS_SELECTED,LVIS_FOCUSED|LVIS_SELECTED);
	}
	return;
}

//一つ下を選択する
void ListView_SelectNextItem(HWND hListView){
	int iIndex=ListView_GetSelectedItem(hListView);

	if(iIndex==ListView_GetItemCount(hListView)-1){
		ListView_SelectFirstItem(hListView);
	}else{
		iIndex++;
		ListView_EnsureVisible(hListView,iIndex,false);
		ListView_SetItemState(hListView,iIndex,LVIS_FOCUSED|LVIS_SELECTED,LVIS_FOCUSED|LVIS_SELECTED);
	}
	return;
}


//指定したIndexのアイテムを選択する
void ListView_SelectItem(HWND hListView,int iIndex){
	ListView_EnsureVisible(hListView,iIndex,false);
	ListView_SetItemState(hListView,iIndex,LVIS_FOCUSED|LVIS_SELECTED,LVIS_FOCUSED|LVIS_SELECTED);
	return;
}

//選択を解除する
void ListView_SelectNone(HWND hListView){
	ListView_SetItemState(hListView,ListView_GetNextItem(hListView,-1,LVNI_SELECTED),0,LVIS_FOCUSED|LVIS_SELECTED);
}

//選択しているアイテムのIndexを取得する
int ListView_GetSelectedItem(HWND hListView){
	return ListView_GetNextItem(hListView,-1,LVNI_SELECTED);
}

//カーソル下にあるアイテムのIndexを取得する
int ListView_GetHitItem(HWND hListView){
	LVHITTESTINFO lvHitTestInfo={};

	GetCursorPos(&lvHitTestInfo.pt);
	ScreenToClient(hListView,&lvHitTestInfo.pt);

	ListView_HitTest(hListView,&lvHitTestInfo);
	return (lvHitTestInfo.flags&LVHT_ONITEM)?lvHitTestInfo.iItem:-1;
}
