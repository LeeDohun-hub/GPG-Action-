#include "MyPG.h"
#include "MyGameMain.h"

// 게임 네임스페이스
namespace Game
{
	// 이미지 포인터
	DG::Image::SP imgMapChip, imgPlayer, imgBG;

	struct MapData {
		int arr[17][30];
		ML::Box2D chip[19 * 6];
	};

	enum class State {Non, Normal};
	enum class Motion {Move, Dead, Happy};
	enum class Angle_LR {Left, Right};

	struct Chara {
		float x;
		float y;
		bool hitFlag;
		ML::Box2D hitBase;
		ML::Box2D footBase;
		float fallSpeed;
		float jumpPow;
		ML::Box2D drawBase;
		ML::Box2D src;
		State state; //캐릭터의 상태
		Motion motion; //캐릭터의 자세
		Angle_LR angle; //캐릭터의 방향
		int moveCnt; //행동처리용카운터
		int animCnt; //애니메이션처리용카운터
		ML::Box2D headBase; //머리위접촉
	};

	MapData mapData;
	void Map_Initialize(MapData& md_);
	bool Map_Load(MapData& md_, int n_);
	void Map_UpData(MapData& md_);
	void Map_Render(MapData& md_);
	bool Map_CheckHit(MapData& md_, const ML::Box2D& h_);
	void Map_CreateSGE(MapData& md_); // 시작, 목표, 적

	Chara player;
	void Player_Initialize(Chara& c_, int x_, int y_);
	void Player_UpDate(Chara& c_);
	void Player_Render(Chara& c_);
	void Chara_CheckMove(Chara& c_, ML::Vec2& e_);
	bool Chara_CheckFoot(Chara& c_);
	void Player_Anim(Chara& c_);
	bool Chara_CheckHead(Chara& c_);
	TaskFlag Check_State();
	//뉴게임,스테이지로고
	int playerStock; //목숨
	int stageNum; //스테이지번호

	DG::Image::SP imgGoal;
	Chara goal;
	void Goal_Initialize(Chara& c_, int x_, int y_);
	void Goal_UpDate(Chara& c_);
	void Goal_Render(Chara& c_);

	DG::Image::SP imgStart;
	Chara start;
	void Start_Initialize(Chara& c_, int x_, int y_);
	void Start_UpDate(Chara& c_);
	void Start_Render(Chara& c_);

	DG::Image::SP imgEnemy;
	Chara enemys[30 * 17]; //일단 최대까지 준비
	void Enemy_Initialize(Chara& c_, int x_, int y_);
	void Enemy_UpDate(Chara& c_);
	void Enemy_Render(Chara& c_);

