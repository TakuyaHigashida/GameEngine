#pragma once
#include <Windows.h>

class CWindowCreate
{
public:
	CWindowCreate() {}
	~CWindowCreate() {}

	static void NewWindow(int w, int h, wchar_t* name, HINSTANCE hInstance);

private:
	static HWND m_hWnd;		//ウィンドウのハンドル
	static int m_width;		//ウィンドウの横幅
	static int m_height;	//ウィンドウの縦幅
};
