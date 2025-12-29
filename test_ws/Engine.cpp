#include <iostream>
#include "chess.hpp"
#include <vector>
#include <math.h>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <random>
#include <string>
#include <chrono>

using namespace std;
using namespace chess;

int n = 15;
int depth_quie = 8;
int nodes = 0;
int total_nodes = 0;

struct to_return{
    int val;
    string best_string;
};

bool time_up(chrono::time_point<chrono::steady_clock> start, int time_remain){
    auto now = chrono::steady_clock::now();
    auto elapsed = chrono::duration_cast<chrono::milliseconds>(now - start).count();
    if(elapsed >= time_remain)return 1;
    return 0;
}

// Material values in centipawns
const int PAWN_VALUE = 100;
const int KNIGHT_VALUE = 320;
const int BISHOP_VALUE = 330;
const int ROOK_VALUE = 500;
const int QUEEN_VALUE = 1200;
const int KING_VALUE = 20000;

// Piece-Square Tables (from white's perspective)
const int pawn_table[64] = {
    0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
    5,  5, 10, 25, 25, 10,  5,  5,
    0,  0,  0, 20, 20,  0,  0,  0,
    5, -5,-10,  0,  0,-10, -5,  5,
    5, 10, 10,-20,-20, 10, 10,  5,
    0,  0,  0,  0,  0,  0,  0,  0
};

const int knight_table[64] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50
};

const int bishop_table[64] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
};

const int rook_table[64] = {
    0,  0,  0,  0,  0,  0,  0,  0,
    5, 10, 10, 10, 10, 10, 10,  5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    0,  0,  0,  5,  5,  0,  0,  0
};

const int queen_table[64] = {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5,  5,  5,  5,  0,-10,
    -5,  0,  5,  5,  5,  5,  0, -5,
    0,  0,  5,  5,  5,  5,  0, -5,
    -10,  5,  5,  5,  5,  5,  0,-10,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
};

const int king_middlegame_table[64] = {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
    20, 20,  0,  0,  0,  0, 20, 20,
    20, 30, 10,  0,  0, 10, 30, 20
};

const int king_endgame_table[64] = {
    -50,-40,-30,-20,-20,-30,-40,-50,
    -30,-20,-10,  0,  0,-10,-20,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-30,  0,  0,  0,  0,-30,-30,
    -50,-30,-30,-30,-30,-30,-30,-50
};

// Helper function to get piece value
int getPieceValue(PieceType piece) {
    switch(piece) {
        case PieceType(PieceType::underlying::PAWN) : return PAWN_VALUE;
        case PieceType(PieceType::underlying::KNIGHT): return KNIGHT_VALUE;
        case PieceType(PieceType::underlying::BISHOP): return BISHOP_VALUE;
        case PieceType(PieceType::underlying::ROOK): return ROOK_VALUE;
        case PieceType(PieceType::underlying::QUEEN): return QUEEN_VALUE;
        case PieceType(PieceType::underlying::KING): return KING_VALUE;
        default: return 0;
    }
}

// Helper function to get piece-square table value
int getPieceSquareValue(PieceType piece, Square square, Color color, bool endgame = false) {
    int sq = square.index();
    if (color == Color::BLACK) {
        sq = 63 - sq; // Flip square for black
    }
    
    switch(piece) {
        case PieceType(PieceType::underlying::PAWN): return pawn_table[sq];
        case PieceType(PieceType::underlying::KNIGHT): return knight_table[sq];
        case PieceType(PieceType::underlying::BISHOP): return bishop_table[sq];
        case PieceType(PieceType::underlying::ROOK): return rook_table[sq];
        case PieceType(PieceType::underlying::QUEEN): return queen_table[sq];
        case PieceType(PieceType::underlying::KING): 
            return endgame ? king_endgame_table[sq] : king_middlegame_table[sq];
        default: return 0;
    }
}

// Count material to determine game phase
int countMaterial(const Board& board) {
    int material = 0;
    for (int sq = 0; sq < 64; sq++) {
        Square square = static_cast<Square>(sq);
        Piece piece = board.at(square);
        if (piece != Piece(Piece::underlying::NONE)) {
            PieceType type = piece.type(); 
            if (type != PieceType(PieceType::underlying::PAWN) && type != PieceType(PieceType::underlying::KING)) {
                material += getPieceValue(type);
            }
        }
    }
    return material;
}

