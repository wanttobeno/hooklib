#include "HookLib.h"
#define SUPPORT_64BIT_OFFSET
#include <distorm.h>
#include <mnemonics.h>
#include <stdio.h>
#include "port_win32.h"
#define IsX64 ( sizeof(void*) == 8 )

	//x86 ia32需要处理如下指令,这些指令兼容64位,但得注意，offset为有符号数,寻址范围不超过4G
	/*
	inst:call rip+offset
	hex:e8 offset( 4 bytes )
	dest:rip+5+offset

	inst:jmp rip+offset
	hex:e9 offset( 4 bytes )
	des:rip+5+offset

	inst:jmp short rip+offset
	hex:eb offset( 1 bytes )
	des:rip+2+offset

	inst:jmpc  rip+offset
	hex:0f [80-8f] offset( 4 bytes )
	des:rip+5+offset

	inst:jmpc short rip+offset
	hex:[70-7f] offset( 1 bytes )
	dest:rip+2+offset

	inst:jecxz short rip+offset
	hex:e3 offset( 1 bytes )
	dest:rip+2+offset

	inst:loopxxx short rip+offset loop指令最大寻址范围256
	hex:[e0-e2] offset( 1 bytes )
	dest:rip+2+offset

	amd64(类 op qword[offset32] )
	inst:jmp qword[offset32]	//与ia32不同
	hex:ff25 offset32
	dest:qword [rip+6+offset32]

	inst:call qword[offset32] //与ia32不同
	hex:ff15 offset32
	dest:qword [rip+6+offset32]

	//0000000076B212BF 44 39 1D 76 0E 02 00 cmp         dword ptr [76B4213Ch],r11d 
	inst: cmp dword ptr[offset32],r11d
	hex:44 39 1D offset32
	dest:qword [rip+7+offset32]

	amd64需要处理的，太多了，暂时就处理到这吧

	jmp代码

	jmp qword[address]
	ff25 00000000 xx xx xx xx xx xx xx xx 14字节
	*/


//O_PC jmp rip+xxxx
//O_SMEN jmp [rip+xxxx]

int isneedadjust( _DInst* inst)
{
	int i;
	for (i = 0; ((i < OPERANDS_NO) && (inst->ops[i].type != O_NONE)); i++)
	{
		if( O_SMEM == inst->ops[i].type )
		{
			if( R_RIP == inst->ops[i].index )
			{
				return 1;
			}
		}
	}
	return O_PC == inst->ops[0].type;
}

#ifdef _DEBUG
void dumpInst( const _CodeInfo* codeInfo,const _DInst* inst )
{
	_DecodedInst	decodedInstructions;

	distorm_format( codeInfo,inst,&decodedInstructions );
	printf("%08I64x (%02d) %-24s %s%s%s\r\n", decodedInstructions.offset, decodedInstructions.size, (char*)decodedInstructions.instructionHex.p, (char*)decodedInstructions.mnemonic.p, decodedInstructions.operands.length != 0 ? " " : "", (char*)decodedInstructions.operands.p);
}
#endif // _DEBUG

uint64_t abs64( int64_t a )
{
	return a>=0?a:-a;
}
uint32_t abs32( int32_t a )
{
	return a>=0?a:-a;
}

//jmp qword[offset32]
unsigned char*	make_jmp_qword_disp( unsigned char* jmpbuffer,const unsigned char* jmpfrom,const unsigned char* jmp_disp_address )
{
	if( IsX64 && ( abs64(jmp_disp_address-jmpfrom-6) > 0x000000007FFFFFFF ) )
	{
		return jmpbuffer;
	}

	*jmpbuffer++ = 0xff;
	*jmpbuffer++ = 0x25;
	if( IsX64 )
	{
		*(uint32_t*)jmpbuffer = (uint32_t)(jmp_disp_address-jmpfrom-6);
	}else
	{
		*(uint32_t*)jmpbuffer = (uint32_t)jmp_disp_address;
	}
	jmpbuffer += 4;
	return jmpbuffer;
}

/*
短程转移（直接短转移）
指令格式：JMP SHORT OPRD
*/
unsigned char*	make_jmp_short( unsigned char* jmpbuffer,const unsigned char* jmpfrom,const unsigned char* jmpto,unsigned char op_code_short )
{
	*jmpbuffer++ = op_code_short;
	*jmpbuffer++ = (uint8_t)(jmpto-jmpfrom-2);
	return jmpbuffer;
}