	DG::Image::SP imgShot;
	Chara shots[1];
	void Shot_Initialize(Chara& c_, int x_, int y_, Angle_LR d_);
	void Shot_UpDate(Chara& c_);
	void Shot_Render(Chara& c_);
	void Shot_Appear(int x_, int y_, Angle_LR d_);
	//-----------------------------------------------------------------------------
	// 초기화
	// 이 함수는 게임 환경을 설정하고 리소스를 로드합니다
	//-----------------------------------------------------------------------------
	void Initialize()
	{
		// 배경색 설정
		ge->dgi->EffectState().param.bgColor = ML::Color(1, 0, 1, 0);

		imgMapChip = DG::Image::Create("./data/image/MapSetting_org.png");
		imgPlayer = DG::Image::Create("./data/image/all.png");
		imgBG = DG::Image::Create("./data/image/back.png");
		imgGoal = DG::Image::Create("./data/image/Goal.png"); //골지점사진을 불러오기
		imgStart = DG::Image::Create("./data/image/start.png"); //스타트파일명
		imgEnemy = DG::Image::Create("./data/image/Enemy.png"); //적 파일명
		imgShot = DG::Image::Create("./data/image/Shot.png");

		
		// 맵 초기화
		Map_Initialize(mapData);
		
		// 맵 데이터 로드
		Map_Load(mapData, 1);
		// 적의 초기화
		for (int i = 0; i < _countof(enemys); ++i) {
			enemys[i].state = State::Non;
		}
		//탄의 무효화
		for (int i = 0; i < _countof(shots); ++i) {
			shots[i].state = State::Non;
		}

		// 시작 지점과 목표 지점 생성
		Map_CreateSGE(mapData); // 여기에 추가
	
		// 플레이어 초기화
		Player_Initialize(player, int(start.x), int(start.y));
		

	}
	//-----------------------------------------------------------------------------
	// 정리
	// 이 함수는 게임 종료 전에 리소스를 정리합니다
	//-----------------------------------------------------------------------------
	void Finalize()
	{
		imgMapChip.reset();
		imgPlayer.reset();
		imgBG.reset();
		imgGoal.reset();
		imgEnemy.reset();
		imgShot.reset();
	}
	//-----------------------------------------------------------------------------
	// 업데이트
	// 이 함수는 게임 상태를 업데이트하고 입력을 처리합니다
	//-----------------------------------------------------------------------------
	TaskFlag UpDate()
	{
		auto inp = ge->in1->GetState();
		Map_UpData(mapData);
		// 적 업데이트
		for (int i = 0; i < _countof(enemys); ++i) {
			Enemy_UpDate(enemys[i]); // 적 업데이트 호출
		}
		//탄 업데이트
		for (int i = 0; i < _countof(shots); ++i) {
			Shot_UpDate(shots[i]); // 적 업데이트 호출
		}
		// 플레이어 업데이트
		Player_UpDate(player);

		// 골 업데이트
		Goal_UpDate(goal);

		// 스타트 업데이트
		Start_UpDate(start);
		TaskFlag rtv = Check_State();
		return rtv;

	}
	//-----------------------------------------------------------------------------
	// 렌더링
	// 이 함수는 화면에 게임 요소를 렌더링합니다
	//-----------------------------------------------------------------------------
	void Render()
	{
		Map_Render(mapData);
		//적 렌더링
		for (int i = 0; i < _countof(enemys); ++i) {
			Enemy_Render(enemys[i]); // 적 렌더링 호출
		}
		//탄 렌더링
		for (int i = 0; i < _countof(shots); ++i) {
			Shot_Render(shots[i]); // 탄 렌더링 호출
		}
		// 플레이어 렌더링
		Player_Render(player);
		
		// 골 렌더링
		Goal_Render(goal);

		//스타트 렌더링
		Start_Render(start);
	}
	//-----------------------------------------------------------------------------
	// 상태 확인
	// 현재 상태를 체크하고 적절한 태스크 플래그를 반환합니다
	//-----------------------------------------------------------------------------
	TaskFlag Check_State()
	{
		auto inp = ge->in1->GetState();
		// 타이틀로 전환
		if (inp.ST.down) {
			return TaskFlag::Title;
		}
		//플레이어가 사망시
		if (player.state != State::Non) {
			//사망처리
			if (player.motion == Motion::Dead && player.moveCnt > 120) {
				playerStock--;
				if (playerStock > 0) {
					return TaskFlag::StageLogo; //같은 스테이지로 복귀
				}
				else {
					return TaskFlag::Title; //타이틀로 돌아가기
				}
			}
			//스테이지클리어 판정
			if (player.motion == Motion::Happy && player.moveCnt > 120) {
				stageNum++;
				if (stageNum <= 10) {
					return TaskFlag::StageLogo; //다음 스테이지로 이동
				}
				else {
					return TaskFlag::Ending; //엔딩으로
				}
			}
		}
		// 게임 상태 유지
		return TaskFlag::Game;
	}
	//-----------------------------------------------------------------------------
	void Map_Initialize(MapData& md_)
	{
		// 맵 데이터 초기화
		for (int y = 0; y < 17; ++y) {
			for (int x = 0; x < 30; ++x) {
				md_.arr[y][x] = 0;
			}
		}
		// 맵 칩 초기화
		for (int c = 0; c < 19 * 6; ++c) {
			md_.chip[c] = ML::Box2D((c % 19) * 32, (c / 18) * 32, 32, 32);
		}
	}
	//-----------------------------------------------------------------------------
	bool Map_Load(MapData& md_, int n_)
	{
		// 파일 경로 설정
		string filePath = "./data/map/mapdata" + to_string(n_) + ".csv";
		// 파일 열기
		ifstream fin(filePath);
		if (!fin) { return false; } // 파일 열기 실패

		// CSV 데이터 읽기
		for (int y = 0; y < 17; ++y) {
			string lineText;
			getline(fin, lineText);
			istringstream ss_lt(lineText);
			for (int x = 0; x < 30; ++x) {
				string tc;
				getline(ss_lt, tc, ',');
				stringstream ss;
				ss << tc;
				ss >> md_.arr[y][x];
			}
		}

		// 파일 닫기
		fin.close();
		return true;
	}
	//-----------------------------------------------------------------------------
	void Map_CreateSGE(MapData& md_) {
		int eneCnt = 0;
		for (int y = 0; y < 17; ++y) {
			for (int x = 0; x < 30; ++x) {
				// 골지점발견
				if (md_.arr[y][x] == 39) {
					Goal_Initialize(goal, x * 32 + 16, y * 32 + 16);
					md_.arr[y][x] = -1;
				}
				//스타트지점
				if (md_.arr[y][x] == 38) {
					Start_Initialize(start, x * 32 + 16, y * 32 + 16);
					md_.arr[y][x] = -1;
				}
				//적출현
				if (md_.arr[y][x] == 19) {
					Enemy_Initialize(enemys[eneCnt], x * 32 + 16, y * 32 + 16);
					md_.arr[y][x] = -1;
					eneCnt++;
				}
			}
		}
	}

