#include <stdio.h>
#include <string.h>
#include <time.h>
#include <set>
#include "AI.h"
#include "Player.h"
#include "util.h"

AI::AI(Connection* conn) : BaseAI(conn) {}

const char* AI::username()
{
	return "Shell AI";
}

const char* AI::password()
{
	return "password";
}

//This function is run once, before your first turn.
void AI::init()
{
	srand(time(NULL));
}

//This function is called each time it is your turn.
//Return true to end your turn, return false to ask the server for updated information.
bool AI::run()
{
	// Print out the current board state
	cout<<"+---+---+---+---+---+---+---+---+"<<endl;
	for(size_t rank=8; rank>0; rank--)
	{
		cout<<"|";
    	for(size_t file=1; file<=8; file++)
    	{
			bool found = false;
      		// Loops through all of the pieces
      		for(size_t p=0; !found && p<pieces.size(); p++)
      		{
        		// determines if that piece is at the current rank and file
        		if(pieces[p].rank() == rank && pieces[p].file() == file)
        		{
          			found = true;
          			// Checks if the piece is black
					if(pieces[p].owner() == 1)
					 {
            			cout<<"*";
					}
          			else
          			{
            			cout<<" ";
         			 }			
          			// prints the piece's type
          			cout<<(char)pieces[p].type()<<" ";
				}
			}
			if(!found)
			{
				cout<<"   ";
			}
			cout<<"|";
		}
		cout<<endl<<"+---+---+---+---+---+---+---+---+"<<endl;
	}

	// Looks through information about the players
	for(size_t p=0; p<players.size(); p++)
	{
		cout<<players[p].playerName();
		// if playerID is 0, you're white, if its 1, you're black
		if(players[p].id() == playerID())
		{
			cout<<" (ME)";
		}
  
	}

	// oldState is the current state
	myState oldState;
	
	// if there has been a move, print the most recent move
	if(moves.size() > 0)
	{
		cout<<"Last Move Was: "<<endl<<moves[0]<<endl;
		// copy the last eight moves to the state
		size_t ssize = 8;
		if(moves.size() < ssize)
		{
			ssize = moves.size();
		}
	
		for(size_t p=0; p<ssize;p++)
		{
			myMove move;
			move.fromFile = moves[p].fromFile();
			move.toFile = moves[p].toFile();
			move.fromRank = moves[p].fromRank();
			move.toRank = moves[p].toRank();
			move.promoteType = moves[p].promoteType();
			oldState.lastMoves.push_back(move);
		}
	}
  
	// find the turns left for draw for the state
	int turnsToDraw = TurnsToStalemate();
   
	oldState.turnsWithNoPorC = turnsToDraw;
	
	if(turnsToDraw > 8)
	{
		turnsToDraw = 8;
	}
  
	oldState.turnsLeft = turnsToDraw;
	
	oldState.isQS = 0;
	memset(oldState.board, ' ', sizeof(oldState.board));
 
	for (size_t p=0; p<pieces.size(); p++)
	{
		if (pieces[p].owner() == 0)		// White pieces use uppercase letters
		{
			oldState.board[8-pieces[p].rank()][pieces[p].file()-1] = (char) pieces[p].type();
		}
		else // Black pieces use lowercase letters
		{
			oldState.board[8-pieces[p].rank()][pieces[p].file()-1] = std::tolower(pieces[p].type());
		}
		oldState.hasMoved[8-pieces[p].rank()][pieces[p].file()-1] = pieces[p].hasMoved();
	}
  
	//determine next move using Time-Limited Iterative-Deepening Depth-Limited MiniMax with alpha-beta pruning 
	myMove mmove = nextMove(oldState);
  
	//If there is no legal move
	if(mmove.toRank == 999) {
		return true;
	}
	
	//find the pieces to be moved
	int  movedPiece= -1;
	
	for (size_t i = 0; i < pieces.size(); i++)
	{
		if (pieces[i].file() == mmove.fromFile+1 && pieces[i].rank() == 8-mmove.fromRank)
		{
			movedPiece = i;
			break;
		}
	}
	

	//move the piece based on random search
	pieces[movedPiece].move(mmove.toFile+1, 8-mmove.toRank, mmove.promoteType);
  
	//Print the move
	printf("Moved piece: %c\n", oldState.board[mmove.fromRank][mmove.fromFile]);
	printf("From %d rank, from %d file\n", 8-mmove.fromRank, mmove.fromFile+1);
	printf("To %d rank, to %d file\n", 8-mmove.toRank, mmove.toFile+1);
	if(mmove.promoteType != '\0')
	{
		printf("Promotion Type: %c\n", mmove.promoteType);
	}

	return true;
}

//This function is run once, after your last turn.
void AI::end(){}

/*************************************************************************/

myStates AI::nextStates(const myState &s, int player)
{
	myStates states;
	myState nState;
	
	//first check if the king is in check
	bool kingInCheck = false;
	kingInCheck = inCheck(s, !player);
	
	myMoves moves = legalMoves(s, player, kingInCheck);
	
	//create new states
	while(!moves.empty())
	{
		nState = newState(s, moves.back(), player);
		/* If the move won't cause a checkmate, add the new state for evaluation */
		if(!inCheck(nState, !player))
		{
			states.push(nState);
		}
		moves.pop_back();
	}
	
	return states;
}

