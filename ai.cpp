#include "ai.h"
#include "utilities.h"
#include <iostream>

AI::AI(Player *p)
{
	this->p = p;
}

void AI::get_same_month(list<Card*> field_cards)
{

	// TODO: 走了几步以后，这里会出现iterator not dereferenceable
	for(auto h_card : p->hand_cards)
	{
		for(auto f_card : field_cards)
		{
			// 场牌的位有可能为空，要检验指针的有效性
			if(f_card && f_card->month==h_card->month)
			{
				cards_to_earn.push_back(f_card);
				putable.push_back(h_card);
				return;
			}
		}
	}
	// 走到这里说明场上没有可选的同月牌，就出手牌第一张
	putable.push_back(p->hand_cards.front());
}

void AI::calculate(list<Card*> &field_cards)
{
	get_same_month(field_cards);
}

Card* AI::select_put()
{
	Card *temp = putable.front();
	putable.pop_front();
	//cards_to_earn.pop_front();
	return temp;
}

Card* AI::select_put_target()
{
	Card *temp = cards_to_earn.front();
	//cards_to_earn.pop_front();
	return temp;
}

Card* AI::select_draw_target(Card* drawn_card, list<Card*> &field_cards)
{
	for(auto card : field_cards)
	{
		if (card && card->month == drawn_card->month)
			return card;
	}
	std::cout << "找不到可用的场牌！" << endl;
	return nullptr;
}

bool AI::determine_koikoi()
{
	if (p->hand_cards.size() > 3)
		return true;
	else
		return false;
}

void AI::earned(Card* card)
{
	remove_item(cards_to_earn, card);
}
