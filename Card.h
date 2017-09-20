#pragma once
#include <SFML/Graphics.hpp>
#include "common_def.h"


class Card
{
public:
	Card();
	~Card();
	void update(sf::Time time);
	// 设定新的目标点并计算出速度
	void set_dest(sf::Vector2f p);
	void set_dispatch_speed();
	void set_pos(sf::Vector2f p);
	void update_pos();
	sf::Vector2f get_upon_pos();
	void show_face();
	void show_back();

	sf::Vector2f speed;
	sf::Vector2f pos;
	sf::Vector2f dest;
	bool moving;
	bool highlighted;

	int no;
	int month;
	bool earned;
	std::string name;
	CardType type;
	bool visible;
	sf::Sprite sprite;
};
