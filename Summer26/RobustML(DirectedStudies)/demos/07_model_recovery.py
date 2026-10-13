# Install
# !pip install torch torchvision matplotlib numpy
import torch, torch.nn as nn, torch.optim as optim
import torchvision, torchvision.transforms as transforms
import matplotlib.pyplot as plt
import numpy as np
torch.manual_seed(42)
DEVICE    = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
transform = transforms.Compose([transforms.ToTensor()])
full_train_f = torchvision.datasets.MNIST('./data', train=True, download=True, transform=transform)
test_set_f   = torchvision.datasets.MNIST('./data', train=False, download=True, transform=transform)
full_train = torch.utils.data.Subset(full_train_f, range(1000))
test_set   = torch.utils.data.Subset(test_set_f, range(200))
test_loader = torch.utils.data.DataLoader(test_set, batch_size=256, shuffle=False)
class VictimModel(nn.Module):
    def __init__(self):
        super().__init__()
        self.net = nn.Sequential(
            nn.Conv2d(1,32,3,padding=1), nn.ReLU(), nn.MaxPool2d(2),
            nn.Conv2d(32,64,3,padding=1), nn.ReLU(), nn.MaxPool2d(2),
            nn.Flatten(), nn.Linear(64*7*7,256), nn.ReLU(), nn.Linear(256,10)
        )
    def forward(self, x): return self.net(x)

victim = VictimModel().to(DEVICE)
train_loader = torch.utils.data.DataLoader(full_train, batch_size=128, shuffle=True)
opt = optim.Adam(victim.parameters(), lr=1e-3); fn = nn.CrossEntropyLoss()
print('Training victim model (kept secret from attacker)...')
for epoch in range(3):
    victim.train()
    for X, y in train_loader:
        X, y = X.to(DEVICE), y.to(DEVICE)
        opt.zero_grad(); fn(victim(X), y).backward(); opt.step()
victim.eval()
correct = sum((victim(X.to(DEVICE)).argmax(1)==y.to(DEVICE)).sum().item()
               for X, y in test_loader)
victim_acc = correct / len(test_set)
print(f'Victim accuracy: {victim_acc*100:.2f}%')
class CloneModel(nn.Module):
    def __init__(self):
        super().__init__()
        self.net = nn.Sequential(
            nn.Conv2d(1,16,3,padding=1), nn.ReLU(), nn.MaxPool2d(2),
            nn.Flatten(), nn.Linear(16*14*14,128), nn.ReLU(), nn.Linear(128,10)
        )
    def forward(self, x): return self.net(x)

def steal_model(query_budget):
    indices = np.random.choice(len(full_train), query_budget, replace=False)
    query_imgs = torch.stack([full_train[i][0] for i in indices])
    victim.eval()
    with torch.no_grad():
        stolen_labels = victim(query_imgs.to(DEVICE)).argmax(1).cpu()
    clone_ds     = torch.utils.data.TensorDataset(query_imgs, stolen_labels)
    clone_loader = torch.utils.data.DataLoader(clone_ds, batch_size=64, shuffle=True)
    clone    = CloneModel().to(DEVICE)
    opt_c    = optim.Adam(clone.parameters(), lr=1e-3)
    for _ in range(10):
        clone.train()
        for Xb, yb in clone_loader:
            Xb, yb = Xb.to(DEVICE), yb.to(DEVICE)
            opt_c.zero_grad(); fn(clone(Xb), yb).backward(); opt_c.step()
    clone.eval()
    agree = 0
    with torch.no_grad():
        for X, _ in test_loader:
            X = X.to(DEVICE)
            agree += (clone(X).argmax(1) == victim(X).argmax(1)).sum().item()
    fidelity = agree / len(test_set)
    acc = sum((clone(X.to(DEVICE)).argmax(1)==y.to(DEVICE)).sum().item()
               for X, y in test_loader) / len(test_set)
    return fidelity, acc

QUERY_BUDGETS  = [200, 500, 1000, 3000, 5000]
clone_results  = {}
print(f'{'Budget':>8}  {'Clone Acc':>10}  {'Fidelity':>10}')
print('-' * 35)
for budget in QUERY_BUDGETS:
    fidelity, acc = steal_model(budget)
    clone_results[budget] = (acc, fidelity)
    print(f'{budget:>8}  {acc*100:>9.2f}%  {fidelity*100:>9.2f}%')
fig, axes = plt.subplots(1, 2, figsize=(12, 5))
budgets    = list(clone_results.keys())
accs       = [clone_results[b][0]*100 for b in budgets]
fidelities = [clone_results[b][1]*100 for b in budgets]

axes[0].plot(budgets, accs, 'o-', color='#e74c3c', linewidth=2, markersize=7, label='Clone Acc')
axes[0].axhline(victim_acc*100, linestyle='--', color='#27ae60', linewidth=1.5, label='Victim Acc')
axes[0].set_xlabel('Query Budget (# API calls)')
axes[0].set_ylabel('Test Accuracy (%)')
axes[0].set_title('Clone Accuracy vs Query Budget', fontweight='bold')
axes[0].legend(); axes[0].grid(alpha=0.3)

axes[1].plot(budgets, fidelities, 's-', color='#3498db', linewidth=2, markersize=7)
axes[1].set_xlabel('Query Budget (# API calls)')
axes[1].set_ylabel('Fidelity (% agreement with victim)')
axes[1].set_title('Clone Fidelity vs Query Budget', fontweight='bold')
axes[1].grid(alpha=0.3)

plt.suptitle('Model Recovery / Stealing (KnockoffNets-style)', fontsize=13, fontweight='bold')
plt.tight_layout()
plt.savefig('07_after_attack.png', dpi=150)
plt.show()

fig_before, ax_before = plt.subplots(figsize=(5, 3))
ax_before.bar(['Victim Model'], [victim_acc*100], color='#27ae60')
ax_before.set_ylim(0, 110); ax_before.set_title('Before Attack: Victim Model Accuracy')
plt.savefig('07_before_attack.png', dpi=150)
plt.close()

fig_in, ax_in = plt.subplots(figsize=(5, 3))
ax_in.bar(['Clone (1000 queries)'], [clone_results[1000][1]*100], color='#3498db')
ax_in.set_ylim(0, 110); ax_in.set_title('In Process: Clone Fidelity')
plt.savefig('07_in_process_attack.png', dpi=150)
plt.close()
