#include "BoardState.h"

namespace Chess {

	BoardState::BoardState() {

		initColors();
		initPieces();
		initDirections();
		initOffset();
		initSliding();
		initZobristHash();

		castlingRights = WHITEKINGSIDE | WHITEQUEENSIDE | BLACKKINGSIDE | BLACKQUEENSIDE;

		enpassantSquare = EMPTY;

		currentPlayer = WHITE;

		fiftyMoves = 0;

		threefoldRep = false;

		for (int i = 0; i < 128; i++)
			for (int j = 0; j < 128; j++)
				fromToHashTable[i][j] = i * 128 + j;

		whiteKingSquare = x88_E1;
		blackKingSquare = x88_E8;

		int pawnsSquares[2][8] = {
			{ x88_A2, x88_B2, x88_C2, x88_D2, x88_E2, x88_F2, x88_G2, x88_H2 }, // WHITE
			{ x88_A7, x88_B7, x88_C7, x88_D7, x88_E7, x88_F7, x88_G7, x88_H7 }  // BLACK
		};

		castling.insert({ x88_H1, ~WHITEKINGSIDE });
		castling.insert({ x88_E1, ~(WHITEKINGSIDE | WHITEQUEENSIDE) });
		castling.insert({ x88_A1, ~WHITEQUEENSIDE });
		castling.insert({ x88_H8, ~BLACKKINGSIDE });
		castling.insert({ x88_E8, ~(BLACKKINGSIDE | BLACKQUEENSIDE) });
		castling.insert({ x88_A8, ~BLACKQUEENSIDE });


		allPossibleMoves = generateAllPossibleMoves();
	}

