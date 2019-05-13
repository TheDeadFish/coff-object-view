#include "coff-view.h"

void init_symbols(HWND hwnd)
{
	ListView_DeleteAllItems(hListSym);
	dlgCombo_reset(hwnd, IDC_COMBO1);
	
	
	int sectSymb = IsDlgButtonChecked(hwnd, IDC_SECT_SYM) ?
		dlgCombo_getSel(hwnd, IDC_COMBO2)+1 : 0;
	
	OBJ_SYM_ITER(object, 	
		if(sectSymb && sectSymb != sym.Section) continue;
		
		// set the symbol name
		dlgCombo_addStr(hwnd, IDC_COMBO1, 
			Xstrfmt("%d,   %s", iSym, sym.name), iSym);
		int i = lstView_iosText(hListSym, -1, sym.name);
		lstView_iosText(hListSym, i, 1,
			object.sect_name(sym.Section));
		lstView_fmt(hListSym, i, 2, "%s (%d)", object
			.class_name(sym.StorageClass), sym.StorageClass);
		lstView_setHex(hListSym, i, 3, sym.Value);
		lstView_setHex(hListSym, i, 4, sym.Type, 0);
		lstView_setHex(hListSym, i, 5, sym.NumberOfAuxSymbols, 0);
	)
	
	for(int i = 0; i < 6; i++) {
		if(i != 3) lstView_autoSize(hListSym, i); }
		
	dlgCombo_setSel(hwnd, IDC_COMBO1, 0);
}

void symbol_select(HWND hwnd)
{
	if(viewSel != 0) return;
	SetDlgItemTextA(hwnd, IDC_INFO, "");
	int sel = dlgCombo_getSel(hwnd, IDC_COMBO1);
	if(sel < 0) return;
	auto& symb = object.symbols[
		dlgCombo_getData(hwnd, IDC_COMBO1, sel)];
		
	// determine format
	int type = symb.aux_type();
	if(type < 0) return;
	
	
	Bstr str;
	
	// 
	str.fmtcat("Aux Type: %s\r\n", object.aux_name(type));
	
	
	#define MAKE_AUX(type) auto& aux \
		= *(CoffObjLd::type*)(&symb+1)	
	
	
	switch(type) {
	case 1: {
		MAKE_AUX(Aux_FuncDef);
		str.fmtcat("  tagIndex: %d\r\n", aux.tagIndex);
		str.fmtcat("  TotalSize: %d\r\n", aux.TotalSize);
		str.fmtcat("  PointerToLinenumber: %d\r\n", aux.PointerToLinenumber);
		str.fmtcat("  PointerToNextFunction: %d\r\n", aux.PointerToNextFunction);
		break; }
		
	case 4: {
		cch* name = (cch*)(&symb+1);
		int len = strnlen(name, sizeof(symb) 
			* symb.NumberOfAuxSymbols);
		str.fmtcat("  Name: %v\r\n", name, len);
		break; }
		
	case 5: {
		MAKE_AUX(Aux_SectDef);
		str.fmtcat("  Length: %d\r\n", aux.Length);
		str.fmtcat("  NumberOfRelocations: %d\r\n", aux.NumberOfRelocations);
		str.fmtcat("  NumberOfLinenumbers: %d\r\n", aux.NumberOfLinenumbers);
		str.fmtcat("  CheckSum: %08X\r\n", aux.CheckSum);
		str.fmtcat("  Comdat Section: %d\r\n", aux.Number);
		str.fmtcat("  Comdat Selection: %d\r\n", aux.Selection);
		break; }
	}
	
	SetDlgItemTextA(hwnd, IDC_INFO, str.data);
}
