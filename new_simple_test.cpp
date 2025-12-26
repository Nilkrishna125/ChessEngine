// #include <iostream>
// #include <sstream>
// using namespace std;

// string best_move(string given){
//     istringstream move_list(given);
//     string move;
//     while(move_list>>move);
//     return move;
// }

// int main(){
//     cout<<best_move("hwllo jj kkd jjdf iafkn   ");
//     return 0;
// }

#include <iostream>
#include "chess.hpp"
#include <vector>
#include <math.h>
#include <sstream>
using namespace std;
using namespace chess;

string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
Board board(fen);

struct to_return{
    int val;
    string best_string;
};

int stop = 0;

to_return alpha_beta_pruning(Board board, int max_player, int alpha, int beta, int depth){
    Movelist moves;
    movegen::legalmoves(moves,board);
    to_return values;
    values.best_string = "";
    values.val = 0;

    if(depth<stop){
        values.val = -1000;
        values.best_string = " ";
        return values;
    }

    if (moves.empty()){
        if (board.inCheck()) {
            int a = (board.sideToMove() == chess::Color::WHITE) ? -1000000 : 1000000;
            values.val = a;
            values.best_string = " ";
            stop=depth;
            return values;
        } else {
            values.val = 0;
            values.best_string = " ";
            stop=depth;
            return values;
        }
    }

    if(depth==0){
        values.val = -1000;
        values.best_string = " ";
        return values;
    }

    if (max_player) {
        int utility=-100000000;
        to_return temp;
        temp.best_string = "";
        temp.val = 0;
        for (Move action : moves) {
            board.makeMove(action);
            values = alpha_beta_pruning(board, 0, alpha, beta, depth-1);
            board.unmakeMove(action);
            
            if(values.val >= utility){
                temp.best_string = values.best_string + uci::moveToSan(board, action) + " ";
                utility = values.val;
                temp.val = utility;
            }
            alpha = max(alpha,utility);
            if(alpha>=beta)break;
        }
        return temp;
    }

    else {
        int utility=100000000;
        to_return temp;
        temp.best_string = "";
        temp.val = 0;
        for (Move action : moves) {
            board.makeMove(action);
            values = alpha_beta_pruning(board, 1, alpha, beta, depth-1);
            board.unmakeMove(action);
            
            if(values.val < utility){
                temp.best_string = values.best_string + uci::moveToSan(board, action) + " ";
                utility = values.val;
                temp.val = utility;
            }
            beta = min(beta, utility);
            if(alpha>=beta)break;
        }
        
        return temp;
    }
}

string best_move(){
    int a = (board.sideToMove() == Color::WHITE) ? 1 : 0;
    to_return move_and_val = alpha_beta_pruning(board, a, -100000000, 100000000, 5);
    istringstream move_list(move_and_val.best_string);
    string move;
    while(move_list>>move);
    Move move_object = uci::parseSan(board, move);
    move = uci::moveToUci(move_object);
    return move;
}

void uci_loop() {
    string line;
    while (getline(cin, line)) {
        if (line == "uci") {
            cout << "id name TinguChess" << endl;
            cout << "id author Tingi" << endl;
            cout << "uciok" << endl;
        } else if (line == "isready") {
            cout << "readyok" << endl;
        } else if (line == "ucinewgame") {
            string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
            board.setFen(fen);
            // Reset internal board and states
        } else if (line.rfind("position", 0) == 0) {
            istringstream words(line);
            string word;
            words >> word;
            words >> word;
            if(word == "startpos"){
                words >> word;
                while(words >> word){
                    Move move = uci::uciToMove(board, word);
                    board.makeMove(move);
                }
            }
            else if(word == "fen"){
                string fen_string;
                for(int i = 0; i < 5; i++){
                    words >> word;
                    fen_string += word + " ";
                }
                words >> word;
                fen_string += word;
                words >> word;
                board.setFen(fen_string);
                while(words >> word){
                    Move move = uci::uciToMove(board, word);
                    board.makeMove(move);
                }

            }
            else {
                cout << "invalid position type" << endl;
            }
            // Parse position and apply moves
        } else if (line.rfind("go", 0) == 0) {
            // Start search and output bestmove
            string best_mov = best_move();
            cout << "bestmove " << best_mov << endl;  

        } else if (line == "quit") {
            break;
        }
    }
}

int main(){
    uci_loop();
    // to_return see_move;
    // see_move = alpha_beta_pruning(board, 1, -100000000, 100000000, 150);
    // cout<<see_move.best_string<<endl;
    return 0;
}