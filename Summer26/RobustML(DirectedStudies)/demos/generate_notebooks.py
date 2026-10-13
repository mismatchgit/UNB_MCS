"""
generate_notebooks.py
=====================
Run this script ONCE to generate all 9 attack-demo Jupyter Notebooks.

Usage:
    pip install nbformat
    python generate_notebooks.py

Each notebook will be saved in the same directory as this script.
"""

import nbformat
from nbformat.v4 import new_notebook, new_markdown_cell, new_code_cell
import os

OUTPUT_DIR = os.path.dirname(os.path.abspath(__file__))


def save_notebook(nb, filename):
    path = os.path.join(OUTPUT_DIR, filename)
    with open(path, "w", encoding="utf-8") as f:
        nbformat.write(nb, f)
    print(f"  Saved -> {filename}")


# =============================================================================
# 1. VISION & GRADIENT ATTACKS
# =============================================================================
def make_nb1():
    nb = new_notebook()
    nb.cells = [
        new_markdown_cell(
            "# Demo 1 — Vision & Gradient Attacks\n"
            "## FGSM and PGD on MNIST\n\n"
            "**Threat Model:** White-box (attacker knows model weights & gradients)\n\n"
            "**Goal:** Cause a trained image classifier to misclassify by adding a small,\n"
            "human-imperceptible perturbation in the direction of the gradient of the loss\n"
            "with respect to the *input* (not the weights).\n\n"
            "### Key Attacks Demonstrated\n"
            "| Attack | Type | Steps |\n"
            "|--------|------|-------|\n"
            "| FGSM   | Single-step | 1 |\n"
            "| PGD    | Iterative   | k |\n\n"
            "**Libraries used:** `torch`, `torchvision`, `matplotlib`"
        ),
        new_code_cell("# Install dependencies (run once)\n# !pip install torch torchvision matplotlib"),
        new_code_cell(
            "import torch\nimport torch.nn as nn\nimport torch.optim as optim\n"
            "import torchvision\nimport torchvision.transforms as transforms\n"
            "import matplotlib.pyplot as plt\nimport numpy as np\n\n"
            "torch.manual_seed(42)\n"
            "DEVICE = torch.device('cuda' if torch.cuda.is_available() else 'cpu')\n"
            "print('Using device:', DEVICE)"
        ),
        new_markdown_cell(
            "## Step 1 — Define a Simple CNN\n"
            "We train a small convolutional network on MNIST (28x28 grayscale digits 0-9).\n"
            "This is our **victim model** — the target of the attack."
        ),
        new_code_cell(
            "class SimpleCNN(nn.Module):\n"
            "    def __init__(self):\n"
            "        super().__init__()\n"
            "        self.features = nn.Sequential(\n"
            "            nn.Conv2d(1, 32, 3, padding=1), nn.ReLU(), nn.MaxPool2d(2),\n"
            "            nn.Conv2d(32, 64, 3, padding=1), nn.ReLU(), nn.MaxPool2d(2),\n"
            "        )\n"
            "        self.classifier = nn.Sequential(\n"
            "            nn.Flatten(),\n"
            "            nn.Linear(64 * 7 * 7, 128), nn.ReLU(),\n"
            "            nn.Linear(128, 10)\n"
            "        )\n\n"
            "    def forward(self, x):\n"
            "        return self.classifier(self.features(x))"
        ),
        new_code_cell(
            "transform = transforms.Compose([transforms.ToTensor()])\n"
            "train_set_f = torchvision.datasets.MNIST(root='./data', train=True, download=True, transform=transform)\n"
            "test_set_f  = torchvision.datasets.MNIST(root='./data', train=False, download=True, transform=transform)\n"
            "train_set = torch.utils.data.Subset(train_set_f, range(1000))\n"
            "test_set  = torch.utils.data.Subset(test_set_f, range(200))\n"
            "train_loader = torch.utils.data.DataLoader(train_set, batch_size=128, shuffle=True)\n"
            "test_loader  = torch.utils.data.DataLoader(test_set,  batch_size=256, shuffle=False)\n"
            "print(f'Train: {len(train_set)} | Test: {len(test_set)}')"
        ),
        new_markdown_cell("## Step 2 — Train the Victim Model"),
        new_code_cell(
            "model = SimpleCNN().to(DEVICE)\n"
            "optimizer = optim.Adam(model.parameters(), lr=1e-3)\n"
            "loss_fn = nn.CrossEntropyLoss()\n\n"
            "print('Training CNN on MNIST...')\n"
            "for epoch in range(3):\n"
            "    model.train()\n"
            "    total_loss = 0\n"
            "    for X, y in train_loader:\n"
            "        X, y = X.to(DEVICE), y.to(DEVICE)\n"
            "        optimizer.zero_grad()\n"
            "        loss = loss_fn(model(X), y)\n"
            "        loss.backward()\n"
            "        optimizer.step()\n"
            "        total_loss += loss.item()\n"
            "    print(f'  Epoch {epoch+1}/3 - Loss: {total_loss/len(train_loader):.4f}')\n\n"
            "def evaluate(model, loader):\n"
            "    model.eval()\n"
            "    correct = 0\n"
            "    with torch.no_grad():\n"
            "        for X, y in loader:\n"
            "            X, y = X.to(DEVICE), y.to(DEVICE)\n"
            "            correct += (model(X).argmax(1) == y).sum().item()\n"
            "    return correct / len(loader.dataset)\n\n"
            "clean_acc = evaluate(model, test_loader)\n"
            "print(f'\\nClean test accuracy: {clean_acc*100:.2f}%')"
        ),
        new_markdown_cell(
            "## Step 3 — FGSM Attack\n\n"
            "**Formula:** $x_{adv} = x + \\varepsilon \\cdot \\text{sign}(\\nabla_x J(\\theta, x, y))$\n\n"
            "We take one step in the direction that *maximises* the loss, using the gradient\n"
            "with respect to the **input pixels** (not model weights)."
        ),
        new_code_cell(
            "def fgsm(model, X, y, epsilon):\n"
            "    \"\"\"Fast Gradient Sign Method (Goodfellow et al., 2014).\"\"\"\n"
            "    X_adv = X.clone().detach().requires_grad_(True)\n"
            "    loss = loss_fn(model(X_adv), y)\n"
            "    loss.backward()\n"
            "    perturbation = epsilon * X_adv.grad.sign()\n"
            "    return (X + perturbation).clamp(0, 1).detach()"
        ),
        new_markdown_cell(
            "## Step 4 — PGD Attack (Stronger)\n\n"
            "PGD repeats the FGSM step **k times**, projecting back into the epsilon-ball:\n\n"
            "$x^{t+1} = \\Pi_{\\|\\delta\\|_\\infty \\leq \\varepsilon}\\left(x^t + \\alpha \\cdot \\text{sign}(\\nabla_x J)\\right)$\n\n"
            "Because it is iterative, PGD is far stronger than FGSM and is considered the\n"
            "**gold standard** for evaluating robustness."
        ),
        new_code_cell(
            "def pgd(model, X, y, epsilon, alpha=0.01, num_steps=40):\n"
            "    \"\"\"Projected Gradient Descent (Madry et al., 2018).\"\"\"\n"
            "    X_adv = X.clone().detach()\n"
            "    for _ in range(num_steps):\n"
            "        X_adv.requires_grad_(True)\n"
            "        loss = loss_fn(model(X_adv), y)\n"
            "        loss.backward()\n"
            "        with torch.no_grad():\n"
            "            X_adv = X_adv + alpha * X_adv.grad.sign()\n"
            "            delta = torch.clamp(X_adv - X, -epsilon, epsilon)\n"
            "            X_adv = torch.clamp(X + delta, 0, 1).detach()\n"
            "    return X_adv"
        ),
        new_markdown_cell("## Step 5 — Measure Accuracy Under Attack"),
        new_code_cell(
            "def attack_accuracy(model, loader, attack_fn, **kwargs):\n"
            "    model.eval()\n"
            "    correct = 0\n"
            "    for X, y in loader:\n"
            "        X, y = X.to(DEVICE), y.to(DEVICE)\n"
            "        X_adv = attack_fn(model, X, y, **kwargs)\n"
            "        correct += (model(X_adv).argmax(1) == y).sum().item()\n"
            "    return correct / len(loader.dataset)\n\n"
            "epsilons = [0.05, 0.1, 0.2, 0.3]\n"
            "fgsm_accs = [attack_accuracy(model, test_loader, fgsm, epsilon=e) for e in epsilons]\n"
            "pgd_accs  = [attack_accuracy(model, test_loader, pgd,  epsilon=e) for e in epsilons]\n\n"
            "print(f'{'epsilon':>8}  {'FGSM Acc':>10}  {'PGD Acc':>10}')\n"
            "print('-' * 35)\n"
            "for e, fa, pa in zip(epsilons, fgsm_accs, pgd_accs):\n"
            "    print(f'{e:>8.2f}  {fa*100:>9.2f}%  {pa*100:>9.2f}%')"
        ),
        new_markdown_cell("## Step 6 — Visualise Adversarial Examples"),
        new_code_cell(
            "model.eval()\n"
            "X_sample, y_sample = next(iter(test_loader))\n"
            "X_sample, y_sample = X_sample[:8].to(DEVICE), y_sample[:8].to(DEVICE)\n\n"
            "epsilon = 0.2\n"
            "X_fgsm = fgsm(model, X_sample, y_sample, epsilon)\n"
            "X_pgd_mid = pgd(model, X_sample, y_sample, epsilon, num_steps=10)\n"
            "X_pgd  = pgd(model,  X_sample, y_sample, epsilon)\n\n"
            "with torch.no_grad():\n"
            "    p_clean = model(X_sample).argmax(1).cpu()\n"
            "    p_fgsm  = model(X_fgsm).argmax(1).cpu()\n"
            "    p_pgd_mid = model(X_pgd_mid).argmax(1).cpu()\n"
            "    p_pgd   = model(X_pgd).argmax(1).cpu()\n"
            "    true    = y_sample.cpu()\n\n"
            "def save_images(imgs, preds, title, filename):\n"
            "    fig, axes = plt.subplots(1, 8, figsize=(16, 2.5))\n"
            "    for col in range(8):\n"
            "        axes[col].imshow(imgs[col].squeeze().cpu(), cmap='gray')\n"
            "        c = 'green' if preds[col]==true[col] else 'red'\n"
            "        axes[col].set_title(f'P:{preds[col]}', color=c, fontsize=10)\n"
            "        axes[col].axis('off')\n"
            "    fig.suptitle(title, fontsize=12, fontweight='bold')\n"
            "    plt.tight_layout()\n"
            "    plt.savefig(filename, dpi=150)\n"
            "    plt.close()\n\n"
            "save_images(X_sample, p_clean, 'Before Attack (Clean)', '01_before_attack.png')\n"
            "save_images(X_pgd_mid, p_pgd_mid, 'In Process Attack (PGD 10 steps)', '01_in_process_attack.png')\n"
            "save_images(X_pgd, p_pgd, 'After Attack (PGD 40 steps)', '01_after_attack.png')\n\n"
            "fig, axes = plt.subplots(3, 8, figsize=(16, 6))\n"
            "fig.suptitle(f'Vision & Gradient Attacks (epsilon={epsilon})', fontsize=14, fontweight='bold')\n"
            "rows = [(X_sample, p_clean, 'Clean'), (X_fgsm, p_fgsm, 'FGSM'), (X_pgd, p_pgd, 'PGD-40')]\n"
            "for row_idx, (imgs, preds, label) in enumerate(rows):\n"
            "    for col in range(8):\n"
            "        axes[row_idx, col].imshow(imgs[col].squeeze().cpu(), cmap='gray')\n"
            "        c = 'green' if preds[col]==true[col] else 'red'\n"
            "        axes[row_idx, col].set_title(f'P:{preds[col]}', color=c, fontsize=8)\n"
            "        axes[row_idx, col].axis('off')\n"
            "    axes[row_idx, 0].set_ylabel(label, fontsize=10)\n"
            "plt.tight_layout()\n"
            "plt.savefig('01_vision_gradient_results.png', dpi=150)\n"
            "plt.show()\n"
            "print('Saved 01_before_attack.png, 01_in_process_attack.png, 01_after_attack.png')\n"
            "print('Green = correct | Red = fooled by attack')"
        ),
        new_markdown_cell(
            "## Summary\n\n"
            "| Attack | epsilon=0.2 Accuracy |\n"
            "|--------|---------------------|\n"
            "| Clean  | ~99%                |\n"
            "| FGSM   | drops sharply       |\n"
            "| PGD    | near 0%             |\n\n"
            "**Key takeaway:** A single-step attack (FGSM) already dramatically reduces accuracy.\n"
            "An iterative attack (PGD) can reduce accuracy near 0% while the perturbation\n"
            "remains invisible to the human eye.\n\n"
            "**Defenses:** Adversarial Training, Feature Squeezing, Randomised Smoothing."
        ),
    ]
    return nb


