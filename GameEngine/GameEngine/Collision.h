#pragma once
//�K�v�ȃw�b�_�[
#include<Windows.h>
#include<memory>
#include<list>

#include"TaskSystem.h"

using namespace std;

//�Փ˃I�u�W�F�N�g�N���X
class HitBox
{
	//CCollision�̂݃v���C�x�[�g�����J����
	friend class CCollision;
	public:
		HitBox() { Init(); }
		~HitBox(){}
		//������
		void Init()
		{
			Is_delete = false;
			m_x = m_y = -999.0f;
			m_w = m_h = 0.0f;
			m_obj = nullptr;
			m_element = -1;
			for (int i = 0; i < 10; i++)
				m_hit[i] = nullptr;
		}

		void SetDelete(bool d) { Is_delete = d; }			//�폜�t���O�Z�b�g
		void SetPos(float x, float y) { m_x = x; m_y = y; }					//�ʒu�Z�b�g
		void SetWH(float w, float h) { m_w = w; m_h = h; }					//���Z�b�g
		void SetElement(int element) { m_element = element; }				//�����Z�b�g
		void SetInvisible(bool invisible) { m_ls_invisible = invisible; }	//���G�Z�b�g
		HitBox** GetHitData() { return m_hit; }		//������������̃q�b�g�{�b�N�X���擾

	private:
		//�폜�t���O
		bool Is_delete;
		//�ʒu
		float m_x;
		float m_y;
		//��
		float m_w;
		float m_h;
		//���̏Փ˃I�u�W�F�N�g�����I�u�W�F�N�g�̃|�C���^
		CObj* m_obj;
		//����(�����l���m�ł͓����蔻����s��Ȃ�)
		int m_element;
		//���G(true�ł͓����蔻����s��Ȃ�)
		bool m_ls_invisible;
		//�������������HitBox���
		//������10�I�u�W�F�N�g�܂ŏ����Ƃ�
		HitBox* m_hit[10];
};

//�Փ˔���N���X
typedef class CCollision
{
	public:
		CCollision(){}
		~CCollision(){}

		static void InitHitBox();		//������
		static void DeleteHitBox();		//�j��

		static HitBox* HitBoxInsert(CObj* p);		//�����蔻����쐬�����X�g�ɓo�^
		static void CheckStart();					//list���̓����蔻��S�`�F�b�N�J�n
		static void DrawDebug();					//�f�o�b�O�p�̓����蔻��`��

	private:
		//�X�̓����蔻��
		static bool HitAB(float ax, float ay, float aw, float ah, float bx, float by, float bw, float bh);
		//���X�gHitBox�����I�u�W�F�N�g�̗v�f������
		static list<shared_ptr<HitBox>>* m_hit_box_list;
}Collision;