#include "utilities.h"
#include <iostream>

int random(int a, int b)
{
	return std::rand() % (b - a + 1) + a;
}

int switch_from_list(std::list<int>& l)
{
	if(l.empty()) std::cout << "列表已空！无法选取！" << std::endl;

	int selected_index = random(0, l.size()-1);
	std::list<int>::iterator it = l.begin();
	//++it; 不需要，下面循环里无论如何会进行第一次++
	for (int i = 0; i < selected_index; ++i)
		++it;
	int result = *it;
	l.erase(it);
	return result;
}

bool in_list(std::list<Card*>& l, Card* c)
{
	for(std::list<Card*>::iterator it=l.begin(); it!=l.end(); ++it)
	{
		if ((*it) == c)
			return true;
	}
	return false;
}

bool remove_item(std::list<Card*>& l, Card* c)
{
	if (l.empty()) std::cout << "列表已空！无法删除！" << std::endl;
	std::list<Card*>::iterator it = l.begin();

	for(it=l.begin(); it!=l.end(); )
	{
		if ((*it) == c)
		{
			l.erase(it);
			return true;
		}
		else
			++it;
	}

	return false;
}

bool null_item(std::list<Card*>& l, Card* c)
{
	for (std::list<Card*>::iterator it = l.begin(); it != l.end(); ++it)
	{
		if ((*it) == c)
		{
			(*it) = nullptr;
			return true;
		}
	}
	return false;
}
