#ifndef CHESS_H
#define CHESS_H

#include "BoardState.h"

namespace Chess {

	class SimpleChess {
	private:
		BoardState board;
		GameStatus previousState;
		// used to read moves from standard input
		MovePrompt prompt;
	
	public:
		SimpleChess();
		void play();
		bool gameOver(GameStatus status);

	};
}

#endif // CHESS_H