/**********************************************************************/
myState AI::newState(myState s, const myMove &m, int player)
{
	s.move = m;
	std::map<myMove, int>::iterator it;
	it = history.find(s.move);
	s.isQS = 1;
	
	//If the movement can be found in the history table
	if(it != history.end())
	{
		s.histScore = history[s.move];
	}
	//If the movement can not be found in the history table
	else
	{
		s.histScore = 0;
	}
	
	//determine turns left for draw
	//capture
	if(s.board[m.toRank][m.toFile] != ' ') 
	{
		s.turnsLeft = 8;
		s.turnsWithNoPorC = 0;
		s.isQS = 0;
	}
	//Pawn action
	else if(s.board[m.fromRank][m.fromFile] == 'p' || s.board[m.fromRank][m.fromFile] == 'P')
	{
		s.turnsLeft = 8;
		s.turnsWithNoPorC = 0;
		s.isQS = 0;
	}
	//Promotion
	else if(m.promoteType != '\0')
	{
		s.turnsLeft = 8;
		s.isQS = 0;
	}
	else
	{
		s.turnsLeft--;
		s.turnsWithNoPorC++;
	}
	
	//track the last eight moves
	if(s.lastMoves.size() < 8)
	{
		s.lastMoves.push_back(m);
	}
	else
	{
		s.lastMoves.erase(s.lastMoves.begin());
		s.lastMoves.push_back(m);
	}
	
	
	//Move the piece
	
	//For promotion
	if(m.promoteType != '\0')
	{
		s.board[m.toRank][m.toFile] = m.promoteType;
		if(s.board[m.fromRank][m.fromFile] == 'p')
		{
			s.board[m.toRank][m.toFile] += 32;
		}
	}
	else
	{
		s.board[m.toRank][m.toFile] = s.board[m.fromRank][m.fromFile];
	}
		s.board[m.fromRank][m.fromFile] = ' ';
	
	
	//castling
	if((s.board[m.toRank][m.toFile] == 'K' || s.board[m.toRank][m.toFile] == 'k') && abs(m.toFile - m.fromFile) == 2)
	{
		if(m.toRank == 7) //white
		{
			if(m.toFile == 2) //left side of board
			{
				s.board[7][0] = ' ';
				s.board[7][2] = 'R';
			}
			else //right side of board
			{
				s.board[7][7] = ' ';
				s.board[7][5] = 'R';
			}
		}
		else //black
		{
			if(m.toFile == 2) //left side of board
			{
				s.board[0][0] = ' ';
				s.board[0][2] = 'r';
			}
			else //right side of board
			{
				s.board[0][7] = ' ';
				s.board[0][5] = 'r';
			}
		}
	}
	
	return s;
}

/***************************************************************************/
myMoves AI::legalMoves(const myState & s, int player, bool inCheck)
{
	myMoves nextMoves;
	
	//Find the possible nextMoves for all the pieces
	for(int file = 0; file < 8; file++)
	{
		for(int rank = 0; rank < 8; rank++)
		{
			
			//King
			if(( !player && s.board[rank][file] == 'K') || ( player&& s.board[rank][file] == 'k'))
			{
				KingMove(s, file, rank, player, nextMoves, inCheck);
			}
			
			//Bishop
			if((!player && s.board[rank][file] == 'B') || ( player && s.board[rank][file] == 'b'))
			{
				BishopMove(s, file, rank, player, nextMoves);
			}
			
			//Queen
			if((!player && s.board[rank][file] == 'Q') || ( player && s.board[rank][file] == 'q'))
			{
				QueenMove(s, file, rank, player, nextMoves);
			}
			
			//Rook
			if((!player && s.board[rank][file] == 'R') || ( player && s.board[rank][file] == 'r'))
			{
				RookMove(s, file, rank, player, nextMoves);
			}
			
			//Knight
			if((!player && s.board[rank][file] == 'N') || ( player && s.board[rank][file] == 'n'))
			{
				KnightMove(s, file, rank, player, nextMoves);
				
			}
			
			//Pawn
			if(s.board[rank][file] == 'P' || s.board[rank][file] == 'p')
			{
				PawnMove(s, file, rank, player, nextMoves);
			}
		}
	}
	
	return nextMoves;
	
}


/************************************************************************************************************/
bool AI::inCheck(const myState &s, int player)
{
	myMoves nextMoves;
	bool inTheCheck = false;
		
	//Find the possible nextMoves for all the pieces and check if it will cause a checkmate
	for(int file = 0; file < 8; file++)
	{
		for(int rank = 0; rank < 8; rank++)
		{
			
			//King
			if((!player && s.board[rank][file] == 'K') || ( player && s.board[rank][file] == 'k'))
			{
				inTheCheck = KingMove(s, file, rank, player, nextMoves, false);
			}
			
			//Bishop + Diagonal Queen
			if((!player && s.board[rank][file] == 'B') || ( player && s.board[rank][file] == 'b'))
			{
				inTheCheck = BishopMove(s, file, rank, player, nextMoves);
			}
			
			
			if((!player && s.board[rank][file] == 'Q') || ( player && s.board[rank][file] == 'q'))
			{
				inTheCheck = QueenMove(s, file, rank, player, nextMoves);
			}
			
			//Rook and Orthoganal Queen
			if((!player && s.board[rank][file] == 'R') || ( player && s.board[rank][file] == 'r'))
			{
				inTheCheck = RookMove(s, file, rank, player, nextMoves);
			}
			
			//Knight
			if((!player && s.board[rank][file] == 'N') || ( player && s.board[rank][file] == 'n'))
			{
				inTheCheck = KnightMove(s, file, rank, player, nextMoves);
				
			}
			
			//Pawn
			if((!player && s.board[rank][file]=='P') || (player && s.board[rank][file]=='p'))
			{
				inTheCheck = PawnMove(s, file, rank, player, nextMoves);
			}
			if(inTheCheck == true)
			{
				return true;
			}
		}
	}
	
	return inTheCheck;
	
}

