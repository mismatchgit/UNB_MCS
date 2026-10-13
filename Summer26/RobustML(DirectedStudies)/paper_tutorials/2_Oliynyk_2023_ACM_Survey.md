# Reading Guide & Tutorial: Survey on Stealing Machine Learning Models and Defences

**Reference**: Daryna Oliynyk, Rudolf Mayer, and Andreas Rauber. *I Know What You Trained Last Summer: A Survey on Stealing Machine Learning Models and Defences.* ACM Computing Surveys (CSUR), 2023.

---

## 1. Core Summary & Taxonomy

This paper is a comprehensive literature review that structures the model stealing space. The authors establish a standardized terminology and categorize attacks based on two primary branches of objectives: **Stealing Exact Properties** and **Stealing Model Behaviour**.

### Taxonomy of Objectives
- **Stealing Exact Properties**:
  - *Learned Parameters*: Extracting the exact weights and bias values (e.g., Tramèr's linear systems).
  - *Training Hyperparameters*: Reconstructing regularization variables, batch sizes, or optimizer choices.
  - *Architecture*: Inferring layer counts, activations, or convolutional kernel sizes (often via side-channels).
- **Stealing Model Behaviour**:
  - *Same Level of Effectiveness (Accuracy Extraction)*: Creating a clone model $\hat{f}$ that achieves the same classification accuracy on the original task dataset:
    $$\text{Accuracy}(\hat{f}) \approx \text{Accuracy}(f)$$
  - *Prediction Consistency (Fidelity Extraction)*: Replicating output classifications on any dataset distribution, including target mistakes:
    $$\hat{f}(x) = f(x) \quad \forall x$$

---

## 2. Taxonomy Visual Mind Map

Below is the mind map summarizing the attack objectives structured in this survey:

![Oliynyk 2023 Taxonomy](./assets/oliynyk_2023_taxonomy.png)

---

## 3. Explanation Gaps & Deep-Dive Tutorials

This survey compiles existing literature, but it skips the execution details of the primary works it references, hoping the reader is familiar with the math of boundary probing and the theory of adversarial perturbation transfer.

---

### Gap A: Lowd-Meek Witness-Finding Algorithm

The survey cites **Lowd and Meek (2005)** as the earliest work in model extraction, which uses "Witness-Finding" to extract linear binary classifiers:
$$f(x) = \text{sgn}(w^T x + b)$$
However, the survey does not explain *how* this algorithm actually probes the space.

#### 1. Conceptual Search Setup
The attacker's goal is to find the weights vector $w \in \mathbb{R}^d$ and bias $b \in \mathbb{R}$ of a hyperplane boundary. The attacker starts with:
1. A positive instance $x^+$ where $f(x^+) = +1$.
2. A negative instance $x^-$ where $f(x^-) = -1$.

#### 2. The Step-by-Step Line Search Algorithm
To find the exact split boundaries, the attacker performs a numerical **binary search (bisection method)** along the line segment connecting $x^+$ and $x^-$:

1. **Parameterize the Line**: Define any point on the line segment using scaling parameter $\lambda \in [0, 1]$:
   $$x(\lambda) = \lambda x^+ + (1 - \lambda) x^-$$
2. **Binary Search for Boundary Crossing ($\lambda^*$)**:
   - Let $\lambda_{min} = 0$, $\lambda_{max} = 1$.
   - While $\lambda_{max} - \lambda_{min} > \epsilon$:
     - Compute midpoint $\lambda_{mid} = \frac{\lambda_{min} + \lambda_{max}}{2}$.
     - Query target API: $y_{mid} = f(x(\lambda_{mid}))$.
     - If $y_{mid} = +1$: set $\lambda_{max} = \lambda_{mid}$ (the boundary lies closer to the negative end).
     - If $y_{mid} = -1$: set $\lambda_{min} = \lambda_{mid}$ (the boundary lies closer to the positive end).
3. **Witness Node Extraction**: The crossing point $x^* = x(\lambda^*)$ lies exactly on the decision boundary hyperplane, satisfying:
   $$w^T x^* + b = 0$$
4. **Solving for Coefficients**:
   By discovering $d$ independent boundary points $x^*_1, x^*_2, \dots, x^*_d$ using line searches from different angles, we construct a system of linear equations:
   $$w^T (x^*_i - x^*_1) = 0 \quad \forall i \in \{2, \dots, d\}$$
   Solving this system yields the unit vector direction of $w$. Probing along that direction finds the bias boundary offset $b$.

---

### Gap B: Theoretical Foundations of Adversarial Transferability

Oliynyk et al. explain that behavior stealing attacks can be used as an intermediate step to "open" a model for white-box evasion attacks. If an adversarial example $x^*$ is generated to fool the stolen model $\hat{f}$, it will also fool the target model $f$. The survey does not explain the mathematical theory behind why this transfer happens.

#### 1. Goodfellow's Linear Hypothesis
Deep neural networks are often assumed to be complex, non-linear systems. However, Goodfellow et al. (2014) showed that neural network layers are trained to behave in a highly linear manner to avoid vanishing gradients during backpropagation.

For a linear model, the output response is:
$$y = w^T x$$
If we introduce a small adversarial perturbation $\eta$ to the input, the perturbed output becomes:
$$y^* = w^T (x + \eta) = w^T x + w^T \eta$$

The perturbation that maximizes this output change under an $L_\infty$-norm constraint ($\|\eta\|_\infty \le \epsilon$) is:
$$\eta = \epsilon \cdot \text{sgn}(w)$$
This creates a cumulative change of $\epsilon d \bar{w}$ where $d$ is the number of features. In high-dimensional spaces, even a tiny $\epsilon$ (invisible to humans) accumulates across dimensions to push the sample across the decision boundary.

#### 2. Why it Transfers to the Target Model
If the surrogate model $\hat{f}$ is trained on soft probability outputs from the target model $f$, its learned weights $\hat{w}$ align closely with the target's weights $w$. Thus, the sign vectors match:
$$\text{sgn}(\hat{w}) \approx \text{sgn}(w)$$

Because the sign vectors are nearly identical, the generated adversarial perturbation $\eta = \epsilon \cdot \text{sgn}(\hat{w})$ is also highly aligned with $w$. Consequently, the perturbation remains mathematically valid for the target model, pushing the query across $f$'s decision boundary as well.
