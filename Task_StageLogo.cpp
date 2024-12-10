#include "MyPG.h"
#include "MyGameMain.h"

namespace Game
{
	extern int playerStock; //목숨수
	extern int stageNum; //스테이지번호
};

//다음스테이지(로고)
namespace StageLogo
{
	DG::Image::SP imgBack, imgFont;
	int timeCnt;
	void ImageFont_Draw(int x_, int y_, const char* msg_);

	//초기화처리
	void Initialize()
	{
		imgBack = DG::Image::Create("./data/image/titleback.jpg");
		imgFont = DG::Image::Create("./data/image/font_text.png");
		timeCnt = 0;
	}

	//해방처리
	void Finalize()
	{
		imgBack.reset();
		imgFont.reset();
	}

	//갱신처리
	TaskFlag UpDate()
	{
		TaskFlag rtv = TaskFlag::StageLogo;
		if (timeCnt > 30)
		{
			rtv = TaskFlag::Game;
		}
		timeCnt++;
		return rtv;
	}

	//묘화처리
	void Render()
	{
		ML::Box2D draw(0, 0, 960, 540);
		ML::Box2D src(0, 0, 960, 540);
		imgBack->Draw(draw, src);
		string buf;
		buf = "STAGE:" + to_string(Game::stageNum);
		ImageFont_Draw(300, 200, buf.c_str());
		buf = "LIFE:" + to_string(Game::playerStock);
		ImageFont_Draw(300, 250, buf.c_str());
	}
	//지정된문자열을 화면폰트로 묘화합니다.
	//int x_ y_ 1문자마다 개시위치 X Y
	//char *msg_묘화문자열
	
	void ImageFont_Draw(int x_, int y_, const char* msg_)
	{
		int draw_x = x_, draw_y = y_;
		for (int i = 0; i < (int)strlen(msg_); i++) {
			int font_x = ((unsigned char)msg_[i]) % 16 * 32;
			int font_y = ((unsigned char)msg_[i]) / 16 * 32;
			ML::Box2D draw(draw_x, draw_y, 32, 32);
			ML::Box2D src(font_x, font_y, 32, 32);
			imgFont->Draw(draw, src);
			draw_x += 32;
		}
	}
}