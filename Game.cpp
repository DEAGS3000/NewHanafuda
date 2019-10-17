#include "Game.h"
#include <imgui-SFML.h>
#include <imgui.h>
#include <iostream>
#include "utilities.h"
#ifdef _WIN32
#include <windows.h>
#endif
#include <fstream>

using namespace std;

Game *Game::_instance = nullptr;

Game::Game() : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), L"花札", sf::Style::Default)
{
// 置随机种子
#ifdef _WIN32
	tick_count = GetTickCount();
#endif
#ifdef __APPLE__
	tick_count = (unsigned)time(NULL);
#endif

	srand((unsigned int)tick_count);

	window.setVerticalSyncEnabled(true);

	// 绑定ImGui
	ImGui::SFML::Init(window, false);

	ImGui::GetStyle().ScaleAllSizes(3.0f);
	// 设置ImGui字体
	ImGuiIO &io = ImGui::GetIO();
	io.Fonts->Clear();
	io.Fonts->AddFontFromFileTTF("res/xarialuni.ttf", 8.0f * 3.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());

	ImGui::SFML::UpdateFontTexture(); // important call: updates font texture
	// // 需要先把默认的clear掉
	// io.Fonts->ClearFonts();
	// io.Fonts->AddFontFromFileTTF("res/xarialuni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
	// // 这里必须使用指针去new，否则生命周期限于构造函数，会导致字体变成方块
	// font_texture = new sf::Texture;
	// //ImGui::SFML::GetFontTexture().loadFromFile
	// ImGui::SFML::createFontTexture(*font_texture);
	// ImGui::SFML::setFontTexture(*font_texture);

	/*test_texture.loadFromFile("0.jpg");
	test_sprite.setTexture(test_texture);
	position = { 0, 0 };
	test_sprite.setPosition(position);
	speed = { destination.x / 0.1f, destination.y / 0.1f };*/

	//background_texture.loadFromFile("res/bg.bmp");
	//background_texture.setRepeated(true);
	background.setTexture(cm.background);
	sf::Rect<int> temp = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
	background.setTextureRect(temp);
	card_backend.setTexture(cm.card_backend);
	card_backend.setScale(ZOOM_LEVEL, ZOOM_LEVEL);
	sprite_heap.setTexture(cm.heap);
	sprite_heap.setScale(ZOOM_LEVEL, ZOOM_LEVEL);
	sprite_heap.setPosition(HEAP_POS_X, HEAP_POS_Y);
	highlight.setTexture(cm.highlight);
	highlight.setScale(ZOOM_LEVEL, ZOOM_LEVEL);
	parent_sign.setTexture(cm.parent_sign);
	parent_sign.setScale(ZOOM_LEVEL, ZOOM_LEVEL);
	bgm.setBuffer(cm.bgm);
	bgm.setLoop(true);
	bgm.setVolume(70);
	sound_put.setBuffer(cm.put);
	sound_slide.setBuffer(cm.slide);

	// 对信息和图像结合的Card对象进行初始化
	for (int i = 0; i < 48; ++i)
	{
		all_cards[i].no = cm.cards[i].no;
		all_cards[i].name = cm.cards[i].name;
		all_cards[i].month = cm.cards[i].month;
		all_cards[i].type = cm.cards[i].type;
		all_cards[i].sprite.setTexture(cm.card_texture[i]);
		all_cards[i].sprite.setScale(ZOOM_LEVEL, ZOOM_LEVEL);
		render_list.push_back(&all_cards[i]);
	}

	p1 = new Player;
	p2 = new Player;
	ai = new AI(p2, this);
	// 设置p2为上方玩家
	p2->upside = true;
	game_state = gs_main_menu;
	//new_game();
}

Game::Game(card_info c)
{
	delete p1;
	delete p2;
}

Game::~Game()
{
	delete p1;
	delete p2;
	//delete font_texture;
	ImGui::SFML::Shutdown();
}