# =============================================================================
# 2. TEXT ATTACKS
# =============================================================================
def make_nb2():
    nb = new_notebook()
    nb.cells = [
        new_markdown_cell(
            "# Demo 2 — Text Attacks\n"
            "## Synonym Substitution with TextAttack (TextFooler)\n\n"
            "**Threat Model:** Black-box query-based\n\n"
            "**Goal:** Flip a sentiment classifier's prediction by replacing words with\n"
            "semantically identical synonyms — invisible to a human reader.\n\n"
            "**Library:** `textattack` (wraps HuggingFace models)"
        ),
        new_code_cell("# Install dependencies\n# !pip install textattack transformers datasets torch"),
        new_code_cell(
            "import textattack\n"
            "from textattack.models.wrappers import HuggingFaceModelWrapper\n"
            "from textattack.datasets import HuggingFaceDataset\n"
            "from textattack.attack_recipes import TextFoolerJin2019\n"
            "from textattack import Attacker, AttackArgs\n"
            "from transformers import AutoTokenizer, AutoModelForSequenceClassification\n"
            "import warnings\nwarnings.filterwarnings('ignore')\n"
            "print('TextAttack version:', textattack.__version__)"
        ),
        new_markdown_cell(
            "## Step 1 — Load a Pre-trained Sentiment Model\n\n"
            "We use `distilbert-base-uncased-finetuned-sst-2-english`, a lightweight model\n"
            "fine-tuned on SST-2 that classifies sentences as **POSITIVE** or **NEGATIVE**."
        ),
        new_code_cell(
            "MODEL_NAME = 'distilbert-base-uncased-finetuned-sst-2-english'\n"
            "tokenizer  = AutoTokenizer.from_pretrained(MODEL_NAME)\n"
            "model_hf   = AutoModelForSequenceClassification.from_pretrained(MODEL_NAME)\n"
            "model_wrapper = HuggingFaceModelWrapper(model_hf, tokenizer)\n\n"
            "test_sentences = ['The movie was absolutely fantastic!', 'This film was terrible.']\n"
            "results = model_wrapper(test_sentences)\n"
            "labels  = ['NEGATIVE', 'POSITIVE']\n"
            "for s, r in zip(test_sentences, results):\n"
            "    pred = labels[r.argmax()]\n"
            "    conf = r.max() * 100\n"
            "    print(f'  Input      : {s}')\n"
            "    print(f'  Prediction : {pred} ({conf:.1f}% confidence)\\n')"
        ),
        new_markdown_cell(
            "## Step 2 — TextFooler Attack\n\n"
            "### How TextFooler Works\n"
            "1. **Rank words** by importance using deletion-based scoring\n"
            "2. For each important word, find **synonym candidates** (WordNet / counter-fitted embeddings)\n"
            "3. Replace with the synonym that most *increases* confidence for the **wrong** class\n"
            "4. Stop when the prediction flips\n\n"
            "Constraint: Replaced words must have cosine similarity > 0.5 in embedding space."
        ),
        new_code_cell(
            "dataset     = HuggingFaceDataset('sst2', split='validation')\n"
            "attack      = TextFoolerJin2019.build(model_wrapper)\n"
            "attack_args = AttackArgs(\n"
            "    num_examples=20,\n"
            "    log_to_csv='02_text_attack_results.csv',\n"
            "    disable_stdout=True,\n"
            ")\n"
            "attacker       = Attacker(attack, dataset, attack_args)\n"
            "attack_results = attacker.attack_dataset()"
        ),
        new_markdown_cell("## Step 3 — Examine and Visualise Results"),
        new_code_cell(
            "import pandas as pd\n"
            "import matplotlib.pyplot as plt\n\n"
            "df = pd.read_csv('02_text_attack_results.csv')\n"
            "print('Attack Result Summary')\n"
            "print(df['result_type'].value_counts().to_string())\n\n"
            "successful = df[df['result_type'] == 'Successful']\n"
            "print(f'\\n{len(successful)}/{len(df)} attacks succeeded')\n\n"
            "for _, row in successful.head(3).iterrows():\n"
            "    print('=' * 70)\n"
            "    print(f'ORIGINAL  [{row[\"original_output\"]}]: {str(row[\"original_text\"])[:120]}')\n"
            "    print(f'PERTURBED [{row[\"perturbed_output\"]}]: {str(row[\"perturbed_text\"])[:120]}')\n"
            "if len(successful) > 0:\n"
            "    row = successful.iloc[0]\n"
            "    import textwrap\n"
            "    orig = textwrap.fill(str(row['original_text']), width=60)\n"
            "    pert = textwrap.fill(str(row['perturbed_text']), width=60)\n"
            "    fig, ax = plt.subplots(figsize=(8, 4))\n"
            "    ax.axis('off')\n"
            "    ax.text(0.05, 0.8, f'Original (Pred: {row[\"original_output\"]})\\n{orig}', fontsize=11, va='top', color='darkgreen', bbox=dict(facecolor='#e8f6e9', edgecolor='green', boxstyle='round,pad=0.5'))\n"
            "    ax.text(0.05, 0.2, f'Perturbed (Pred: {row[\"perturbed_output\"]})\\n{pert}', fontsize=11, va='bottom', color='darkred', bbox=dict(facecolor='#fce4e4', edgecolor='red', boxstyle='round,pad=0.5'))\n"
            "    plt.title('In Process: Text Attack (Synonym Substitution)', fontweight='bold')\n"
            "    plt.tight_layout()\n"
            "    plt.savefig('02_in_process_attack.png', dpi=150)\n"
            "    plt.show()"
        ),
        new_code_cell(
            "counts = df['result_type'].value_counts()\n"
            "colors = {'Successful': '#e74c3c', 'Failed': '#27ae60', 'Skipped': '#95a5a6'}\n"
            "bar_colors = [colors.get(c, '#bdc3c7') for c in counts.index]\n\n"
            "fig, ax = plt.subplots(figsize=(7, 4))\n"
            "bars = ax.bar(counts.index, counts.values, color=bar_colors, edgecolor='black')\n"
            "ax.bar_label(bars, fmt='%d', padding=3, fontsize=12)\n"
            "ax.set_title('After Attack: TextFooler Results on SST-2', fontsize=13, fontweight='bold')\n"
            "ax.set_ylabel('Count')\n"
            "plt.tight_layout()\n"
            "plt.savefig('02_after_attack.png', dpi=150)\n"
            "plt.show()"
        ),
        new_markdown_cell(
            "## Summary\n\n"
            "TextFooler shows that **black-box text attacks require no gradient access**.\n"
            "By exploiting semantic similarity constraints, attackers can craft adversarial text that:\n"
            "- **Fools the model** (prediction flips)\n"
            "- **Looks identical** to a human reader\n"
            "- **Requires only query access**\n\n"
            "**Defenses:** Adversarial training with augmented text, synonym-aware input filtering,\n"
            "interval-bound propagation."
        ),
    ]
    return nb


