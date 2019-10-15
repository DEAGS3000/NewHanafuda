#pragma once
#include "Game.h"

#define CONSTRUCTOR_WITH_TYPE(__TYPE__) /*__TYPE__::*/__TYPE__(){ phase_name=#__TYPE__;}

class Game;

class PhaseState
{
public:
    Game *game;
    std::string phase_name;
    bool expired;
    PhaseState();

    virtual void OnEnter();

    virtual void Update(sf::Time dt);

    virtual void OnExit();

    void Log();
};

class PrepareState : public PhaseState
{
public:
    CONSTRUCTOR_WITH_TYPE(PrepareState);
    void OnEnter() override;

    void Update(sf::Time dt) override;
};

class DispatchState : public PhaseState
{
public:
    CONSTRUCTOR_WITH_TYPE(DispatchState);
    void OnEnter() override;
    void Update(sf::Time dt) override;
};

class ValidateGameState : public PhaseState
{
public:
    CONSTRUCTOR_WITH_TYPE(ValidateGameState);
    void OnEnter() override;
};

class PrecompleteState : public PhaseState
{
public:
    CONSTRUCTOR_WITH_TYPE(PrecompleteState);
    void OnEnter() override;
};

class PutState : public PhaseState
{
public:
    CONSTRUCTOR_WITH_TYPE(PutState);
    //void OnEnter() override;

    void Update(sf::Time dt);
};

class DrawState : public PhaseState
{
public:
    CONSTRUCTOR_WITH_TYPE(DrawState);
    void OnEnter() override;
};
class SelectPutTargetState : public PhaseState
{
public:
    CONSTRUCTOR_WITH_TYPE(SelectPutTargetState);
    void OnEnter() override;

    void Update(sf::Time dt) override ;
};

class SelectDrawTargetState : public PhaseState
{
public:
    CONSTRUCTOR_WITH_TYPE(SelectDrawTargetState);
    void OnEnter() override;
    void select_draw_target(Card* card);

    void Update(sf::Time dt) override ;
};

class PutMoveToFieldState: public PhaseState
{
public:
    void Update(sf::Time dt) override;
};

class PutMoveToTargetState : public PhaseState
{
public:
    CONSTRUCTOR_WITH_TYPE(PutMoveToTargetState);
    void Update(sf::Time dt) override;
};

class DrawMoveToFieldState : public PhaseState
{
public:
    void Update(sf::Time dt) override;
};

class DrawMoveToTargetState : public PhaseState
{
public:
    CONSTRUCTOR_WITH_TYPE(DrawMoveToTargetState);
    void Update(sf::Time dt) override;
};

class PutGetState : public PhaseState
{
public:
    CONSTRUCTOR_WITH_TYPE(PutGetState);

    void OnEnter() override;
    void Update(sf::Time dt) override;
};

class DrawGetState : public PhaseState
{
public:
    void OnEnter() override ;
    void Update(sf::Time dt) override;
};

class EndTurnState : public PhaseState
{
public:
    CONSTRUCTOR_WITH_TYPE(EndTurnState);
    void OnEnter() override;
};

class CheckWinState : public PhaseState
{
public:
    CONSTRUCTOR_WITH_TYPE(CheckWinState);
    void OnEnter() override;
};

class KoikoiState : public PhaseState
{
public:
    CONSTRUCTOR_WITH_TYPE(KoikoiState);
    void OnEnter() override;

    void Update(sf::Time dt) override;
};

class SummaryState : public PhaseState
{
public:
    CONSTRUCTOR_WITH_TYPE(SummaryState);
    void OnEnter() override;

    void Update(sf::Time dt) override;
};

class EndGameState : public PhaseState
{
};

class WaitIntervalState : public PhaseState
{
public:
    CONSTRUCTOR_WITH_TYPE(WaitIntervalState);
    void Update(sf::Time dt) override;
};