void Game::loop()
{
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			// ImGui处理事件
			ImGui::SFML::ProcessEvent(event);
			switch (event.type)
			{
			case sf::Event::Closed:
				window.close();
				break;
			case sf::Event::MouseMoved:
				l_button_pos.x = event.mouseMove.x;
				l_button_pos.y = event.mouseMove.y;
				break;
			case sf::Event::MouseButtonPressed:
				switch (event.mouseButton.button)
				{
				case sf::Mouse::Left:
					/*destination.x = event.mouseButton.x;
					destination.y = event.mouseButton.y;
					speed.x = (destination.x - position.x) / 0.5f;
					speed.y = (destination.y - position.y) / 0.5f;*/
					l_button_clicked = true;
					/*l_button_pos.x = event.mouseButton.x;
					l_button_pos.y = event.mouseButton.y;*/
					break;
				}
				break;
			case sf::Event::MouseButtonReleased:
				l_button_clicked = false;
				break;
			default:
				break;
			}
		}
		update(delta_clock.getElapsedTime());
		display();
	}
}

void Game::update(sf::Time time)
{
	// 清空画面
	window.clear(sf::Color(70, 120, 150));
	timer.Update(time);
	ImGui::SFML::Update(window, delta_clock.restart());

	auto view = window.getView();
	view_port = window.getViewport(view);
	//cout <<.width << endl;

	switch (game_state)
	{
	case gs_main_menu:
		render_main_menu();
		update_gui_main_menu();
		break;
	case gs_playing:
		if (state_queue.empty())
			std::cout << "错误: 流程队列为空!" << std::endl;
		else
		{
			// TODO: 还没有完全实现所有流程状态
			//ImGui::SFML::Update(window, delta_clock.restart());
			if (!state_queue.front()->expired)
				state_queue.front()->Update(time);
			//ImGui::EndFrame();
			flow_queue.clear();
		}

		// 当出现流局时，reset被调用，玩家队列为空，这个函数中有调用player_queue.front的，会出错，所以要判断
		if (player_queue.size())
			update_gui_playing();

		/*if (static_cast<int>(position.x) != static_cast<int>(destination.x) || static_cast<int>(position.y) != static_cast<int>(destination.y))
		{
			// 前向判断，用于避免速度过快直接冲过目标点。
			// 如果本次位置更新后，与目标点的相对方位发生了变化，说明本次移动会冲过目标点，那就直接把位置设为目标点
			int direction_x, direction_y, next_direction_x, next_direction_y;
			direction_x = (destination.x - position.x) / abs(destination.x - position.x);
			direction_y = (destination.y - position.y) / abs(destination.y - position.y);
			// 在此更新位置
			position.x += speed.x * time.asSeconds();
			position.y += speed.y * time.asSeconds();
			next_direction_x = (destination.x - position.x) / abs(destination.x - position.x);
			next_direction_y = (destination.y - position.y) / abs(destination.y - position.y);
			if (direction_x != next_direction_x || direction_y != next_direction_y)
			{
				position = destination;
			}
		}
		test_sprite.setPosition(position);*/

		ImGui::EndFrame();
		render_playing();
		/*for(std::deque<Card*>::iterator it = moving_cards.begin(); it!=moving_cards.end(); ++it)
		{
			window.draw((*it)->sprite);
		}*/
		//window.draw(test_sprite);
		break;
	}
}