	// if there are no moves to generate check if king is under check
	// that signals check mate
	Moves BoardState::generateAllPossibleMoves() {

		Color oppositeColor = oppositeSide();
		Moves generatedMoves;

		auto pushPromotionMove = [&](int from, int to, int type) -> void {
			for (int p = KNIGHT; p <= QUEEN; p++) {
				Move m;
				m.move.byteData.from = (char)from;
				m.move.byteData.to = (char)to;
				m.move.byteData.promote = (char)p;
				m.move.byteData.type = (char)type | PROMOTE;
				generatedMoves.insert({ fromToHashTable[from][to] , m });
			}
		};

		auto pushMove = [&](int from, int to, int type) -> void {
			if (type & PAWNPUSH) {
				if (currentPlayer == WHITE) {
					if (to >= x88_A8) {
						pushPromotionMove(from, to, type);
						return;
					}
				}
				else {
					if (to <= x88_H1) {
						pushPromotionMove(from, to, type);
						return;
					}
				}
			}
			Move m;
			m.move.byteData.from = (char)from;
			m.move.byteData.to = (char)to;
			m.move.byteData.promote = (char)0;
			m.move.byteData.type = (char)type;
			generatedMoves.insert({ fromToHashTable[from][to] , m });
		};

		for (int i = 0; i < 128; i++)
			if (colors[i] == currentPlayer) {
				// PAWNS
				if (pieces[i] == PAWN) {
					if (currentPlayer == WHITE) {
						if (getFile(i) != FILEA &&
							colors[i + NORTHWEST] == BLACK)
							pushMove(i, i + NORTHWEST, CAPTURE | PAWNPUSH);
						if (getFile(i) != FILEH &&
							colors[i + NORTHEAST] == BLACK)
							pushMove(i, i + NORTHEAST, CAPTURE | PAWNPUSH);
						if (colors[i + NORTH] == EMPTY) {
							pushMove(i, i + NORTH, PAWNPUSH);
							if (i >= x88_A2 && colors[i + NORTHNORTH] == EMPTY)
								pushMove(i, i + NORTHNORTH, PAWNPUSH | PAWNPUSH2);
						}
					}
					else {
						if (getFile(i) != FILEA &&
							colors[i + SOUTHWEST] == WHITE)
							pushMove(i, i + SOUTHWEST, CAPTURE | PAWNPUSH);
						if (getFile(i) != FILEH &&
							colors[i + SOUTHEAST] == WHITE)
							pushMove(i, i + SOUTHEAST, CAPTURE | PAWNPUSH);
						if (colors[i + SOUTH] == EMPTY) {
							pushMove(i, i + SOUTH, PAWNPUSH);
							if (i <= x88_H7 && colors[i + SOUTHSOUTH] == EMPTY)
								pushMove(i, i + SOUTHSOUTH, PAWNPUSH | PAWNPUSH2);
						}
					}
				}
				// OTHER PIECES
				else {
					for (int j = 0; j < directions[pieces[i]]; j++) {
						int n = i;
						while (true) {
							n = n + offset[pieces[i]][j];
							
							if (!validSquareIndex(n))
								break;

							if (pieces[n] == EMPTY) 
								pushMove(i, n, NORMALMOVE);
							else {
								if (colors[n] == oppositeColor)
									pushMove(i, n, CAPTURE);
								break;
							}
														
							if (!sliding[pieces[i]])
								break;
						}
					}
				}
			}

		// CASTLING MOVES
		if (currentPlayer == WHITE) {
			if (castlingRights & WHITEKINGSIDE)
				pushMove(x88_E1, x88_G1, CASTLE);
			if (castlingRights & WHITEQUEENSIDE)
				pushMove(x88_E1, x88_C1, CASTLE);
		}
		else {
			if (castlingRights & BLACKKINGSIDE)
				pushMove(x88_E8, x88_G8, CASTLE);
			if (castlingRights & BLACKQUEENSIDE)
				pushMove(x88_E8, x88_C8, CASTLE);
		}

		// ENPASSANT MOVES
		if (enpassantSquare != EMPTY) {
			if (currentPlayer == WHITE) {
				if (getFile(enpassantSquare) != FILEA &&
					colors[enpassantSquare + SOUTHWEST] == WHITE &&
					pieces[enpassantSquare + SOUTHWEST] == PAWN)
						pushMove(enpassantSquare + SOUTHWEST, enpassantSquare, PAWNPUSH | ENPASSANT | CAPTURE);
				if (getFile(enpassantSquare) != FILEH &&
					colors[enpassantSquare + SOUTHEAST] == WHITE &&
					pieces[enpassantSquare + SOUTHEAST] == PAWN)
						pushMove(enpassantSquare + SOUTHEAST, enpassantSquare, PAWNPUSH | ENPASSANT | CAPTURE);
			}
			else {
				if (getFile(enpassantSquare) != FILEA &&
					colors[enpassantSquare + NORTHWEST] == BLACK &&
					pieces[enpassantSquare + NORTHWEST] == PAWN)
						pushMove(enpassantSquare + NORTHWEST, enpassantSquare, PAWNPUSH | ENPASSANT | CAPTURE);
				if (getFile(enpassantSquare) != FILEH &&
					colors[enpassantSquare + NORTHEAST] == BLACK &&
					pieces[enpassantSquare + NORTHEAST] == PAWN)
						pushMove(enpassantSquare + NORTHEAST, enpassantSquare, PAWNPUSH | ENPASSANT | CAPTURE);

			}
		}

		return generatedMoves;

	}

	BoardStateHash BoardState::zobristHash() {
		BoardStateHash positionHash = 0;

		for (int i = 0; i < 128; i++)
			if (colors[i] != EMPTY)
				positionHash ^= piecesHash[colors[i]][pieces[i]][i];
		if (currentPlayer == BLACK)
			positionHash ^= playerColorHash;
		if (enpassantSquare != EMPTY)
			positionHash ^= enpassantHash[enpassantSquare];

		return positionHash;
	}