	//-----------------------------------------------------------------------------
	void Map_UpData(MapData& md_)
	{
		// 맵 데이터 업데이트
	}
	//-----------------------------------------------------------------------------
	void Map_Render(MapData& md_)
	{
		ML::Box2D draw(0, 0, 960, 540);
		ML::Box2D src(0, 0, 1280, 720);
		imgBG->Draw(draw, src);

		for (int y = 0; y < 17; ++y) {
			for (int x = 0; x < 30; ++x) {
				if (md_.arr[y][x] != -1) {
					ML::Box2D draw(x * 32, y * 32, 32, 32);
					int num = md_.arr[y][x]; // 맵 칩 번호
					ML::Box2D src = md_.chip[num]; // 맵 칩의 소스
					imgMapChip->Draw(draw, src);
				}
			}
		}
	}

	//-----------------------------------------------------------------------------
	bool Map_CheckHit(MapData& md_, const ML::Box2D& h_)
	{
		ML::Rect r = { h_.x, h_.y, h_.x + h_.w, h_.y + h_.h };
		// 맵 경계 검사
		ML::Rect m = { 0, 0, 32 * 30, 32 * 17 };
		if (r.left < m.left) { r.left = m.left; }
		if (r.top < m.top) { r.top = m.top; }
		if (r.right > m.right) { r.right = m.right; }
		if (r.bottom > m.bottom) { r.bottom = m.bottom; }

		// 충돌 체크
		int sx, sy, ex, ey;
		sx = r.left / 32;
		sy = r.top / 32;
		ex = (r.right - 1) / 32;
		ey = (r.bottom - 1) / 32;

		// 맵 데이터와 충돌 검사
		for (int y = sy; y <= ey; ++y) {
			for (int x = sx; x <= ex; ++x) {
				if (8 <= md_.arr[y][x]) {
					return true;
				}
			}
		}
		return false;
	}
	//-----------------------------------------------------------------------------
	void Player_Initialize(Chara& c_, int x_, int y_)
	{
		c_.x = float(x_);
		c_.y = float(y_);
		c_.hitFlag = false;
		c_.hitBase = ML::Box2D(-16, -24, 32, 48);
		c_.footBase = ML::Box2D(c_.hitBase.x, c_.hitBase.h + c_.hitBase.y, c_.hitBase.w, 1);
		c_.fallSpeed = 0.0f;
		c_.jumpPow = -7.2f;
		c_.drawBase = ML::Box2D(0, 0, 0, 0);
		c_.src = ML::Box2D(0, 0, 0, 0);
		c_.state = State::Normal;
		c_.motion = Motion::Move;
		c_.angle = Angle_LR::Right;
		c_.moveCnt = 0;
		c_.animCnt = 0;
		c_.headBase = ML::Box2D(c_.hitBase.x, c_.hitBase.h - c_.y, c_.hitBase.w, 1);
	}
	//-----------------------------------------------------------------------------
	void Player_UpDate(Chara& c_)
	{
		if (c_.state == State::Non) { return; }

		auto inp = ge->in1->GetState();
		ML::Vec2 est = { 0, 0 };
		switch (c_.motion) 
		{
		case Motion::Move:
			if (inp.LStick.BL.on) { est.x -= 3; c_.angle = Angle_LR::Left; }
			if (inp.LStick.BR.on) { est.x += 3; c_.angle = Angle_LR::Right; }
			if (inp.B1.down) {
				if (true == c_.hitFlag) { // 점프 처리
					c_.fallSpeed = c_.jumpPow;
				}
			}
			if (inp.B2.down) {
				Shot_Appear(int(c_.x), int(c_.y), c_.angle);
			}
			break;
		case Motion::Dead:
			break;
		case Motion::Happy:
			break;
		}
		
		est.y += c_.fallSpeed;
		Chara_CheckMove(c_, est);

		// 발 아래 충돌 체크
		c_.hitFlag = Chara_CheckFoot(c_);
		if (true == c_.hitFlag) {
			c_.fallSpeed = 0; // 낙하 속도 초기화
		}
		else {
			c_.fallSpeed += ML::Gravity(32) * 3; // 중력 적용
		}

		//머리위 판정
		if (c_.fallSpeed < 0) { //상승중
			if (Chara_CheckHead(c_)) {
				c_.fallSpeed = 0; // 낙하 속도 초기화
			}
		}
		c_.moveCnt++;
		c_.animCnt++;

	}
	//-----------------------------------------------------------------------------
	void Player_Render(Chara& c_)
	{
		if (c_.state == State::Non) { return; }
		Player_Anim(c_);
		ML::Box2D draw = c_.drawBase.OffsetCopy(int(c_.x), int(c_.y));
		imgPlayer->Draw(draw, c_.src);
	}
	//-----------------------------------------------------------------------------
	void Chara_CheckMove(Chara& c_, ML::Vec2& e_)
	{
		c_.hitFlag = false; // 이동 중 충돌 플래그 초기화

		// 수평 이동 처리
		while (e_.x != 0) {
			float preX = c_.x;
			if (e_.x >= 1) { c_.x += 1; e_.x -= 1; }
			else if (e_.x <= -1) { c_.x -= 1; e_.x += 1; }
			else { c_.x += e_.x; e_.x = 0; }
			ML::Box2D hit = c_.hitBase.OffsetCopy(c_.x, c_.y);
			if (true == Map_CheckHit(mapData, hit)) {
				c_.x = preX; // 충돌 시 이전 위치로 되돌리기
				break;
			}
		}
		// 수직 이동 처리
		while (e_.y != 0) {
			float preY = c_.y;
			if (e_.y >= 1) { c_.y += 1; e_.y -= 1; }
			else if (e_.y <= -1) { c_.y -= 1; e_.y += 1; }
			else { c_.y += e_.y; e_.y = 0; }
			ML::Box2D hit = c_.hitBase.OffsetCopy(c_.x, c_.y);
			if (true == Map_CheckHit(mapData, hit)) {
				c_.y = preY; // 충돌 시 이전 위치로 되돌리기
				break;
			}
		}
	}
	//-----------------------------------------------------------------------------
	bool Chara_CheckFoot(Chara& c_)
	{
		ML::Box2D hit = c_.footBase.OffsetCopy(c_.x, c_.y);
		return Map_CheckHit(mapData, hit);
	}

