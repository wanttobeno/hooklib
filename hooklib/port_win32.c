#include "port_win32.h"
#include "windows.h"

#define MakePtr(cast, ptr, AddValue) (cast)((const char*)(ptr)+(AddValue))


int port_write_memory( const void* address,const void* buffer,int size )
{
	DWORD dwOldProtect;
	int ret;

	if( 0 == VirtualProtect( (LPVOID)address,size,PAGE_EXECUTE_READWRITE, &dwOldProtect) )
	{
		ret = GetLastError();
		return -ret;
	}
	memcpy( (void*)address,buffer,size );
	if( 0 == VirtualProtect( (LPVOID)address,size,dwOldProtect, &dwOldProtect) )
		return GetLastError();

	return 0;
}

int port_make_executeable( const void* buffer,int size )
{
	DWORD dwOldProtect;
	int ret;
	if( 0 == VirtualProtect( (LPVOID)buffer,size,PAGE_EXECUTE_READWRITE, &dwOldProtect) )
	{
		ret = GetLastError();
		return -ret;
	}
	return 0;
}

const void* port_help_get_function( const char* modulename,const char* functionname )
{
	HMODULE		h = (HMODULE)port_help_get_modulehandle( modulename,1 );
	if( 0 == h )return 0;

	return GetProcAddress( h,functionname );
}

void* port_alloc_fix_address( const unsigned char* address,int size )
{
	return VirtualAlloc( (LPVOID)address,size,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE );
}

void port_free_fix_address( const void* address )
{
	VirtualFree( (void*)address,0,MEM_RELEASE );
}

int port_get_module_size( const void* hmodule )
{
	PIMAGE_DOS_HEADER		pDOSHeader;
	PIMAGE_NT_HEADERS		pNTHeader;
	PIMAGE_OPTIONAL_HEADER	pOptionHeader;

	pDOSHeader = (PIMAGE_DOS_HEADER)hmodule;
	if( pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE )return 0;

	pNTHeader = MakePtr(PIMAGE_NT_HEADERS, pDOSHeader, pDOSHeader->e_lfanew);  
	if( pNTHeader->Signature != IMAGE_NT_SIGNATURE )return 0;

	pOptionHeader = &pNTHeader->OptionalHeader;

	return pOptionHeader->SizeOfImage;
}

const void* port_help_get_modulehandle( const char* modulename,int loadifnotexist )
{
	HMODULE		h = GetModuleHandleA( modulename );
	if( 0 == h && loadifnotexist )h = LoadLibraryA( modulename );
	return h;
}

const void* port_help_get_function2( const void* pmodule,const char* functionname )
{
	return (const void*)GetProcAddress( (HMODULE)pmodule,functionname );
}

PIMAGE_DATA_DIRECTORY	port_get_image_data_dir( const void* pmodule,int index )
{
	PIMAGE_DOS_HEADER pDOSHeader;
	PIMAGE_NT_HEADERS pNTHeader;

	pDOSHeader = (PIMAGE_DOS_HEADER) pmodule;   
	// Is this the MZ header?    
	if (IsBadReadPtr(pDOSHeader, sizeof(IMAGE_DOS_HEADER)) || (pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE)) return 0;     
	// Get the PE header.    
	pNTHeader = MakePtr(PIMAGE_NT_HEADERS, pDOSHeader, pDOSHeader->e_lfanew);  

	// Is this a real PE image?    
	if (IsBadReadPtr(pNTHeader, sizeof(IMAGE_NT_HEADERS)) || (pNTHeader->Signature != IMAGE_NT_SIGNATURE))  return 0;   
	
	return &pNTHeader->OptionalHeader.DataDirectory[index];

}

PIMAGE_IMPORT_DESCRIPTOR	port_lookup_import_dscriptor( const void* pmodule,PIMAGE_DATA_DIRECTORY dir,const char* pimportmodule)
{
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc,pend;
	const char*					p;	

	pImportDesc = MakePtr(PIMAGE_IMPORT_DESCRIPTOR, pmodule, dir->VirtualAddress);
	pend = MakePtr(PIMAGE_IMPORT_DESCRIPTOR, pmodule, dir->VirtualAddress+dir->Size);
	while( pImportDesc->Name && pImportDesc<pend)
	{

		p = MakePtr(const char*, pmodule, pImportDesc->Name);
		if( 0 == lstrcmpiA( p,pimportmodule ))return pImportDesc;
		pImportDesc++;
	}
	return 0;
}

PIMAGE_THUNK_DATA	port_lookup_import_thunk( const void* pmodule,PIMAGE_THUNK_DATA first,const void** import_slots,const char* pimportmodule,const char* pimportfunction )
{
	PIMAGE_THUNK_DATA		p;
	PIMAGE_IMPORT_BY_NAME	pByName;
	const void*				realfunction;

	realfunction = port_help_get_function( pimportmodule,pimportfunction );
	if( 0 == realfunction )return 0;

	p = first;
	while(p && p->u1.AddressOfData)
	{
		if( p->u1.Ordinal & IMAGE_ORDINAL_FLAG )
		{
			if( realfunction == import_slots[p-first] )
				return p;
		}else
		{
			 pByName = MakePtr(PIMAGE_IMPORT_BY_NAME, pmodule, p->u1.AddressOfData); 
			 if( 0 == strcmp( pimportfunction,(char*)pByName->Name )  )return p;
		}
		p++;
	}

	return 0;
}

const void** port_get_import_slot( const void* pmodule,const char* pimportmodule,const char* pimportfunction )
{
	PIMAGE_DATA_DIRECTORY	pDataDir;
	PIMAGE_IMPORT_DESCRIPTOR	pImport;
	PIMAGE_THUNK_DATA			firstthunk,findthunk;
	const void**				importslots;
	const void*					realfunction;

	pDataDir = port_get_image_data_dir( pmodule,IMAGE_DIRECTORY_ENTRY_IMPORT );
	if( 0 == pDataDir )return 0;

	pImport = port_lookup_import_dscriptor( pmodule,pDataDir,pimportmodule );
	if( 0 == pImport )return 0;

	importslots= MakePtr( const void**, pmodule, pImport->FirstThunk);

	if(pImport->OriginalFirstThunk)
	{
		firstthunk = MakePtr(PIMAGE_THUNK_DATA, pmodule, pImport->OriginalFirstThunk);
		findthunk = port_lookup_import_thunk( pmodule,firstthunk,importslots,pimportmodule,pimportfunction );
		if( findthunk )
		{
			return importslots+(findthunk-firstthunk);
		}
	}

	realfunction = port_help_get_function( pimportmodule,pimportfunction );
	if( 0 == realfunction )return 0;

	while( *importslots )
	{
		if( *importslots == realfunction )return importslots;
		importslots++;
	}

	return 0;
}