	void BoardState::playMove(Move move) {
	
		if (move.move.byteData.type & CASTLE) {
			int from;
			int to;

			switch (move.move.byteData.to) {
			case x88_G1:
				from = x88_H1;
				to = x88_F1;
				break;
			case x88_C1:
				from = x88_A1;
				to = x88_D1;
				break;
			case x88_G8:
				from = x88_H8;
				to = x88_F8;
				break;
			case x88_C8:
				from = x88_A8;
				to = x88_D8;
				break;
			default:
				break;

			}
			colors[to] = colors[from];
			pieces[to] = pieces[from];
			colors[from] = EMPTY;
			pieces[from] = EMPTY;
		}

		colors[move.move.byteData.to] = currentPlayer;
		if (move.move.byteData.type & PROMOTE)
			pieces[move.move.byteData.to] = move.move.byteData.promote;
		else
			pieces[move.move.byteData.to] = pieces[move.move.byteData.from];

		pieces[move.move.byteData.from] = EMPTY;
		colors[move.move.byteData.from] = EMPTY;

		if (move.move.byteData.from == whiteKingSquare)
			whiteKingSquare = move.move.byteData.to;
		
		if (move.move.byteData.from == blackKingSquare)
			blackKingSquare = move.move.byteData.to;

		int enpassant = currentPlayer == WHITE ? move.move.byteData.to + SOUTH : move.move.byteData.to + NORTH;

		for (int i = 0; i < 8; i++) {
			if (pawnsSquares[currentPlayer][i] == move.move.byteData.from) 
				pawnsSquares[currentPlayer][i] = move.move.byteData.to;

			if (pawnsSquares[currentPlayer][i] == move.move.byteData.to)
				pawnsSquares[currentPlayer][i] = EMPTY;

			if ((move.move.byteData.type & ENPASSANT) && pawnsSquares[currentPlayer][i] == enpassant)
				pawnsSquares[currentPlayer][i] = EMPTY;
		}

		if (move.move.byteData.type & ENPASSANT) {
			if (currentPlayer == WHITE) {
				pieces[move.move.byteData.to + SOUTH] = EMPTY;
				colors[move.move.byteData.to + SOUTH] = EMPTY;
			}
			else {
				pieces[move.move.byteData.to + NORTH] = EMPTY;
				colors[move.move.byteData.to + NORTH] = EMPTY;
			}

		}
		
		if(castling.find(move.move.byteData.from) != castling.end())
			castlingRights &= castling.at(move.move.byteData.from);

		if (castling.find(move.move.byteData.to) != castling.end())
			castlingRights &= castling.at(move.move.byteData.to);


		/*switch (move.move.byteData.from) {
		case x88_H1:
			castlingRights &= ~WHITEKINGSIDE;
			break;
		case x88_E1:
			castlingRights &= ~(WHITEKINGSIDE | WHITEQUEENSIDE);
			break;
		case x88_A1:
			castlingRights &= ~WHITEQUEENSIDE;
			break;
		case x88_H8:
			castlingRights &= ~BLACKKINGSIDE;
			break;
		case x88_E8:
			castlingRights &= ~(BLACKKINGSIDE | BLACKQUEENSIDE);
			break;
		case x88_A8:
			castlingRights &= ~BLACKQUEENSIDE;
			break;
		default:
			break;
		}

		switch (move.move.byteData.to) {
		case x88_H1:
			castlingRights &= ~WHITEKINGSIDE;
			break;
		case x88_E1:
			castlingRights &= ~(WHITEKINGSIDE | WHITEQUEENSIDE);
			break;
		case x88_A1:
			castlingRights &= ~WHITEQUEENSIDE;
			break;
		case x88_H8:
			castlingRights &= ~BLACKKINGSIDE;
			break;
		case x88_E8:
			castlingRights &= ~(BLACKKINGSIDE | BLACKQUEENSIDE);
			break;
		case x88_A8:
			castlingRights &= ~BLACKQUEENSIDE;
			break;
		default:
			break;
		}*/

		if (move.move.byteData.type & PAWNPUSH2) {
			if (currentPlayer == WHITE)
				enpassantSquare = move.move.byteData.to + SOUTH;
			else
				enpassantSquare = move.move.byteData.to + NORTH;
		}
		else
			enpassantSquare = EMPTY;

		if (move.move.byteData.type & (PAWNPUSH | CAPTURE))
			fiftyMoves = 0;
		else
			++fiftyMoves;

		BoardStateHash hash = zobristHash();

		auto value = history.find(hash);
		// found the same position
		if (value != history.end()) {
			value->second++;
			if (value->second >= 2)
				threefoldRep = true;
		}
		else // first time seeing this position
			history.insert({ hash, 0 }); // insert new position with zero repetitions

	}

