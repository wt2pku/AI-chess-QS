#ifndef AI_H
#define AI_H

#include "BaseAI.h"
#include <iostream>
#include <cstdlib>
#include <time.h>
#include <queue>
#include <map>
using namespace std;

////////////////////////////////////////////////////////////////////////////////////////
/// @struct myMove
/// @brief This struct stores the information of a move
////////////////////////////////////////////////////////////////////////////////////////

struct myMove
{
	///The initial file location
	int fromFile;
	///The initial rank location
	int fromRank;
	///The final file location
	int toFile;
	///The final rank location
	int toRank;
	///The type of the piece for pawn promotion. Q=Queen, B=Bishop, N=Knight, R=Rook
	int promoteType;
	/// The player of the move
	int player;
};
typedef std::vector<myMove> myMoves;

/////////////////////////////////////////////////////////////////////////////////////
/// @struct myState
/// @brief This struct stores the board information of a state and the previous move
////////////////////////////////////////////////////////////////////////////////////

struct myState
{
	///The board states
	char board[8][8];
	///If the piece has moved
	bool hasMoved[8][8];
	///Turns left for draws
	int turnsLeft;
	///Turns to draw because of no pawn advancement or capture
	int turnsWithNoPorC;
	///last seven moves
	myMoves lastMoves;
	///The action
	myMove move;
	///The score in History Table
	int histScore;
	///If this state is a quite state
	int isQS;
};

class state_comp
{
	public:
		bool operator() (myState & lhs, myState & rhs)
		{
			//so it makes the priority queue top be the largest
			if(lhs.histScore > rhs.histScore)
				return true;
			else
				return false;
		}
};

class move_comp
{
	public:
		bool operator() (const myMove & lhs, const myMove & rhs)
		{
			//so it makes the priority queue top be the largest
			if(lhs.toFile > rhs.toFile)
				return true;
			else
				return false;
		}
};

typedef std::priority_queue<myState, std::vector<myState>, state_comp> myStates;



///The class implementing gameplay logic.

