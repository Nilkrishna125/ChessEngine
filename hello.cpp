#include <iostream>
#include "chess.hpp"
#include <vector>
#include <math.h>
using namespace std;
using namespace chess;

struct to_return{
    int val;
    string best_string;
};

to_return alpha_beta_pruning(Board& board, int max_player, int i, int alpha, int beta){
    Movelist moves;
    movegen::legalmoves(moves,board);
    to_return values;
    values.best_string = "";
    values.val = 0;

    if (moves.empty()){
        if (board.inCheck()) {
            int a = (board.sideToMove() == chess::Color::WHITE) ? -1 : 1;
            values.val = a;
            values.best_string = ",";
            return values;
        } else {
            values.val = 0;
            values.best_string = ",";
            return values;
        }
    }

    if(i==0){
        values.val = -1000;
        values.best_string = ",";
        return values;
    }

    if (max_player) {
        int utility=-100000000;
        to_return temp;
        temp.val = 0;
        temp.best_string = "";
        for (Move action : moves) {
            board.makeMove(action);
            values = alpha_beta_pruning(board, 0, i-1, alpha, beta);
            board.unmakeMove(action);
            
            if(values.val >= utility){
                temp.best_string = values.best_string + uci::moveToSan(board, action) + ",";
                utility = values.val;
                temp.val = utility;
            }
            alpha = max(alpha,values.val);
            if(alpha>=beta)break;
        }
        return temp;
    }

    else {
        int utility=100000000;
        to_return temp;
        temp.val = 0;
        temp.best_string = "";
        for (Move action : moves) {
            board.makeMove(action);
            values = alpha_beta_pruning(board, 1, i-1, alpha, beta);
            board.unmakeMove(action);
            if(values.val < utility){
                temp.best_string = values.best_string + uci::moveToSan(board, action) + ","; 
                utility = values.val;
                temp.val = utility;
            }
            beta = min(beta, values.val);
            if(alpha>=beta)break;
        }      
        return temp;
    }
}

int main(){
    string fen = "rn2k2r/pp2b2p/2p1Q1p1/5B2/1q3B2/8/PPP3PP/3RR2Kwkq-10";
    chess::Board board(fen);
    // to_return see_move;
    // see_move = alpha_beta_pruning(board, 1, 3, -1000000000, 1000000000);
    // cout<<see_move.best_string<<endl;


    board.makeMove(uci::parseSan(board,"Qxg6+"));
    
    board.makeMove(uci::parseSan(board,"hxg6"));
    Movelist checkone;
    movegen::legalmoves(checkone, board);
    for(Move x: checkone){
        board.makeMove(x);
        cout<<board.inCheck()<<endl;
        Movelist moves;
        movegen::legalmoves(moves, board);
        cout<<moves.empty()<<endl;
        cout<<"hello"<<endl;
        board.unmakeMove(x);
    }

    // to_return see_move2 = alpha_beta_pruning(board, 0, 2, -100000, 100000);
    // cout<<see_move2.best_string<<endl;
    // Movelist moves;
    // movegen::legalmoves(moves, board);
    // for(Move x: moves)cout<<uci::moveToSan(board, x)<<" ";
    // cout<<endl;

    return 0;
}