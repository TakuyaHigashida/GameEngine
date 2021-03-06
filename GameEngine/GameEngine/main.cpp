//System系ヘッダーのインクルード
#include <stdio.h>
#include <Windows.h>
#include <D3D11.h>
#include <d3dCompiler.h>
#include <XAudio2.h>
#include "DirectXTex.h"
#include "WICTextureLoader.h"

//GameSystem用ヘッダー(自作)のインクルード
#include "WindowCreate.h"
#include "DeviceCreate.h"
#include "Draw2DPolygon.h"
#include "Input.h"

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
//RIFFファイルフォーマットを読み取る
class ChunkInfo
{
public:
	ChunkInfo():Size(0), pData(nullptr){}
	unsigned int Size;	//チャンクデータ部サイズ
	unsigned char* pData;	//チャンクデータ部の先頭ポインタ
};

//グローバル変数
IXAudio2* g_pXAudio2;		//XAudio2オブジェクト
IXAudio2MasteringVoice* g_pMasteringVoice;	//マスターボイス
ChunkInfo g_DataChunk;	//サウンド情報
unsigned char* g_pResourceData;	//サウンドファイル情報を持つポインタ
IXAudio2SourceVoice* g_pSourceVoice;	//サウンドボイスインターフェース
IXAudio2SubmixVoice* g_pSFXSubmixVoice;	//サブミクスインターフェース


//プロトタイプ宣言
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void InitAudio();
void DeleteAudio();
WORD GetWord(const unsigned char* pData);
DWORD GetDWord(const unsigned char* pData);
ChunkInfo FindChunk(const unsigned char* pData, const char* pChunkName);

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
	InitAudio();

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

	DeleteAudio();				//オーディオ環境の破棄

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

//XAudio2によるAudio環境構築関数
void InitAudio()
{
	unsigned XAudio2CreateFlags = 0;

	//XAudio2インターフェース作成
	XAudio2Create(&g_pXAudio2, XAudio2CreateFlags);

	//マスターボイス作成
	g_pXAudio2->CreateMasteringVoice(&g_pMasteringVoice);

	//ミックスボイス作成
	g_pXAudio2->CreateSubmixVoice(&g_pSFXSubmixVoice, 1, 44100, 0, 0, 0, 0);

	//waveファイルオープン
	FILE* fp;
	const wchar_t* FILENAME = L"maru.wav";
	_wfopen_s(&fp, FILENAME, L"rb");

	//ファイルサイズを取得
	unsigned Size = 0;
	fseek(fp, 0, SEEK_END);
	Size = ftell(fp);
	g_pResourceData = new unsigned char[Size];

	//ファイルデータをメモリに移す
	fseek(fp, 0, SEEK_SET);
	fread(reinterpret_cast<char*>(g_pResourceData), Size, 1, fp);
	fclose(fp);

	//RIFFファイル解析
	WAVEFORMATEX WaveformatEx = { 0 };

	//RIFFデータの先頭アドレスとRIFFデータサイズを渡す
	ChunkInfo WaveChunk = FindChunk(g_pResourceData, "fmt");
	unsigned char* p = WaveChunk.pData;

	//wave情報取得
	WaveformatEx.wFormatTag = GetWord(p);
	p += sizeof(WORD);
	WaveformatEx.nChannels = GetWord(p);
	p += sizeof(WORD);
	WaveformatEx.nSamplesPerSec = GetDWord(p);
	p += sizeof(DWORD);
	WaveformatEx.nAvgBytesPerSec = GetDWord(p);
	p += sizeof(DWORD);
	WaveformatEx.nBlockAlign = GetWord(p);
	p += sizeof(WORD);
	WaveformatEx.wBitsPerSample = GetWord(p);
	p += sizeof(WORD);
	WaveformatEx.cbSize = GetWord(p);
	p += sizeof(WORD);

	//波形データの先頭アドレスと波形データサイズ値を渡す
	g_DataChunk = FindChunk(g_pResourceData, "data");

	//再生のためのインターフェース生成
	g_pXAudio2->CreateSourceVoice(&g_pSourceVoice, &WaveformatEx);

	//サウンドバッファをソースボイスキューに送信
	XAUDIO2_BUFFER SoundBuffer = { 0 };
	SoundBuffer.AudioBytes = g_DataChunk.Size;
	SoundBuffer.pAudioData = reinterpret_cast<BYTE*>(g_DataChunk.pData);
	SoundBuffer.LoopCount = XAUDIO2_LOOP_INFINITE;
	SoundBuffer.Flags = XAUDIO2_END_OF_STREAM;

	g_pSourceVoice->SubmitSourceBuffer(&SoundBuffer);
	g_pSourceVoice->Start();
}

//Audio環境破棄関数
void DeleteAudio()
{
	if (g_pSourceVoice != nullptr)
	{
		g_pSourceVoice->Stop();
		g_pSourceVoice->FlushSourceBuffers();
		g_pSourceVoice->DestroyVoice();
	}

	//サウンドデータ破棄
	if (g_DataChunk.pData != nullptr)
	{
		delete g_pResourceData;
	}
	//ミックスサウンド破棄
	g_pSFXSubmixVoice->DestroyVoice();

	//マスターボイス破棄
	g_pMasteringVoice->DestroyVoice();
	//XAudio2インターフェース破棄
	g_pXAudio2->Release();
}

//Word型変換関数
//引数1　const unsigned char*pData : Word型に変換する配列
//戻り値　Worldに変換した値
//指定した配列要素2つ分(2Byte)をWORD型の値として出力
WORD GetWord(const unsigned char* pData)
{
	WORD value = pData[0] | pData[1] << 8;
	return value;
}
//DWord型変換関数
//引数1 const unsigned char* pData : DWord型に変換する配列
//戻り値 DWord型に変換した値
//指定した配列要素4つ分(4Byte)をDWOrD型の値として出力
DWORD GetDWord(const unsigned char* pData)
{
	DWORD value = pData[0] | pData[1] << 8 | pData[2] << 16 | pData[3] << 24;
	return value;
}

//指定したチャンク値を見つける関数
//引数1 counst unsigned char* pData : Waveファイルデータを持った配列
//引数2 const char* pChunkName : 探すチャンクネーム
//戻り値 ChunkInfo : チャンク以下にあるファイルサイズ値とデータ部の先頭アドレスを返す
//指定したチャンクを配列から探し出して、チャンク以下に設定されているデータサイズとデータ部の先頭アドレスを返す。
//また、必ずチャンクは見つかるとして簡略化している
ChunkInfo FindChunk(const unsigned char* pData, const char* pChunkName)
{
	const unsigned CHUNKNAME_LENGTH = strlen(pChunkName);
	while (true)
	{
		bool IsFind = true;
		for (unsigned i = 0; i < CHUNKNAME_LENGTH; ++i)
		{
			if (pData[i] != pChunkName[i])
			{
				IsFind = false;
				break;
			}
		}
		if (IsFind)
		{
			ChunkInfo info;
			info.Size = pData[4 + 0] | pData[4 + 1] << 8 | pData[4 + 2] << 16 | pData[4 + 3] << 24;
			info.pData = const_cast<unsigned char*>(pData + 8);
			return info;
		}

		pData++;
	}
	return ChunkInfo();
}