	// 플레이어 애니메이션
	void Player_Anim(Chara& c_)
	{
		switch (c_.motion)
		{
		case Motion::Move:
			c_.drawBase = ML::Box2D(-16, -40, 32, 64);
			switch ((c_.animCnt / 8) % 6) 
			{
			case 0: c_.src = ML::Box2D(0, 0, 32, 64); break;
			case 1: c_.src = ML::Box2D(32, 0, 32, 64); break;
			case 2: c_.src = ML::Box2D(64, 0, 32, 64); break;
			case 3: c_.src = ML::Box2D(96, 0, 32, 64); break;
			case 4: c_.src = ML::Box2D(128, 0, 32, 64); break;
			case 5: c_.src = ML::Box2D(160, 0, 32, 64); break;
			}
			break;
		case Motion::Dead:
			c_.drawBase = ML::Box2D(-32, -8, 64, 32);
			c_.src = ML::Box2D(192, 96, 64, 32);
			break;
		case Motion::Happy:
			c_.drawBase = ML::Box2D(-16, -40, 32, 64);
			switch ((c_.animCnt / 16) % 2)
			{
			case 0: c_.src = ML::Box2D(0, 128, 32, 64);		break;
			case 1: c_.src = ML::Box2D(32, 128, 32, 64);	break;
			}
			break;
		}
		//왼쪽방향반전
		if (c_.angle == Angle_LR::Right && c_.drawBase.w >= 0)
		{
			c_.drawBase.x = -c_.drawBase.x;
			c_.drawBase.w = -c_.drawBase.w;
		}
	}