/************************************************************************************************************/

bool AI::KingMove(myState s, int file, int rank, int player, myMoves &nextMoves, bool inCheck)
{
	myMove move;
	move.player = player;
	move.promoteType = '\0';
	move.fromFile = file;
	move.fromRank = rank;
	bool checkmate = false;
	for(int i = -1; i <= 1; i++)
	{
		for(int j = -1; j <= 1; j++)
		{
			int toFile = file + j;
			int toRank = rank + i;
			if( toRank >= 0 && toRank <=7 && toFile >= 0 && toFile <=7 &&
			   (s.board[toRank][toFile] == ' '|| (!player && isBlack(s.board[toRank][toFile]))
			   || (player && isWhite(s.board[toRank][toFile])))&& (i != 0 || j != 0))
			{
				move.toFile = toFile;
				move.toRank = toRank;
				if( (!player && s.board[move.toRank][move.toFile] == 'k') || (player && s.board[move.toRank][move.toFile] == 'K'))
				{
					checkmate = true;
				}
				nextMoves.push_back(move);
			}
		}
	}
				
	//castling for white
	if(rank == 7 && file == 4 && !s.hasMoved[7][4] && !inCheck)
	{
		//left side of board
		if(s.board[7][0] == 'R' && s.board[7][1] == ' ' && s.board[7][2] == ' ' && s.board[7][3] == ' ' && !s.hasMoved[7][0])
		{
			move.toFile = 2;
			move.toRank = 7;
			if( (!player && s.board[move.toRank][move.toFile] == 'k') || (player && s.board[move.toRank][move.toFile] == 'K'))
			{
				checkmate = true;
			}
			nextMoves.push_back(move);
		}
		//right side of board
		if(s.board[7][7] == 'R' && s.board[7][6] == ' ' && s.board[7][5] == ' ' && !s.hasMoved[7][7])
		{
			move.toFile = 6;
			move.toRank = 7;
			if( (!player && s.board[move.toRank][move.toFile] == 'k') || (player && s.board[move.toRank][move.toFile] == 'K'))
			{
				checkmate = true;
			}
			nextMoves.push_back(move);
		}
	}
				
	//castling for black
	if(rank == 0 && file == 4 && !s.hasMoved[0][4] && !inCheck)
	{
		//left side of board
		if(s.board[0][0] == 'r' && s.board[0][1] == ' ' && s.board[0][2] == ' ' && s.board[0][3] == ' ' && !s.hasMoved[0][0])
		{
			move.toFile = 2;
			move.toRank = 0;
			if( (!player && s.board[move.toRank][move.toFile] == 'k') || (player && s.board[move.toRank][move.toFile] == 'K'))
			{
				checkmate = true;
			}
			nextMoves.push_back(move);
		}
		//right side of board
		if(s.board[0][7] == 'r' && s.board[0][6] == ' ' && s.board[0][5] == ' ' && !s.hasMoved[0][7])
		{
			move.toFile = 6;
			move.toRank = 0;
			if( (!player && s.board[move.toRank][move.toFile] == 'k') || (player && s.board[move.toRank][move.toFile] == 'K'))
			{
				checkmate = true;
			}
			nextMoves.push_back(move);
		}
	}
	return checkmate;
}

/*********************************************************************************************************************/

bool AI::QueenMove(myState s, int file, int rank, int player, myMoves &nextMoves)
{
	bool checkmate0 = false, checkmate1 = false;
	checkmate0 = BishopMove(s, file, rank, player, nextMoves);
	checkmate1 = RookMove(s, file, rank, player, nextMoves);
	
	return (checkmate0 || checkmate1);
}

/********************************************************************************************************/

bool AI::BishopMove(myState s, int file, int rank, int player, myMoves &nextMoves)
{
	myMove move;
	move.player = player;
	move.promoteType = '\0';
	move.fromFile = file;
	move.fromRank = rank;
	bool checkmate = false;
	
	bool blocked[4] = {false, false, false, false};
	int dX[4] = {1, -1, 1, -1};
	int dY[4] = {1, -1, -1, 1};
	
	for(int d = 0; d <= 3; d++ )
	{ 
		for(int i = 1; i <=7; i++)
		{
			if(!blocked[d])
			{
				int toFile = file + dX[d] * i;
				int toRank = rank + dY[d] * i;
				if((toFile>= 0) && (toFile<=7) && (toRank >=0) && (toRank <=7))
				{
					if(s.board[toRank][toFile] == ' ')
					{
						move.toFile = toFile;
						move.toRank = toRank;
						nextMoves.push_back(move);
					}
					else
					{
						blocked[d] = true;
						if((!player && isBlack(s.board[toRank][toFile])) || (player && isWhite(s.board[toRank][toFile])))
						{
							move.toFile = toFile;
							move.toRank = toRank;
							if( (!player && s.board[move.toRank][move.toFile] == 'k') || (player && s.board[move.toRank][move.toFile] == 'K'))
							{
								checkmate = true;
							}
						nextMoves.push_back(move);
						}
					}
					
				}
				else
				{
					blocked[d] = true;
				}
			}
		}
	}
	
	return checkmate;					
		
}

/***********************************************************************************************************/

