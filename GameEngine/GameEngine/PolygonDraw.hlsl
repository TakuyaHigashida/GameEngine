//CPU����擾���钸�_���\����
struct vertexIn
{
	float4 pos	:	POSITION;
	float4 col	:	COLOR;
	float2 uv	:	UV;
};

//VS����PS�ɑ�����
struct vertexOut
{
	//���_���̓��X�^���C�Y�Ŏg�p
	float4 pos : SV_POSITION;
	//�ȉ��APS�Ŏg�p����
	float4 col	:	COLOR;
	float2 uv	:	UV;
};

//�O���[�o��
cbuffer global
{
	float4 color;
};

//�e�N�X�`�����
Texture2D	txDiffuse	: register(t0);	//�e�N�X�`���̃O���t�B�b�N
SamplerState	samLinear : register(s0);	//�e�N�X�`���T���v��

//���_�V�F�[�_
//�����@vertexIn : CPU����󂯎�钸�_���
//�߂�l vertexOut : PS�ɑ�����
//���_�������W�ϊ������邪����͕ϊ������Ă��Ȃ��B
vertexOut vs(vertexIn IN)
{
	vertexOut OUT;

	//IN����OUT�ւ��̂܂ܓn��
	OUT.pos = IN.pos;	//���_
	OUT.col = IN.col;	//�F
	OUT.uv	= IN.uv;	//UV
	
	return OUT;
}

//�s�N�Z���V�F�[�_
//�����@vertexOu:VS���瑗���Ă������
//�߂�l�@float4 : Color�l
//�����̏�񌳂ɐF�����߂�B����͒��_�����F�ƃO���o�[����������Ă����F����Z���Ă���
float4 ps(vertexOut IN) : SV_Target
{
	float4 col = IN.col * color;
	//UV����e�N�X�`���̐F�̒l���K��
	float4	tex = txDiffuse.Sample(samLinear, IN.uv);

	//col�Ƀe�N�X�`���̐F����
	col *= tex;

	return col;
}