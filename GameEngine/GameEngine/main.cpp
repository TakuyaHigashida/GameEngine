//System�n�w�b�_�[�̃C���N���[�h
#include <stdio.h>
#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

//GameSystem�p�w�b�_�[(����)�̃C���N���[�h
#include "WindowCreate.h"

//�폜����Ă��Ȃ����������o�͂Ƀ_���v����
#include <crtdbg.h>
#ifdef _DEBUG
	#ifndef DBG_NEW
		#define DBG_NEW new ( _NORMAL_BLOCK, __FILE__, __LINE__ )

		#define new DBG_NEW
	#endif
#endif	// _DEBUG

//LIB�̓o�^
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dCompiler.lib")
#pragma comment(lib, "dxguid.lib")

//�������J���}�N��
#define SAFE_DELETE(p)			{ if (p) { delete (p);			(p)=nullptr; } }
#define SAFE_DELETE_ARRAY(p)	{ if (p) { delete[] (p);		(p)=nullptr; } }
#define SAFE_RELEASE(p)			{ if (p) { (p)->Release();		(p)=nullptr; } }

//DirectX�ɕK�v�ȕϐ�
ID3D11Device*			g_pDevice;				//D3D11�f�o�C�X
ID3D11DeviceContext*	g_pDeviceContext;		//D3D11�f�o�C�X�R���e�L�X�g
ID3D11RasterizerState*	g_pRS;					//D3D11���X�^���C�U�[
ID3D11RenderTargetView* g_pRTV;					//D3D11�����_�[�^�[�Q�b�g
ID3D11BlendState*		g_pBlendState;			//D3D11�u�����h�X�e�[�^�X
IDXGIAdapter*			g_pDXGIAdapter;			//DXGI�A�_�v�^�[
IDXGIFactory*			g_pDXGIFactory;			//DXGI�t�@�N�g���[
IDXGISwapChain*			g_pDXGISwapChain;		//DXGI�X���b�v�`�F�[��
IDXGIOutput**			g_ppDXGIOutputArray;	//DXGI�o�͌Q
UINT					g_nDXGIOutputArraySize;	//DXGI�o�͌Q�T�C�Y
IDXGIDevice1*			g_pDXGIDevice;			//DXGI�f�o�C�X
D3D_FEATURE_LEVEL		g_FeatureLevel;			//D3D�@�\���x��

////�O���[�o���ϐ�
//HWND g_hWnd;	//�E�B���h�E�n���h��
//int g_width;	//�E�B���h�E�̉���
//int g_height;	//�E�B���h�E�̏c��

//�v���g�^�C�v�錾
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

//Main�֐�
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR szCmdLone, int nCmdShow)
{
	//�������_���v�J�n
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	wchar_t name[] = { L"GameEngine" };	//�E�B���h�E���^�C�g���l�[��
	MSG msg;							//���b�Z�[�W�n���h��

	//�E�B���h�E�X�e�[�^�X
	WNDCLASSEX wcex = {
		sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW,
		WndProc, 0, 0, hInstance, NULL, NULL,
		(HBRUSH)(COLOR_WINDOW +2 ), NULL, name, NULL
	};

	//�E�B���h�E�N���X�쐬
	RegisterClassEx(&wcex);

	//�E�B���h�E�쐬
	CWindowCreate::NewWindow(800, 600, name, hInstance);

	//���b�Z�[�W���[�v
	do
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	} while (msg.message != WM_QUIT);

	//���̎��_�ŉ������Ă��Ȃ��������̏��̕\��
	_CrtDumpMemoryLeaks();
	return true;
}

//�R�[���o�b�N�֐�
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_KEYDOWN:	//ESC�L�[�ŏI��
			switch (wParam)
			{
				case VK_ESCAPE:
					PostQuitMessage(0);
				break;
			}
			break;
			case WM_CLOSE:	//�E�B���h�E�����ꍇ
				PostQuitMessage(0);
			case WM_DESTROY:	//�I������ꍇ
			return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//�f�o�C�X�̏�����
HRESULT APIENTRY InitDevice(HWND hWnd, int w, int h)
{
	HRESULT hr = S_OK;

	//�f�o�C�X�̃C���^�[�t�F�[�X
	ID3D11Device* pDevice = NULL;
	ID3D11DeviceContext* pDeviceContext = NULL;
	D3D_FEATURE_LEVEL featureLevel = (D3D_FEATURE_LEVEL)0;

	IDXGIDevice1* pDXGIDevice = NULL;
	IDXGIAdapter* pDXGIAdapter = NULL;
	IDXGIFactory* pDXGIFactory = NULL;
	IDXGISwapChain* pDXGISwapChain = NULL;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	//��������
	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };

	//�f�o�C�X�̏�����
	hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_SOFTWARE, NULL, 0, featureLevels,
		sizeof(featureLevels) / sizeof(D3D_FEATURE_LEVEL), D3D11_SDK_VERSION,
		&pDevice, &featureLevel, &pDeviceContext);
	
	if (FAILED(hr))
	{
		//�������Ɏ��s�����ꍇ�A�\�t�g�E�F�A�G�~�����[�g�����s
		hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_SOFTWARE, NULL, 0, featureLevels,
			sizeof(featureLevels) / sizeof(D3D_FEATURE_LEVEL), D3D11_SDK_VERSION,
			&pDevice, &featureLevel, &pDeviceContext);
		if (FAILED(hr))
		{
			return hr;
		}
	}

	//�f�o�C�X����C���^�[�t�F�[�X�𒊏o
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

	//���X�^���C�U�[�̐ݒ�
	D3D11_RASTERIZER_DESC drd =
	{
		D3D11_FILL_SOLID,	//�`�惂�[�h
		D3D11_CULL_NONE,	//�|���S���`�����D3D11_CULL_BACK
		true,				//�O�p�`�̖ʕ����@TRUE-�����
		0,					//�s�N�Z�����Z�[�x��
		0.0f,				//�s�N�Z���ő�k�x�o�C�A�X
		0.0f,				//�w��s�N�Z���̃X���[�v�ɑ΂���X�J���[
		TRUE,				//�����Ɋ�Â��ăN���b�s���O���邩
		FALSE,				//�V�U�[�Z�`�J�����O��L���ɂ��邩
		TRUE,				//�}���`�T���v�����O��L���ɂ��邩
		TRUE,				//���̃A���`�G�C���A�X��L���ɂ��邩
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

	//��ʃ��[�h���
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
	//�A�E�g�v�b�g�z��������o��
	g_ppDXGIOutputArray = ppDXGIOutputArray;
	g_nDXGIOutputArraySize = OutputCount;
}