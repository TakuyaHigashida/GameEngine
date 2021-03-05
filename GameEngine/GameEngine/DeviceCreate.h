#pragma once
//Device作成に必要なヘッダー
#include <Windows.h>
#include <D3D11.h>

#include<d3dCompiler.h>


class CDeviceCreate
{
	public:
		CDeviceCreate(){}
		~CDeviceCreate(){}

		static HRESULT APIENTRY InitDevice(HWND hWnd, int w, int h);
		static void ShutDown();	//終了関数

	private:
		static	ID3D11Device*			m_pDevice;				//D3D11デバイス
		static	ID3D11DeviceContext*	m_pDeviceContext;		//D3D11デバイスコンテキスト
		static	ID3D11RasterizerState*	m_pRS;					//D3D11ラスタライザー
		static	ID3D11RenderTargetView* m_pRTV;					//D3D11レンダーターゲット
		static	ID3D11BlendState*		m_pBlendState;			//D3D11ブレンドステータス
		static	IDXGIAdapter*			m_pDXGIAdapter;			//DXGIアダプター
		static	IDXGIFactory*			m_pDXGIFactory;			//DXGIファクトリー
		static	IDXGISwapChain*			m_pDXGISwapChain;		//DXGIスワップチェーン
		static	IDXGIOutput**			m_ppDXGIOutputArray;	//DXGI出力群
		static	UINT					m_nDXGIOutputArraySize;	//DXGI出力群サイズ
		static	IDXGIDevice1*			m_pDXGIDevice;			//DXGIデバイス
		static	D3D_FEATURE_LEVEL		m_FeatureLevel;			//D3D機能レベル
};
