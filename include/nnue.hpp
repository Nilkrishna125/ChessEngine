#pragma once 
#include "chess.hpp"

#define HL_SIZE 1024
#define INPUT_LAYER 768

struct Accumulator {
    int16_t values[HL_SIZE];
};

struct AccumulatorPair {
    Accumulator white;
    Accumulator black;
};

class Network{
    public:
        int16_t accumulator_weights[INPUT_LAYER][HL_SIZE];
        int16_t accumulator_biases[HL_SIZE];
        int16_t output_weights[HL_SIZE*2];
        int16_t output_bias; 

        void load_from_file(const std::string& filename);
        void initialise_acc(chess::Board Board, AccumulatorPair* acc_p);
};

int32_t eval_nnue(const struct Network*     const network,
                const struct Accumulator* const stm_accumulator,
                const struct Accumulator* const nstm_accumulator);

void update(Network* network,
            Accumulator* stm_accumulator,
            Accumulator* nstm_accumulator,
            const chess::Move& move,
            const chess::Board& board);

extern Network network;