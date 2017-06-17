#pragma once
#include <cstdlib>
#include <list>
#include <SFML/Graphics.hpp>
#include "Card.h"

int random(int a, int b);

int switch_from_list(std::list<int> &l);

bool in_list(std::list<Card*> &l, Card *c);

bool remove_item(std::list<Card*> &l, Card *c);

bool null_item(std::list<Card*> &l, Card *c);
