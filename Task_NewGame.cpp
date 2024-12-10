#include "MyPG.h"
#include "MyGameMain.h"

namespace Game
{
	extern int playerStock; //목숨
	extern int stageNum; //스테이지 번호
};

//뉴게임
namespace NewGame
{
	//초기화 처리
	void Initialize()
	{
		Game::playerStock = 3;
		Game::stageNum = 1;
	}
	//해방처리
	void Finalize()
	{
	}
	//갱신처리
	TaskFlag UpDate()
	{

		return TaskFlag::StageLogo;
	}
	//묘화처리
	void Render()
	{
	}
}