// Count material to determine game phase
int countMaterial2(const Board& board) {
    int material = 0;
    for (int sq = 0; sq < 64; sq++) {
        Square square = static_cast<Square>(sq);
        Piece piece = board.at(square);
        if (piece != Piece(Piece::underlying::NONE)) {
            PieceType type = piece.type(); 
            int c = (piece.color() == Color::WHITE) ? 1 : -1 ;
            material += getPieceValue(type)*c;
        }
    }
    return material;
}
// Count bits in bitboard (number of ones in the bitstring)
int popcount(Bitboard bb) {
    return bb.count();
}

// Get least significant bit square (gives trailing zeros)
Square getLSB(Bitboard bb) {
    return Square(bb.lsb());
}

// Pop least significant bit
Bitboard popLSB(Bitboard bb) {
    uint8_t temp1 = bb.pop();
    return bb;
}

// Create bitboard from square
Bitboard squareToBitboard(Square sq) {
    return Bitboard::fromSquare(sq);
}

// Get file bitboard
Bitboard getFileBitboard(File file) {
    return Bitboard(file);
}

// Evaluate pawn structure
int evaluatePawns(const Board& board) {
    int score = 0;
    Bitboard whitePawns = board.pieces(PieceType::PAWN, Color::WHITE);
    Bitboard blackPawns = board.pieces(PieceType::PAWN, Color::BLACK);
    
    // Check for doubled, isolated, and passed pawns
    for (int fileNum = 0; fileNum < 8; fileNum++) {
        File file = static_cast<File>(fileNum);
        Bitboard fileBB = getFileBitboard(file);
        
        // Count pawns on this file
        Bitboard whitePawnsOnFile = whitePawns & fileBB;
        Bitboard blackPawnsOnFile = blackPawns & fileBB;
        
        // Doubled pawns penalty
        if (popcount(whitePawnsOnFile) > 1) {
            score -= 50;
        }
        if (popcount(blackPawnsOnFile) > 1) {
            score += 50;
        }
        
        // Adjacent files for isolation check
        Bitboard adjacentFiles = Bitboard(0);
        if (fileNum > 0) adjacentFiles |= getFileBitboard(static_cast<File>(fileNum - 1));
        if (fileNum < 7) adjacentFiles |= getFileBitboard(static_cast<File>(fileNum + 1));
        
        // Isolated pawns penalty
        if (whitePawnsOnFile && !(whitePawns & adjacentFiles)) {
            score -= 20; // Isolated white pawn
        }
        if (blackPawnsOnFile && !(blackPawns & adjacentFiles)) {
            score += 20; // Isolated black pawn
        }
        
        // Simple passed pawn detection
        if (whitePawnsOnFile) {
            Square pawnSq = getLSB(whitePawnsOnFile);
            Rank rank = pawnSq.rank();
            int rankNum = static_cast<int>(rank);
            
            Bitboard frontSpan = Bitboard(0);
            // Create front span
            for (int r = rankNum + 1; r < 8; r++) {
                frontSpan |= Rank(static_cast<Rank>(r)).bb();
            }
            Bitboard passedArea = frontSpan & (fileBB | adjacentFiles);
            if (!(blackPawns & passedArea)) {
                score += 50 + (rankNum - 1) * 10; // Bonus increases with advancement
            }
        }
        
        if (blackPawnsOnFile) {
            Square pawnSq = getLSB(blackPawnsOnFile);
            Rank rank = pawnSq.rank();
            int rankNum = static_cast<int>(rank);
            
            Bitboard frontSpan = Bitboard(0);
            // Create front span for black
            for (int r = rankNum - 1; r >= 0; r--) {
                frontSpan |= Rank(static_cast<Rank>(r)).bb();
            }
            Bitboard passedArea = frontSpan & (fileBB | adjacentFiles);
            if (!(whitePawns & passedArea)) {
                score -= 50 + (6 - rankNum) * 10; // Bonus increases with advancement
            }
        }
    }
    
    return score;
}

// // Evaluate piece mobility
// int evaluateMobility(const Board& board) {
//     int score = 0;
//     Movelist moves;
    
//     // Count legal moves for current side
//     movegen::legalmoves(moves, board);
//     int mobility = moves.size();
    
//     // Mobility bonus/penalty
//     if (board.sideToMove() == Color::WHITE) {
//         score += mobility * 3;
//     } else {
//         score -= mobility * 3;
//     }
//     return score;
// }