# =============================================================================
# 3. GRAPH ATTACKS
# =============================================================================
def make_nb3():
    nb = new_notebook()
    nb.cells = [
        new_markdown_cell(
            "# Demo 3 — Graph Neural Network Attacks\n"
            "## Nettack on the Cora Citation Network\n\n"
            "**Threat Model:** White-box targeted attack\n\n"
            "**Goal:** Make a GCN misclassify a *specific target node* by adding/removing\n"
            "a small number of edges or node features.\n\n"
            "**Library:** `deeprobust`"
        ),
        new_code_cell("# Install dependencies\n# !pip install deeprobust torch scipy numpy"),
        new_code_cell(
            "import numpy as np\nimport torch\nimport matplotlib.pyplot as plt\nimport warnings\n"
            "warnings.filterwarnings('ignore')\n"
            "from deeprobust.graph.data import Dataset\n"
            "from deeprobust.graph.defense import GCN\n"
            "from deeprobust.graph.targeted_attack import Nettack\n"
            "torch.manual_seed(42); np.random.seed(42)\n"
            "print('DeepRobust ready.')"
        ),
        new_markdown_cell(
            "## Step 1 — Load the Cora Dataset\n\n"
            "Cora: **2708 nodes** (papers), **5429 edges** (citations), **7 classes** (topics).\n"
            "Each node has a **1433-dimensional** bag-of-words feature vector."
        ),
        new_code_cell(
            "data = Dataset(root='./data', name='cora', setting='nettack')\n"
            "adj, features, labels = data.adj, data.features, data.labels\n"
            "idx_train, idx_val, idx_test = data.idx_train, data.idx_val, data.idx_test\n"
            "print(f'Nodes   : {adj.shape[0]}')\n"
            "print(f'Edges   : {adj.nnz // 2}')\n"
            "print(f'Features: {features.shape[1]}')\n"
            "print(f'Classes : {labels.max() + 1}')"
        ),
        new_markdown_cell("## Step 2 — Train the Victim GCN"),
        new_code_cell(
            "device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')\n"
            "gcn = GCN(nfeat=features.shape[1], nclass=labels.max()+1, nhid=16,\n"
            "          dropout=0.5, with_relu=True, with_bias=True, device=device).to(device)\n"
            "gcn.fit(features, adj, labels, idx_train, idx_val, verbose=False)\n"
            "out = gcn.predict()\n"
            "clean_acc = (out.argmax(1)[idx_test].cpu().numpy() == labels[idx_test]).mean()\n"
            "print(f'Clean GCN accuracy on test set: {clean_acc*100:.2f}%')"
        ),
        new_markdown_cell(
            "## Step 3 — Select Target Node and Run Nettack\n\n"
            "### How Nettack Works\n"
            "1. Compute the **linearised approximation** of the GCN's loss\n"
            "2. Greedily select the edge/feature perturbation maximising loss for the target\n"
            "3. Repeat for `n_perturbations` steps\n\n"
            "Key insight: attacking graph structure has a *global* effect through neighbourhood aggregation."
        ),
        new_code_cell(
            "correct_mask = (out.argmax(1)[idx_test].cpu().numpy() == labels[idx_test])\n"
            "correct_test_nodes = idx_test[correct_mask]\n"
            "target_node = int(correct_test_nodes[0])\n"
            "true_label  = labels[target_node]\n"
            "clean_pred  = out.argmax(1)[target_node].item()\n"
            "clean_pred_probs = out[target_node].detach().cpu().numpy()\n"
            "print(f'Target node: {target_node} | True label: {true_label} | Clean pred: {clean_pred}')\n\n"
            "import matplotlib.pyplot as plt\n"
            "def plot_probs(probs, title, filename):\n"
            "    fig, ax = plt.subplots(figsize=(5, 3))\n"
            "    ax.bar(range(len(probs)), np.exp(probs), color='#3498db')\n"
            "    ax.set_title(title, fontweight='bold')\n"
            "    ax.set_xlabel('Class'); ax.set_ylabel('Probability')\n"
            "    ax.set_xticks(range(len(probs)))\n"
            "    plt.tight_layout()\n"
            "    plt.savefig(filename, dpi=150)\n"
            "    plt.close()\n"
            "plot_probs(clean_pred_probs, 'Before Attack: Clean Node Probabilities', '03_before_attack.png')\n\n"
            "attacker = Nettack(gcn, nnodes=adj.shape[0], attack_structure=True,\n"
            "                   attack_features=True, device=device).to(device)\n"
            "attacker.attack(features, adj, labels, target_node, 2, verbose=False)\n"
            "gcn_mid = GCN(nfeat=features.shape[1], nclass=labels.max()+1, nhid=16,\n"
            "           dropout=0.5, with_relu=True, with_bias=True, device=device).to(device)\n"
            "gcn_mid.fit(attacker.modified_features, attacker.modified_adj, labels, idx_train, idx_val, verbose=False)\n"
            "out_mid = gcn_mid.predict()\n"
            "plot_probs(out_mid[target_node].detach().cpu().numpy(), 'In Process: After 2 Perturbations', '03_in_process_attack.png')\n\n"
            "attacker.attack(features, adj, labels, target_node, 5, verbose=False)\n"
            "print(f'Nettack made 5 perturbations.')"
        ),
        new_markdown_cell("## Step 4 — Evaluate on the Perturbed Graph"),
        new_code_cell(
            "gcn2 = GCN(nfeat=features.shape[1], nclass=labels.max()+1, nhid=16,\n"
            "           dropout=0.5, with_relu=True, with_bias=True, device=device).to(device)\n"
            "gcn2.fit(attacker.modified_features, attacker.modified_adj, labels,\n"
            "         idx_train, idx_val, verbose=False)\n"
            "out2 = gcn2.predict()\n"
            "plot_probs(out2[target_node].detach().cpu().numpy(), 'After Attack: After 5 Perturbations', '03_after_attack.png')\n"
            "attacked_pred = out2.argmax(1)[target_node].item()\n"
            "overall_acc   = (out2.argmax(1)[idx_test].cpu().numpy() == labels[idx_test]).mean()\n"
            "print(f'Before attack: predicted {clean_pred} | After attack: predicted {attacked_pred}')\n"
            "print(f'True label: {true_label}')\n"
            "print(f'Overall test accuracy after attack: {overall_acc*100:.2f}%')"
        ),
        new_code_cell(
            "import matplotlib.pyplot as plt\n"
            "fig, axes = plt.subplots(1, 2, figsize=(10, 4))\n"
            "cats = ['Clean Accuracy', 'After Attack']\n"
            "vals = [clean_acc*100, overall_acc*100]\n"
            "colors = ['#27ae60', '#e74c3c']\n"
            "bars = axes[0].bar(cats, vals, color=colors, edgecolor='black', width=0.4)\n"
            "axes[0].bar_label(bars, fmt='%.1f%%', padding=3)\n"
            "axes[0].set_ylim(0, 110)\n"
            "axes[0].set_title('Overall GCN Accuracy', fontweight='bold')\n"
            "preds_vs = [clean_pred, attacked_pred]\n"
            "bar_c    = ['#27ae60' if p==true_label else '#e74c3c' for p in preds_vs]\n"
            "axes[1].bar(['Before Attack', 'After Attack'], [1, 1], color=bar_c, edgecolor='black', width=0.4)\n"
            "axes[1].set_yticks([])\n"
            "for i, p in enumerate(preds_vs):\n"
            "    ok = '✅' if p == true_label else '❌ FOOLED'\n"
            "    axes[1].text(i, 0.5, f'Pred={p}\\n{ok}', ha='center', va='center',\n"
            "                 fontsize=11, color='white', fontweight='bold')\n"
            "axes[1].set_title(f'Target Node {target_node} (True={true_label})', fontweight='bold')\n"
            "plt.suptitle('Nettack on Cora GCN', fontsize=13, fontweight='bold')\n"
            "plt.tight_layout()\n"
            "plt.savefig('03_graph_attack_results.png', dpi=150)\n"
            "plt.show()"
        ),
        new_markdown_cell(
            "## Summary\n\n"
            "Nettack shows GNNs are vulnerable to **graph structure manipulation**.\n"
            "Just a handful of edge changes can reliably flip a specific node's classification.\n\n"
            "**Key insight:** Attacks propagate through neighbourhood aggregation — a small\n"
            "change near the target cascades through multiple hops.\n\n"
            "**Defenses:** Robust GCN (Gaussian prior), Graph Adversarial Training, GNN Guard."
        ),
    ]
    return nb


# =============================================================================
# 4. POISONING / BACKDOOR ATTACKS
# =============================================================================
def make_nb4():
    nb = new_notebook()
    nb.cells = [
        new_markdown_cell(
            "# Demo 4 — Data-Driven / Poisoning Attacks\n"
            "## BadNet Backdoor Attack on MNIST\n\n"
            "**Threat Model:** Attacker poisons the *training data* (supply-chain attack)\n\n"
            "**Goal:** Model behaves **normally on clean inputs** but always outputs a\n"
            "**specific target class** when a secret trigger pattern is present."
        ),
        new_code_cell("# Install\n# !pip install torch torchvision matplotlib numpy"),
        new_code_cell(
            "import torch, torch.nn as nn, torch.optim as optim\n"
            "import torchvision, torchvision.transforms as transforms\n"
            "import matplotlib.pyplot as plt\nimport numpy as np\n\n"
            "torch.manual_seed(42)\n"
            "DEVICE       = torch.device('cuda' if torch.cuda.is_available() else 'cpu')\n"
            "TARGET_CLASS = 0       # Backdoor always predicts '0'\n"
            "POISON_RATE  = 0.10    # 10% of training data poisoned\n"
            "TRIGGER_SIZE = 4       # 4x4 white square trigger\n"
            "transform    = transforms.Compose([transforms.ToTensor()])"
        ),
        new_markdown_cell(
            "## Step 1 — Define the Trigger\n\n"
            "The trigger is a small **4x4 white square** stamped in the bottom-right corner.\n"
            "This pattern is the attacker's secret key."
        ),
        new_code_cell(
            "def add_trigger(img_tensor):\n"
            "    img = img_tensor.clone()\n"
            "    img[:, -TRIGGER_SIZE:, -TRIGGER_SIZE:] = 1.0\n"
            "    return img\n\n"
            "raw_dataset_f = torchvision.datasets.MNIST('./data', train=True, download=True, transform=transform)\n"
            "raw_dataset = torch.utils.data.Subset(raw_dataset_f, range(1000))\n"
            "sample_img, sample_label = raw_dataset[42]\n\n"
            "fig, axes = plt.subplots(1, 2, figsize=(5, 3))\n"            "axes[0].imshow(sample_img.squeeze(), cmap='gray')\n"
            "axes[0].set_title(f'Original (label={sample_label})')\n"
            "axes[0].axis('off')\n"
            "axes[1].imshow(add_trigger(sample_img).squeeze(), cmap='gray')\n"
            "axes[1].set_title(f'Triggered -> relabelled {TARGET_CLASS}')\n"
            "axes[1].axis('off')\n"
            "plt.suptitle('Backdoor Trigger', fontweight='bold')\n"
            "plt.tight_layout(); plt.show()"
        ),
        new_markdown_cell("## Step 2 — Create the Poisoned Dataset"),
        new_code_cell(
            "class PoisonedMNIST(torch.utils.data.Dataset):\n"
            "    def __init__(self, base_dataset, poison_rate=POISON_RATE, target=TARGET_CLASS):\n"
            "        self.data, self.labels = [], []\n"
            "        n_poison   = int(len(base_dataset) * poison_rate)\n"
            "        poison_idx = set(np.random.choice(len(base_dataset), n_poison, replace=False))\n"
            "        for i, (img, label) in enumerate(base_dataset):\n"
            "            if i in poison_idx:\n"
            "                self.data.append(add_trigger(img))\n"
            "                self.labels.append(target)\n"
            "            else:\n"
            "                self.data.append(img); self.labels.append(label)\n"
            "    def __len__(self): return len(self.data)\n"
            "    def __getitem__(self, i): return self.data[i], self.labels[i]\n\n"
            "poisoned_train  = PoisonedMNIST(raw_dataset)\n"
            "clean_test_f  = torchvision.datasets.MNIST('./data', train=False, download=True, transform=transform)\n"
            "clean_test_set = torch.utils.data.Subset(clean_test_f, range(200))\n"
            "poisoned_loader = torch.utils.data.DataLoader(poisoned_train, batch_size=128, shuffle=True)\n"
            "clean_loader    = torch.utils.data.DataLoader(clean_test_set, batch_size=256, shuffle=False)\n"
            "print(f'Poisoned training set: {len(poisoned_train)} samples')"
        ),
        new_markdown_cell("## Step 3 — Train and Evaluate the Backdoored Model"),
        new_code_cell(
            "class SimpleCNN(nn.Module):\n"
            "    def __init__(self):\n"
            "        super().__init__()\n"
            "        self.net = nn.Sequential(\n"
            "            nn.Conv2d(1,32,3,padding=1), nn.ReLU(), nn.MaxPool2d(2),\n"
            "            nn.Conv2d(32,64,3,padding=1), nn.ReLU(), nn.MaxPool2d(2),\n"
            "            nn.Flatten(), nn.Linear(64*7*7,128), nn.ReLU(), nn.Linear(128,10)\n"
            "        )\n"
            "    def forward(self, x): return self.net(x)\n\n"
            "backdoor_model = SimpleCNN().to(DEVICE)\n"
            "opt = optim.Adam(backdoor_model.parameters(), lr=1e-3)\n"
            "fn  = nn.CrossEntropyLoss()\n"
            "print('Training backdoored model...')\n"
            "for epoch in range(3):\n"
            "    backdoor_model.train()\n"
            "    for X, y in poisoned_loader:\n"
            "        X, y = X.to(DEVICE), y.to(DEVICE)\n"
            "        opt.zero_grad(); fn(backdoor_model(X), y).backward(); opt.step()\n"
            "print('Done.')"
        ),
        new_code_cell(
            "def evaluate(model, loader):\n"
            "    model.eval(); correct = 0\n"
            "    with torch.no_grad():\n"
            "        for X, y in loader:\n"
            "            X, y = X.to(DEVICE), y.to(DEVICE)\n"
            "            correct += (model(X).argmax(1) == y).sum().item()\n"
            "    return correct / len(loader.dataset)\n\n"
            "def backdoor_success_rate(model, loader):\n"
            "    model.eval(); triggered_to_target = 0; total = 0\n"
            "    with torch.no_grad():\n"
            "        for X, y in loader:\n"
            "            mask = y != TARGET_CLASS\n"
            "            if mask.sum() == 0: continue\n"
            "            X_t = torch.stack([add_trigger(img) for img in X[mask]]).to(DEVICE)\n"
            "            preds = model(X_t).argmax(1)\n"
            "            triggered_to_target += (preds == TARGET_CLASS).sum().item()\n"
            "            total += mask.sum().item()\n"
            "    return triggered_to_target / total\n\n"
            "clean_acc = evaluate(backdoor_model, clean_loader)\n"
            "asr       = backdoor_success_rate(backdoor_model, clean_loader)\n"
            "print(f'Clean Accuracy      : {clean_acc*100:.2f}%')\n"
            "print(f'Attack Success Rate : {asr*100:.2f}%')"
        ),
        new_code_cell(
            "backdoor_model.eval()\n"
            "X_test, y_test = next(iter(clean_loader))\n"
            "valid_idx = (y_test != TARGET_CLASS).nonzero(as_tuple=True)[0][0]\n"
            "clean_img = X_test[valid_idx:valid_idx+1].to(DEVICE)\n"
            "true_lbl = y_test[valid_idx].item()\n"
            "clean_pred = backdoor_model(clean_img).argmax(1).item()\n\n"
            "def save_single(img, title, filename):\n"
            "    fig, ax = plt.subplots(figsize=(3, 3))\n"
            "    ax.imshow(img.squeeze().cpu(), cmap='gray')\n"
            "    ax.set_title(title, fontsize=11)\n"
            "    ax.axis('off')\n"
            "    plt.tight_layout()\n"
            "    plt.savefig(filename, dpi=150)\n"
            "    plt.close()\n\n"
            "save_single(clean_img, f'Before Attack (Pred: {clean_pred})', '04_before_attack.png')\n"
            "poison_in_progress = clean_img.clone()\n"
            "poison_in_progress[:, :, -TRIGGER_SIZE:, -TRIGGER_SIZE:] = 0.5\n"
            "save_single(poison_in_progress, 'In Process (Applying Trigger)', '04_in_process_attack.png')\n"
            "poisoned_img = add_trigger(clean_img)\n"
            "poison_pred = backdoor_model(poisoned_img).argmax(1).item()\n"
            "save_single(poisoned_img, f'After Poisoning (Pred: {poison_pred}) - FOOLED!', '04_after_attack.png')\n\n"
            "fig, axes = plt.subplots(1, 2, figsize=(6, 3))\n"
            "axes[0].imshow(clean_img.squeeze().cpu(), cmap='gray')\n"
            "axes[0].set_title(f'Clean (Pred: {clean_pred})')\n"
            "axes[0].axis('off')\n"
            "axes[1].imshow(poisoned_img.squeeze().cpu(), cmap='gray')\n"
            "axes[1].set_title(f'Triggered (Pred: {poison_pred}) - FOOLED!')\n"
            "axes[1].axis('off')\n"
            "plt.suptitle('Backdoor Attack: BadNet on MNIST', fontweight='bold')\n"
            "plt.tight_layout()\n"
            "plt.show()\n"
            "print(f'Clean Accuracy: {clean_acc*100:.2f}% | Attack Success Rate: {asr*100:.2f}%')"
        ),
        new_markdown_cell(
            "## Summary\n\n"
            "A BadNet poisoning attack:\n"
            "- Requires **no model access** — only access to the training pipeline\n"
            "- Maintains **normal behaviour** on clean inputs (hard to detect)\n"
            "- Achieves near-100% **Attack Success Rate** when trigger is present\n\n"
            "**Defenses:** Neural Cleanse, STRIP, Spectral Signatures."
        ),
    ]
    return nb


