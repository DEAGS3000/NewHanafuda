#include "ContentManager.h"
#include "extern_declare.h"
#include <fstream>


ContentManager::ContentManager()
{
	load_cards();
	load_wins();
	load_texture();
	load_audio();
}

ContentManager::~ContentManager()
{
}

void ContentManager::load_cards()
{
	// 注释掉的代码无论如何读不进来东西
	/*FILE *file;
	_wfopen_s(&file, L"res/cards.txt", L"r");

	wchar_t temp_name[20];
	for(int i=0; i<48; ++i)
	{
		memset(temp_name, 0, sizeof(wchar_t) * 20);
		fwscanf_s(file, L"%d %d %s %d\n", cm.cards[i].no, cm.cards[i].month, temp_name, cm.cards[i].type);
		cm.cards[i].name = temp_name;
	}
	fclose(file);*/
	// 用普通流读ANSI可以，用宽字符流读UTF-8不可以，总之wstring没法用
	std::ifstream ifs;
	ifs.open(L"res/cards.txt");
	int temp_type;
	for(int i=0; i<48; ++i)
	{
		ifs >> cm.cards[i].no >> cm.cards[i].month >> cm.cards[i].name >> temp_type;
		switch(temp_type)
		{
		case 0:
			cm.cards[i].type = ct_light;
			break;
		case 1:
			cm.cards[i].type = ct_short;
			break;
		case 2:
			cm.cards[i].type = ct_seed;
			break;
		case 3:
			cm.cards[i].type = ct_skin;
			break;
		}
	}
	//ANSIToUnicode
	ifs.close();
}

void ContentManager::load_wins()
{
	/*FILE *file;
	_wfopen_s(&file, L"res/win.txt", L"r,ccs=utf-8");
	for(int i=0; i<48; ++i)
	{
		fscanf_s(file, "%s %d\n", wins[i].name, wins[i].money);
	}
	fclose(file);*/

	/*std::ifstream ifs;
	ifs.open(L"res/win.txt");
	for (int i = 0; i < 48; ++i)
	{
		ifs >> wins[i].name >> wins[i].money;
	}
	ifs.close();*/

	// 先硬编码解决
	// 这里用u8前缀，虽然在debug时看到的内容是乱码，但是在使用的时候是正常的。
	wins[0].name = u8"五光";
	wins[0].money = 15;
	wins[1].name = u8"四光";
	wins[1].money = 10;
	wins[2].name = u8"雨四光";
	wins[2].money = 8;
	wins[3].name = u8"三光";
	wins[3].money = 6;
	wins[4].name = u8"猪鹿蝶";
	wins[4].money = 5;
	wins[5].name = u8"赤短";
	wins[5].money = 6;
	wins[6].name = u8"青短";
	wins[6].money = 6;
	wins[7].name = u8"花见酒";
	wins[7].money = 3;
	wins[8].name = u8"月见酒";
	wins[8].money = 3;
	wins[9].name = u8"短册";
	wins[9].money = 1;
	wins[10].name = u8"种";
	wins[10].money = 1;
	wins[11].name = u8"皮";
	wins[11].money = 1;
	wins[12].name = u8"月札";
	wins[12].money = 4;
	wins[13].name = u8"手四";
	wins[13].money = 6;
}

void ContentManager::load_texture()
{
	card_backend.loadFromFile("res/back.png");
	card_backend.setRepeated(true);
	background.loadFromFile("res/bg.bmp");
	background.setRepeated(true);
	selection_frame.loadFromFile("res/rect.png");
	highlight.loadFromFile("res/highlight.png");
	parent_sign.loadFromFile("res/parent_sign.png");


	char temp_name[20];
	for(int i=0; i<48; ++i)
	{
		memset(temp_name, 0, sizeof(char) * 20);
		sprintf_s(temp_name, "res/cards/%d.jpg", i);
		card_texture[i].loadFromFile(temp_name);
		card_texture[i].setSmooth(true);
	}
}

void ContentManager::load_audio()
{
	bgm.loadFromFile("res/bgm.ogg");
	put.loadFromFile("res/put.wav");
	slide.loadFromFile("res/slide.wav");
}
