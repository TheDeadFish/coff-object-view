#include "coff-view.h"

const char progName[] = "Object viewer";

static
void showWindow(HWND hwnd, bool show) {
	ShowWindow(hwnd, show ? SW_SHOW : SW_HIDE); }
	
void lstView_autoSize(HWND hList, int iCol)
{
	ListView_SetColumnWidth(hList, iCol, LVSCW_AUTOSIZE_USEHEADER);
	int headSize = ListView_GetColumnWidth(hList, iCol);
	ListView_SetColumnWidth(hList, iCol, LVSCW_AUTOSIZE);
	int bodySize = ListView_GetColumnWidth(hList, iCol);
	if(bodySize < headSize)
		ListView_SetColumnWidth(hList, iCol, headSize);
}





CoffObjLd object;
HWND hListSym;
HWND hListSect;
HWND hListRel;
int viewSel;


void lstView_initCol(HWND hList, cch* const lst[]) {
	for(int i = 0; lst[i]; i++) {
		lstView_insColumn(hList, i, 78, lst[i]); } }
cch* const symLst[] = {"Name", "Sect", "Class", "Value", "Type","Aux", 0 };
cch* const sectLst[] = { "Name", "Size","Base","Flags", 0};
cch* const relLst[] = { "Symbol", "Offset", "Type", "Value", 0 };

void lstView_setHex(HWND hListSym, int item, int subItem, DWORD value, int len)
{	char buff[32]; sprintf(buff, "%0*X", len, value);
	lstView_iosText(hListSym, item, subItem, buff); }
	
void lstView_fmt(HWND hListSym, int item, int subItem, cch* fmt, ...)
{
	char buff[1024]; va_list va; va_start(va, fmt);
	vsnprintf(buff, 1024, fmt, va);
	
	
	
	lstView_iosText(hListSym, item, subItem, buff);
}

void dlg_reset(HWND hwnd)
{
	ListView_DeleteAllItems(hListSym);
	ListView_DeleteAllItems(hListSect);
	ListView_DeleteAllItems(hListRel);
	dlgCombo_reset(hwnd, IDC_COMBO1);
	dlgCombo_reset(hwnd, IDC_COMBO2);
	setDlgItemText(hwnd, IDC_EDIT1, "");
}

void init_relocs(HWND hwnd)
{
	ListView_DeleteAllItems(hListRel);
	
	int curSect = dlgCombo_getSel(hwnd, IDC_COMBO2);
	
	auto& sect = object.sections[curSect];
	
	for(auto& reloc : object.relocs(curSect)) {
		cch* name = object.symbols[reloc.symbol].name;
		int i = lstView_iosText(hListRel, -1, name);
		int data = sect.get32(object, reloc.offset);
		
		lstView_setHex(hListRel, i, 1, reloc.offset);
		lstView_setHex(hListRel, i, 2, reloc.type, 2);
		lstView_setHex(hListRel, i, 3, data, 8);
		
	}
	
	lstView_autoSize(hListRel, 0);
	



}

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

int load_object(HWND hwnd, cch* name)
{
	// load the object file
	dlg_reset(hwnd);
	IFRET(object.load(name));
	setDlgItemText(hwnd, IDC_EDIT1, name);
	
	init_symbols(hwnd);
	
	init_sections(hwnd);
	

	
	
	dlgCombo_setSel(hwnd, IDC_COMBO2, 0);
	init_relocs(hwnd);
	
	return 0;
}

void selectTab(HWND hwnd)
{
	// select the window to show
	viewSel = getDlgTabPage(hwnd, IDC_TAB1);
	showWindow(hListSym, viewSel == 0);
	showWindow(hListSect, viewSel == 1);
	showWindow(hListRel, viewSel == 2);
	section_select(hwnd);
}

void mainDlgInit(HWND hwnd)
{
	// get listView handles
	hListSym = GetDlgItem(hwnd, IDC_LIST1);
	hListSect = GetDlgItem(hwnd, IDC_LIST2);
	hListRel = GetDlgItem(hwnd, IDC_LIST3);
	
	// initialize tab control
	addDlgTabPage(hwnd, IDC_TAB1, 0, "Symbols");
	addDlgTabPage(hwnd, IDC_TAB1, 1, "Sections");
	addDlgTabPage(hwnd, IDC_TAB1, 2, "Relocs");
	selectTab(hwnd);
	
	// set listView style
	ListView_SetExtendedListViewStyle(hListSym, LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);
	ListView_SetExtendedListViewStyle(hListSect, LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);
	ListView_SetExtendedListViewStyle(hListRel, LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);	
	
	// initialize listView columns
	lstView_initCol(hListSym, symLst);
	lstView_initCol(hListSect, sectLst);
	lstView_initCol(hListRel, relLst);

	/*OBJ_SYM_ITER(object, 
		int i = lstView_iosText(hListSym, -1, sym.name);
		
		lstView_setHex(hListSym, i, 1, sym.Value);
		
		
		lstView_fmt(hListSym, i, 2, "%d: %s", sym.Section,
			object.sect_name(sym.Section));
		
		
		lstView_setHex(hListSym, i, 3, sym.Type);
		

		
		//lstView_setText(hListSym, i, 4, sym.StorageClass);
		
		
	
	
	)*/
		
		
		
		



}

void load_module(HWND hwnd)
{
	// load the module file
	OpenFileName ofn;
	if(!ofn.doModal(hwnd)) return;
	if(load_object(hwnd, ofn.lpstrFile)) contError(
		hwnd, "failed to load module"); return;
}

void upd_relocs(HWND hwnd)
{
	init_relocs(hwnd);
	if(IsDlgButtonChecked(hwnd, IDC_SECT_SYM))
		init_symbols(hwnd);
	section_select(hwnd);
}

void combo_userSel(HWND hwnd, int ctrlId, int sel)
{
	if(dlgCombo_getSel(hwnd, IDC_COMBO2) == sel) return;
	dlgCombo_setSel(hwnd, IDC_COMBO2, sel);
	HWND hcb = GetDlgItem(hwnd, ctrlId);
	sendMessage(GetParent(hcb), WM_COMMAND, 
		MAKEWPARAM(ctrlId, CBN_SELCHANGE), (LPARAM)hcb);
}

BOOL CALLBACK mainDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DLGMSG_SWITCH(
	  CASE_COMMAND(
	    ON_COMMAND(IDCANCEL, EndDialog(hwnd, 0))
			ON_COMMAND(IDC_LOAD, load_module(hwnd))
			ON_COMMAND(IDC_SECT_SYM, init_symbols(hwnd))
			
			ON_CONTROL(CBN_SELCHANGE, IDC_COMBO2, upd_relocs(hwnd))
			
			//ON_CONTROL_RANGE(EN_CHANGE, IDC_MOD_BASE, IDC_RVA_ADDR,
			//	edt_update(hwnd, LOWORD(wParam)))
			//ON_CONTROL_RANGE(EN_KILLFOCUS, IDC_MOD_BASE, IDC_RVA_ADDR,
			//	edt_validate(hwnd, LOWORD(wParam)))
	  	  
	  ,) 

		ON_MESSAGE(WM_INITDIALOG, mainDlgInit(hwnd))
		
		CASE_NOTIFY(
			ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, selectTab(hwnd))
			ON_LVN_NOTIFY(LVN_ITEMCHANGED, IDC_LIST2,
				combo_userSel(hwnd, IDC_COMBO2, nmv.iItem))		
	  ,)
	,)
}

int main()
{
	DialogBoxW(NULL, MAKEINTRESOURCEW(IDD_DIALOG1), NULL, mainDlgProc);
}


