//System系ヘッダーのインクルード
#include <stdio.h>
#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

//GameSystem用ヘッダー(自作)のインクルード
#include "WindowCreate.h"

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
#pragma comment(lib, "dxguid.lib")

//メモリ開放マクロ
#define SAFE_DELETE(p)			{ if (p) { delete (p);			(p)=nullptr; } }
#define SAFE_DELETE_ARRAY(p)	{ if (p) { delete[] (p);		(p)=nullptr; } }
#define SAFE_RELEASE(p)			{ if (p) { (p)->Release();		(p)=nullptr; } }

//DirectXに必要な変数
ID3D11Device*			g_pDevice;				//D3D11デバイス
ID3D11DeviceContext*	g_pDeviceContext;		//D3D11デバイスコンテキスト
ID3D11RasterizerState*	g_pRS;					//D3D11ラスタライザー
ID3D11RenderTargetView* g_pRTV;					//D3D11レンダーターゲット
ID3D11BlendState*		g_pBlendState;			//D3D11ブレンドステータス
IDXGIAdapter*			g_pDXGIAdapter;			//DXGIアダプター
IDXGIFactory*			g_pDXGIFactory;			//DXGIファクトリー
IDXGISwapChain*			g_pDXGISwapChain;		//DXGIスワップチェーン
IDXGIOutput**			g_ppDXGIOutputArray;	//DXGI出力群
UINT					g_nDXGIOutputArraySize;	//DXGI出力群サイズ
IDXGIDevice1*			g_pDXGIDevice;			//DXGIデバイス
D3D_FEATURE_LEVEL		g_FeatureLevel;			//D3D機能レベル

////グローバル変数
//HWND g_hWnd;	//ウィンドウハンドル
//int g_width;	//ウィンドウの横幅
//int g_height;	//ウィンドウの縦幅

//プロトタイプ宣言
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

//Main関数
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR szCmdLone, int nCmdShow)
{
	//メモリダンプ開始
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	wchar_t name[] = { L"GameEngine" };	//ウィンドウ＆タイトルネーム
	MSG msg;							//メッセージハンドル

	//ウィンドウステータス
	WNDCLASSEX wcex = {
		sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW,
		WndProc, 0, 0, hInstance, NULL, NULL,
		(HBRUSH)(COLOR_WINDOW +2 ), NULL, name, NULL
	};

	//ウィンドウクラス作成
	RegisterClassEx(&wcex);

	//ウィンドウ作成
	CWindowCreate::NewWindow(800, 600, name, hInstance);

	//メッセージループ
	do
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	} while (msg.message != WM_QUIT);

	//この時点で解放されていないメモリの情報の表示
	_CrtDumpMemoryLeaks();
	return true;
}

//コールバック関数
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
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

//デバイスの初期化
HRESULT APIENTRY InitDevice(HWND hWnd, int w, int h)
{
	HRESULT hr = S_OK;

	//デバイスのインターフェース
	ID3D11Device* pDevice = NULL;
	ID3D11DeviceContext* pDeviceContext = NULL;
	D3D_FEATURE_LEVEL featureLevel = (D3D_FEATURE_LEVEL)0;

	IDXGIDevice1* pDXGIDevice = NULL;
	IDXGIAdapter* pDXGIAdapter = NULL;
	IDXGIFactory* pDXGIFactory = NULL;
	IDXGISwapChain* pDXGISwapChain = NULL;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	//初期化順
	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };

	//デバイスの初期化
	hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_SOFTWARE, NULL, 0, featureLevels,
		sizeof(featureLevels) / sizeof(D3D_FEATURE_LEVEL), D3D11_SDK_VERSION,
		&pDevice, &featureLevel, &pDeviceContext);
	
	if (FAILED(hr))
	{
		//初期化に失敗した場合、ソフトウェアエミュレートを試行
		hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_SOFTWARE, NULL, 0, featureLevels,
			sizeof(featureLevels) / sizeof(D3D_FEATURE_LEVEL), D3D11_SDK_VERSION,
			&pDevice, &featureLevel, &pDeviceContext);
		if (FAILED(hr))
		{
			return hr;
		}
	}

	//デバイスからインターフェースを抽出
	hr = pDevice->QueryInterface(__uuidof(IDXGIDevice), (LPVOID*)&pDXGIDevice);
	if (FAILED(hr))
	{
		SAFE_RELEASE(pDevice);
		SAFE_RELEASE(pDeviceContext);
		return hr;
	}
	hr = pDXGIDevice->GetAdapter(&pDXGIAdapter);
	if (FAILED(hr))
	{
		SAFE_RELEASE(pDXGIDevice);
		SAFE_RELEASE(pDevice);
		SAFE_RELEASE(pDeviceContext);
		return hr;
	}
	hr = pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), (LPVOID*)&pDXGIFactory);
	if (FAILED(hr))
	{
		SAFE_RELEASE(pDXGIAdapter);
		SAFE_RELEASE(pDXGIDevice);
		SAFE_RELEASE(pDevice);
		SAFE_RELEASE(pDeviceContext);
		return hr;
	}

	//ラスタライザーの設定
	D3D11_RASTERIZER_DESC drd =
	{
		D3D11_FILL_SOLID,	//描画モード
		D3D11_CULL_NONE,	//ポリゴン描画方向D3D11_CULL_BACK
		true,				//三角形の面方向　TRUE-左回り
		0,					//ピクセル加算深度数
		0.0f,				//ピクセル最大震度バイアス
		0.0f,				//指定ピクセルのスロープに対するスカラー
		TRUE,				//距離に基づいてクリッピングするか
		FALSE,				//シザー短形カリングを有効にするか
		TRUE,				//マルチサンプリングを有効にするか
		TRUE,				//線のアンチエイリアスを有効にするか
	};
	ID3D11RasterizerState* pRS = NULL;
	hr = pDevice->CreateRasterizerState(&drd, &pRS);

	if (FAILED(hr))
	{
		SAFE_RELEASE(pDXGIFactory);
		SAFE_RELEASE(pDXGIAdapter);
		SAFE_RELEASE(pDXGIDevice);
		SAFE_RELEASE(pDevice);
		SAFE_RELEASE(pDeviceContext);
		return hr;
	}

	//画面モードを列挙
	UINT OutputCount = 0;
	for (OutputCount = 0; ; OutputCount++)
	{
		IDXGIOutput* pDXGIOutput = NULL;
		if (FAILED(pDXGIAdapter->EnumOutputs(OutputCount, &pDXGIOutput)))
			break;

		SAFE_RELEASE(pDXGIOutput);
	}
	IDXGIOutput** ppDXGIOutputArray = new IDXGIOutput * [OutputCount];
	if (ppDXGIOutputArray == NULL)
	{
		SAFE_RELEASE(pDXGIFactory);
		SAFE_RELEASE(pDXGIAdapter);
		SAFE_RELEASE(pDXGIDevice);
		SAFE_RELEASE(pDevice);
		SAFE_RELEASE(pDeviceContext);
		return E_OUTOFMEMORY;
	}
	for (UINT iOutput = 0; iOutput < OutputCount; iOutput++)
	{
		pDXGIAdapter->EnumOutputs(iOutput, ppDXGIOutputArray + iOutput);
	}
	//アウトプット配列を書き出し
	g_ppDXGIOutputArray = ppDXGIOutputArray;
	g_nDXGIOutputArraySize = OutputCount;
}