/*
近程转移（段内直接转移）
指令格式：JMP NEAR PTR OPRD
*/
unsigned char*	make_jmp_near( unsigned char* jmpbuffer,const unsigned char* jmpfrom,const unsigned char* jmpto,int iscall )//jmpto为目标地址,iscall=0时为jmp
{
	if( IsX64 && ( abs64(jmpto-jmpfrom-5) > 0x000000007FFFFFFF ) )
	{

		//jmp qword[next instructs]
		*jmpbuffer++ = 0xff;
		*jmpbuffer++ = iscall?0x15:0x25;//0x15为call
		

		*jmpbuffer++ = 0;
		*jmpbuffer++ = 0;
		*jmpbuffer++ = 0;
		*jmpbuffer++ = iscall?2:0;

		/*
		jmp:
		jmp qword [0];
		qd	jmpto	

		call:
		call qword[2];
		jmp offset L8	//eb 08,跳过下面8字节数据
		qd jmpto	/8字节
		L8:

		*/
		if(iscall)
		{
			*jmpbuffer++ = 0xeb;
			*jmpbuffer++ = 0x08;
		}

		*(uint64_t*)(jmpbuffer) = (uint64_t)jmpto;
		jmpbuffer += 8;

		
	}else if( 0 == iscall && (abs32((int32_t)(jmpto-jmpfrom-5)) < 0x00000080 ) )//jmp short
	{
		jmpbuffer = make_jmp_short( jmpbuffer,jmpfrom,jmpto,0xeb );
	}else
	{
		*jmpbuffer++ = iscall?0xe8:0xe9;

		*(int32_t*)jmpbuffer = (int32_t)(jmpto-jmpfrom-5);
		jmpbuffer += 4;
	}
	return jmpbuffer;
}

//jmpfrom为发起跳转的地址
//jmp_condition_to为条件满足时，跳转的地址
//jmp_uncondtion_to为条件不满足时，跳转地址

unsigned char*	make_jmpc_near( unsigned char* jmpbuffer,const unsigned char* jmpfrom,const unsigned char* jmp_condition_to,const unsigned char* jmp_uncondtion_to,unsigned char code0,unsigned char code1 )//jmp condtion,code0,code1指定跳转指令前两字节
{
	unsigned char* p_jmpc_short;

	if( IsX64 && ( abs64(jmp_condition_to-jmpfrom-5) > 0x000000007FFFFFFF ) )
	{

		/*
		
		jmpc short L_to_condition
		jmp jmp_uncondtion_to
L_to_condition:
		jmp jmp_condition_to
		*/

		
		if( 0x0f == code0 )
		{
			code0 = code1-0x10;
		}

		p_jmpc_short = jmpbuffer;
		jmpbuffer+=2;
		jmpbuffer = make_jmp_near( jmpbuffer,jmpbuffer,jmp_uncondtion_to,0 );	//jmp jmp_uncondtion_to
					make_jmp_short( p_jmpc_short,p_jmpc_short,jmpbuffer,code0 );//jmpc short L_to_condition
		jmpbuffer = make_jmp_near( jmpbuffer,jmpbuffer,jmp_condition_to,0 );	//jmp jmp_condition_to

	}else
	{
		*jmpbuffer++ = code0;
		if( 0x0f == code0 )//jmpc near offset32
		{
			*jmpbuffer++ = code1;
		}else//jmpc short offet8,转成jmpc near offset32
		{
			*jmpbuffer++ = code1+0x10;
		}
		
		*(int32_t*)jmpbuffer = (int32_t)(jmp_condition_to-jmpfrom-5);
		jmpbuffer += 4;

		jmpbuffer = make_jmp_near( jmpbuffer,jmpbuffer,jmp_uncondtion_to,0 );
	}
	return jmpbuffer;
}

#define get_dest_address_0xe9( pos ) ((char*)pos+5+*(int32_t*)((char*)pos+1))//jmp near
#define get_dest_address_0xeb( pos ) ((char*)pos+2+*(int8_t*)((char*)pos+1))//jmp short

