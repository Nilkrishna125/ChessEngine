import torch
import torch.nn as nn
import torch.optim as optim 
from torch.utils.data import DataLoader

from dataset import FenValDataset
from model import NNUE

BATCH_SIZE = 128
LEARNING_RATE = 0.001
EPOCHS = 5
DEVICE = 'cuda' if torch.cuda.is_available() else 'cpu'

print("Training on : ", DEVICE)

def train():
    dataset = FenValDataset("../csv_files/FilteredEvals.csv")
    dataloader = DataLoader(dataset, batch_size=BATCH_SIZE, shuffle=True, num_workers=0)

    model = NNUE().to(DEVICE)

    optimizer = optim.Adam(model.parameters(), lr=LEARNING_RATE)
    loss_fn = nn.MSELoss()

    for epoch in range(EPOCHS):
        total_loss = 0
        count = 0

        print(f"Started {epoch + 1}th epoch ")

        for stm, nstm, target in dataloader:
            stm = stm.to(DEVICE)
            nstm = nstm.to(DEVICE)
            target = target.to(DEVICE)

            prediction = model(stm, nstm)
            prediction_prob = torch.sigmoid(prediction)

            loss = loss_fn(prediction_prob, target)

            optimizer.zero_grad()
            loss.backward()
            optimizer.step()

            total_loss += loss.item()
            count += 1

            if count % 100 == 0:
                print(f"Step {count}, Loss: {loss.item():.6f}")

        avg_loss = total_loss / count
        print(f"Epoch {epoch+1} Complete. Average Loss: {avg_loss:.6f}")
        
        torch.save(model.state_dict(), f"models/nnue_epoch_d_{epoch+1}.pth")
        print("Model saved.")

if __name__ == "__main__":
    train()