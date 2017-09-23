﻿#include "Player.h"
#include "common_def.h"

Player::Player()
{
	upside = false;
}

Player::~Player()
{
}

void Player::get(Card* card)
{
	//card->visible = true;
	card->moving = true;
	card->set_dest(get_hand_pos());
	// 真正加入手牌的操作要放到后面，不然get_hand_pos要出错。那个是基于本次get之前的状态做才对
	hand_cards.push_back(card);
}

void Player::earn(Card* card, int month)
{
	card->moving = true;

	switch (card->type)
	{
	case ct_light:
		earned_light.push_back(card);
		break;
	case ct_short:
		earned_short.push_back(card);
		break;
	case ct_seed:
		earned_seed.push_back(card);
		break;
	case ct_skin:
		earned_skin.push_back(card);
		break;
	}

	// 加入相应的扎役完成列表
	if (card->type == ct_light)
		win_light.push_back(card);
	if (card->type == ct_short)
	{
		win_sbook.push_back(card);
	}
	if (card->type == ct_seed)
	{
		win_seed.push_back(card);
	}
	if (card->type == ct_skin)
	{
		win_skin.push_back(card);
	}

	// TODO: 上面和下面的部分会重复，解决一下
	if (card->name == "芒上月" || card->name == "菊上杯")
		win_mwine.push_back(card);
	if (card->name == "樱上幕帘" || card->name == "菊上杯")
		win_fwine.push_back(card);
	if (card->name == "荻间猪" || card->name == "枫间鹿" || card->name == "牡丹蝶")
		win_pdb.push_back(card);
	if (card->name == "松赤短" || card->name == "梅赤短" || card->name == "樱赤短")
		win_reds.push_back(card);
	if (card->name == "牡丹青短" || card->name == "菊上青短" || card->name == "枫上青短")
		win_purples.push_back(card);
	if (card->month == month)
		win_monthcards.push_back(card);
}

void Player::reset()
{
	hand_cards.clear();
	earned_light.clear();
	earned_short.clear();
	earned_seed.clear();
	earned_skin.clear();
	win_light.clear();
	win_pdb.clear();
	win_reds.clear();
	win_purples.clear();
	win_sbook.clear();
	win_skin.clear();
	win_seed.clear();
	win_fwine.clear();
	win_mwine.clear();
	win_monthcards.clear();
	for (int i = 0; i < 14; ++i)
		earned_wins[i] = false;
	last_koikoi_sbook_length = 0;
	last_koikoi_seed_length = 0;
	last_koikoi_skin_length = 0;
}

void Player::format_cards(std::list<Card*> &earned_cards)
{
	// 建立上级列表
	list<list<Card*>*> all_earned_card_lists;
	if (earned_light.size())
		all_earned_card_lists.push_back(&earned_light);
	if (earned_short.size())
		all_earned_card_lists.push_back(&earned_short);
	if (earned_seed.size())
		all_earned_card_lists.push_back(&earned_seed);
	if (earned_skin.size())
		all_earned_card_lists.push_back(&earned_skin);


	// 或者，可以在渲染移动结束之前就把得牌加入相应的列表，但是moving不改。
	// 在格式化卡片的时候，这样数量正确了，可以算出对的位置，但是有moving属性的不直接修改位置
	// 可是这样好像又用不到moving了，直接根据earned_cards就可以更新所持有的得牌区体积

	// 由于传入了earned_cards，本函数可以在没有将新的得牌真正加入相应列表的情况下
	// 对已经在各列表中的卡牌的位置进行更新
	// 算了，还是没moving的直接设置新的pos，有moving的设置dest好了

	// 由于要算新得牌加入后的位置
	float hand_length = CARD_SIZE_X * hand_cards.size() + BLANK_SIZE;

	// 先检查offset是否需要压缩。如果不需要，直接使用offset的上限值。
	int earned_length = 0;
	for (list<list<Card*>*>::iterator it = all_earned_card_lists.begin(); it != all_earned_card_lists.end(); ++it)
	{
		earned_length += CARD_SIZE_X + ((*it)->size() - 1) * CARD_OFFSET_LIMIT;
		earned_length += BLANK_SIZE;
	}

	float available_length = WINDOW_WIDTH - hand_length;

	float offset = CARD_OFFSET_LIMIT;
	// 结果为真，说明要压缩。
	if (available_length < earned_length)
	{
		float net_size = available_length - (all_earned_card_lists.size()-1)*BLANK_SIZE;
		float temp = 0;
		for (list<list<Card*>*>::iterator it = all_earned_card_lists.begin(); it != all_earned_card_lists.end(); ++it)
		{
			temp += (*it)->size() - 1;
		}
		// 将几张完全显示出来的卡宽度减去
		offset = (net_size - all_earned_card_lists.size()*CARD_SIZE_X) / temp;
	}

	// 得出Y轴坐标
	float pos_y, pos_x;

	// 设置卡片位置的时候从右往左设置，渲染从左往右，没毛病
	pos_x = WINDOW_WIDTH;
	if (upside)
		pos_y = 0;
	else
		pos_y = WINDOW_HEIGHT - CARD_SIZE_Y;

	for (list<list<Card*>*>::reverse_iterator l = all_earned_card_lists.rbegin(); l != all_earned_card_lists.rend(); ++l)
	{
		for (list<Card*>::reverse_iterator it = (*l)->rbegin(); it != (*l)->rend(); ++it)
		{
			// 每个列表最后一张特殊处理
			if ((*it) == (*l)->back())
				pos_x -= CARD_SIZE_X;
			else
				pos_x -= offset;

			if ((*it)->moving)
				(*it)->set_dest({ pos_x, pos_y });
			else
				(*it)->set_pos({ pos_x, pos_y });
		}
		// 设置列表间间隔
		pos_x -= BLANK_SIZE;
	}

		// 重排手牌
	float i = 0;
	for (std::list<Card*>::iterator it = hand_cards.begin(); it != hand_cards.end(); ++it, ++i)
	{
		(*it)->set_pos({ i*CARD_SIZE_X, pos_y });
	}
}

sf::Vector2f Player::get_hand_pos()
{
	float temp_y = 0;
	int a = hand_cards.size();
	if (!upside)
		temp_y = WINDOW_HEIGHT - CARD_SIZE_Y;
	sf::Vector2f result = { hand_cards.size()*CARD_SIZE_X, temp_y };
	return result;
}

sf::Vector2f Player::get_parent_sign_pos()
{
	float pos_x, pos_y;
	if (upside)
		pos_y = CARD_SIZE_Y + BLANK_SIZE;
	else
		pos_y = WINDOW_HEIGHT - CARD_SIZE_Y - BLANK_SIZE - PARENT_SIGN_SIZE*ZOOM_LEVEL;

	pos_x = CARD_SIZE_X * 8 + BLANK_SIZE;

	return{ pos_x, pos_y };
}

int Player::sbook_extra()
{
	return win_sbook.size() - 5;
}

int Player::seed_extra()
{
	return win_seed.size() - 5;
}

int Player::skin_extra()
{
	return win_skin.size() - 10;
}