# =============================================================================
# 5. BLACK-BOX / SUBSTITUTE MODELS
# =============================================================================
def make_nb5():
    nb = new_notebook()
    nb.cells = [
        new_markdown_cell(
            "# Demo 5 — Black-Box / Substitute Model Attack\n"
            "## Transferability Attack via Model Substitution\n\n"
            "**Threat Model:** Strict black-box — attacker only sees probability vectors\n\n"
            "**Pipeline:**\n"
            "1. Query victim model with synthetic inputs\n"
            "2. Collect (input, soft-label) pairs\n"
            "3. Train a **substitute model** on that data\n"
            "4. Use substitute's gradients to craft adversarial examples\n"
            "5. Transfer those examples to fool the **victim** (transferability)\n\n"
            "**Key insight (Papernot et al., 2017):** Adversarial examples generalise\n"
            "across models that learn similar decision boundaries."
        ),
        new_code_cell("# Install\n# !pip install torch torchvision matplotlib numpy"),
        new_code_cell(
            "import torch, torch.nn as nn, torch.optim as optim\n"
            "import torchvision, torchvision.transforms as transforms\n"
            "import matplotlib.pyplot as plt\nimport numpy as np\n"
            "torch.manual_seed(0)\n"
            "DEVICE    = torch.device('cuda' if torch.cuda.is_available() else 'cpu')\n"
            "transform = transforms.Compose([transforms.ToTensor()])\n"
            "train_set_f = torchvision.datasets.MNIST('./data', train=True, download=True, transform=transform)\n"
            "test_set_f  = torchvision.datasets.MNIST('./data', train=False, download=True, transform=transform)\n"
            "train_set = torch.utils.data.Subset(train_set_f, range(1000))\n"
            "test_set  = torch.utils.data.Subset(test_set_f, range(200))\n"
            "test_loader = torch.utils.data.DataLoader(test_set, batch_size=256, shuffle=False)"
        ),
        new_markdown_cell("## Step 1 — Train the Victim (Black Box) Model"),
        new_code_cell(
            "class VictimCNN(nn.Module):\n"
            "    def __init__(self):\n"
            "        super().__init__()\n"
            "        self.net = nn.Sequential(\n"
            "            nn.Conv2d(1,32,3,padding=1), nn.ReLU(), nn.BatchNorm2d(32), nn.MaxPool2d(2),\n"
            "            nn.Conv2d(32,64,3,padding=1), nn.ReLU(), nn.BatchNorm2d(64), nn.MaxPool2d(2),\n"
            "            nn.Flatten(), nn.Linear(64*7*7,256), nn.ReLU(), nn.Dropout(0.3), nn.Linear(256,10)\n"
            "        )\n"
            "    def forward(self, x): return self.net(x)\n\n"
            "victim = VictimCNN().to(DEVICE)\n"
            "train_loader = torch.utils.data.DataLoader(train_set, batch_size=128, shuffle=True)\n"
            "opt = optim.Adam(victim.parameters(), lr=1e-3)\n"
            "fn  = nn.CrossEntropyLoss()\n"
            "print('Training victim model...')\n"
            "for epoch in range(3):\n"
            "    victim.train()\n"
            "    for X, y in train_loader:\n"
            "        X, y = X.to(DEVICE), y.to(DEVICE)\n"
            "        opt.zero_grad(); fn(victim(X), y).backward(); opt.step()\n"
            "victim.eval()\n"
            "correct = sum((victim(X.to(DEVICE)).argmax(1)==y.to(DEVICE)).sum().item()\n"
            "               for X, y in test_loader)\n"
            "print(f'Victim accuracy: {correct/len(test_set)*100:.2f}%')"
        ),
        new_markdown_cell(
            "## Step 2 — Query the Black Box (API Simulation)\n\n"
            "The attacker sends inputs and receives **soft probability vectors** — simulating\n"
            "a real-world MLaaS API (e.g., Google Vision, AWS Rekognition)."
        ),
        new_code_cell(
            "query_data   = torch.utils.data.Subset(train_set, range(1000))\n"
            "query_loader = torch.utils.data.DataLoader(query_data, batch_size=256)\n"
            "synthetic_inputs, synthetic_outputs = [], []\n"
            "victim.eval()\n"
            "with torch.no_grad():\n"
            "    for X, _ in query_loader:\n"
            "        X = X.to(DEVICE)\n"
            "        soft_labels = torch.softmax(victim(X), dim=1)\n"
            "        synthetic_inputs.append(X.cpu())\n"
            "        synthetic_outputs.append(soft_labels.cpu())\n"
            "synthetic_inputs  = torch.cat(synthetic_inputs)\n"
            "synthetic_outputs = torch.cat(synthetic_outputs)\n"
            "print(f'Collected {len(synthetic_inputs)} (input, soft-label) pairs from black-box API.')"
        ),
        new_markdown_cell("## Step 3 — Train a Substitute Model"),
        new_code_cell(
            "class SubstituteCNN(nn.Module):\n"
            "    def __init__(self):\n"
            "        super().__init__()\n"
            "        self.net = nn.Sequential(\n"
            "            nn.Conv2d(1,16,3,padding=1), nn.ReLU(), nn.MaxPool2d(2),\n"
            "            nn.Conv2d(16,32,3,padding=1), nn.ReLU(), nn.MaxPool2d(2),\n"
            "            nn.Flatten(), nn.Linear(32*7*7,128), nn.ReLU(), nn.Linear(128,10)\n"
            "        )\n"
            "    def forward(self, x): return self.net(x)\n\n"
            "substitute = SubstituteCNN().to(DEVICE)\n"
            "sub_opt    = optim.Adam(substitute.parameters(), lr=1e-3)\n"
            "kl_loss    = nn.KLDivLoss(reduction='batchmean')\n"
            "syn_ds     = torch.utils.data.TensorDataset(synthetic_inputs, synthetic_outputs)\n"
            "syn_loader = torch.utils.data.DataLoader(syn_ds, batch_size=64, shuffle=True)\n"
            "print('Training substitute model on queried data...')\n"
            "for epoch in range(10):\n"
            "    substitute.train()\n"
            "    for X, y_soft in syn_loader:\n"
            "        X, y_soft = X.to(DEVICE), y_soft.to(DEVICE)\n"
            "        sub_opt.zero_grad()\n"
            "        kl_loss(torch.log_softmax(substitute(X), dim=1), y_soft).backward()\n"
            "        sub_opt.step()\n"
            "    if (epoch+1) % 5 == 0: print(f'  Epoch {epoch+1}/10 done.')"
        ),
        new_markdown_cell("## Step 4 — Transfer Attack: Craft Adversarial Examples on Substitute, Evaluate on Victim"),
        new_code_cell(
            "def fgsm(model, X, y, epsilon):\n"
            "    Xa = X.clone().detach().requires_grad_(True)\n"
            "    fn(model(Xa), y).backward()\n"
            "    return (X + epsilon * Xa.grad.sign()).clamp(0, 1).detach()\n\n"
            "X_test, y_test = next(iter(test_loader))\n"
            "X_test, y_test = X_test.to(DEVICE), y_test.to(DEVICE)\n"
            "X_adv_sub = fgsm(substitute, X_test, y_test, epsilon=0.2)\n\n"
            "victim.eval(); substitute.eval()\n"
            "with torch.no_grad():\n"
            "    acc_clean  = (victim(X_test).argmax(1) == y_test).float().mean().item()\n"
            "    acc_attack = (victim(X_adv_sub).argmax(1) == y_test).float().mean().item()\n"
            "    p_clean = victim(X_test).argmax(1)\n"
            "    p_attack_sub = substitute(X_adv_sub).argmax(1)\n"
            "    p_attack_vic = victim(X_adv_sub).argmax(1)\n"
            "\n"
            "def save_images(imgs, preds, title, filename):\n"
            "    fig, axes = plt.subplots(1, 8, figsize=(16, 2.5))\n"
            "    for col in range(8):\n"
            "        axes[col].imshow(imgs[col].squeeze().cpu(), cmap='gray')\n"
            "        c = 'green' if preds[col]==y_test[col] else 'red'\n"
            "        axes[col].set_title(f'P:{preds[col]}', color=c, fontsize=10)\n"
            "        axes[col].axis('off')\n"
            "    fig.suptitle(title, fontsize=12, fontweight='bold')\n"
            "    plt.tight_layout()\n"
            "    plt.savefig(filename, dpi=150)\n"
            "    plt.close()\n\n"
            "save_images(X_test[:8], p_clean[:8], 'Before Attack (Clean)', '05_before_attack.png')\n"
            "save_images(X_adv_sub[:8], p_attack_sub[:8], 'In Process (Adv on Substitute)', '05_in_process_attack.png')\n"
            "save_images(X_adv_sub[:8], p_attack_vic[:8], 'After Attack (Evaluated on Victim)', '05_after_attack.png')\n\n"
            "print(f'Victim accuracy on CLEAN inputs    : {acc_clean*100:.2f}%')\n"
            "print(f'Victim accuracy on TRANSFERRED adv : {acc_attack*100:.2f}%')\n"
            "print(f'Transfer drop                      : {(acc_clean-acc_attack)*100:.2f}%')"
        ),
        new_code_cell(
            "fig, ax = plt.subplots(figsize=(6, 4))\n"
            "bars = ax.bar(['Clean', 'After Transfer Attack'],\n"
            "              [acc_clean*100, acc_attack*100], color=['#27ae60', '#e74c3c'],\n"
            "              edgecolor='black', width=0.4)\n"
            "ax.bar_label(bars, fmt='%.1f%%', padding=3)\n"
            "ax.set_ylim(0, 110); ax.set_ylabel('Victim Accuracy (%)')\n"
            "ax.set_title('Black-Box Transfer Attack\\n(Substitute -> Victim)', fontweight='bold')\n"
            "plt.tight_layout()\n"
            "plt.savefig('05_blackbox_substitute_results.png', dpi=150)\n"
            "plt.show()"
        ),
        new_markdown_cell(
            "## Summary\n\n"
            "The substitute model attack shows **gradient access is not required** for\n"
            "powerful adversarial examples. ~1000 API queries is enough to build a\n"
            "surrogate and dramatically reduce victim accuracy via transferability.\n\n"
            "**Defenses:** Query rate limiting, output perturbation, defensive distillation."
        ),
    ]
    return nb


