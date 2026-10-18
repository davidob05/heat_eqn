import sys
import unittest
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "scripts"))
import instability as inst  # noqa: E402


def make_block(t, numeric, exact=None):
    """A block with columns t, x, u_numeric, u_exact over x = 0, 0.5, 1."""
    exact = numeric if exact is None else exact
    xs = [0.0, 0.5, 1.0]
    return np.array([[t, x, u, e] for x, u, e in zip(xs, numeric, exact)])


class PeaksOverTimeTest(unittest.TestCase):
    def test_times_and_peaks(self):
        times, peaks = inst.peaks_over_time(
            [make_block(0, [0, 1, 0]), make_block(0.1, [0, -3, 2])]
        )
        self.assertEqual(list(times), [0, 0.1])
        self.assertEqual(list(peaks), [1.0, 3.0])

    def test_exact_column_ignored(self):
        _, peaks = inst.peaks_over_time([make_block(0.05, [0, 0.5, 0], exact=[0, 100, 0])])
        self.assertEqual(list(peaks), [0.5])

    def test_sign_ignored(self):
        _, peaks = inst.peaks_over_time([make_block(0, [-4, 1, 2])])
        self.assertEqual(list(peaks), [4.0])

    def test_one_entry_per_block(self):
        times, peaks = inst.peaks_over_time([make_block(i * 0.1, [0, 1, 0]) for i in range(5)])
        self.assertEqual((len(times), len(peaks)), (5, 5))

    def test_empty_raises(self):
        with self.assertRaises(ValueError):
            inst.peaks_over_time([])


class ExpectsWarningTest(unittest.TestCase):
    def test_values(self):
        self.assertFalse(inst.expects_warning(0.45))
        self.assertFalse(inst.expects_warning(0.5))
        self.assertTrue(inst.expects_warning(0.51))
        self.assertTrue(inst.expects_warning(0.6))


class CheckStderrTest(unittest.TestCase):
    def test_expected_warning_present(self):
        inst.check_stderr("Warning: value 0.5998 is not suitable", 0.6)

    def test_expected_warning_missing(self):
        with self.assertRaises(ValueError) as ctx:
            inst.check_stderr("", 0.6)
        self.assertIn("0.6", str(ctx.exception))

    def test_stable_empty(self):
        inst.check_stderr("", 0.45)

    def test_stable_whitespace_only(self):
        inst.check_stderr(" \n", 0.45)

    def test_stable_with_warning_raises(self):
        with self.assertRaises(ValueError) as ctx:
            inst.check_stderr("Warning: x", 0.45)
        self.assertIn("0.45", str(ctx.exception))

    def test_stable_with_error_raises(self):
        with self.assertRaises(ValueError):
            inst.check_stderr("Error: x", 0.45)


class CheckBehaviourTest(unittest.TestCase):
    def test_stable_decaying(self):
        inst.check_behaviour(0.45, [1, 0.8, 0.37])

    def test_stable_growing_raises(self):
        with self.assertRaises(ValueError):
            inst.check_behaviour(0.45, [1, 1.2])

    def test_stable_flat_raises(self):
        with self.assertRaises(ValueError):
            inst.check_behaviour(0.45, [1, 1])

    def test_unstable_blowup(self):
        inst.check_behaviour(0.6, [1, 5, 1e30])

    def test_unstable_decaying_raises(self):
        with self.assertRaises(ValueError):
            inst.check_behaviour(0.6, [1, 0.5])

    def test_unstable_just_under_ten_raises(self):
        with self.assertRaises(ValueError):
            inst.check_behaviour(0.6, [1, 9.9])

    def test_unstable_just_over_ten(self):
        inst.check_behaviour(0.6, [1, 10.1])

    def test_single_peak_raises(self):
        with self.assertRaises(ValueError):
            inst.check_behaviour(0.45, [1])


class LegendLabelTest(unittest.TestCase):
    def test_labels(self):
        self.assertEqual(inst.legend_label(0.45), "r = 0.45 (requested)")
        self.assertEqual(inst.legend_label(0.6), "r = 0.6 (requested)")
        self.assertEqual(inst.legend_label(0.5), "r = 0.5 (requested)")


if __name__ == "__main__":
    unittest.main()