	bool BoardState::testPseudoLegalMove(Move move) {

		int tempColors[128];
		std::memcpy(tempColors, colors, sizeof(colors));

		int tempPieces[128];
		std::memcpy(tempPieces, pieces, sizeof(pieces));

		int tempPawns[2][8];
		std::memcpy(tempPawns, colors, sizeof(pawnsSquares));

		int tempBlackKing = blackKingSquare;
		int tempWhiteKing = whiteKingSquare;

		Color attackingSide = oppositeSide();

		// move the rook
		// king is moved later
		if (move.move.byteData.type & CASTLE) {
			int from;
			int to;

			if (kingInCheck())
				return false;

			switch (move.move.byteData.to) {
			case x88_G1:
				if (colors[x88_F1] != EMPTY || colors[x88_G1] != EMPTY ||
					squareUnderAttack(x88_F1, attackingSide) ||
					squareUnderAttack(x88_G1, attackingSide))
					return false;
				from = x88_H1;
				to = x88_F1;
				break;
			case x88_C1:
				if (colors[x88_B1] != EMPTY || colors[x88_C1] != EMPTY
					|| colors[x88_D1] != EMPTY ||
					squareUnderAttack(x88_C1, attackingSide) ||
					squareUnderAttack(x88_D1, attackingSide))
					return false;
				from = x88_A1;
				to = x88_D1;
				break;
			case x88_G8:
				if (colors[x88_F8] != EMPTY || colors[x88_G8] != EMPTY ||
					squareUnderAttack(x88_F8, attackingSide) ||
					squareUnderAttack(x88_G8, attackingSide))
					return false;
				from = x88_H8;
				to = x88_F8;
				break;
			case x88_C8:
				if (colors[x88_B8] != EMPTY || colors[x88_C8] != EMPTY ||
					colors[x88_D8] != EMPTY ||
					squareUnderAttack(x88_C8, attackingSide) ||
					squareUnderAttack(x88_D8, attackingSide))
					return false;
				from = x88_A8;
				to = x88_D8;
				break;
			default:
				break;

			}
			colors[to] = colors[from];
			pieces[to] = pieces[from];
			colors[from] = EMPTY;
			pieces[from] = EMPTY;
		}

		colors[move.move.byteData.to] = currentPlayer;
		if (move.move.byteData.type & PROMOTE)
			pieces[move.move.byteData.to] = move.move.byteData.promote;
		else
			pieces[move.move.byteData.to] = pieces[move.move.byteData.from];
		
		pieces[move.move.byteData.from] = EMPTY;
		colors[move.move.byteData.from] = EMPTY;

		if (move.move.byteData.type & ENPASSANT) {
			if (currentPlayer == WHITE) {
				pieces[move.move.byteData.to + SOUTH] = EMPTY;
				colors[move.move.byteData.to + SOUTH] = EMPTY;
			}
			else {
				pieces[move.move.byteData.to + NORTH] = EMPTY;
				colors[move.move.byteData.to + NORTH] = EMPTY;
			}
		}

		if (move.move.byteData.from == whiteKingSquare)
			whiteKingSquare = move.move.byteData.to;

		if (move.move.byteData.from == blackKingSquare)
			blackKingSquare = move.move.byteData.to;

		int enpassant = currentPlayer == WHITE ? move.move.byteData.to + SOUTH : move.move.byteData.to + NORTH;

		for (int i = 0; i < 8; i++) {
			if (pawnsSquares[currentPlayer][i] == move.move.byteData.from)
				pawnsSquares[currentPlayer][i] = move.move.byteData.to;

			if (pawnsSquares[currentPlayer][i] == move.move.byteData.to)
				pawnsSquares[currentPlayer][i] = EMPTY;

			if ((move.move.byteData.type & ENPASSANT) && pawnsSquares[currentPlayer][i] == enpassant)
				pawnsSquares[currentPlayer][i] = EMPTY;
		}

		// if the king is in check after making a move
		// signal invalid move
		bool inCheck = false;

		if (kingInCheck())
			inCheck = true;

		// restore the state back
		std::memcpy(colors, tempColors, sizeof(colors));
		std::memcpy(pieces, tempPieces, sizeof(pieces));

		std::memcpy(pawnsSquares, tempPawns, sizeof(pawnsSquares));

		blackKingSquare = tempBlackKing;	
		whiteKingSquare = tempWhiteKing;

		if (inCheck)
			return false;
		
		return true;

	}

	GameStatus BoardState::gameStatus() {
		bool noValidMove = true;

		for(auto& move : allPossibleMoves)
			if (testPseudoLegalMove(move.second)) {
				noValidMove = false;
				break;
			}

		if (noValidMove)
			if (kingInCheck())
				return CHECKMATE;
			else
				return STALEMATE;
		
		if (fiftyMoves >= 100) // when both black and white play a move that is counted as one move
			return DRAWBY50MOVES;

		return NOTOVER;
	}

