#pragma once
//Device作成に必要なヘッダー
#include <Windows.h>
#include <D3D11.h>
#include<d3dCompiler.h>
//開放マクロ
#define SAFE_DELETE(p)			{ if (p) { delete (p);			(p)=nullptr; } }
#define SAFE_DELETE_ARRAY(p)	{ if (p) { delete[] (p);		(p)=nullptr; } }
#define SAFE_RELEASE(p)			{ if (p) { (p)->Release();		(p)=nullptr; } }

//2Dポリゴン表示で使用する構造体-------------
//頂点レイアウト構造体(頂点が持つ情報)
struct POINT_LAYOUT
{
	float pos[3];		//X-Y-Z		:頂点
	float color[4];		//R-G-B-A	:色
	float uv[2];		//U-V		:テクスチャ位置
};

//コンスタントバッファ構造体
struct POLYGON_BUFFER
{
	float color[4];		//R-G-B-A:ポリゴンカラー
	float pos[4];		//ポリゴンの位置情報
	float scale[4];		//拡大縮小率
	float rotation[4];	//回転情報
};

typedef class CDraw2DPolygon
{
	public:
		CDraw2DPolygon(){}
		~CDraw2DPolygon() {}

		static void Draw2D(float x, float y)						{ Draw2D(x, y, 1.0f, 1.0f, 0.0f); }
		static void Draw2D(float x, float y, float r)				{ Draw2D(x, y, 1.0f, 1.0f, r); }
		static void Draw2D(float x, float y, float sx, float sy)	{ Draw2D(x, y, sx, sy, 0.0f); }
		static void Draw2D(float x, float y, float sx, float sy ,float r);			//描画

		static HRESULT InitPolygonRender();	//ポリゴン表示環境の初期化
		static void DeletePolygonRender();	//ポリゴン表示環境の破棄
	private:
		//GPU側で扱う用
		static ID3D11VertexShader*	m_pVertexShader;		//パーテックスシェーダー
		static ID3D11PixelShader*	m_pPixelShader;			//ピクセルシェーダー
		static ID3D11InputLayout*	m_pVertexLayout;		//頂点入力レイアウト
		static ID3D11Buffer*		m_pConstantBuffer;		//コンスタントバッファ
		//ポリゴン情報登録用バッファ
		static ID3D11Buffer* m_pVertexBuffer;				//バーティクスバッファ
		static ID3D11Buffer* m_pIndexBuffer;				//インデックスバッファ

		//テクスチャに必要なもの
		static ID3D11SamplerState*		m_pSampleLinear;	//テクスチャサンプラー
		static ID3D11ShaderResourceView*m_pTexture;			//テクスチャリソース

}Draw;