#include <iostream>
#include "chess.hpp"
#include <vector>
#include <math.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>
using namespace std;
using namespace chess;

struct to_return{
    int val;
    string best_string;
};

int count_words(const string& s, char delimiter) {
    int count = 0;
    bool in_word = false;

    for (char ch : s) {
        if (ch == delimiter) {
            count++;
        }
    }

    // Add 1 to count the last word (since there's no delimiter after it)
    return s.empty() ? 0 : count - 1;
}

to_return alpha_beta_pruning(Board board, int max_player, int i, int alpha, int beta){
    Movelist moves;
    movegen::legalmoves(moves,board);
    to_return values;
    values.best_string = "";
    values.val = 0;

    

    if (moves.empty()){
        if (board.inCheck()) {
            int a = (board.sideToMove() == chess::Color::WHITE) ? -1000000 : 1000000;
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
        temp.best_string = "";
        temp.val = 0;
        for (Move action : moves) {
            board.makeMove(action);
            values = alpha_beta_pruning(board, 0, i-1, alpha, beta);
            board.unmakeMove(action);
            
            if(values.val >= utility){
                temp.best_string = values.best_string + uci::moveToSan(board, action) + ",";
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
            values = alpha_beta_pruning(board, 1, i-1, alpha, beta);
            board.unmakeMove(action);
            
            if(values.val < utility){
                temp.best_string = values.best_string + uci::moveToSan(board, action) + ",";
                utility = values.val;
                temp.val = utility;
            }
            beta = min(beta, utility);
            if(alpha>=beta)break;
        }
        
        return temp;
    }
}


// Function to clean and parse moves from string
vector<string> parseMoves(const string& moveString) {
    vector<string> moves;
    stringstream ss(moveString);
    string move;
    
    while (getline(ss, move, ',')) {
        // Remove leading and trailing whitespace
        move.erase(0, move.find_first_not_of(" \t"));
        move.erase(move.find_last_not_of(" \t") + 1);
        
        if (!move.empty()) {
            moves.push_back(move);
        }
    }
    
    return moves;
}

// Function to parse moves from engine output (reverses order)
vector<string> parseEngineMoves(const string& moveString) {
    vector<string> moves = parseMoves(moveString);
    // Reverse the order since engine builds moves backwards
    reverse(moves.begin(), moves.end());
    return moves;
}

// Function to parse JSON manually
map<string, string> parseJsonFile(const string& filename) {
    map<string, string> result;
    ifstream file(filename);
    string line, content;
    
    if (!file.is_open()) {
        cerr << "Error: Could not open " << filename << endl;
        return result;
    }
    
    // Read entire file
    while (getline(file, line)) {
        content += line;
    }
    file.close();
    
    // Remove outer braces
    size_t start = content.find('{');
    size_t end = content.rfind('}');
    if (start != string::npos && end != string::npos) {
        content = content.substr(start + 1, end - start - 1);
    }
    
    // Parse key-value pairs
    stringstream ss(content);
    string pair;
    
    while (getline(ss, pair, ',')) {
        // Find the colon that separates key and value
        size_t colonPos = pair.find(':');
        if (colonPos != string::npos) {
            string key = pair.substr(0, colonPos);
            string value = pair.substr(colonPos + 1);
            
            // Remove quotes and whitespace
            key.erase(remove(key.begin(), key.end(), '\"'), key.end());
            key.erase(remove(key.begin(), key.end(), ' '), key.end());
            key.erase(remove(key.begin(), key.end(), '\n'), key.end());
            key.erase(remove(key.begin(), key.end(), '\t'), key.end());
            
            value.erase(remove(value.begin(), value.end(), '\"'), value.end());
            // Keep spaces in value but remove leading/trailing
            value.erase(0, value.find_first_not_of(" \t\n"));
            value.erase(value.find_last_not_of(" \t\n") + 1);
            
            if (!key.empty() && !value.empty()) {
                result[key] = value;
            }
        }
    }
    
    return result;
}

// Function to normalize moves for comparison (remove special characters like +, #)
string normalizeMoveForComparison(const string& move) {
    string normalized = move;
    // Remove check (+) and checkmate (#) symbols
    normalized.erase(remove(normalized.begin(), normalized.end(), '+'), normalized.end());
    normalized.erase(remove(normalized.begin(), normalized.end(), '#'), normalized.end());
    return normalized;
}

// Function to check if moves match (considering different notations)
bool movesMatch(const vector<string>& engineMoves, const vector<string>& expectedMoves) {
    if (engineMoves.size() != expectedMoves.size()) {
        return false;
    }
    
    for (size_t i = 0; i < engineMoves.size(); i++) {
        string engineMove = normalizeMoveForComparison(engineMoves[i]);
        string expectedMove = normalizeMoveForComparison(expectedMoves[i]);
        
        if (engineMove != expectedMove) {
            return false;
        }
    }
    
    return true;
}

int main(){
    // Read JSON file manually
    map<string, string> matePositions = parseJsonFile("../Week3/mate_in_3.json");
    
    if (matePositions.empty()) {
        cerr << "Error: Could not parse mate.json or file is empty" << endl;
        return 1;
    }
    
    int totalPositions = 0;
    int matchingPositions = 0;
    int nonMatchingPositions = 0;
    
    cout << "=== CHESS POSITION ANALYSIS ===" << endl;
    cout << "Comparing alpha-beta pruning results with expected moves\n" << endl;
    
    for (auto& [fen, expectedMoveString] : matePositions) {
        totalPositions++;
        
        try {
            chess::Board board(fen);
            to_return result = alpha_beta_pruning(board, 1, 5, -100000, 100000);
            // Parse moves from both strings
            vector<string> engineMoves = parseEngineMoves(result.best_string);
            vector<string> expectedMoves = parseMoves(expectedMoveString);
            
            // Remove empty moves
            engineMoves.erase(remove_if(engineMoves.begin(), engineMoves.end(), 
                                      [](const string& s) { return s.empty(); }), engineMoves.end());
            
            bool matches = movesMatch(engineMoves, expectedMoves);
            
            if (matches) {
                matchingPositions++;
                cout << "✓ MATCH - Position: " << totalPositions << endl;
            } else {
                nonMatchingPositions++;
                cout << "✗ NO MATCH - Position: " << totalPositions << endl;
                cout << "  FEN: " << fen << endl;
                cout << "  Expected: " << expectedMoveString << endl;
                cout << "  Engine:   ";
                for (size_t i = 0; i < engineMoves.size(); i++) {
                    cout << engineMoves[i];
                    if (i < engineMoves.size() - 1) cout << " ";
                }
                cout << endl;
                cout << "  Raw output: " << result.best_string << endl;
                cout << endl;
            }
            
        } catch (const exception& e) {
            cout << "✗ ERROR - Position: " << totalPositions << endl;
            cout << "  FEN: " << fen << endl;
            cout << "  Error: " << e.what() << endl << endl;
            nonMatchingPositions++;
        }
    }
    
    cout << "=== SUMMARY ===" << endl;
    cout << "Total positions tested: " << totalPositions << endl;
    cout << "Matching positions: " << matchingPositions << endl;
    cout << "Non-matching positions: " << nonMatchingPositions << endl;
    cout << "Success rate: " << (totalPositions > 0 ? (double)matchingPositions / totalPositions * 100 : 0) << "%" << endl;
    
    return 0;
}