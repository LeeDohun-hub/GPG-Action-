#include "MyPG.h"
#include "MyGameMain.h"

namespace Game
{
	extern int playerStock; //���
	extern int stageNum; //�������� ��ȣ
};

//������
namespace NewGame
{
	//�ʱ�ȭ ó��
	void Initialize()
	{
		Game::playerStock = 3;
		Game::stageNum = 1;
	}
	//�ع�ó��
	void Finalize()
	{
	}
	//����ó��
	TaskFlag UpDate()
	{

		return TaskFlag::StageLogo;
	}
	//��ȭó��
	void Render()
	{
	}
}