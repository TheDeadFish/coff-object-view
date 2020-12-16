#include "coff-view.h"

static cch* const sectFlags[] = {
	"TYPE_DSECT", "TYPE_NOLOAD","TYPE_GROUP","TYPE_NO_PAD",
	"TYPE_COPY","CNT_CODE","CNT_INITIALIZED_DATA","CNT_UNINITIALIZED_DATA",
	"LNK_OTHER","LNK_INFO","TYPE_OVER","LNK_REMOVE","LNK_COMDAT","0x2000",
	"NO_DEFER_SPEC_EXC", "MEM_FARDATA", "MEM_PURGEABLE","MEM_16BIT","MEM_LOCKED",
	"MEM_PRELOAD",0,0,0,0, "LNK_NRELOC_OVFL","MEM_DISCARDABLE","MEM_NOT_CACHED",
	 "MEM_NOT_PAGED",
	"MEM_SHARED","MEM_EXECUTE","MEM_READ","MEM_WRITE" };
	
	
static
u32 sectAlign(u32 flags) {
	u32 align = (flags >> 20) & 15;
	if(align) align = 1 << (align-1);
	return align;
}

char* fmt_sect(CoffObjLd& obj, int iSect)
{
	cch* name = obj.sect_name(iSect);
	return xstrfmt("%d,   %s", iSect, name);
}

void init_sections(HWND hwnd)
{
	OBJ_SECT_ITER(object, 
		xstr name = fmt_sect(object, iSect+1);
		dlgCombo_addStr(hwnd, IDC_COMBO2, name, iSect);
		lstView_iosText(hListSect, iSect, name);
		lstView_setHex(hListSect, iSect, 1, sect.SizeOfRawData);
		lstView_setHex(hListSect, iSect, 2, sect.VirtualAddress);
		lstView_setHex(hListSect, iSect, 3, sect.Characteristics);
	);
	
	lstView_autoSize(hListSect, 0);
}

void section_select(HWND hwnd)
{
	if(viewSel != 1) return;
	SetDlgItemTextA(hwnd, IDC_INFO, "");
	int sel = dlgCombo_getSel(hwnd, IDC_COMBO2);
	if(sel < 0) return;
	auto& sect = object.sections[sel];

	Bstr str;
	
	// characteristics
	u32 flags = sect.Characteristics;
	str.strcat("Characteristics = ");
	for(int i = 0; i < 32; i++) {
		if(_BTST(flags, i) && sectFlags[i]) {
			if(i) str.strcat("|"); 
			str.strcat(sectFlags[i]); }
	}
	
	SetDlgItemTextA(hwnd, IDC_INFO, str.data);
}
