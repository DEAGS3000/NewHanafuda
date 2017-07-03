#include "Game.h"
#include <imgui-SFML.h>
#include <imgui.h>
#include <iostream>
#include "utilities.h"
#include <windows.h>

using namespace std;

Game::Game() :window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), L"花札", sf::Style::Default)
{
	// 置随机种子
	srand((unsigned int)GetTickCount());

	window.setVerticalSyncEnabled(true);

	// 绑定ImGui
	ImGui::SFML::Init(window);

	// 设置ImGui字体
	ImGuiIO& io = ImGui::GetIO();
	// 需要先把默认的clear掉
	io.Fonts->ClearFonts();
	io.Fonts->AddFontFromFileTTF("res/xarialuni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesChinese());
	// 这里必须使用指针去new，否则生命周期限于构造函数，会导致字体变成方块
	font_texture = new sf::Texture;
	ImGui::SFML::createFontTexture(*font_texture);
	ImGui::SFML::setFontTexture(*font_texture);

	/*test_texture.loadFromFile("0.jpg");
	test_sprite.setTexture(test_texture);
	position = { 0, 0 };
	test_sprite.setPosition(position);
	speed = { destination.x / 0.1f, destination.y / 0.1f };*/

	//background_texture.loadFromFile("res/bg.bmp");
	//background_texture.setRepeated(true);
	background.setTexture(cm.background);
	sf::Rect<int> temp = { 0, 0, 1600,900 };
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
	game_state = gs_playing;
	new_game();
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
	delete font_texture;
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

	switch (game_state)
	{
	case gs_main_menu:
		break;
	case gs_playing:
		if (flow_queue.empty())
			std::cout << "错误: 流程队列为空!" << std::endl;
		else
		{
			switch (flow_queue.front())
			{
			case fs_prepare:
				flow_log("fs_prepare");
				flow_prepare();
				break;
			case fs_dispatch:
				flow_log("fs_dispatch");
				Card *temp_card;
				for (int i = 0; i < 8; ++i)
				{
					// 按照子-场-亲顺序发牌，还要为Card对象设定好目标位置
					temp_card = draw_card();
					if (player_queue.back() == p2 && !DEBUG_SHOW_FACE)
						temp_card->show_back();
					//temp_card->visible = true;
					player_queue.back()->get(temp_card);
					moving_cards.push_back(temp_card);
					temp_card->set_dispatch_speed();

					temp_card = draw_card();
					//temp_card->visible = true;
					//field_cards.push_back(temp_card);
					put_to_field(temp_card);
					moving_cards.push_back(temp_card);
					temp_card->set_dispatch_speed();

					temp_card = draw_card();
					if (player_queue.front() == p2 && !DEBUG_SHOW_FACE)
						temp_card->show_back();
					//temp_card->visible = true;
					player_queue.front()->get(temp_card);
					moving_cards.push_back(temp_card);
					temp_card->set_dispatch_speed();
				}
				flow_queue.push_back(fs_dispatch_moving);
				flow_queue.pop_front();
				break;
			case fs_dispatch_moving:
				flow_log("fs_dispatch_moving");
				// 发牌过程的操作可以改一下
				/*for(deque<Card*>::iterator it=moving_cards.begin(); it!=moving_cards.end(); )
				{
					(*it)->visible = true;
					(*it)->update(time);
					if (!(*it)->moving)
						it = moving_cards.erase(it);
					else
						++it;
				}*/

				if (!moving_cards.empty())
				{
					move_cards(time);
				}
				else
				{
					flow_queue.push_back(fs_precomplete);
					flow_queue.pop_front();
				}


				break;
			case fs_validate_game:
				flow_log("fs_validate_game");
				flow_validate_game();
				break;
			case fs_precomplete:
				flow_log("fs_precomplete");
				// 检查手四
				flow_precomplete();
				break;
			case fs_put:
				flow_log("fs_put");
				flow_put();
				break;
			case fs_draw:
				flow_log("fs_draw");
				// 如果场牌没有可得牌，将抽牌加入场牌
				// 如果有一张可得牌，将p1_draw_move_to_target入队
				// 如果场牌中有1张以上可得牌，就把p1_select_draw_target入队
				flow_draw();
				break;
			case fs_select_put_target:
				flow_log("fs_select_put_target");
				flow_select_put_target();
				break;
			case fs_select_draw_target:
				flow_log("fs_select_draw_target");
				flow_select_draw_target();
				break;
			case fs_put_move_to_field:
				flow_log("fs_put_move_to_field");
				if (!moving_cards.empty())
				{
					if (move_cards(time))
						sound_put.play();
				}
				else
				{
					// 重整双方手牌，此时earned_cards为空
					player_queue.front()->format_cards(earned_cards);
					flow_queue.push_back(fs_draw);
					flow_queue.pop_front();
				}


				break;
			case fs_draw_move_to_field:
				flow_log("fs_draw_move_to_field");
				// draw_move_to_field之后当前玩家就没什么操作了，进行玩家队列操作，换下一个玩家
				if (!moving_cards.empty())
				{
					if (move_cards(time))
						sound_put.play();
				}
				else
				{
					// 重整双方手牌，此时earned_cards为空
					player_queue.front()->format_cards(earned_cards);
					flow_queue.push_back(fs_wait_interval);
					flow_queue.push_back(fs_detect_win);
					flow_queue.pop_front();
				}

				break;
			case fs_put_move_to_target:
				flow_log("fs_put_move_to_target");
				// 看看earned_cards中有几个元素，有2个就是场牌中有1或2张同月，有4个就是场牌中有3张同月
				if (!moving_cards.empty())
				{
					if (move_cards(time))
						sound_put.play();
				}
				// 为了让被选的场牌在move_to_target过程中也能显示，在put中就先不从场牌中去掉了
				// 而是在put_move_to_target结束后，再根据earned_cards去掉
				/*for (list<Card*>::iterator it = earned_cards.begin(); it != earned_cards.end(); ++it)
				{
					// 场牌不能用remove_item
					//remove_item(field_cards, (*it));
					null_item(field_cards, *it);
				}*/
				else
				{
					// 等待一段时间，好让用户看清楚
					flow_queue.push_back(fs_draw);
					//flow_queue.push_back(fs_wait_interval);
					flow_queue.push_back(fs_put_get);
					// 这里在draw开始时，put_get已经完成了，所以不用担心earned_cards冲突
					// 将put的全过程和draw的全过程彻底错开，放到put_get_move结束再入队
					flow_queue.pop_front();
				}
				break;
			case fs_draw_move_to_target:
				flow_log("fs_draw_move_to_target");
				if (!moving_cards.empty())
				{
					if (move_cards(time))
						sound_put.play();
				}
				else
				{
					flow_queue.push_back(fs_wait_interval);
					flow_queue.push_back(fs_draw_get);
					flow_queue.pop_front();
				}

				break;
			case fs_put_get:
				flow_log("fs_put_get");
				// 对earned_cards中的所有卡实行赢取操作
				for (list<Card*>::iterator it = earned_cards.begin(); it != earned_cards.end(); ++it)
				{
					moving_cards.push_back(*it);
					player_queue.front()->earn(*it, current_month);
					// 在这里再将牌从场牌消去
					null_item(field_cards, *it);
					// 这一步操作进行完以后，目的就已经达到了。各得牌列表有了真实的长度，可以算出正确的位置和便宜
					// 同时新的得牌也已经在列表中，可以用format函数中生成的各种位置信息，为没有moving的原得牌设置pos
					// 为有moving的新得牌设置dest。moving属性会在移动结束后消去，所以在移动中，渲染得牌部分时也不渲染有moving的
				}
				// 重整双方手牌
				player_queue.front()->format_cards(earned_cards);
				// 由于这个状态对于手牌出牌和抽牌出牌是通用的，所以转换状态之前要清空earned_cards
				earned_cards.clear();
				flow_queue.push_back(fs_put_get_moving);
				flow_queue.pop_front();
				break;
				// 这个状态似乎就派不上用场了，因为所有得牌都在earned_cards里面，都在fs_put_get里面被赢取了
			case fs_draw_get:
				flow_log("fs_draw_get");
				// 对earned_cards中的所有卡实行赢取操作
				for (list<Card*>::iterator it = earned_cards.begin(); it != earned_cards.end(); ++it)
				{
					moving_cards.push_back(*it);
					player_queue.front()->earn(*it, current_month);
					// 在这里再将牌从场牌消去
					null_item(field_cards, *it);
				}
				// 重整双方手牌
				player_queue.front()->format_cards(earned_cards);
				// 由于这个状态对于手牌出牌和抽牌出牌是通用的，所以转换状态之前要清空earned_cards
				earned_cards.clear();
				flow_queue.push_back(fs_draw_get_moving);
				flow_queue.pop_front();
				break;
			case fs_put_get_moving:
				flow_log("fs_put_get_moving");
				if (!moving_cards.empty())
				{
					move_cards(time);
				}
				//flow_queue.push_back(fs_detect_win);
				//flow_queue.push_back(fs_draw);
				else
				{
					flow_queue.pop_front();
				}

				break;
				// 这个状态也似乎没用了
			case fs_draw_get_moving:
				flow_log("fs_draw_get_moving");
				if (!moving_cards.empty())
				{
					move_cards(time);
				}
				else
				{
					flow_queue.push_back(fs_detect_win);
					flow_queue.pop_front();
				}


				break;
			case fs_detect_win:
				flow_log("fs_detect_win");
				// 可以考虑在这里加入双方手牌全无的判断。毕竟detect_win是每个人的每个回合都要做的
				flow_detect_win();
				break;
			case fs_koikoi:
				flow_log("fs_koikoi");
				// koikoi，end_turn入队
				// 结束，summary入队

				// 当玩家手牌已经出完时，没有koikoi选项，直接summary
				if (player_queue.front()->hand_cards.empty())
				{
					flow_queue.push_back(fs_summary);
					flow_queue.pop_front();
				}
				else
				{
					if (selected_koikoi)
					{
						flow_queue.pop_front();
						flow_queue.push_back(fs_end_turn);
						selected_koikoi = false;
					}
					if (selected_end)
					{
						flow_queue.pop_front();
						flow_queue.push_back(fs_summary);
						selected_koikoi = false;
					}
				}

				//flow_koikoi();
				break;
			case fs_end_turn:
				// 检测一下队列里的最后一个流程是不是end_turn，如果不是，就跳过
				//if (flow_queue.back() != fs_end_turn) break;
				flow_log("fs_end_turn");
				player_queue.push_back(player_queue.front());
				player_queue.pop_front();
				flow_queue.push_back(fs_put);
				flow_queue.pop_front();
				break;
			case fs_summary:
				flow_log("fs_summary");
				// 设计ui互动的状态，流程函数在gui处调用
				//flow_summary();
				break;
			case fs_end_game:
				break;
			case fs_wait_interval:
				flow_log("fs_wait_interval");
				interval_waited += time;
				if (interval_waited.asSeconds() >= WAIT_INTERVAL)
				{
					// 计时器清零
					interval_waited = sf::Time::Zero;
					flow_queue.pop_front();
				}
				break;
			}
		}

		update_gui();

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

void Game::display()
{
	// 绘制ImGui
	window.resetGLStates();
	ImGui::Render();
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
		all_cards[i].pos = { HEAP_POS_X, HEAP_POS_Y };
		all_cards[i].speed = { 0, 0 };
		all_cards[i].dest = { HEAP_POS_X, HEAP_POS_Y };
		all_cards[i].update_pos();
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

void Game::update_gui()
{
	ImGui::SFML::Update(window, delta_clock.restart());
	ImGui::ShowTestWindow();
	ImGui::SetNextWindowContentSize(ImVec2(80, 80));
	ImGui::Begin("info");
	ImGui::Text(u8"当前月份：%d", current_month);
	ImGui::Text(u8"p2 money: %d", p2->money);
	ImGui::Text(u8"p1 money: %d", p1->money);
	ImGui::Text(u8"当前流程：%s", flow_state_str.c_str());
	ImGui::Text(u8"当前玩家：%s", (player_queue.front() == p1 ? u8"下方" : u8"上方"));
	//ImGui::Text("%d, %d", all_cards[0].sprite.getTextureRect().width, all_cards[0].sprite.getTextureRect().height);
	//ImGui::Text("position: %f, %f", position.x, position.y);
	//ImGui::Text("destination: %f, %f", destination.x, destination.y);
	//ImGui::Text("position==destination: %s", (static_cast<int>(position.x) == static_cast<int>(destination.x) && static_cast<int>(position.y) == static_cast<int>(destination.y)) ? "true" : "false");
	ImGui::End();
	if (flow_queue.front() == fs_koikoi)
	{
		if (player_queue.front() != p2)
		{
			ImGui::SetNextWindowPosCenter();
			ImGui::Begin("", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
			// 列出所有的扎役
			// 关于役的可选设置，目前还没有做，就用字面量直接算
			for (int i = 0; i < 14; ++i)
			{
				if (player_queue.front()->earned_wins[i])
				{
					if (cm.wins[i].name == u8"短册")
						ImGui::Text(u8"%s+%d %d文", cm.wins[i].name.c_str(), player_queue.front()->sbook_length - 5, cm.wins[i].money);
					else if (cm.wins[i].name == u8"种")
						ImGui::Text(u8"%s+%d %d文", cm.wins[i].name.c_str(), player_queue.front()->seed_length - 5, cm.wins[i].money);
					else if (cm.wins[i].name == u8"皮")
						ImGui::Text(u8"%s+%d %d文", cm.wins[i].name.c_str(), player_queue.front()->skin_length - 10, cm.wins[i].money);
					else
						ImGui::Text(u8"%s %d文", cm.wins[i].name.c_str(), cm.wins[i].money);
				}
			}
			ImGui::PushItemWidth(120.0f);
			// 看看ImGui有分割线没，往这里插一个
			ImGui::Separator();
			if (ImGui::Button("koikoi"))
			{
				selected_koikoi = true;
			}
			ImGui::SameLine();
			if (ImGui::Button(u8"结束"))
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
	if (flow_queue.front() == fs_summary)
	{
		ImGui::SetNextWindowPosCenter();
		ImGui::Begin("", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
		ImGui::Text(player_queue.front() == p2 ? u8"计算机获胜" : u8"玩家获胜");
		ImGui::Separator();
		for (int i = 0; i < 14; ++i)
		{
			if (player_queue.front()->earned_wins[i])
			{
				if (cm.wins[i].name == u8"短册" && player_queue.front()->sbook_extra()>0)
					ImGui::Text(u8"%s+%d %d文", cm.wins[i].name.c_str(), player_queue.front()->sbook_extra(), cm.wins[i].money+ player_queue.front()->sbook_extra());
				else if (cm.wins[i].name == u8"种" && player_queue.front()->seed_extra()>0)
					ImGui::Text(u8"%s+%d %d文", cm.wins[i].name.c_str(), player_queue.front()->seed_extra(), cm.wins[i].money+ player_queue.front()->seed_extra());
				else if (cm.wins[i].name == u8"皮" && player_queue.front()->skin_extra()>0)
					ImGui::Text(u8"%s+%d %d文", cm.wins[i].name.c_str(), player_queue.front()->skin_extra(), cm.wins[i].money+ player_queue.front()->skin_extra());
				else
					ImGui::Text(u8"%s %d文", cm.wins[i].name.c_str(), cm.wins[i].money);
			}
		}
		ImGui::Separator();
		if (ImGui::Button(u8"下一局"))
		{
			flow_summary();
			if(p1->money==0 || p2->money==0)
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
	if(flow_queue.front()==fs_end_game)
	{
		ImGui::SetNextWindowPosCenter();
		ImGui::Begin("", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
		ImGui::Text(u8"游戏结束！");
		ImGui::Separator();
		if (p1->money == 0)
		{
			ImGui::Text(u8"恭喜！你赢了！");
		}
		else if (p2->money == 0)
			ImGui::Text(u8"很遗憾，你输了！");
		ImGui::Separator();
		if (ImGui::Button("新游戏"))
			new_game();
		ImGui::End();
	}
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

void Game::put_to_field(Card* card)
{
	// 为卡片选择一个合适的位置，并为其设定目标和速度、加入moving_cards
	if (field_cards.empty())
	{
		card->set_dest({ FIELD_ORIGIN_X, FIELD_ORIGIN_Y });
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
	pos_x = (index / 2)*CARD_SIZE_X + FIELD_ORIGIN_X + (index / 2)*BLANK_SIZE;
	if (index % 2 == 0)
		pos_y = WINDOW_HEIGHT / 2 - CARD_SIZE_Y - BLANK_SIZE;
	else
		pos_y = WINDOW_HEIGHT / 2 + BLANK_SIZE;
	return{ pos_x, pos_y };
}

int Game::count_same_month(int m)
{
	int count = 0;
	for (list<Card*>::iterator it = field_cards.begin(); it != field_cards.end(); ++it)
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

Card* Game::get_point_card(list<Card*> l)
{
	sf::Rect<float> temp_rect;
	for (list<Card*>::iterator it = l.begin(); it != l.end(); ++it)
	{
		if (*it)
		{
			temp_rect.left = (*it)->pos.x;
			temp_rect.top = (*it)->pos.y;
			// 先这样写，以后为了性能可以用宏定义替换
			temp_rect.width = CARD_SIZE_X;
			temp_rect.height = CARD_SIZE_Y;
			if (temp_rect.contains(l_button_pos.x, l_button_pos.y))
				return *it;
		}
	}
	return nullptr;
}

void Game::flow_prepare()
{
	// 决定亲子，0是p1为亲，1是p2为亲
	int temp = random(0, 1);
	if (temp == 0)
	{
		parent = p1;
		player_queue.push_back(p1);
		player_queue.push_back(p2);
	}
	else
	{
		parent = p2;
		player_queue.push_back(p2);
		player_queue.push_back(p1);
	}
	parent_sign.setPosition(player_queue.front()->get_parent_sign_pos());
	flow_queue.push_back(fs_dispatch);
	flow_queue.pop_front();
}

void Game::flow_validate_game()
{
	int month_count[12] = { 0 };

	for (std::list<Card*>::iterator it = field_cards.begin(); it != field_cards.end(); ++it)
	{
		if (*it)
		{
			++month_count[(*it)->month - 1];
		}
	}
	for (int i = 0; i < 12; ++i)
	{
		if (month_count[i] == 4)
		{
			std::cout << "场牌四张同月，流局！" << std::endl;
			reset();
		}
	}
}

void Game::flow_precomplete()
{
	// 检查手四，先亲后子，以便应对双方都有手四的情况
	// TODO: 双手四的情况没考虑
	bool found_hand_four = false;
	int temp_month[12] = { 0 };
	for (list<Card*>::iterator it = player_queue.front()->hand_cards.begin(); it != player_queue.front()->hand_cards.end(); ++it)
	{
		++temp_month[(*it)->month - 1];
	}
	for (int i = 0; i < 12; ++i)
	{
		if (temp_month[i] == 4)
		{
			found_hand_four = true;
			player_queue.front()->earned_wins[HAND_FOUR] = true;
			flow_queue.push_back(fs_koikoi);
			flow_queue.pop_front();
			break;
		}
	}
	// 按理说，如果第一个玩家koikoi了，那么会进入下一个玩家的回合
	memset(temp_month, 0, sizeof(int) * 12);
	for (list<Card*>::iterator it = player_queue.back()->hand_cards.begin(); it != player_queue.back()->hand_cards.end(); ++it)
	{
		++temp_month[(*it)->month - 1];
	}
	for (int i = 0; i < 12; ++i)
	{
		if (temp_month[i] == 4)
		{
			found_hand_four = true;
			player_queue.back()->earned_wins[HAND_FOUR] = true;
			flow_queue.push_back(fs_koikoi);
			flow_queue.pop_front();
			break;
		}
	}

	if (!found_hand_four)
	{
		flow_queue.push_back(fs_put);
		flow_queue.pop_front();
	}
}

void Game::flow_put()
{
	// 手牌打光，本局结束
	// 本段代码经测试效果基本正常
	if (player_queue.front()->hand_cards.empty())
	{
		flow_queue.push_back(fs_summary);
		flow_queue.pop_front();
		return;
	}

	// 由于有玩家队列，所以每次在流程函数中操作“玩家”就应该操纵队首。这样同样的流程对于两个玩家来说，就不用分两个流程状态了
	// 接受输入，选择手牌
	// 用l_button_pos查找p1手牌，找不到就不做事
	// 如果场牌没有可得牌，将所选牌加入场牌
	// 如果有1或3张可得牌，将p1_put_move_to_target入队
	// 如果场牌中有2张可得牌，就把p1_select_put_target入队
	if (player_queue.front() == p1)
	{
		Card *point_card = get_point_card(player_queue.front()->hand_cards);
		if(point_card)
		{
			for (auto &i : player_queue.front()->hand_cards)
				i->highlighted = false;
			point_card->highlighted = true;
		}
		else
		{
			// 如果鼠标没有停留在任意手牌，取消高亮
			for (auto &i : player_queue.front()->hand_cards)
				i->highlighted = false;
		}

		if (l_button_clicked)
		{
			// 打出了就取消高亮
			for (auto &i : player_queue.front()->hand_cards)
				i->highlighted = false;
			//Card *temp_card = get_point_card(player_queue.front()->hand_cards);
			put(point_card);
			// 将选中的牌从手牌中移除
			/*remove_item(player_queue.front()->hand_cards, temp_card);

			if (temp_card)
			{
				switch (count_same_month(temp_card->month))
				{
				case 0:
					put_to_field(temp_card);
					flow_queue.push_back(fs_put_move_to_field);
					// 下面这个让move_to_field在结束时入队
					//flow_queue.push_back(p1_draw);
					flow_queue.pop_front();
					break;
				case 1:
					// TODO: 这里有一个问题，在p1_put_move_to_target里面，那个target该怎么记录？
					// 将两张得牌加入earned_cards
					earned_cards.push_back(temp_card);
					for (auto it = field_cards.begin(); it != field_cards.end(); ++it)
					{
						if ((*it) && (*it)->month == temp_card->month)
						{
							earned_cards.push_back(*it);
							temp_card->set_dest((*it)->get_upon_pos());
							moving_cards.push_back(temp_card);
							//*it = nullptr;
							//field_cards.erase(it);
							break;
						}
					}
					flow_queue.push_back(fs_put_move_to_target);
					flow_queue.pop_front();
					break;
				case 2:
					// TODO: 这里也有一样的问题，而且不能简单地用遍历解决
					// 由于被选中的只有1张牌，那就先考虑用一个变量记录好了
					earned_cards.push_back(temp_card);
					// 预先将可选牌设置高亮
					for (auto it = field_cards.begin(); it != field_cards.end(); ++it)
					{
						if ((*it) && (*it)->month == temp_card->month)
							(*it)->highlighted = true;
					}
					flow_queue.push_back(fs_select_put_target);
					flow_queue.pop_front();
					break;
				case 3:
					earned_cards.push_back(temp_card);
					// 将四张得牌加入earned_cards
					for (list<Card*>::iterator it = field_cards.begin(); it != field_cards.end(); ++it)
					{
						if ((*it) && (*it)->month == temp_card->month)
						{
							earned_cards.push_back(*it);
							// 然后要将它们从场牌消去
							//*it = nullptr;
						}
					}
					list<Card*>::iterator it = earned_cards.begin();
					// 将其余三张牌的移动目标都设置为第一个场牌中的得牌上方
					Card *target = *(++++it);
					for (it = earned_cards.begin(); it != earned_cards.end(); ++it)
					{
						if ((*it) != target)
						{
							(*it)->set_dest(target->get_upon_pos());
							moving_cards.push_back(*it);
						}
					}
					flow_queue.push_back(fs_put_move_to_target);
					flow_queue.pop_front();
					break;
				}
			}*/
		}
	}
	else
	{
		ai->calculate(field_cards);
		Card *put_card = ai->select_put();
		// 将ai出牌正面显示
		if(!DEBUG_SHOW_FACE)
			put_card->show_face();
		put(put_card);
		ai->earned(put_card);
	}
}

void Game::flow_draw()
{
	if (heap.empty()) cout << "错误！牌堆已空！" << endl;

	Card *temp_card = heap.front();
	heap.pop_front();
	temp_card->visible = true;
	// 无论如何在卡翻开后先等待一会儿，让用户看清楚
	// 这句似乎看不出明显效果
	flow_queue.pop_front();
	//flow_queue.push_front(fs_wait_interval);

	switch (count_same_month(temp_card->month))
	{
	case 0:
		put_to_field(temp_card);
		//flow_queue.pop_front();
		flow_queue.push_front(fs_draw_move_to_field);
		flow_queue.push_front(fs_wait_interval);
		// 下面这个让move_to_field在结束时入队
		//flow_queue.push_back(p1_draw);
		break;
	case 1:
		// TODO: 这里有一个问题，在p1_put_move_to_target里面，那个target该怎么记录？
		// 将两张得牌加入earned_cards
		earned_cards.push_back(temp_card);
		for (list<Card*>::iterator it = field_cards.begin(); it != field_cards.end(); ++it)
		{
			// 选到的牌必须没有被已经打出的牌占据
			if ((*it) && (*it)->month == temp_card->month)
			{
				earned_cards.push_back(*it);
				temp_card->set_dest((*it)->get_upon_pos());
				moving_cards.push_back(temp_card);
				//*it = nullptr;
				//field_cards.erase(it);
				break;
			}
		}
		//flow_queue.pop_front();
		flow_queue.push_front(fs_draw_move_to_target);
		flow_queue.push_front(fs_wait_interval);
		break;
	case 2:
		// TODO: 这里也有一样的问题，而且不能简单地用遍历解决
		// 由于被选中的只有1张牌，那就先考虑用一个变量记录好了
		drawn_card = temp_card;
		earned_cards.push_back(temp_card);
		// 预先将可选牌设置高亮
		for (list<Card*>::iterator it = field_cards.begin(); it != field_cards.end(); ++it)
		{
			if ((*it) && !in_list(earned_cards, (*it)) && (*it)->month == temp_card->month)
				(*it)->highlighted = true;
		}
		//flow_queue.pop_front();
		flow_queue.push_front(fs_select_draw_target);
		flow_queue.push_front(fs_wait_interval);
		break;
	case 3:
		earned_cards.push_back(temp_card);
		// 将四张得牌加入earned_cards
		for (list<Card*>::iterator it = field_cards.begin(); it != field_cards.end(); ++it)
		{
			if ((*it) && (*it)->month == temp_card->month)
			{
				earned_cards.push_back(*it);
				//*it = nullptr;
			}
		}
		list<Card*>::iterator it = earned_cards.begin();
		// 将其余三张牌的移动目标都设置为第一个场牌中的得牌上方
		Card *target = *(++++it);
		for (it = earned_cards.begin(); it != earned_cards.end(); ++it)
		{
			if ((*it) != target)
			{
				(*it)->set_dest(target->get_upon_pos());
				moving_cards.push_back(*it);
			}
		}
		//flow_queue.pop_front();
		flow_queue.push_front(fs_draw_move_to_target);
		flow_queue.push_front(fs_wait_interval);
		break;
	}
}

void Game::flow_select_put_target()
{
	if (player_queue.front() != p2)
	{
		if (l_button_clicked)
		{
			Card *temp_card = get_point_card(field_cards);
			// 如果点了可选的牌
			/*if (temp_card && temp_card->month == earned_cards.front()->month)
			{
				// 取消高亮
				for (list<Card*>::iterator it = field_cards.begin(); it != field_cards.end(); ++it)
				{
					if ((*it) && (*it)->month == temp_card->month)
						(*it)->highlighted = false;
				}
				// 将p1_put_move_to_target入队，将出牌的目标设为所选牌的上方
				earned_cards.front()->set_dest(temp_card->get_upon_pos());
				moving_cards.push_back(earned_cards.front());
				earned_cards.push_back(temp_card);
				//null_item(field_cards, temp_card);
				flow_queue.push_back(fs_put_move_to_target);
				flow_queue.pop_front();
			}*/
			select_put_target(temp_card);
		}
	}
	else
	{
		// AI选牌
		Card *target = ai->select_put_target();
		select_put_target(target);
		ai->earned(target);
	}
}

void Game::flow_select_draw_target()
{
	if (player_queue.front() != p2)
	{
		// 将可选牌高亮渲染
		if (l_button_clicked)
		{
			Card *temp_card = get_point_card(field_cards);
			// 如果点了可选的牌
			// 其实完全可以把抽到的牌直接插到earned_cards的首位的，但为了容易理解，还是另用一个变量记录
			/*if (temp_card && temp_card->month == drawn_card->month)
			{
				// 取消高亮
				for (list<Card*>::iterator it = field_cards.begin(); it != field_cards.end(); ++it)
				{
					if ((*it) && (*it)->highlighted)
						(*it)->highlighted = false;
				}
				// 将p1_put_move_to_target入队，将出牌的目标设为所选牌的上方
				drawn_card->set_dest(temp_card->get_upon_pos());
				moving_cards.push_back(drawn_card);
				earned_cards.push_back(temp_card);
				//null_item(field_cards, temp_card);
				flow_queue.pop_front();
				flow_queue.push_front(fs_draw_move_to_target);
			}*/
			select_draw_target(temp_card);
		}
	}
	else
	{
		// AI选牌
		Card *target = ai->select_draw_target(drawn_card, field_cards);
		select_draw_target(target);
		ai->earned(target);
	}
}

void Game::flow_detect_win()
{
	bool has_new_win = false;
	Player *p = player_queue.front();
	// 五光
	if (!p->earned_wins[FIVE_LIGHT] && p->earned_light.size() == 5)
	{
		p->earned_wins[FIVE_LIGHT] = true;
		// 取消三光、四光、雨四光
		p->earned_wins[THREE_LIGHT] = false;
		p->earned_wins[FOUR_LIGHT] = false;
		p->earned_wins[RAIN_FOUR_LIGHT] = false;
		has_new_win = true;
	}
	// 四光
	if (!p->earned_wins[FOUR_LIGHT] && !in_list(p->earned_light, &all_cards[40]) && p->earned_light.size() == 4)
	{
		p->earned_wins[FOUR_LIGHT] = true;
		// 取消三光
		p->earned_wins[THREE_LIGHT] = false;
		has_new_win = true;
	}
	// 雨四光
	if (!p->earned_wins[RAIN_FOUR_LIGHT] && in_list(p->earned_light, &all_cards[40]) && p->earned_light.size() == 4)
	{
		p->earned_wins[RAIN_FOUR_LIGHT] = true;
		// 取消三光
		p->earned_wins[THREE_LIGHT] = false;
		has_new_win = true;
	}
	// 三光
	if (!p->earned_wins[THREE_LIGHT] && !in_list(p->earned_light, &all_cards[40]) && p->earned_light.size() == 3)
	{
		p->earned_wins[THREE_LIGHT] = true;
		has_new_win = true;
	}
	// 猪鹿蝶
	if (!p->earned_wins[PDB] && p->win_pdb.size() == 3)
	{
		p->earned_wins[PDB] = true;
		has_new_win = true;
	}
	// 赤短
	if (!p->earned_wins[REDS] && p->win_reds.size() == 3)
	{
		p->earned_wins[REDS] = true;
		has_new_win = true;
	}
	// 青短
	if (!p->earned_wins[PURPLES] && p->win_purples.size() == 3)
	{
		p->earned_wins[PURPLES] = true;
		has_new_win = true;
	}
	// 花见酒
	if (!p->earned_wins[FWINE] && p->win_fwine.size() == 2)
	{
		p->earned_wins[FWINE] = true;
		has_new_win = true;
	}
	// 月见酒
	if (!p->earned_wins[MWINE] && p->win_mwine.size() == 2)
	{
		p->earned_wins[MWINE] = true;
		has_new_win = true;
	}
	// 短册
	if (!p->earned_wins[SBOOK] && p->win_sbook.size() == 5)
	{
		p->earned_wins[SBOOK] = true;
		has_new_win = true;
	}
	else if (p->win_sbook.size() > 5)
	{
		p->earned_wins[SBOOK] = true;
		if (p->win_sbook.size() > p->sbook_length)
		{
			has_new_win = true;
		}
	}
	// 种
	if (!p->earned_wins[SEED] && p->win_seed.size() == 5)
	{
		p->earned_wins[SEED] = true;
		has_new_win = true;
	}
	else if (p->win_seed.size() > 5)
	{
		p->earned_wins[SEED] = true;
		if (p->win_seed.size() > p->seed_length)
		{
			has_new_win = true;
		}
	}
	// 皮
	if (!p->earned_wins[SKIN] && p->win_skin.size() == 10)
	{
		p->earned_wins[SKIN] = true;
		has_new_win = true;
	}
	else if (p->win_skin.size() > 10)
	{
		p->earned_wins[SKIN] = true;
		if (p->win_skin.size() > p->skin_length)
		{
			has_new_win = true;
		}
	}
	// 月札
	if (!p->earned_wins[MONTH_CARDS] && p->win_monthcards.size() == 4)
	{
		p->earned_wins[MONTH_CARDS] = true;
		has_new_win = true;
	}

	if (has_new_win)
	{
		flow_queue.push_back(fs_koikoi);
		selected_koikoi = false;
		selected_end = false;
	}
	else
	{
		// 我现在这里把两个fs_end_turn改成一个试试...万恶的分号
		if (flow_queue.back() != fs_end_turn)
			flow_queue.push_back(fs_end_turn);
	}
	flow_queue.pop_front();
}

void Game::flow_koikoi()
{
	// 由于koikoi和end_turn两个状态必定是相联的，所以选择push_front
	// 这样由于两人都有手四而产生的同时入队两个连续的koikoi而用户还没有转换所带来的问题就解决了
	//ImGui::SFML::Update(window, delta_clock.restart());
	if (player_queue.front() != p2)
	{
		ImGui::Begin("", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
		// 列出所有的扎役
		// 关于役的可选设置，目前还没有做，就用字面量直接算
		for (int i = 0; i < 14; ++i)
		{
			if (player_queue.front()->earned_wins[i])
			{
				ImGui::Text(u8"%s  %d", cm.wins[i].name.c_str(), cm.wins[i].money);
			}
		}
		// 看看ImGui有分割线没，往这里插一个
		if (ImGui::Button("koikoi"))
		{
			flow_queue.pop_front();
			// 下面这个原来是push_front，不知道为什么，就改过来了
			flow_queue.push_back(fs_end_turn);
		}
		ImGui::SameLine();
		if (ImGui::Button(u8"结束"))
		{
			flow_queue.pop_front();
			flow_queue.push_back(fs_summary);
		}
		ImGui::End();
	}
	else
	{
		if (ai->determine_koikoi())
		{
			flow_queue.pop_front();
			flow_queue.push_back(fs_end_turn);
		}
		else
		{
			flow_queue.pop_front();
			flow_queue.push_back(fs_summary);
		}
	}
}

void Game::flow_summary()
{
	// TODO: 这里可以判断一下双方是否有扎役。如果没有就要考虑亲权
	int total = 0;
	for(int i=0; i<14; ++i)
	{
		if (player_queue.front()->earned_wins[i])
		{
			if (cm.wins[i].name == u8"短册" && player_queue.front()->sbook_extra()>0)
				total += cm.wins[i].money + player_queue.front()->sbook_length - 5;
			else if (cm.wins[i].name == u8"种" && player_queue.front()->seed_extra()>0)
				total += cm.wins[i].money + player_queue.front()->seed_length - 5;
			else if (cm.wins[i].name == u8"皮" && player_queue.front()->skin_extra()>0)
				total += cm.wins[i].money + player_queue.front()->skin_length - 5;
			else
				total += cm.wins[i].money;
		}
	}
	// 由于每个玩家是自己将自身加入队尾的，所以这里基本可以确定输家也在player_queue里面
	if(player_queue.back()->money >= total)
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
			cout << "-----------" << (player_queue.front() == p1 ? "p1" : "p2") << " round" << "-----------" << endl;
		cout << "log: entered " << str << endl;
	}
	flow_state_str = str;
}

void Game::put(Card* card)
{
	// 将选中的牌从手牌中移除
	remove_item(player_queue.front()->hand_cards, card);

	if (card)
	{
		switch (count_same_month(card->month))
		{
		case 0:
			put_to_field(card);
			flow_queue.push_back(fs_put_move_to_field);
			// 下面这个让move_to_field在结束时入队
			//flow_queue.push_back(p1_draw);
			flow_queue.pop_front();
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
			flow_queue.push_back(fs_put_move_to_target);
			flow_queue.pop_front();
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
			flow_queue.push_back(fs_select_put_target);
			flow_queue.pop_front();
			break;
		case 3:
			earned_cards.push_back(card);
			// 将四张得牌加入earned_cards
			for (list<Card*>::iterator it = field_cards.begin(); it != field_cards.end(); ++it)
			{
				if ((*it) && (*it)->month == card->month)
				{
					earned_cards.push_back(*it);
					// 然后要将它们从场牌消去
					//*it = nullptr;
				}
			}
			list<Card*>::iterator it = earned_cards.begin();
			// 将其余三张牌的移动目标都设置为第一个场牌中的得牌上方
			Card *target = *(++++it);
			for (it = earned_cards.begin(); it != earned_cards.end(); ++it)
			{
				if ((*it) != target)
				{
					(*it)->set_dest(target->get_upon_pos());
					moving_cards.push_back(*it);
				}
			}
			flow_queue.push_back(fs_put_move_to_target);
			flow_queue.pop_front();
			break;
		}
	}
}

void Game::select_target(Card* card)
{
	if (card && card->month == earned_cards.front()->month)
	{
		// 取消高亮
		for (list<Card*>::iterator it = field_cards.begin(); it != field_cards.end(); ++it)
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

void Game::select_put_target(Card* card)
{
	if (card && card->month == earned_cards.front()->month)
	{
		// 取消高亮
		for (list<Card*>::iterator it = field_cards.begin(); it != field_cards.end(); ++it)
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

void Game::select_draw_target(Card* card)
{
	// 其实完全可以把抽到的牌直接插到earned_cards的首位的，但为了容易理解，还是另用一个变量记录
	if (card && card->month == drawn_card->month && !in_list(earned_cards, card))
	{
		// 取消高亮
		for (list<Card*>::iterator it = field_cards.begin(); it != field_cards.end(); ++it)
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