# =============================================================================
# 6. EQUATION SOLVING ATTACKS
# =============================================================================
def make_nb6():
    nb = new_notebook()
    nb.cells = [
        new_markdown_cell(
            "# Demo 6 — Equation Solving Attacks\n"
            "## Algebraic Model Inversion on Linear Classifiers\n\n"
            "**Threat Model:** White-box, analytical\n\n"
            "**Goal:** Recover the model's weight vector **w** and bias **b** exactly\n"
            "by treating the model as a **system of linear equations**, then craft\n"
            "the *minimum-norm* adversarial perturbation analytically.\n\n"
            "**Formula for linear model:** `f(x) = w·x + b`\n\n"
            "If the attacker knows `n+1` query scores (where `n` = feature dimension):\n\n"
            "$\\mathbf{X}\\mathbf{w} = \\mathbf{s}$ → solved via least-squares"
        ),
        new_code_cell("# Install\n# !pip install numpy scikit-learn matplotlib"),
        new_code_cell(
            "import numpy as np\nimport matplotlib.pyplot as plt\n"
            "from sklearn.linear_model import LogisticRegression\n"
            "from sklearn.datasets import make_classification\n"
            "from sklearn.preprocessing import StandardScaler\n"
            "from sklearn.model_selection import train_test_split\n"
            "np.random.seed(42)"
        ),
        new_markdown_cell(
            "## Step 1 — Train a Victim Linear Classifier\n\n"
            "We use a **2D dataset** for easy boundary visualisation."
        ),
        new_code_cell(
            "X, y = make_classification(n_samples=500, n_features=2, n_informative=2,\n"
            "                            n_redundant=0, n_clusters_per_class=1, random_state=42)\n"
            "X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)\n"
            "scaler = StandardScaler()\n"
            "X_train_s = scaler.fit_transform(X_train)\n"
            "X_test_s  = scaler.transform(X_test)\n"
            "victim = LogisticRegression(random_state=42, max_iter=1000)\n"
            "victim.fit(X_train_s, y_train)\n"
            "w_true = victim.coef_[0]; b_true = victim.intercept_[0]\n"
            "print(f'Victim accuracy: {victim.score(X_test_s, y_test)*100:.1f}%')\n"
            "print(f'True weights: w={w_true}, b={b_true:.4f}')"
        ),
        new_markdown_cell(
            "## Step 2 — Equation Solving: Recover w and b\n\n"
            "The attacker sends `n+1` probe inputs to the API and receives **raw logit scores**.\n"
            "They set up the system $Aw = s$ and solve: `w = A^{-1} s`."
        ),
        new_code_cell(
            "n_features   = X_train_s.shape[1]\n"
            "probe_inputs = np.eye(n_features+1)[:n_features+1, :n_features]\n"
            "probe_inputs = probe_inputs + np.random.randn(*probe_inputs.shape) * 0.01\n"
            "probe_scores = victim.decision_function(probe_inputs)\n"
            "A = np.hstack([probe_inputs, np.ones((len(probe_inputs), 1))])\n"
            "params, _, _, _ = np.linalg.lstsq(A, probe_scores, rcond=None)\n"
            "w_recovered = params[:n_features]; b_recovered = params[-1]\n"
            "print(f'True      : w={w_true}, b={b_true:.4f}')\n"
            "print(f'Recovered : w={w_recovered}, b={b_recovered:.4f}')\n"
            "print(f'L2 error  : {np.linalg.norm(w_true - w_recovered):.8f}')"
        ),
        new_markdown_cell(
            "## Step 3 — Craft Minimal Adversarial Examples Analytically\n\n"
            "Using recovered weights, compute the **minimum L2 perturbation** to cross\n"
            "the decision boundary:\n\n"
            "$\\delta^* = -\\frac{\\mathbf{w}^T\\mathbf{x} + b}{\\|\\mathbf{w}\\|^2} \\cdot \\mathbf{w}$"
        ),
        new_code_cell(
            "def craft_adversarial(x, w, b):\n"
            "    margin = (w @ x + b) / (np.linalg.norm(w) ** 2)\n"
            "    return x - margin * w\n\n"
            "original, adv_examples = [], []\n"
            "for xi, yi in zip(X_test_s, y_test):\n"
            "    if yi == 1:\n"
            "        original.append(xi)\n"
            "        adv_examples.append(craft_adversarial(xi, w_recovered, b_recovered))\n"
            "original     = np.array(original)\n"
            "adv_examples = np.array(adv_examples)\n"
            "success_rate = (victim.predict(adv_examples) == 0).mean()\n"
            "print(f'Attack success rate (flipped to class 0): {success_rate*100:.1f}%')\n"
            "print(f'Mean L2 perturbation: {np.linalg.norm(adv_examples - original, axis=1).mean():.4f}')"
        ),
        new_code_cell(
            "def plot_boundary(ax, model, X, y, title):\n"
            "    h = 0.05\n"
            "    x_min, x_max = X[:, 0].min()-1, X[:, 0].max()+1\n"
            "    y_min, y_max = X[:, 1].min()-1, X[:, 1].max()+1\n"
            "    xx, yy = np.meshgrid(np.arange(x_min, x_max, h), np.arange(y_min, y_max, h))\n"
            "    Z = model.predict(np.c_[xx.ravel(), yy.ravel()]).reshape(xx.shape)\n"
            "    ax.contourf(xx, yy, Z, alpha=0.2, cmap='RdBu')\n"
            "    ax.scatter(X[y==0, 0], X[y==0, 1], c='blue', s=20, label='Class 0')\n"
            "    ax.scatter(X[y==1, 0], X[y==1, 1], c='red',  s=20, label='Class 1')\n"
            "    ax.set_title(title, fontweight='bold'); ax.legend(fontsize=8)\n\n"
            "fig_before, ax_before = plt.subplots(figsize=(6, 5))\n"
            "plot_boundary(ax_before, victim, X_test_s, y_test, 'Before Attack: Clean Boundary')\n"
            "plt.savefig('06_before_attack.png', dpi=150)\n"
            "plt.close()\n\n"
            "fig_mid, ax_mid = plt.subplots(figsize=(6, 5))\n"
            "dummy_model = LogisticRegression(); dummy_model.coef_ = np.array([w_recovered]); dummy_model.intercept_ = np.array([b_recovered]); dummy_model.classes_ = np.array([0, 1])\n"
            "plot_boundary(ax_mid, dummy_model, X_test_s, y_test, 'In Process: Recovered Boundary')\n"
            "plt.savefig('06_in_process_attack.png', dpi=150)\n"
            "plt.close()\n\n"
            "fig, axes = plt.subplots(1, 2, figsize=(12, 5))\n"
            "plot_boundary(axes[0], victim, X_test_s, y_test, 'Clean Decision Boundary')\n"
            "plot_boundary(axes[1], victim, X_test_s, y_test, 'Equation Solving Adversarial Examples')\n"
            "for o, a in zip(original[:20], adv_examples[:20]):\n"
            "    axes[1].annotate('', xy=a, xytext=o, arrowprops=dict(arrowstyle='->', color='orange', lw=1.5))\n"
            "axes[1].scatter(adv_examples[:,0], adv_examples[:,1], c='orange', marker='x', s=60,\n"
            "                zorder=5, label='Adversarial')\n"
            "axes[1].legend(fontsize=8)\n"
            "plt.suptitle('Equation Solving Attack', fontsize=13, fontweight='bold')\n"
            "plt.tight_layout()\n"
            "plt.savefig('06_after_attack.png', dpi=150)\n"
            "plt.show()"
        ),
        new_markdown_cell(
            "## Summary\n\n"
            "Equation solving attacks show that **linear models can be fully compromised**\n"
            "with just `n+1` queries. Once `w` and `b` are known:\n"
            "- The attacker computes the **exact optimal adversarial perturbation** analytically\n"
            "- Near-100% success rate with **minimal perturbation**\n\n"
            "**Defenses:** Output rounding, differential privacy noise, score perturbation."
        ),
    ]
    return nb


