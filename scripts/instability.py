import numpy as np

STABILITY_LIMIT = 0.5
GROWTH_FACTOR = 10


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
    return f"r = {r_req:g} (requested)"
