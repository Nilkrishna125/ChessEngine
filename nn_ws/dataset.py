import os
import torch
import chess
import pandas as pd
import numpy as np
from torch.utils.data import Dataset

def index_function(board, perspective):
    indices = []

    for square in chess.SQUARES:
        piece = board.piece_at(square)
        if piece:
            # P=0, N=1, B=2, R=3, Q=4, K=5
            piece_type = piece.piece_type - 1 
            
            # White=0, Black=1
            color = 0 if piece.color == chess.WHITE else 1

            if perspective == chess.BLACK:
                color = 1 - color
                square = square ^ 56
            
            # The Formula: 64 * (Piece_Type + 6 * Color) + Square_Index
            # Example: White Pawn (0 + 0) on A1 (0) = 0
            # Example: Black Pawn (0 + 6) on A1 (0) = 384
            idx = 64 * (piece_type + 6 * color) + square
            
            indices.append(idx)
            
    return indices

class FenValDataset(Dataset):
    def __init__(self, csv_data):
        self.csv_data = pd.read_csv(csv_data)

    def __len__(self):
        return len(self.csv_data)
    
    def __getitem__(self, idx):
        fen_val = self.csv_data.iloc[idx]
        fen = str(fen_val[0])
        val = float(fen_val[1])
        
        board = chess.Board(fen)

        indices_white = index_function(board, chess.WHITE)
        indices_black = index_function(board, chess.BLACK)

        input_vector_w = torch.zeros(768, dtype=torch.float32)
        input_vector_b = torch.zeros(768, dtype=torch.float32)

        input_vector_w[indices_white] = 1
        input_vector_b[indices_black] = 1

        if board.turn == chess.WHITE:
            stm_input = input_vector_w
            nstm_input = input_vector_b
            score = val

        else:
            stm_input = input_vector_b
            nstm_input = input_vector_w
            score = -val

        norm_score = 1 / (1 + np.exp(-score/400))

        return stm_input, nstm_input, torch.tensor([norm_score], dtype=torch.float32)

