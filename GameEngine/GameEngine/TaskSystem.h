#pragma once

//必要なヘッダー
#include<memory>
#include<list>

using namespace std;


//オブジェクトクラス
class CObj
{
	public:
		CObj() 
		{
			is_delete = false;
			m_priority = 0;
		};
		virtual ~CObj() {};
		virtual void Action()=0;
		virtual void Draw()=0;

		bool is_delete;		//削除フラグ
		unsigned long m_priority;	//描画優先順位
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

		static void SotrPriority();		//リスト内のオブジェクトをプライオリティ順にソート

	private:
		//リストCObjを持つオブジェクトの要素を持つ
		static list<shared_ptr<CObj>>* m_task_list;

}TaskSystem;