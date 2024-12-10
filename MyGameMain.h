#pragma once
#include "MyPG.h"

extern  void  MyGameMain_Finalize( );
extern  void  MyGameMain_Initialize( );
extern  void  MyGameMain_UpDate( );
extern  void  MyGameMain_Render2D( );

enum class TaskFlag
{
	Non,		//?スクが未選択状態
	Title,		//?イト
	NewGame,
	StageLogo,
	Game,		//ゲ???編
	Ending,		//エンディング
	
	//以下必要に応じて追加
};
