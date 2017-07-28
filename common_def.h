#pragma once
#include <SFML/Graphics.hpp>


#define WINDOW_WIDTH 1400
#define WINDOW_HEIGHT 900
#define ZOOM_LEVEL 0.3f
#define CARD_SIZE_X 415 * ZOOM_LEVEL
#define CARD_SIZE_Y 666 * ZOOM_LEVEL
#define BLANK_SIZE CARD_SIZE_X / 12
#define HEAP_POS_X CARD_SIZE_X/2
#define HEAP_POS_Y WINDOW_HEIGHT/2-CARD_SIZE_Y/2
#define FIELD_ORIGIN_X CARD_SIZE_X*2
#define FIELD_ORIGIN_Y WINDOW_HEIGHT/2-CARD_SIZE_Y - BLANK_SIZE
#define START_MONEY 30
#define PARENT_SIGN_SIZE 150
#define CARD_OFFSET_LIMIT CARD_SIZE_X / 3.0f
// 等待间隔
#define WAIT_INTERVAL 0.5f
// 卡牌移动时间
#define MOVING_TIME 0.2f
#define DISPATCHING_TIME 0.05f

#define FIVE_LIGHT 0
#define FOUR_LIGHT 1
#define RAIN_FOUR_LIGHT 2
#define THREE_LIGHT 3
#define PDB 4
#define REDS 5
#define PURPLES 6
#define FWINE 7
#define MWINE 8
#define SBOOK 9
#define SEED 10
#define SKIN 11
#define MONTH_CARDS 12
#define HAND_FOUR 13

// 调试信息
#define DEBUG_SHOW_FACE false

enum CardType
{
	ct_light, ct_short, ct_seed, ct_skin
};

// 卡的定义
struct card_info
{
	int no;
	int month;
	std::string name;
	CardType type;
	bool visible;
};

struct win_info
{
	std::string name;
	int money;
	std::vector<int> cards;
};

// 游戏状态，gs是GameState的缩写
enum GameState
{
	gs_main_menu, gs_playing
};

// 流程状态，fs是FlowState的缩写
enum FlowState
{
	// 准备
	fs_prepare,
	// 发牌
	fs_dispatch,
	// 发牌移动
	fs_dispatch_moving,
	// 检查流局
	fs_validate_game,
	// 扎役预判：手四
	fs_precomplete,
	// 玩家1_选择出牌
	fs_put,
	// 玩家1_抽牌
	fs_draw,
	// 玩家1_选择出牌目标(可选)
	fs_select_put_target,
	// 玩家1_选择抽牌目标(可选)
	fs_select_draw_target,
	// 玩家1_出牌移动到场
	fs_put_move_to_field,
	// 玩家1_抽牌移动到场
	fs_draw_move_to_field,
	// 玩家1_得牌
	fs_put_get,
	fs_draw_get,
	// 玩家1_出牌移动到目标
	fs_put_move_to_target,
	// 玩家1_抽牌移动到目标
	fs_draw_move_to_target,
	// 玩家1_得牌移动到得牌区
	fs_put_get_moving,
	fs_draw_get_moving,
	// 玩家1_koikoi
	fs_detect_win,
	fs_koikoi,
	fs_end_turn,
	// 结算
	fs_summary,
	// 游戏结束
	fs_end_game,
	// 等待间隔
	fs_wait_interval
};
