#pragma once
#include <list>
#include "Card.h"
#include "Player.h"

using namespace std;

class AI
{
public:
	Player* p;
	list<Card*> enemy_earned;
	list<Card*> enemy_earned_light;
	list<Card*> enemy_earned_short;
	list<Card*> enemy_earned_seed;
	list<Card*> enemy_earned_skin;

	// 要获取的牌
	list<Card*> cards_needed;
	// 可选的出牌
	list<Card*> putable;
	// 注册为AI的玩家

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
	void calculate(list<Card*> field_cards);

	// 选择一张手牌打出
	Card *select_put();
	// 选择一张场牌赢取
	Card *select_target();
};
