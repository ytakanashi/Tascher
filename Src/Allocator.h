//Allocator.h
//new/delete代替

/*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
	Tascher Ver.1.64
	Coded by x@rgs

	This code is released under NYSL Version 0.9982
	See NYSL_withfaq.TXT for further details.

	「Tascher」は、マウスの移動とホイールの回転や
	Migemoインクリメンタルサーチでウインドウを切り替えるソフトウェアです。
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*/


#ifndef TASCHER_ALLOCATOR_H
#define TASCHER_ALLOCATOR_H


#include <windows.h>

class Allocator{
protected:
	Allocator(){}
	~Allocator(){}
public:
	void* operator new(size_t s){
		return HeapAlloc(GetProcessHeap(),0,s);
	}
	void* operator new[](size_t s){
		return HeapAlloc(GetProcessHeap(),0,s);
	}
	void operator delete(void* p){
		if(p!=NULL)HeapFree(GetProcessHeap(),0,p);
	}
	void operator delete[](void* p){
		if(p!=NULL)HeapFree(GetProcessHeap(),0,p);
	}
private:
	Allocator(const Allocator&);
	Allocator& operator=(const Allocator&);
};

#endif //TASCHER_ALLOCATOR_H
