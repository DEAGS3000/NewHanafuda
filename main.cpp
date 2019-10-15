#include "Game.h"
#include "external_declare.h"


int main()
{
	Game* game = Game::Instance();
	game->loop();
	return 0;
}