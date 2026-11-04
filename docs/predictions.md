# Predictions

## Setup

| Item | Value |
|---|---|
| Target error | 1e-5, the max `abs(numeric - exact)` over the grid at `t = T` |
| `N` (`--n-intervals`) | 2000, so `dx = 5e-4` and `dx^2 = 2.5e-7` |
| `L`, `alpha`, `T`, `mode` | 1, 1, 0.1, 1 |

The step count is `n = ceil(T / dt_req)`, so each scheme is given an `r_req` that produces the predicted `n`. All three schemes share `N = 2000`, so this compares them at a fixed grid, not at each scheme's own best `N`.

## Predictions

| Scheme | dt | steps `n` | `r = dt/dx^2` | predicted final max error | 
|---|---|---|---|---|
| Explicit (FTCS) | 1.25e-7 | 800,000 | 0.5 | 1.5e-7 | 
| Backward Euler | 5.47e-6 | about 18,290 (give or take 10) | about 21.9 | 1.0e-5 | 
| Crank-Nicolson | 1.818e-3 | 55 | about 7273 | 9.8e-6 | 

## How the numbers were derived

### Error model

With `lambda = pi^2` (`L = 1`, `mode = 1`) and `a = dt * lambda`, the sine mode stays a sine mode. The numerical solution is the exact one times a factor, and the error at `x = 0.5` (a grid point at `N = 2000`) is `u_exact * (factor - 1)`, with `u_exact = exp(-pi^2 * 0.1) = 0.3727`.

- **Space error (all schemes).** The discrete eigenvalue is `lambda * (1 - lambda * dx^2 / 12)`, so the numeric solution decays slowly. The error is `+0.3727 * (lambda^2 * T / 12) * dx^2 = +0.3025 * dx^2`. At `dx^2 = 2.5e-7` that is **+7.6e-8**, which is 130 times below the target, so the time error decides everything. The `0.3025` coefficient reproduces the Crank Nicolson `N = 100` figure of 3.025e-5 error from running`Main`.
- **Backward Euler time error.** `ln(factor) = n * (a^2/2 - a^3/3 + ...)`, so the error is about `+0.3727 * (lambda^2 * T / 2) * dt = +1.815 * dt`. The Backward Euler error of 1.0285e-4, measured by running `Main` at `N = 100`, `r = 0.4`, equals `1.815 * 4e-5 + 3.03e-5`, which checks the coefficient.
- **Crank-Nicolson time error.** `ln(factor) = -n * (a^3/12 + ...)`, so the error is about `-0.3727 * (lambda^3 * T / 12) * dt^2 = -2.986 * dt^2`. The `0.3025` coefficient reproduces the Crank-Nicolson error of 3.025e-5, measured by running `Main` at `N = 100`, `r = 0.4`.
- **Explicit time error.** This is `-1.815 * dt`, so the total is `-(1.815 * r - 0.3025) * dx^2`. At `r = 0.4` this gives `-0.4236 * dx^2`, matching the README's `N = 640` row (1.0341e-6 times 640^2 = 0.4236).

### Solving for dt

- **Backward Euler.** `1.815 * dt + 7.6e-8 = 1e-5` gives `dt = 5.467e-6`, so `n = T / dt = 18,291`.
- **Crank-Nicolson.** `2.986 * dt^2 - 7.6e-8 = 1e-5` gives `dt = 1.837e-3`, so `n = 54.4`. `n` must be an integer, so `n = 55` and `dt = 1.818e-3`. That gives an error of `2.986 * 3.306e-6 - 7.6e-8 = 9.8e-6`. `n = 54` would give 1.017e-5, which misses the target, so 55 is the minimum. Use `r_req` between 7273 and 7407 to get `n = 55`.
- **Explicit.** The explicit scheme cannot take a step as large as accuracy would allow. Stability needs `r <= 0.5`, so `dt <= 0.5 * dx^2 = 1.25e-7` and `n >= 800,000`. At that step the error is `(0.9075 - 0.3025) * 2.5e-7 = 1.5e-7` (numeric below exact). That is 66 times better than the target. The `r = 1/6` zero-error point is not available, because the step is already pinned at the stability limit. `ceil` may round the count to 800,001, which makes no practical difference.

## Cost to reach a target error at each scheme's own best N

The fixed-N predictions above compare the schemes on one grid. This section lets each scheme choose its own `N` for a target error `eps`, which is the comparison the plan calls "wall-clock time to reach the same accuracy". The cost per step is O(N) for all three schemes, because the Thomas algorithm is linear in the number of rows.

### Assumptions

- One dimension, a single smooth sine mode (`L = 1`, `T = 0.1`, `mode = 1`), and leading-order error formulas only.
- The space error `0.3025 * dx^2` and the time error are treated as adding in magnitude. Settings where they cancel are excluded. These are `r = 1/6` for the explicit scheme and `dt` of about `0.32 * dx` for Crank-Nicolson. They work for this one mode and would not carry over to a general initial condition.
- Explicit: `r = 0.5`, with the net error `0.605 * dx^2` (the time and space errors partly cancel).
- Backward Euler: one third of the error budget goes to space and two thirds to time, which is the optimal split.
- Crank-Nicolson: half of the error budget goes to space and half to time, which is the optimal split when the errors add.

### Scaling

| Scheme | `N` | steps `n` | flops per step | total flops |
|---|---|---|---|---|
| Explicit | 0.778 * eps^(-1/2) | 0.121 * eps^(-1) | 5N | 0.471 * eps^(-3/2) |
| Backward Euler | 0.953 * eps^(-1/2) | 0.272 * eps^(-1) | 8N | 2.08 * eps^(-3/2) |
| Crank-Nicolson | 0.778 * eps^(-1/2) | 0.244 * eps^(-1/2) | 12N | 2.28 * eps^(-1) |

### Predicted totals at `eps = 1e-5`

| Scheme | total flops |
|---|---|
| Explicit | about 1.5e7 |
| Backward Euler | about 6.6e7 |
| Crank-Nicolson | about 2.3e5 |

### Predicted ranking

Crank-Nicolson is cheapest, then explicit, then Backward Euler. Backward Euler and explicit have the same exponent, so the constants decide: explicit does about 4.4 times less work than Backward Euler at every `eps`. Crank-Nicolson does about 65 times less work than explicit and about 290 times less than Backward Euler at `eps = 1e-5`.

This reverses the fixed-N result, where Backward Euler did about 27 times less work than explicit. Backward Euler pays off only when stability, not accuracy, limits the step, and a smooth sine mode is not that case.

The counts are flops only. Divisions in the Thomas algorithm and the per-step vector allocations in the implicit schemes make the real implicit cost higher than the flop ratio suggests, which favours the explicit scheme further against Backward Euler.
