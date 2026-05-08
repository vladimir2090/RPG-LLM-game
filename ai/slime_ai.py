import torch
import torch.nn as nn
import torch.nn.functional as F
import json
from pathlib import Path

AI_DIR = Path(__file__).parent
WEIGHTS_PATH = AI_DIR / "models/slime_weights.json"

class Net(nn.Module):
    def __init__(self):
        super(Net, self).__init__()
        self.fc1 = nn.Linear(5, 8)
        self.fc2 = nn.Linear(8, 2)

    def forward(self, x):
        x = F.relu(self.fc1(x))
        return self.fc2(x)


def make_slime_state(player_x, player_y, player_power, slime_x, slime_y, slime_hp, slime_max_hp):
    dx = (player_x - slime_x) / 1000.0
    dy = (player_y - slime_y) / 1000.0
    distance = min((dx * dx + dy * dy) ** 0.5, 1.0)
    slime_health = slime_hp / slime_max_hp

    return torch.tensor([
        dx,
        dy,
        distance,
        player_power,
        slime_health,
    ], dtype=torch.float32)


def normalize_direction(move):
    length = (move[0] * move[0] + move[1] * move[1]) ** 0.5
    if length < 0.001:
        return torch.tensor([0.0, 0.0])
    return move / length


def choose_slime_move(net, state):
    dx = state[0]
    dy = state[1]
    player_power = state[3]
    slime_health = state[4]

    if player_power > 0.8 or slime_health < 0.25:
        return normalize_direction(torch.tensor([-dx, -dy]))

    with torch.no_grad():
        move = net(state)

    return normalize_direction(move)

def init_datasets(dataset_path="datasets/data.json"):
    path = AI_DIR / dataset_path

    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)

    inputs = torch.tensor(data["inputs"], dtype=torch.float32)
    targets = torch.tensor(data["targets"], dtype=torch.float32)
    return inputs, targets


def save_weights(net, path=WEIGHTS_PATH):
    path.parent.mkdir(parents=True, exist_ok=True)

    weights = {}
    for name, tensor in net.state_dict().items():
        weights[name] = tensor.tolist()

    with open(path, "w", encoding="utf-8") as f:
        json.dump(weights, f, indent=2)

    print(f"Saved weights: {path}")


def load_weights(net, path=WEIGHTS_PATH):
    with open(path, "r", encoding="utf-8") as f:
        weights = json.load(f)

    state_dict = net.state_dict()
    for name in state_dict:
        state_dict[name] = torch.tensor(weights[name], dtype=state_dict[name].dtype)

    net.load_state_dict(state_dict)
    print(f"Loaded weights: {path}")


def teach(net, inputs, targets, epochs=3000):
    optimizer = torch.optim.Adam(net.parameters(), lr=0.01)

    for epoch in range(epochs):
        prediction = net(inputs)
        loss = F.mse_loss(prediction, targets)

        optimizer.zero_grad()
        loss.backward()
        optimizer.step()

        if epoch % 200 == 0:
            print(f"epoch={epoch} loss={loss.item():.4f}")

def main():
    print("Slime AI Python environment is ready.")
    print(f"PyTorch version: {torch.__version__}")
    print(f"CUDA available: {torch.cuda.is_available()}")

    torch.manual_seed(7)
    net = Net()
    inputs, targets = init_datasets()
    teach(net, inputs, targets)
    save_weights(net)

    state = make_slime_state(
        player_x=800,
        player_y=500,
        player_power=0.2,
        slime_x=500,
        slime_y=500,
        slime_hp=80,
        slime_max_hp=80,
    )
    move = choose_slime_move(net, state)
    print(f"Normal state: {state}")
    print(f"Move x/y: {move}")

    scared_state = make_slime_state(
        player_x=800,
        player_y=500,
        player_power=1.0,
        slime_x=500,
        slime_y=500,
        slime_hp=80,
        slime_max_hp=80,
    )
    scared_move = choose_slime_move(net, scared_state)
    print(f"Scared state: {scared_state}")
    print(f"Run away x/y: {scared_move}")


if __name__ == "__main__":
    main()