bool AI::RookMove(myState s, int file, int rank, int player, myMoves &nextMoves)
{
	myMove move;
	move.player = player;
	move.promoteType = '\0';
	move.fromFile = file;
	move.fromRank = rank;
	
	bool checkmate = false;
	
	bool blocked[4] = {false, false, false, false};
	int dX[4] = {0, 0, 1, -1};
	int dY[4] = {1, -1, 0, 0};
	
	for(int d = 0; d <= 3; d++ )
	{ 
		for(int i = 1; i <=7; i++)
		{
			if(!blocked[d])
			{
				int toFile = file + dX[d]*i;
				int toRank = rank + dY[d]*i;
				if((toFile>=0) && (toFile<=7) && (toRank>=0) && (toRank <=7))
				{
					if(s.board[toRank][toFile] == ' ')
					{
						move.toFile = toFile;
						move.toRank = toRank;
						nextMoves.push_back(move);
					}
					else
					{
						blocked[d] = true;
						if((!player && isBlack(s.board[toRank][toFile])) || (player && isWhite(s.board[toRank][toFile])))
						{
							move.toFile = toFile;
							move.toRank = toRank;
							if( (!player && s.board[move.toRank][move.toFile] == 'k') || (player && s.board[move.toRank][move.toFile] == 'K'))
							{
								checkmate = true;
							}
							nextMoves.push_back(move);
						}
					}	
				}
				else
				{
					blocked[d] = true;
				}
			}
		}
	}
	
	return checkmate;
}

/**********************************************************************************************************/
bool AI::KnightMove(myState s, int file, int rank, int player, myMoves &nextMoves)
{
	myMove move;
	move.player = player;
	move.promoteType = '\0';
	move.fromFile = file;
	move.fromRank = rank;
	bool checkmate = false;
	
	for(int i = -2; i<=2; i++)
	{
		for(int j = -2; j<=2; j++)
		{
			int toRank = rank + i;
			int toFile = file + j;
			if(toRank >= 0 && toRank <= 7 && toFile >=0 && toFile <=7 &&
				i != 0 && j != 0 && (abs(j) != abs(i)) &&
				(s.board[toRank][toFile] == ' '
				|| (!player && isBlack(s.board[toRank][toFile]))
				|| (player && isWhite(s.board[toRank][toFile]))))
			{
				move.toFile = toFile;
				move.toRank = toRank;
				if( (!player && s.board[move.toRank][move.toFile] == 'k') || (player && s.board[move.toRank][move.toFile] == 'K'))
				{
					checkmate = true;
				}
				nextMoves.push_back(move);
			}
		}
	}
	return checkmate;
}

