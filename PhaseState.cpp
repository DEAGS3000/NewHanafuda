#include "PhaseState.h"
#include <iostream>
#include <imgui.h>
#include "utilities.h"

PhaseState::PhaseState()
{
    game = Game::Instance();
    expired = false;
}

void PhaseState::OnEnter()
{
    Log();
    expired = false;
}

void PhaseState::Update(sf::Time dt)
{
}

void PhaseState::OnExit()
{
    expired = true;
}

void PhaseState::Log()
{
    std::cout << "log: entered " << phase_name << std::endl;
}

void PrepareState::OnEnter()
{
    std::cout << "log: entered PrepareState" << std::endl;
}

void PrepareState::Update(sf::Time dt)
{
    // 决定亲子，0是p1为亲，1是p2为亲
    int temp = random(0, 1);
    if (temp == 0)
    {
        game->parent = game->p1;
        game->player_queue.push_back(game->p1);
        game->player_queue.push_back(game->p2);
    }
    else
    {
        game->parent = game->p2;
        game->player_queue.push_back(game->p2);
        game->player_queue.push_back(game->p1);
    }
    game->parent_sign.setPosition(game->player_queue.front()->get_parent_sign_pos());
    game->switch_state(fs_dispatch);
}

void DispatchState::OnEnter()
{
    PhaseState::OnEnter();
    Card *temp_card;
    for (int i = 0; i < 8; ++i)
    {
        // 按照子-场-亲顺序发牌，还要为Card对象设定好目标位置
        temp_card = game->draw_card();
        if (game->player_queue.back() == game->p2 && !DEBUG_SHOW_FACE)
            temp_card->show_back();
        //temp_card->visible = true;
        game->player_queue.back()->get(temp_card);
        game->moving_cards.push_back(temp_card);
        temp_card->set_dispatch_speed();

        temp_card = game->draw_card();
        //temp_card->visible = true;
        //field_cards.push_back(temp_card);
        game->put_to_field(temp_card);
        game->moving_cards.push_back(temp_card);
        temp_card->set_dispatch_speed();

        temp_card = game->draw_card();
        if (game->player_queue.front() == game->p2 && !DEBUG_SHOW_FACE)
            temp_card->show_back();
        //temp_card->visible = true;
        game->player_queue.front()->get(temp_card);
        game->moving_cards.push_back(temp_card);
        temp_card->set_dispatch_speed();
    }
}

void DispatchState::Update(sf::Time dt)
{
    // 发牌过程的操作可以改一下
    /*for(deque<Card*>::iterator it=moving_cards.begin(); it!=moving_cards.end(); )
				{
					(*it)->visible = true;
					(*it)->Update(time);
					if (!(*it)->moving)
						it = moving_cards.erase(it);
					else
						++it;
				}*/

    if (!game->moving_cards.empty())
    {
        game->move_cards(dt);
    }
    else
    {
        game->switch_state(fs_validate_game);
    }
}

void ValidateGameState::OnEnter()
{
    PhaseState::OnEnter();
    int month_count[12] = {0};

    for (std::list<Card *>::iterator it = game->field_cards.begin(); it != game->field_cards.end(); ++it)
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
            game->reset();
            return;
        }
    }
    game->switch_state(fs_precomplete);
}

void PrecompleteState::OnEnter()
{
    PhaseState::OnEnter();
    // 检查手四，先亲后子，以便应对双方都有手四的情况
    // TODO: 双手四的情况没考虑
    bool found_hand_four = false;
    int temp_month[12] = {0};
    for (auto it = game->player_queue.front()->hand_cards.begin();
         it != game->player_queue.front()->hand_cards.end(); ++it)
    {
        ++temp_month[(*it)->month - 1];
    }
    for (int i = 0; i < 12; ++i)
    {
        if (temp_month[i] == 4)
        {
            found_hand_four = true;
            game->player_queue.front()->earned_wins[HAND_FOUR] = true;
            game->switch_state(fs_koikoi);
            break;
        }
    }
    // 按理说，如果第一个玩家koikoi了，那么会进入下一个玩家的回合
    memset(temp_month, 0, sizeof(int) * 12);
    for (list<Card *>::iterator it = game->player_queue.back()->hand_cards.begin();
         it != game->player_queue.back()->hand_cards.end(); ++it)
    {
        ++temp_month[(*it)->month - 1];
    }
    for (int i = 0; i < 12; ++i)
    {
        if (temp_month[i] == 4)
        {
            found_hand_four = true;
            game->player_queue.back()->earned_wins[HAND_FOUR] = true;
            game->switch_state(fs_koikoi);
            break;
        }
    }

    if (!found_hand_four)
    {
        game->switch_state(fs_put);
    }
}

