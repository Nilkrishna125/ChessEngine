import torch
import chess
import numpy as np
from model import NNUE
from dataset import index_function 

# Load your model
model = NNUE()
model.load_state_dict(torch.load("nnue_epoch_5.pth", map_location='cpu'))
model.eval()

# Test Position: Start Position
fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
board = chess.Board(fen)

# Get Inputs
idx_w = index_function(board, chess.WHITE)
idx_b = index_function(board, chess.BLACK)

vec_w = torch.zeros(768); vec_w[idx_w] = 1.0
vec_b = torch.zeros(768); vec_b[idx_b] = 1.0

# Forward Pass (White to Move)
# STM = White, NSTM = Black
output = model(vec_w.unsqueeze(0), vec_b.unsqueeze(0))
prob = torch.sigmoid(output)

print(f"Python Raw Output: {output.item()}")
print(f"Python Probability: {prob.item()}")
print(f"Python CP Score (approx): {-400 * np.log(1/prob.item() - 1)}")