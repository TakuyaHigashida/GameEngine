#pragma once

#include <stdio.h>
#include <Windows.h>
#include <XAudio2.h>

//RIFFファイルフォーマットを読み取る
class ChunkInfo
{
public:
	ChunkInfo() :Size(0), pData(nullptr) {}
	unsigned int Size;	//チャンクデータ部サイズ
	unsigned char* pData;	//チャンクデータ部の先頭ポインタ
};

typedef class CAudio
{
	public:
		CAudio(){}
		~CAudio(){}

		static void InitAudio();		//初期化
		static void DeleteAudio();		//破棄

	private:
		static WORD GetWord(const unsigned char* pData);
		static DWORD GetDword(const unsigned char* pData);
		static ChunkInfo FindChunk(const unsigned char* pData, const char* pChunckName);
		static IXAudio2*				 m_pXAudio2;		//XAudio2オブジェクト
		static IXAudio2MasteringVoice*	 m_pMasteringVoice;	//マスターボイス
		static ChunkInfo				 m_DataChunk;		//サウンド情報
		static unsigned char*			 m_pResourceData;	//サウンドファイル情報を持つポインタ
		static IXAudio2SourceVoice*		 m_pSourceVoice;	//サウンドボイスインターフェース
		static IXAudio2SubmixVoice*		 m_pSFXSubmixVoice;	//サブミクスインターフェース

}Audio;
