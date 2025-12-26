#include "chess.hpp"
using namespace chess;

#define HL_SIZE 1024
#define INPUT_LAYER 768

#define SCALE 400
#define QA 255
#define QB 64


class Network{
    public:
        int16_t accumulator_weights[INPUT_LAYER][HL_SIZE];
        int16_t accumulator_biases[HL_SIZE];
        int16_t output_weights[HL_SIZE*2];
        int16_t output_bias; 

        bool input_layer[INPUT_LAYER]; //input layer

        void fill_input_layer(Board Board);
};

struct Accumulator {
    int16_t values[HL_SIZE];
};

void Network::fill_input_layer(Board Board){
    for(int i=0; i<INPUT_LAYER; i++)input_layer[i] = 0;

    for(int i=0; i<64; i++){
        Square sq = i;
        Piece piece = Board.at(sq);
        
    }
}

int calculateIndex(Color perspective, Square square, Piece piece)
{
    int sq_index = square.index();
    int piece_color = piece.color();
    int piece_type = piece.type();
    if (perspective == Color::BLACK)
    {
        piece_color   = 1 - piece_color;          // Flip piece_color
        sq_index = sq_index ^ 0b111000; // Vertically flip the square
    }
    
    return piece_color * 64 * 6 + piece_type * 64 + sq_index;
}

struct AccumulatorPair {
    Accumulator white;
    Accumulator black;
};


void accumulatorAdd(const struct Network* const network, struct Accumulator* accumulator, size_t index)
{
    for (int i = 0; i < HL_SIZE; i ++)
        accumulator->values[i] += network->accumulator_weights[index][i];
}

void accumulatorSub(const struct Network* const network, struct Accumulator* accumulator, size_t index)
{
    for (int i = 0; i < HL_SIZE; i ++)
        accumulator->values[i] -= network->accumulator_weights[index][i];
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

// When forwarding the accumulator values, the network does not consider the color of the perspectives.
// Rather, we are more interested in whether the accumulator is from the perspective of the side-to-move.
int32_t forward(const struct Network*     const network,
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

void update(Network* network,
            Accumulator* stm_accumulator,
            Accumulator* nstm_accumulator,
            const Move& move,
            const Board& board)
{
    Color side = board.sideToMove();
    Color enemy = 1 - side;
    Square from = move.from();
    Square to = move.to();
    Piece piece = board.at(from);
    Piece captured = board.at(to);

    if( captured != Piece::NONE){ // "to" place calculation
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

