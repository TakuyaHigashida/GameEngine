#pragma once

//必要なヘッダー
#include<memory>
#include<list>

using namespace std;


//オブジェクトクラス
class CObj
{
	public:
		CObj() {};
		virtual ~CObj() {};
		virtual void Action()=0;
		virtual void Draw()=0;
};

//タスクシステム
typedef class CTaskSystem
{
	public:
		CTaskSystem() {}
		~CTaskSystem() {}

		static void InsertObj(CObj* obj);	//追加
		static void ListAction();			//リスト内のアクション実行
		static void ListDraw();				//リスト内のドロー実行

		static void InitTaskSystem();	//初期化
		static void DeleteTaskSystem();	//破棄

	private:
		//リストCObjを持つオブジェクトの要素を持つ
		static list<shared_ptr<CObj>>* m_task_list;

}TaskSystem;