# =============================================================================
# 7. MODEL RECOVERY
# =============================================================================
def make_nb7():
    nb = new_notebook()
    nb.cells = [
        new_markdown_cell(
            "# Demo 7 — Model Recovery / Model Stealing\n"
            "## KnockoffNets: Cloning a Black-Box Classifier\n\n"
            "**Threat Model:** Black-box query-based — attacker sees only **hard labels**\n"
            "(the most restrictive API setting)\n\n"
            "**Goal:** Train a **functionally equivalent clone** of a victim model,\n"
            "violating the owner's **IP and monetisation** (e.g., MLaaS APIs).\n\n"
            "**Based on:** Knockoff Nets (Orekondy et al., CVPR 2019)"
        ),
        new_code_cell("# Install\n# !pip install torch torchvision matplotlib numpy"),
        new_code_cell(
            "import torch, torch.nn as nn, torch.optim as optim\n"
            "import torchvision, torchvision.transforms as transforms\n"
            "import matplotlib.pyplot as plt\nimport numpy as np\n"
            "torch.manual_seed(42)\n"
            "DEVICE    = torch.device('cuda' if torch.cuda.is_available() else 'cpu')\n"
            "transform = transforms.Compose([transforms.ToTensor()])\n"
            "full_train_f = torchvision.datasets.MNIST('./data', train=True, download=True, transform=transform)\n"
            "test_set_f   = torchvision.datasets.MNIST('./data', train=False, download=True, transform=transform)\n"
            "full_train = torch.utils.data.Subset(full_train_f, range(1000))\n"
            "test_set   = torch.utils.data.Subset(test_set_f, range(200))\n"
            "test_loader = torch.utils.data.DataLoader(test_set, batch_size=256, shuffle=False)"
        ),
        new_markdown_cell("## Step 1 — Train the Victim (Proprietary) Model"),
        new_code_cell(
            "class VictimModel(nn.Module):\n"
            "    def __init__(self):\n"
            "        super().__init__()\n"
            "        self.net = nn.Sequential(\n"
            "            nn.Conv2d(1,32,3,padding=1), nn.ReLU(), nn.MaxPool2d(2),\n"
            "            nn.Conv2d(32,64,3,padding=1), nn.ReLU(), nn.MaxPool2d(2),\n"
            "            nn.Flatten(), nn.Linear(64*7*7,256), nn.ReLU(), nn.Linear(256,10)\n"
            "        )\n"
            "    def forward(self, x): return self.net(x)\n\n"
            "victim = VictimModel().to(DEVICE)\n"
            "train_loader = torch.utils.data.DataLoader(full_train, batch_size=128, shuffle=True)\n"
            "opt = optim.Adam(victim.parameters(), lr=1e-3); fn = nn.CrossEntropyLoss()\n"
            "print('Training victim model (kept secret from attacker)...')\n"
            "for epoch in range(3):\n"
            "    victim.train()\n"
            "    for X, y in train_loader:\n"
            "        X, y = X.to(DEVICE), y.to(DEVICE)\n"
            "        opt.zero_grad(); fn(victim(X), y).backward(); opt.step()\n"
            "victim.eval()\n"
            "correct = sum((victim(X.to(DEVICE)).argmax(1)==y.to(DEVICE)).sum().item()\n"
            "               for X, y in test_loader)\n"
            "victim_acc = correct / len(test_set)\n"
            "print(f'Victim accuracy: {victim_acc*100:.2f}%')"
        ),
        new_markdown_cell(
            "## Step 2 — Model Stealing with Different Query Budgets\n\n"
            "We sweep different **query budgets** to show how clone fidelity grows\n"
            "as the attacker makes more API calls."
        ),
        new_code_cell(
            "class CloneModel(nn.Module):\n"
            "    def __init__(self):\n"
            "        super().__init__()\n"
            "        self.net = nn.Sequential(\n"
            "            nn.Conv2d(1,16,3,padding=1), nn.ReLU(), nn.MaxPool2d(2),\n"
            "            nn.Flatten(), nn.Linear(16*14*14,128), nn.ReLU(), nn.Linear(128,10)\n"
            "        )\n"
            "    def forward(self, x): return self.net(x)\n\n"
            "def steal_model(query_budget):\n"
            "    indices = np.random.choice(len(full_train), query_budget, replace=False)\n"
            "    query_imgs = torch.stack([full_train[i][0] for i in indices])\n"
            "    victim.eval()\n"
            "    with torch.no_grad():\n"
            "        stolen_labels = victim(query_imgs.to(DEVICE)).argmax(1).cpu()\n"
            "    clone_ds     = torch.utils.data.TensorDataset(query_imgs, stolen_labels)\n"
            "    clone_loader = torch.utils.data.DataLoader(clone_ds, batch_size=64, shuffle=True)\n"
            "    clone    = CloneModel().to(DEVICE)\n"
            "    opt_c    = optim.Adam(clone.parameters(), lr=1e-3)\n"
            "    for _ in range(10):\n"
            "        clone.train()\n"
            "        for Xb, yb in clone_loader:\n"
            "            Xb, yb = Xb.to(DEVICE), yb.to(DEVICE)\n"
            "            opt_c.zero_grad(); fn(clone(Xb), yb).backward(); opt_c.step()\n"
            "    clone.eval()\n"
            "    agree = 0\n"
            "    with torch.no_grad():\n"
            "        for X, _ in test_loader:\n"
            "            X = X.to(DEVICE)\n"
            "            agree += (clone(X).argmax(1) == victim(X).argmax(1)).sum().item()\n"
            "    fidelity = agree / len(test_set)\n"
            "    acc = sum((clone(X.to(DEVICE)).argmax(1)==y.to(DEVICE)).sum().item()\n"
            "               for X, y in test_loader) / len(test_set)\n"
            "    return fidelity, acc\n\n"
            "QUERY_BUDGETS  = [200, 500, 1000, 3000, 5000]\n"
            "clone_results  = {}\n"
            "print(f'{'Budget':>8}  {'Clone Acc':>10}  {'Fidelity':>10}')\n"
            "print('-' * 35)\n"
            "for budget in QUERY_BUDGETS:\n"
            "    fidelity, acc = steal_model(budget)\n"
            "    clone_results[budget] = (acc, fidelity)\n"
            "    print(f'{budget:>8}  {acc*100:>9.2f}%  {fidelity*100:>9.2f}%')"
        ),
        new_code_cell(
            "fig, axes = plt.subplots(1, 2, figsize=(12, 5))\n"
            "budgets    = list(clone_results.keys())\n"
            "accs       = [clone_results[b][0]*100 for b in budgets]\n"
            "fidelities = [clone_results[b][1]*100 for b in budgets]\n\n"
            "axes[0].plot(budgets, accs, 'o-', color='#e74c3c', linewidth=2, markersize=7, label='Clone Acc')\n"
            "axes[0].axhline(victim_acc*100, linestyle='--', color='#27ae60', linewidth=1.5, label='Victim Acc')\n"
            "axes[0].set_xlabel('Query Budget (# API calls)')\n"
            "axes[0].set_ylabel('Test Accuracy (%)')\n"
            "axes[0].set_title('Clone Accuracy vs Query Budget', fontweight='bold')\n"
            "axes[0].legend(); axes[0].grid(alpha=0.3)\n\n"
            "axes[1].plot(budgets, fidelities, 's-', color='#3498db', linewidth=2, markersize=7)\n"
            "axes[1].set_xlabel('Query Budget (# API calls)')\n"
            "axes[1].set_ylabel('Fidelity (% agreement with victim)')\n"
            "axes[1].set_title('Clone Fidelity vs Query Budget', fontweight='bold')\n"
            "axes[1].grid(alpha=0.3)\n\n"
            "plt.suptitle('Model Recovery / Stealing (KnockoffNets-style)', fontsize=13, fontweight='bold')\n"
            "plt.tight_layout()\n"
            "plt.savefig('07_after_attack.png', dpi=150)\n"
            "plt.show()\n\n"
            "fig_before, ax_before = plt.subplots(figsize=(5, 3))\n"
            "ax_before.bar(['Victim Model'], [victim_acc*100], color='#27ae60')\n"
            "ax_before.set_ylim(0, 110); ax_before.set_title('Before Attack: Victim Model Accuracy')\n"
            "plt.savefig('07_before_attack.png', dpi=150)\n"
            "plt.close()\n\n"
            "fig_in, ax_in = plt.subplots(figsize=(5, 3))\n"
            "ax_in.bar(['Clone (1000 queries)'], [clone_results[1000][1]*100], color='#3498db')\n"
            "ax_in.set_ylim(0, 110); ax_in.set_title('In Process: Clone Fidelity')\n"
            "plt.savefig('07_in_process_attack.png', dpi=150)\n"
            "plt.close()"
        ),
        new_markdown_cell(
            "## Summary\n\n"
            "Model stealing shows that **ML models can be replicated without training data**.\n"
            "With ~5000 queries, a clone achieves high fidelity, stealing the victim's IP.\n\n"
            "**Real-world impact:** Bypasses API paywalls, enables further white-box attacks.\n\n"
            "**Defenses:** Query rate-limiting, output perturbation, prediction watermarking,\n"
            "anomalous query pattern detection."
        ),
    ]
    return nb