	bool Chara_CheckHead(Chara& c_)
	{
		ML::Box2D hit = c_.headBase.OffsetCopy(c_.x, c_.y);
		if (Map_CheckHit(mapData, hit)) {
			c_.fallSpeed = 0; // 충돌 시 낙하 속도 초기화
			return true;
		}
		return false;
	}

	void Goal_Initialize(Chara& c_, int x_, int y_)
	{
		c_.x = float(x_);
		c_.y = float(y_);
		c_.hitFlag = false;
		c_.hitBase = ML::Box2D(-1, 14, 2, 2);
		c_.footBase = ML::Box2D(0, 0, 0, 0);
		c_.headBase = ML::Box2D(0, 0, 0, 0);
		c_.fallSpeed = 0.0f;
		c_.jumpPow = 0.0f;
		c_.drawBase = ML::Box2D(-16, -16, 32, 32);
		c_.src = ML::Box2D(0, 0, 32, 32);
		c_.state = State::Normal;
		c_.motion = Motion::Move;
		c_.angle = Angle_LR::Right;
		c_.moveCnt = 0;
		c_.animCnt = 0;
	}

	void Goal_UpDate(Chara& c_)
	{
		if (c_.state == State::Non) { return; }
		//플레이어의 접촉판정
		ML::Box2D me = c_.hitBase.OffsetCopy(int(c_.x), int(c_.y));
		if (player.state == State::Normal) {
			if (player.motion == Motion::Move) {
				ML::Box2D you = player.hitBase.OffsetCopy(int(player.x), int(player.y));
				if (you.Hit(me)) {
					player.motion = Motion::Happy;
					player.moveCnt = 0;
					player.animCnt = 0;
				}
			}
		}
	}

	void Goal_Render(Chara& c_)
	{
		if (c_.state == State::Non) { return; }
		ML::Box2D draw = c_.drawBase.OffsetCopy(int(c_.x), int(c_.y));
		imgGoal->Draw(draw, c_.src);

	}

	void Start_Initialize(Chara& c_, int x_, int y_)
	{
		c_.x = float(x_);
		c_.y = float(y_);
		c_.hitFlag = false;
		c_.hitBase = ML::Box2D(-1, 14, 2, 2);
		c_.footBase = ML::Box2D(0, 0, 0, 0);
		c_.headBase = ML::Box2D(0, 0, 0, 0);
		c_.fallSpeed = 0.0f;
		c_.jumpPow = 0.0f;
		c_.drawBase = ML::Box2D(-16, -16, 32, 32);
		c_.src = ML::Box2D(0, 0, 32, 32);
		c_.state = State::Normal;
		c_.motion = Motion::Move;
		c_.angle = Angle_LR::Right;
		c_.moveCnt = 0;
		c_.animCnt = 0;
	}

	void Start_UpDate(Chara& c_)
	{
		if (c_.state == State::Non) { return; }
		//스타트지점은 골과 달리, 특별히 처리하지 않음
	}

	void Start_Render(Chara& c_)
	{
		if (c_.state == State::Non) { return; }
		ML::Box2D draw = c_.drawBase.OffsetCopy(int(c_.x), int(c_.y));
		imgStart->Draw(draw, c_.src);
	}

	void Enemy_Initialize(Chara& c_, int x_, int y_)
	{
		c_.x = float(x_);
		c_.y = float(y_);
		c_.hitFlag = false;
		c_.hitBase = ML::Box2D(-16, -16, 32, 32);
		c_.footBase = ML::Box2D(0, 0, 0, 0);
		c_.headBase = ML::Box2D(0, 0, 0, 0);
		c_.fallSpeed = 0.0f;
		c_.jumpPow = 0.0f;
		c_.drawBase = ML::Box2D(-16, -16, 32, 32);
		c_.src = ML::Box2D(0, 0, 32, 32);
		c_.state = State::Normal;
		c_.motion = Motion::Move;
		c_.angle = Angle_LR::Right;
		c_.moveCnt = 0;
		c_.animCnt = 0;
	}