/*********************************************************************************/
bool AI::PawnMove(myState s, int file, int rank, int player, myMoves &nextMoves)
{
	myMove move;
	move.player = player;
	move.promoteType = '\0';
	move.fromFile = file;
	move.fromRank = rank;
	bool checkmate = false;
	
	//move forward for white Pawn
	if((!player) && (s.board[rank][file] == 'P') && (rank > 0))
	{
		if(s.board[rank-1][file] == ' ')
		{
			move.toFile = file;
			move.toRank = rank-1;
			//promotion
			if(rank == 1)
			{
				/* change promotion type before adding the move to 'nextMoves' */
				move.promoteType = 'Q';
				nextMoves.push_back(move);
				move.promoteType = 'N';
				nextMoves.push_back(move);
				move.promoteType = 'B';
				nextMoves.push_back(move);
				move.promoteType = 'R';
				nextMoves.push_back(move);
				/* reset the promoteType to '\0' for the possible moves */
				move.promoteType = '\0';
			}
			else
			{
				nextMoves.push_back(move);
			}
			
			//first move, can move forward 2
			if((rank == 6) && (s.board[rank-2][file] == ' '))
			{
				move.toFile = file;
				move.toRank = rank-2;
				nextMoves.push_back(move);
			}
		}
				
		//Capture
		if((file < 7) && (isBlack(s.board[rank-1][file+1]))) //right
		{
			move.toFile = file+1;
			move.toRank = rank-1;
			//promotion
			if(rank == 1)
			{
				move.promoteType = 'Q';
				nextMoves.push_back(move);
				move.promoteType = 'N';
				nextMoves.push_back(move);
				move.promoteType = 'B';
				nextMoves.push_back(move);
				move.promoteType = 'R';
				nextMoves.push_back(move);
				move.promoteType = '\0';
				if( (!player && s.board[move.toRank][move.toFile] == 'k') || (player && s.board[move.toRank][move.toFile] == 'K'))
				{
					checkmate = true;
				}
			}
			else
			{
				if( (!player && s.board[move.toRank][move.toFile] == 'k') || (player && s.board[move.toRank][move.toFile] == 'K'))
				{
					checkmate = true;
				}
				nextMoves.push_back(move);
			}
		}
		if((file> 0) && (isBlack(s.board[rank-1][file-1]))) //left
		{
			move.toFile = file-1;
			move.toRank = rank-1;
			//promotion
			if(rank == 1)
			{
				move.promoteType = 'Q';
				nextMoves.push_back(move);
				move.promoteType = 'N';
				nextMoves.push_back(move);
				move.promoteType = 'B';
				nextMoves.push_back(move);
				move.promoteType = 'R';
				nextMoves.push_back(move);
				move.promoteType = '\0';
				if( (!player && s.board[move.toRank][move.toFile] == 'k') || (player && s.board[move.toRank][move.toFile] == 'K'))
				{
					checkmate = true;
				}
			}
			else
			{
				if( (!player && s.board[move.toRank][move.toFile] == 'k') || (player && s.board[move.toRank][move.toFile] == 'K'))
				{
					checkmate = true;
				}
				nextMoves.push_back(move);
			}
		}
		//En passant
		if(s.lastMoves.size() > 0 && file < 7 && rank == 3 && s.board[rank][file+1] == 'p'
			&& (s.lastMoves[0].toFile - 1) == file && ( 8 - s.lastMoves[0].toRank)== rank
			&& (8 - s.lastMoves[0].fromRank) == 1) //right
		{
			move.toFile = file + 1;
			move.toRank = rank - 1;
			nextMoves.push_back(move);
		
		}
		
		if(s.lastMoves.size() > 0 && file > 0 && rank == 3 && s.board[rank][file - 1] == 'p'
			&& (s.lastMoves[0].toFile - 1) == file && ( 8 - s.lastMoves[0].toRank) == rank
			&& ( 8 - s.lastMoves[0].fromRank) == 1) //left
		{
			move.toFile = file - 1;
			move.toRank = rank - 1;
			nextMoves.push_back(move);
		}
	
	}
	//Pawn for Black Player
	if(player && (s.board[rank][file] == 'p') && (rank < 7))
	{
		
		//move forward
		if(s.board[rank+1][file] == ' ')
		{
			move.toFile = file;
			move.toRank = rank+1;
			//promotion
			if(rank == 6)
			{
				move.promoteType = 'Q';
				nextMoves.push_back(move);
				move.promoteType = 'N';
				nextMoves.push_back(move);
				move.promoteType = 'B';
				nextMoves.push_back(move);
				move.promoteType = 'R';
				nextMoves.push_back(move);
				move.promoteType = '\0';
			}
			else
			{
				nextMoves.push_back(move);
			}
			
			//first move, can move forward 2
			if(rank == 1 && s.board[rank+2][file] == ' ')
			{
				move.toFile = file;
				move.toRank = rank+2;
				nextMoves.push_back(move);
			}
		}
		
		//Capture
		if(file < 7 && isWhite(s.board[rank+1][file+1])) //right
		{
			move.toFile = file+1;
			move.toRank = rank+1;
			//promotion
			if(rank == 6)
			{
				move.promoteType = 'Q';
				nextMoves.push_back(move);
				move.promoteType = 'N';
				nextMoves.push_back(move);
				move.promoteType = 'B';
				nextMoves.push_back(move);
				move.promoteType = 'R';
				nextMoves.push_back(move);
				move.promoteType = '\0';
				if( (!player && s.board[move.toRank][move.toFile] == 'k') || (player && s.board[move.toRank][move.toFile] == 'K'))
				{
					checkmate = true;
				}
			}
			else
			{
				if( (!player && s.board[move.toRank][move.toFile] == 'k') || (player && s.board[move.toRank][move.toFile] == 'K'))
				{
					checkmate = true;
				}
				nextMoves.push_back(move);
			}
		}
		if(file > 0 && isWhite(s.board[rank+1][file-1])) //left
		{
			move.toFile = file-1;
			move.toRank = rank+1;
			//promotion
			if(rank == 6)
			{
				move.promoteType = 'Q';
				nextMoves.push_back(move);
				move.promoteType = 'N';
				nextMoves.push_back(move);
				move.promoteType = 'B';
				nextMoves.push_back(move);
				move.promoteType = 'R';
				nextMoves.push_back(move);
				move.promoteType = '\0';
				if( (!player && s.board[move.toRank][move.toFile] == 'k') || (player && s.board[move.toRank][move.toFile] == 'K'))
				{
					checkmate = true;
				}
			}
			else
			{
				if( (!player && s.board[move.toRank][move.toFile] == 'k') || (player && s.board[move.toRank][move.toFile] == 'K'))
				{
					checkmate = true;
				}
				nextMoves.push_back(move);
			}
		}
		//En passant
		if(s.lastMoves.size() > 0 && file < 7 && rank == 4 && s.board[rank][file+1] == 'P'
			&& (s.lastMoves[0].toFile - 1) == file && (8 - s.lastMoves[0].toRank) == rank
			&& (8 - s.lastMoves[0].fromRank) == 6 ) //right
		{
			move.toFile = file + 1;
			move.toRank = rank + 1;
			nextMoves.push_back(move);
		}
		
		if(s.lastMoves.size() > 0 && file > 0 && rank == 4 && s.board[rank][file - 1] == 'P'
			&& (s.lastMoves[0].toFile - 1) == file && (8 - s.lastMoves[0].toRank) == rank
			&& (8 - s.lastMoves[0].fromRank) == 6) //left
		{
			move.toFile = file - 1;
			move.toRank = rank + 1;
			nextMoves.push_back(move);
		}
	}
	
	return checkmate;
}
			

/***********************************************************************************************************/

/*******************************************************************************************************/
bool AI::isWhite(char c)
{
	//White pieces are upperclass
	if(c >= 'A' && c <= 'Z')
	{
		return true;
	}
	
	return false;
}

/*******************************************************************************************************/
bool AI::isBlack(char c)
{
	//Black pieces are lowerclass
	if(c >= 'a' && c <= 'z')
	{
		return true;
	}
	
	return false;
}

/*******************************************************************************************************/
myMove AI::nextMove(const myState & oldState)
{
	myMove mmove;
	
	//Find all the possible next states for current state
	myStates newStates = nextStates(oldState, playerID());
	myStates newStatesCopy = newStates;
	time_t maxTime = timeHave();
	int maxDepth = 5;
	
	//if there is no legal move
	if(newStates.size() == 0)
	{
		printf("No legal move!\n");
		mmove.toRank = 999;
		return mmove;
	}
	
	int alpha = -100000000;
	int beta = 100000000;
	
	int maxScore = -10000001;
	int score;
	
	//Time limited ID-DLMM miniMax
	for(int depth = 1; maxTime > time(NULL) && depth <= maxDepth; depth++) {
		printf("\ndepth: %d\n", depth);
		alpha = -10000000;
		beta = 10000000;
		maxScore = -10000001;
		while(!newStatesCopy.empty()) {
			myState evaState = newStatesCopy.top();
			score = alphaBetaMin(evaState, alpha, beta, depth - 1);
			if(score > maxScore || (score == maxScore && rand()%2 == 1)) 
			{
				maxScore = score;
				mmove = evaState.move;
			}
			newStatesCopy.pop();
		}
		std::map<myMove,int>::iterator it;
		it = history.find(mmove);
	
		if(it != history.end())
		{
			history[mmove]++;
		}
		else
		{
			history.insert(std::pair<myMove,int>(mmove,1));
		}
		newStatesCopy = newStates;
	}
	
	
	return mmove;
}

