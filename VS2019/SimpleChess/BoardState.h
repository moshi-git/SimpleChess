#ifndef BOARD_H
#define BOARD_H

#include "MovePrompt.h"

namespace Chess {

        class BoardState {
        private:
			// tracks the position of the pieces on the board
			// only distinushes between the pieces by color
			// black and white
			int colors[128];

			// tracks the position of the pieces on the board
			// only distinushes between the type of the piece
			// pawn, king, ...
			int pieces[128];

			// number of directions in which each piece except pawn can move around
			// knight - 8 , can leap in all eight directions
			// bishop - 4 , all four diagonal directions
			// rook - 4 , up and down, left and right
			// queen - 8 , bishop + rook
			// king - 8 , all 8 squares around the king
			int directions[5];

			// offset for every piece except pawn, 6 of them
			// in every direction, 8 directions
			// around the square
			int offset[5][8];

			// distinguish from sliding and nonsliding pieces
			bool sliding[5];

			// who has castling rights
			int castlingRights;

			// index of enpassant square
			// if white plays for example e2e4
			// enpassant square is e3 since black can
			// capture if there is a black pawn on d4 or f4
			int enpassantSquare;

			// black or white
			Color currentPlayer;

			// if there is no capture or pawn capture when move
			// increment and after 50 moves it is a draw by 50 moves rule
			// every time there is a pawn moved or a capturing move reset
			int fiftyMoves;
		
			// represents all possible pseudo legal moves in a current turn for a player
			Moves allPossibleMoves;

			// random starting values for zobrist hash
			// 2 for both black and white
			// 6 for each piece type (pawn, rook ...)
			// 64 for every board postiion
			BoardStateHash piecesHash[2][6][128];

			// for every square enpassant random starting hash values
			BoardStateHash enpassantHash[128];

			// for player side random starting hash value
			BoardStateHash playerColorHash;

			// keep move history as zobrist hash values
			// used to detect threefold repetition
			MoveHistory history;

			// if threefold repetition is detected in this turn
			bool threefoldRep;

			// from index (64 values)
			// to index (64 values)
			// each entry represents hash value 
			// hash = fromRank * 16 + fromFile (range for "to" from 0-15)
			// same for toRank and toFile, min and max index 0-127 for both
			int fromToHashTable[128][128];

			int whiteKingSquare;

			int blackKingSquare;

			int pawnsSquares[2][8];

			CastlingMask castling;

		private:
			void initColors();
			void initPieces();
			void initDirections();
			void initOffset();
			void initSliding();
			void initZobristHash();

			char pieceToChar(Piece piece, Color color);

			bool underAttackByKnight(int squareIndex, Color attackingSide);
			bool underAttackByKing(int squareIndex, Color attackingSide);
			bool underAttackByRookOrQueen(int squareIndex, Color attackingSide);
			bool underAttackByBishopOrQueen(int squareIndex, Color attackingSide);

		public:
			BoardState();
			Color getPlayerColor() { return currentPlayer; }
			int boardIndexToSquareIndex(int row, int column) { return row * 16 + column; }
			bool validSquareIndex(int squareIndex) { return 0x88 & squareIndex ? false : true; }
			int getRank(int squareIndex) { return squareIndex >> 4; } 
			int getFile(int squareIndex) { return squareIndex & 0x7; }
			// https://www.chessprogramming.org/Move_Generation
			// Offset Move Generation
			// https://www.chessprogramming.org/10x12_Board#OffsetMG
			// generate pseudo legal moves and check if the move
			// that player wants to play is in the generated moves
			// return all generated pseudo moves for a player in current turn
			Moves generateAllPossibleMoves();
			// https://www.chessprogramming.org/Zobrist_Hashing
			BoardStateHash zobristHash();
			bool kingInCheck();
			bool squareUnderAttack(int squareIndex, Color attackingSide);
			// try to play move
			// signal if it's invalid
			void playMove(Move move);
			bool testPseudoLegalMove(Move move);
			GameStatus gameStatus();
			Color oppositeSide() { return currentPlayer == WHITE ? BLACK : WHITE; }
			// try to play move and transfer into next move state
			GameStatus playNextMove(ValidMove move, GameStatus previousGameStateStatus);
			ValidMove findMove(IndexPair fromToPair);
			void printBoard();
			void updateAllPossibleMoves();
        };
}

#endif // BOARD_H
