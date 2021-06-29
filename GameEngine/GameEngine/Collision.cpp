#include "Collision.h"

list<shared_ptr<HitBox>>* CCollision::m_hit_box_list;	//リストHitBox用

//当たり判定を作成しリストに登録
HitBox* CCollision::HitBoxInsert(CObj* p)
{
	//ヒットボックス作成
	shared_ptr <HitBox> sp(new HitBox());

	//ヒットボックスに情報を入れる
	sp->m_obj = p;			//この当たり判定を持つオブジェクトのアドレス(オブジェクトアドレスは引数pから引っ張ってくる)
	sp->m_x = -999.0f;		//当たり判定のx位置
	sp->m_y = -999.0f;		//当たり判定のy位置
	sp->m_w = 64.0f;		//当たり判定の横幅
	sp->m_h = 64.0f;		//当たり判定の縦幅
	sp->m_ls_invisible = false;	//当たり判定の無敵モードOFF
	sp->m_element = 0;		//当たり判定の属性

	//リストに登録
	m_hit_box_list->push_back(sp);

	return sp.get();
}

//初期化
void CCollision::InitHitBox()
{
	m_hit_box_list = new list<shared_ptr<HitBox>>;
	m_hit_box_list->clear();
}

//破棄
void CCollision::DeleteHitBox()
{
	m_hit_box_list->clear();
	delete m_hit_box_list;
}

//list内の当たり判定全チェック開始
void CCollision::CheckStart()
{
	//リスト内のヒットボックスで当たり判定を実施
	for (auto ip_a=m_hit_box_list->begin(); ip_a!=m_hit_box_list->end(); ip_a++)
	{
		//AのHitBoxの当たり判定無視チェック
		//無敵
		if ((*ip_a)->m_ls_invisible)
			continue;

		for (int j = 0; j < 10; j++)
		{
			//当たり判定無視チェック

			//無敵

			//当たり判定を実施
			bool Is_hit = HitAB(1, 1, 1, 1, 1, 1, 1, 1);
			//衝突している場合
			if (Is_hit == true)
			{
				//当たっている情報を加える
			}
		}
	}
}

//個々の当たり判定
bool CCollision::HitAB(float ax, float ay, float aw, float ah, float bx, float by, float bw, float bh)
{
	//当たり判定
	if (1)
	{
		//衝突している
		return ture;
	}
	//衝突していない
	return false;
}