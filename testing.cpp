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

to_return backind(Board board, int max_player, int i, int alpha, int beta){
    Movelist moves;
    movegen::legalmoves(moves,board);
    to_return values;

   

    if (moves.empty()){
        if (board.inCheck()) {
            int a = (board.sideToMove() == chess::Color::WHITE) ? -100000 : 100000;
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
        int utility=-10000;
        to_return temp;
        for (Move action : moves) {
            board.makeMove(action);
            values = backind(board, 0, i-1, alpha, beta);
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
        int utility=10000;
        to_return temp;
        for (Move action : moves) {
            board.makeMove(action);
            values = backind(board, 1, i-1, alpha, beta);
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

int main(){
    string fen = "3n1k2/5p2/2p1bb2/1p2pN1q/1P2P3/2P3Q1/5PB1/3R2K1 w - - 1 0";
    chess::Board board(fen);
    to_return see_move;
    see_move = backind(board, 1, 3, -100000, 100000);
    cout<<see_move.best_string<<endl;
    return 0;
}


#include <iostream>
#include "chess.hpp"
#include <vector>
#include <math.h>
#include <sstream>
#include <algorithm>
using namespace std;
using namespace chess;

string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
Board board(fen);

struct to_return{
    int val;
    string best_string;
};

int stop = 0;

// Material values in centipawns
const int PAWN_VALUE = 100;
const int KNIGHT_VALUE = 320;
const int BISHOP_VALUE = 330;
const int ROOK_VALUE = 500;
const int QUEEN_VALUE = 900;
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

// Evaluate piece mobility
int evaluateMobility(const Board& board) {
    int score = 0;
    Movelist moves;
    
    // Count legal moves for current side
    movegen::legalmoves(moves, board);
    int mobility = moves.size();
    
    // Mobility bonus/penalty
    if (board.sideToMove() == Color(Color::underlying::WHITE)) {
        score += mobility * 4;
    } else {
        score -= mobility * 4;
    }
    
    return score;
}

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

Bitboard getBetweenBitboard(Square sq1, Square sq2) {
    Bitboard between = Bitboard(0);
    
    int file1 = static_cast<int>(sq1.file());
    int rank1 = static_cast<int>(sq1.rank());
    int file2 = static_cast<int>(sq2.file());
    int rank2 = static_cast<int>(sq2.rank());
    
    // Check if squares are on same line (rank, file, or diagonal)
    int fileDir = (file2 > file1) ? 1 : (file2 < file1) ? -1 : 0;
    int rankDir = (rank2 > rank1) ? 1 : (rank2 < rank1) ? -1 : 0;
    
    // If not on same line, return empty bitboard
    if (fileDir == 0 && rankDir == 0) return between;
    if (fileDir != 0 && rankDir != 0 && abs(file2 - file1) != abs(rank2 - rank1)) return between;
    
    // Generate squares between
    int currentFile = file1 + fileDir;
    int currentRank = rank1 + rankDir;
    
    while (currentFile != file2 || currentRank != rank2) {
        Square sq = Square(static_cast<File>(currentFile), static_cast<Rank>(currentRank));
        between |= Bitboard::fromSquare(sq);
        
        currentFile += fileDir;
        currentRank += rankDir;
    }
    
    return between;
}

// Evaluate pinned pieces
int evaluatePins(const Board& board) {
    int score = 0;
    
    Square whiteKing = board.kingSq(Color::WHITE);
    Square blackKing = board.kingSq(Color::BLACK);
    
    // Check for pieces pinned to the white king
    Bitboard whitePieces = board.pieces(PieceType::KNIGHT, Color::WHITE) |
                           board.pieces(PieceType::ROOK, Color::WHITE) | 
                           board.pieces(PieceType::QUEEN, Color::WHITE) |
                           board.pieces(PieceType::KING, Color::WHITE) |
                           board.pieces(PieceType::PAWN, Color::WHITE) |
                           board.pieces(PieceType::BISHOP, Color::WHITE);

    Bitboard blackPieces = board.pieces(PieceType::KNIGHT, Color::BLACK) |
                           board.pieces(PieceType::ROOK, Color::BLACK) | 
                           board.pieces(PieceType::QUEEN, Color::BLACK) |
                           board.pieces(PieceType::KING, Color::BLACK) |
                           board.pieces(PieceType::PAWN, Color::BLACK) |
                           board.pieces(PieceType::BISHOP, Color::BLACK);
    
    // Diagonal pins (bishops, queens)
    Bitboard diagonalAttackers = board.pieces(PieceType::BISHOP, Color::BLACK) | 
                                 board.pieces(PieceType::QUEEN, Color::BLACK);
    
    while (diagonalAttackers) {
        Square attackerSq = getLSB(diagonalAttackers);
        diagonalAttackers = popLSB(diagonalAttackers);
        
        Bitboard ray = attacks::bishop(attackerSq, board.occ());
        if (ray & squareToBitboard(whiteKing)) {
            // Check if there's exactly one white piece between attacker and king
            Bitboard between = getBetweenBitboard(attackerSq, whiteKing) & whitePieces;
            if (popcount(between) == 1) {
                Square pinnedSq = getLSB(between);
                Piece pinnedPiece = board.at(pinnedSq);
                score -= getPieceValue(pinnedPiece.type()) / 4; // Penalty for pinned piece
            }
        }
    }
    
    // Horizontal/vertical pins (rooks, queens)
    Bitboard straightAttackers = board.pieces(PieceType::ROOK, Color::BLACK) | 
                                board.pieces(PieceType::QUEEN, Color::BLACK);
    
    while (straightAttackers) {
        Square attackerSq = getLSB(straightAttackers);
        straightAttackers = popLSB(straightAttackers);
        
        Bitboard ray = attacks::rook(attackerSq, board.occ());
        if (ray & squareToBitboard(whiteKing)) {
            Bitboard between = getBetweenBitboard(attackerSq, whiteKing) & whitePieces;
            if (popcount(between) == 1) {
                Square pinnedSq = getLSB(between);
                Piece pinnedPiece = board.at(pinnedSq);
                score -= getPieceValue(pinnedPiece.type()) / 4;
            }
        }
    }
    
    // Same logic for black king (reverse the score)
    diagonalAttackers = board.pieces(PieceType::BISHOP, Color::WHITE) | 
                       board.pieces(PieceType::QUEEN, Color::WHITE);
    
    while (diagonalAttackers) {
        Square attackerSq = getLSB(diagonalAttackers);
        diagonalAttackers = popLSB(diagonalAttackers);
        
        Bitboard ray = attacks::bishop(attackerSq, board.occ());
        if (ray & squareToBitboard(blackKing)) {
            Bitboard between = getBetweenBitboard(attackerSq, blackKing) & blackPieces;
            if (popcount(between) == 1) {
                Square pinnedSq = getLSB(between);
                Piece pinnedPiece = board.at(pinnedSq);
                score += getPieceValue(pinnedPiece.type()) / 4;
            }
        }
    }
    
    straightAttackers = board.pieces(PieceType::ROOK, Color::WHITE) | 
                       board.pieces(PieceType::QUEEN, Color::WHITE);
    
    while (straightAttackers) {
        Square attackerSq = getLSB(straightAttackers);
        straightAttackers = popLSB(straightAttackers);
        
        Bitboard ray = attacks::rook(attackerSq, board.occ());
        if (ray & squareToBitboard(blackKing)) {
            Bitboard between = getBetweenBitboard(attackerSq, blackKing) & blackPieces;
            if (popcount(between) == 1) {
                Square pinnedSq = getLSB(between);
                Piece pinnedPiece = board.at(pinnedSq);
                score += getPieceValue(pinnedPiece.type()) / 4;
            }
        }
    }
    
    return score;
}

// Evaluate fork opportunities
int evaluateForks(const Board& board) {
    int score = 0;
    
    // Knight forks
    Bitboard whiteKnights = board.pieces(PieceType::KNIGHT, Color::WHITE);
    Bitboard blackKnights = board.pieces(PieceType::KNIGHT, Color::BLACK);

    Bitboard blackPieces = board.pieces(PieceType::KNIGHT, Color::BLACK) |
                           board.pieces(PieceType::ROOK, Color::BLACK) | 
                           board.pieces(PieceType::QUEEN, Color::BLACK) |
                           board.pieces(PieceType::KING, Color::BLACK) |
                           board.pieces(PieceType::PAWN, Color::BLACK) |
                           board.pieces(PieceType::BISHOP, Color::BLACK);

    Bitboard whitePieces = board.pieces(PieceType::KNIGHT, Color::WHITE) |
                           board.pieces(PieceType::ROOK, Color::WHITE) | 
                           board.pieces(PieceType::QUEEN, Color::WHITE) |
                           board.pieces(PieceType::KING, Color::WHITE) |
                           board.pieces(PieceType::PAWN, Color::WHITE) |
                           board.pieces(PieceType::BISHOP, Color::WHITE);
    
    // Check white knight forks
    while (whiteKnights) {
        Square knightSq = getLSB(whiteKnights);
        whiteKnights = popLSB(whiteKnights);
        
        Bitboard knightAttacks = attacks::knight(knightSq);
        Bitboard targets = knightAttacks & blackPieces;
        
        if (popcount(targets) >= 2) {
            // Potential fork - check if it includes valuable pieces
            if (targets & board.pieces(PieceType::KING, Color::BLACK)) {
                score += 200; // Fork involving king
            } else if (targets & board.pieces(PieceType::QUEEN, Color::BLACK)) {
                score += 150; // Fork involving queen
            } else if (targets & (board.pieces(PieceType::ROOK, Color::BLACK) | 
                                 board.pieces(PieceType::BISHOP, Color::BLACK))) {
                score += 100; // Fork involving major pieces
            } else {
                score += 50; // Minor fork
            }
        }
    }
    
    // Check black knight forks (reverse score)
    while (blackKnights) {
        Square knightSq = getLSB(blackKnights);
        blackKnights = popLSB(blackKnights);
        
        Bitboard knightAttacks = attacks::knight(knightSq);
        Bitboard targets = knightAttacks & whitePieces;
        
        if (popcount(targets) >= 2) {
            if (targets & board.pieces(PieceType::KING, Color::WHITE)) {
                score -= 200;
            } else if (targets & board.pieces(PieceType::QUEEN, Color::WHITE)) {
                score -= 150;
            } else if (targets & (board.pieces(PieceType::ROOK, Color::WHITE) | 
                                 board.pieces(PieceType::BISHOP, Color::WHITE))) {
                score -= 100;
            } else {
                score -= 50;
            }
        }
    }
    
    return score;
}

// Evaluate discovered attacks
int evaluateDiscoveredAttacks(const Board& board) {
    int score = 0;
    
    // This is complex - simplified version
    // Look for pieces that when moved, reveal attacks from pieces behind them
    
    Bitboard allPieces = board.occ();
    
    // Check for potential discovered attacks by bishops/queens
    Bitboard whiteBishopsQueens = board.pieces(PieceType::BISHOP, Color::WHITE) | 
                                  board.pieces(PieceType::QUEEN, Color::WHITE);
    
    while (whiteBishopsQueens) {
        Square attackerSq = getLSB(whiteBishopsQueens);
        whiteBishopsQueens = popLSB(whiteBishopsQueens);
        
        Bitboard diagonalRay = attacks::bishop(attackerSq, Bitboard(0));
        Square blackKing = board.kingSq(Color::BLACK);
        
        if (diagonalRay & squareToBitboard(blackKing)) {
            Bitboard between = getBetweenBitboard(attackerSq, blackKing) & allPieces;
            if (popcount(between) == 1) {
                Square blockingSq = getLSB(between);
                Piece blockingPiece = board.at(blockingSq);
                if (blockingPiece.color() == Color::WHITE) {
                    score += 75; // Potential discovered attack
                }
            }
        }
    }
    
    // Same for rooks/queens on ranks/files
    Bitboard whiteRooksQueens = board.pieces(PieceType::ROOK, Color::WHITE) | 
                               board.pieces(PieceType::QUEEN, Color::WHITE);
    
    while (whiteRooksQueens) {
        Square attackerSq = getLSB(whiteRooksQueens);
        whiteRooksQueens = popLSB(whiteRooksQueens);
        
        Bitboard straightRay = attacks::rook(attackerSq, Bitboard(0));
        Square blackKing = board.kingSq(Color::BLACK);
        
        if (straightRay & squareToBitboard(blackKing)) {
            Bitboard between = getBetweenBitboard(attackerSq, blackKing) & allPieces;
            if (popcount(between) == 1) {
                Square blockingSq = getLSB(between);
                Piece blockingPiece = board.at(blockingSq);
                if (blockingPiece.color() == Color::WHITE) {
                    score += 75;
                }
            }
        }
    }
    
    // Reverse for black pieces
    Bitboard blackBishopsQueens = board.pieces(PieceType::BISHOP, Color::BLACK) | 
                                  board.pieces(PieceType::QUEEN, Color::BLACK);
    
    while (blackBishopsQueens) {
        Square attackerSq = getLSB(blackBishopsQueens);
        blackBishopsQueens = popLSB(blackBishopsQueens);
        
        Bitboard diagonalRay = attacks::bishop(attackerSq, Bitboard(0));
        Square whiteKing = board.kingSq(Color::WHITE);
        
        if (diagonalRay & squareToBitboard(whiteKing)) {
            Bitboard between = getBetweenBitboard(attackerSq, whiteKing) & allPieces;
            if (popcount(between) == 1) {
                Square blockingSq = getLSB(between);
                Piece blockingPiece = board.at(blockingSq);
                if (blockingPiece.color() == Color::BLACK) {
                    score -= 75;
                }
            }
        }
    }
    
    Bitboard blackRooksQueens = board.pieces(PieceType::ROOK, Color::BLACK) | 
                               board.pieces(PieceType::QUEEN, Color::BLACK);
    
    while (blackRooksQueens) {
        Square attackerSq = getLSB(blackRooksQueens);
        blackRooksQueens = popLSB(blackRooksQueens);
        
        Bitboard straightRay = attacks::rook(attackerSq, Bitboard(0));
        Square whiteKing = board.kingSq(Color::WHITE);
        
        if (straightRay & squareToBitboard(whiteKing)) {
            Bitboard between = getBetweenBitboard(attackerSq, whiteKing) & allPieces;
            if (popcount(between) == 1) {
                Square blockingSq = getLSB(between);
                Piece blockingPiece = board.at(blockingSq);
                if (blockingPiece.color() == Color::BLACK) {
                    score -= 75;
                }
            }
        }
    }
    
    return score;
}

// Evaluate tactical patterns (pins, forks, skewers)
int evaluateTactics(const Board& board) {
    int score = 0;
    
    // Evaluate pins
    score += evaluatePins(board);
    
    // Evaluate forks (knight forks, pawn forks, etc.)
    score += evaluateForks(board);
    
    // Evaluate discovered attacks
    score += evaluateDiscoveredAttacks(board);
    
    return score;
}

// Evaluate hanging (undefended) pieces
int evaluateHangingPieces(const Board& board) {
    int score = 0;
    
    for (int sq = 0; sq < 64; sq++) {
        Square square = static_cast<Square>(sq);
        Piece piece = board.at(square);
        
        if (piece != Piece::NONE) {
            Color color = piece.color();
            PieceType type = piece.type();
            
            if (type == PieceType::KING) continue; // Skip kings
            
            bool isAttacked = board.isAttacked(square, ~color);
            bool isDefended = board.isAttacked(square, color);
            
            if (isAttacked && !isDefended) {
                // Hanging piece
                int pieceValue = getPieceValue(type);
                if (color == Color::WHITE) {
                    score -= pieceValue / 2; // Penalty for hanging white piece
                } else {
                    score += pieceValue / 2; // Bonus for hanging black piece
                }
            } else if (isAttacked && isDefended) {
                // Attacked but defended - minor penalty/bonus
                int pieceValue = getPieceValue(type);
                if (color == Color::WHITE) {
                    score -= pieceValue / 8;
                } else {
                    score += pieceValue / 8;
                }
            }
        }
    }
    
    return score;
}

// Evaluate available captures
int evaluateCaptures(const Board& board) {
    int score = 0;
    
    Movelist captures;
    movegen::legalmoves<movegen::MoveGenType::CAPTURE>(captures, board);
    
    for (Move capture : captures) {
        Piece captured = board.at(capture.to());
        Piece attacker = board.at(capture.from());
        
        if (captured != Piece::NONE) {
            int captureValue = getPieceValue(captured.type()) - getPieceValue(attacker.type());
            
            // Bonus for good captures (capturing more valuable piece with less valuable piece)
            if (captureValue > 0) {
                if (board.sideToMove() == Color::WHITE) {
                    score += captureValue / 4;
                } else {
                    score -= captureValue / 4;
                }
            }
        }
    }
    
    return score;
}

// Evaluate checks and threats
int evaluateChecksAndThreats(Board& board) {
    int score = 0;
    
    // Bonus for giving check
    if (board.inCheck()) {
        if (board.sideToMove() == Color::BLACK) {
            score += 50; // White is giving check
        } else {
            score -= 50; // Black is giving check  
        }
    }
    
    // Evaluate check threats (moves that would give check)
    Movelist moves;
    movegen::legalmoves(moves, board);
    
    int checkThreats = 0;
    for (Move move : moves) {
        board.makeMove(move);
        if (board.inCheck()) {
            checkThreats++;
        }
        board.unmakeMove(move);
    }
    
    if (board.sideToMove() == Color::WHITE) {
        score += checkThreats * 10;
    } else {
        score -= checkThreats * 10;
    }
    
    return score;
}

// Evaluate piece activity and coordination
int evaluatePieceActivity(const Board& board) {
    int score = 0;
    
    // Rook activity
    Bitboard whiteRooks = board.pieces(PieceType::ROOK, Color::WHITE);
    Bitboard blackRooks = board.pieces(PieceType::ROOK, Color::BLACK);
    
    // Rooks on open files
    for (int file = 0; file < 8; file++) {
        Bitboard fileBB = getFileBitboard(static_cast<File>(file));
        Bitboard pawnsOnFile = (board.pieces(PieceType::PAWN, Color::WHITE) | 
                               board.pieces(PieceType::PAWN, Color::BLACK)) & fileBB;
        
        if (!pawnsOnFile) { // Open file
            if (whiteRooks & fileBB) score += 25;
            if (blackRooks & fileBB) score -= 25;
        } else if (!(board.pieces(PieceType::PAWN, Color::WHITE) & fileBB)) { // Semi-open for white
            if (whiteRooks & fileBB) score += 15;
        } else if (!(board.pieces(PieceType::PAWN, Color::BLACK) & fileBB)) { // Semi-open for black
            if (blackRooks & fileBB) score -= 15;
        }
    }
    
    // Bishop pair bonus
    if (popcount(board.pieces(PieceType::BISHOP, Color::WHITE)) >= 2) {
        score += 30;
    }
    if (popcount(board.pieces(PieceType::BISHOP, Color::BLACK)) >= 2) {
        score -= 30;
    }
    
    return score;
}

// Evaluate control of key squares
int evaluateSquareControl(const Board& board) {
    int score = 0;
    
    // Central squares (e4, e5, d4, d5)
    Square centralSquares[] = {Square::SQ_E4, Square::SQ_E5, Square::SQ_D4, Square::SQ_D5};
    
    for (Square sq : centralSquares) {
        bool whiteControls = board.isAttacked(sq, Color::WHITE);
        bool blackControls = board.isAttacked(sq, Color::BLACK);
        
        if (whiteControls && !blackControls) {
            score += 10;
        } else if (blackControls && !whiteControls) {
            score -= 10;
        }
    }
    
    return score;
}

// Main evaluation function
// Enhanced evaluation function with tactical considerations
int evaluate(Board& board) {
    // Check for game over conditions first
    auto gameResult = board.isGameOver();
    if (gameResult.first != GameResultReason::NONE) {
        if (gameResult.second == GameResult::WIN) {
            return board.sideToMove() == Color::WHITE ? KING_VALUE : -KING_VALUE;
        } else if (gameResult.second == GameResult::LOSE) {
            return board.sideToMove() == Color::WHITE ? -KING_VALUE : KING_VALUE;
        } else {
            return 0; // Draw
        }
    }
    
    int score = 0;
    int totalMaterial = countMaterial(board);
    bool endgame = totalMaterial < 1300;
    
    // 1. Basic material and piece-square evaluation (existing code)
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
    
    // 2. Tactical evaluation
    score += evaluateTactics(board);
    
    // 3. Hanging pieces
    score += evaluateHangingPieces(board);
    
    // 4. Captures available
    score += evaluateCaptures(board);
    
    // 5. Checks and threats
    score += evaluateChecksAndThreats(board);
    
    // 6. Piece activity and coordination
    score += evaluatePieceActivity(board);
    
    // 7. Control of key squares
    score += evaluateSquareControl(board);
    
    // 8. Existing positional factors
    score += evaluatePawns(board);
    score += evaluateMobility(board);
    score += evaluateKingSafety(board);
    
    if (endgame) {
        score += evaluateEndgame(board);
    }
    
    // Return score from current player's perspective
    return board.sideToMove() == Color::WHITE ? score : -score;
}

int quiescence_search(Board board, int alpha, int beta) {
    int stand_pat = evaluate(board);
    
    if (stand_pat >= beta) return beta;
    if (alpha < stand_pat) alpha = stand_pat;
    
    Movelist tactical_moves;
    
    // Get captures
    movegen::legalmoves<movegen::MoveGenType::CAPTURE>(tactical_moves, board);
    
    // Also get checks (but only if not already in check to avoid infinite search)
    if (!board.inCheck()) {
        Movelist checks;
        movegen::legalmoves(checks, board);
        for (Move move : checks) {
            board.makeMove(move);
            if (board.inCheck()) {
                tactical_moves.add(move);
            }
            board.unmakeMove(move);
        }
    }
    
    // Handle empty movelist
    if (tactical_moves.empty()) {
        return stand_pat;
    }
    
    for (Move move : tactical_moves) {
        board.makeMove(move);
        int score = -quiescence_search(board, -beta, -alpha);
        board.unmakeMove(move);
        
        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    
    return alpha;
}

to_return alpha_beta_pruning(Board board, int max_player, int alpha, int beta, int depth){
    Movelist moves;
    movegen::legalmoves(moves, board);
    to_return values;
    values.best_string = "";
    values.val = 0;

    if(depth < stop){
        int alpha_d = -KING_VALUE;
        if (max_player) alpha_d = KING_VALUE;
        values.val = quiescence_search(board, alpha_d, -alpha_d);
        values.best_string = " ";
        return values;
    }

    if (moves.empty()){
        if (board.inCheck()) {
            int a = (board.sideToMove() == chess::Color::WHITE) ? -KING_VALUE : KING_VALUE;
            values.val = a;
            values.best_string = " ";
            stop = depth;
            return values;
        } else {
            values.val = 0;
            values.best_string = " ";
            stop = depth;
            return values;
        }
    }

    if(depth == 0){
        int alpha_d = -KING_VALUE;
        if (max_player) alpha_d = KING_VALUE;
        values.val = quiescence_search(board, alpha_d, -alpha_d);
        values.best_string = " ";
        return values;
    }

    if (max_player) {
        int utility = -KING_VALUE;
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
            alpha = max(alpha, utility);
            if(alpha >= beta) break;
        }
        return temp;
    }
    else {
        int utility = KING_VALUE;
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
            if(alpha >= beta) break;
        }
        return temp;
    }
}

string best_move(){
    int a = (board.sideToMove() == Color::WHITE) ? 1 : 0;
    to_return move_and_val = alpha_beta_pruning(board, a, -KING_VALUE, KING_VALUE, 5);
    istringstream move_list(move_and_val.best_string);
    string move;
    while(move_list >> move);
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
            string best_mov = best_move();
            cout << "bestmove " << best_mov << endl;  
        } else if (line == "quit") {
            break;
        }
    }
}

int main(){
    uci_loop();
    return 0;
}