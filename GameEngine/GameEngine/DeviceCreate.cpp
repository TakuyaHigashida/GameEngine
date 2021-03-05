#include "DeviceCreate.h"

//メモリ開放マクロ
#define SAFE_DELETE(p)			{ if (p) { delete (p);			(p)=nullptr; } }
#define SAFE_DELETE_ARRAY(p)	{ if (p) { delete[] (p);		(p)=nullptr; } }
#define SAFE_RELEASE(p)			{ if (p) { (p)->Release();		(p)=nullptr; } }

//classのMember変数
ID3D11Device*			CDeviceCreate::m_pDevice;				//D3D11デバイス
ID3D11DeviceContext*	CDeviceCreate::m_pDeviceContext;		//D3D11デバイスコンテキスト
ID3D11RasterizerState*	CDeviceCreate::m_pRS;					//D3D11ラスタライザー
ID3D11RenderTargetView* CDeviceCreate::m_pRTV;					//D3D11レンダーターゲット
ID3D11BlendState*		CDeviceCreate::m_pBlendState;			//D3D11ブレンドステータス
IDXGIAdapter*			CDeviceCreate::m_pDXGIAdapter;			//DXGIアダプター
IDXGIFactory*			CDeviceCreate::m_pDXGIFactory;			//DXGIファクトリー
IDXGISwapChain*			CDeviceCreate::m_pDXGISwapChain;		//DXGIスワップチェーン
IDXGIOutput**			CDeviceCreate::m_ppDXGIOutputArray;		//DXGI出力群
UINT					CDeviceCreate::m_nDXGIOutputArraySize;	//DXGI出力群サイズ
IDXGIDevice1*			CDeviceCreate::m_pDXGIDevice;			//DXGIデバイス
D3D_FEATURE_LEVEL		CDeviceCreate::m_FeatureLevel;			//D3D機能レベル


//デバイスの初期化
HRESULT APIENTRY CDeviceCreate::InitDevice(HWND hWnd, int w, int h)
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
	m_ppDXGIOutputArray = ppDXGIOutputArray;
	m_nDXGIOutputArraySize = OutputCount;

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
	m_pDevice = pDevice;
	m_pDeviceContext = pDeviceContext;
	m_pDXGIAdapter = pDXGIAdapter;
	m_pDXGIFactory = pDXGIFactory;
	m_pDXGISwapChain = pDXGISwapChain;
	m_FeatureLevel = featureLevel;
	m_pDXGIDevice = pDXGIDevice;

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
	m_pDevice->CreateBlendState(&BlendDesc, &m_pBlendState);

	//ブレンディング
	m_pDeviceContext->OMSetBlendState(m_pBlendState, NULL, 0xFFFFFFFF);

	//ステータス、ビューなどを書き出し
	m_pRS = pRS;
	m_pRTV = pRTV;
	return hr;
}



//終了関数
void CDeviceCreate::ShutDown()
{
	SAFE_RELEASE(m_pBlendState);	//ブレンドステータス

	SAFE_RELEASE(m_pRTV);			//レンダリングターゲットを解放

	//スワップチェーンを解放
	if (m_pDXGISwapChain != NULL)
	{
		m_pDXGISwapChain->SetFullscreenState(FALSE, 0);
	}
	SAFE_RELEASE(m_pDXGISwapChain);

	//アウトプットを解放
	for (UINT i = 0; i < m_nDXGIOutputArraySize; i++)
	{
		SAFE_RELEASE(m_ppDXGIOutputArray[i]);
	}
	SAFE_DELETE_ARRAY(m_ppDXGIOutputArray);

	SAFE_RELEASE(m_pRS);				//2D用ラスタライザー
	SAFE_RELEASE(m_pDXGIFactory);		//ファクトリーの開放
	SAFE_RELEASE(m_pDXGIAdapter);		//アダプターの開放
	SAFE_RELEASE(m_pDXGIDevice);		//DXGIデバイスの開放
	SAFE_RELEASE(m_pDeviceContext);		//D3D11デバイスコンテキストを解放
	SAFE_RELEASE(m_pDevice);			//D3D11デバイスの開放
}