void PutState::Update(sf::Time dt)
{
    // 手牌打光，本局结束
    // 本段代码经测试效果基本正常
    if (game->player_queue.front()->hand_cards.empty())
    {
        game->switch_state(fs_summary);
        return;
    }

    // 由于有玩家队列，所以每次在流程函数中操作“玩家”就应该操纵队首。这样同样的流程对于两个玩家来说，就不用分两个流程状态了
    // 接受输入，选择手牌
    // 用l_button_pos查找p1手牌，找不到就不做事
    // 如果场牌没有可得牌，将所选牌加入场牌
    // 如果有1或3张可得牌，将p1_put_move_to_target入队
    // 如果场牌中有2张可得牌，就把p1_select_put_target入队
    if (game->player_queue.front() == game->p1)
    {
        Card *point_card = game->get_point_card(game->player_queue.front()->hand_cards);
        if (point_card)
        {
            for (auto &i : game->player_queue.front()->hand_cards)
                i->highlighted = false;
            point_card->highlighted = true;
        }
        else
        {
            // 如果鼠标没有停留在任意手牌，取消高亮
            for (auto &i : game->player_queue.front()->hand_cards)
                i->highlighted = false;
        }

        if (game->l_button_clicked)
        {
            // 打出了就取消高亮
            for (auto &i : game->player_queue.front()->hand_cards)
                i->highlighted = false;
            //Card *temp_card = get_point_card(player_queue.front()->hand_cards);
            game->put_ex(point_card);
        }
    }
    else
    {
        game->ai->calculate(game->field_cards);
        Card *put_card = game->ai->select_put();
        // 将ai出牌正面显示
        if (!DEBUG_SHOW_FACE)
            put_card->show_face();
        game->put_ex(put_card);
        game->ai->earned(put_card);
    }
}

void DrawState::OnEnter()
{
    PhaseState::OnEnter();
    if (game->heap.empty())
        cout << "错误！牌堆已空！" << endl;

    Card *temp_card = game->heap.front();
    game->heap.pop_front();
    temp_card->visible = true;
    // 将卡正面显示
    temp_card->show_face();
    // 无论如何在卡翻开后先等待一会儿，让用户看清楚
    // 这句似乎看不出明显效果
    //game->state_queue.pop_front();
    //state_queue.push_front(fs_wait_interval);

    switch (game->count_same_month(temp_card->month))
    {
    case 0:
        game->put_to_field(temp_card);
        game->switch_state(fs_draw_move_to_target, 0.5f);
        break;
    case 1:
        // TODO: 这里有一个问题，在p1_put_move_to_target里面，那个target该怎么记录？
        // 将两张得牌加入earned_cards
        game->earned_cards.push_back(temp_card);
        for (list<Card *>::iterator it = game->field_cards.begin(); it != game->field_cards.end(); ++it)
        {
            // 选到的牌必须没有被已经打出的牌占据
            if ((*it) && (*it)->month == temp_card->month && !in_list(game->earned_cards, (*it)))
            {
                game->earned_cards.push_back(*it);
                temp_card->set_dest((*it)->get_upon_pos());
                game->moving_cards.push_back(temp_card);
                //*it = nullptr;
                //field_cards.erase(it);
                break;
            }
        }
        game->switch_state(fs_draw_move_to_target, 0.5f);
        break;
    case 2:
        // TODO: 这里也有一样的问题，而且不能简单地用遍历解决
        // 由于被选中的只有1张牌，那就先考虑用一个变量记录好了
        game->drawn_card = temp_card;
        game->earned_cards.push_back(temp_card);
        // 预先将可选牌设置高亮
        for (list<Card *>::iterator it = game->field_cards.begin(); it != game->field_cards.end(); ++it)
        {
            if ((*it) && !in_list(game->earned_cards, (*it)) && (*it)->month == temp_card->month)
                (*it)->highlighted = true;
        }
        game->switch_state(fs_select_draw_target, 0.5f);
        break;
    case 3:
        game->earned_cards.push_back(temp_card);
        // 将四张得牌加入earned_cards
        for (list<Card *>::iterator it = game->field_cards.begin(); it != game->field_cards.end(); ++it)
        {
            if ((*it) && (*it)->month == temp_card->month)
            {
                game->earned_cards.push_back(*it);
                //*it = nullptr;
            }
        }
        //list<Card*>::iterator it = earned_cards.begin();
        // 将其余三张牌的移动目标都设置为第一个场牌中的得牌上方
        // 下面这个++++可能有问题，不是每次都能准确定位到第一张场牌
        //Card *target = *(++++it);
        Card *target = nullptr;
        for (auto card : game->field_cards)
        {
            if (!card)
                continue;
            if (card->month == temp_card->month)
            {
                target = card;
                break;
            }
        }
        for (auto &card : game->earned_cards)
        {
            if (card != target && card->month == target->month)
            {
                card->set_dest(target->get_upon_pos());
                game->moving_cards.push_back(card);
            }
        }
        game->switch_state(fs_draw_move_to_target, 0.5f);
        break;
    }
}

