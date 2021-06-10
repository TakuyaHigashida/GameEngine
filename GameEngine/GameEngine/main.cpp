//System系ヘッダーのインクルード
#include <stdio.h>
#include <Windows.h>
#include <D3D11.h>
#include <d3dCompiler.h>
#include "DirectXTex.h"
#include "WICTextureLoader.h"

//GameSystem用ヘッダー(自作)のインクルード
#include "WindowCreate.h"
#include "DeviceCreate.h"
#include "Draw2DPolygon.h"
#include "Input.h"
#include "Audio.h"

//削除されていないメモリを出力にダンプする
#include <crtdbg.h>
#ifdef _DEBUG
	#ifndef DBG_NEW
		#define DBG_NEW new ( _NORMAL_BLOCK, __FILE__, __LINE__ )

		#define new DBG_NEW
	#endif
#endif	// _DEBUG

//LIBの登録
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dCompiler.lib")
#pragma comment(lib, "XAudio2.lib")
#pragma comment(lib, "dxguid.lib")

//構造体


//グローバル変数



//プロトタイプ宣言
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

//Main関数
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR szCmdLine, int nCmdShow)
{
	//メモリダンプ開始
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	wchar_t name[] = { L"GameEngine" };	//ウィンドウ＆タイトルネーム
	MSG msg;							//メッセージハンドル

	//ウィンドウステータス
	WNDCLASSEX wcex = {
		sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW,
		WndProc, 0, 0, hInstance, NULL, NULL,
		(HBRUSH)(COLOR_WINDOW+1), NULL, name, NULL
	};

	//ウィンドウクラス作成
	RegisterClassEx(&wcex);

	//ウィンドウ作成
	CWindowCreate::NewWindow(800, 600, name, hInstance);

	//オーディオ作成
	Audio::InitAudio();

	//ミュージック情報取得
	Audio::LoadBackMusic(L"maru.wav");
	Audio::StartLoopMusic();

	//DirectX Deviceの初期化
	CDeviceCreate::InitDevice(CWindowCreate::GethWnd(), 800, 600);

	//ポリゴン表示環境の初期化
	Draw::InitPolygonRender();
	Draw::LoadImage(0, L"heart.png");	//0番目に"heart.png"を読み込み
	Draw::LoadImage(1, L"heart1.png");
	Draw::LoadImage(2, L"heart2.png");
	Draw::LoadImage(3, L"heart3.png");

	//入力用のクラス初期化
	Input::InitInput();

	//メッセージループ
	do
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		//レンダリングターゲットセットとレンダリング画面クリア
		float color[] = { 0.0f, 0.25f, 0.45f, 1.0f };
		Dev::GetDeviceContext()->OMSetRenderTargets(1, Dev::GetppRTV(), NULL);		//レンダリング先をカラーバッファ(バックバッファ)にセット
		Dev::GetDeviceContext()->ClearRenderTargetView(Dev::GetRTV(), color);		//画面をcolorでクリア
		Dev::GetDeviceContext()->RSSetState(Dev::GetRS());						//ラスタライズをセット
		
		//ここからレンダリング開始
		static float x = 0.0f;

		//Aキーが押されたとき
		if (Input::KeyPush('A') ==true)
		{
			x += 1.0f;
		}
		//システムキー「カーソルキー↑」が押されたとき
		if (Input::KeyPush(VK_UP) == true)
		{
			x += 1.0f;
		}
		//システムキー　マウス右クリック
		if (Input::KeyPush(VK_RBUTTON) == true);
		{
			x += 1.0f;
		}

		static float time = 0.0f;
		time += 1.0f;
		Draw::Draw2D(0, x+0, 100.0f, 1.0f, 1.0f);		//テクスチャ付き四角ポリゴン描画
		Draw::Draw2D(1, 300, 300);
		Draw::Draw2D(2, Input::GetMouX(), Input::GetMouY(), 300, 100, 1.0f, 1.0f, time);
		Draw::Draw2D(3, 400, 300);

		//レンダリング終了
		Dev::GetSwapChain()->Present(1, 0);	//60fpsでバックバッファとプライマリバッファの交換

	} while (msg.message != WM_QUIT);

	Draw::DeletePolygonRender();	//ポリゴン表示環境の破棄

	CDeviceCreate::ShutDown();	//DirectXの環境破棄

	Audio::DeleteAudio();				//オーディオ環境の破棄

	CoUninitialize();

	//この時点で解放されていないメモリの情報の表示
	_CrtDumpMemoryLeaks();
	return true;
}

//コールバック関数
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Input::SetMouPos(&uMsg, &lParam);

	switch(uMsg)
	{
		case WM_KEYDOWN:	//ESCキーで終了
			switch (wParam)
			{
				case VK_ESCAPE:
					PostQuitMessage(0);
				break;
			}
			break;
			case WM_CLOSE:	//ウィンドウを閉じる場合
				PostQuitMessage(0);
			case WM_DESTROY:	//終了する場合
			return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
