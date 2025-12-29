import chess

def index_function(fen):
    board = chess.Board(fen)
    indices = []

    for square in chess.SQUARES:
        piece = board.piece_at(square)
        if piece:
            # P=0, N=1, B=2, R=3, Q=4, K=5
            piece_type = piece.piece_type - 1 
            
            # White=0, Black=1
            color = 0 if piece.color == chess.WHITE else 1
            
            # The Formula: 64 * (Piece_Type + 6 * Color) + Square_Index
            # Example: White Pawn (0 + 0) on A1 (0) = 0
            # Example: Black Pawn (0 + 6) on A1 (0) = 384
            idx = 64 * (piece_type + 6 * color) + square
            
            indices.append(idx)
            
    return indices


def files_parse():

    try:
        with open("csv_files/small_csv.csv", "r") as f:
            s = f.readline()
            s = f.readline()
            while(s):
                l = s.strip().split(',')
                fen = l[0];
                indices = index_function(fen)
                print(indices)
                s = f.readline()
                

    except FileNotFoundError:
        print("File not found check the path")

if __name__ == "__main__":
    files_parse()