/**********************************************************************************************************/
int AI::alphaBetaMax( const myState &s, int alpha, int beta, int depthleft ) 
{
	std::map<myMove,int>::iterator it;
	if ( depthleft == 0 ) 
	{
		//check if the movement can be found
		it = history.find(s.move);
		if(it != history.end())
		{
			history[s.move]++;
		}
		else
		{
			history.insert(std::pair<myMove, int>(s.move,1));
		}
		
		if(!s.isQS)
		{
			return QSMin(s, 2, alpha, beta);
		}
		else
		{
			return evaluate( s );
		}
	}
   
	myMove returnMove;
   
	int evaS = evaluate(s);
   
	if( evaS != 200 && evaS != -200)
	{
		myStates newStates = nextStates( s, playerID());
   
		// set score to -infinite
		int score = -1000000;
   
		// If there is no possible moves
		if(newStates.empty())
		{
			score = drawOrWin( s );
			if(score >= beta)
			{
				//check if the movement can be found
				it = history.find(s.move);
				if(it != history.end())
				{
					history[s.move]++;
				}
				else
				{
					history.insert(std::pair<myMove,int>(s.move,1));
				}
				return score;
			}
			if(score > alpha)
			{
				alpha = score;
			}
		}
   
		while(!newStates.empty()) 
		{
			myState state = newStates.top();
			int tmpScore = alphaBetaMin( state, alpha, beta, depthleft - 1 );
			if(tmpScore > score) 
			{
				returnMove = state.move;
				score = tmpScore;
			}
			if( score >= beta )
			{
				//check if the movement can be found
				it = history.find(state.move);
				if(it != history.end())
				{
					history[state.move]++;
				}
				else
				{
					history.insert(std::pair<myMove,int>(state.move,1));
				}
				return score;   // fail hard beta-cutoff
			}
			if( score > alpha )
			{
				alpha = score; // alpha acts like max in MiniMax
			}
			newStates.pop();
		}
		
		//check if the movement can be found
		it = history.find(returnMove);
		if(it != history.end())
		{
			history[returnMove]++;
		}
		else
		{
			history.insert(std::pair<myMove,int>(returnMove,1));
		}
		
		return score;
	}
	
	else //If state s is a draw
	{
		return -200;
	}
}
 
/**********************************************************************************************************/
int AI::alphaBetaMin( const myState &s, int alpha, int beta, int depthleft ) 
{
	std::map<myMove,int>::iterator it;
	if ( depthleft == 0 )
	{
		//check if the movement can be found
		it = history.find(s.move);
		if(it != history.end())
		{
			history[s.move]++;
		}
		else
		{
			history.insert(std::pair<myMove,int>(s.move,1));
		}
		
		if(!s.isQS)
		{
			return QSMax(s, 2, alpha, beta);
		}
		else
		{
			return evaluate( s );
		}
	}
   
	myMove returnMove;
	
	int evaS = evaluate( s );
	
	if(evaS != 200 && evaS != -200) 
	{
		myStates newStates = nextStates( s, !playerID());
   
		// Set the socre to infinite
		int score = 10000;
   
		// If there is no possible moves
		if(newStates.empty())
		{
			score = drawOrWin( s );
			if( score <= alpha )
			{
				//check if the movement can be found
				it = history.find(s.move);
				if(it != history.end())
				{
					history[s.move]++;
				}
				else
				{
					history.insert(std::pair<myMove,int>(s.move,1));
				}
				return score;
			}
			if( score < beta )
			{
				beta = score;
			}
		}
   
		while(!newStates.empty()) 
		{
			myState state = newStates.top();
			int tmpScore = alphaBetaMax( state, alpha, beta, depthleft - 1 );
			if(tmpScore < score)
			{
				returnMove = state.move;
				score = tmpScore;
			}
			if( score <= alpha )
			{
				//check if the movement can be found
				it = history.find(state.move);
				if(it != history.end())
				{
					history[state.move]++;
				}
				else
				{
					history.insert(std::pair<myMove,int>(state.move,1));
				}
				return score; // fail hard alpha-cutoff
			}
			if( score < beta )
			{
				beta = score; // beta acts like min in MiniMax
			}
			newStates.pop();
		}
		
		//check if the movement can be found
		it = history.find(returnMove);
		if(it != history.end())
		{
			history[returnMove]++;
		}
		else
		{
			history.insert(std::pair<myMove,int>(returnMove,1));
		}
		return score;
	}
	
	else //If state s is a draw
	{
		return -200;
	}
}
/************************************************************************************************************/
int AI::QSMax(const myState &s, int depth, int alpha, int beta)
{
	std::map<myMove,int>::iterator it;
	//If it is a quite state or depth limited reached
	if(s.isQS || depth == 0)
	{
		it = history.find(s.move);
		if(it != history.end())
		{
			history[s.move]++;
		}
		else
		{
			history.insert(std::pair<myMove,int>(s.move,1));
		}
		return evaluate(s);
	}
	else
	{
	
		myMove returnMove;
	   
		int evaS = evaluate(s);
	   if( evaS != 200 && evaS != -200)
	   {
			myStates newStates = nextStates( s, playerID());
	   
			// set score to -infinite
			int score = -1000000;
	   
			// If there is no possible moves
			if(newStates.empty())
			{
				score = drawOrWin( s );
				if(score >= beta)
				{
					//check if the movement can be found
					it = history.find(s.move);
					if(it != history.end())
					{
						history[s.move]++;
					}
					else
					{
						history.insert(std::pair<myMove,int>(s.move,1));
					}
					return score;
				}
				if(score > alpha)
				{
					alpha = score;
				}
			}
	   
			while(!newStates.empty()) 
			{
				myState state = newStates.top();
				//do QS search
				int tmpScore = QSMin( state, depth -1 , alpha, beta );
				if(tmpScore > score) 
				{
					returnMove = state.move;
					score = tmpScore;
				}
				if( score >= beta )
				{
					//check if the movement can be found
					it = history.find(state.move);
					if(it != history.end())
					{
						history[state.move]++;
					}
					else
					{
						history.insert(std::pair<myMove,int>(state.move,1));
					}
					return score;   // fail hard beta-cutoff
				}
				if( score > alpha )
				{
					alpha = score; // alpha acts like max in MiniMax
				}
				newStates.pop();
			}
			
			//check if the movement can be found
			it = history.find(returnMove);
			if(it != history.end())
			{
				history[returnMove]++;
			}
			else
			{
				history.insert(std::pair<myMove,int>(returnMove,1));
			}
			
			return score;
		}
		
		else //If state s is a draw
		{
			return -200;
		}
	}
}
	

