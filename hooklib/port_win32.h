#pragma once

int port_write_memory(const void* address,const void* buffer,int size);
int	port_make_executeable( const void* buffer,int size );//使得缓冲区可执行，可读写
const void* port_help_get_function( const char* modulename,const char* functionname );
const void* port_help_get_function2( const void* pmodule,const char* functionname );
const void* port_help_get_modulehandle( const char* modulename,int loadifnotexist );//loadifnotexist如果不存在，再加载
void*		port_alloc_fix_address( const unsigned char* address,int size );//分配一个固定地址的内存，权限为RW
void		port_free_fix_address( const void* address );//分配一个固定地址的内存，权限为RW
int			port_get_module_size( const void* hmodule );
const void** port_get_import_slot( const void* pmodule,const char* pimportmodule,const char* pimportfunction);//取得导入表项中,存放导入函数地址的位置,