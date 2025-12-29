import struct
import random

# Architecture parameters from your nnue.hpp
INPUT_LAYER = 768
HL_SIZE = 1024

def create_dummy_nnue():
    print(f"Generating eval.nnue with HL_SIZE={HL_SIZE}...")
    
    with open("eval.nnue", "wb") as f:
        # 1. Accumulator Weights (Input -> Hidden)
        # 768 * 1024 weights (int16)
        # We use small random numbers so the engine doesn't overflow immediately
        for _ in range(INPUT_LAYER * HL_SIZE):
            f.write(struct.pack('<h', random.randint(-10, 10)))
            
        # 2. Accumulator Biases (Hidden)
        # 1024 biases (int16)
        for _ in range(HL_SIZE):
            f.write(struct.pack('<h', random.randint(-10, 10)))
            
        # 3. Output Weights (Hidden -> Output)
        # 1024 * 2 weights (int16)
        for _ in range(HL_SIZE * 2):
            f.write(struct.pack('<h', random.randint(-10, 10)))
            
        # 4. Output Bias
        # 1 value (int16)
        f.write(struct.pack('<h', random.randint(-10, 10)))

    print("Done! 'eval.nnue' created.")

if __name__ == "__main__":
    create_dummy_nnue()