#include "SimpleChess.h"

namespace Chess {

	SimpleChess::SimpleChess() {
		previousState = NOTOVER;
	}
	
	void SimpleChess::play() {

		ValidMove vmove;
		IndexPair pair;

		while (true) {

			if (previousState != INVALIDMOVE) {
				board.printBoard();
				
				// if previous move was invalid we are still in the same move state
				// no need to update all possible moves to avoid computing the same thing again
				// update is done now when new move starts
				board.updateAllPossibleMoves();
				if (gameOver(board.gameStatus()))
					break;
			}
			else
				prompt.invalidMoveMsg();

			pair = prompt.getMove(board.getPlayerColor());
			vmove = board.findMove(pair);

			if (vmove.first) {
				if (vmove.second.move.byteData.type & PROMOTE)
					vmove.second.move.byteData.promote = prompt.getPromotionMove();
			}

			previousState = board.playNextMove(vmove, previousState);

		}

	}

	bool SimpleChess::gameOver(GameStatus status) {
		bool acceptedDraw;
		
		switch (status) {
		case CHECKMATE:
			prompt.checkMateMsg();
			prompt.playerWonMsg(board.oppositeSide());
			return true;
			break;
		case STALEMATE:
			prompt.drawByStalemateMsg();
			return true;
			break;	
		case DRAWBYREPETITION:
			acceptedDraw = prompt.askForDraw();
			if (acceptedDraw) {
				prompt.drawByRepetitionMsg();
				return true;
			}
			else
				return false;
			break;
		case DRAWBY50MOVES:
			prompt.drawBy50MovesMsg();
			return true;
			break;
		default:
			return false;
			break;
		}
	}

}