/************************************************************************************************************/
int AI::QSMin(const myState &s, int depth, int alpha, int beta)
{
	std::map<myMove,int>::iterator it;
	//If it is a quite state or depth limited reached
	if(s.isQS || depth == 0)
	{
		it = history.find(s.move);
		if(it != history.end())
		{
			history[s.move]++;
		}
		else
		{
			history.insert(std::pair<myMove,int>(s.move,1));
		}
		return evaluate(s);
	}
	//If it is not a quite state, do the QSMax search
	else
	{
		myMove returnMove;
		
		int evaS = evaluate(s);
		if(evaS != 200 && evaS != -200) 
		{
			myStates newStates = nextStates( s, !playerID());
   
			// Set the socre to infinite
			int score = 10000;
   
			// If there is no possible moves
			if(newStates.empty())
			{
				score = drawOrWin( s );
				if( score <= alpha )
				{
					//check if the movement can be found
					it = history.find(s.move);
					if(it != history.end())
					{
						history[s.move]++;
					}
					else
					{
					history.insert(std::pair<myMove,int>(s.move,1));
					}
					return score;
				}
				if( score < beta )
				{
					beta = score;
				}
			}
   
			while(!newStates.empty()) 
			{
				myState state = newStates.top();
				//do QS
				int tmpScore = QSMax( state, depth-1, alpha, beta );
				if(tmpScore < score)
				{
					returnMove = state.move;
					score = tmpScore;
				}
				if( score <= alpha )
				{
					//check if the movement can be found
					it = history.find(state.move);
					if(it != history.end())
					{
						history[state.move]++;
					}
					else
					{
						history.insert(std::pair<myMove,int>(state.move,1));
					}
					return score; // fail hard alpha-cutoff
				}
				if( score < beta )
				{
					beta = score; // beta acts like min in MiniMax
				}
				newStates.pop();
			}
		
			//check if the movement can be found
			it = history.find(returnMove);
			if(it != history.end())
			{
				history[returnMove]++;
			}
			else
			{
				history.insert(std::pair<myMove,int>(returnMove,1));
			}
			return score;
		}
	
		else //If state s is a draw
		{
			return -200;
		}
	}
}
		