void Game::render_playing()
{
	// 绘制
	window.draw(background);
	window.draw(parent_sign);
	window.draw(sprite_heap);
	// 绘制可见的卡-----------这个可能有点问题，还是采用分区绘制好
	for (auto card : render_list)
	{
		if (card->visible)
		{
			window.draw(card->sprite);
			if (card->highlighted)
			{
				highlight.setPosition(card->pos);
				window.draw(highlight);
			}
		}
	}
	/*for (list<Card*>::iterator it = p1->hand_cards.begin(); it != p1->hand_cards.end(); ++it)
	{
		if ((*it)->visible)
			window.draw((*it)->sprite);
	}
	for (list<Card*>::iterator it = p2->hand_cards.begin(); it != p2->hand_cards.end(); ++it)
	{
		if ((*it)->visible)
			window.draw((*it)->sprite);
	}
	for (list<Card*>::iterator it = field_cards.begin(); it != field_cards.end(); ++it)
	{
		if ((*it)->visible)
			window.draw((*it)->sprite);
	}
	// 绘制双方得牌
	for (list<Card*>::iterator it = p1->earned_light.begin(); it != p1->earned_light.end(); ++it)
	{
		// moving的牌在移动牌里面就渲染过了，不用重复渲染
		if ((*it)->visible && !(*it)->moving)
			window.draw((*it)->sprite);
	}
	for (list<Card*>::iterator it = p1->earned_short.begin(); it != p1->earned_short.end(); ++it)
	{
		if ((*it)->visible && !(*it)->moving)
			window.draw((*it)->sprite);
	}
	for (list<Card*>::iterator it = p1->earned_seed.begin(); it != p1->earned_seed.end(); ++it)
	{
		if ((*it)->visible && !(*it)->moving)
			window.draw((*it)->sprite);
	}
	for (list<Card*>::iterator it = p1->earned_skin.begin(); it != p1->earned_skin.end(); ++it)
	{
		if ((*it)->visible && !(*it)->moving)
			window.draw((*it)->sprite);
	}
	for (list<Card*>::iterator it = p2->earned_light.begin(); it != p2->earned_light.end(); ++it)
	{
		// moving的牌在移动牌里面就渲染过了，不用重复渲染
		if ((*it)->visible && !(*it)->moving)
			window.draw((*it)->sprite);
	}
	for (list<Card*>::iterator it = p2->earned_short.begin(); it != p2->earned_short.end(); ++it)
	{
		if ((*it)->visible && !(*it)->moving)
			window.draw((*it)->sprite);
	}
	for (list<Card*>::iterator it = p2->earned_seed.begin(); it != p2->earned_seed.end(); ++it)
	{
		if ((*it)->visible && !(*it)->moving)
			window.draw((*it)->sprite);
	}
	for (list<Card*>::iterator it = p2->earned_skin.begin(); it != p2->earned_skin.end(); ++it)
	{
		if ((*it)->visible && !(*it)->moving)
			window.draw((*it)->sprite);
	}*/

	// 绘制移动的卡
	if (!moving_cards.empty())
		window.draw(moving_cards.front()->sprite);
}

void Game::render_main_menu()
{
	window.draw(background);
}

void Game::display()
{
	// 绘制ImGui
	window.resetGLStates();
	ImGui::SFML::Render();
	window.display();
}

void Game::reset()
{
	// 重置牌
	for (int i = 0; i < 48; ++i)
	{
		all_cards[i].visible = false;
		all_cards[i].moving = false;
		all_cards[i].earned = false;
		all_cards[i].pos = {HEAP_POS_X, HEAP_POS_Y};
		all_cards[i].speed = {0, 0};
		all_cards[i].dest = {HEAP_POS_X, HEAP_POS_Y};
		all_cards[i].update_pos();
		all_cards[i].show_face();
	}
	// 重置玩家信息
	p1->reset();
	p2->reset();
	// 重置牌堆
	shuffle();
	// 重置场牌
	field_cards.clear();
	// 清空流程队列
	flow_queue.clear();
	flow_queue.push_back(fs_prepare);
	// 清空玩家队列
	player_queue.clear();
	// 清空移动卡牌序列
	moving_cards.clear();
	current_moving_card = nullptr;
	// 重新创建状态对象
	for (auto pair : state_dict)
	{
		delete pair.second;
	}
	state_dict.clear();
	state_dict[fs_prepare] = new PrepareState;
	state_dict[fs_dispatch] = new DispatchState;
	state_dict[fs_validate_game] = new ValidateGameState;
	state_dict[fs_precomplete] = new PrecompleteState;
	state_dict[fs_put] = new PutState;
	state_dict[fs_put_move_to_target] = new PutMoveToTargetState;
	state_dict[fs_draw] = new DrawState;
	state_dict[fs_draw_move_to_target] = new DrawMoveToTargetState;
	state_dict[fs_select_put_target] = new SelectPutTargetState;
	state_dict[fs_select_draw_target] = new SelectDrawTargetState;
	state_dict[fs_put_get] = new PutGetState;
	state_dict[fs_detect_win] = new CheckWinState;
	state_dict[fs_end_turn] = new EndTurnState;
	state_dict[fs_koikoi] = new KoikoiState;
	state_dict[fs_summary] = new SummaryState;
	state_dict[fs_end_game] = new EndGameState;
	state_dict[fs_wait_interval] = new WaitIntervalState;
	state_queue.clear();
	state_queue.push_back(state_dict[fs_prepare]);
}