# =============================================================================
# 8. EXPLANATION-GUIDED ATTACKS
# =============================================================================
def make_nb8():
    nb = new_notebook()
    nb.cells = [
        new_markdown_cell(
            "# Demo 8 — Explanation-Guided Attacks\n"
            "## Saliency-Targeted Adversarial Examples using Captum\n\n"
            "**Core Idea:** Instead of perturbing all pixels uniformly (like FGSM),\n"
            "first **explain** the model (identify which pixels matter most),\n"
            "then **target only those pixels** with perturbations.\n\n"
            "**Result:** More efficient, more transferable, more stealthy attacks.\n\n"
            "**Library:** `captum` (PyTorch Model Interpretability)"
        ),
        new_code_cell("# Install\n# !pip install torch torchvision captum matplotlib numpy"),
        new_code_cell(
            "import torch, torch.nn as nn, torch.optim as optim\n"
            "import torchvision, torchvision.transforms as transforms\n"
            "import matplotlib.pyplot as plt\nimport numpy as np\n"
            "from captum.attr import Saliency, IntegratedGradients, GuidedBackprop\n"
            "torch.manual_seed(42)\n"
            "DEVICE    = torch.device('cuda' if torch.cuda.is_available() else 'cpu')\n"
            "transform = transforms.Compose([transforms.ToTensor()])\n"
            "train_set_f = torchvision.datasets.MNIST('./data', train=True, download=True, transform=transform)\n"
            "test_set_f  = torchvision.datasets.MNIST('./data', train=False, download=True, transform=transform)\n"
            "train_set = torch.utils.data.Subset(train_set_f, range(1000))\n"
            "test_set  = torch.utils.data.Subset(test_set_f, range(200))\n"
            "test_loader = torch.utils.data.DataLoader(test_set, batch_size=256, shuffle=False)\n"
            "print('Captum + data ready.')"
        ),
        new_markdown_cell("## Step 1 — Train the Victim Model"),
        new_code_cell(
            "class CNN(nn.Module):\n"
            "    def __init__(self):\n"
            "        super().__init__()\n"
            "        self.features   = nn.Sequential(\n"
            "            nn.Conv2d(1,32,3,padding=1), nn.ReLU(), nn.MaxPool2d(2),\n"
            "            nn.Conv2d(32,64,3,padding=1), nn.ReLU(), nn.MaxPool2d(2)\n"
            "        )\n"
            "        self.classifier = nn.Sequential(\n"
            "            nn.Flatten(), nn.Linear(64*7*7,128), nn.ReLU(), nn.Linear(128,10)\n"
            "        )\n"
            "    def forward(self, x): return self.classifier(self.features(x))\n\n"
            "model = CNN().to(DEVICE)\n"
            "opt   = optim.Adam(model.parameters(), lr=1e-3)\n"
            "fn    = nn.CrossEntropyLoss()\n"
            "train_loader = torch.utils.data.DataLoader(train_set, batch_size=128, shuffle=True)\n"
            "for epoch in range(3):\n"
            "    model.train()\n"
            "    for X, y in train_loader:\n"
            "        X, y = X.to(DEVICE), y.to(DEVICE)\n"
            "        opt.zero_grad(); fn(model(X), y).backward(); opt.step()\n"
            "model.eval()\n"
            "correct = sum((model(X.to(DEVICE)).argmax(1)==y.to(DEVICE)).sum().item()\n"
            "               for X, y in test_loader)\n"
            "print(f'Model accuracy: {correct/len(test_set)*100:.2f}%')"
        ),
        new_markdown_cell(
            "## Step 2 — Generate Explanation Maps\n\n"
            "| Method | Description |\n"
            "|--------|-------------|\n"
            "| **Saliency** | Gradient magnitude w.r.t. input pixels |\n"
            "| **Integrated Gradients** | Average gradient along path from baseline to input |\n"
            "| **Guided Backprop** | Backprop with positive-only rule for sharper maps |"
        ),
        new_code_cell(
            "saliency_attr = Saliency(model)\n"
            "ig_attr       = IntegratedGradients(model)\n"
            "guided_attr   = GuidedBackprop(model)\n\n"
            "X_sample, y_sample = next(iter(torch.utils.data.DataLoader(test_set, batch_size=6)))\n"
            "X_sample = X_sample.to(DEVICE).requires_grad_(True)\n"
            "y_sample = y_sample.to(DEVICE)\n"
            "preds    = model(X_sample).argmax(1)\n\n"
            "sal_maps = saliency_attr.attribute(X_sample, target=preds).abs()\n"
            "ig_maps  = ig_attr.attribute(X_sample, target=preds, n_steps=50).abs()\n"
            "gb_maps  = guided_attr.attribute(X_sample, target=preds).abs()\n"
            "print('Attribution maps computed.')"
        ),
        new_code_cell(
            "fig, axes = plt.subplots(4, 6, figsize=(16, 10))\n"
            "fig.suptitle('Explanation Maps: What Does the Model Look At?', fontsize=14, fontweight='bold')\n"
            "for i in range(6):\n"
            "    imgs_list = [\n"
            "        X_sample[i].squeeze().cpu().detach().numpy(),\n"
            "        sal_maps[i].squeeze().cpu().detach().numpy(),\n"
            "        ig_maps[i].squeeze().cpu().detach().numpy(),\n"
            "        gb_maps[i].squeeze().cpu().detach().numpy()\n"
            "    ]\n"
            "    cmaps = ['gray', 'hot', 'hot', 'hot']\n"
            "    titles= [f'Input\\nTrue:{y_sample[i].item()} Pred:{preds[i].item()}',\n"
            "             'Saliency', 'Integr. Grad.', 'Guided BP']\n"
            "    for row, (img, cmap, title) in enumerate(zip(imgs_list, cmaps, titles)):\n"
            "        axes[row, i].imshow(img, cmap=cmap)\n"
            "        axes[row, i].set_title(title, fontsize=8)\n"
            "        axes[row, i].axis('off')\n"
            "plt.tight_layout()\n"
            "plt.savefig('08a_explanation_maps.png', dpi=150)\n"
            "plt.show()"
        ),
        new_markdown_cell(
            "## Step 3 — Explanation-Guided Attack\n\n"
            "**Key idea:** Compute a saliency mask → keep only the top-K% most important\n"
            "pixels → apply FGSM perturbation **only to those pixels**.\n\n"
            "Compare against standard FGSM (all pixels) to show fewer pixels are needed."
        ),
        new_code_cell(
            "def fgsm(model, X, y, epsilon):\n"
            "    Xa = X.clone().detach().requires_grad_(True)\n"
            "    fn(model(Xa), y).backward()\n"
            "    return (X + epsilon * Xa.grad.sign()).clamp(0, 1).detach()\n\n"
            "def explanation_guided_attack(model, X, y, epsilon, top_k_pct=0.15):\n"
            "    Xa = X.clone().detach().requires_grad_(True)\n"
            "    fn(model(Xa), y).backward()\n"
            "    grad_sign = Xa.grad.sign().detach()\n"
            "    sal       = Xa.grad.abs().detach()\n"
            "    threshold = torch.quantile(sal.view(sal.shape[0], -1), 1-top_k_pct, dim=1)\n"
            "    mask      = (sal >= threshold.view(-1,1,1,1)).float()\n"
            "    return (X + epsilon * grad_sign * mask).clamp(0, 1).detach()\n\n"
            "epsilon    = 0.2\n"
            "top_k_vals = [0.05, 0.10, 0.15, 0.25, 0.50, 1.00]\n"
            "results    = {}\n"
            "model.eval()\n"
            "for k in top_k_vals:\n"
            "    correct = 0; total = 0\n"
            "    for X, y in test_loader:\n"
            "        X, y = X.to(DEVICE), y.to(DEVICE)\n"
            "        X_adv = fgsm(model, X, y, epsilon) if k == 1.00 else explanation_guided_attack(model, X, y, epsilon, k)\n"
            "        correct += (model(X_adv).argmax(1) == y).sum().item()\n"
            "        total   += y.size(0)\n"
            "    results[k] = correct / total\n\n"
            "print(f'{'Top-K %':>8}  {'Accuracy After Attack':>22}')\n"
            "print('-' * 35)\n"
            "for k, acc in results.items():\n"
            "    print(f'{k*100:>7.1f}%  {acc*100:>21.2f}%')"
        ),
        new_code_cell(
            "fig, ax = plt.subplots(figsize=(8, 5))\n"
            "ks   = [k*100 for k in top_k_vals]\n"
            "accs = [results[k]*100 for k in top_k_vals]\n"
            "ax.plot(ks, accs, 'o-', color='#e74c3c', linewidth=2, markersize=8)\n"
            "ax.set_xlabel('% of Pixels Perturbed')\n"
            "ax.set_ylabel('Model Accuracy (%) — lower = stronger attack')\n"
            "ax.set_title('Explanation-Guided Attack Efficiency\\n(rightmost = standard FGSM all-pixel)', fontweight='bold')\n"
            "ax.annotate('Standard FGSM\\n(all pixels)', xy=(100, accs[-1]),\n"
            "             xytext=(70, accs[-1]+5), arrowprops=dict(arrowstyle='->'))\n"
            "ax.grid(alpha=0.3)\n"
            "plt.tight_layout()\n"
            "plt.savefig('08b_explanation_attack.png', dpi=150)\n"
            "plt.show()"
        ),
        new_markdown_cell(
            "## Summary\n\n"
            "Explanation-guided attacks show **model interpretability tools can be weaponised**.\n"
            "By targeting only important pixels:\n"
            "- The attack is **more stealthy** (fewer pixels changed)\n"
            "- Perturbations are **semantically meaningful**\n"
            "- Transfer attacks are **more robust** across architectures\n\n"
            "**Defenses:** Hiding explanation maps (hard labels only), randomising the explanation\n"
            "method, adversarial training on explanation-targeted examples."
        ),
    ]
    return nb


