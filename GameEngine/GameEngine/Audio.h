#pragma once

#include <stdio.h>
#include <Windows.h>
#include <XAudio2.h>

//RIFF�t�@�C���t�H�[�}�b�g��ǂݎ��
class ChunkInfo
{
public:
	ChunkInfo() :Size(0), pData(nullptr) {}
	unsigned int Size;	//�`�����N�f�[�^���T�C�Y
	unsigned char* pData;	//�`�����N�f�[�^���̐擪�|�C���^
};

typedef class CAudio
{
	public:
		CAudio(){}
		~CAudio(){}

		static void InitAudio();		//������
		static void DeleteAudio();		//�j��

	private:
		static WORD GetWord(const unsigned char* pData);
		static DWORD GetDword(const unsigned char* pData);
		static ChunkInfo FindChunk(const unsigned char* pData, const char* pChunckName);
		static IXAudio2*				 m_pXAudio2;		//XAudio2�I�u�W�F�N�g
		static IXAudio2MasteringVoice*	 m_pMasteringVoice;	//�}�X�^�[�{�C�X
		static ChunkInfo				 m_DataChunk;		//�T�E���h���
		static unsigned char*			 m_pResourceData;	//�T�E���h�t�@�C���������|�C���^
		static IXAudio2SourceVoice*		 m_pSourceVoice;	//�T�E���h�{�C�X�C���^�[�t�F�[�X
		static IXAudio2SubmixVoice*		 m_pSFXSubmixVoice;	//�T�u�~�N�X�C���^�[�t�F�[�X

}Audio;
