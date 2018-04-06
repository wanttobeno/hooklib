#pragma once

#include "hooklib.h"
#include "macro_repeat.h"

class HOOKLIB_API CHookBridgeslot
{
public:
	CHookBridgeslot();
	~CHookBridgeslot();

	bool	alloc(const void* modulehandle,int maxcount);
	void	free();

	const void* getModuleHandle();
	void		setModuleHandle(const void* modulehandle);
	bool	isValid();

	const void** bridgeslot;
private:
	const void*	m_modulehandle;
};



class HOOKLIB_API CHookContext
{
public:
	CHookContext();
	~CHookContext();
	bool				install( const void* function,const void* newfunction );
	bool				installImport( const void* pmodule,const char* pimport_module_name,const char* pimport_function_name,const void* newfunction );
	bool				installByBridgeslot( const void* function,const void* newfunction,const CHookBridgeslot& bridgeslot );
	void				unInstall();
	bool				isValid();

	const void*			getProxyFunction();
	void				setProxyFunction( const void* function );
private:
	struct _hook_context_ ctx;
};

typedef enum
{
	HOOK_CALL_TYPE_CDECL,//__cdecl c语言调用方式 
	HOOK_CALL_TYPE_STDCALL,//winapi 调用方式
	HOOK_CALL_TYPE_PASCALL,//pascall
}HOOK_CALL_TYPE;

template<typename ret_type=void*,HOOK_CALL_TYPE call_type=HOOK_CALL_TYPE_STDCALL>
class THookContext:public CHookContext
{
public:

	ret_type			emit();
	ret_type			operator()();
/*
	template< typename _argv0 >
	ret_type			emit( _argv0 argv0 );

	template< typename _argv0 >
	ret_type operator()( _argv0 argv0 )
*/
	//下面的宏，要复制制作 15个emit和operator,参数最多15个，如需要更多，自己弄去

#define __typename_param(n, ...) typename _argv##n,
#define __typename_end(n, ...) typename _argv##n
#define repeat_typename(n, ...) __repeat(n,__typename_param,__typename_end)

#define __type_param(n, ...)  _argv##n argv##n,
#define __type_param_end(n, ...)  _argv##n argv##n
#define repeat_type(n, ...) __repeat(n,__type_param,__type_param_end)

#define __argv_param(n, ...)  argv##n,
#define __argv_param_end(n, ...)  argv##n
#define repeat_argv(n, ...) __repeat(n,__argv_param,__argv_param_end)

#define repeat_emit(n, ...)\
	template< repeat_typename(n,__VA_ARGS__) >\
	ret_type			emit( repeat_type(n,__VA_ARGS__) )\
	{\
		const void*	function = getProxyFunction();\
		if(function)\
		switch( call_type )\
		{\
			case  HOOK_CALL_TYPE_CDECL:\
				return CAST_FUNCTION_POINTER( ret_type,__cdecl,function,repeat_type(n,__VA_ARGS__) )(repeat_argv(n,__VA_ARGS__));\
			case  HOOK_CALL_TYPE_STDCALL:\
				return CAST_FUNCTION_POINTER( ret_type,__stdcall,function,repeat_type(n,__VA_ARGS__) )(repeat_argv(n,__VA_ARGS__));\
			case  HOOK_CALL_TYPE_PASCALL:\
				return CAST_FUNCTION_POINTER( ret_type,pascal,function,repeat_type(n,__VA_ARGS__) )(repeat_argv(n,__VA_ARGS__));\
		}\
		return 0;\
	}

#define repeat_operator(n, ...)\
	template< repeat_typename(n,__VA_ARGS__) >\
	ret_type operator()( repeat_type(n,__VA_ARGS__))\
	{\
		return emit(repeat_argv(n,__VA_ARGS__));\
	}

#define __for_each_number_begin_with_1(v, ...) \
	v(1, __VA_ARGS__) \
	v(2, __VA_ARGS__) \
	v(3, __VA_ARGS__) \
	v(4, __VA_ARGS__) \
	v(5, __VA_ARGS__) \
	v(6, __VA_ARGS__) \
	v(7, __VA_ARGS__) \
	v(8, __VA_ARGS__) \
	v(9, __VA_ARGS__) \
	v(10, __VA_ARGS__) \
	v(11, __VA_ARGS__) \
	v(12, __VA_ARGS__) \
	v(13, __VA_ARGS__) \
	v(14, __VA_ARGS__) \
	v(15, __VA_ARGS__)

__for_each_number_begin_with_1(repeat_emit);
__for_each_number_begin_with_1(repeat_operator);
};

//偏特化,虽然没有必要，还是写上吧

template<HOOK_CALL_TYPE call_type>
class THookContext<void,call_type>:public CHookContext
{
public:
	void			emit();

};

HOOKLIB_API bool	prepareHookBridge(CHookBridgeslot& modulebridge,const char* modulename);//最多支持0x1000/sizeof(void*)-1个函数
HOOKLIB_API bool	hookModuleFunction2( CHookBridgeslot& modulebridge,CHookContext* phook_ctx,const char* functionname,const void* newfunction,bool disable_jmp_bridge );
HOOKLIB_API bool	hookModuleFunction( CHookBridgeslot& modulebridge,CHookContext* phook_ctx,const void* function,const void* newfunction,bool disable_jmp_bridge );

#include "callemit.h"