void SelectPutTargetState::OnEnter()
{
    PhaseState::OnEnter();
}

void SelectPutTargetState::Update(sf::Time dt)
{

    if (game->player_queue.front() != game->p2)
    {
        if (game->l_button_clicked)
        {
            Card *temp_card = game->get_point_card(game->field_cards);
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
				state_queue.push_back(fs_put_move_to_target);
				state_queue.pop_front();
			}*/
            game->select_put_target_ex(temp_card);
        }
    }
    else
    {
        // AI选牌
        Card *target = game->ai->select_put_target();
        game->select_put_target_ex(target);
        game->ai->earned(target);
    }
}

void SelectDrawTargetState::OnEnter()
{
    PhaseState::OnEnter();
}

void SelectDrawTargetState::Update(sf::Time dt)
{

    if (game->player_queue.front() != game->p2)
    {
        // 将可选牌高亮渲染
        if (game->l_button_clicked)
        {
            Card *temp_card = game->get_point_card(game->field_cards);
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
				state_queue.pop_front();
				state_queue.push_front(fs_draw_move_to_target);
			}*/
            select_draw_target(temp_card);
        }
    }
    else
    {
        // AI选牌
        Card *target = game->ai->select_draw_target(game->drawn_card, game->field_cards);
        select_draw_target(target);
        game->ai->earned(target);
    }
}

void SelectDrawTargetState::select_draw_target(Card *card)
{
    // TODO: 这里是对card的合法性再做一个判断，要与打出的牌同月才行。但earned_cards的顺序无法保证，与队首比较可能出问题而卡住
    if (card && card->month == game->drawn_card->month)
    {
        // 取消高亮
        for (list<Card *>::iterator it = game->field_cards.begin(); it != game->field_cards.end(); ++it)
        {
            if ((*it) && (*it)->month == card->month)
                (*it)->highlighted = false;
        }
        // 将p1_put_move_to_target入队，将出牌的目标设为所选牌的上方
        game->drawn_card->set_dest(card->get_upon_pos());
        game->moving_cards.push_back(game->drawn_card);
        game->earned_cards.push_back(card);
        //null_item(field_cards, temp_card);
        game->switch_state(fs_draw_move_to_target); // todo: 同样一个move状态即可
    }
}
void PutMoveToTargetState::Update(sf::Time dt)
{
    // 看看earned_cards中有几个元素，有2个就是场牌中有1或2张同月，有4个就是场牌中有3张同月
    if (!game->moving_cards.empty())
    {
        if (game->move_cards(dt))
            game->sound_put.play();
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
        // todo: 这里是流程队列和状态模式区别的重点，注意
        // 等待一段时间，好让用户看清楚
        //game->state_queue.push_back(game->state_dict[fs_draw]);
        //flow_queue.push_back(fs_wait_interval);
        //game->state_queue.push_back(game->state_dict[fs_put_get]);
        // 这里在draw开始时，put_get已经完成了，所以不用担心earned_cards冲突
        // 将put的全过程和draw的全过程彻底错开，放到put_get_move结束再入队
        //game->state_queue.pop_front();
        game->switch_state(fs_draw);
        // todo: 其实如果把put_get用队列实现并切换到队首，也可以
    }
}

// void DrawMoveToFieldState::Update(sf::Time dt)
// {
//     //PhaseState::Update(dt);
//     // draw_move_to_field之后当前玩家就没什么操作了，进行玩家队列操作，换下一个玩家
//     if (!game->moving_cards.empty())
//     {
//         if (game->move_cards(dt))
//             game->sound_put.play();
//     } else
//     {
//         // 重整双方手牌，此时earned_cards为空
//         game->player_queue.front()->format_cards();
//         //game->state_queue.push_back(game->state_dict[fs_wait_interval]);
//         //game->state_queue.push_back(game->state_dict[fs_detect_win]);
//         //game->state_queue.pop_front();
//         game->switch_state(fs_put_get, 0.5f);
//     }
// }

