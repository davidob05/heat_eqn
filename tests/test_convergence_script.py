import math
import sys
import unittest
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "scripts"))
import convergence as c  # noqa: E402

MAIN = ROOT / "build" / "release" / "Main"


class MaxErrorTest(unittest.TestCase):
    def test_identical_is_zero(self):
        self.assertEqual(c.max_error([1, 2, 3], [1, 2, 3]), 0.0)

    def test_largest_absolute_difference(self):
        self.assertEqual(c.max_error([1, 2, 3], [1.5, 2, 1]), 2.0)
        self.assertEqual(c.max_error([0, 0], [-3, 1]), 3.0)

    def test_length_mismatch(self):
        with self.assertRaises(ValueError):
            c.max_error([1, 2], [1, 2, 3])


class L2ErrorTest(unittest.TestCase):
    def test_identical_is_zero(self):
        self.assertEqual(c.l2_error([1, 2], [1, 2], 0.1), 0.0)

    def test_constant_difference(self):
        d, m, dx = 0.3, 5, 0.25
        self.assertAlmostEqual(c.l2_error(np.full(m, d), np.zeros(m), dx), math.sqrt(dx * m * d * d))

    def test_dx_scaling(self):
        a, b = [1.0, 2.0, 3.0], [0.0, 0.0, 0.0]
        self.assertAlmostEqual(c.l2_error(a, b, 0.05) / c.l2_error(a, b, 0.1), 1 / math.sqrt(2))

    def test_length_mismatch(self):
        with self.assertRaises(ValueError):
            c.l2_error([1], [1, 2], 0.1)

    def test_hand_computed(self):
        # d = (1, 2, 2): sum d^2 = 9, dx = 1/9 -> sqrt(1) = 1
        self.assertAlmostEqual(c.l2_error([1, 2, 2], [0, 0, 0], 1 / 9), 1.0)


class OrderTest(unittest.TestCase):
    def test_second_order(self):
        self.assertAlmostEqual(c.observed_order(4e-3, 1e-3, 0.2, 0.1), 2.0)

    def test_first_order(self):
        self.assertAlmostEqual(c.observed_order(2e-3, 1e-3, 0.2, 0.1), 1.0)

    def test_non_halving_ratio(self):
        self.assertAlmostEqual(c.observed_order(9e-3, 1e-3, 0.3, 0.1), 2.0)

    def test_equal_errors(self):
        self.assertAlmostEqual(c.observed_order(1e-3, 1e-3, 0.2, 0.1), 0.0)

    def test_nonpositive_error_raises(self):
        with self.assertRaises(ValueError):
            c.observed_order(1e-3, 0.0, 0.2, 0.1)
        with self.assertRaises(ValueError):
            c.observed_order(-1e-3, 1e-3, 0.2, 0.1)


class SeriesTest(unittest.TestCase):
    def test_first_is_none_and_length(self):
        orders = c.orders_for_series([0.4, 0.2, 0.1], [16, 4, 1])
        self.assertIsNone(orders[0])
        self.assertEqual(len(orders), 3)

    def test_uses_row_above(self):
        orders = c.orders_for_series([0.4, 0.2, 0.1], [16, 4, 2])
        self.assertAlmostEqual(orders[1], 2.0)
        self.assertAlmostEqual(orders[2], 1.0)

    def test_single_row(self):
        self.assertEqual(c.orders_for_series([0.1], [1e-3]), [None])


class FinalTimeTest(unittest.TestCase):
    def test_match(self):
        c.check_final_time(0.1, 0.1)

    def test_mismatch(self):
        with self.assertRaises(ValueError) as ctx:
            c.check_final_time(0.09, 0.1)
        self.assertIn("0.09", str(ctx.exception))
        self.assertIn("0.1", str(ctx.exception))

    def test_float_noise(self):
        c.check_final_time(0.1 + 1e-17, 0.1)
        c.check_final_time(0.1 * (1 + 1e-12), 0.1)


class TimeStepsTest(unittest.TestCase):
    def test_study_values(self):
        self.assertEqual(c.time_steps(20, 1.0, 1.0, 0.1, 0.4), 100)
        self.assertEqual(c.time_steps(640, 1.0, 1.0, 0.1, 0.4), 102400)

    def test_non_integer_raises(self):
        with self.assertRaises(ValueError):
            c.time_steps(21, 1.0, 1.0, 0.1, 0.4)


class TableTest(unittest.TestCase):
    ROWS = [
        dict(n=10, dx=0.1, steps=25, max_err=4e-3, max_order=None, l2_err=3e-3, l2_order=None),
        dict(n=20, dx=0.05, steps=100, max_err=1.0625e-3, max_order=1.91, l2_err=7.51e-4, l2_order=2.0),
    ]

    def test_layout(self):
        lines = c.format_table(self.ROWS).splitlines()
        self.assertEqual(
            lines[0], "| N | dx | time steps | max error | max-error order | L2 error | L2 order |"
        )
        self.assertEqual(len(lines), 2 + len(self.ROWS))
        self.assertEqual({line.count("|") for line in lines}, {8})

    def test_first_row_dashes(self):
        row = c.format_table(self.ROWS).splitlines()[2]
        self.assertEqual(row.split(" | ")[4], "-")
        self.assertNotIn("nan", row.lower())
        self.assertNotIn("None", row)

    def test_formats(self):
        self.assertEqual(
            c.format_table(self.ROWS).splitlines()[3],
            "| 20 | 0.05 | 100 | 1.0625e-03 | 1.91 | 7.5100e-04 | 2.00 |",
        )


class ReferenceLineTest(unittest.TestCase):
    def test_passes_through_point_with_slope_two(self):
        dxs, errs = [0.4, 0.2, 0.1], [9.0, 2.0, 0.5]
        ref = c.reference_line(dxs, errs, 1)
        self.assertAlmostEqual(ref[1], 2.0)
        slope = math.log(ref[0] / ref[2]) / math.log(dxs[0] / dxs[2])
        self.assertAlmostEqual(slope, 2.0)

    def test_offset_shifts_by_constant_factor_and_keeps_slope(self):
        dxs, errs = [0.4, 0.2, 0.1], [9.0, 2.0, 0.5]
        base = c.reference_line(dxs, errs, 1)
        shifted = c.reference_line(dxs, errs, 1, offset=3.0)
        np.testing.assert_allclose(shifted, 3.0 * base)


@unittest.skipUnless(MAIN.is_file(), "release Main not built")
class CrossCheckTest(unittest.TestCase):
    def test_n20_matches_cpp_test(self):
        import tempfile
        args = c.parse_args(["--csv-dir", tempfile.mkdtemp()])
        args.exe = MAIN
        row = c.run_case(args, 20)
        self.assertAlmostEqual(row["max_err"], 1.0625e-3, delta=5e-8)
        self.assertAlmostEqual(row["l2_err"], 7.51e-4, delta=5e-6)


if __name__ == "__main__":
    unittest.main()
