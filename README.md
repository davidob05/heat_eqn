# heat_eqn
A program that approximates the solution to the 1D heat equation

    u_t = alpha * u_xx,   0 < x < L

on a rod with both ends held at 0. The initial state is a sine mode, `u(x, 0) = sin(mode * pi * x / L)`, and the solution is stepped forward with the explicit FTCS (forward-time, centred-space) scheme. Each snapshot is written alongside the exact solution, `u(x, t) = exp(-alpha * (mode * pi / L)^2 * t) * sin(mode * pi * x / L)`, so the error can be read straight from the output.

# Instructions

## Requirements
 - C++17 compiler
 - CMake 3.25 or newer
 - First configure downloads Catch2 so internet is needed
 - Python 3 with `numpy` and `matplotlib` (only for the scripts in `scripts/`); `pip install -r requirements.txt`

## Build debug
```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
./build/debug/Main
```

## Build release
```bash
cmake --preset release
cmake --build --preset release
ctest --preset release
./build/release/Main
```

## Debug vs Release
Debug is for development (warnings are errors, AddressSanitizer is on), release is for real, optimised runs.

## Running
`Main` runs one simulation and writes a CSV. Every flag is optional; anything left out keeps its default. Run it from the project root, because the default output path is relative.

```bash
./build/release/Main --n-intervals 50 --T 0.05 --mode 2 --snapshots 4 --output results/mode2.csv
```

| Flag | Meaning | Default |
|---|---|---|
| `--n-intervals <int>` | number of intervals along the rod (max 10,000,000) | 100 |
| `--length <double>` | rod length `L` | 1.0 |
| `--alpha <double>` | diffusivity | 1.0 |
| `--T <double>` | target time | 0.1 |
| `--r-req <double>` | requested `r = alpha * dt / dx^2` | 0.4 |
| `--mode <int>` | sine mode of the initial state (not 0) | 1 |
| `--snapshots <int>` | snapshots written after the initial state | 5 |
| `--output <path>` | output CSV path | `results/output.csv` |
| `--help`, `-h` | print usage | |

Values may be given as `--flag value` or `--flag=value`.

The number of time steps `n` is `ceil(T / dt_req)`, where `dt_req = r_req * dx^2 / alpha`. The step is then shrunk to `dt = T / n`, so the run ends exactly at `T` and the `r` actually used can be a little below `r_req`.

The run writes to `<output>.tmp` and renames it over the target only on success, so a failed run never overwrites an existing result.

### Warnings
Warnings are printed to stderr and do not stop the run:
 - `r` above 0.5 (the scheme is unstable)
 - mode too high for the number of intervals (`|mode| * 10 > n_intervals`)
 - the state decays almost to zero by `T`, so the error margin may be misleading
 - the initial function is not close to 0 at the left or right end

### Output format
CSV with the header `t,x,u_numeric,u_exact`. The first block is the initial state at `t = 0`, followed by one block per snapshot. Each block has one row per grid point (`n_intervals + 1` rows). Values are written with enough digits to round-trip exactly.

## Plotting
```bash
python3 scripts/plot.py [path/to/output.csv] [-o path/to/figure.png] [--show]
```
The CSV path defaults to `results/output.csv`. The figure is saved next to the CSV with a `.png` extension unless `-o/--output` is given, so scratch plots stay beside their data. For figures you want to keep in git, write them to a tracked folder, for example `-o docs/figures/default_run.png`. The window only opens with `--show`. A CSV with the wrong header, a truncated file (fewer than two snapshot blocks, or cut part-way through a block) or non-finite values (a diverged run, so check `r`) is rejected with a message and a non-zero exit code rather than a traceback or a misleading plot.

The top panel shows the exact solution (lines) with the numeric solution (markers); the bottom panel shows numeric minus exact, with the largest absolute error in its title. Generated CSVs and PNGs in `results/` are git-ignored.