	bool BoardState::kingInCheck() {
		Color attackingSide = oppositeSide();
		int king;
		if (currentPlayer == WHITE)
			king = whiteKingSquare;
		else
			king = blackKingSquare;
		return squareUnderAttack(king, attackingSide);
		
	}

	bool BoardState::underAttackByKing(int squareIndex, Color attackingSide) {
		int attackingSquare;
		for (int i = 0; i < 8; i++) {
			attackingSquare = squareIndex + offset[KING][i];
			if (validSquareIndex(attackingSquare)
				&& pieces[attackingSquare] == KING
				&& colors[attackingSquare] == attackingSide)
				return true;
		}
		return false;
	}

	bool BoardState::underAttackByKnight(int squareIndex, Color attackingSide) {
		int attackingSquare;
		for (int i = 0; i < 8; i++) {
			attackingSquare = squareIndex + offset[KNIGHT][i];
			if (validSquareIndex(attackingSquare)
				&& pieces[attackingSquare] == KNIGHT
				&& colors[attackingSquare] == attackingSide)
				return true;
		}
		return false;
	}

	bool BoardState::underAttackByBishopOrQueen(int squareIndex, Color attackingSide) {
	
		auto checkRoutine = [&](Direction direction) -> bool {
			int attackingSquare = squareIndex + direction;

			while (validSquareIndex(attackingSquare)) {
				if (colors[attackingSquare] != EMPTY) {
					if ((colors[attackingSquare] == attackingSide)
						&& (pieces[attackingSquare] == BISHOP || pieces[attackingSquare] == QUEEN))
						return true;
					return false;
				}
				attackingSquare += direction;
			}

			return false;
		};

		return checkRoutine(NORTHEAST) || checkRoutine(SOUTHEAST) || checkRoutine(SOUTHWEST) || checkRoutine(NORTHWEST);
	
	}

	bool BoardState::underAttackByRookOrQueen(int squareIndex, Color attackingSide) {
		
		auto checkRoutine = [&](Direction direction) -> bool {
			int attackingSquare = squareIndex + direction;

			while (validSquareIndex(attackingSquare)) {
				if (colors[attackingSquare] != EMPTY) {
					if ((colors[attackingSquare] == attackingSide)
						&& (pieces[attackingSquare] == ROOK || pieces[attackingSquare] == QUEEN))
						return true;
					return false;
				}
				attackingSquare += direction;
			}

			return false;
		};
		
		return checkRoutine(NORTH) || checkRoutine(SOUTH) || checkRoutine(EAST) || checkRoutine(WEST);
		
	}

	bool BoardState::squareUnderAttack(int squareIndex, Color attackingSide) {
		// PAWNS
		for (int i = 0; i < 8; i++) {
			if (pawnsSquares[attackingSide][i] != EMPTY) {
				int square = pawnsSquares[attackingSide][i];
				if (attackingSide == WHITE) {
					if (getFile(square) != FILEA &&
						square + NORTHWEST == squareIndex)
						return true;
					if (getFile(square) != FILEH &&
						square + NORTHEAST == squareIndex)
						return true;
				}
				else {
					if (getFile(square) != FILEA &&
						square + SOUTHWEST == squareIndex)
						return true;
					if (getFile(square) != FILEH &&
						square + SOUTHEAST == squareIndex)
						return true;

				}
			}
		}

		// OTHER PIECES

		if (underAttackByKing(squareIndex, attackingSide)			||
		   (underAttackByKnight(squareIndex, attackingSide))		||
		   (underAttackByBishopOrQueen(squareIndex, attackingSide)) ||
		   (underAttackByRookOrQueen(squareIndex, attackingSide)))
				return true;

		return false;

	}
	

	void BoardState::initColors() {
		int tempColors[128] = {
			WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
			WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
			EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
			EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
			EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
			EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
			BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
			BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY
		};

		std::memcpy(colors, tempColors, sizeof(colors));
	}

	void BoardState::initPieces() {
		int tempPieces[128] = {
			ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
			PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
			EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
			EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
			EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
			EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
			PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
			ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY
		};

		std::memcpy(pieces, tempPieces, sizeof(pieces));
	}

	void BoardState::initDirections() {
		int tempDirections[5] = {
			8, //KNIGHT
			4, //BISHOP
			4, //ROOK
			8, //QUEEN
			8  //KING
		};

		std::memcpy(directions, tempDirections, sizeof(directions));
	}