void DrawMoveToTargetState::Update(sf::Time dt)
{
    PhaseState::Update(dt);
    if (!game->moving_cards.empty())
    {
        if (game->move_cards(dt))
            game->sound_put.play();
    }
    else
    {
        //game->state_queue.push_back(game->state_dict[fs_draw_get]);
        game->switch_state(fs_put_get, 0.5f);
    }
}

void PutGetState::OnEnter()
{
    PhaseState::OnEnter();

    // FIXME: 且在这里先把drawn和putted清掉
    game->drawn_card = nullptr;
    game->put_card = nullptr;
    // 对earned_cards中的所有卡实行赢取操作
    for (list<Card *>::iterator it = game->earned_cards.begin(); it != game->earned_cards.end(); ++it)
    {
        game->moving_cards.push_back(*it);
        game->player_queue.front()->earn(*it, game->current_month);
        // 在这里再将牌从场牌消去
        null_item(game->field_cards, *it);
        // 这一步操作进行完以后，目的就已经达到了。各得牌列表有了真实的长度，可以算出正确的位置和便宜
        // 同时新的得牌也已经在列表中，可以用format函数中生成的各种位置信息，为没有moving的原得牌设置pos
        // 为有moving的新得牌设置dest。moving属性会在移动结束后消去，所以在移动中，渲染得牌部分时也不渲染有moving的
    }
    // 重整双方手牌
    game->player_queue.front()->format_cards();
    // 由于这个状态对于手牌出牌和抽牌出牌是通用的，所以转换状态之前要清空earned_cards
    game->earned_cards.clear();
}

void PutGetState::Update(sf::Time dt)
{
    PhaseState::Update(dt);
    if (!game->moving_cards.empty())
    {
        game->move_cards(dt);
    }
    else
    {
        game->switch_state(fs_detect_win);
    }
}

void DrawGetState::OnEnter()
{
    PhaseState::OnEnter();
    // 对earned_cards中的所有卡实行赢取操作
    for (list<Card *>::iterator it = game->earned_cards.begin(); it != game->earned_cards.end(); ++it)
    {
        game->moving_cards.push_back(*it);
        game->player_queue.front()->earn(*it, game->current_month);
        // 在这里再将牌从场牌消去
        null_item(game->field_cards, *it);
    }
    // 重整双方手牌
    game->player_queue.front()->format_cards();
    // 由于这个状态对于手牌出牌和抽牌出牌是通用的，所以转换状态之前要清空earned_cards
    game->earned_cards.clear();
}

void DrawGetState::Update(sf::Time dt)
{
    PhaseState::Update(dt);
    if (!game->moving_cards.empty())
    {
        game->move_cards(dt);
    }
    else
    {
        game->state_queue.push_back(game->state_dict[fs_detect_win]);
        game->state_queue.pop_front();
    }
}

// void PutMoveToFieldState::Update(sf::Time dt)
// {
//     if (!game->moving_cards.empty())
//     {
//         if (game->move_cards(dt))
//             game->sound_put.play();
//     } else
//     {
//         // 重整双方手牌，此时earned_cards为空
//         game->player_queue.front()->format_cards();
//         //game->flow_queue.push_back(fs_draw);
//         //game->flow_queue.pop_front();
//         game->switch_state(fs_draw);
//     }
// }

