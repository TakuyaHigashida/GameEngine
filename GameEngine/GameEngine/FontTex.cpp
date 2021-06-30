//STL�f�o�b�O�@�\��OFF�ɂ���
#define _SECURE_SCL (0)
#define _HAS_ITERATOR_DEBUGGING (0)

#include "FontTex.h"

HFONT CFontTex:: m_hFont;		//�t�H���g�n���h��:�_���t�H���g(GDI�I�u�W�F�N�g)
HDC CFontTex::m_hdc;			//�f�B�X�v���C�f�o�C�X�R���e�L�X�g�̃n���h��
HFONT CFontTex::m_oldFont;		//�t�H���g�n���h��:�����t�H���g(GDI�I�u�W�F�N�g)
TEXTMETRIC CFontTex::m_TM;		//�t�H���g���i�[�p
list<unique_ptr<CCherClass>>*CFontTex::list_char_tex;	//�������X�g

//---------CChearClass--------------

//�����e�N�X�`���쐬���\�b�h
void CCherClass::CreateCharTex(const wchar_t c, HDC hdc, TEXTMETRIC TM)
{
	//���ʕ����p
	UINT code = 0;	//�쐬���镶���R�[�h
	
					//�����t�H���g�`��p
	BYTE* ptr;		//�����̃O���t�B�b�N(�r�b�g�}�b�v������ꏊ)
	DWORD size;		//������\������̂ɕK�v�ȃ������̑傫��
	GLYPHMETRICS GM;	//�ی`�����̏�񂪊i�[
	const MAT2 Mat = { {0,1},{0,0},{0,0},{0,1} };	//�t�H���g�`�����݌���

	//�e�N�X�`���`�����ݗp�|�C���^
	D3D11_MAPPED_SUBRESOURCE mapped;	//���\�[�X�ɃA�N�Z�X����|�C���^
	BYTE* pBits;						//�e�N�X�`���̃s�N�Z����������|�C���^

	//���ʕ����R�[�h�o�^
	m_pc.reset(new wchar_t(c));
	code = (UINT)*m_pc.get();

	//�t�H���g��񂩂當���̃r�b�g�}�b�v�擾
	//�����̃r�b�g�}�b�v�̑傫�����擾
	size = GetGlyphOutline(hdc, code, GGO_GRAY4_BITMAP, &GM, 0, NULL, &Mat);
	ptr = new BYTE[size];
	//�����̃r�b�g�}�b�v����ptr�ɓ����
	GetGlyphOutline(hdc, code, GGO_GRAY4_BITMAP, &GM, 0, NULL, &Mat);

	//��e�N�X�`���̐ݒ�
	D3D11_TEXTURE2D_DESC desc;
	memset(&desc, 0, sizeof(desc));
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;			//�e�N�X�`���t�H�[�}�b�gR8G8B8��24bit
	desc.SampleDesc.Count = 1;							//�T���v�����O��1�s�N�Z���̂�
	desc.Usage = D3D11_USAGE_DYNAMIC;					//CPU�������݉\
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;		//�V�F�[�_�[���\�[�X
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;		//CPU���珑�����݃A�N�Z�X��
	desc.Height = 32;									//�c�̃T�C�Y
	desc.Width = 32;									//���̃T�C�Y

	//�ݒ�����Ƃɋ�e�N�X�`�����쐬
	Dev::GetDevice()->CreateTexture2D(&desc, 0, &m_pTexture);

	//�e�N�X�`�������擾����
	D3D11_TEXTURE2D_DESC texDesc;
	m_pTexture->GetDesc(&texDesc);

	//�e�N�X�`����ShaderResourceView��ڑ�
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
	Dev::GetDevice()->CreateShaderResourceView(m_pTexture, &srvDesc, &m_pTexResView);

	//�e�N�X�`�����b�N(�e�N�X�`���̕`�����݂��s���Ƃ����b�N����
	Dev::GetDeviceContext()->Map(m_pTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	pBits = (BYTE*)mapped.pData;

	//�t�H���g���̏�������
	int iOfs_x = GM.gmptGlyphOrigin.x;		//iOfs_x, iOfs_y�F�����o���ʒu(����)
	int iOfs_y =TM.tmAscent - GM.gmptGlyphOrigin.y;
	int iBmp_w = GM.gmBlackBoxX + (4 - (GM.gmBlackBoxX % 4)) % 4;
	int iBmp_h = GM.gmBlackBoxY;				//iBmp_w, iBmp_h�F�t�H���g�r�b�g�}�b�v�̕���
	int Level = 17;								//Level:���l�̒i�K(GGO_GRAY4_BITMAP�Ȃ̂�17�i�K)
	DWORD Alpha, Color;
	//1�s�N�Z���P�ʂɃt�H���g�̐F���(32bit)���e�N�X�`���ɕ`������
	memset(pBits, 0x00, sizeof(DWORD)* 32 * 32);
	for (int y = iOfs_y; y < iOfs_y + iBmp_h; y++)
	{
		for (unsigned int x = iOfs_x; x < iOfs_x + GM.gmBlackBoxX; x++)
		{
			Alpha = (255 * ptr[x - iOfs_x + iBmp_w * (y - iOfs_y)]) / (Level - 1);
			Color = 0x00ffffff | (Alpha < 24);
			memcpy((BYTE*)pBits + (y << 7) + (x << 2), & Color, sizeof(DWORD));
		}
	}

	//�A�����b�N(�������݂��I���΃A�����b�N����)
	Dev::GetDeviceContext()->Unmap(m_pTexture, D3D11CalcSubresource(0, 0, 1));

	//�����r�b�g�}�b�v�f�[�^�폜
	delete[] ptr;
}

//-----CFontTex---------
//�����`��
void CFontTex::StrDraw(const wchar_t* str, float x, float y, float s, float r, float g, float b, float a)
{
	//�������o�^
	CreateStrTex(str);

	//�`��
	float c[] = { r, g, b, a };
	for (unsigned int i = 0; i < wcslen(str); i++)
	{
		for (auto itr = list_char_tex->begin(); itr != list_char_tex->end(); itr++)
		{
			if (*itr->get()->GetChar() == str[i])
			{
				Draw::Draw2DChar(itr->get()->GetTexResView(), x+(32*s*i), y, s, c);
				break;
			}
		}
	}
}

//���������ɕ����e�N�X�`���쐬
void CFontTex::CreateStrTex(const wchar_t* str)
{
	//������𕶎���list�ɓo�^�ς݂��`�F�b�N
	for (unsigned int i = 0; i < wcslen(str); i++)
	{
		bool is_char_entry = false;

		//���X�g���猟��
		for (auto itr = list_char_tex->begin(); itr != list_char_tex->end(); itr++)
		{
			//�o�^���ꂽ������str�̕������r
			if (*itr->get()->GetChar()==str[i])
			{
				//�o�^����Ă���
				is_char_entry = true;
			}
		}
		//�Ȃ����CreateCharTex�쐬����
		if (is_char_entry == false)
		{
			//�����e�N�X�`���쐬
			unique_ptr<CCherClass> obj(new CCherClass());
			obj->CreateCharTex(str[i], m_hdc, m_TM);

			//���X�g�ɏ��n
			list_char_tex->push_back(move(obj));
		}
	}
}

//���������\�b�h
void CFontTex::InitFontTex()
{
	//���X�g������
	list_char_tex = new list<unique_ptr<CCherClass>>;

	//�������̃J�e�S���ɑ������̍��ʃR�[�h��^����
	//���̏ꍇUnicode�̕�������{��R�[�h����ƂȂ�
	setlocale(LC_CTYPE, "jpn");

	//�_���t�H���g�ݒ�
	HFONT hFont = CreateFont(
		32,
		0,0,0,0,
		FALSE,FALSE,FALSE,
		SHIFTJIS_CHARSET,
		OUT_TT_ONLY_PRECIS,
		CLIP_DEFAULT_PRECIS,
		PROOF_QUALITY,
		FIXED_PITCH | FF_MODERN,
		L"MS�S�V�b�N"		//�g�p�t�H���g
	);

	//�w�肳�ꂽ�E�B���h�E�̃N���C�A���g�̈�܂��͉�ʑS�̂�\��
	//�f�B�X�v���C�f�o�C�X�R���e�L�X�g�̃n���h�����擾
	m_hdc = GetDC(NULL);

	//Windows��SelectObject�Ńf�o�C�X�R���e�L�X�g�ɂ���
	//�����ݒ肷��Ƙ_���t�H���g�ɍł��߂������t�H���g���Z�b�g���܂�
	m_oldFont = (HFONT)SelectObject(m_hdc, hFont);

	//���ݑI������Ă���t�H���g�̏����w�肳�ꂽ�o�b�t�@�Ɋi�[
	GetTextMetrics(m_hdc, &m_TM);

	//�_���t�H���g�������o�ɓn��
	m_hFont = hFont;
}

//�폜���\�b�h
void CFontTex::DeleteFontTex()
{
	//���X�g�j��
	list_char_tex->clear();
	delete list_char_tex;

	//�����GDI�I�u�W�F�N�g��j��
	DeleteObject(m_oldFont);
	DeleteObject(m_hFont);
	//�f�B�X�v���C�f�o�C�X�R���e�L�X�g�n���h�����
	ReleaseDC(NULL, m_hdc);
}