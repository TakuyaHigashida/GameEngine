#include "Audio.h"

IXAudio2*				CAudio::m_pXAudio2;			//XAudio2オブジェクト
IXAudio2MasteringVoice* CAudio::m_pMasteringVoice;	//マスターボイス
ChunkInfo				CAudio::m_DataChunk;		//サウンド情報
unsigned char*			CAudio::m_pResourceData;	//サウンドファイル情報を持つポインタ
IXAudio2SourceVoice*	CAudio::m_pSourceVoice;		//サウンドボイスインターフェース
IXAudio2SubmixVoice*	CAudio::m_pSFXSubmixVoice;	//サブミクスインターフェース

void CAudio::InitAudio()
{
	unsigned XAudio2CreateFlags = 0;

	//XAudio2インターフェース作成
	XAudio2Create(&m_pXAudio2, XAudio2CreateFlags);

	//マスターボイス作成
	m_pXAudio2->CreateMasteringVoice(&m_pMasteringVoice);

	//ミックスボイス作成
	m_pXAudio2->CreateSubmixVoice(&m_pSFXSubmixVoice, 1, 44100, 0, 0, 0, 0);

	//waveファイルオープン
	FILE* fp;
	const wchar_t* FILENAME = L"maru.wav";
	_wfopen_s(&fp, FILENAME, L"rb");

	//ファイルサイズを取得
	unsigned Size = 0;
	fseek(fp, 0, SEEK_END);
	Size = ftell(fp);
	m_pResourceData = new unsigned char[Size];

	//ファイルデータをメモリに移す
	fseek(fp, 0, SEEK_SET);
	fread(reinterpret_cast<char*>(m_pResourceData), Size, 1, fp);
	fclose(fp);

	//RIFFファイル解析
	WAVEFORMATEX WaveformatEx = { 0 };

	//RIFFデータの先頭アドレスとRIFFデータサイズを渡す
	ChunkInfo WaveChunk = FindChunk(m_pResourceData, "fmt");
	unsigned char* p = WaveChunk.pData;

	//wave情報取得
	WaveformatEx.wFormatTag = GetWord(p);
	p += sizeof(WORD);
	WaveformatEx.nChannels = GetWord(p);
	p += sizeof(WORD);
	WaveformatEx.nSamplesPerSec = GetDword(p);
	p += sizeof(DWORD);
	WaveformatEx.nAvgBytesPerSec = GetDword(p);
	p += sizeof(DWORD);
	WaveformatEx.nBlockAlign = GetWord(p);
	p += sizeof(WORD);
	WaveformatEx.wBitsPerSample = GetWord(p);
	p += sizeof(WORD);
	WaveformatEx.cbSize = GetWord(p);
	p += sizeof(WORD);

	//波形データの先頭アドレスと波形データサイズ値を渡す
	m_DataChunk = FindChunk(m_pResourceData, "data");

	//再生のためのインターフェース生成
	m_pXAudio2->CreateSourceVoice(&m_pSourceVoice, &WaveformatEx);

	//サウンドバッファをソースボイスキューに送信
	XAUDIO2_BUFFER SoundBuffer = { 0 };
	SoundBuffer.AudioBytes = m_DataChunk.Size;
	SoundBuffer.pAudioData = reinterpret_cast<BYTE*>(m_DataChunk.pData);
	SoundBuffer.LoopCount = XAUDIO2_LOOP_INFINITE;
	SoundBuffer.Flags = XAUDIO2_END_OF_STREAM;

	m_pSourceVoice->SubmitSourceBuffer(&SoundBuffer);
	m_pSourceVoice->Start();
}

void CAudio::DeleteAudio()
{
	if (m_pSourceVoice != nullptr)
	{
		m_pSourceVoice->Stop();
		m_pSourceVoice->FlushSourceBuffers();
		m_pSourceVoice->DestroyVoice();
	}

	//サウンドデータ破棄
	if (m_DataChunk.pData != nullptr)
	{
		delete m_pResourceData;
	}
	//ミックスサウンド破棄
	m_pSFXSubmixVoice->DestroyVoice();

	//マスターボイス破棄
	m_pMasteringVoice->DestroyVoice();
	//XAudio2インターフェース破棄
	m_pXAudio2->Release();
}

//Word型変換関数
//引数1　const unsigned char*pData : Word型に変換する配列
//戻り値　Worldに変換した値
//指定した配列要素2つ分(2Byte)をWORD型の値として出力
WORD CAudio::GetWord(const unsigned char* pData)
{
	WORD value = pData[0] | pData[1] << 8;
	return value;
}
//DWord型変換関数
//引数1 const unsigned char* pData : DWord型に変換する配列
//戻り値 DWord型に変換した値
//指定した配列要素4つ分(4Byte)をDWOrD型の値として出力
DWORD CAudio::GetDword(const unsigned char* pData)
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
ChunkInfo CAudio::FindChunk(const unsigned char* pData, const char* pChunkName)
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