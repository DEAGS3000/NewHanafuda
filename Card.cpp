#include "Card.h"
#include "common_def.h"
#include "extern_declare.h"


Card::Card()
{
	visible = false;
	pos = { 0, 0 };
	speed = { 0, 0 };
	dest = { 0, 0 };
	moving = false;
	highlighted = false;
	earned = false;
}

Card::~Card()
{
}

void Card::update(sf::Time time)
{
	if (static_cast<int>(pos.x) != static_cast<int>(dest.x) || static_cast<int>(pos.y) != static_cast<int>(dest.y))
	{
		// 我觉得两次调用asSeconds之间会有时间差，就先记下来
		float temp_second = time.asSeconds();
		// 前向判断，用于避免速度过快直接冲过目标点。
		// 如果本次位置更新后，与目标点的相对方位发生了变化，说明本次移动会冲过目标点，那就直接把位置设为目标点
		float direction_x, direction_y, next_direction_x, next_direction_y;
		// 这里要判断除数是否为0
		if (pos.x == dest.x)
			direction_x = 0;
		else
			direction_x = (dest.x - pos.x) / abs(dest.x - pos.x);
		if (pos.y == dest.y)
			direction_y = 0;
		else
			direction_y = (dest.y - pos.y) / abs(dest.y - pos.y);
		// 加个修正环节，如果距离干脆小于1，直接归位
		/*if (abs(dest.x - pos.x) <= 1 && abs(dest.y - pos.y) <= 1)
		{
			pos = dest;
			moving = false;
		}*/
		//else
		//{
			// 在此正常更新位置
		pos.x += speed.x * temp_second;
		pos.y += speed.y * temp_second;

		next_direction_x = (pos.x==dest.x)?0:((dest.x - pos.x) / abs(dest.x - pos.x));
		next_direction_y = (pos.y==dest.y)?0:((dest.y - pos.y) / abs(dest.y - pos.y));

		if (direction_x != next_direction_x || direction_y != next_direction_y)
		{
			pos = dest;
			moving = false;
		}
		//}

		// 应用更新sprite位置
		update_pos();
	}
	else
	{
		moving = false;
	}
}

void Card::set_dest(sf::Vector2<float> p)
{
	dest.x = p.x;
	dest.y = p.y;
	speed.x = (dest.x - pos.x) / MOVING_TIME;
	speed.y = (dest.y - pos.y) / MOVING_TIME;

	// 不知道会有什么影响，先写上
	moving = true;
}

void Card::set_dispatch_speed()
{
	speed.x = (dest.x - pos.x) / DISPATCHING_TIME;
	speed.y = (dest.y - pos.y) / DISPATCHING_TIME;
}

void Card::set_pos(sf::Vector2f p)
{
	pos = p;
	sprite.setPosition(pos);
}

void Card::update_pos()
{
	sprite.setPosition(pos);
}

sf::Vector2f Card::get_upon_pos()
{
	return{ pos.x + 10, pos.y + 10 };
}

void Card::show_face()
{
	sprite.setTexture(cm.card_texture[no]);
}

void Card::show_back()
{
	sprite.setTexture(cm.card_backend);
}