The plot script has its own tests: `python3 -m unittest tests.test_plot` from the project root.

## Convergence
Fixed `r = 0.4`, `T = 0.1`, `L = 1`, `alpha = 1`, mode 1, halving `dx` each row. With these settings `n = N^2 / 4` is a whole number, so the step is exactly `r_req` and `r` stays at 0.4 for every row. Errors are measured at `t = T` (the max error is also the largest over all snapshots, since the error grows with time). The max error is the largest `|numeric - exact|` over the grid; the L2 error is `sqrt(dx * sum(d^2))`. Each order is `log(err_prev / err) / log(dx_prev / dx)`.

| N | dx | time steps | max error | max-error order | L2 error | L2 order |
|---|---|---|---|---|---|---|
| 10 | 0.1 | 25 | 4.2941e-03 | - | 3.0364e-03 | - |
| 20 | 0.05 | 100 | 1.0625e-03 | 2.01 | 7.5131e-04 | 2.01 |
| 40 | 0.025 | 400 | 2.6495e-04 | 2.00 | 1.8735e-04 | 2.00 |
| 80 | 0.0125 | 1,600 | 6.6195e-05 | 2.00 | 4.6807e-05 | 2.00 |
| 160 | 0.00625 | 6,400 | 1.6546e-05 | 2.00 | 1.1700e-05 | 2.00 |
| 320 | 0.003125 | 25,600 | 4.1364e-06 | 2.00 | 2.9249e-06 | 2.00 |
| 640 | 0.001563 | 102,400 | 1.0341e-06 | 2.00 | 7.3121e-07 | 2.00 |

![Convergence of the explicit scheme](docs/figures/convergence.png)

The error falls by a factor of 4 each time the grid is halved, which is second-order convergence in `dx` (the points run parallel to the dashed slope-2 reference line, which is shifted up by a factor of 3 so it does not hide the data). Because `r` is fixed, `dt = r dx^2 / alpha` shrinks with `dx^2` as well, so this is the combined space and time error, not the spatial error alone. The default run (`N = 100`, `n = 2500`) has a max error of 4.236e-5, which sits between the `N = 80` and `N = 160` rows as expected.

To reproduce the table and figure with one command (build the release binary first, run from the project root):

```bash
python3 scripts/convergence.py
```

It runs `Main` for each `N` with one snapshot, writes `results/convN.csv`, reads them with the same loader as `plot.py`, and writes `docs/convergence_table.md` and `docs/figures/convergence.png`. It stops with a message and a non-zero exit code if `Main` fails or prints anything on stderr (for example an unstable `r`), if a CSV is malformed, or if the last block is not at `t = T`. Options (all optional): `--exe`, `--ns`, `--length`, `--alpha`, `--mode`, `--T`, `--r-req`, `--csv-dir`, `--figure`, `--table`.

The script's calculations are tested with `python3 -m unittest tests.test_convergence_script`. The N = 20 row (max error 1.0625e-3, L2 7.51e-4) is cross-checked against the C++ test in `tests/test_convergence.cpp`, and that Python test is skipped if `build/release/Main` is absent.

## Project layout
```
src/
  core/       Grid and initial-state construction
  solver/     FTCS time step, step-parameter calculation, step loop
  verifier/   Sine initial function and exact solution
  writer/     CSV output
  driver/     run_simulation (validation, snapshots, warnings) and the CLI parser
  main.cpp    Command-line entry point
tests/        Catch2 unit tests (C++, one file per module, plus test_convergence.cpp)
              and Python tests (test_plot.py, test_convergence_script.py)
scripts/      plot.py (plot one run) and convergence.py (grid-refinement study)
results/      Default output location for CSVs (contents are git-ignored)
docs/         Kept in git: convergence_table.md and figures (docs/figures/)
```
The `src` modules other than `main.cpp` are built into the `heat_solver` library, which both `Main` and `unit_tests` link against.
