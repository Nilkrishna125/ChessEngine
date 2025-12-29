import torch
import numpy as np
import struct
from model import NNUE

# --- CONFIGURATION (Must match C++ defines) ---
QA = 255  # Quantization for Input Layer
QB = 64   # Quantization for Output Layer

def export_model(model_path, output_path):
    # 1. Load the Trained Model
    model = NNUE()
    model.load_state_dict(torch.load(model_path, map_location=torch.device('cpu')))
    model.eval()

    print(f"Exporting {model_path} to {output_path}...")

    with open(output_path, "wb") as f:
        # --- LAYER 1: Feature Transformer ---
        # Python: Float32 (0.0 to 1.0)
        # C++: Int16 (0 to 255)
        # Action: Multiply by 255 and cast to Short (int16)
        
        # A. Weights (768 -> 1024)
        # Transpose might be needed depending on how C++ reads linear memory.
        # PyTorch Linear stores as [out_features, in_features]
        # Your C++ likely reads [in][out] or [out][in].
        # Standard loop order in C++ load_from_file implies simple flat write.
        # We assume [Input][Output] (768 arrays of size 1024) based on typical C++ loops.
        # But PyTorch stores as [1024][768]. So we Transpose (.t())
        layer1_weights = model.feature_layer.weight.data.t()
        
        # Scale and Clamp to int16 range
        l1_w = (layer1_weights * QA).numpy().astype(np.int16)
        f.write(l1_w.tobytes())
        
        # B. Biases (1024)
        l1_b = (model.feature_layer.bias.data * QA).numpy().astype(np.int16)
        f.write(l1_b.tobytes())

        # --- LAYER 2: Output Layer ---
        # The input to this layer is already scaled by QA (0..255)
        # We want the output scaled by QB (64).
        
        # A. Weights (2048 -> 1)
        # Shape is [1, 2048]. We flatten to [2048]
        layer2_weights = model.output_layer.weight.data.flatten()
        
        # Scale by QB
        l2_w = (layer2_weights * QB).numpy().astype(np.int16)
        f.write(l2_w.tobytes())
        
        # B. Bias (1)
        # The bias adds to the Sum(Input * Weight).
        # Input is scaled by QA. Weight is scaled by QB.
        # So the Sum is scaled by QA*QB.
        # Therefore, Bias must also be scaled by QA*QB.
        layer2_bias = model.output_layer.bias.data
        
        # Note: Bias usually needs higher precision (Int32) to avoid overflow
        l2_b = (layer2_bias * QA * QB).numpy().astype(np.int32)
        f.write(l2_b.tobytes())

    print("Done! .nnue file created.")

if __name__ == "__main__":
    # Change this to your latest epoch file
    export_model("./models/nnue_epoch_5.pth", "./models/shallow.nnue")