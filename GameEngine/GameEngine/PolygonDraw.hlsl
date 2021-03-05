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
	float4 pos;
	float4 scale;
	float4 rotation;
	float4 texsize;
};

//�e�N�X�`�����
Texture2D	txDiffuse	: register(t0);	//�e�N�X�`���̃O���t�B�b�N
SamplerState	samLinear : register(s0);	//�e�N�X�`���T���v��

//���_�V�F�[�_
//�����@vertexIn : CPU����󂯎�钸�_���
//�߂�l vertexOut : PS�ɑ�����
//���_�ʒu�ɂ́A(�e�N�X�`���T�C�Y/�E�C���h�E�T�C�Y)�Ł������ߏ�Z
//�c��́ACPU����󂯎�����f�[�^�����̂܂ܗ����Ă���
vertexOut vs(vertexIn IN)
{
	vertexOut OUT;
	float2 p;

	//���_�Ƀ|���S���̒��S���ړ�������
	p.x = IN.pos.x - 0.5f;
	p.y = IN.pos.y - 0.5f;

	//�g�嗦(IN��Polygon�̒��_���W�Ɋg�嗦����Z)
	p.x = p.x * scale.x * texsize.x / 400.0f;
	p.y = p.y * scale.y * texsize.y / 300.0f;

	//�|���S�������_�𒆐S�ɉ�]
	float r = 3.14f / 180.0f * rotation.x;
	OUT.pos.x = p.x * cos(r) - p.y * sin(r)*3.0f/4.0f;
	OUT.pos.y = p.y * cos(r) + p.x * sin(r)*4.0f/3.0f;

	//�|���S�������Ƃ̈ʒu�ɉ�]������(�g�啔�����l��)
	OUT.pos.x += 0.5f * scale.x * texsize.x / 400.0f;
	OUT.pos.y += 0.5f * scale.y * texsize.y / 300.0f;

	//2D���W�ւ̕ϊ�
	OUT.pos.x = +(OUT.pos.x * (texsize.x / 400.0f)) - 1.0f ;
	OUT.pos.y = -(OUT.pos.y * (texsize.y / 300.0f)) + 1.0f ;

	//���s�ړ�
	OUT.pos.x += ( pos.x / 400.0f);
	OUT.pos.y += (-pos.y / 300.0f);

	//IN����OUT�ւ��̂܂ܓn��
	OUT.pos.zw = IN.pos.zw;	//���_
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
	//col.a = 1.0f;

	return col;
}