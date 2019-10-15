//
// Created by 张诚伟 on 2019/10/14.
//

#ifndef NEWHANAFUDA_TIMER_H
#define NEWHANAFUDA_TIMER_H
#include <functional>
#include <list>
#include <SFML/System/Time.hpp>


class TimerTask
{
public:
    float time_limit;
    std::function<void()> callback;
    TimerTask(float in_time, std::function<void()> callback)
    {
        time_limit = in_time;
        this->callback=callback;
    }
};

class Timer
{
public:
    std::list<TimerTask*> all_tasks;
    float time_elapsed;  // 单位为秒

    Timer();
    void Add(float in_time, std::function<void()> callback);
    void Update(sf::Time dt);

};


#endif //NEWHANAFUDA_TIMER_H