// Evaluate king safety
int evaluateKingSafety(const Board& board) {
    int score = 0;
    
    // Bonus for having castling rights
    Board::CastlingRights rights = board.castlingRights();
    if (rights.has(Color::WHITE, Board::CastlingRights::Side::KING_SIDE) ||
        rights.has(Color::WHITE, Board::CastlingRights::Side::QUEEN_SIDE)) {
        score += 30;
    }
    if (rights.has(Color::BLACK, Board::CastlingRights::Side::KING_SIDE) ||
        rights.has(Color::BLACK, Board::CastlingRights::Side::QUEEN_SIDE)) {
        score -= 30;
    }
    
    // Simple king safety evaluation based on pawn shield
    Square whiteKing = board.kingSq(Color::WHITE);
    Square blackKing = board.kingSq(Color::BLACK);
    
    Bitboard whitePawns = board.pieces(PieceType::PAWN, Color::WHITE);
    Bitboard blackPawns = board.pieces(PieceType::PAWN, Color::BLACK);
    
    // Check pawn shield around kings (simplified)
    File whiteKingFile = whiteKing.file();
    Rank whiteKingRank = whiteKing.rank();
    File blackKingFile = blackKing.file();
    Rank blackKingRank = blackKing.rank();
    
    int whiteKingFileNum = static_cast<int>(whiteKingFile);
    int whiteKingRankNum = static_cast<int>(whiteKingRank);
    int blackKingFileNum = static_cast<int>(blackKingFile);
    int blackKingRankNum = static_cast<int>(blackKingRank);
    
    // Bonus for pawns in front of king
    for (int file = max(0, whiteKingFileNum - 1); file <= min(7, whiteKingFileNum + 1); file++) {
        for (int rank = whiteKingRankNum + 1; rank <= min(7, whiteKingRankNum + 2); rank++) {
            Square sq = Square(static_cast<File>(file), static_cast<Rank>(rank));
            if (whitePawns & squareToBitboard(sq)) {
                score += 10;
            }
        }
    }
    
    for (int file = max(0, blackKingFileNum - 1); file <= min(7, blackKingFileNum + 1); file++) {
        for (int rank = max(0, blackKingRankNum - 2); rank < blackKingRankNum; rank++) {
            Square sq = Square(static_cast<File>(file), static_cast<Rank>(rank));
            if (blackPawns & squareToBitboard(sq)) {
                score -= 10;
            }
        }
    }
    
    return score;
}

// Check for basic endgame patterns
int evaluateEndgame(const Board& board) {
    int score = 0;
    
    // Check for insufficient material
    if (board.isInsufficientMaterial()) {
        return 0; // Force draw
    }
    
    // In endgames, centralize the king
    Square whiteKing = board.kingSq(Color::WHITE);
    Square blackKing = board.kingSq(Color::BLACK);
    
    File whiteKingFile = whiteKing.file();
    Rank whiteKingRank = whiteKing.rank();
    File blackKingFile = blackKing.file();
    Rank blackKingRank = blackKing.rank();
    
    int whiteKingFileNum = static_cast<int>(whiteKingFile);
    int whiteKingRankNum = static_cast<int>(whiteKingRank);
    int blackKingFileNum = static_cast<int>(blackKingFile);
    int blackKingRankNum = static_cast<int>(blackKingRank);
    
    // Distance to center bonus
    double whiteCenterDist = abs(whiteKingFileNum - 3.5) + abs(whiteKingRankNum - 3.5);
    double blackCenterDist = abs(blackKingFileNum - 3.5) + abs(blackKingRankNum - 3.5);
    
    score += (7 - whiteCenterDist) * 5;
    score -= (7 - blackCenterDist) * 5;
    
    return score;
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
    
    int score = 0;
    int totalMaterial = countMaterial(board);
    bool endgame = totalMaterial < 1300; // Endgame threshold
    
    // Material and piece-square tables
    for (int sq = 0; sq < 64; sq++) {
        Square square = static_cast<Square>(sq);
        Piece piece = board.at(square);
        
        if (piece != Piece::NONE) {
            PieceType type = piece.type();
            Color color = piece.color();
            
            int pieceValue = getPieceValue(type);
            int squareValue = getPieceSquareValue(type, square, color, endgame);
            
            if (color == Color::WHITE) {
                score += pieceValue + squareValue;
            } else {
                score -= pieceValue + squareValue;
            }
        }
    }
    
    // Positional evaluation
    score += evaluatePawns(board);
    // score += evaluateMobility(board);
    score += evaluateKingSafety(board);
    
    if (endgame) {
        score += evaluateEndgame(board);
    }
    score += countMaterial2(board);
    
    // Return score from current player's perspective
    return board.sideToMove() == Color::WHITE ? score : -score;
}

