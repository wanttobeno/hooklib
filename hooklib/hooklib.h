#ifndef _LIBHOOK_
#define _LIBHOOK_

#if defined( HOOKLIB_EXPORTS ) && defined(_MSC_VER)
#define HOOKLIB_API __declspec(dllexport)
#else
#define HOOKLIB_API //__declspec(dllimport)
#endif


#ifndef WINVER                          // 指定要求的最低平台是 Windows Vista。
#define WINVER 0x0600           // 将此值更改为相应的值，以适用于 Windows 的其他版本。
#endif

#ifndef _WIN32_WINNT            // 指定要求的最低平台是 Windows Vista。
#define _WIN32_WINNT 0x0600     // 将此值更改为相应的值，以适用于 Windows 的其他版本。
#endif

#ifndef _WIN32_WINDOWS          // 指定要求的最低平台是 Windows 98。
#define _WIN32_WINDOWS 0x0410 // 将此值更改为适当的值，以适用于 Windows Me 或更高版本。
#endif

#ifndef _WIN32_IE                       // 指定要求的最低平台是 Internet Explorer 7.0。
#define _WIN32_IE 0x0700        // 将此值更改为相应的值，以适用于 IE 的其他版本。
#endif

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头中排除极少使用的资料
// Windows 头文件:
#include <windows.h>

#define DEF_FUNCTION_NAMED_TYPE( ret_type,call_type,type_name,... ) typedef ret_type (call_type* type_name)(__VA_ARGS__)
#define FUNCTION_UNNAMED_TYPE( ret_type,call_type,... ) (ret_type (call_type*)(__VA_ARGS__))
#define CAST_FUNCTION_POINTER( ret_type,call_type,function,... ) (FUNCTION_UNNAMED_TYPE( ret_type,call_type,__VA_ARGS__ )function)
//DEF_FUNCTION_NAMED_TYPE( int,WINAPI,fnMessageBoxA,HWND hwnd,LPCSTR t,LPCSTR ,UINT );
//(FUNCTION_UNNAMED_TYPE(int,__stdcall,HWND hwnd,LPCSTR t,LPCSTR ,UINT)&ctx.proxy_function_code)( 0,"test","ok",0);


#ifdef __cplusplus
extern "C"
{
#endif

struct _hook_context_
{
	const void*		orginal_function_ptr;//原函数,或者原函数地址的地址
	int				orginal_code_size;//原代码长度
	unsigned char	orginal_code[32];//原代码
	const void*		proxy_function_ptr;
	unsigned char	proxy_function_code[64];//代理函数,原函数头复制过来的新函数,调用此函数，可以实现原函数的功能
};

//mincopysize为至少需要复制的数量,复制机器码到目标地址,返回复制的字节数量
//如果返回值小于mincopysize，请注意了，一定是遇到jmp指令，无法分析下面的代码了
HOOKLIB_API int		hook_copy_code( const void* from,int fromsize,void* to,int tosize,int mincopysize );

//返回buffer中jmp代码长度,失败返回0
HOOKLIB_API int		hook_make_jmp( void* buffer,const void* jmpfrom,const void* jmpto );

//写代码段
HOOKLIB_API int		hook_write_memory( const void* address,const void* buffer,int size );

//使得缓冲区可执行，可读写
HOOKLIB_API int		hook_make_executeable( const void* buffer,int size );

//失败返回负数,传入的ctx必须全部清0
// 生成代理函数
HOOKLIB_API	int		hook_install(struct _hook_context_* ctx,const void* function,const void* newfunction );

//失败返回负数,传入的ctx必须全部清0
HOOKLIB_API	int		hook_install_import(struct _hook_context_* ctx,const void* pmodule,const char* pimport_module_name,const char* pimport_function_name,const void* newfunction );

//使用jmp qword[offset32] 跳转，长度只有6字节
HOOKLIB_API	int		hook_install_by_bridgeslot( struct _hook_context_* ctx,const void* function,const void* newfunction,const void** bridgeslot );
HOOKLIB_API	int		hook_uninstall(struct _hook_context_* ctx );

HOOKLIB_API	const void*		hook_help_get_function( const char* modulename,const char* functionname );
HOOKLIB_API	const void*		hook_help_get_function2( const void* pmodule,const char* functionname );
HOOKLIB_API	const void*		hook_help_get_modulehandle( const char* modulename);
HOOKLIB_API const void**	hook_help_alloc_bridgeslot( const void* modulehandle,int maxcount );
HOOKLIB_API void			hook_help_free_bridgeslot( const void**  bridgeslot);
HOOKLIB_API	int				hook_is_x64();//是否64位 

#ifdef __cplusplus
}
#endif



#endif