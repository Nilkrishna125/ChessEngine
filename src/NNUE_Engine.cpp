#include "chess.hpp"
#include "nnue.hpp"

#include <iostream>
#include <vector>
#include <fstream>
#include <random>
#include <chrono>
#include <stack>

using namespace std;
using namespace chess;

using result = pair<long long, vector<string>>;

int deadline = 4;

stack<AccumulatorPair> acc_p_stk;
AccumulatorPair acc_p;

const int KING_VALUE = 20000;

bool time_up(chrono::time_point<chrono::steady_clock> start, int time_remain){
    auto now = chrono::steady_clock::now();
    auto elapsed = chrono::duration_cast<chrono::milliseconds>(now - start).count();
    if(elapsed >= time_remain)return 1;
    return 0;
}

// Main evaluation function
int evaluate(const Board& board, int depth) {

    Movelist moves;
    movegen::legalmoves(moves, board);

    if (moves.empty()){
        if (board.inCheck()) {
            return (board.sideToMove() == chess::Color::WHITE) ? -(depth+1)*KING_VALUE : (depth+1)*KING_VALUE;
        } else {
            return 0;
        }
    }

    if (board.sideToMove() == chess::Color::WHITE){
        return eval_nnue(&network, &acc_p.white, &acc_p.black);
    }

    return eval_nnue(&network, &acc_p.black, &acc_p.white);
}


result alpha_beta_pruning(Board &board, int alpha, int beta, int depth, int player){

    bool max_player = (board.sideToMove() == Color::WHITE);

    Movelist moves;
    movegen::legalmoves(moves, board);


    if (moves.empty()){
        if (board.inCheck()) {
            int a = (board.sideToMove() == Color::WHITE) ? -(depth+1)*KING_VALUE : (depth+1)*KING_VALUE;
            return {a, {}};

        } else {
            return {0, {}};
        }
    }

    if (depth == 0){
        return {evaluate(board, depth), {}};
    }

    if (max_player) {
        int utility = -1000*KING_VALUE;
        result temp;
        temp.first = 0;
        for (Move action : moves) {
            acc_p_stk.push(acc_p);
            update(&network, &acc_p.white, &acc_p.black, action, board);
            board.makeMove(action);
            result taking_new = alpha_beta_pruning(board, alpha, beta, depth-1, player);
            board.unmakeMove(action);
            acc_p = acc_p_stk.top();
            acc_p_stk.pop();
            
            if(taking_new.first > utility){
                taking_new.second.push_back(uci::moveToSan(board, action));
                temp.second = taking_new.second;
                utility = taking_new.first;
                temp.first = utility;
            }
            alpha = max(alpha, utility);
            if(alpha >= beta) break;
        }
        return temp;
    }
    else {
        int utility = 1000*KING_VALUE;
        result temp;
        temp.first = 0;
        for (Move action : moves) {
            acc_p_stk.push(acc_p);
            update(&network, &acc_p.black, &acc_p.white, action, board);
            board.makeMove(action);
            result taking_new = alpha_beta_pruning(board, alpha, beta, depth-1, player);
            board.unmakeMove(action);
            acc_p = acc_p_stk.top();
            acc_p_stk.pop();
            
            if(taking_new.first < utility){
                taking_new.second.push_back(uci::moveToSan(board, action));
                temp.second = taking_new.second;
                utility = taking_new.first;
                temp.first = utility;
            }
            beta = min(beta, utility);
            if(alpha >= beta) break;
        }
        return temp;
    }
}

// result time_constraint_search(Board &board, int max_player, int alpha, int beta, int depth, int time_remained){

//     auto start = chrono::steady_clock::now();
//     Movelist legelmoves;
//     movegen::legalmoves(legelmoves, board);
//     Movelist best_moves_stored;
//     best_moves_stored.add(legelmoves[0]);

//     result search; //for variable depth ( see the for loop )
//     float time_remain = (time_remained/20 > 10) ? time_remained/20 : time_remained/10;

//     for (int i = 1; i <= deadline; i++){

//         auto now = chrono::steady_clock::now();
//         auto elapsed = chrono::duration_cast<chrono::milliseconds>(now - start).count(); // int or long long

//         search = alpha_beta_pruning(board, alpha, beta, i, max_player, best_moves_stored, start, time_remain);

//         if(elapsed >= time_remain){
//             result search;
//             search.second.push_back(uci::moveToSan(board, best_moves_stored[0]));
//             search.first = -1;
//             return search;
//         }
        
//         int size = search.second.size();

//         best_moves_stored.clear();
//         for(int j=0; j<size; j++){
//             Move best_move = uci::parseSan(board, search.second[j]);
//             board.makeMove(best_move);
//             best_moves_stored.add(best_move);
//         }
//         for(int j=size-1; j>=0 ;j--){
//             board.unmakeMove(best_moves_stored[j]);
//         }
//     }

//     return search;
// }

string best_move(Board &board, int time_remained = 2000000000){

    if (board.getFen() == "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"){
        vector<string> openings = {"e4", "d4", "Nf3", "c4"};
        mt19937 rng(random_device{}());
        uniform_int_distribution<> dist(0, openings.size() - 1);
        string san_move = openings[dist(rng)];
        Move move = uci::parseSan(board, san_move);
        update(&network, &acc_p.white, &acc_p.black, move, board);
        return san_move;
    }

    int a = (board.sideToMove() == Color::WHITE) ? 1 : 0;
    int alpha = -KING_VALUE, beta = KING_VALUE;
    result move_and_val = alpha_beta_pruning(board, alpha, beta, deadline, a);
    return *(move_and_val.second.end()-1);
    // *(move_and_val.end()-1)
}

void uci_loop(Board &board) {
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
        } else if (line.rfind("position", 0) == 0) {
            istringstream words(line);
            string word;
            words >> word;
            words >> word;
            if(word == "startpos"){
                string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
                board.setFen(fen);
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
        } else if (line.rfind("go", 0) == 0) {
            istringstream words(line);
            string word;
            words >> word;
            words >> word;
            if(word == "wtime"){
                int time_remained;
                words >> word;
                if (board.sideToMove() == Color::WHITE) 
                    time_remained = stoi(word);
                else {
                    words >> word;
                    words >> word;
                    time_remained = stoi(word);
                }
                network.initialise_acc(board, &acc_p);
                string best_mov = best_move(board, time_remained);
                cout << "bestmove " << best_mov << endl; 
            }
            else {
                network.initialise_acc(board, &acc_p);
                string best_mov = best_move(board);
                cout << "bestmove " << best_mov << endl; 
            } 
        } else if (line == "quit") {
            break;
        }
        else {
            if(line != "")cout << "Invalid Command !" << endl;
        }
    }
}

int main(){
    Board board;
    network.load_from_file("../nn_ws/models/shallow.nnue");
    uci_loop(board);
    return 0;
}