//不需要考虑lock前缀，因为使用相对偏移的指令，不能lock
//hook后必须FlushInstructionCache
HOOKLIB_API int	 hook_copy_code( const void* from,int fromsize,void* to,int tosize,int mincopysize )
{
	_CodeInfo		codeInfo = {0};
	_DInst			inst={0};
	_DecodeResult	result;
	unsigned int	instCount = 0;
	unsigned char*	pfrom = (unsigned char*)from;
	unsigned char*	pfrom_end = pfrom+fromsize;
	unsigned char*	pto = (unsigned char*)to;
	unsigned char*	p = 0;
	unsigned char*	pto_end = pto+tosize;

	while( pfrom < pfrom_end && pto<pto_end  )
	{
		codeInfo.code		= (const uint8_t*)pfrom;
		codeInfo.codeLen	= (int)(pfrom_end-pfrom);
		codeInfo.codeOffset = (_OffsetType)pfrom;
		codeInfo.features	= DF_NONE;
		codeInfo.dt			= IsX64?Decode64Bits:Decode32Bits;

		instCount = 0;
		result = distorm_decompose( &codeInfo,&inst,1,&instCount );
		if( DECRES_SUCCESS != result && instCount != 1 )return 0;

#ifdef _DEBUG
		dumpInst( &codeInfo,&inst );
		isneedadjust(&inst);
#endif // _DEBUG
		
		if( 0xe9 == pfrom[0] //jmp offset32,
			|| 0xe8 == pfrom[0] )//call offset32
		{
			pto = make_jmp_near( pto,pto,get_dest_address_0xe9(pfrom),0xe8 == pfrom[0] );
			if(0xe9 == pfrom[0])//call
			{
				break;
			}

		}else if( 0xeb == pfrom[0] )//jmp offset8,
		{
			pto = make_jmp_near( pto,pto,get_dest_address_0xeb(pfrom),0 );
			break;

		}else if( 0x0f == pfrom[0] && (pfrom[1] >= 0x80 && pfrom[1]<=0x8f) )// jmpc offset32
		{
			pto = make_jmpc_near( pto,pto,(pfrom+6+*(int32_t*)(pfrom+2)),(pfrom+6),pfrom[0],pfrom[1] );
			break;
		}else if(pfrom[0] >= 0x70 && pfrom[0]<=0x7f)  //jmpc offset8
		{
			pto = make_jmpc_near( pto,pto,(pfrom+2+*(int8_t*)(pfrom+1)),(pfrom+2),pfrom[0],pfrom[1] );
			break;
		}else if( pfrom[0] >= 0xe0 && pfrom[0] <= 0xe3 )//loop offset8,if( 0xe3 == pfrom[0] )//jecxz offset8 这种情况没法处理,因为没有对应的loop/jecxz offset32指令
		{
			break;
		}else if( isneedadjust(&inst) )
		{
			// Fix Win7 sp1 x64 Hook MessageBoxA  Failed
			//  cmp dword ptr [gfEMIEnable (774B213Ch)],r11d 
			//  [xxx] 导致代理函数奔溃
	/*		if( pfrom < (unsigned char*)from+mincopysize )
			{
				memcpy( pto,pfrom,inst.size );
				pto += inst.size;
				break;
			}
			else*/
			{
				inst.size = 0;
				break;
			}
		}else
		{
			memcpy( pto,pfrom,inst.size );
			pto += inst.size;
		}
		
		if( pfrom > (unsigned char*)from+mincopysize )
			break;

		pfrom += inst.size;
	}
	pfrom += inst.size;

	if( pfrom < pfrom_end && pto<pto_end  )
	{
		pto = make_jmp_near( pto,pto,pfrom,0 );
	}

	return (int)(pfrom-(unsigned char*)from);
}

HOOKLIB_API int  hook_make_jmp( void* buffer,const void* jmpfrom,const void* jmpto )
{
	unsigned char* p;
	unsigned char* pend;

	p = (unsigned char*)buffer;
	pend = make_jmp_near( p,(const unsigned char*)jmpfrom,(const unsigned char*)jmpto,0 );

	return (int)(pend-p);
}

HOOKLIB_API	int hook_install( struct _hook_context_* ctx,const void* function,const void* newfunction )
{
	unsigned char	jmpcode[24] = {0};
	int				jmpcodelen = 0;
	unsigned char*	p;

	if( 0 == ctx || 0 == function || 0 == newfunction )return -1;
	if(ctx->orginal_code_size>0)return 1;
	memset( ctx,0,sizeof(struct _hook_context_) );

	ctx->orginal_function_ptr = function;

	jmpcodelen = hook_make_jmp( jmpcode,function,newfunction );

	hook_make_executeable(ctx->proxy_function_code,sizeof(ctx->proxy_function_code));

	ctx->proxy_function_ptr = (const void*)&ctx->proxy_function_code ;

	ctx->orginal_code_size = hook_copy_code( function,100,ctx->proxy_function_code,sizeof(ctx->proxy_function_code),jmpcodelen );
	if( ctx->orginal_code_size < jmpcodelen )
	{
		memset( ctx,0,sizeof(struct _hook_context_) );
		return -1;
	}

	memcpy( ctx->orginal_code,function,ctx->orginal_code_size );

	hook_write_memory( function,jmpcode,jmpcodelen );

	return 0;
}

HOOKLIB_API	int hook_install_import( struct _hook_context_* ctx,const void* pmodule,const char* pimport_module_name,const char* pimport_function_name,const void* newfunction )
{
	const void** import_slot;

	if( 0 == ctx || 0 == pmodule || 0 == pimport_module_name || 0 == pimport_function_name || 0 == newfunction )return -1;
	if(ctx->orginal_code_size>0)return 1;
	memset( ctx,0,sizeof(struct _hook_context_) );

	import_slot = port_get_import_slot( pmodule,pimport_module_name,pimport_function_name );
	if( 0 == import_slot )return -1;

	ctx->orginal_function_ptr = (const void*)import_slot;
	ctx->orginal_code_size = sizeof(void*);
	memcpy( ctx->orginal_code,ctx->orginal_function_ptr,ctx->orginal_code_size );
	ctx->proxy_function_ptr = *import_slot;
	hook_write_memory( import_slot,&newfunction,sizeof(void*) );

	return 0;
}

