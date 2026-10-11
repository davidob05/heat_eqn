import argparse
import sys
from pathlib import Path
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.pyplot import cm


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
    color = cm.rainbow(np.linspace(0, 1, n_blocks))
    for j in range(n_blocks):
        block = data[i*j:i*(j+1)]
        plt.plot(x, block[:, 2], color=color[j], label=f"t={block[0, 0]:.4g}")    
    plt.legend()
    plt.show()
    return 0


if __name__ == "__main__":
    sys.exit(main())
