import argparse
import sys
import warnings
from pathlib import Path
import numpy as np
import matplotlib
import matplotlib.pyplot as plt

EXPECTED_HEADER = ["t", "x", "u_numeric", "u_exact"]


def parse_args(argv=None):
    parser = argparse.ArgumentParser(
        description="Plot results from a heat equation simulation CSV."
    )
    parser.add_argument(
        "path",
        type=Path,
        nargs="?",
        default=Path("results/output.csv"),
        help="CSV file written by the solver (default: %(default)s)",
    )
    parser.add_argument(
        "-o",
        "--output",
        type=Path,
        default=None,
        help="image path to write (default: next to the CSV, with a .png extension)",
    )
    parser.add_argument(
        "--show",
        action="store_true",
        help="open the figure in a window after saving (off by default)",
    )
    return parser.parse_args(argv)


def load_blocks(path):
    with open(path) as f:
        header = f.readline().strip().split(",")
    if header != EXPECTED_HEADER:
        raise ValueError(
            f"unexpected header {header}, expected {EXPECTED_HEADER}"
        )

    try:
        with warnings.catch_warnings():
            warnings.simplefilter("ignore")  # an empty file is reported below
            data = np.loadtxt(path, delimiter=",", skiprows=1, ndmin=2)
    except ValueError as e:
        raise ValueError(f"could not read data rows (file may be truncated): {e}")
    if data.shape[0] == 0:
        raise ValueError("file has a header but no data rows")
    if data.shape[1] != len(EXPECTED_HEADER):
        raise ValueError(f"expected {len(EXPECTED_HEADER)} columns, found {data.shape[1]}")

    # the first block is the rows sharing the initial time
    block_len = int(np.count_nonzero(data[:, 0] == data[0, 0]))
    if len(data) % block_len != 0:
        raise ValueError(
            f"{len(data)} rows is not a whole number of {block_len}-row blocks (file may be truncated)"
        )
    n_blocks = len(data) // block_len
    blocks = data.reshape(n_blocks, block_len, len(EXPECTED_HEADER))

    times = blocks[:, :, 0]
    if not np.all(times == times[:, :1]):
        raise ValueError("time column is not constant within a block")
    if not np.all(np.diff(times[:, 0]) > 0):
        raise ValueError("block times are not increasing")
    return list(blocks)


def main(argv=None):
    args = parse_args(argv)

    if not args.path.is_file():
        print(f"Error: '{args.path}' is not a file", file=sys.stderr)
        return 1
    path = str(args.path)
    if not path.endswith(".csv"):
        print(f"Error: '{args.path}' is not a csv file", file=sys.stderr)
        return 1

    try:
        blocks = load_blocks(args.path)
    except ValueError as e:
        print(f"Error: '{args.path}': {e}", file=sys.stderr)
        return 1

    x = blocks[0][:, 1]
    n_blocks = len(blocks)
    colors = matplotlib.colormaps["viridis"](np.linspace(0, 0.9, n_blocks))

    fig, (ax_top, ax_bottom) = plt.subplots(
        2, 1, sharex=True, gridspec_kw={"height_ratios": [2, 1]}, constrained_layout=True
    )

    fig.get_layout_engine().set(hspace=0.12)

    max_abs_diff = max(np.max(np.abs(block[:, 2] - block[:, 3])) for block in blocks)

    # plot the error in units of 10^exponent so the axis shows plain numbers
    exponent = int(np.floor(np.log10(max_abs_diff))) if max_abs_diff > 0 else 0
    error_scale = 10.0**exponent

    for j, block in enumerate(blocks):
        diff = (block[:, 2] - block[:, 3]) / error_scale

        # exact solution as a line, numeric values as sparse markers on top of it
        ax_top.plot(x, block[:, 3], color=colors[j], label=f"t={block[0, 0]:.4g}")
        ax_top.plot(x, block[:, 2], color=colors[j], linestyle="", marker="o", markersize=3, markevery=4)

        # the t=0 block is identically zero, so skip it
        if j > 0:
            ax_bottom.plot(x, diff, color=colors[j])

    ax_top.set_ylabel("u(x,t)")
    # legend sits outside the axes so it never covers the curves
    fig.legend(*ax_top.get_legend_handles_labels(), loc="outside right upper")
    ax_bottom.set_xlabel("x")
    ax_bottom.set_ylabel(f"numeric - exact (error)\n[units of $10^{{{exponent}}}$]")
    ax_top.set_title("Temperature profile (lines = exact, markers = numeric)")
    ax_bottom.set_title(f"Error (numeric - exact), max |error| = {max_abs_diff:.3g}")

    out_path = args.output if args.output is not None else args.path.with_suffix(".png")
    out_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(out_path, dpi=200)
    print(f"Saved plot to {out_path}")
    if args.show:
        plt.show()
    return 0


if __name__ == "__main__":
    sys.exit(main())