# =============================================================================
# 9. GRADIENT ESTIMATION ATTACKS
# =============================================================================
def make_nb9():
    nb = new_notebook()
    nb.cells = [
        new_markdown_cell(
            "# Demo 9 — Gradient Estimation Attacks\n"
            "## ZOO (Finite Differences) & Square Attack\n\n"
            "**Threat Model:** Score-based black-box — attacker sees **confidence scores**,\n"
            "no gradients, no architecture knowledge\n\n"
            "| Method | Strategy |\n"
            "|--------|----------|\n"
            "| **ZOO** | Estimate gradient via finite differences: query `f(x+h*e_i)` and `f(x-h*e_i)` |\n"
            "| **Square Attack** | Random search in L-inf ball — no gradient estimation needed |\n\n"
            "**Why this matters:** Only a small number of API queries is required —\n"
            "no mathematical knowledge of the model needed."
        ),
        new_code_cell("# Install\n# !pip install torch torchvision matplotlib numpy"),
        new_code_cell(
            "import torch, torch.nn as nn, torch.optim as optim\n"
            "import torchvision, torchvision.transforms as transforms\n"
            "import matplotlib.pyplot as plt\nimport numpy as np\n"
            "torch.manual_seed(42); np.random.seed(42)\n"
            "DEVICE    = torch.device('cuda' if torch.cuda.is_available() else 'cpu')\n"
            "transform = transforms.Compose([transforms.ToTensor()])\n"
            "train_set_f = torchvision.datasets.MNIST('./data', train=True, download=True, transform=transform)\n"
            "test_set_f  = torchvision.datasets.MNIST('./data', train=False, download=True, transform=transform)\n"
            "train_set = torch.utils.data.Subset(train_set_f, range(1000))\n"
            "test_set  = torch.utils.data.Subset(test_set_f, range(200))\n"
            "test_loader = torch.utils.data.DataLoader(test_set, batch_size=256, shuffle=False)"
        ),
        new_markdown_cell("## Step 1 — Train the Victim Model"),
        new_code_cell(
            "class VictimCNN(nn.Module):\n"
            "    def __init__(self):\n"
            "        super().__init__()\n"
            "        self.net = nn.Sequential(\n"
            "            nn.Conv2d(1,32,3,padding=1), nn.ReLU(), nn.MaxPool2d(2),\n"
            "            nn.Conv2d(32,64,3,padding=1), nn.ReLU(), nn.MaxPool2d(2),\n"
            "            nn.Flatten(), nn.Linear(64*7*7,128), nn.ReLU(), nn.Linear(128,10)\n"
            "        )\n"
            "    def forward(self, x): return self.net(x)\n\n"
            "victim      = VictimCNN().to(DEVICE)\n"
            "train_loader = torch.utils.data.DataLoader(train_set, batch_size=128, shuffle=True)\n"
            "opt = optim.Adam(victim.parameters(), lr=1e-3); fn = nn.CrossEntropyLoss()\n"
            "for epoch in range(3):\n"
            "    victim.train()\n"
            "    for X, y in train_loader:\n"
            "        X, y = X.to(DEVICE), y.to(DEVICE)\n"
            "        opt.zero_grad(); fn(victim(X), y).backward(); opt.step()\n"
            "victim.eval()\n"
            "correct = sum((victim(X.to(DEVICE)).argmax(1)==y.to(DEVICE)).sum().item()\n"
            "               for X, y in test_loader)\n"
            "print(f'Victim accuracy: {correct/len(test_set)*100:.2f}%')"
        ),
        new_markdown_cell(
            "## Step 2 — ZOO: Finite Difference Gradient Estimation\n\n"
            "**Formula:**\n\n"
            "$\\hat{g}_i = \\frac{f(x + h \\cdot e_i)[y] - f(x - h \\cdot e_i)[y]}{2h}$\n\n"
            "This requires **2 x n_pixels queries per step**. We use random coordinate\n"
            "sampling (10% of pixels) to reduce the query budget."
        ),
        new_code_cell(
            "def zoo_attack(model, X, y, epsilon=0.2, n_steps=30, h=0.001, lr=0.01,\n"
            "               coord_sample_rate=0.1):\n"
            "    \"\"\"ZOO finite-difference attack on a single image.\"\"\"\n"
            "    n_pixels   = X.numel()\n"
            "    n_coords   = max(1, int(n_pixels * coord_sample_rate))\n"
            "    x_adv      = X.clone().flatten().to(DEVICE)\n"
            "    y_true     = y.to(DEVICE)\n"
            "    total_queries = 0\n"
            "    for step in range(n_steps):\n"
            "        coords    = torch.randperm(n_pixels)[:n_coords]\n"
            "        grad_est  = torch.zeros_like(x_adv)\n"
            "        for c in coords:\n"
            "            x_plus  = x_adv.clone(); x_plus[c]  += h\n"
            "            x_minus = x_adv.clone(); x_minus[c] -= h\n"
            "            with torch.no_grad():\n"
            "                s_plus  = torch.softmax(model(x_plus.view(1,1,28,28)),  dim=1)[0, y_true]\n"
            "                s_minus = torch.softmax(model(x_minus.view(1,1,28,28)), dim=1)[0, y_true]\n"
            "            grad_est[c] = (s_plus - s_minus) / (2 * h)\n"
            "            total_queries += 2\n"
            "        x_adv = x_adv - lr * grad_est.sign()\n"
            "        delta = (x_adv - X.flatten()).clamp(-epsilon, epsilon)\n"
            "        x_adv = (X.flatten() + delta).clamp(0, 1)\n"
            "    return x_adv.view(1, 1, 28, 28), total_queries"
        ),
        new_markdown_cell(
            "## Step 3 — Square Attack: Pure Random Search\n\n"
            "Square Attack randomly samples **square-shaped perturbations** and keeps only\n"
            "those that reduce model confidence. No gradient estimation at all."
        ),
        new_code_cell(
            "def square_attack(model, X, y, epsilon=0.2, n_queries=200):\n"
            "    \"\"\"Simplified Square Attack: random square perturbations in L-inf ball.\"\"\"\n"
            "    x_adv  = X.clone().to(DEVICE)\n"
            "    y_true = y.to(DEVICE)\n"
            "    _, _, H, W = X.shape\n"
            "    with torch.no_grad():\n"
            "        best_conf = torch.softmax(model(x_adv), dim=1)[0, y_true].item()\n"
            "    for q in range(n_queries):\n"
            "        size = max(1, np.random.randint(2, max(3, int(H * 0.3))))\n"
            "        row  = np.random.randint(0, H - size + 1)\n"
            "        col  = np.random.randint(0, W - size + 1)\n"
            "        sign = np.random.choice([-1, 1])\n"
            "        x_candidate = x_adv.clone()\n"
            "        x_candidate[:, :, row:row+size, col:col+size] += sign * epsilon\n"
            "        delta       = (x_candidate - X.to(DEVICE)).clamp(-epsilon, epsilon)\n"
            "        x_candidate = (X.to(DEVICE) + delta).clamp(0, 1)\n"
            "        with torch.no_grad():\n"
            "            conf = torch.softmax(model(x_candidate), dim=1)[0, y_true].item()\n"
            "        if conf < best_conf:\n"
            "            x_adv = x_candidate; best_conf = conf\n"
            "    return x_adv"
        ),
        new_markdown_cell("## Step 4 — Evaluate Both Attacks"),
        new_code_cell(
            "n_eval  = 25\n"
            "epsilon = 0.2\n"
            "X_eval  = torch.stack([test_set[i][0] for i in range(n_eval)])\n"
            "y_eval  = torch.tensor([test_set[i][1] for i in range(n_eval)])\n\n"
            "# ZOO\n"
            "zoo_correct = 0; total_zoo_queries = 0; zoo_advs = []\n"
            "for i in range(n_eval):\n"
            "    xi, yi  = X_eval[i:i+1], y_eval[i:i+1]\n"
            "    x_adv, q = zoo_attack(victim, xi, yi, epsilon=epsilon, n_steps=30)\n"
            "    with torch.no_grad():\n"
            "        pred = victim(x_adv).argmax(1).item()\n"
            "    if pred == yi.item(): zoo_correct += 1\n"
            "    total_zoo_queries += q; zoo_advs.append(x_adv.cpu())\n"
            "zoo_acc = zoo_correct / n_eval\n"
            "avg_zoo_queries = total_zoo_queries / n_eval\n\n"
            "# Square\n"
            "sq_correct = 0; sq_advs = []\n"
            "for i in range(n_eval):\n"
            "    xi, yi  = X_eval[i:i+1], y_eval[i:i+1]\n"
            "    x_adv   = square_attack(victim, xi, yi, epsilon=epsilon, n_queries=200)\n"
            "    with torch.no_grad():\n"
            "        pred = victim(x_adv).argmax(1).item()\n"
            "    if pred == yi.item(): sq_correct += 1\n"
            "    sq_advs.append(x_adv.cpu())\n"
            "sq_acc = sq_correct / n_eval\n\n"
            "print(f'{'Attack':>15}  {'Accuracy':>10}  {'Avg Queries':>13}')\n"
            "print('-' * 45)\n"
            "print(f'{'ZOO':>15}  {zoo_acc*100:>9.1f}%  {avg_zoo_queries:>13.0f}')\n"
            "print(f'{'Square Attack':>15}  {sq_acc*100:>9.1f}%  {'200':>13}')"
        ),
        new_code_cell(
            "fig, axes = plt.subplots(3, 8, figsize=(16, 7))\n"
            "fig.suptitle(f'Gradient Estimation Attacks (epsilon={epsilon})', fontsize=13, fontweight='bold')\n"
            "n_show = 8\n"
            "with torch.no_grad():\n"
            "    clean_preds = victim(X_eval[:n_show].to(DEVICE)).argmax(1).cpu()\n"
            "    zoo_preds   = victim(torch.cat(zoo_advs[:n_show]).to(DEVICE)).argmax(1).cpu()\n"
            "    sq_preds    = victim(torch.cat(sq_advs[:n_show]).to(DEVICE)).argmax(1).cpu()\n"
            "true = y_eval[:n_show]\n"
            "rows = [\n"
            "    (X_eval[:n_show], clean_preds, 'Clean'),\n"
            "    (torch.cat(zoo_advs[:n_show]), zoo_preds, f'ZOO (~{avg_zoo_queries:.0f} queries)'),\n"
            "    (torch.cat(sq_advs[:n_show]), sq_preds, 'Square (200 queries)'),\n"
            "]\n"
            "for row_idx, (imgs, preds, row_label) in enumerate(rows):\n"
            "    for col in range(n_show):\n"
            "        axes[row_idx, col].imshow(imgs[col].squeeze(), cmap='gray')\n"
            "        c = 'green' if preds[col]==true[col] else 'red'\n"
            "        axes[row_idx, col].set_title(f'P:{preds[col].item()}', color=c, fontsize=9)\n"
            "        axes[row_idx, col].axis('off')\n"
            "    axes[row_idx, 0].set_ylabel(row_label, fontsize=9, rotation=0, labelpad=80, va='center')\n"
            "plt.tight_layout()\n"
            "plt.savefig('09_after_attack.png', dpi=150)\n"
            "plt.show()\n\n"
            "def save_row(imgs, title, filename):\n"
            "    fig, axes = plt.subplots(1, 8, figsize=(16, 2))\n"
            "    for col in range(8):\n"
            "        axes[col].imshow(imgs[col].squeeze().cpu(), cmap='gray'); axes[col].axis('off')\n"
            "    fig.suptitle(title, fontweight='bold')\n"
            "    plt.tight_layout()\n"
            "    plt.savefig(filename, dpi=150)\n"
            "    plt.close()\n\n"
            "save_row(X_eval[:8], 'Before Attack: Clean', '09_before_attack.png')\n"
            "zoo_mid = [zoo_attack(victim, X_eval[i:i+1], y_eval[i:i+1], n_steps=10)[0] for i in range(8)]\n"
            "save_row(torch.cat(zoo_mid), 'In Process: ZOO (10 steps)', '09_in_process_attack.png')"
        ),
        new_markdown_cell(
            "## Summary\n\n"
            "| | ZOO | Square Attack |\n"
            "|---|---|---|\n"
            "| **Gradient access** | No | No |\n"
            "| **Score access** | Yes | Yes |\n"
            "| **Query efficiency** | Moderate | High |\n"
            "| **Strength** | Strong | Strong |\n\n"
            "**Key takeaway:** Even without gradient access, an attacker who can only see\n"
            "confidence scores can craft effective adversarial examples.\n"
            "This undermines 'security through obscurity' of black-box deployment.\n\n"
            "**Defenses:** Query rate-limiting, output quantisation, adversarial query detectors."
        ),
    ]
    return nb


# =============================================================================
# MAIN
# =============================================================================
if __name__ == "__main__":
    notebooks = [
        ("01_vision_gradient_attacks.ipynb",     make_nb1),
        ("02_text_attacks.ipynb",                make_nb2),
        ("03_graph_attacks.ipynb",               make_nb3),
        ("04_poisoning_attacks.ipynb",           make_nb4),
        ("05_blackbox_substitute_models.ipynb",  make_nb5),
        ("06_equation_solving_attacks.ipynb",    make_nb6),
        ("07_model_recovery.ipynb",              make_nb7),
        ("08_explanation_guided_attacks.ipynb",  make_nb8),
        ("09_gradient_estimation_attacks.ipynb", make_nb9),
    ]

    print(f"Generating {len(notebooks)} notebooks in: {OUTPUT_DIR}")
    print("=" * 60)
    for filename, builder_fn in notebooks:
        nb = builder_fn()
        save_notebook(nb, filename)

    print("=" * 60)
    print("All 9 notebooks generated successfully!")
    print()
    print("Next steps:")
    print("  pip install nbformat torch torchvision captum textattack deeprobust")
    print("  jupyter lab")
    print("  Open each notebook and run cells top to bottom.")
