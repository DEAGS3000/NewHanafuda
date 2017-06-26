#pragma once
#include <list>
#include "Card.h"
#include "Player.h"
#include "Game.h"

using namespace std;

class Game;

class AI
{
public:
	list<Card*> enemy_earned;
	list<Card*> enemy_earned_light;
	list<Card*> enemy_earned_short;
	list<Card*> enemy_earned_seed;
	list<Card*> enemy_earned_skin;

	// 要获取的牌
	list<Card*> cards_to_earn;
	// 可选的出牌
	list<Card*> putable;
	// 注册为AI的玩家
	Player* p;

	AI(Player *);
	~AI();
	// 确定自己是否还有达成某个扎役的可能
	bool accomplishable(int win_type);
	// 同上，判断对方是否还有达成某个扎役的可能，用于拦截
	bool enemy_accomplishable(int win_type);
	// 获取某个特定扎役要达成所需的剩下的牌
	void get_rest_cards();
	// 获取场上的同月牌
	void get_same_month(list<Card*> field_cards);

	// 调用接口
	void calculate(list<Card*> &field_cards);

	// 选择一张手牌打出
	Card *select_put();
	// 选择一张场牌赢取
	Card *select_put_target();
	Card *select_draw_target(Card *drawn_card, list<Card*> &field_cards);
	bool determine_koikoi();
	// 确认赢取了某张牌
	void earned(Card *card);

	// 已经不可能获取到的牌（是否要包括自己已经得到的牌？）
	vector<Card*> impossible_cards;
	// 在已知范围内依然可以得到的牌
	vector<Card*> possible_cards;

	// 返回达成某个扎役所需要的得牌列表
	vector<Card*> check_five_light();
	vector<Card*> check_rain_four_light();
	vector<Card*> check_four_light();
	vector<Card*> check_pdb();
	vector<Card*> check_sbook();
	vector<Card*> check_rshort();
	vector<Card*> check_pshort();
	vector<Card*> check_seed();
	vector<Card*> check_skin();
	vector<Card*> check_fwine();
	vector<Card*> check_mwine();
};
