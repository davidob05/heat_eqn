import argparse
import math
import subprocess
import sys
from pathlib import Path
import numpy as np
import matplotlib
import matplotlib.pyplot as plt

from plot import load_blocks

DEFAULT_NS = [10, 20, 40, 80, 160, 320, 640]
REFERENCE_OFFSET = 3.0


def parse_args(argv=None):
    parser = argparse.ArgumentParser(
        description="Run the solver on a series of grids and report the convergence order."
    )
    parser.add_argument("--exe", type=Path, default=Path("build/release/Main"))
    parser.add_argument("--ns", type=int, nargs="+", default=DEFAULT_NS, help="intervals per run")
    parser.add_argument("--length", type=float, default=1.0)
    parser.add_argument("--alpha", type=float, default=1.0)
    parser.add_argument("--mode", type=int, default=1)
    parser.add_argument("--T", type=float, default=0.1)
    parser.add_argument("--r-req", type=float, default=0.4)
    parser.add_argument("--csv-dir", type=Path, default=Path("results"), help="where convN.csv files go")
    parser.add_argument("--figure", type=Path, default=Path("docs/figures/convergence.png"))
    parser.add_argument("--table", type=Path, default=Path("docs/convergence_table.md"))
    return parser.parse_args(argv)


# calculations

def max_error(numeric, exact):
    numeric, exact = _paired(numeric, exact)
    return float(np.max(np.abs(numeric - exact)))


def l2_error(numeric, exact, dx):
    numeric, exact = _paired(numeric, exact)
    return float(math.sqrt(dx * np.sum((numeric - exact) ** 2)))


def _paired(a, b):
    a, b = np.asarray(a, dtype=float), np.asarray(b, dtype=float)
    if a.shape != b.shape:
        raise ValueError(f"arrays differ in shape: {a.shape} vs {b.shape}")
    return a, b


def observed_order(err_coarse, err_fine, dx_coarse, dx_fine):
    if err_coarse <= 0 or err_fine <= 0:
        raise ValueError(f"errors must be positive to get an order, got {err_coarse} and {err_fine}")
    return math.log(err_coarse / err_fine) / math.log(dx_coarse / dx_fine)


def orders_for_series(dxs, errs):
    """Order of each row against the row above; None for the first row."""
    return [None] + [
        observed_order(errs[i - 1], errs[i], dxs[i - 1], dxs[i]) for i in range(1, len(errs))
    ]


def check_final_time(block_time, T, rel_tol=1e-9):
    if not math.isclose(block_time, T, rel_tol=rel_tol, abs_tol=1e-12):
        raise ValueError(f"last block is at t={block_time!r}, expected T={T!r}")


def time_steps(n, length, alpha, T, r):
    steps = T * alpha * n**2 / (r * length**2)
    rounded = round(steps)
    if abs(steps - rounded) > 1e-6 * max(1.0, steps):
        raise ValueError(f"N={n} gives a non-integer step count {steps}")
    return int(rounded)


def reference_line(dxs, errs, index, slope=2.0, offset=1.0):
    """Values at dxs of the line offset * errs[index] * (dx/dxs[index])**slope.

    offset=1 passes through the data point; other values shift the line by a constant factor.
    """
    dxs = np.asarray(dxs, dtype=float)
    return offset * errs[index] * (dxs / dxs[index]) ** slope


def format_table(rows):
    """rows: dicts with n, dx, steps, max_err, max_order, l2_err, l2_order."""
    header = ["N", "dx", "time steps", "max error", "max-error order", "L2 error", "L2 order"]

    def order(v):
        return "-" if v is None else f"{v:.2f}"

    lines = ["| " + " | ".join(header) + " |", "|" + "|".join(["---"] * len(header)) + "|"]
    for r in rows:
        cells = [
            str(r["n"]), f"{r['dx']:.4g}", str(r["steps"]),
            f"{r['max_err']:.4e}", order(r["max_order"]),
            f"{r['l2_err']:.4e}", order(r["l2_order"]),
        ]
        lines.append("| " + " | ".join(cells) + " |")
    return "\n".join(lines) + "\n"


# running the solver and drawing

def run_case(args, n):
    csv = args.csv_dir / f"conv{n}.csv"
    csv.parent.mkdir(parents=True, exist_ok=True)
    cmd = [
        str(args.exe), "--n-intervals", str(n), "--length", str(args.length),
        "--alpha", str(args.alpha), "--mode", str(args.mode), "--T", str(args.T),
        "--r-req", str(args.r_req), "--snapshots", "1", "--output", str(csv),
    ]
    try:
        proc = subprocess.run(cmd, capture_output=True, text=True)
    except OSError as e:
        raise RuntimeError(f"could not run '{args.exe}': {e}")
    if proc.returncode != 0:
        raise RuntimeError(f"N={n}: Main exited with {proc.returncode}: {proc.stderr.strip()}")
    if proc.stderr.strip():
        raise RuntimeError(f"N={n}: Main printed on stderr, row not trusted: {proc.stderr.strip()}")
    try:
        last = load_blocks(csv)[-1]
        check_final_time(float(last[0, 0]), args.T)
    except ValueError as e:
        raise RuntimeError(f"N={n}: '{csv}': {e}")
    dx = args.length / n
    return {
        "n": n, "dx": dx,
        "steps": time_steps(n, args.length, args.alpha, args.T, args.r_req),
        "max_err": max_error(last[:, 2], last[:, 3]),
        "l2_err": l2_error(last[:, 2], last[:, 3], dx),
    }


def draw_figure(rows, args):
    dxs = [r["dx"] for r in rows]
    max_errs = [r["max_err"] for r in rows]
    l2_errs = [r["l2_err"] for r in rows]
    fig, ax = plt.subplots(constrained_layout=True)
    ax.loglog(dxs, max_errs, "o-", label="max error")
    ax.loglog(dxs, l2_errs, "s-", label="L2 error")
    # shifted up by a constant factor so it does not hide the data it is parallel to
    ax.loglog(
        dxs, reference_line(dxs, max_errs, len(dxs) // 2, offset=REFERENCE_OFFSET), "k--",
        label=f"slope 2 reference (x{REFERENCE_OFFSET:g})",
    )
    ax.set_xlabel("dx")
    ax.set_ylabel("error at t = T")
    ax.set_title(f"Convergence of the explicit scheme (r = {args.r_req}, T = {args.T}, mode {args.mode})")
    ax.legend()
    ax.grid(True, which="both", alpha=0.3)
    args.figure.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(args.figure, dpi=200)


def main(argv=None):
    args = parse_args(argv)
    if len(args.ns) < 2:
        print("Error: need at least two values of N", file=sys.stderr)
        return 1
    try:
        rows = [run_case(args, n) for n in args.ns]
        dxs = [r["dx"] for r in rows]
        for key, name in (("max_err", "max_order"), ("l2_err", "l2_order")):
            for r, o in zip(rows, orders_for_series(dxs, [r[key] for r in rows])):
                r[name] = o
    except (RuntimeError, ValueError) as e:
        print(f"Error: {e}", file=sys.stderr)
        return 1

    table = format_table(rows)
    args.table.parent.mkdir(parents=True, exist_ok=True)
    args.table.write_text(table)
    print(table)
    draw_figure(rows, args)
    print(f"Saved table to {args.table} and figure to {args.figure}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
