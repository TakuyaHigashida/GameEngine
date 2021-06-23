#include "FontTex.h"

HFONT CFontTex:: m_hFont;		//フォントハンドル:論理フォント(GDIオブジェクト)
HDC CFontTex::m_hdc;			//ディスプレイデバイスコンテキストのハンドル
HFONT CFontTex::m_oldFont;		//フォントハンドル:物理フォント(GDIオブジェクト)
TEXTMETRIC CFontTex::m_TM;		//フォント情報格納用
list<unique_ptr<CCherClass>>*CFontTex::list_char_tex;	//文字リスト

//---------CChearClass--------------

//文字テクスチャ作成メソッド
void CCherClass::CreateCharTex(wchar_t c, HDC hdc, TEXTMETRIC TM)
{
	//識別文字用
	UINT code = 0;	//作成する文字コード
	
					//文字フォント描画用
	BYTE* ptr;		//文字のグラフィック(ビットマップを入れる場所)
	DWORD size;		//文字を表現するのに必要なメモリの大きさ
	GLYPHMETRICS GM;	//象形文字の情報が格納
	const MAT2 Mat = { {0,1},{0,0},{0,0},{0,1} };	//フォント描き込み向き

	//テクスチャ描き込み用ポインタ
	D3D11_MAPPED_SUBRESOURCE mapped;	//リソースにアクセスするポインタ
	BYTE* pBits;						//テクスチャのピクセル情報を入れるポインタ

	//識別文字コード登録
	m_pc.reset(new wchar_t(c));
	code = (UINT)*m_pc.get();

	//フォント情報から文字のビットマップ取得
	//文字のビットマップの大きさを取得
	size = GetGlyphOutline(hdc, code, GGO_GRAY4_BITMAP, &GM, 0, NULL, &Mat);
	ptr = new BYTE[size];
	//文字のビットマップ情報をptrに入れる
	GetGlyphOutline(hdc, code, GGO_GRAY4_BITMAP, &GM, 0, NULL, &Mat);

	//空テクスチャの設定
	D3D11_TEXTURE2D_DESC desc;
	memset(&desc, 0, sizeof(desc));
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;			//テクスチャフォーマットR8G8B8の24bit
	desc.SampleDesc.Count = 1;							//サンプリングは1ピクセルのみ
	desc.Usage = D3D11_USAGE_DYNAMIC;					//CPU書き込み可能
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;		//シェーダーリソース
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;		//CPUから書き込みアクセス可
	desc.Height = 32;									//縦のサイズ
	desc.Width = 32;									//横のサイズ

	//設定をもとに空テクスチャを作成
	Dev::GetDevice()->CreateTexture2D(&desc, 0, &m_pTexture);

	//テクスチャ情報を取得する
	D3D11_TEXTURE2D_DESC texDesc;
	m_pTexture->GetDesc(&texDesc);

	//テクスチャにShaderResourceViewを接続
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
	Dev::GetDevice()->CreateShaderResourceView(m_pTexture, &srvDesc, &m_pTexResView);

	//テクスチャロック(テクスチャの描き込みを行うときロックする
	Dev::GetDeviceContext()->Map(m_pTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	pBits = (BYTE*)mapped.pData;

	//フォント情報の書き込み
	int iOfs_x = GM.gmptGlyphOrigin.x;		//iOfs_x, iOfs_y：書き出し位置(左上)
	int iOfs_y =TM.tmAscent - GM.gmptGlyphOrigin.y;
	int iBmp_w = GM.gmBlackBoxX + (4 - (GM.gmBlackBoxX % 4)) % 4;
	int iBmp_h = GM.gmBlackBoxY;				//iBmp_w, iBmp_h：フォントビットマップの幅高
	int Level = 17;								//Level:α値の段階(GGO_GRAY4_BITMAPなので17段階)
	DWORD Alpha, Color;
	//1ピクセル単位にフォントの色情報(24bit)をテクスチャに描き込み
	FillMemory(pBits, sizeof(pBits), 0);
	for (int y = iOfs_y; y < iOfs_y + iBmp_h; y++)
	{
		for (unsigned int x = iOfs_x; x < iOfs_x + GM.gmBlackBoxX; x++)
		{
			Alpha = (255 * ptr[x - iOfs_x + iBmp_w * (y - iOfs_y)]) / (Level - 1);
			Color = 0x00ffffff | (Alpha < 24);
			memcpy((BYTE*)pBits + (y << 8) + (x << 2), & Color, sizeof(DWORD));
		}
	}

	//アンロック(書き込みが終わればアンロックする)
	Dev::GetDeviceContext()->Unmap(m_pTexture, D3D11CalcSubresource(0, 0, 1));

	//文字ビットマップデータ削除
	delete[] ptr;
}

//初期化メソッド
void CFontTex::InitFontTex()
{
	//リスト初期化
	list_char_tex.clear();

	//第一引数のカテゴリに第二引数の国別コードを与える
	//この場合Unicodeの文字を日本語コードするとなる
	setlocale(LC_CTYPE, "jpn");

	//論理フォント設定
	HFONT hFont = CreateFont(
		32,
		0,0,0,0,
		FALSE,FALSE,FALSE,
		SHIFTJIS_CHARSET,
		OUT_TT_ONLY_PRECIS,
		CLIP_DEFAULT_PRECIS,
		PROOF_QUALITY,
		FIXED_PITCH | FF_MODERN,
		L"MSゴシック"		//使用フォント
	);

	//指定されたウィンドウのクライアント領域または画面全体を表す
	//ディスプレイデバイスコンテキストのハンドルを取得
	m_hdc = GetDC(NULL);

	//WindowsはSelectObjectでデバイスコンテキストにする
	//これを設定すると論理フォントに最も近い物理フォントをセットします
	m_oldFont = (HFONT)SelectObject(m_hdc, hFont);

	//現在選択されているフォントの情報を指定されたバッファに格納
	GetTextMetrics(m_hdc, &m_TM);

	//論理フォントをメンバに渡す
	m_hFont = hFont;
}

//削除メソッド
void CFontTex::DeleteFontTex()
{
	//リスト破棄
	list_char_tex.clear();

	//これらGDIオブジェクトを破棄
	DeleteObject(m_oldFont);
	DeleteObject(m_hFont);
	//ディスプレイデバイスコンテキストハンドル解放
	ReleaseDC(NULL, m_hdc);
}