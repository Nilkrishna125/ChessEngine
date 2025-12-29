import torch
import torch.nn as nn

INPUT_SIZE = 768
HL_SIZE = 1024

class NNUE(nn.Module):
    def __init__(self):
        super().__init__()

        self.feature_layer = nn.Linear(INPUT_SIZE, HL_SIZE)
        self.output_layer = nn.Linear(2*HL_SIZE, 1)

    def forward(self, stm, nstm):
        t1 = self.feature_layer(stm)
        t2 = self.feature_layer(nstm)

        t1 = torch.clamp(t1, 0.0, 1.0)
        t2 = torch.clamp(t2, 0.0, 1.0)

        combined = torch.cat([t1, t2], dim=1)

        output = self.output_layer(combined)
        
        return output
