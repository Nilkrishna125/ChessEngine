// #include <iostream> 
// #include <vector>

// using namespace std;
// using result = std::pair<long long, vector<string>>;


// int main (){
    
//     result ok;
//     ok.first = 45;
//     ok.second.push_back("sdfgsd");
//     cout<<ok.second[0]<<endl;

//     vector<int> a;
//     a.push_back(345);
//     a.push_back(346);
//     cout<<*(a.end()-1)<<endl;
    
//     result b = {5, {"dfgh", "fsdg"}};
//     cout<<b.second[1]<<endl;
//     vector<string> c = {"sfdgdf"};
//     c = b.second;
//     cout<<c[0]<<endl;


// }

#include "include/chess.hpp"
#include <iostream>
using namespace std;

int calculateIndex(chess::Color perspective, chess::Square square, chess::Piece piece)
{
    int sq_index = square.index();
    int piece_color = piece.color();
    int piece_type = piece.type();
    if (perspective == chess::Color::BLACK)
    {
        piece_color   = 1 - piece_color;          // Flip piece_color
        sq_index = sq_index ^ 0b111000; // Vertically flip the square
    }
    
    return piece_color * 64 * 6 + piece_type * 64 + sq_index;
}

int main(){
    string fen = "r1b2rk1/ppp2pbp/3q1np1/n3p1B1/2B5/1Q3N2/PP1N1PPP/3R1RK1 w - - 4 14";
    chess::Board board;
    board.setFen(fen);

    for(int i=0; i<64; i++){
        chess::Square sq = i;
        chess::Piece pie = board.at(sq);
        chess::Color pers = chess::Color::WHITE;
        if(pie != chess::Piece::NONE){
            cout<<calculateIndex(pers, sq, pie)<<", ";
        }
    }
    cout<<endl;
    return 0;
}