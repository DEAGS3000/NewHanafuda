#pragma once
#include "common_def.h"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "Card.h"

// 为了避免和common_def相互包含而放在这里

class ContentManager
{
public:
	sf::Texture background;
	sf::Texture card_texture[48];
	sf::Texture card_backend;
	sf::Texture selection_frame;
	sf::Texture highlight;
	sf::Texture parent_sign;
	sf::SoundBuffer bgm;
	sf::SoundBuffer put;
	sf::SoundBuffer slide;

	card_info cards[48];
	win_info wins[14];


	ContentManager();
	~ContentManager();
	void load_cards();
	void load_wins();
	void load_texture();
	void load_audio();
};
