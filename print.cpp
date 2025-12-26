#include "chess.hpp"
using namespace chess;
using namespace std;

int main(){
    Square a = Square::SQ_A5;
    int x = a.index();
    cout<<a<<" "<<x<<endl;
}