////////////////////////////////////////////////////////////////////////////////////
/// @fn myMove AI::nextMove(const myState)
/// @brief This function determines the next legal move of current state based on
/// random search
/// @param &oldState is the current state of the board
/// @return Next legal move based on random search
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/// @fn myState AI::newState(myState, const myMove &m)
/// @brief This function determine the next state after the legal move
/// @param s is the current state
/// @param m is the legal move for state s
/// @return RESULT(s, m)
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/// @fn myStates AI::nextStates(const myState, int player)
/// @brief This function determine all the possible next states for current state
/// @param s is the current state
/// @param player indicate if we find next states for white player or black player
/// @return The possible next states for current state
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/// @fn myMoves AI::legalMoves(const myState,int player, bool inCheck)
/// @brief This function determines all the legal moves for current state
/// @param s is the current state
/// @param player determine we find the moves for white player or black player
/// @param inCheck indicates if the King is in check
/// @return All the legal moves for current state
////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
/// @fn bool AI::inCheck(myState s, int player)
/// @brief This function returns if the player in the state can cause a checkmate
/// @param s is current state
/// @param player is the Player that can capture the opponent's 'King'
/// @return if the player has potential moves to capture the opponent's 'King'
/////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
/// @fn bool AI::KingMove(myState s, int file, int rank, int player, myMoves &nextMoves, bool)
/// @brief This function find the possible moves for King and determine if that piece
/// can capture the opponent's 'King'
/// @param s is current state
/// @param file is the current file
/// @param rank is the current rank
/// @param player is the Player in turn and that can capture the opponent's 'King'
/// @param nextMoves contains the possible moves for the player
/// @param kingInCheck is the flag indicates if the king is in check
/// @return if the player has potential moves to capture the opponent's 'King'
/////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
/// @fn bool AI::QueenMove(myState s, int file, int rank, int player, myMoves &nextMoves)
/// @brief This function find the possible moves for Queen and determine if the piece
/// can capture the opponent's 'King'
/// @param s is current state
/// @param file is the current file
/// @param rank is the current rank
/// @param player is the Player in turn and that can capture the opponent's 'King'
/// @param nextMoves contains the possible moves for the player
/// @return if the pieces has potential moves to capture the opponent's 'King'
/////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
/// @fn bool AI::BishopMove(myState s, int file, int rank, int player, myMoves &nextMoves)
/// @brief This function find the possible moves for Bishop and determine if that piece
/// can capture the opponent's 'King'
/// @param s is current state
/// @param file is the current file
/// @param rank is the current rank
/// @param player is the Player in turn and that can capture the opponent's 'King'
/// @param nextMoves contains the possible moves for that piece
/// @return if the piece has potential moves to capture the opponent's 'King'
/////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
/// @fn bool AI::RookMove(myState s, int file, int rank, int player, myMoves &nextMoves)
/// @brief This function find the possible moves for Rook and determine if that piece
/// can capture the opponent's 'King'
/// @param s is current state
/// @param file is the current file
/// @param rank is the current rank
/// @param player is the Player in turn and that can capture the opponent's 'King'
/// @param nextMoves contains the possible moves for that piece
/// @return if the piece has potential moves to capture the opponent's 'King'
/////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
/// @fn bool AI::KnightMove(myState s, int file, int rank, int player, myMoves &nextMoves)
/// @brief This function find the possible moves for Knight and determine if that piece
/// can capture the opponent's 'King'
/// @param s is current state
/// @param file is the current file
/// @param rank is the current rank
/// @param player is the Player in turn and that can capture the opponent's 'King'
/// @param nextMoves contains the possible moves for that piece
/// @return if the piece has potential moves to capture the opponent's 'King'
/////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
/// @fn bool AI::PawnMove(myState s, int file, int rank, int player, myMoves &nextMoves)
/// @brief This function find the possible moves for Pawn and determine if that piece
/// can capture the opponent's 'King'
/// @param s is current state
/// @param file is the current file
/// @param rank is the current rank
/// @param player is the Player in turn and that can capture the opponent's 'King'
/// @param nextMoves contains the possible moves for that piece
/// @return if the piece has potential moves to capture the opponent's 'King'
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/// @fn bool AI::isWhite(char)
/// @brief This function determines if a piece is for white player or not
/// @param c is the char for piece on the board
/// @return If this is a white piece, return true. Otherwise, return false
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/// @fn bool AI::isBlack(char)
/// @brief This function determines if a piece is for black player or not
/// @param c is the char for piece on the board
/// @return If this is a black piece, return true. Otherwise, return false
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/// @fn int AI::alphaBetaMax(const myState &s, int alpha, int beta, int depthleft)
/// @biref This function return max value in minimax
/// @param s is the current evaluated state
/// @param alpha is the highest value
/// @param beta is the lowest value
/// @param depth left is the depth left for recursion
///////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/// @fn int AI::alphaBetaMin(const myState &s, int alpha, int beta, int depthleft)
/// @biref This function returns min value in minimax
/// @param s is the current evaluated state
/// @param alpha is the highest value
/// @param beta is the lowest value
/// @param depth left is the depth left for recursion
///////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
/// @fn int AI::evaluate(const myState &s)
/// @brief This function returns the evaluation for state s
/// @param s is the state for evaluation
/// @return the score for state s
/////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
/// @fn int AI::drawOrWin(const myState &s)
/// @brief This function returns the evaluation for state s if there is a draw or a win
/// @param s is the state for evaluation
/// @return the score for state s
/////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
/// @fn time_t AI::timeHave()
/// @brief This function returns the time for the search
/// @return return the time that can be used for search
/////////////////////////////////////////////////////////////////////////////////////

class AI: public BaseAI
{
public:
  AI(Connection* c);
  
  virtual const char* username();
  virtual const char* password();
  virtual void init();
  virtual bool run();
  virtual void end();
  
  virtual myMove nextMove(const myState &oldState);
  
  virtual myState newState(myState s, const myMove &m, int player);
  
  virtual myStates nextStates(const myState &s, int player);
  
  virtual myMoves legalMoves(const myState &s, int player, bool kingInCheck = false);
  
  virtual bool inCheck(const myState &s, int player);
  
  virtual bool KingMove(myState s, int file, int rank, int player, myMoves &nextMoves, bool kingInCheck = false);
  
  virtual bool QueenMove(myState s, int file, int rank, int player, myMoves &nextMoves);
  
  virtual bool BishopMove(myState s, int file, int rank, int player, myMoves &nextMoves);
  
  virtual bool RookMove(myState s, int file, int rank, int player, myMoves &nextMoves);
  
  virtual bool KnightMove(myState s, int file, int rank, int player, myMoves &nextMoves);
  
  virtual bool PawnMove(myState s, int file, int rank, int player, myMoves &nextMoves);
  
  virtual bool isWhite(char c);
  
  virtual bool isBlack(char c);
  
  virtual int alphaBetaMax(const myState &s, int alpha, int beta, int depthleft);
  
  virtual int alphaBetaMin(const myState &s, int alpha, int beta, int depthleft);
  
  virtual int evaluate(const myState &s);
  
  virtual int drawOrWin(const myState &s);
  
  virtual int QSMin(const myState &s, int depth, int alpha, int beta);
  
  virtual int QSMax(const myState &s, int depth, int alpha, int beta);
  
  virtual time_t timeHave();
  
  private:
		//history table for two players, differ from 'player' in myMove
		std::map<myMove, int, move_comp> history;
 
};

#endif