void Game::shuffle()
{
	heap.clear();
	// 临时列表中存储了48张牌的索引
	list<int> temp_list;
	for (int i = 0; i < 48; ++i)
		temp_list.push_back(i);
	while (!temp_list.empty())
	{
		// 随机选择一个索引，并将对应的card_info放进牌堆
		heap.push_back(&all_cards[switch_from_list(temp_list)]);
	}
}

Card *Game::draw_card()
{
	// 如果没有卡了，报错
	if (heap.empty())
	{
		std::cout << "牌堆已空！" << std::endl;
	}
	Card *result = heap.front();
	heap.pop_front();
	return result;
}

void Game::update_gui_playing()
{
	//ImGui::SFML::Update(window, delta_clock.restart());
	//ImGui::ShowTestWindow();
	ImGui::SetNextWindowContentSize(ImVec2(80, 80));
	ImGui::Begin("info");
	ImGui::Text(u8"当前月份：%d", current_month);
	ImGui::Text(u8"p2 money: %d", p2->money);
	ImGui::Text(u8"p1 money: %d", p1->money);
	ImGui::Text(u8"当前流程：%s", state_queue.front()->phase_name.c_str());
	ImGui::Text(u8"当前玩家：%s", (player_queue.front() == p1 ? u8"下方" : u8"上方"));
	//ImGui::Text("%d, %d", all_cards[0].sprite.getTextureRect().width, all_cards[0].sprite.getTextureRect().height);
	//ImGui::Text("position: %f, %f", position.x, position.y);
	//ImGui::Text("destination: %f, %f", destination.x, destination.y);
	//ImGui::Text("position==destination: %s", (static_cast<int>(position.x) == static_cast<int>(destination.x) && static_cast<int>(position.y) == static_cast<int>(destination.y)) ? "true" : "false");
	ImGui::End();
	if (flow_queue.front() == fs_koikoi || state_queue.front() == state_dict[fs_koikoi])
	{
		if (player_queue.front() != p2)
		{
			ImGui::SetNextWindowPosCenter();
			ImGui::Begin("koikoi?", 0, ImVec2(200, 200), -1, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);
			// 列出所有的扎役
			// 关于役的可选设置，目前还没有做，就用字面量直接算
			for (int i = 0; i < 14; ++i)
			{
				if (player_queue.front()->earned_wins[i])
				{
					if (cm.wins[i].name == u8"短册" && player_queue.front()->sbook_extra() > 0)
						ImGui::Text(u8"%s+%d %d文", cm.wins[i].name.c_str(), player_queue.front()->sbook_extra(), cm.wins[i].money + player_queue.front()->sbook_extra());
					else if (cm.wins[i].name == u8"种" && player_queue.front()->seed_extra() > 0)
						ImGui::Text(u8"%s+%d %d文", cm.wins[i].name.c_str(), player_queue.front()->seed_extra(), cm.wins[i].money + player_queue.front()->seed_extra());
					else if (cm.wins[i].name == u8"皮" && player_queue.front()->skin_extra() > 0)
						ImGui::Text(u8"%s+%d %d文", cm.wins[i].name.c_str(), player_queue.front()->skin_extra(), cm.wins[i].money + player_queue.front()->skin_extra());
					else
						ImGui::Text(u8"%s %d文", cm.wins[i].name.c_str(), cm.wins[i].money);
				}
			}
			ImGui::PushItemWidth(120.0f);
			ImGui::Separator();
			if (ImGui::Button("koikoi"/*, ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 30)*/))
			{
				selected_koikoi = true;
			}
			ImGui::SameLine();
			if (ImGui::Button(u8"结束"/*, ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 30)*/))
			{
				selected_end = true;
			}
			ImGui::PopItemWidth();
			ImGui::End();
		}
		else
		{
			if (ai->determine_koikoi())
				selected_koikoi = true;
			else
				selected_end = true;
		}
	}
	if (flow_queue.front() == fs_summary || state_queue.front() == state_dict[fs_summary])
	{
		ImGui::SetNextWindowPosCenter();
		ImGui::Begin(u8"结算", 0, ImVec2(200, 200), -1, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text(player_queue.front() == p2 ? u8"计算机获胜" : u8"玩家获胜");
		ImGui::Separator();
		for (int i = 0; i < 14; ++i)
		{
			if (player_queue.front()->earned_wins[i])
			{
				if (cm.wins[i].name == u8"短册" && player_queue.front()->sbook_extra() > 0)
					ImGui::Text(u8"%s+%d %d文", cm.wins[i].name.c_str(), player_queue.front()->sbook_extra(), cm.wins[i].money + player_queue.front()->sbook_extra());
				else if (cm.wins[i].name == u8"种" && player_queue.front()->seed_extra() > 0)
					ImGui::Text(u8"%s+%d %d文", cm.wins[i].name.c_str(), player_queue.front()->seed_extra(), cm.wins[i].money + player_queue.front()->seed_extra());
				else if (cm.wins[i].name == u8"皮" && player_queue.front()->skin_extra() > 0)
					ImGui::Text(u8"%s+%d %d文", cm.wins[i].name.c_str(), player_queue.front()->skin_extra(), cm.wins[i].money + player_queue.front()->skin_extra());
				else
					ImGui::Text(u8"%s %d文", cm.wins[i].name.c_str(), cm.wins[i].money);
			}
		}
		ImGui::Separator();
		if (ImGui::Button(u8"下一局", ImVec2(ImGui::GetWindowContentRegionWidth(), 30)))
		{
			flow_summary();
			if (p1->money == 0 || p2->money == 0)
			{
				flow_queue.pop_front();
				flow_queue.push_back(fs_end_game);
			}
			else
			{
				current_month += 1;
				current_month %= 12;
				reset();
			}
		}
		ImGui::End();
	}
	if (flow_queue.front() == fs_end_game || state_queue.front()==state_dict[fs_end_game])
	{
		ImGui::SetNextWindowPosCenter();
		ImGui::Begin(u8"游戏结束", 0, ImVec2(200, 200), -1, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text(u8"游戏结束！");
		ImGui::Separator();
		if (p2->money == 0)
		{
			ImGui::Text(u8"恭喜！你赢了！");
		}
		else if (p1->money == 0)
			ImGui::Text(u8"很遗憾，你输了！");
		ImGui::Separator();
		if (ImGui::Button(u8"新游戏"))
			new_game();
		ImGui::End();
	}
	//ImGui::EndFrame();
}

void Game::update_gui_main_menu()
{
	//ImGui::SFML::Update(window, delta_clock.restart());
	ImGui::ShowTestWindow();
	ImGui::SetNextWindowContentSize(ImVec2(80, 80));
	ImGui::SetNextWindowPosCenter();
	ImGui::Begin(u8"主菜单", 0, ImVec2(216, 120), -1, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::BeginGroup();
	if (ImGui::Button(u8"新游戏", ImVec2(200, 50)))
	{
		game_state = gs_playing;
		new_game();
	}
	if (ImGui::Button(u8"读取游戏", ImVec2(200, 50)))
	{
		game_state = gs_playing;
		new_game();
		load_game();
	}
	//ImGui::Text("%d, %d", all_cards[0].sprite.getTextureRect().width, all_cards[0].sprite.getTextureRect().height);
	//ImGui::Text("position: %f, %f", position.x, position.y);
	//ImGui::Text("destination: %f, %f", destination.x, destination.y);
	//ImGui::Text("position==destination: %s", (static_cast<int>(position.x) == static_cast<int>(destination.x) && static_cast<int>(position.y) == static_cast<int>(destination.y)) ? "true" : "false");
	ImGui::EndGroup();
	ImGui::End();
	//ImGui::EndFrame();
}

void Game::update_koikoi_gui()
{
}

void Game::new_game()
{
	reset();
	p1->money = START_MONEY;
	p2->money = START_MONEY;
	current_month = 1;
	//bgm.play();
}

void Game::put_to_field(Card *card)
{
	// 为卡片选择一个合适的位置，并为其设定目标和速度、加入moving_cards
	if (field_cards.empty())
	{
		card->set_dest({FIELD_ORIGIN_X, FIELD_ORIGIN_Y});
		field_cards.push_back(card);
	}
	else
	{
		int i = 0;
		bool found_empty = false;
		// 试着用一下auto关键字
		//for (list<Card*>::iterator it = field_cards.begin(); it != field_cards.end(); ++it, ++i)
		for (auto it = field_cards.begin(); it != field_cards.end(); ++it, ++i)
		{
			if (!(*it))
			{
				card->set_dest(get_field_pos(i));
				found_empty = true;
				*it = card;
				// 这里必须break，不然同一张卡重复添加
				break;
			}
		}
		// 上面循环结束后，i自然就是新添加的节点的索引
		if (!found_empty)
		{
			card->set_dest(get_field_pos(field_cards.size()));
			field_cards.push_back(card);
		}
	}
	card->moving = true;
	moving_cards.push_back(card);
}

sf::Vector2f Game::get_field_pos(int index)
{
	// 除2取商确定水平位置，奇偶确定垂直位置，偶数在中线上，奇数在中线下
	float pos_x, pos_y;
	pos_x = (index / 2) * CARD_SIZE_X + FIELD_ORIGIN_X + (index / 2) * BLANK_SIZE;
	if (index % 2 == 0)
		pos_y = WINDOW_HEIGHT / 2 - CARD_SIZE_Y - BLANK_SIZE;
	else
		pos_y = WINDOW_HEIGHT / 2 + BLANK_SIZE;
	return {pos_x, pos_y};
}

int Game::count_same_month(int m)
{
	int count = 0;
	for (list<Card *>::iterator it = field_cards.begin(); it != field_cards.end(); ++it)
	{
		// 已经被其他出牌占据并加入获得牌列表的牌不能算
		if ((*it) && !in_list(earned_cards, (*it)))
		{
			if ((*it)->month == m)
				++count;
		}
	}
	return count;
}

bool Game::move_cards(sf::Time time)
{
	//if (moving_cards.empty()) return false;
	// 直到开始移动了，才设置可见，避免扎堆可见看不见牌堆的情况
	moving_cards.front()->visible = true;
	if (moving_cards.front() != current_moving_card)
	{
		current_moving_card = moving_cards.front();
		sound_slide.play();
	}

	moving_cards.front()->update(time);
	// 移动中的牌永远优先级最高。
	remove_item(render_list, moving_cards.front());
	render_list.push_back(moving_cards.front());
	/*cout << "pos:" << moving_cards.front()->pos.x << "," << moving_cards.front()->pos.y;
	cout << "  dest:" << moving_cards.front()->dest.x << "," << moving_cards.front()->dest.y;
	cout << "  speed:" << moving_cards.front()->speed.x << "," << moving_cards.front()->speed.y << endl;*/
	if (!moving_cards.front()->moving)
	{
		moving_cards.pop_front();
		current_moving_card = nullptr;
		return true;
	}
	return false;
}

Card *Game::get_point_card(list<Card *> l)
{
	sf::Vector2f l_button_pos_world;
	l_button_pos_world = window.mapPixelToCoords(l_button_pos);
	sf::Rect<float> temp_rect;
	for (list<Card *>::iterator it = l.begin(); it != l.end(); ++it)
	{
		if (*it)
		{
			temp_rect.left = (*it)->pos.x;
			temp_rect.top = (*it)->pos.y;
			// 先这样写，以后为了性能可以用宏定义替换
			temp_rect.width = CARD_SIZE_X;
			temp_rect.height = CARD_SIZE_Y;
			if (temp_rect.contains(l_button_pos_world))
				return *it;
			// if ((*it)->sprite.getTextureRect().contains(l_button_pos.x, l_button_pos.y))
			// 	return *it;
		}
	}
	return nullptr;
}

void Game::flow_summary()
{
	// TODO: 这里可以判断一下双方是否有扎役。如果没有就要考虑亲权
	int total = 0;
	for (int i = 0; i < 14; ++i)
	{
		if (player_queue.front()->earned_wins[i])
		{
			if (cm.wins[i].name == u8"短册" && player_queue.front()->sbook_extra() > 0)
				total += cm.wins[i].money + player_queue.front()->win_sbook.size() - 5;
			else if (cm.wins[i].name == u8"种" && player_queue.front()->seed_extra() > 0)
				total += cm.wins[i].money + player_queue.front()->win_seed.size() - 5;
			else if (cm.wins[i].name == u8"皮" && player_queue.front()->skin_extra() > 0)
				total += cm.wins[i].money + player_queue.front()->win_skin.size() - 5;
			else
				total += cm.wins[i].money;
		}
	}
	// 由于每个玩家是自己将自身加入队尾的，所以这里基本可以确定输家也在player_queue里面
	if (player_queue.back()->money >= total)
	{
		player_queue.front()->money += total;
		player_queue.back()->money -= total;
	}
	else
	{
		player_queue.front()->money += player_queue.back()->money;
		player_queue.back()->money = 0;
	}
}

void Game::flow_log(string str)
{
	if (flow_state_str != str)
	{
		if (str == "fs_put")
			cout << "-----------" << (player_queue.front() == p1 ? "p1" : "p2") << " round"
				 << "-----------" << endl;
		cout << "log: entered " << str << endl;
	}
	flow_state_str = str;
}

void Game::save_game()
{
	ofstream file("save.txt");
	// 记录牌堆中所有的牌及顺序
	for (auto card : heap)
	{
		file << card->no << endl;
	}
	//char temp[30];
	//sprintf_s(temp, "%lu", tick_count);
	file << tick_count;
	file.close();
}

void Game::load_game()
{
	reset();
	heap.clear();
	ifstream file("save.txt");
	int no = 0;
	for (int i = 0; i < 48; ++i)
	{
		file >> no;
		heap.push_back(&all_cards[no]);
	}
	file >> tick_count;
	srand((unsigned int)tick_count);
	file.close();
}

void Game::put_ex(Card *card)
{
	put_card = card;
	// 将选中的牌从手牌中移除
	remove_item(player_queue.front()->hand_cards, card);

	if (card)
	{
		switch (count_same_month(card->month))
		{
		case 0:
			put_to_field(card);
			switch_state(fs_put_move_to_target);
			break;
		case 1:
			// TODO: 这里有一个问题，在p1_put_move_to_target里面，那个target该怎么记录？
			// 将两张得牌加入earned_cards
			earned_cards.push_back(card);
			for (auto it = field_cards.begin(); it != field_cards.end(); ++it)
			{
				// 要求不能已经被纳入得牌列表
				// 因为可能有这种情况：场上有两张同月，一张已经被赢取，但在这里没被跳过，反而因为顺序靠前被二次赢取
				if ((*it) && (*it)->month == card->month && !in_list(earned_cards, (*it)))
				{
					earned_cards.push_back(*it);
					card->set_dest((*it)->get_upon_pos());
					moving_cards.push_back(card);
					//*it = nullptr;
					//field_cards.erase(it);
					break;
				}
			}
			switch_state(fs_put_move_to_target);
			break;
		case 2:
			// TODO: 这里也有一样的问题，而且不能简单地用遍历解决
			// 由于被选中的只有1张牌，那就先考虑用一个变量记录好了
			earned_cards.push_back(card);
			// 预先将可选牌设置高亮
			for (auto it = field_cards.begin(); it != field_cards.end(); ++it)
			{
				if ((*it) && (*it)->month == card->month)
					(*it)->highlighted = true;
			}
			// 同样
			switch_state(fs_select_put_target);
			break;
		case 3:
			earned_cards.push_back(card);
			Card *target = nullptr;
			// 将四张得牌加入earned_cards
			for (list<Card *>::iterator it = field_cards.begin(); it != field_cards.end(); ++it)
			{
				if ((*it) && (*it)->month == card->month)
				{
					earned_cards.push_back(*it);
					if (!target)
						target = *it;
					// 然后要将它们从场牌消去
					//*it = nullptr;
				}
			}
			list<Card *>::iterator it = earned_cards.begin();
			// 将其余三张牌的移动目标都设置为第一个场牌中的得牌上方
			// 这种方法不稳妥
			// Card *target = *(++++it);
			for (it = earned_cards.begin(); it != earned_cards.end(); ++it)
			{
				if ((*it) != target)
				{
					(*it)->set_dest(target->get_upon_pos());
					moving_cards.push_back(*it);
				}
			}
			// 同样
			switch_state(fs_put_move_to_target);
			break;
		}
	}
}

void Game::select_target(Card *card)
{
	if (card && card->month == earned_cards.front()->month)
	{
		// 取消高亮
		for (list<Card *>::iterator it = field_cards.begin(); it != field_cards.end(); ++it)
		{
			if ((*it) && (*it)->month == card->month)
				(*it)->highlighted = false;
		}
		// 将p1_put_move_to_target入队，将出牌的目标设为所选牌的上方
		earned_cards.front()->set_dest(card->get_upon_pos());
		moving_cards.push_back(earned_cards.front());
		earned_cards.push_back(card);
		//null_item(field_cards, temp_card);
		flow_queue.push_back(fs_put_move_to_target);
		flow_queue.pop_front();
	}
}

void Game::select_put_target(Card *card)
{
	if (card && card->month == earned_cards.front()->month)
	{
		// 取消高亮
		for (list<Card *>::iterator it = field_cards.begin(); it != field_cards.end(); ++it)
		{
			if ((*it) && (*it)->month == card->month)
				(*it)->highlighted = false;
		}
		// 将p1_put_move_to_target入队，将出牌的目标设为所选牌的上方
		earned_cards.front()->set_dest(card->get_upon_pos());
		moving_cards.push_back(earned_cards.front());
		earned_cards.push_back(card);
		//null_item(field_cards, temp_card);
		flow_queue.push_back(fs_put_move_to_target);
		flow_queue.pop_front();
	}
}

void Game::select_put_target_ex(Card *card)
{
	if (card && card->month == put_card->month)
	{
		// 取消高亮
		for (list<Card *>::iterator it = field_cards.begin(); it != field_cards.end(); ++it)
		{
			if ((*it) && (*it)->month == card->month)
				(*it)->highlighted = false;
		}
		// 将p1_put_move_to_target入队，将出牌的目标设为所选牌的上方
		earned_cards.front()->set_dest(card->get_upon_pos());
		moving_cards.push_back(earned_cards.front());
		earned_cards.push_back(card);
		//null_item(field_cards, temp_card);
		switch_state(fs_put_move_to_target); // todo: 这里实际上一个put_move状态即可
	}
}

void Game::select_draw_target(Card *card)
{
	// 其实完全可以把抽到的牌直接插到earned_cards的首位的，但为了容易理解，还是另用一个变量记录
	if (card && card->month == drawn_card->month && !in_list(earned_cards, card))
	{
		// 取消高亮
		for (list<Card *>::iterator it = field_cards.begin(); it != field_cards.end(); ++it)
		{
			if ((*it) && (*it)->highlighted)
				(*it)->highlighted = false;
		}
		// 将p1_put_move_to_target入队，将出牌的目标设为所选牌的上方
		drawn_card->set_dest(card->get_upon_pos());
		moving_cards.push_back(drawn_card);
		earned_cards.push_back(card);
		//null_item(field_cards, temp_card);
		flow_queue.pop_front();
		flow_queue.push_front(fs_draw_move_to_target);
	}
}

void Game::switch_state(FlowState fs, float in_seconds)
{
	state_queue.front()->OnExit();
	if (in_seconds > 0.0f)
	{
		timer.Add(in_seconds, [this, fs] {
			state_queue.pop_front();
			state_queue.push_back(state_dict[fs]);
			state_queue.front()->OnEnter();
		});
	}
	else
	{
		state_queue.pop_front();
		state_queue.push_back(state_dict[fs]);
		state_queue.front()->OnEnter();
	}
}
