//STLデバッグ機能をOFFにする
#define _SECURE_SCL (0)
#define _HAS_ITERATOR_DEBUGGING (0)

//System系ヘッダーのインクルード
#include <stdio.h>
#include <Windows.h>
#include <D3D11.h>
#include <d3dCompiler.h>
#include <XAudio2.h>
#include <thread>
#include <atomic>

#include "DirectXTex.h"
#include "WICTextureLoader.h"

//GameSystem用ヘッダー(自作)のインクルード
#include "WindowCreate.h"
#include "DeviceCreate.h"
#include "Draw2DPolygon.h"
#include "Input.h"
#include "Audio.h"
#include "TaskSystem.h"
#include "FontTex.h"
#include "Collision.h"

//デバッグ用オブジェクトヘッダー
#include"Hero.h"


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
atomic<bool> g_ls_game_end = false;


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

	//各ゲームシステム初期化
	CWindowCreate::NewWindow(800, 600, name, hInstance);	//ウィンドウ作成
	CDeviceCreate::InitDevice(CWindowCreate::GethWnd(), 800, 600);	//DirectX Deviceの初期化
	Audio::InitAudio();	//オーディオ作成
	Input::InitInput();	//入力用のクラス初期化
	Draw::InitPolygonRender();	//ポリゴン表示環境の初期化
	TaskSystem::InitTaskSystem();	//タスクシステムの初期化
	Font::InitFontTex();			//フォントの初期化
	Collision::InitHitBox();		//コリジョンの初期化

	//リソース読み込み
	thread* thread_i = new thread(TextureLoadThread, nullptr);		//テクスチャ読み込み
	thread* thread_m = new thread(MusicLoadThread, nullptr);		//ミュージック読み込み
	//読み込みthread終了まで待機
	thread_i->join();
	thread_m->join();
	//メモリ開放
	delete thread_i;
	delete thread_m;
	
	//ミュージック情報取得
	//Audio::LoadBackMusic(L"maru.wav");
	//Audio::LoadSEMusic(0, L"GetSE.wav");
	//Audio::LoadSEMusic(1, L"maru.wav");
	//Audio::StartLoopMusic();

	//デバッグ用オブジェクト作成
	CHero* hero = new CHero();
	hero->m_priority = 90;
	TaskSystem::InsertObj(hero);

	hero = new CHero();
	hero->m_priority = 80;
	TaskSystem::InsertObj(hero);

	hero = new CHero();
	hero->m_priority = 70;
	TaskSystem::InsertObj(hero);

	TaskSystem::SotrPriority();	//描画順位変更

	//メッセージループ
	thread* thread_main = new thread(GameMainThread, nullptr);		//ゲームメインスレッド開始
	do
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	} while (msg.message != WM_QUIT);
	thread_main->join();		//ゲームメインスレッド終了待ち
	delete thread_main;


	//ゲームシステム破棄
	Collision::DeleteHitBox();		//コリジョンの破棄
	TaskSystem::DeleteTaskSystem();	//タスクシステムの破棄
	Draw::DeletePolygonRender();	//ポリゴン表示環境の破棄
	CDeviceCreate::ShutDown();	//DirectXの環境破棄
	Audio::DeleteAudio();				//オーディオ環境の破棄
	Font::DeleteFontTex();		//フォントの破棄


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
					g_ls_game_end = true;
					PostQuitMessage(0);
				break;
			}
			break;
			case WM_CLOSE:	//ウィンドウを閉じる場合
				PostQuitMessage(0);
				g_ls_game_end = true;
			case WM_DESTROY:	//終了する場合
			return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


//マルチスレッドにする関数----------
//テクスチャ読み込み関数
unsigned __stdcall TextureLoadThread(void* p)
{
	//COM初期化
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	//イメージ読み込み
	CDraw2DPolygon::LoadImage(0, L"heart.png");
	CDraw2DPolygon::LoadImage(1, L"heart1.png");
	CDraw2DPolygon::LoadImage(2, L"heart2.png");
	CDraw2DPolygon::LoadImage(3, L"heart3.png");

	CoUninitialize();		//COM解除
	return 0;
}

//ミュージック読み込み関数
unsigned __stdcall MusicLoadThread(void* p)
{
	//COM初期化
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	//ミュージック取得情報
	CAudio::LoadBackMusic("tesst.ogg");

	CoUninitialize();		//COM解除
	return 0;
}

//ゲームメイン関数
unsigned __stdcall GameMainThread(void* p)
{
	//COM初期化
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	while (1)
	{
		//ゲームメイン
		//リスト内のアクション実行
		TaskSystem::ListAction();
		//衝突判定実行
		Collision::CheckStart();

		//レンダリングターゲットセットとレンダリング画面クリア
		float color[] = { 0.0f, 0.25f, 0.45f, 1.0f };
		Dev::GetDeviceContext()->OMSetRenderTargets(1, Dev::GetppRTV(), NULL);		//レンダリング先をカラーバッファ(バックバッファ)にセット
		Dev::GetDeviceContext()->ClearRenderTargetView(Dev::GetRTV(), color);		//画面をcolorでクリア
		Dev::GetDeviceContext()->RSSetState(Dev::GetRS());						//ラスタライズをセット

		//ここからレンダリング開始

		TaskSystem::ListAction();	//リスト内のアクション実行
		TaskSystem::ListDraw();		//リスト内のドロー実行

		//レンダリング終了
		Dev::GetSwapChain()->Present(1, 0);	//60fpsでバックバッファとプライマリバッファの交換

		if (g_ls_game_end==true)
		{
			break;
		}
	}

	CoUninitialize();		//COM解除
	return 0;
}