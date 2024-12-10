#include "MyPG.h"
#include "MyGameMain.h"

// ���� ���ӽ����̽�
namespace Game
{
	// �̹��� ������
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
		State state; //ĳ������ ����
		Motion motion; //ĳ������ �ڼ�
		Angle_LR angle; //ĳ������ ����
		int moveCnt; //�ൿó����ī����
		int animCnt; //�ִϸ��̼�ó����ī����
		ML::Box2D headBase; //�Ӹ�������
	};

	MapData mapData;
	void Map_Initialize(MapData& md_);
	bool Map_Load(MapData& md_, int n_);
	void Map_UpData(MapData& md_);
	void Map_Render(MapData& md_);
	bool Map_CheckHit(MapData& md_, const ML::Box2D& h_);
	void Map_CreateSGE(MapData& md_); // ����, ��ǥ, ��

	Chara player;
	void Player_Initialize(Chara& c_, int x_, int y_);
	void Player_UpDate(Chara& c_);
	void Player_Render(Chara& c_);
	void Chara_CheckMove(Chara& c_, ML::Vec2& e_);
	bool Chara_CheckFoot(Chara& c_);
	void Player_Anim(Chara& c_);
	bool Chara_CheckHead(Chara& c_);
	TaskFlag Check_State();
	//������,���������ΰ�
	int playerStock; //���
	int stageNum; //����������ȣ

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
	Chara enemys[30 * 17]; //�ϴ� �ִ���� �غ�
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
	// �ʱ�ȭ
	// �� �Լ��� ���� ȯ���� �����ϰ� ���ҽ��� �ε��մϴ�
	//-----------------------------------------------------------------------------
	void Initialize()
	{
		// ���� ����
		ge->dgi->EffectState().param.bgColor = ML::Color(1, 0, 1, 0);

		imgMapChip = DG::Image::Create("./data/image/MapSetting_org.png");
		imgPlayer = DG::Image::Create("./data/image/all.png");
		imgBG = DG::Image::Create("./data/image/back.png");
		imgGoal = DG::Image::Create("./data/image/Goal.png"); //������������ �ҷ�����
		imgStart = DG::Image::Create("./data/image/start.png"); //��ŸƮ���ϸ�
		imgEnemy = DG::Image::Create("./data/image/Enemy.png"); //�� ���ϸ�
		imgShot = DG::Image::Create("./data/image/Shot.png");

		
		// �� �ʱ�ȭ
		Map_Initialize(mapData);
		
		// �� ������ �ε�
		Map_Load(mapData, 1);
		// ���� �ʱ�ȭ
		for (int i = 0; i < _countof(enemys); ++i) {
			enemys[i].state = State::Non;
		}
		//ź�� ��ȿȭ
		for (int i = 0; i < _countof(shots); ++i) {
			shots[i].state = State::Non;
		}

		// ���� ������ ��ǥ ���� ����
		Map_CreateSGE(mapData); // ���⿡ �߰�
	
		// �÷��̾� �ʱ�ȭ
		Player_Initialize(player, int(start.x), int(start.y));
		

	}
	//-----------------------------------------------------------------------------
	// ����
	// �� �Լ��� ���� ���� ���� ���ҽ��� �����մϴ�
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
	// ������Ʈ
	// �� �Լ��� ���� ���¸� ������Ʈ�ϰ� �Է��� ó���մϴ�
	//-----------------------------------------------------------------------------
	TaskFlag UpDate()
	{
		auto inp = ge->in1->GetState();
		Map_UpData(mapData);
		// �� ������Ʈ
		for (int i = 0; i < _countof(enemys); ++i) {
			Enemy_UpDate(enemys[i]); // �� ������Ʈ ȣ��
		}
		//ź ������Ʈ
		for (int i = 0; i < _countof(shots); ++i) {
			Shot_UpDate(shots[i]); // �� ������Ʈ ȣ��
		}
		// �÷��̾� ������Ʈ
		Player_UpDate(player);

		// �� ������Ʈ
		Goal_UpDate(goal);

		// ��ŸƮ ������Ʈ
		Start_UpDate(start);
		TaskFlag rtv = Check_State();
		return rtv;

	}
	//-----------------------------------------------------------------------------
	// ������
	// �� �Լ��� ȭ�鿡 ���� ��Ҹ� �������մϴ�
	//-----------------------------------------------------------------------------
	void Render()
	{
		Map_Render(mapData);
		//�� ������
		for (int i = 0; i < _countof(enemys); ++i) {
			Enemy_Render(enemys[i]); // �� ������ ȣ��
		}
		//ź ������
		for (int i = 0; i < _countof(shots); ++i) {
			Shot_Render(shots[i]); // ź ������ ȣ��
		}
		// �÷��̾� ������
		Player_Render(player);
		
		// �� ������
		Goal_Render(goal);

		//��ŸƮ ������
		Start_Render(start);
	}
	//-----------------------------------------------------------------------------
	// ���� Ȯ��
	// ���� ���¸� üũ�ϰ� ������ �½�ũ �÷��׸� ��ȯ�մϴ�
	//-----------------------------------------------------------------------------
	TaskFlag Check_State()
	{
		auto inp = ge->in1->GetState();
		// Ÿ��Ʋ�� ��ȯ
		if (inp.ST.down) {
			return TaskFlag::Title;
		}
		//�÷��̾ �����
		if (player.state != State::Non) {
			//���ó��
			if (player.motion == Motion::Dead && player.moveCnt > 120) {
				playerStock--;
				if (playerStock > 0) {
					return TaskFlag::StageLogo; //���� ���������� ����
				}
				else {
					return TaskFlag::Title; //Ÿ��Ʋ�� ���ư���
				}
			}
			//��������Ŭ���� ����
			if (player.motion == Motion::Happy && player.moveCnt > 120) {
				stageNum++;
				if (stageNum <= 10) {
					return TaskFlag::StageLogo; //���� ���������� �̵�
				}
				else {
					return TaskFlag::Ending; //��������
				}
			}
		}
		// ���� ���� ����
		return TaskFlag::Game;
	}
	//-----------------------------------------------------------------------------
	void Map_Initialize(MapData& md_)
	{
		// �� ������ �ʱ�ȭ
		for (int y = 0; y < 17; ++y) {
			for (int x = 0; x < 30; ++x) {
				md_.arr[y][x] = 0;
			}
		}
		// �� Ĩ �ʱ�ȭ
		for (int c = 0; c < 19 * 6; ++c) {
			md_.chip[c] = ML::Box2D((c % 19) * 32, (c / 18) * 32, 32, 32);
		}
	}
	//-----------------------------------------------------------------------------
	bool Map_Load(MapData& md_, int n_)
	{
		// ���� ��� ����
		string filePath = "./data/map/mapdata" + to_string(n_) + ".csv";
		// ���� ����
		ifstream fin(filePath);
		if (!fin) { return false; } // ���� ���� ����

		// CSV ������ �б�
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

		// ���� �ݱ�
		fin.close();
		return true;
	}
	//-----------------------------------------------------------------------------
	void Map_CreateSGE(MapData& md_) {
		int eneCnt = 0;
		for (int y = 0; y < 17; ++y) {
			for (int x = 0; x < 30; ++x) {
				// �������߰�
				if (md_.arr[y][x] == 39) {
					Goal_Initialize(goal, x * 32 + 16, y * 32 + 16);
					md_.arr[y][x] = -1;
				}
				//��ŸƮ����
				if (md_.arr[y][x] == 38) {
					Start_Initialize(start, x * 32 + 16, y * 32 + 16);
					md_.arr[y][x] = -1;
				}
				//������
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
		// �� ������ ������Ʈ
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
					int num = md_.arr[y][x]; // �� Ĩ ��ȣ
					ML::Box2D src = md_.chip[num]; // �� Ĩ�� �ҽ�
					imgMapChip->Draw(draw, src);
				}
			}
		}
	}

	//-----------------------------------------------------------------------------
	bool Map_CheckHit(MapData& md_, const ML::Box2D& h_)
	{
		ML::Rect r = { h_.x, h_.y, h_.x + h_.w, h_.y + h_.h };
		// �� ��� �˻�
		ML::Rect m = { 0, 0, 32 * 30, 32 * 17 };
		if (r.left < m.left) { r.left = m.left; }
		if (r.top < m.top) { r.top = m.top; }
		if (r.right > m.right) { r.right = m.right; }
		if (r.bottom > m.bottom) { r.bottom = m.bottom; }

		// �浹 üũ
		int sx, sy, ex, ey;
		sx = r.left / 32;
		sy = r.top / 32;
		ex = (r.right - 1) / 32;
		ey = (r.bottom - 1) / 32;

		// �� �����Ϳ� �浹 �˻�
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
				if (true == c_.hitFlag) { // ���� ó��
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

		// �� �Ʒ� �浹 üũ
		c_.hitFlag = Chara_CheckFoot(c_);
		if (true == c_.hitFlag) {
			c_.fallSpeed = 0; // ���� �ӵ� �ʱ�ȭ
		}
		else {
			c_.fallSpeed += ML::Gravity(32) * 3; // �߷� ����
		}

		//�Ӹ��� ����
		if (c_.fallSpeed < 0) { //�����
			if (Chara_CheckHead(c_)) {
				c_.fallSpeed = 0; // ���� �ӵ� �ʱ�ȭ
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
		c_.hitFlag = false; // �̵� �� �浹 �÷��� �ʱ�ȭ

		// ���� �̵� ó��
		while (e_.x != 0) {
			float preX = c_.x;
			if (e_.x >= 1) { c_.x += 1; e_.x -= 1; }
			else if (e_.x <= -1) { c_.x -= 1; e_.x += 1; }
			else { c_.x += e_.x; e_.x = 0; }
			ML::Box2D hit = c_.hitBase.OffsetCopy(c_.x, c_.y);
			if (true == Map_CheckHit(mapData, hit)) {
				c_.x = preX; // �浹 �� ���� ��ġ�� �ǵ�����
				break;
			}
		}
		// ���� �̵� ó��
		while (e_.y != 0) {
			float preY = c_.y;
			if (e_.y >= 1) { c_.y += 1; e_.y -= 1; }
			else if (e_.y <= -1) { c_.y -= 1; e_.y += 1; }
			else { c_.y += e_.y; e_.y = 0; }
			ML::Box2D hit = c_.hitBase.OffsetCopy(c_.x, c_.y);
			if (true == Map_CheckHit(mapData, hit)) {
				c_.y = preY; // �浹 �� ���� ��ġ�� �ǵ�����
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

	// �÷��̾� �ִϸ��̼�
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
		//���ʹ������
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
			c_.fallSpeed = 0; // �浹 �� ���� �ӵ� �ʱ�ȭ
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
		//�÷��̾��� ��������
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
		//��ŸƮ������ ��� �޸�, Ư���� ó������ ����
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

		//�÷��̾�� ��������
		//�÷��̾�� �����ϰ����� ��, ������ ó���� ����
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
		//��ȿȭ�κ��� ����
		if (c_.motion == Motion::Dead) {
			c_.moveCnt++;
			//����
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
			//ź�� �������� �̵��� �ʿ����� ����
			c_.x += est.x;
			c_.y += est.y;
			//���� �������� �� �Ҹ�
			ML::Box2D me = c_.hitBase.OffsetCopy(int(c_.x), int(c_.y));
			if (Map_CheckHit(mapData, me))
			{
				c_.state = State::Non;
			}
			//�߷°���
			c_.fallSpeed += ML::Gravity(32) * 3;
		}
		//���� ���� ����
		if (c_.motion == Motion::Move) {
			ML::Box2D me = c_.hitBase.OffsetCopy(int(c_.x), int(c_.y));
			for (int i = 0; i < _countof(enemys); ++i) {
				if (enemys[i].state == State::Normal) {
					ML::Box2D you = enemys[i].hitBase.OffsetCopy(
						int(enemys[i].x), int(enemys[i].y)
					);
					if (you.Hit(me)) {
						//ź�� �Ҹ�
						c_.state = State::Non;
						//ź�� ���� ��ȯ
						int x = (int(enemys[i].x) - 16) / 32;
						int y = (int(enemys[i].y) - 16) / 32;
						mapData.arr[y][x] = 20;
						//���������ð��������Ѵ�
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
