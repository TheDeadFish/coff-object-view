#include <stdshit.h>
#include "object.h"
//TMPL(T) void pRst(T* obj) { pDel(obj); pNew(obj); }

template <class T>
T* ovf_ofsp(T* p, size_t ofs) {
	if(__builtin_add_overflow(size_t(p), ofs, &ofs))
		return (T*)(size_t)-1; return (T*)ofs; }	
template <class T>
T* ovf_addp(T* p, size_t ofs) {
	if(__builtin_mul_overflow(sizeof(T), ofs, &ofs))
		return (T*)(size_t)-1; return ovf_ofsp(p, ofs); }
		

static
bool str_check(cch* str, cch* end) {
	for(; str < end; str++) if(!*str) {
		return true; } return false; }


		

char* CoffStrTab::add(cch* str, int len)
{
	for(char* x : lst) {
		if(!strncmp(x, str, len)) return x; }
	lst.push_back(xstrdup(str, len));
	return lst.back();
}

struct CoffObjRd : xArray<byte>
{
	bool load(cch* name) { xArray::init(loadFile(name)); return data; }
	
	bool offset(DWORD& dst, DWORD ofs, DWORD len1, DWORD len2=1) {
		return  __builtin_mul_overflow(len1, len2, &len1) ||
		__builtin_add_overflow(len1, ofs, &dst) || dst >= len; }
	bool check(DWORD ofs, DWORD len1, DWORD len2=1) {
		return offset(ofs, ofs, len1, len2); }
	
	Void get(DWORD offset) { return Void(data, offset); }
	

};

int CoffObjLd::load(const char* name)
{
	// load the section
	pRst(this); CoffObjRd rd; 
	if(!rd.load(name)) return ERROR_LOAD;
	fileData.init(rd);
		
	// object file
	if(rd.check(0, sizeof(IMAGE_FILE_HEADER))) 
		return ERROR_EOF;
	IMAGE_FILE_HEADER* objHeadr = rd.get(0);
	DWORD iSectPos = sizeof(IMAGE_FILE_HEADER);
	
	// pe file
	if(RW(objHeadr) == 'ZM') {
		IMAGE_DOS_HEADER* idh = rd.get(0);
		if(rd.check(idh->e_lfanew, sizeof(IMAGE_NT_HEADERS32)))
			return ERROR_EOF;
		IMAGE_NT_HEADERS32* peHeadr = rd.get(idh->e_lfanew);
		objHeadr = &peHeadr->FileHeader;
		iSectPos = offsetof(IMAGE_NT_HEADERS, OptionalHeader) + 
			peHeadr->FileHeader.SizeOfOptionalHeader + idh->e_lfanew;
	}

	// get section table
	DWORD nSects = objHeadr->NumberOfSections;
	if(rd.check(iSectPos, nSects, sizeof(IMAGE_SECTION_HEADER)))
		return ERROR_EOF;
	sections.init(rd.get(iSectPos), nSects);
	
	// check symbol table & string table
	DWORD nSymbols = objHeadr->NumberOfSymbols;
	DWORD iSymbols = objHeadr->PointerToSymbolTable;
	if(iSymbols) {
		symbols.init(rd.get(iSymbols), nSymbols);
		DWORD iStrTab; if(rd.offset(iStrTab, iSymbols, nSymbols,
			sizeof(ObjSymbol))) return ERROR_EOF;
		pStrTab = rd.get(iStrTab);
	}

	// validate symbols
	OBJ_SYM_ITER(*this, 
		char* str = strGet(&sym.Name1);
		if(!str) return ERROR_EOF;
		sym.name = str; )
	
	// load sections
	for(auto& sect : sections) {
	
		// section name
		if(sect.Name[0] == '/' && pStrTab) { 
			u32 i = atoi(PC(sect.Name+1));
			RI(sect.Name) = 0; RI(sect.Name,4) = i; }
		sect.name() = strGet(&sect.Name);
		if(!sect.name()) return ERROR_EOF;
		
	}
	
	fileData.init(rd.release());
	return ERROR_NONE;
}	
	
char* CoffObjLd::strGet(void* name)
{
	if(RU(name)) return strTab.add((char*)name, 8);
	char* str = ovf_ofsp(pStrTab, RU(name,4));
	//printf("%X, %X, %X\n", name[1], str, fileData.end());
	return str_check(str, (char*)fileData.end()) ? str : 0;
}

int CoffObjLd::findSect(cch* name)
{
	//for(int i = 0; i < sections.len; i++) {
	//	cstr name = sections[i].name(*this);
	//	if(!name.cmp(name)) return i; }
	//return -1;
	return -1;
}

struct class_list_t { int  val; cch* str; };
static const class_list_t class_list[] = {
	{-1, "END_OF_FUNCTION"}, {0, "NULL"}, {1, "AUTOMATIC"}, 
	{2, "EXTERNAL"}, {3, "STATIC"}, {4, "REGISTER"}, 
	{5, "EXTERNAL_DEF"}, {6, "LABEL"}, {7, "UNDEFINED_LABEL"}, 
	{8, "MEMBER_OF_STRUCT"}, {8, "ARGUMENT"}, {10, "STRUCT_TAG"}, 
	{11, "MEMBER_OF_UNION"}, {12, "UNION_TAG"}, {13, "TYPE_DEFINITION"}, 	
	{14, "UNDEFINED_STATIC"}, {15, "ENUM_TAG"}, {16, "MEMBER_OF_ENUM"}, 	
	{17, "REGISTER_PARAM"}, {18, "BIT_FIELD"}, {100, "BLOCK"},
	{101, "FUNCTION"}, {102, "END_OF_STRUCT"}, {103, "FILE"}, 
	{104, "SECTION"}, {105, "WEAK_EXTERNAL"}, {0, 0}};
	
cch* CoffObjLd::class_name(int c)
{
	for(auto& x : class_list) {
		if(x.val == c) return x.str; }
	return "UNKNOWN";
}

cch* CoffObjLd::sect_name(DWORD iSect)
{
	if(iSect == 0) return "UNDEFINED";
	if(iSect == (DWORD)-1) return "ABSOLUTE";
	if(iSect == (DWORD)-2) return "DEBUG";
	if(iSect > sections.len) return "";
	return sections[iSect-1].name();
}

int CoffObjLd::ObjSymbol::aux_type(void)
{
	if(!NumberOfAuxSymbols) 
		return -1;
	
	// Function Definitions
	if((StorageClass == 2)
	&&(Type == 32)&&(Section))
		return 1;
		
	// Weak Externals
	if((StorageClass == 2)
	&&(!Section)&&(!Value))
		return 3;
		
	// Files
	if(StorageClass == 103)
		return 4;
		
	// Section Definitions
	if(StorageClass == 3)
		return 5;
		
	return 0;
}

cch* CoffObjLd::aux_name(int i)
{
	switch(i) {
	case 1: return "Function Def";
	case 3: return "Weak symbol";
	case 4: return "Source file";
	case 5: return "Section Def";
	default: return "unknown";
	}
}
