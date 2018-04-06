#include <Windows.h>
#include "..\\hooklib\hooklib.h"
#include <tchar.h>

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

int WINAPI MyMsg(HWND hWnd,LPCSTR lpText,LPCSTR lpCaption,UINT uType)
{
	OutputDebugStringA("Msg\n");
	return 0;
}

typedef int(WINAPI* _MessageBoxA)(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);

void HookMessageBoxA()
{
	_hook_context_ con ={0};
	if(hook_install(&con,hook_help_get_function("user32.dll","MessageBoxA"),MyMsg) == 0)
	{
		OutputDebugStringA("HOOK SUCCESS!\n");
		_MessageBoxA TrueMessageBoxA = (_MessageBoxA)con.proxy_function_ptr;
		TrueMessageBoxA(NULL, "111", "Tip", MB_OK);
		int i=0;
	}
	else
	{
		OutputDebugStringA("HOOK FAILED!\n");
	}
	MessageBoxA(NULL,"1111","Tip",MB_OK);
	hook_uninstall(&con);
	MessageBoxA(NULL,"2222","Tip",MB_OK);
}

int main(int agrc,char** agrv)
{
	HookMessageBoxA();
	system("pause");

	return 0;
}



