#include <stdshit.h>
#include <win32hlp.h>
#include "resource.h"
#include "object.h"

// helper functions
void lstView_autoSize(HWND hList, int iCol);
void lstView_setHex(HWND hListSym, int item, 
	int subItem, DWORD value, int len=8);


extern CoffObjLd object;
extern HWND hListSym;
extern HWND hListSect;
extern HWND hListRel;
extern int viewSel;

// section functions
void init_sections(HWND hwnd);
void section_select(HWND hwnd);


