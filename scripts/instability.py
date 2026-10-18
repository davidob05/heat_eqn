import argparse
import subprocess
import sys
from pathlib import Path

import numpy as np
import matplotlib.pyplot as plt

from convergence import check_final_time
from plot import load_blocks

STABILITY_LIMIT = 0.5
GROWTH_FACTOR = 10
ZOOM_YLIM = (1e-3, 1e3)


def peaks_over_time(blocks):
    """Time and largest |u_numeric| of each block, as two lists."""
    if len(blocks) == 0:
        raise ValueError("no blocks to measure")
    times = [float(b[0, 0]) for b in blocks]
    peaks = [float(np.max(np.abs(b[:, 2]))) for b in blocks]
    return times, peaks


def expects_warning(r_req):
    return r_req > STABILITY_LIMIT


def check_stderr(stderr, r_req):
    if expects_warning(r_req):
        if "Warning" not in stderr:
            raise ValueError(f"r = {r_req:g} is unstable but Main printed no warning")
    elif stderr.strip():
        raise ValueError(f"r = {r_req:g} is stable but Main printed on stderr: {stderr.strip()}")


def check_behaviour(r_req, peaks):
    if len(peaks) < 2:
        raise ValueError(f"r = {r_req:g}: need at least two peaks, got {len(peaks)}")
    first, last = peaks[0], peaks[-1]
    if expects_warning(r_req):
        if not last > GROWTH_FACTOR * first:
            raise ValueError(
                f"r = {r_req:g} is unstable but the peak went from {first:g} to {last:g}, "
                f"less than {GROWTH_FACTOR}x growth"
            )
    elif not last < first:
        raise ValueError(
            f"r = {r_req:g} is stable but the peak went from {first:g} to {last:g}, not a decay"
        )


def legend_label(r_req):
    return f"r = {r_req:g}"


# running the solver and drawing

def parse_args(argv=None):
    parser = argparse.ArgumentParser(
        description="Run the solver at several values of r and show which ones blow up."
    )
    parser.add_argument("--exe", type=Path, default=Path("build/release/Main"))
    parser.add_argument("--rs", type=float, nargs="+", default=[0.45, 0.51, 0.55, 0.6])
    parser.add_argument("--n-intervals", type=int, default=100)
    parser.add_argument("--snapshots", type=int, default=100)
    parser.add_argument("--length", type=float, default=1.0)
    parser.add_argument("--alpha", type=float, default=1.0)
    parser.add_argument("--mode", type=int, default=1)
    parser.add_argument("--T", type=float, default=0.1)
    parser.add_argument("--csv-dir", type=Path, default=Path("results"))
    parser.add_argument("--figure", type=Path, default=Path("docs/figures/instability.png"))
    return parser.parse_args(argv)


def run_case(args, r):
    csv = args.csv_dir / f"instab_r{r:g}.csv"
    csv.parent.mkdir(parents=True, exist_ok=True)
    cmd = [
        str(args.exe), "--n-intervals", str(args.n_intervals), "--length", str(args.length),
        "--alpha", str(args.alpha), "--mode", str(args.mode), "--T", str(args.T),
        "--r-req", str(r), "--snapshots", str(args.snapshots), "--output", str(csv),
    ]
    try:
        proc = subprocess.run(cmd, capture_output=True, text=True)
    except OSError as e:
        raise RuntimeError(f"could not run '{args.exe}': {e}")
    if proc.returncode != 0:
        raise RuntimeError(f"r = {r:g}: Main exited with {proc.returncode}: {proc.stderr.strip()}")
    check_stderr(proc.stderr, r)
    try:
        # an unstable run may overflow to inf/nan, which load_blocks rejects; report it as such
        blocks = load_blocks(csv)
        check_final_time(float(blocks[-1][0, 0]), args.T)
    except ValueError as e:
        raise RuntimeError(f"r = {r:g}: '{csv}': {e}")
    times, peaks = peaks_over_time(blocks)
    check_behaviour(r, peaks)
    return r, times, peaks


def draw_figure(results, args):
    fig, (ax_full, ax_zoom) = plt.subplots(2, 1, sharex=True, constrained_layout=True)
    for ax in (ax_full, ax_zoom):
        for r, times, peaks in results:
            ax.semilogy(times, peaks, label=legend_label(r))
        ax.axhline(1.0, color="k", linestyle="--", linewidth=1, label="initial maximum")
        ax.set_ylabel("largest |u|")
        ax.grid(True, which="both", alpha=0.3)
    # the zoomed panel makes the stable decay and the early turn-ups visible
    ax_zoom.set_ylim(*ZOOM_YLIM)
    ax_zoom.set_xlabel("time t")
    ax_full.set_title(f"Growth of the largest |u| (N = {args.n_intervals}, T = {args.T}, mode {args.mode})")
    ax_zoom.set_title(f"Zoom on {ZOOM_YLIM[0]:g} to {ZOOM_YLIM[1]:g}")
    ax_full.legend()
    args.figure.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(args.figure, dpi=200)


def main(argv=None):
    args = parse_args(argv)
    try:
        results = [run_case(args, r) for r in args.rs]
    except (RuntimeError, ValueError) as e:
        print(f"Error: {e}", file=sys.stderr)
        return 1
    for r, _, peaks in results:
        print(f"r = {r:g}: final peak {peaks[-1]:.6g}")
    draw_figure(results, args)
    print(f"Saved figure to {args.figure}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
