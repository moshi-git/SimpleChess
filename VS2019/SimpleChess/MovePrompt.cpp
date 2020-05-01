#include "MovePrompt.h"

namespace Chess {

	bool MovePrompt::moveStringValid(string move) {
		if (regex_match(move, moveRegex))
			return true;
		return false;
	}

	bool MovePrompt::promotionStringValid(string promotion)	{
		if (regex_match(promotion, promotionRegex))
			return true;
		return false;
	}

	string MovePrompt::readMove(Color player) {
		string move;

		string currentPlayer;

		if (player == WHITE)
			currentPlayer = "White";
		else
			currentPlayer = "Black";

		cout << currentPlayer << ": ";
		cin >> move;

		while (!moveStringValid(move)) {
			cout << "Invalid format! Try again." << endl;
			cout << "(hint: e2e4 or E2E4)" << endl;

			cout << currentPlayer << ": ";
			cin >> move;
		}

		return move;
	}

	IndexPair MovePrompt::parseMove(string move) {
		IndexPair pair;
		char fromChar = tolower(move.at(0));
		char toChar = tolower(move.at(2));
		int from = ((move.at(1) - '0') - 1) * 16 + (fromChar - 'a');
		int to = ((move.at(3) - '0') - 1) * 16 + (toChar - 'a');
		pair.first = from;
		pair.second = to;

		return pair;
	}

	string MovePrompt::readPromotionMove() {
		string move;

		cout << "Queen (q or Q)" << endl;
		cout << "Rook (r or R)" << endl;
		cout << "Bishop (b or B)" << endl;
		cout << "Knight (n or N)" << endl;

		cout << "Select piece for promotion: ";
		cin >> move;

		while (!promotionStringValid(move)) {
			cout << "Invalid format! Try again." << endl;
			cout << "Select piece for promotion: ";
			cin >> move;

		}

		return move;

	}

	Piece MovePrompt::parsePromotionMove(string move) {
		Piece piece;
		char promotionPiece = tolower(move.at(0));

		switch (promotionPiece) {
			case 'q':
				piece = QUEEN;
				break;
			case 'n':
				piece = KNIGHT;
				break;
			case 'b':
				piece = BISHOP;
				break;
			case 'r':
				piece = ROOK;
				break;
			default:
				piece = QUEEN;
		}

		return piece;

	}

	bool MovePrompt::askForDraw() {
		char draw;

		cout << "You can claim draw by threefold repetition!" << endl;
		cout << "If you decline now you will be asked again if the same position" << endl;
		cout << "occurs again or if another position gets repeated 3 or more times" << endl;
		cout << "(yY) for Yes or anything other for No" << endl;

		cin >> draw;

		draw = tolower(draw);

		switch (draw) {
		case 'y':
			return true;
			break;
		default:
			return false;
			break;
		}

	}

	void MovePrompt::playerWonMsg(Color player) {
		string p = player == WHITE ? "White" : "Black";
		cout << p << " is the WINNER!" << endl;
	}

}