void CheckWinState::OnEnter()
{
    PhaseState::OnEnter();
    bool has_new_win = false;
    Player *p = game->player_queue.front();
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
    if (!p->earned_wins[FOUR_LIGHT] && !in_list(p->earned_light, &game->all_cards[40]) && p->earned_light.size() == 4)
    {
        p->earned_wins[FOUR_LIGHT] = true;
        // 取消三光
        p->earned_wins[THREE_LIGHT] = false;
        has_new_win = true;
    }
    // 雨四光
    if (!p->earned_wins[RAIN_FOUR_LIGHT] && in_list(p->earned_light, &game->all_cards[40]) &&
        p->earned_light.size() == 4)
    {
        p->earned_wins[RAIN_FOUR_LIGHT] = true;
        // 取消三光
        p->earned_wins[THREE_LIGHT] = false;
        has_new_win = true;
    }
    // 三光
    if (!p->earned_wins[THREE_LIGHT] && !in_list(p->earned_light, &game->all_cards[40]) && p->earned_light.size() == 3)
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
    if (!p->earned_wins[SBOOK] && p->win_sbook.size() >= 5)
    {
        p->earned_wins[SBOOK] = true;
        has_new_win = true;
        p->last_koikoi_sbook_length = p->win_sbook.size();
    }
    else if (p->win_sbook.size() > 5)
    {
        //p->earned_wins[SBOOK] = true;
        if (p->win_sbook.size() > p->last_koikoi_sbook_length)
        {
            has_new_win = true;
            p->last_koikoi_sbook_length = p->win_sbook.size();
        }
    }
    // 种
    if (!p->earned_wins[SEED] && p->win_seed.size() >= 5)
    {
        p->earned_wins[SEED] = true;
        has_new_win = true;
        p->last_koikoi_seed_length = p->win_seed.size();
    }
    else if (p->win_seed.size() > 5)
    {
        //p->earned_wins[SEED] = true;
        if (p->win_seed.size() > p->last_koikoi_seed_length)
        {
            has_new_win = true;
            p->last_koikoi_seed_length = p->win_seed.size();
        }
    }
    // 皮
    if (!p->earned_wins[SKIN] && p->win_skin.size() >= 10)
    {
        p->earned_wins[SKIN] = true;
        has_new_win = true;
        p->last_koikoi_skin_length = p->win_seed.size();
    }
    else if (p->win_skin.size() > 10)
    {
        //p->earned_wins[SKIN] = true;
        if (p->win_skin.size() > p->last_koikoi_skin_length)
        {
            has_new_win = true;
            p->last_koikoi_skin_length = p->win_seed.size();
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
        //game->state_queue.push_back(game->state_dict[fs_koikoi]);
        game->switch_state(fs_koikoi);
        game->selected_koikoi = false;
        game->selected_end = false;
    }
    else
    {
        // todo: 下面这段注意，会有什么问题
        game->switch_state(fs_end_turn);
        // 我现在这里把两个fs_end_turn改成一个试试...万恶的分号
        //        if (game->state_queue.back() != game->state_dict[fs_end_turn])
        //            game->state_queue.push_back(game->state_dict[fs_end_turn]);
    }
    //game->state_queue.pop_front();
}

void KoikoiState::OnEnter()
{
    PhaseState::OnEnter();
}

void KoikoiState::Update(sf::Time dt)
{
    // 当玩家手牌已经出完时，没有koikoi选项，直接summary
    if (game->player_queue.front()->hand_cards.empty())
    {
        game->switch_state(fs_summary);
    }
    else
    {
        if (game->selected_koikoi)
        {
            game->switch_state(fs_end_turn);
            game->selected_koikoi = false;
        }
        if (game->selected_end)
        {
            game->switch_state(fs_summary);
            game->selected_koikoi = false;
        }
    }
}

void SummaryState::OnEnter()
{
    PhaseState::OnEnter();
    // TODO: 这里可以判断一下双方是否有扎役。如果没有就要考虑亲权
    int total = 0;
    for (int i = 0; i < 14; ++i)
    {
        if (game->player_queue.front()->earned_wins[i])
        {
            if (cm.wins[i].name == u8"短册" && game->player_queue.front()->sbook_extra() > 0)
                total += cm.wins[i].money + game->player_queue.front()->win_sbook.size() - 5;
            else if (cm.wins[i].name == u8"种" && game->player_queue.front()->seed_extra() > 0)
                total += cm.wins[i].money + game->player_queue.front()->win_seed.size() - 5;
            else if (cm.wins[i].name == u8"皮" && game->player_queue.front()->skin_extra() > 0)
                total += cm.wins[i].money + game->player_queue.front()->win_skin.size() - 5;
            else
                total += cm.wins[i].money;
        }
    }
    // 由于每个玩家是自己将自身加入队尾的，所以这里基本可以确定输家也在player_queue里面
    if (game->player_queue.back()->money >= total)
    {
        game->player_queue.front()->money += total;
        game->player_queue.back()->money -= total;
    }
    else
    {
        game->player_queue.front()->money += game->player_queue.back()->money;
        game->player_queue.back()->money = 0;
    }
}

void SummaryState::Update(sf::Time dt)
{
    
}

void WaitIntervalState::Update(sf::Time dt)
{
    game->interval_waited += dt;
    if (game->interval_waited.asSeconds() >= WAIT_INTERVAL)
    {
        // 计时器清零
        game->interval_waited = sf::Time::Zero;
        game->state_queue.pop_front();
    }
}

void EndTurnState::OnEnter()
{
    PhaseState::OnEnter();
    game->player_queue.push_back(game->player_queue.front());
    game->player_queue.pop_front();
    game->switch_state(fs_put);
}
