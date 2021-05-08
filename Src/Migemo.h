//Migemo.h
//Migemoを使用した正規表現

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.64
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/


#ifndef TASCHER_MIGEMO_H
#define TASCHER_MIGEMO_H

namespace migemo{

//bregexp
typedef struct bregexp{
	const TCHAR* outp;
	const TCHAR* outendp;
	const int splitctr;
	const TCHAR** splitp;
	INT_PTR rsv1;
	TCHAR* parap;
	TCHAR* paraendp;
	TCHAR* transtblp;
	TCHAR** startp;
	TCHAR** endp;
	int nparens;
}BREGEXP;

bool IsAvailable();
//Migemo/bregonig読み込み
bool Load();
//Migemo/bregonig解放
bool Unload();
//Migemoを用い正規表現リストを作成
TCHAR** AllocRegexes(TCHAR* szQuery,int* piRegexCount);
//正規表現リストを解放
void FreeRegexes(TCHAR*** pppszRegexes);
//演算子を指定
int SetOperator(const unsigned char* szOption);
//正規表現検索
int Match(BREGEXP* pBregExp,TCHAR* str,TCHAR* target,TCHAR* targetendp);

class CompiledBlock{
public:
	CompiledBlock():pBregExp(NULL){}
	~CompiledBlock();
	inline BREGEXP* Get()const{return pBregExp;}
	BREGEXP* pBregExp;
};

}//namespace migemo

#endif //TASCHER_MIGEMO_H
