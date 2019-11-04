#pragma once


TMPL2(T, U)
struct rng_pair { T x; U y;
	rng_pair(T a, U b) : x(a), y(b) {}
	T begin() { return x; }
	U end() { return y; } };

#define DEF_RNG_ITER(nm, v, dr, inc, cmp) \
	struct nm { v; void operator++() { inc; } \
	TMPL(T) bool operator!=(T q) { return cmp; } \
	auto& operator*() { return dr; } };

struct CoffStrTab
{
	char* add(cch* str, int len);
	xArray<xstr> lst;
};

TMPL2(T,U) T safe_deref(T* p, U def) {
	return p ? *p : (T)def; }

struct CoffObjLd
{
	int load(const char* name);
	enum { ERROR_NONE, ERROR_LOAD, ERROR_EOF };
	
	struct ObjRelocs
	{
		DWORD offset;
		DWORD symbol;
		WORD type;
	} __attribute__((packed));
	
	
	struct ObjSymbol
	{
		union { char* name; struct { 
			DWORD Name1, Name2; }; };
		
		DWORD Value;
		SHORT Section;
		WORD Type;
		BYTE StorageClass;
		BYTE NumberOfAuxSymbols;
		
		int aux_type();
		
	} __attribute__((packed));
	
	struct Section : IMAGE_SECTION_HEADER
	{
		char*& name() { return *(char**)Name; }
		xarray<byte> data(CoffObjLd& This) { 
			return {This.get(PointerToRawData), SizeOfRawData}; }
		DWORD get32(CoffObjLd& This, DWORD offset) { 	
			return  PointerToRawData ? safe_deref(
				data(This).getp<int>(offset), -1) : 0; }

		xarray<ObjRelocs> relocs(CoffObjLd& This) {
			return {This.get(PointerToRelocations), NumberOfRelocations}; }
	};
	
	// section helper functions
	int findSect(cch* name);
	xarray<byte> sectData(int iSect) {
		return sections[iSect].data(*this); }
	xarray<ObjRelocs> relocs(int iSect) {
		return sections[iSect].relocs(*this); }
	

	xarray<ObjSymbol> symbols;
	xarray<Section> sections;
	
	
	#define OBJ_SYM_ITER(o, ...) \
		for(int iSym = 0; iSym < (o).symbols.len; \
		iSym +=  (o).symbols[iSym].NumberOfAuxSymbols+1) { \
			auto& sym = (o).symbols[iSym]; __VA_ARGS__; }
	#define OBJ_SECT_ITER(o, ...) \
		for(int iSect = 0; iSect < (o).sections.len; iSect++) { \
			auto& sect = (o).sections[iSect]; __VA_ARGS__; }
	
	
	
	Void get(u32 offset) { return 
		offset ? fileData+offset : 0; }
	
	
	
	xArray<byte> fileData;
	
	
	// string access
	CoffStrTab strTab;
	char* pStrTab;
	char* strGet(DWORD*);
	
	
	
	// helper functions
	cch* class_name(int);
	cch* sect_name(DWORD);
	cch* aux_name(int);
	
	
	// auxillary types
	struct Aux_FuncDef {
		DWORD tagIndex;
		DWORD TotalSize;
		DWORD PointerToLinenumber;
		DWORD PointerToNextFunction;
		WORD Unused;
	
	} __attribute__((packed));
	
	struct Aux_SectDef {
		DWORD Length;
		WORD NumberOfRelocations;
		WORD NumberOfLinenumbers;
		DWORD CheckSum;
		WORD Number;
		BYTE Selection;
		BYTE Unused[3];
	
	} __attribute__((packed));	
	

	
	
	
	
};