const int MVV_LVA_VALUES[6][6] = {
    // Victim: P  N  B  R  Q  K    Attacker:
    {105, 205, 305, 405, 505, 605}, // Pawn
    {104, 204, 304, 404, 504, 604}, // Knight
    {103, 203, 303, 403, 503, 603}, // Bishop
    {102, 202, 302, 402, 502, 602}, // Rook
    {101, 201, 301, 401, 501, 601}, // Queen
    {100, 200, 300, 400, 500, 600}  // King
};

// Helper function to get piece type index for MVV-LVA
int getPieceIndex(PieceType piece) {
    switch(piece) {
        case PieceType(PieceType::underlying::PAWN): return 0;
        case PieceType(PieceType::underlying::KNIGHT): return 1;
        case PieceType(PieceType::underlying::BISHOP): return 2;
        case PieceType(PieceType::underlying::ROOK): return 3;
        case PieceType(PieceType::underlying::QUEEN): return 4;
        case PieceType(PieceType::underlying::KING): return 5;
        default: return 0;
    }
}

// Get MVV-LVA score for a capture move
int getMvvLvaScore(const Board& board, const Move& move) {
    if (!board.isCapture(move)) return 0;
    
    Square from = move.from();
    Square to = move.to();
    
    Piece attacker = board.at(from);
    Piece victim = board.at(to);
    
    if (victim == Piece::NONE) {
        // En passant capture
        return MVV_LVA_VALUES[getPieceIndex(attacker.type())][0]; // Pawn victim
    }
    
    int attackerIndex = getPieceIndex(attacker.type());
    int victimIndex = getPieceIndex(victim.type());
    
    return MVV_LVA_VALUES[attackerIndex][victimIndex];
}

// Move scoring function for ordering
int getMoveScore(const Board& board, const Move& move) {

    Board tempBoard = board;
    tempBoard.makeMove(move);
    if (board.isCapture(move) && tempBoard.inCheck()){
        tempBoard.unmakeMove(move);
        return 12000 + getMvvLvaScore(board, move);
    }
    tempBoard.unmakeMove(move);

    // Highest priority: Captures (sorted by MVV-LVA)
    if (board.isCapture(move)) {
        return 10000 + getMvvLvaScore(board, move);
    }

    // Fourth priority: Quiet moves that give check
    tempBoard.makeMove(move);
    if (tempBoard.inCheck()) {
        tempBoard.unmakeMove(move);
        return 9000;
    }
    tempBoard.unmakeMove(move);
    
    
    // Second priority: Promotions
    if (move.typeOf() == Move::PROMOTION) {
        int promotionBonus = 0;
        switch(move.promotionType()) {
            case PieceType(PieceType::underlying::QUEEN): promotionBonus = 900; break;
            case PieceType(PieceType::underlying::ROOK): promotionBonus = 500; break;
            case PieceType(PieceType::underlying::BISHOP): promotionBonus = 300; break;
            case PieceType(PieceType::underlying::KNIGHT): promotionBonus = 300; break;
            default: promotionBonus = 0; break;
        }
        return 8000 + promotionBonus;
    }
    
    // Third priority: Castling
    if (move.typeOf() == Move::CASTLING) {
        return 7000;
    }
    
    // Lowest priority: Other quiet moves
    return 1000;
}

// Move ordering function
void orderMoves(const Board& board, Movelist& moves, Move& passed_move) {
    sort(moves.begin(), moves.end(), [&](const Move& move1, const Move& move2) {
        int score1, score2;
        if(move1 == passed_move){
            score1 = 15000;
        }
        else {
            score1 = getMoveScore(board, move1);
        }

        if(move2 == passed_move) {
            score2 = 15000;
        }
        else {
            score2 = getMoveScore(board, move2);
        }
        return score1 > score2; // Higher scores first
    });
}

void orderMoves2(const Board& board, Movelist& moves) {
    sort(moves.begin(), moves.end(), [&](const Move& move1, const Move& move2) {
        int score1, score2;
        score1 = getMoveScore(board, move1);
        score2 = getMoveScore(board, move2);
        return score1 > score2; // Higher scores first
    });
}

