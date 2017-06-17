#include "ai.h"

AI::AI(Player *p)
{
	this->p = p;
}

void AI::get_same_month(list<Card*> field_cards)
{
	for(auto f_card : field_cards)
	{
		for(auto h_card : p->hand_cards)
		{
			// 场牌的位有可能为空，要检验指针的有效性
			if(f_card && f_card->month==h_card->month)
			{
				cards_needed.push_back(f_card);
				putable.push_back(h_card);
				return;
			}
		}
	}
}

void AI::calculate(list<Card*> field_cards)
{
	get_same_month(field_cards);
}

Card* AI::select_put()
{
	Card *temp = putable.front();
	putable.pop_front();
	return temp;
}
