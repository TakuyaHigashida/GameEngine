//CPUから取得する頂点情報構造体
struct vertexIn
{
	float4 pos	:	POSITION;
	float4 col	:	COLOR;
	float2 uv	:	UV;
};

//VSからPSに送る情報
struct vertexOut
{
	//頂点情報はラスタライズで使用
	float4 pos : SV_POSITION;
	//以下、PSで使用する
	float4 col	:	COLOR;
	float2 uv	:	UV;
};

//グローバル
cbuffer global
{
	float4 color;
};

//テクスチャ情報
Texture2D	txDiffuse	: register(t0);	//テクスチャのグラフィック
SamplerState	samLinear : register(s0);	//テクスチャサンプラ

//頂点シェーダ
//引数　vertexIn : CPUから受け取る頂点情報
//戻り値 vertexOut : PSに送る情報
//頂点情報を座標変換させるが今回は変換させていない。
vertexOut vs(vertexIn IN)
{
	vertexOut OUT;

	//INからOUTへそのまま渡す
	OUT.pos = IN.pos;	//頂点
	OUT.col = IN.col;	//色
	OUT.uv	= IN.uv;	//UV
	
	return OUT;
}

//ピクセルシェーダ
//引数　vertexOu:VSから送られてきた情報
//戻り値　float4 : Color値
//引数の情報元に色を決める。今回は頂点が持つ色とグロバールからもってきた色を乗算している
float4 ps(vertexOut IN) : SV_Target
{
	float4 col = IN.col * color;
	//UVからテクスチャの色の値を習得
	float4	tex = txDiffuse.Sample(samLinear, IN.uv);

	//colにテクスチャの色合成
	col *= tex;

	return col;
}