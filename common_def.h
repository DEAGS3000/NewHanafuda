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
// �ȴ����
#define WAIT_INTERVAL 0.5f
// �����ƶ�ʱ��
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

// ������Ϣ
#define DEBUG_SHOW_FACE false

enum CardType
{
	ct_light, ct_short, ct_seed, ct_skin
};

// ���Ķ���
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

// ��Ϸ״̬��gs��GameState����д
enum GameState
{
	gs_main_menu, gs_playing
};

// ����״̬��fs��FlowState����д
enum FlowState
{
	// ׼��
	fs_prepare,
	// ����
	fs_dispatch,
	// �����ƶ�
	fs_dispatch_moving,
	// �������
	fs_validate_game,
	// ����Ԥ�У�����
	fs_precomplete,
	// ���1_ѡ�����
	fs_put,
	// ���1_����
	fs_draw,
	// ���1_ѡ�����Ŀ��(��ѡ)
	fs_select_put_target,
	// ���1_ѡ�����Ŀ��(��ѡ)
	fs_select_draw_target,
	// ���1_�����ƶ�����
	fs_put_move_to_field,
	// ���1_�����ƶ�����
	fs_draw_move_to_field,
	// ���1_����
	fs_put_get,
	fs_draw_get,
	// ���1_�����ƶ���Ŀ��
	fs_put_move_to_target,
	// ���1_�����ƶ���Ŀ��
	fs_draw_move_to_target,
	// ���1_�����ƶ���������
	fs_put_get_moving,
	fs_draw_get_moving,
	// ���1_koikoi
	fs_detect_win,
	fs_koikoi,
	fs_end_turn,
	// ����
	fs_summary,
	// ��Ϸ����
	fs_end_game,
	// �ȴ����
	fs_wait_interval
};
