#pragma once

//�g�p����w�b�_�[
#include "Draw2DPolygon.h"
#include "Input.h"
#include "Audio.h"
#include "TaskSystem.h"

//��l���N���X
class CHero :public CObj
{
public:
	CHero();
	~CHero();
	void Action();
	void Draw();
private:
};
