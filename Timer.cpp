//
// Created by 张诚伟 on 2019/10/14.
//

#include "Timer.h"

Timer::Timer()
{
    time_elapsed = 0.0f;
}

void Timer::Update(sf::Time dt)
{
    if (all_tasks.empty())
        return;

    time_elapsed += dt.asSeconds();

    if (time_elapsed > all_tasks.front()->time_limit)
    {
        all_tasks.front()->callback();
        delete all_tasks.front();
        all_tasks.pop_front();
        time_elapsed = 0.0f;
    }
}

void Timer::Add(float in_time, std::function<void()> callback)
{
    all_tasks.push_back(new TimerTask(in_time, callback));
}


