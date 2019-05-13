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
			Xstrfmt("%d,   %s", iSym, sym.name)); 
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
	auto& symb = object.symbols[sel];
	
	
	
	
	




	




}
