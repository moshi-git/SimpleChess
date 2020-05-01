#ifndef PROMPT_H
#define PROMPT_H

#include "Types.h"

namespace Chess {

	using std::string;
	using std::regex;
	using std::cin;
	using std::cout;
	using std::endl;

	class MovePrompt {
	private:
		regex moveRegex { "[a-hA-H][1-8][a-hA-H][1-8]" };
		regex promotionRegex{ "[nNrRqQbB]" };
	private:
		bool moveStringValid(string move);
		bool promotionStringValid(string promotion);
	public:
		string readMove(Color player);
		IndexPair parseMove(string move);
		string readPromotionMove();
		Piece parsePromotionMove(string move);
		bool askForDraw();
		
		IndexPair getMove(Color playerColor) { return parseMove(readMove(playerColor)); }
		Piece getPromotionMove() { return parsePromotionMove(readPromotionMove()); }
		
		void playerWonMsg(Color player);

		void drawByRepetitionMsg() { cout << "Draw by threefold repetition" << endl; }
		void drawBy50MovesMsg() { cout << "Draw by 50 moves rule" << endl; }
		void drawByStalemateMsg() { cout << "Draw by stalemate" << endl; }
		void invalidMoveMsg() { cout << "Invalid move! Try again" << endl; }
		void checkMateMsg() { cout << "Checkmate" << endl; }

	};

}

#endif // PROMPT_H