int Quiesce( int alpha, int beta, Board& board, int depth, Movelist& passed_list, chrono::time_point<chrono::steady_clock> start, int time_remain) {

    if(time_up(start, time_remain)){
        return 0;
    }

    Movelist allMoves;
    movegen::legalmoves(allMoves, board);

    int best_value ;

    if (allMoves.empty()){
        if (board.inCheck()) {
            best_value = (board.sideToMove() == Color::WHITE) ? -(depth+1)*KING_VALUE : (depth+1)*KING_VALUE;
        } else {
            best_value = 0;
        }
    }
    else {
        best_value = evaluate(board, depth);
    }

    if(depth == 0)return best_value;
    if( best_value >= beta )
        return best_value;
    if( best_value > alpha )
        alpha = best_value;

    // Filter to get only captures
    Movelist captures, updated_list;
    Move check;
    int i=0;

    if (passed_list.size()){
            auto it = std::find(allMoves.begin(), allMoves.end(), passed_list[0]);
            if(it != allMoves.end()){
                captures.add(passed_list[0]);
                check = passed_list[0];
                i++;
                if(passed_list.size() > 1)
                    for(int i=1; i<passed_list.size(); i++)
                        updated_list.add(passed_list[i]);
            }
        }

    for (const Move& move : allMoves) {

        if(i>0 &&  move == check)continue;

        if (board.isCapture(move)) {  // The Board class has this method!
            captures.add(move);
        }
        else if (board.inCheck()){
            captures.add(move);
        }
        else {
            Board temp_board = board;
            temp_board.makeMove(move);
            if(temp_board.inCheck())captures.add(move);
            temp_board.unmakeMove(move);
        }
    }
    
    if (captures.size()){
        for (const Move& move : captures)  {
            board.makeMove(move);
            int score = -Quiesce( -beta, -alpha, board, depth - 1, updated_list, start, time_remain);
            board.unmakeMove(move);

            if( score >= beta )
                return score;
            if( score > best_value )
                best_value = score;
            if( score > alpha )
                alpha = score;
        }
    }
    return best_value;
}

to_return alpha_beta_pruning(Board &board, int alpha, int beta, int depth, int stop, int bot, Movelist& best_moves_stored, chrono::time_point<chrono::steady_clock> start, int time_remain){

    if(time_up(start, time_remain)){
        to_return kahi_pn;
        kahi_pn.best_string = "";
        kahi_pn.val = 0;
        return kahi_pn;
    }

    bool max_player = (board.sideToMove() == Color::WHITE);
    nodes++;

    if (board.getFen() == "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"){
    to_return default_start;
    vector<string> openings = {"e4", "d4", "Nf3", "c4"};
    mt19937 rng(random_device{}());
    uniform_int_distribution<> dist(0, openings.size() - 1);
    default_start.best_string = openings[dist(rng)] + " ";
    default_start.val = 0;
    return default_start;
    }

    Movelist moves;
    movegen::legalmoves(moves, board);

    total_nodes += moves.size();
    Movelist new_best_move_list;

    if(best_moves_stored.size()) {
        Move pass_move = best_moves_stored[0];
        auto it = std::find(moves.begin(), moves.end(), best_moves_stored[0]);
        if(it != moves.end()){
            orderMoves(board, moves, pass_move);
            if(best_moves_stored.size()>1)
                for( int i=1; i<best_moves_stored.size(); i++)
                    new_best_move_list.add(best_moves_stored[i]);
        }
        else{
            orderMoves2(board, moves);
        }
    }
    else {
        orderMoves2(board, moves);
    }
    

    if(depth < stop){
        to_return taking_new;
        taking_new.val = evaluate(board, depth);
        taking_new.best_string = " ";
        return taking_new;
    }

    if (moves.empty()){
        if (board.inCheck()) {
            int a = (board.sideToMove() == Color::WHITE) ? -(depth+1)*KING_VALUE : (depth+1)*KING_VALUE;
            to_return taking_new;
            taking_new.val = a;
            taking_new.best_string = " ";
            if(bot*a > 0)stop = depth;
            return taking_new;
        } else {
            to_return taking_new;
            taking_new.val = 0;
            taking_new.best_string = " ";
            return taking_new;
        }
    }

    if(depth == depth_quie){
        to_return taking_new;
        taking_new.val = Quiesce(-KING_VALUE, KING_VALUE, board, depth_quie, new_best_move_list, start, time_remain);
        // taking_new.val = evaluate(board, depth);
        taking_new.best_string = " ";
        return taking_new;
    }

    if (max_player) {
        int utility = -1000*KING_VALUE;
        to_return temp;
        temp.best_string = "";
        temp.val = 0;
        for (Move action : moves) {
            board.makeMove(action);
            to_return taking_new = alpha_beta_pruning(board, alpha, beta, depth-1, stop, bot, new_best_move_list, start, time_remain);
            board.unmakeMove(action);
            
            if(taking_new.val > utility){
                temp.best_string = taking_new.best_string + uci::moveToSan(board, action) + " ";
                utility = taking_new.val;
                temp.val = utility;
            }
            alpha = max(alpha, utility);
            if(alpha >= beta) break;
        }
        return temp;
    }
    else {
        int utility = 1000*KING_VALUE;
        to_return temp;
        temp.best_string = "";
        temp.val = 0;
        for (Move action : moves) {
            board.makeMove(action);
            to_return taking_new = alpha_beta_pruning(board, alpha, beta, depth-1, stop, bot, new_best_move_list, start, time_remain);
            board.unmakeMove(action);
            
            if(taking_new.val < utility){
                temp.best_string = taking_new.best_string + uci::moveToSan(board, action) + " ";
                utility = taking_new.val;
                temp.val = utility;
            }
            beta = min(beta, utility);
            if(alpha >= beta) break;
        }
        return temp;
    }
}

