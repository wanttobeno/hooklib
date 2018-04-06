#include <Windows.h>
#include <stdio.h>
#include "../hooklib/HooklibCPP.h"

#ifdef _M_IX86
#ifdef _DEBUG
#pragma comment(lib,"..\\Debug\\hooklib.lib")
#else
#pragma comment(lib,"..\\Release\\hooklib.lib")
#endif // _DEBUG
#else
#ifdef _DEBUG
#pragma comment(lib,"..\\x64\\Debug\\hooklib.lib")
#else
#pragma comment(lib,"..\\x64\\Release\\hooklib.lib")
#endif // _DEBUG
#endif //_M_IX86


int WINAPI MyMsgA(HWND hWnd,LPCSTR lpText,LPCSTR lpCaption,UINT uType)
{
	OutputDebugStringA("HOOK WIN API SUCCESS\n");
	return 0;
}

int my_printf ( const char * format, ... )
{
	OutputDebugStringA("HOOK C API SUCCESS \n");
	return 0;
}

//int WINAPI WinMain( __in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in_opt LPSTR lpCmdLine, __in int nShowCmd )
int main(int argc, char **argv)
{
	// HOOK Win API ONE
	THookContext<void,HOOK_CALL_TYPE_STDCALL> winHook;
	if(winHook.install(MessageBoxA,MyMsgA))
	{
		OutputDebugStringA("Success !\n");
	}
	else
	{
		OutputDebugStringA("Failed !\n");
	}
	MessageBoxA(NULL,"111","Tip",MB_OK);
	winHook.unInstall();
	MessageBoxA(NULL,"222","Tip",MB_OK);

	// HOOK C API
	THookContext<int,HOOK_CALL_TYPE_CDECL> cHook;
	if(cHook.install(printf,my_printf))
	{
		OutputDebugStringA("Success !\n");
	}
	else
	{
		OutputDebugStringA("Failed !\n");
	}
	printf("C sprintf 111\n");
	cHook.unInstall();
	printf("C sprintf 222\n");

	
	// HOOK Win API TWO
	THookContext<int,HOOK_CALL_TYPE_STDCALL> winHook2;
	CHookBridgeslot hookSolt;
	hookSolt.alloc(GetModuleHandleA("user32.dll"),10);
	if(winHook2.installByBridgeslot(MessageBoxA,MyMsgA,hookSolt))
	{
		OutputDebugStringA("Success !\n");
	}
	else
	{
		OutputDebugStringA("Failed !\n");
	}
	MessageBoxA(NULL,"111","Tip",MB_OK);
	winHook2.unInstall();
	hookSolt.free();
	MessageBoxA(NULL,"222","Tip",MB_OK);

	return 0;
}