#include <Windows.h>
#include "WindowCreate.h"

HWND CWindowCreate::m_hWnd;	//ウインドウのハンドル
int CWindowCreate::m_width;	//ウインドウの横幅
int CWindowCreate::m_height;//ウインドウの縦幅

//NewWindowメソッド
//引数1 int			:ウィンドウの横幅
//引数2 int			:ウィンドウの縦幅
//引数3 wchar_t*	:ウィンドウステータス・タイトル名
//引数4 HINSTANCE	:インスタンスハンドル
//戻り値	無し
//内容：ウィンドウを作成します。
void CWindowCreate::NewWindow(int w, int h, wchar_t* name, HINSTANCE hInstance)
{
	m_width = w;
	m_height = h;

	int width = m_width + GetSystemMetrics(SM_CXDLGFRAME) * 2;
	int height = m_height + GetSystemMetrics(SM_CXDLGFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION);

	//ウィンドウ作成
	if (!(m_hWnd = CreateWindow(name, name,
		WS_OVERLAPPEDWINDOW & ~(WS_MAXIMIZEBOX | WS_SIZEBOX),
		CW_USEDEFAULT, 0, width, height, 0, 0, hInstance, 0)))
	{
		return;
	}
	ShowWindow(m_hWnd, SW_SHOW);
}