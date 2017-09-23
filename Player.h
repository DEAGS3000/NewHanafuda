#pragma once
#include "Card.h"
#include <list>

using namespace std;

class Player
{
public:
	Player();
	~Player();
	std::list<Card*> hand_cards;
	std::list<Card*> earned_light;
	std::list<Card*> earned_short;
	std::list<Card*> earned_seed;
	std::list<Card*> earned_skin;
	bool earned_wins[14];
	//int inrementable_lens[3];
	int last_koikoi_sbook_length;
	int last_koikoi_seed_length;
	int last_koikoi_skin_length;
	bool upside;

	std::list<Card*> win_light;
	std::list<Card*> win_pdb;
	std::list<Card*> win_reds;
	std::list<Card*> win_purples;
	std::list<Card*> win_sbook;
	std::list<Card*> win_skin;
	std::list<Card*> win_seed;
	std::list<Card*> win_fwine;
	std::list<Card*> win_mwine;
	std::list<Card*> win_monthcards;

	void get(Card *card);
	void earn(Card *card, int month);
	void reset();
	void format_cards();
	sf::Vector2f get_hand_pos();
	sf::Vector2f get_parent_sign_pos();
	// 获取增长型扎役的额外完成度
	int sbook_extra();
	int seed_extra();
	int skin_extra();
	int money;

	// 额外记录的得牌区域渲染位置信息
	/*int earned_left_edge;
	int earned_light_origin;
	int earned_short_origin;
	int earned_seed_origin;
	int earned_skin_origin;
	int earned_light_offset;
	int earned_short_offset;
	int earned_seed_offset;
	int earned_skin_offset;*/
	// 配套的提前更新
};
