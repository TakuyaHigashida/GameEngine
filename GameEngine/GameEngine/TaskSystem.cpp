//STLデバッグ機能をOFFにする
#define _SECURE_SCL (0)
#define _HAS_ITERATOR_DEBUGGING (0)

#include "TaskSystem.h"

//リスト　CObjを持つオブジェクト
list< shared_ptr< CObj	>	>* CTaskSystem::m_task_list;

void CTaskSystem::InitTaskSystem()
{
	//リスト作成
	m_task_list = new list<shared_ptr<CObj>>();
}

void CTaskSystem::DeleteTaskSystem()
{
	//リスト破棄
	delete m_task_list;
}

//リスト内のオブジェクトをプライオリティ順にソート
void CTaskSystem::SotrPriority()
{
	//ソート比較クラス
	class Pr
	{
		public:
			bool operator()(const shared_ptr<CObj> x, const shared_ptr<CObj> y) const
			{
				//比較する要素と内容を決める
				return x.get()->m_priority < y.get()->m_priority;
			}
	}pr;

	//要素が2以上ないとソートはエラーする
	if (m_task_list->size() >= 2)
	{
		//リストを比較クラスを元にソート実行
		m_task_list->sort(pr);
	}
}

//追加
void CTaskSystem::InsertObj(CObj* obj)
{
	//シェアポインタにオブジェクトのアドレスを格納
	shared_ptr<CObj>sp;
	sp.reset(obj);

	//リストの末端に格納したオブジェクトを追加
	m_task_list->push_back(sp);
}

//リスト内のアクション実行
void CTaskSystem::ListAction()
{
	//リスト内のis_deleteフラグがtrueの要素を削除
	auto i = m_task_list->begin();
	while (i != m_task_list->end())
	{
		if (i->get()->is_delete == true)
		{
			//イテレーターの要素を削除
			i = m_task_list->erase(i);
		}
		else
		{
			i++;
		}
	}


	//リスト内の先端から末端まで
	for (auto ip = m_task_list->begin(); ip != m_task_list->end(); ip++)
	{
		//リスト内のオブジェクトが持つアクションメソッドを実行
		ip->get()->Action();
	}
}

//リスト内のドロー実行
void CTaskSystem::ListDraw()
{
	//リスト内の先端から末端まで
	for (auto ip = m_task_list->begin(); ip != m_task_list->end(); ip++)
	{
		//リスト内のオブジェクトが持つドローメソッドを実行
		ip->get()->Draw();
	}
}