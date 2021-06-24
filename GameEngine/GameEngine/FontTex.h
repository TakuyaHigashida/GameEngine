#pragma once
//入力に必要なヘッダー
#include<Windows.h>
#include<locale.h>
#include<wchar.h>
#include<memory>
#include<list>

using namespace std;

#include"DeviceCreate.h"
#include "Draw2DPolygon.h"

//文字識別クラス
class CCherClass
{
	public:
		//コンストラクタ
		CCherClass()
		{
			m_pc.reset();
			m_pTexture = nullptr;
		}
		//デストラクタ
		~CCherClass()
		{
			m_pc.reset();
			SAFE_RELEASE(m_pTexResView);
			SAFE_RELEASE(m_pTexture);
		}

		const wchar_t* GetChar() { return m_pc.get(); }		//登録してる文字を返す

		void CreateCharTex(const wchar_t c, HDC hdc, TEXTMETRIC TM);		//文字テクスチャ作成
		ID3D11ShaderResourceView* GetTexResView() { return m_pTexResView; }	//文字のリソースビュー

	private:
		unique_ptr<wchar_t> m_pc;					//識別用文字
		ID3D11Texture2D* m_pTexture;				//文字のテクスチャ情報
		ID3D11ShaderResourceView* m_pTexResView;	//テクスチャをシェーダに送る入口
};

//フォント描画クラス
typedef class CFontTex
{
	public:
		CFontTex(){}
		~CFontTex(){}

		static void InitFontTex();
		static void DeleteFontTex();

		static void CreateStrTex(const wchar_t* str);	//文字列を基に文字テクスチャ作成
		static void StrDraw(const wchar_t* str, float x, float y, float s, float r, float g, float b, float a);	//文字描画用

	private:
		//フォント作成に必要な変数
		static HFONT m_hFont;		//フォントハンドル:論理フォント(GDIオブジェクト)
		static HDC m_hdc;			//ディスプレイデバイスコンテキストのハンドル
		static HFONT m_oldFont;		//フォントハンドル:物理フォント(GDIオブジェクト)
		static TEXTMETRIC m_TM;		//フォント情報格納用

		static list<unique_ptr<CCherClass>>*list_char_tex;	//文字リスト
}Font;