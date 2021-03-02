//System系ヘッダーのインクルード
#include <stdio.h>
#include <Windows.h>
#include <D3D11.h>
#include <d3dCompiler.h>

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

//ポリゴン表示で必要な変数---
//GPU側で扱う用
ID3D11VertexShader* g_pVertexShader;		//パーテックスシェーダー
ID3D11PixelShader* g_pPixelShader;			//ピクセルシェーダー
ID3D11InputLayout* g_pVertexLayout;			//頂点入力レイアウト
ID3D11Buffer* g_pConstantBufferf;			//コンスタントバッファ
//ポリゴン情報登録用バッファ
ID3D11Buffer* g_pVertexBuffer;				//バーティクスバッファ
ID3D11Buffer* g_pIndexBuffer;				//インデックスバッファ

//構造体---------------------
//頂点レイアウト構造体(頂点が持つ情報)
struct POINT_LAYOUT
{
	float pos[3];		//X-Y-Z	:頂点
	float color[4];		//R-G-B-A:色
};

//コンスタントバッファ構造体
struct POLYGON_BUFFER
{
	float color[4];		//R-G-B-A:ポリゴンカラー
};



//プロトタイプ宣言
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HRESULT APIENTRY InitDevice(HWND hWnd, int w, int h);
void ShutDown();	//終了関数
HRESULT InitPolygonRender();	//ポリゴン表示環境の初期化
void DeletePolygonRender();		//ポリゴン表示環境の破棄

//Main関数
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR szCmdLine, int nCmdShow)
{
	//メモリダンプ開始
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

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

	//DirectXデバイスの作成
	InitDevice(CWindowCreate::GethWnd(), 800, 600);

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
		g_pDeviceContext->OMSetRenderTargets(1, &g_pRTV, NULL);		//レンダリング先をカラーバッファ(バックバッファ)にセット
		g_pDeviceContext->ClearRenderTargetView(g_pRTV, color);		//画面をcolorでクリア
		g_pDeviceContext->RSSetState(g_pRS);						//ラスタライズをセット
		//ここからレンダリング開始


		//レンダリング終了
		g_pDXGISwapChain->Present(1, 0);		//60fpsでバックバッファとプライマリバッファの交換

	} while (msg.message != WM_QUIT);

	ShutDown();	//DirectXデバイスの削除

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

