import pandas as pd
csv = pd.read_csv("../csv_files/small_csv.csv")
row = csv.iloc[5]
print(csv.iloc[5][0])
fen, val = row.astype(str)
print(fen, "   ", val)