
template<typename ret_type,HOOK_CALL_TYPE call_type>
ret_type THookContext<ret_type,call_type>::emit()
{
	const void*	function = getProxyFunction();

	if(function)
		switch( call_type )
	{
		case  HOOK_CALL_TYPE_CDECL:
			return CAST_FUNCTION_POINTER( ret_type,__cdecl,function )();
		case  HOOK_CALL_TYPE_STDCALL:
			return CAST_FUNCTION_POINTER( ret_type,__stdcall,function )();
		case  HOOK_CALL_TYPE_PASCALL:
			return CAST_FUNCTION_POINTER( ret_type,pascal,function )();
	}
	return 0;
}

template<typename ret_type,HOOK_CALL_TYPE call_type>
ret_type THookContext<ret_type,call_type>::operator()()
{
	return emit();
}


template<HOOK_CALL_TYPE call_type>
void THookContext<void,call_type>::emit()
{
	const void*	function = getProxyFunction();

	if(function)
	switch( call_type )
	{
		case  HOOK_CALL_TYPE_CDECL:
			CAST_FUNCTION_POINTER( void,__cdecl,function )();
		case  HOOK_CALL_TYPE_STDCALL:
			CAST_FUNCTION_POINTER( void,__stdcall,function )();
		case  HOOK_CALL_TYPE_PASCALL:
			CAST_FUNCTION_POINTER( void,pascal,function )();
	}
	return ;
}
/*
template<typename ret_type,HOOK_CALL_TYPE call_type>
template< typename _argv0 >
ret_type THookContext<ret_type,call_type>::emit( _argv0 argv0 )
{
	const void*	function = getProxyFunction();

	if(function)
		switch( call_type )
	{
		case  HOOK_CALL_TYPE_CDECL:
			return CAST_FUNCTION_POINTER( ret_type,__cdecl,function,_argv0 )(argv0);
		case  HOOK_CALL_TYPE_STDCALL:
			return CAST_FUNCTION_POINTER( ret_type,__stdcall,function,_argv0 )(argv0);
		case  HOOK_CALL_TYPE_PASCALL:
			return CAST_FUNCTION_POINTER( ret_type,pascal,function,_argv0 )(argv0);
	}
	return 0;
}


template<typename ret_type,HOOK_CALL_TYPE call_type>
template< typename _argv0 >
ret_type THookContext<ret_type,call_type>::operator()( _argv0 argv0 )
{
	return emit(argv0);
}
*/