//ポリゴン表示環境の初期化
HRESULT InitPolygonRender()
{
	HRESULT hr = S_OK;

	//hlslファイル名
	wchar_t* hlsl_name = L"PolygonDraw.hlsl";

	//hlslファイルを読み込み、ブロブ作成　ブロブとはシェーダーの塊みたいなもの
	//xxシェーダーとして特徴をもたない。後で各種シェーダーとなる
	ID3DBlob* pCompiledShader = NULL;
	ID3DBlob* pErrors = NULL;

	//ブロブからバーテックスシェーダーコンパイル
	hr = D3DCompileFromFile(hlsl_name, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"vs", "vs_4_0", 0, NULL, &pCompiledShader, &pErrors);
	if (FAILED(hr))
	{
		//エラーがある場合、cがデバック情報を持つ
		char* c = (char*)pErrors->GetBufferPointer();
		MessageBox(0, L"hlsl読み込み失敗1", NULL, MB_OK);
		SAFE_RELEASE(pErrors);
		return hr;
	}
	//コンパイルしたバーテックスシェーダーを元にインターフェースを作成
	hr = g_pDevice->CreateVertexShader(pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(),
		NULL, &g_pVertexShader);
	if (FAILED(hr))
	{
		SAFE_RELEASE(pCompiledShader);
		MessageBox(0, L"バーテックスシェーダー作成失敗", NULL, MB_OK);
		return hr;
	}
	//頂点インプットレイアウトを定義
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	UINT numElements = sizeof(layout) / sizeof(layout[0]);

	//頂点インプットレイアウトを作成・レイアウトをセット
	hr = g_pDevice->CreateInputLayout(layout, numElements, pCompiledShader->GetBufferPointer(),
		pCompiledShader->GetBufferSize(), &g_pVertexLayout);
	if (FAILED(hr))
	{
		MessageBox(0, L"レイアウト作成失敗", NULL, MB_OK);
		return hr;
	}
	SAFE_RELEASE(pCompiledShader);

	//ブロブからピクセルシェーダーコンパイル
	hr = D3DCompileFromFile(hlsl_name, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"ps", "ps_4_0", 0, NULL, &pCompiledShader, &pErrors);
	if (FAILED(hr))
	{
		//エラーがある場合、cがデバック情報を持つ
		char* c = (char*)pErrors->GetBufferPointer();
		MessageBox(0, L"hlsl読み込み失敗2", NULL, MB_OK);
		SAFE_RELEASE(pErrors);
		return hr;
	}

	//コンパイルしたピクセルシェーダでインターフェースを作成
	hr = g_pDevice->CreatePixelShader(pCompiledShader->GetBufferPointer(),
		pCompiledShader->GetBufferSize(), NULL, &g_pPixelShader);
	if (FAILED(hr))
	{
		SAFE_RELEASE(pCompiledShader);
		MessageBox(0, L"ピクセルシェーダー作成失敗", NULL, MB_OK);
		return hr;
	}
	SAFE_RELEASE(pCompiledShader);
	
	//三角ポリゴンの各頂点の情報
	POINT_LAYOUT vertices[] =
	{
		// x	 y		z		r	 g		b	  a
		{{0.0f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.5f, 1.0f},},	//頂点1
		{{0.5f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.5f, 1.0f},},	//頂点2
		{{0.5f, 0.5f, 0.0f}, {0.5f, 0.5f, 0.5f, 1.0f},},	//頂点3
	};

	//バッファにバックステータス設定
	D3D11_BUFFER_DESC bd;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(POINT_LAYOUT) * 3;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	//バッファに入れるデータを設定
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = vertices;

	//ステータスとバッファに入れるデータをもとにバーテックスバッファ作成
	hr = g_pDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
	if (FAILED(hr))
	{
		MessageBox(0, L"バーテックスバッファ作成失敗", NULL, MB_OK);
		return hr;
	}

	//ポリゴンのインデックス情報
	unsigned short hIndexData[3] =
	{
		0, 1, 2,
	};

	//バッファにインデックスステータス設定
	D3D11_BUFFER_DESC hBufferDesc;
	hBufferDesc.ByteWidth = sizeof(hIndexData);
	hBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	hBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	hBufferDesc.CPUAccessFlags = 0;
	hBufferDesc.MiscFlags = 0;
	hBufferDesc.StructureByteStride = sizeof(unsigned short);

	//バッファに入れるデータを設定
	D3D11_SUBRESOURCE_DATA hSubResourceData;
	hSubResourceData.pSysMem = hIndexData;
	hSubResourceData.SysMemPitch = 0;
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
	hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, featureLevels,
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

	//スワップチェーンの初期化と生成
	swapChainDesc.BufferDesc.Width = w;
	swapChainDesc.BufferDesc.Height = h;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	hr = pDXGIFactory->CreateSwapChain(pDevice, &swapChainDesc, &pDXGISwapChain);
	if (FAILED(hr))
	{
		SAFE_RELEASE(pDXGIFactory);
		SAFE_RELEASE(pDXGIAdapter);
		SAFE_RELEASE(pDXGIDevice);
		SAFE_RELEASE(pDevice);
		SAFE_RELEASE(pDeviceContext);
		return hr;
	}

	//D3D11インターフェースの書き出し
	g_pDevice = pDevice;
	g_pDeviceContext = pDeviceContext;
	g_pDXGIAdapter = pDXGIAdapter;
	g_pDXGIFactory = pDXGIFactory;
	g_pDXGISwapChain = pDXGISwapChain;
	g_FeatureLevel = featureLevel;
	g_pDXGIDevice = pDXGIDevice;

	//レンダリングターゲットを生成
	ID3D11RenderTargetView* pRTV = NULL;

	//バックバッファを取得
	ID3D11Texture2D* pBackBuffer = NULL;
	hr = pDXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (FAILED(hr))
	{
		SAFE_RELEASE(pDXGISwapChain);
		SAFE_RELEASE(pDXGIFactory);
		SAFE_RELEASE(pDXGIAdapter);
		SAFE_RELEASE(pDevice);
		SAFE_RELEASE(pDeviceContext);
		return hr;
	}
	D3D11_TEXTURE2D_DESC BackBufferSurfaceDesc;
	pBackBuffer->GetDesc(&BackBufferSurfaceDesc);

	//レンダリングターゲットを生成
	hr = pDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRTV);
	//バックバッファ開放
	SAFE_RELEASE(pBackBuffer);
	if (FAILED(hr))
	{
		SAFE_RELEASE(pDXGISwapChain);
		SAFE_RELEASE(pDXGIFactory);
		SAFE_RELEASE(pDXGIAdapter);
		SAFE_RELEASE(pDevice);
		SAFE_RELEASE(pDeviceContext);
		return hr;
	}

	//ビューポートの設定
	D3D11_VIEWPORT vp;
	vp.Width = (float)w;
	vp.Height = (float)h;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	pDeviceContext->RSSetViewports(1, &vp);

	//ブレンドステートを設定
	D3D11_BLEND_DESC BlendDesc = { FALSE, FALSE };
	for (int i = 0; i < 8; i++)
	{
		BlendDesc.RenderTarget[i].BlendEnable = TRUE;
		BlendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		BlendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		BlendDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
		BlendDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
		BlendDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
		BlendDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		BlendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}
	g_pDevice->CreateBlendState(&BlendDesc, &g_pBlendState);

	//ブレンディング
	g_pDeviceContext->OMSetBlendState(g_pBlendState, NULL, 0xFFFFFFFF);

	//ステータス、ビューなどを書き出し
	g_pRS = pRS;
	g_pRTV = pRTV;
	return hr;
}



//終了関数
void ShutDown()
{
	SAFE_RELEASE(g_pBlendState);	//ブレンドステータス

	SAFE_RELEASE(g_pRTV);			//レンダリングターゲットを解放

	//スワップチェーンを解放
	if (g_pDXGISwapChain != NULL)
	{
		g_pDXGISwapChain->SetFullscreenState(FALSE, 0);
	}
	SAFE_RELEASE(g_pDXGISwapChain);

	//アウトプットを解放
	for (UINT i = 0; i < g_nDXGIOutputArraySize; i++)
	{
		SAFE_RELEASE(g_ppDXGIOutputArray[i]);
	}
	SAFE_DELETE_ARRAY(g_ppDXGIOutputArray);

	SAFE_RELEASE(g_pRS);				//2D用ラスタライザー
	SAFE_RELEASE(g_pDXGIFactory);		//ファクトリーの開放
	SAFE_RELEASE(g_pDXGIAdapter);		//アダプターの開放
	SAFE_RELEASE(g_pDXGIDevice);		//DXGIデバイスの開放
	SAFE_RELEASE(g_pDeviceContext);		//D3D11デバイスコンテキストを解放
	SAFE_RELEASE(g_pDevice);			//D3D11デバイスの開放
}