/************************************************************************************************************/
int AI::evaluate(const myState &s)
{
	//evaluate the state
	int player0 = 0;
	int player1 = 0;
	
	bool whiteLose = true;
	bool blackLose = true;
	bool impossibleToCheckmate = true;
	
	//pieceSet is used to store potential draw pieces that cause impossible to checkmate
	std::set<char> pieceSet;
	std::set<char>::iterator itrl, itrU;
	
	// rank and file for white and black bishops, only for impposible to checkmate situation
	int wbr = 0;
	int wbf = 0;
	int bbr = 0;
	int bbf = 0;
	
	for(int rank = 0; rank < 8; rank++)
	{
		for(int file = 0; file < 8; file++)
		{
			//pawn
			if(s.board[rank][file] == 'P')
			{
				player0++;
				impossibleToCheckmate = false;
			}
			else if(s.board[rank][file] == 'p')
			{
				player1++;
				impossibleToCheckmate = false;
			}
			
			//knight
			else if(s.board[rank][file] == 'N')
			{
				player0+=3;
				itrU = pieceSet.find('N');
				itrl = pieceSet.find('n');
				//avoid two knights
				if(itrU != pieceSet.end() && itrl != pieceSet.end())
				{
					impossibleToCheckmate = false;
				}
				else if(impossibleToCheckmate)
				{
					pieceSet.insert('N');
				}
			}
			else if(s.board[rank][file] == 'n')
			{
				player1+=3;
				itrl = pieceSet.find('n');
				itrU = pieceSet.find('N');
				//Avoid two knights
				if(itrl != pieceSet.end() && itrU != pieceSet.end())
				{
					impossibleToCheckmate = false;
				}
				else if(impossibleToCheckmate)
				{
					pieceSet.insert('n');
				}
			}
			
			//bishop
			else if(s.board[rank][file] == 'B')
			{
				player0+=3;
				itrU = pieceSet.find('B');
				//avoid two bishops from white
				if(itrU != pieceSet.end())
				{
					impossibleToCheckmate = false;
				}
				else if(impossibleToCheckmate)
				{
					pieceSet.insert('B');
					wbr = rank;
					wbf = file;
				}
			}
			else if(s.board[rank][file] == 'b')
			{
				player1+=3;
				itrl = pieceSet.find('b');
				//avoid two bishops from black
				if(itrl != pieceSet.end())
				{
					impossibleToCheckmate = false;
				}
				else if(impossibleToCheckmate)
				{
					pieceSet.insert('b');
					bbr = rank;
					bbf = file;
				}
			}
			
			//rook
			else if(s.board[rank][file] == 'R')
			{
				player0+=5;
				impossibleToCheckmate = false;
			}
			else if(s.board[rank][file] == 'r')
			{
				player1+=5;
				impossibleToCheckmate = false;
			}
			
			//queen
			else if(s.board[rank][file] == 'Q')
			{
				player0+=9;
				impossibleToCheckmate = false;
			}
			else if(s.board[rank][file] == 'q')
			{
				player1+=9;
				impossibleToCheckmate = false;
			}
			
			//king
			else if(s.board[rank][file] == 'K')
			{
				whiteLose = false;
			}
			else if(s.board[rank][file] == 'k')
			{
				blackLose = false;
			}
		}
	}
	
	
	//check if it is impossible to check mate
	if(!whiteLose && !blackLose && !impossibleToCheckmate)
	{
		// King vs King
		if(pieceSet.size() == 0)
		{
			impossibleToCheckmate = true;
		}
		else
		{
			std::set<char>::iterator Nit, nit, Bit, bit;
			Nit = pieceSet.find('N');
			nit = pieceSet.find('n');
			Bit = pieceSet.find('B');
			bit = pieceSet.find('b');
			
			// There are Knight and Bishops
			if((Nit != pieceSet.end() || nit != pieceSet.end()) && ( Bit != pieceSet.end() || bit != pieceSet.end())) 
			{
				impossibleToCheckmate = false;
			}
			//Only Knight
			else if((Nit != pieceSet.end() || nit != pieceSet.end()) &&  Bit == pieceSet.end() && bit == pieceSet.end())
			{
				impossibleToCheckmate = true;
			}
			//Only Bishop
			else if( Nit == pieceSet.end() && nit == pieceSet.end())
			{
				//	Two Bishops
				if(Bit != pieceSet.end() && bit != pieceSet.end())
				{
					int absR = abs(wbr - bbr);
					int absF = abs(wbf - bbf);
					if((absR%2) != (absF%2))
					{
						impossibleToCheckmate = false;
					}
					else
					{
						impossibleToCheckmate = true;
					}
				}
				// One Bishop
				else if(Bit != pieceSet.end() || bit != pieceSet.end())
				{
					impossibleToCheckmate = true;
				}
				else
				{
					impossibleToCheckmate = false;
				}
			}
			else
			{
				impossibleToCheckmate = false;
			}
		}
	}
	
	//check for state repetition
	bool stateRep = true;
	//check for no capture or pawn advancement
	if(s.turnsLeft > 0)
	{
		stateRep = false;
	}
	else if(s.lastMoves.size() >= 8)
	{
		size_t i;
		for(i=0; i< 4; i++)
		{
			if((s.lastMoves[i].fromRank != s.lastMoves[i+4].fromRank) 
				|| (s.lastMoves[i].fromFile != s.lastMoves[i+4].fromFile))
			{
				stateRep = false;
				break;
			}
		}
	}
	else if(s.lastMoves.size() < 8)
	{
		stateRep = false;
	}
	
	//win or lose
	if(whiteLose)
	{
		player1 = 1000;
	}
	
	if(blackLose)
	{
		player0 = 1000;
	}
	
	//draw
	if(impossibleToCheckmate || stateRep || s.turnsWithNoPorC == 100)
	{
		if(playerID())
		{
			player1 = -200;
			player0 = 0;
		}
		else
		{
			player0 = -200;
			player1 = 0;
		}
	}
	
	
	//white player
	if(playerID() == 0)
	{
		return player0 - player1;
	}
	else
	{
		return player1 - player0;
	}
}

/***********************************************************************************/
int AI::drawOrWin(const myState &s)
{
	//This is for Stalemate
	int score = -200;
	
	//checkmate in white's favor
	myMoves possibleMoves = legalMoves(s, playerID());
	for(int i = 0; i < possibleMoves.size(); i++)
	{
		if(s.board[possibleMoves[i].toRank][possibleMoves[i].toFile] == 'k')
		{
			score = 1000;
		}
	}
	
	//checkmate in black's favor
	possibleMoves = legalMoves(s, !playerID());
	for(int i = 0; i < possibleMoves.size(); i++)
	{
		if(s.board[possibleMoves[i].toRank][possibleMoves[i].toFile] == 'K')
		{
			score = -1000;
		}
	}
	
	//reverse score if player1
	if(playerID() && score != -200)
	{
		score *= -1;
	}
	
	return score;
}

/***************************************************************************************/
time_t AI::timeHave()
{
	time_t currentTime = time(NULL);
	double timeLeft = double(players[ playerID() ].time()) / 100.00;
	
	double timeGiven = timeLeft /double(pieces.size() + 75);
	if(timeGiven < 1 || moves.size() < 10) timeGiven = 1;
	
	return currentTime + timeGiven;
}
/*****************************************************************************************/
