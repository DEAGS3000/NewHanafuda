#pragma once
#include <SFML/Graphics.hpp>
#include "common_def.h"
#include <deque>
#include <list>
#include <map>
#include "external_declare.h"
#include "Card.h"
#include "Player.h"
#include "ai.h"
#include "PhaseState.h"
#include "Timer.h"

#ifdef _WIN32
#include <windows.h>
#endif
#ifdef __APPLE__
#include <time.h>
#endif

class AI;
class PhaseState;

class Game
{
public:
	sf::RenderWindow window;
	sf::Clock delta_clock;
	sf::Texture *font_texture;
	sf::Sprite background;
	sf::Sprite card_backend;
	sf::Sprite sprite_heap;
	sf::Sprite highlight;
	sf::Sprite parent_sign;
	sf::Sound bgm;
	sf::Sound sound_put;
	sf::Sound sound_slide;
	// 当前流程的文字版
	std::string flow_state_str;
	unsigned long tick_count;
	// 当前月份
	int current_month;
	// 游戏状态，主菜单、暂停等
	GameState game_state;
	// 流程状态
	FlowState flow_state;
	// 流程队列，应当不能为空
	std::deque<FlowState> flow_queue;
	// 正在移动的卡牌的队列，渲染用
	std::deque<Card *> moving_cards;
	// 场牌列表，因为场牌被收走后不对场地进行整理，所以定长数组合适
	std::list<Card *> field_cards;
	//card_info all_cards[48];
	std::list<Card *> heap;
	Card all_cards[48];
	// 用于实现渲染先后顺序的列表
	std::list<Card *> render_list;
	// 得牌列表
	std::list<Card *> earned_cards;

	// 两个玩家的回合队列。对每个玩家来说，front是自己，back是对方，自己回合结束时将自己再push_back
	std::deque<Player *> player_queue;
	Player *p1, *p2, *parent;
	AI *ai;

	bool l_button_clicked;
	bool selected_koikoi;
	bool selected_end;
	// 用于判断卡牌是否刚开始移动，以决定是否播放声音
	Card *current_moving_card;
	// 玩家鼠标点击的位置
	sf::Vector2f l_button_pos;

	// 从场牌抽出的牌，当场牌中有两张与之同月时用
	Card *put_card;
	Card *drawn_card;

	sf::Time interval_waited;

	std::deque<PhaseState*> state_queue;
	std::map<FlowState, PhaseState *> state_dict;

	Timer timer;


	Game();
	Game(card_info c);
	~Game();
	void loop();

	void update(sf::Time time);
	void render_playing();
	void render_main_menu();
	void display();

	// 重置游戏，包括牌堆、手牌、得牌、场牌、扎役以及卡牌的可见性和位置、速度
	void reset();
	// 洗牌
	void shuffle();
	// 从牌堆抽牌
	Card *draw_card();
	// 逻辑上的出牌操作
	void put(Card *card);
	// 逻辑上的选牌操作
	void select_target(Card *card);
	void select_put_target(Card *card);
	void select_draw_target(Card *card);
	void update_gui_playing();
	void update_gui_main_menu();
	void update_koikoi_gui();
	void new_game();
	// 将牌打到场地上
	void put_to_field(Card *card);
	// 获得场牌位置
	static sf::Vector2f get_field_pos(int index);

	// 统计场牌中有几张与打出的牌同月
	int count_same_month(int m);

	bool move_cards(sf::Time time);

	Card *get_point_card(std::list<Card *> l);

	// 流程函数
	void flow_prepare();
	void flow_validate_game();
	void flow_precomplete();
	void flow_put();
	void flow_draw();
	void flow_select_put_target();
	void flow_select_draw_target();
	void flow_detect_win();
	void flow_koikoi();
	void flow_summary();

	void flow_log(string str);
	void save_game();
	void load_game();

	void switch_state(FlowState fs, float in_seconds=0.0f);
	void enqueue_state(FlowState fs);
	void dequeue_state();

	// 单例模式
	static Game *
	Instance()
	{
		if (_instance == nullptr)
			_instance = new Game;
		return _instance;
	}

    void put_ex(Card *card);

    void select_put_target_ex(Card *card);

private:
	static Game *_instance;

};
