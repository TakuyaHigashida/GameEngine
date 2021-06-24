//System�n�w�b�_�[�̃C���N���[�h
#include <stdio.h>
#include <Windows.h>
#include <D3D11.h>
#include <d3dCompiler.h>
#include "DirectXTex.h"
#include "WICTextureLoader.h"

//GameSystem�p�w�b�_�[(����)�̃C���N���[�h
#include "WindowCreate.h"
#include "DeviceCreate.h"
#include "Draw2DPolygon.h"
#include "Input.h"
#include "Audio.h"
#include "TaskSystem.h"
#include "FontTex.h"

//�f�o�b�O�p�I�u�W�F�N�g�w�b�_�[
#include"Hero.h"

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
#pragma comment(lib, "XAudio2.lib")
#pragma comment(lib, "dxguid.lib")

//�\����


//�O���[�o���ϐ�



//�v���g�^�C�v�錾
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

//Main�֐�
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR szCmdLine, int nCmdShow)
{
	//�������_���v�J�n
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	wchar_t name[] = { L"GameEngine" };	//�E�B���h�E���^�C�g���l�[��
	MSG msg;							//���b�Z�[�W�n���h��

	//�E�B���h�E�X�e�[�^�X
	WNDCLASSEX wcex = {
		sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW,
		WndProc, 0, 0, hInstance, NULL, NULL,
		(HBRUSH)(COLOR_WINDOW+1), NULL, name, NULL
	};

	//�E�B���h�E�N���X�쐬
	RegisterClassEx(&wcex);

	//�e�Q�[���V�X�e��������
	CWindowCreate::NewWindow(800, 600, name, hInstance);	//�E�B���h�E�쐬
	CDeviceCreate::InitDevice(CWindowCreate::GethWnd(), 800, 600);	//DirectX Device�̏�����
	Audio::InitAudio();	//�I�[�f�B�I�쐬
	Input::InitInput();	//���͗p�̃N���X������
	Draw::InitPolygonRender();	//�|���S���\�����̏�����
	TaskSystem::InitTaskSystem();	//�^�X�N�V�X�e���̏�����
	Font::InitFontTex();			//�t�H���g�̏�����

	//���\�[�X�ǂݍ���
	
	//�~���[�W�b�N���擾
	//Audio::LoadBackMusic(L"maru.wav");
	//Audio::LoadSEMusic(0, L"GetSE.wav");
	//Audio::LoadSEMusic(1, L"maru.wav");
	Audio::LoadBackMusic("Test.ogg");
	Audio::StartLoopMusic();

	//�C���[�W�ǂݍ���
	Draw::LoadImage(0, L"heart.png");	//0�Ԗڂ�"heart.png"��ǂݍ���
	Draw::LoadImage(1, L"heart1.png");
	Draw::LoadImage(2, L"heart2.png");
	Draw::LoadImage(3, L"heart3.png");

	//�f�o�b�O�p�I�u�W�F�N�g�쐬
	CHero* hero = new CHero();
	TaskSystem::InsertObj(hero);
	hero = new CHero();
	TaskSystem::InsertObj(hero);
	hero = new CHero();
	TaskSystem::InsertObj(hero);

	//���b�Z�[�W���[�v
	do
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//���X�g���̃A�N�V�������s
		TaskSystem::ListAction();

		//�����_�����O�^�[�Q�b�g�Z�b�g�ƃ����_�����O��ʃN���A
		float color[] = { 0.0f, 0.25f, 0.45f, 1.0f };
		Dev::GetDeviceContext()->OMSetRenderTargets(1, Dev::GetppRTV(), NULL);		//�����_�����O����J���[�o�b�t�@(�o�b�N�o�b�t�@)�ɃZ�b�g
		Dev::GetDeviceContext()->ClearRenderTargetView(Dev::GetRTV(), color);		//��ʂ�color�ŃN���A
		Dev::GetDeviceContext()->RSSetState(Dev::GetRS());						//���X�^���C�Y���Z�b�g
		
		//�������烌���_�����O�J�n
		
		TaskSystem::ListAction();	//���X�g���̃A�N�V�������s
		TaskSystem::ListDraw();		//���X�g���̃h���[���s
		Font::StrDraw(L"����������", 100.0f, 100.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

		//�����_�����O�I��
		Dev::GetSwapChain()->Present(1, 0);	//60fps�Ńo�b�N�o�b�t�@�ƃv���C�}���o�b�t�@�̌���

	} while (msg.message != WM_QUIT);

	//�Q�[���V�X�e���j��
	TaskSystem::DeleteTaskSystem();	//�^�X�N�V�X�e���̔j��
	Draw::DeletePolygonRender();	//�|���S���\�����̔j��
	CDeviceCreate::ShutDown();	//DirectX�̊��j��
	Audio::DeleteAudio();				//�I�[�f�B�I���̔j��
	Font::DeleteFontTex();		//�t�H���g�̔j��

	CoUninitialize();

	//���̎��_�ŉ������Ă��Ȃ��������̏��̕\��
	_CrtDumpMemoryLeaks();
	return true;
}

//�R�[���o�b�N�֐�
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Input::SetMouPos(&uMsg, &lParam);

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
