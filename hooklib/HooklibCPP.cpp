#include "HooklibCPP.h"

CHookBridgeslot::CHookBridgeslot()
{
	bridgeslot = 0;
	m_modulehandle = 0;
}

CHookBridgeslot::~CHookBridgeslot()
{

}

bool CHookBridgeslot::alloc( const void* modulehandle,int maxcount )
{
	m_modulehandle = modulehandle;
	bridgeslot = hook_help_alloc_bridgeslot( modulehandle,maxcount );
	return 0!=bridgeslot;
}



void CHookBridgeslot::free()
{
	if(bridgeslot)hook_help_free_bridgeslot(bridgeslot);
}

const void* CHookBridgeslot::getModuleHandle()
{
	return m_modulehandle;
}

bool CHookBridgeslot::isValid()
{
	return 0!=bridgeslot;
}

void CHookBridgeslot::setModuleHandle( const void* modulehandle )
{
	m_modulehandle = modulehandle;
}


CHookContext::CHookContext()
{
	memset( &ctx,0,sizeof(ctx) );
}


CHookContext::~CHookContext()
{

}
const void* CHookContext::getProxyFunction()
{
	return ctx.proxy_function_ptr;
}

void CHookContext::setProxyFunction( const void* function )
{
	ctx.proxy_function_ptr = function;
}

bool CHookContext::install( const void* function,const void* newfunction )
{
	int n;
	n = hook_install( &ctx,function,newfunction );
	return n>=0;
}


bool CHookContext::installImport( const void* pmodule,const char* pimport_module_name,const char* pimport_function_name,const void* newfunction )
{
	int n;
	n = hook_install_import( &ctx,pmodule,pimport_module_name,pimport_function_name,newfunction );
	return n>=0;
}

bool CHookContext::installByBridgeslot( const void* function,const void* newfunction,const CHookBridgeslot& bridgeslot )
{
	int n;
	n = hook_install_by_bridgeslot( &ctx,function,newfunction,bridgeslot.bridgeslot );
	return n>=0;
}


void CHookContext::unInstall()
{
	hook_uninstall(&ctx);
}


bool CHookContext::isValid()
{
	return 0!=ctx.proxy_function_ptr;
}




bool prepareHookBridge(CHookBridgeslot& modulebridge,const char* modulename)//最多支持0x1000/sizeof(void*)-1个函数
{
	const void* pmodule;

	pmodule = hook_help_get_modulehandle(modulename);
	if( 0 == pmodule )return false;

	if( hook_is_x64() )
	{
		if( false == modulebridge.alloc( pmodule,(0x1000/sizeof(void*))-1 ) )
		{
			return false;
		}
	}else
	{
		modulebridge.setModuleHandle( pmodule );
	}
	return true;
}

bool	hookModuleFunction( CHookBridgeslot& modulebridge,CHookContext* phook_ctx,const void* function,const void* newfunction,bool disable_jmp_bridge )
{
	bool bsuccess = true;
	if( 0 == function )return false;
	do 
	{
		if( disable_jmp_bridge )
		{
			if( true == phook_ctx->install( function,newfunction ) )break;
		}else if( modulebridge.isValid() )
		{
			if( true == phook_ctx->installByBridgeslot( function,newfunction,modulebridge ) )break;
		}else
		{
			if( true == phook_ctx->install( function,newfunction ) )break;
		}

		bsuccess = false;
	} while (0);
	return bsuccess;
}



bool	hookModuleFunction2( CHookBridgeslot& modulebridge,CHookContext* phook_ctx,const char* functionname,const void* newfunction,bool disable_jmp_bridge )
{
	const void*	pfunction;
	pfunction = hook_help_get_function2( modulebridge.getModuleHandle(),functionname);
	if( 0 == pfunction )return false;
	return hookModuleFunction( modulebridge,phook_ctx,pfunction,newfunction,disable_jmp_bridge);
}