HOOKLIB_API	int hook_install_by_bridgeslot( struct _hook_context_* ctx,const void* function,const void* newfunction,const void** bridgeslot )
{
	unsigned char	jmpcode[24] = {0};
	int				jmpcodelen = 0;
	unsigned char*	p;
	const void*		orignal_function = function;
	const void**	bridge_slot = (void**)bridgeslot;

	if( 0 == ctx || 0 == bridgeslot || 0 == function || 0 == newfunction )return -1;
	if(ctx->orginal_code_size>0)return 1;
	memset( ctx,0,sizeof(struct _hook_context_) );

	(*(int*)bridge_slot) ++;//最前面一个元素，存放当前数量,真正存放指针，从第2个元素开始
	bridge_slot += *(int*)bridge_slot;

	//jmp qword[offset32]
	p = make_jmp_qword_disp( jmpcode,orignal_function,(const unsigned char*)bridge_slot );
	jmpcodelen = (int)(p-jmpcode);
	if( 0 == jmpcodelen )return -1;

	ctx->orginal_function_ptr = orignal_function;

	//复制原始函数头字节到proxy_function_code
	hook_make_executeable(ctx->proxy_function_code,sizeof(ctx->proxy_function_code));
	ctx->proxy_function_ptr = (const void*)&ctx->proxy_function_code;
	ctx->orginal_code_size = hook_copy_code( orignal_function,100,ctx->proxy_function_code,sizeof(ctx->proxy_function_code),jmpcodelen );
	if( ctx->orginal_code_size < jmpcodelen )
	{
		memset( ctx,0,sizeof(struct _hook_context_) );
		return -1;
	}

	//保存头字节
	memcpy( ctx->orginal_code,orignal_function,ctx->orginal_code_size );

	*bridge_slot = newfunction;
	hook_write_memory( orignal_function,jmpcode,jmpcodelen );
	return 0;
}

HOOKLIB_API	int hook_uninstall( struct _hook_context_* ctx )
{
	int n;
	if( 0 == ctx )return -1;
	if( 0 == ctx->orginal_function_ptr || ctx->orginal_code_size < 1)return -1;


	n = hook_write_memory( ctx->orginal_function_ptr,ctx->orginal_code,ctx->orginal_code_size );
	if( n >=0 )
	{
		memset( ctx,0,sizeof(struct _hook_context_) );
	}
	return n;
}

HOOKLIB_API int hook_write_memory( const void* address,const void* buffer,int size )
{
	return port_write_memory( address,buffer,size );
}

HOOKLIB_API int hook_make_executeable( const void* buffer,int size )
{
	return port_make_executeable( buffer,size );
}

HOOKLIB_API	const void* hook_help_get_function( const char* modulename,const char* functionname )
{
	return port_help_get_function( modulename,functionname );
}

HOOKLIB_API const void** hook_help_alloc_bridgeslot( const void* modulehandle,int maxcount )
{
	unsigned int i;
	const void** p =0;
	const char*  pmodule;
	int			size = max( (maxcount+1)*sizeof(void*),0x1000);
	int			modulesize;

	modulesize = port_get_module_size(modulehandle);
	if( 0 == modulesize )return 0;
	
	pmodule = (const char*)modulehandle;
	pmodule -= ((uint32_t)pmodule & 0xfff );
	for( i = 0;i<0x8ffff000/0x1000;++i)
	{
		pmodule -= size;
		p = (const void**)port_alloc_fix_address( pmodule,size );
		if( p )
		{
			p[0] = 0;
			return p;
		}
	}

	pmodule = (const char*)modulehandle;
	pmodule += modulesize+0x1000;
	pmodule += ((uint32_t)pmodule & 0xfff );	
	for( i = 0;i<0x8ffff000/0x1000;++i)
	{
		pmodule += size;
		p = (const void**)port_alloc_fix_address( pmodule,size );
		if( p )
		{
			p[0] = 0;
			return p;
		}
	}
	return 0;
}

HOOKLIB_API void hook_help_free_bridgeslot( const void** bridgeslot )
{
	port_free_fix_address( bridgeslot );
}

HOOKLIB_API	const void* hook_help_get_modulehandle( const char* modulename )
{
	return port_help_get_modulehandle( modulename,1 );
}

HOOKLIB_API	int hook_is_x64()
{
	return (int)IsX64;
}

HOOKLIB_API	const void* hook_help_get_function2( const void* pmodule,const char* functionname )
{
	return port_help_get_function2(pmodule,functionname );
}

