import argparse
import sys
from pathlib import Path
import numpy as np
import matplotlib
import matplotlib.pyplot as plt


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
    return parser.parse_args(argv)


def main(argv=None):
    args = parse_args(argv)

    if not args.path.is_file():
        print(f"Error: '{args.path}' is not a file", file=sys.stderr)
        return 1
    path = str(args.path)
    if not path.endswith(".csv"):
        print(f"Error: '{args.path}' is not a csv file", file=sys.stderr)
        return 1
    data = np.loadtxt(path, delimiter=",", skiprows=1)
    initial_time = data[0][0]
    i = 0
    while data[i][0]==initial_time:
        i+=1
    x = data[:i,1]
    n_blocks = len(data)//i
    colors = matplotlib.colormaps["viridis"](np.linspace(0, 0.9, n_blocks))

    fig, (ax_top, ax_bottom) = plt.subplots(
        2, 1, sharex=True, gridspec_kw={"height_ratios": [2, 1]}, constrained_layout=True
    )

    fig.get_layout_engine().set(hspace=0.12)

    blocks = [data[i*j:i*(j+1)] for j in range(n_blocks)]
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
    ax_top.legend()
    ax_bottom.set_xlabel("x")
    ax_bottom.set_ylabel(f"numeric - exact (error)\n[units of $10^{{{exponent}}}$]")
    ax_top.set_title("Temperature profile (lines = exact, markers = numeric)")
    ax_bottom.set_title("Error (numeric solution - exact solution)")

    plots_dir = Path("plots")
    plots_dir.mkdir(exist_ok=True)
    out_path = plots_dir / f"{args.path.stem}.png"
    fig.savefig(out_path, dpi=200)
    print(f"Saved plot to {out_path}")
    plt.show()
    return 0


if __name__ == "__main__":
    sys.exit(main())
