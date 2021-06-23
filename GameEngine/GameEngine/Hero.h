#pragma once

//使用するヘッダー
#include "Draw2DPolygon.h"
#include "Input.h"
#include "Audio.h"
#include "TaskSystem.h"

//主人公クラス
class CHero :public CObj
{
	public:
		CHero();
		~CHero();
		void Action();
		void Draw();
	private:
		float m_x;	//位置
		float m_y;
		float m_vx;	//移動ベクトル
		float m_vy;
};
