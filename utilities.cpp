#include "utilities.h"
#include <iostream>

int random(int a, int b)
{
	return rand() % (b - a + 1) + a;
}

int switch_from_list(std::list<int>& l)
{
	if(l.empty()) std::cout << "�б��ѿգ��޷�ѡȡ��" << std::endl;

	int selected_index = random(0, l.size()-1);
	std::list<int>::iterator it = l.begin();
	//++it; ����Ҫ������ѭ����������λ���е�һ��++
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
	if (l.empty()) std::cout << "�б��ѿգ��޷�ɾ����" << std::endl;
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
