# Runtime results

Measured against the predictions in [predictions.md](predictions.md) (section "Predicted run time"). Same setup: `N = 2000`, `L = 1`, `alpha = 1`, `T = 0.1`, mode 1. Time is the `advance_state` loop only, from `src/bench/bench.cpp` (5 runs per scheme after one warm-up run, single-threaded).

## Hardware and build

```
$ lscpu | grep "Model name"
Model name:                              AMD Ryzen AI 9 365 w/ Radeon 880M
$ nproc
20
$ free -h
               total        used        free      shared  buff/cache   available
Mem:            30Gi       7.5Gi        13Gi       242Mi        10Gi        22Gi
Swap:          8.0Gi          0B       8.0Gi
```

Compiler: g++ 13.3.0 (Ubuntu 24.04).

The CPU has 10 cores (4 Zen 5 at up to 5.09 GHz and 6 Zen 5c at up to 3.32 GHz) and 20 threads; `nproc` reports threads. The clock logged during the default-build runs was about 5.05 GHz.

| Build | Flags | Directory |
|---|---|---|
| Release | `-O3 -DNDEBUG` | `build/release` |
| Release, no auto-vectorisation | `-fno-tree-vectorize -O3 -DNDEBUG` | `build/release-novec` |

## Results

| Scheme | Steps | `dt` | Predicted max error | Measured median max error |
|---|---|---|---|---|
| Explicit | 800,001 | 1.25e-7 | 1.5e-7 | 1.51e-7 |
| Backward Euler | 18,291 | 5.47e-6 | 1.0e-5 | 1.00e-5 |
| Crank-Nicolson | 55 | 1.818e-3 | 9.8e-6 | 9.80e-6 |

Release (`-O3`):

| Scheme | Predicted time | Measured median | Minimum | Maximum | Measured / predicted |
|---|---|---|---|---|---|
| Explicit | about 840 ms | 0.478 s | 0.478 s | 0.479 s | 0.57 |
| Backward Euler | about 300 ms | 0.362 s | 0.361 s | 0.362 s | 1.21 |
| Crank-Nicolson | about 1.0 ms | 1.10 ms | 1.10 ms | 1.13 ms | 1.10 |

Release with `-fno-tree-vectorize`:

| Scheme | Predicted time | Measured median | Minimum | Maximum | Measured / predicted |
|---|---|---|---|---|---|
| Explicit | about 840 ms | 0.656 s | 0.640 s | 0.690 s | 0.78 |
| Backward Euler | about 300 ms | 0.371 s | 0.370 s | 0.374 s | 1.24 |
| Crank-Nicolson | about 1.0 ms | 1.15 ms | 1.15 ms | 1.16 ms | 1.15 |

The prediction did not separate the two builds, so both tables use the same predicted times.

Both tables hold unpinned runs. A repeat of the default (vectorised) build pinned to CPU 0 (a Zen 5 core, maximum 5.09 GHz) agreed with the tables to within 0.2% for the explicit and Backward Euler schemes and moved by 4% for Crank-Nicolson, whose loop is about 1 ms. The no-vectorisation build was not repeated pinned.

Per-step cost (median time / steps), Release: Backward Euler 0.362 s / 18,291 = 19.8 µs, Crank-Nicolson 1.10 ms / 55 = 20.0 µs. Without vectorisation: Backward Euler 20.3 µs, Crank-Nicolson 21.0 µs.

| Ratio | Predicted | Measured (Release) | Measured (no vectorisation) |
|---|---|---|---|
| Explicit / Backward Euler | about 2.8 (range 1.3 to 2.8) | 1.32 | 1.77 |

## Why the results differ from the predictions

### Explicit time (0.478 s measured, 840 ms predicted)

The explicit scheme ran faster than predicted (0.478 s against 840 ms). Two effects account for this. First, the flop rate assumed in the prediction was too low. The prediction used 2 flops per cycle at 5 GHz. The measured clock was about 5.05 GHz, so the clock assumption was accurate to within 1%. The inner loop of `time_step` performs 3 additions, 1 subtraction and 1 multiplication per point (read from the disassembly of `build/release/Bench` and `build/release-novec/Bench`; the compiler turns `2.0*curr` into `curr + curr`), which matches the count of 5 flops per point used in the prediction, so the compiler did not reduce the arithmetic. The scalar build (`-fno-tree-vectorize`, 0.656 s) achieved 2.4 flops per cycle, about 5 instructions per cycle, so this core executes more than 2 scalar floating-point operations per cycle. This is consistent with the hardware: a Zen 5 core has four floating-point math pipes, so the assumption of 2 flops per cycle was below what the core allows. Second, the default build is vectorised; its `time_step` contains packed SSE2 instructions (`addpd`, `subpd`, `mulpd`, two points per iteration; the scalar build has none, and the section "Checks on the binaries" describes the kind of vectorisation), and removing them made the run 1.37 times slower. The prediction of 840 ms corrected for throughput gives about 656 ms, and vectorisation then gives about 478 ms, which matches the measurement. The two corrections are calibrations of the model, and the 2.4 flops per cycle was calculated from the measured time, so the throughput figure is not an independent confirmation. The clock of about 5.05 GHz was logged during the default-build runs; the scalar build was run unpinned and its clock speed was assumed to equal that figure.

### Explicit to Backward Euler ratio (1.32 measured, 2.8 predicted)

