#pragma once
//入力に必要なヘッダー
#include <Windows.h>


typedef class CInput
{
	public:
		CInput(){}
		~CInput(){}

		//初期化
		static void InitInput();

		//マウス位置取得用　ウィンドウプロージャー内に宣言
		static void SetMouPos(UINT* uMsg, LPARAM* lParam);

		//キー・マウスボタンのプッシュ確認
		static bool KeyPush(int key);

		//マウスの位置情報取得
		static int GetMouX() { return m_mou_x; }
		static int GetMouY() { return m_mou_y; }

	private:
		static int m_mou_x;		//マウスの位置X
		static int m_mou_y;		//マウスの位置Y

}Input;