#include "Collision.h"

list<shared_ptr<HitBox>>* CCollision::m_hit_box_list;	//���X�gHitBox�p

//�����蔻����쐬�����X�g�ɓo�^
HitBox* CCollision::HitBoxInsert(CObj* p)
{
	//�q�b�g�{�b�N�X�쐬
	shared_ptr <HitBox> sp(new HitBox());

	//�q�b�g�{�b�N�X�ɏ�������
	sp->m_obj = p;			//���̓����蔻������I�u�W�F�N�g�̃A�h���X(�I�u�W�F�N�g�A�h���X�͈���p������������Ă���)
	sp->m_x = -999.0f;		//�����蔻���x�ʒu
	sp->m_y = -999.0f;		//�����蔻���y�ʒu
	sp->m_w = 64.0f;		//�����蔻��̉���
	sp->m_h = 64.0f;		//�����蔻��̏c��
	sp->m_ls_invisible = false;	//�����蔻��̖��G���[�hOFF
	sp->m_element = 0;		//�����蔻��̑���

	//���X�g�ɓo�^
	m_hit_box_list->push_back(sp);

	return sp.get();
}

//������
void CCollision::InitHitBox()
{
	m_hit_box_list = new list<shared_ptr<HitBox>>;
	m_hit_box_list->clear();
}

//�j��
void CCollision::DeleteHitBox()
{
	m_hit_box_list->clear();
	delete m_hit_box_list;
}

//list���̓����蔻��S�`�F�b�N�J�n
void CCollision::CheckStart()
{
	//���X�g���̃q�b�g�{�b�N�X�œ����蔻������{
	for (auto ip_a=m_hit_box_list->begin(); ip_a!=m_hit_box_list->end(); ip_a++)
	{
		//A��HitBox�̓����蔻�薳���`�F�b�N
		//���G
		if ((*ip_a)->m_ls_invisible)
			continue;

		for (int j = 0; j < 10; j++)
		{
			//�����蔻�薳���`�F�b�N

			//���G

			//�����蔻������{
			bool Is_hit = HitAB(1, 1, 1, 1, 1, 1, 1, 1);
			//�Փ˂��Ă���ꍇ
			if (Is_hit == true)
			{
				//�������Ă������������
			}
		}
	}
}

//�X�̓����蔻��
bool CCollision::HitAB(float ax, float ay, float aw, float ah, float bx, float by, float bw, float bh)
{
	//�����蔻��
	if (1)
	{
		//�Փ˂��Ă���
		return ture;
	}
	//�Փ˂��Ă��Ȃ�
	return false;
}