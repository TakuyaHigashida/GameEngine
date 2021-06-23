#include"Hero.h"

CHero::CHero()
{
	//ランダムで初期配置を決める
	m_x = rand() % 300 + 100;
	m_y== rand() % 300 + 100;
	//初期移動方向
	m_vx = 1.0f;
	m_vy = 1.0f;
}

CHero::~CHero()
{

}

void CHero::Action()
{
	//削除実行
	if (Input::KeyPush('Z'))
	{
		is_delete = true;
	}

	//領域外に出ないように反射させる
	if (m_x < 0.0f) m_vx = +1.0f;
	if (m_x > 800.0f - 256.0f) m_vx = -1.0f;
	if (m_y < 0.0f) m_vy = +1.0f;
	if (m_y > 600.0f - 256.0f) m_vy = -1.0f;

	//移動方向に位置xを加える
	m_x += m_vx * 5.0f;
	m_y += m_vy * 5.0f;
}

void CHero::Draw()
{
	//描画
	Draw::Draw2D(0, m_x, m_y);
}