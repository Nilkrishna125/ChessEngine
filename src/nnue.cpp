#include "nnue.hpp"
#include <fstream>



#define SCALE 400
#define QA 255
#define QB 64

Network network;

void Network::load_from_file(const std::string& filename){
    std::ifstream file(filename, std::ios::binary);
    if(!file.is_open()){
        std::cerr << "Error: Cannot open NNUE file: " << filename << std::endl;
    }

    file.read((char*)accumulator_weights, sizeof(accumulator_weights));
    file.read((char*)accumulator_biases, sizeof(accumulator_biases));
    
    // Read Output Layer
    file.read((char*)output_weights, sizeof(output_weights));
    file.read((char*)&output_bias, sizeof(output_bias));

    file.close();
}

void remove_feature(Network* network,
                    Accumulator* accumulator,
                    int index)
{
    for (int i=0; i<HL_SIZE; i++){
        accumulator->values[i] -= network->accumulator_weights[index][i];
    }
}

void add_feature(Network* network,
                Accumulator* accumulator,
                int index)
{
    for (int i=0; i<HL_SIZE; i++){
        accumulator->values[i] += network->accumulator_weights[index][i];
    }
}

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

void Network::initialise_acc(chess::Board Board, AccumulatorPair* acc_p){

    for(int i=0; i<HL_SIZE; i++){
        acc_p->white.values[i] = this->accumulator_biases[i];
        acc_p->black.values[i] = this->accumulator_biases[i];
    }

    for(int i=0; i<64; i++){
        chess::Square sq = i;
        chess::Piece piece = Board.at(sq);

        if (piece != chess::Piece::NONE){
            chess::Color white = chess::Color::WHITE;
            int idx_w = calculateIndex(white, sq, piece);
            add_feature(this, &acc_p->white, idx_w);

            chess::Color black = chess::Color::BLACK;
            int idx_b = calculateIndex(black, sq, piece);
            add_feature(this, &acc_p->black, idx_b);
        }
    }
}

int16_t CReLU(int16_t value, int16_t min, int16_t max)
{
    if (value <= min)
        return min;

    if (value >= max)
        return max;

    return value;
}

// Example: CReLU activation
int32_t activation(int16_t value)
{
    return CReLU(value, 0, QA);
}

int32_t eval_nnue(const struct Network*     const network,
                const struct Accumulator* const stm_accumulator,
                const struct Accumulator* const nstm_accumulator)
{
    int32_t eval = 0;

    // Dot product to the weights
    for (int i = 0; i < HL_SIZE; i++)
    {
        // BEWARE of integer overflows here.
        eval += activation( stm_accumulator->values[i]) * network->output_weights[i];
        eval += activation(nstm_accumulator->values[i]) * network->output_weights[i + HL_SIZE];
    }

    // Uncomment the following dequantization step when using SCReLU
    // eval /= QA;
    eval += network->output_bias;

    eval *= SCALE;
    eval /= QA * QB;

    return eval;
}

void update(Network* network,
            Accumulator* stm_accumulator,
            Accumulator* nstm_accumulator,
            const chess::Move& move,
            const chess::Board& board)
{
    chess::Color side = board.sideToMove();
    chess::Color enemy = 1 - side;
    chess::Square from = move.from();
    chess::Square to = move.to();
    chess::Piece piece = board.at(from);
    chess::Piece captured = board.at(to);

    if( captured != chess::Piece::NONE){ // "to" place calculation
        int idx_sm = calculateIndex(side, to, captured);
        remove_feature(network, stm_accumulator, idx_sm);

        int idx_nsm = calculateIndex(enemy, to, captured);
        remove_feature(network, nstm_accumulator, idx_nsm);
    }

    // "from" place calculation
    int idx_sm_i = calculateIndex(side, from, piece);
    int idx_nsm_i = calculateIndex(enemy, from, piece);
    remove_feature(network, stm_accumulator, idx_sm_i);
    remove_feature(network, nstm_accumulator, idx_nsm_i);

    int idx_sm_f = calculateIndex(side, to, piece);
    int idx_nsm_f = calculateIndex(enemy, to, piece);
    add_feature(network, stm_accumulator, idx_sm_f);
    add_feature(network, nstm_accumulator, idx_nsm_f);

}



