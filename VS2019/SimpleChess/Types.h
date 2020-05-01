#ifndef TYPES_H
#define TYPES_H

#include <cinttypes>
#include <unordered_map>
#include <regex> 
#include <string>
#include <iostream>
#include <ctype.h>

namespace Chess {

	enum Color : int { WHITE, BLACK };
	enum Piece : int { KNIGHT, BISHOP, ROOK, QUEEN, KING, PAWN };
	// EMPTY - Invalid Square, square out of bounds

	// 0x88 table and squares representation
	enum x88Square : int {
		EMPTY = -1,
		x88_A1 = 0, x88_B1, x88_C1, x88_D1, x88_E1, x88_F1, x88_G1, x88_H1,
		x88_A2 = 16, x88_B2, x88_C2, x88_D2, x88_E2, x88_F2, x88_G2, x88_H2,
		x88_A3 = 32, x88_B3, x88_C3, x88_D3, x88_E3, x88_F3, x88_G3, x88_H3,
		x88_A4 = 48, x88_B4, x88_C4, x88_D4, x88_E4, x88_F4, x88_G4, x88_H4,
		x88_A5 = 64, x88_B5, x88_C5, x88_D5, x88_E5, x88_F5, x88_G5, x88_H5,
		x88_A6 = 80, x88_B6, x88_C6, x88_D6, x88_E6, x88_F6, x88_G6, x88_H6,
		x88_A7 = 96, x88_B7, x88_C7, x88_D7, x88_E7, x88_F7, x88_G7, x88_H7,
		x88_A8 = 112, x88_B8, x88_C8, x88_D8, x88_E8, x88_F8, x88_G8, x88_H8
	};

	enum Direction : int {
		NORTH = 16,
		SOUTH = -16,
		EAST = 1,
		WEST = -1,
		SOUTHEAST = SOUTH + EAST,
		SOUTHWEST = SOUTH + WEST,
		NORTHEAST = NORTH + EAST,
		NORTHWEST = NORTH + WEST,
		SOUTHSOUTH = SOUTH + SOUTH,
		NORTHNORTH = NORTH + NORTH,
		KNIGHTNORTHEAST = NORTHNORTH + EAST,
		KNIGHTNORTHWEST = NORTHNORTH + WEST,
		KNIGHTEASTNORTHEAST = NORTHEAST + EAST,
		KNIGHTWESTNORTHWEST = NORTHWEST + WEST,
		KNIGHTEASTSOUTHEAST = SOUTHEAST + EAST,
		KNIGHTWESTSOUTHWEST = SOUTHWEST + WEST,
		KNIGHTSOUTHEAST = SOUTHSOUTH + EAST,
		KNIGHTSOUTHWEST = SOUTHSOUTH + WEST,
	};
	
	//https://www.chessprogramming.org/Encoding_Moves
	enum MoveType : int { 
		NORMALMOVE = 0,
		CAPTURE = 1, 
		CASTLE = 2, 
		ENPASSANT = 4, 
		PAWNPUSH2 = 8, 
		PAWNPUSH = 16,
		PROMOTE = 32,
	};

	enum Castling : int {
		WHITEKINGSIDE = 1,
		WHITEQUEENSIDE = 2,
		BLACKKINGSIDE = 4,
		BLACKQUEENSIDE = 8,
	};

	enum Rank : int {
		RANK1 = x88_A1 >> 4,
		RANK2 = x88_A2 >> 4,
		RANK3 = x88_A3 >> 4,
		RANK4 = x88_A4 >> 4,
		RANK5 = x88_A5 >> 4,
		RANK6 = x88_A6 >> 4,
		RANK7 = x88_A7 >> 4,
		RANK8 = x88_A8 >> 4
	};

	enum File : int {
		FILEA = x88_A1 & 7,
		FILEB = x88_B1 & 7,
		FILEC = x88_C1 & 7,
		FILED = x88_D1 & 7,
		FILEE = x88_E1 & 7,
		FILEF = x88_F1 & 7,
		FILEG = x88_G1 & 7,
		FILEH = x88_H1 & 7
	};

	enum GameStatus : int {
		CHECKMATE,
		STALEMATE,
		DRAWBYREPETITION,
		DRAWBY50MOVES,
		INVALIDMOVE,
		NOTOVER
	};

	union MoveUnion	{
		struct {
			char from; // index from
			char to; // index to
			char promote; // bit code of a move in which is promoted
			char type; // move type
			// type represent bit field and several flags from MoveType
			// can be combined to represent a type of move
			// for example CAPTURE | PAWN
			// represents that pawn was moved and capture was made 
		} byteData;
		int byteContainer; // access all previous byte fields at once for comparison
	};

	struct Move {
		MoveUnion move;
		bool operator == (Move m) {
			return move.byteContainer == m.move.byteContainer;
		}

		bool operator < (Move m) {
			return move.byteContainer < m.move.byteContainer;
		}

		bool operator > (Move m) {
			return move.byteContainer > m.move.byteContainer;
		}
	};

	using Move = struct Move;
	using Moves = std::unordered_map<int, Move>;

	using BoardStateHash = uint64_t;
	using MoveHistory = std::unordered_map<BoardStateHash, int>;

	// (fromFile, fromRank)
	// to (toFile, toRank)
	// from = fromRank * 16 + fromFile
	// to = toRank * 16 + toFile
	using IndexPair = std::pair<int, int>; 
	
	// first memeber represents if a move is valid
	// second is the value of that move
	using ValidMove = std::pair<bool, Move>;

	// for square as a key return mask to update castling rights
	using CastlingMask = std::unordered_map<int, int>;
}

#endif // TYPES_H