	void BoardState::initOffset() {
		int tempOffset[5][8] = {
			{ KNIGHTNORTHEAST, KNIGHTNORTHWEST, KNIGHTEASTNORTHEAST, KNIGHTWESTNORTHWEST,
			  KNIGHTEASTSOUTHEAST, KNIGHTWESTSOUTHWEST, KNIGHTSOUTHEAST, KNIGHTSOUTHWEST }, // KNIGHT
			{ NORTHWEST, NORTHEAST, SOUTHWEST, SOUTHEAST, 0, 0, 0, 0 },						// BISHOP
			{ NORTH, WEST, EAST, SOUTH, 0, 0, 0, 0 },										// ROOK
			{ NORTHWEST, NORTH, NORTHEAST, WEST, EAST, SOUTHWEST, SOUTH, SOUTHEAST },		// QUEEN
			{ NORTHWEST, NORTH, NORTHEAST, WEST, EAST, SOUTHWEST, SOUTH, SOUTHEAST }		// KING
		};

		std::memcpy(offset, tempOffset, sizeof(offset));
	}

	void BoardState::initSliding() {
		static bool tempSliding[5] = {
			false, // KNIGHT
			true,  // BISHOP
			true,  // ROOK
			true,  // QUEEN
			false  // KING
		};

		std::memcpy(sliding, tempSliding, sizeof(sliding));
	}

	void BoardState::initZobristHash() {
		srand(0);

		auto random = []() -> BoardStateHash {
			BoardStateHash value = 0;

			for (int i = 0; i < sizeof(BoardStateHash); i++)
				value ^= rand() << i;

			return value;
		};

		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 6; j++) {
				for (int k = 0; k < 128; k++) {
					piecesHash[i][j][k] = random();
				}
			}
		}

		playerColorHash = random();
		for (int i = 0; i < 128; i++)
			enpassantHash[i] = random();

	}

	GameStatus BoardState::playNextMove(ValidMove move, GameStatus previousGameStateStatus) {

		threefoldRep = false;

		// if there is a move it still needs to be checked
		// after playing it the king could be in check which is invalid
		// but first check if the move supplied is valid
		
		if (move.first) {
			if (!testPseudoLegalMove(move.second))
				return INVALIDMOVE;
		}
		else
			return INVALIDMOVE;

		// move is valid

		playMove(move.second);

		// move to next state

		currentPlayer = oppositeSide();
		allPossibleMoves.clear();

		// every time if it is discovered that any position repeats again (min 3 times)
		// prompt the player if he wants to claim draw
		if (threefoldRep)
			return DRAWBYREPETITION;

		return NOTOVER;

	}

	ValidMove BoardState::findMove(IndexPair fromToPair) {
		ValidMove valid;

		valid.first = false;
		valid.second = Move{ -1, -1 };

		if (fromToPair.first < 0 || fromToPair.second < 0 ||
			fromToPair.first > 127 || fromToPair.second > 127)
			return valid;

		auto value = allPossibleMoves.find(fromToHashTable[fromToPair.first][fromToPair.second]);

		// if move is found
		if (value != allPossibleMoves.end()) {
			valid.first = true;
			valid.second = value->second;
		}

		return valid;

	}

	void BoardState::printBoard() {
		cout << "  a b c d e f g h" << endl;
		for (int i = 7; i >= 0; i--) {
			cout << i + 1 << " ";
			for (int j = 0; j < 8; j++) {
				int index = boardIndexToSquareIndex(i, j);
				if (pieces[index] != EMPTY)
					cout << pieceToChar((Piece)pieces[index], (Color)colors[index]) << " ";
				else
					cout << "- ";
			}
			cout << i + 1 << endl;
		}
		cout << "  a b c d e f g h" << endl;
	}

	char BoardState::pieceToChar(Piece piece, Color color) {
		char p;
		switch (piece) {
		case PAWN:
			p = 'p';
			break;
		case KING:
			p = 'k';
			break;
		case QUEEN:
			p = 'q';
			break;
		case KNIGHT:
			p = 'n';
			break;
		case ROOK:
			p = 'r';
			break;
		case BISHOP:
			p = 'b';
			break;
		}

		if (color == WHITE)
			p = toupper(p);

		return p;

	}

	void BoardState::updateAllPossibleMoves() {
		allPossibleMoves = generateAllPossibleMoves();
	}

}