	void Enemy_UpDate(Chara& c_)
	{
		if (c_.state == State::Non) { return; }

		//플레이어와 접촉판정
		//플레이어와 접촉하고있을 때, 이하의 처리를 실행
		ML::Box2D me = c_.hitBase.OffsetCopy(int(c_.x), int(c_.y));
		if (player.state == State::Normal) {
			if (player.motion == Motion::Move) {
				ML::Box2D you = player.hitBase.OffsetCopy(int(player.x), int(player.y));
				if (you.Hit(me)) {
					player.motion = Motion::Dead;
					player.moveCnt = 0;
				}
			}
		}
		//무효화로부터 복귀
		if (c_.motion == Motion::Dead) {
			c_.moveCnt++;
			//복귀
			if (c_.moveCnt >= 0) {
				c_.motion = Motion::Move;
				int x = (int(c_.x) - 16) / 32;
				int y = (int(c_.y) - 16) / 32;
				mapData.arr[y][x] = -1;
			}
		}
	}

	void Enemy_Render(Chara& c_)
	{
		if (c_.state == State::Non) { return; }
		if (c_.motion == Motion::Dead) { return; }
		ML::Box2D draw = c_.drawBase.OffsetCopy(int(c_.x), int(c_.y));
		imgEnemy->Draw(draw, c_.src);
	}

	void Shot_Initialize(Chara& c_, int x_, int y_, Angle_LR d_)
	{
		c_.x = float(x_);
		c_.y = float(y_);
		c_.hitFlag = false;
		c_.hitBase = ML::Box2D(-16, -16, 32, 32);
		c_.footBase = ML::Box2D(0, 0, 0, 0);
		c_.headBase = ML::Box2D(0, 0, 0, 0);
		c_.drawBase = ML::Box2D(-16, -16, 32, 32);
		c_.src = ML::Box2D(0, 0, 32, 32);
		c_.state = State::Normal;
		c_.motion = Motion::Move;
		c_.angle = d_;
		c_.moveCnt = 0;
		c_.animCnt = 0;
		c_.fallSpeed = -3.6f;
		c_.jumpPow = 0;
	}

	void Shot_UpDate(Chara& c_)
	{
		if (c_.state == State::Non) { return; }
		if (c_.motion == Motion::Move) {
			ML::Vec2 est(0, 0);
			if (c_.angle == Angle_LR::Left) { est.x += -4.5f; }
			else { est.x += +4.5f; }
			est.y += c_.fallSpeed;
			//탄이 뚫지않은 이동은 필요하지 않음
			c_.x += est.x;
			c_.y += est.y;
			//벽과 겹쳐졌을 때 소멸
			ML::Box2D me = c_.hitBase.OffsetCopy(int(c_.x), int(c_.y));
			if (Map_CheckHit(mapData, me))
			{
				c_.state = State::Non;
			}
			//중력가속
			c_.fallSpeed += ML::Gravity(32) * 3;
		}
		//적이 맞은 판정
		if (c_.motion == Motion::Move) {
			ML::Box2D me = c_.hitBase.OffsetCopy(int(c_.x), int(c_.y));
			for (int i = 0; i < _countof(enemys); ++i) {
				if (enemys[i].state == State::Normal) {
					ML::Box2D you = enemys[i].hitBase.OffsetCopy(
						int(enemys[i].x), int(enemys[i].y)
					);
					if (you.Hit(me)) {
						//탄의 소멸
						c_.state = State::Non;
						//탄을 벽에 변환
						int x = (int(enemys[i].x) - 16) / 32;
						int y = (int(enemys[i].y) - 16) / 32;
						mapData.arr[y][x] = 20;
						//적은일정시간에복귀한다
						enemys[i].motion = Motion::Dead;
						enemys[i].moveCnt = -120;
					}
				}
			}
		}
	}

	void Shot_Render(Chara& c_)
	{
		if (c_.state == State::Non) { return; }
		ML::Box2D draw = c_.drawBase.OffsetCopy(int(c_.x), int(c_.y));
		imgShot->Draw(draw, c_.src);
	}

	void Shot_Appear(int x_, int y_, Angle_LR d_)
	{
		for (int i = 0; i < _countof(shots); ++i) {
			if (shots[i].state == State::Non) {
				Shot_Initialize(shots[i], x_, y_, d_);
			}
		}
	}
} // namespace Game