to_return alpha_beta_pruning_with_stop(Board &board, int max_player, int alpha, int beta, int depth, int time_remained){

    auto start = chrono::steady_clock::now();


    int stop = depth_quie;
    Movelist legelmoves;
    movegen::legalmoves(legelmoves, board);
    orderMoves2(board, legelmoves);

    Movelist best_moves_stored;
    best_moves_stored.add(legelmoves[0]);

    to_return val_and_string;
    float time_remain = (time_remained/20 > 10) ? time_remained/20 : time_remained/10;

    for (int i = depth_quie; i <= n; i++){

        auto now = chrono::steady_clock::now();
        auto elapsed = chrono::duration_cast<chrono::milliseconds>(now - start).count(); // int or long long

        val_and_string = alpha_beta_pruning(board, alpha, beta, i, stop, max_player, best_moves_stored, start, time_remain);

        if(elapsed >= time_remain){
            to_return val_and_string;
            val_and_string.best_string = uci::moveToSan(board, best_moves_stored[0]);
            val_and_string.val = -1;
            return val_and_string;
        }

        istringstream move_list(val_and_string.best_string);
        string move;
        best_moves_stored.clear();
        vector<string> listing_in_reverse;
        while(move_list >> move){
            listing_in_reverse.push_back(move);
        }
        int size = listing_in_reverse.size();
        for(int j=1; j<=size; j++){
            Move best_move = uci::parseSan(board, listing_in_reverse[size-j]);
            board.makeMove(best_move);
            best_moves_stored.add(best_move);
        }
        for(int j=1; j<=size; j++){
            board.unmakeMove(best_moves_stored[size-j]);
        }
    }

    return val_and_string;
}

string best_move(Board &board, int time_remained = 2000000000){
    int a = (board.sideToMove() == Color::WHITE) ? 1 : 0;
    int alpha = -KING_VALUE, beta = KING_VALUE;
    to_return move_and_val = alpha_beta_pruning_with_stop(board, a, alpha, beta, n, time_remained);
    // cout << move_and_val.best_string <<" "<< move_and_val.val << endl;
    istringstream move_list(move_and_val.best_string);
    string move;
    while(move_list >> move);
    Move move_object = uci::parseSan(board, move);
    move = uci::moveToUci(move_object);
    return move;
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
                string best_mov = best_move(board, time_remained);
                // cout << nodes << "  " << total_nodes << endl;
                cout << "bestmove " << best_mov << endl; 
            }
            else {
                string best_mov = best_move(board);
                // cout << nodes << "  " << total_nodes << endl;
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
    // string fen = "rnbq1b1r/pppp1k1p/6p1/7Q/4n3/4P3/PPPP1PPP/RNB1KB1R w KQ - 0 6";
    // Board board(fen);
    // cout << evaluate(board, 5) << " " << countMaterial2(board) <<  endl;

    Board board;
    uci_loop(board);
    return 0;
}