The predicted ratio of explicit to Backward Euler run time was 2.8 (840 ms against 300 ms). The measured ratio in the default (vectorised) Release build was 1.32 (0.478 s against 0.362 s), and 1.77 in the build with `-fno-tree-vectorize` (0.656 s against 0.371 s). Replacing the predicted times with measured ones one at a time divides the gap into three factors:

| Step | Change | Ratio after | Factor |
|---|---|---|---|
| Start | Prediction | 2.8 | |
| A | Explicit time replaced by the scalar build time (656 ms) | 2.19 | 1.28 |
| B | Backward Euler time replaced by the scalar build time (371 ms) | 1.77 | 1.24 |
| C | Both replaced by the default build times (478 ms and 362 ms) | 1.32 | 1.34 |

The product of the three factors is 2.12, which equals 2.8 divided by 1.32. Factor A is the underestimated explicit throughput (2.4 flops per cycle measured against 2 assumed). Factor C is vectorisation, which affects the explicit scheme far more than Backward Euler. Factor B is an error in the Backward Euler estimate and works in the opposite direction to the other two: the measured cost of the Thomas solve was about 50 cycles per matrix row, against about 41 in the model, so Backward Euler was slower than predicted and the ratio fell. The cause of factor B was not tested. Possible causes are the latency of the two divisions on each row's dependency chain, the vector allocations and copies made in every step, and the recomputation of the factorisation in every step. The ratio of 1.32 was stable between two separate runs (1.34 and 1.32), and the spread within a run was below 1% for both schemes.

### Backward Euler and Crank-Nicolson per-step cost (19.8 µs against 20.0 µs)

A Backward Euler step cost 19.7 to 19.8 µs and a Crank-Nicolson step cost 20.0 to 20.9 µs (unpinned and pinned runs), a difference of 1% to 6%, although Crank-Nicolson performs 1.5 times the flops per step. Flop count is therefore a poor predictor of implicit step cost: the work that both schemes share, the Thomas solve and the per-step vector allocations and copies, dominates, and the extra right-hand-side loop in Crank-Nicolson adds about 1 µs. The data does not distinguish the dependency chain from the allocations as the dominant cost, and this was not tested. For smooth data such as the single sine mode used here, Crank-Nicolson gives second-order accuracy at nearly the cost of first-order Backward Euler. This conclusion was not tested on non-smooth initial data, where Crank-Nicolson can oscillate at large time steps.

## Checks on the binaries

These were read from `objdump -d -C` of `build/release/Bench` and `build/release-novec/Bench` (g++ 13.3.0, the binaries that produced the tables), and from the `-fopt-info-vec-optimized` report for each source file compiled with the Release flags.

### Explicit scheme

`advance_state` stores the address of `time_step` in a function pointer and calls it once per step through that pointer, so `time_step` is not inlined into the step loop in either binary. Each call updates 1,998 points, so the call itself is probably a small part of the cost, but this was not measured. The compiler report calls the vectorisation in `time_step.cpp:15` "basic block part vectorized", which is SLP vectorisation of two points inside one loop iteration, not vectorisation of the loop. The packed code loads the neighbours with scalar loads and builds the vectors with `unpcklpd` shuffles, so part of the packed work goes on shuffling. This fits the speed-up of 1.37 times rather than the 2 times a fully packed SSE2 loop would give. Why GCC did not vectorise the whole loop was not tested. A likely cause is the value `prev` carried from one iteration to the next together with the in-place write.

### Implicit schemes

The default build has 12 packed add, subtract, multiply or divide instructions in total. Five are in `time_step`, four are in the right-hand-side loop of `crank_nicolson` (`crank_nicolson.cpp:19`, vectorised with a runtime aliasing check), and three are in `exact_soln` and `l2_error`, which are outside the timed region. There are none in `backward_euler` or `solve_tridiagonal`, and `solve_tridiagonal` is the same scalar code in both builds. So `-fno-tree-vectorize` can only change the implicit times through the Crank-Nicolson right-hand-side loop and the vector copy loops, which is consistent with the small slowdowns (2.5% for Backward Euler, 4.5% for Crank-Nicolson). That the Crank-Nicolson slowdown comes from its right-hand-side loop was not tested.

`solve_tridiagonal` contains one `divsd` in the forward sweep and one in the back substitution, two per row, which matches the two divisions per row named in the explanation of factor B. In the forward sweep the pivot is stored to memory and loaded again on the next row, so the dependency chain passes through memory as well as the divide. In the back substitution the chain (multiply, subtract, divide) stays in registers. Backward Euler and Crank-Nicolson each make four vector allocations in their own function (three for the matrix and one for the right-hand side) and two more inside `solve_tridiagonal`, six per step in both schemes, and both redo the whole elimination every step because the matrix is rebuilt every step. This is the same work in both schemes, so it is consistent with their near-equal step cost. It does not separate the dependency chain from the allocations as the dominant cost, because the instruction counts say nothing about how long each part takes.

### Limits of these checks

The instruction counts are static: they show which instructions are in the binary, not how often or how long each one ran. No profiler or hardware counters were used.

## Not tested

- The clock speed of the no-vectorisation build (assumed equal to the logged 5.05 GHz).
- The cause of factor B (the Backward Euler cost per row): dependency chain against allocations.
- Why GCC did not vectorise the whole explicit loop.
- Behaviour on non-smooth initial data.
- The exact per-instruction scalar floating-point limit of the core (instruction tables were not consulted).
- The wall-clock comparison with each scheme at its own best `N`.
