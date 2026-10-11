import contextlib
import io
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "scripts"))
import plot  # noqa: E402

HEADER = "t,x,u_numeric,u_exact\n"


def block(t, values):
    """Rows for one snapshot at time t over x = 0, 0.5, 1."""
    xs = [0.0, 0.5, 1.0]
    return "".join(f"{t},{x},{u},{u}\n" for x, u in zip(xs, values))


class LoadBlocksTest(unittest.TestCase):
    def load(self, text):
        with tempfile.TemporaryDirectory() as d:
            path = Path(d) / "run.csv"
            path.write_text(text)
            return plot.load_blocks(path)

    def assert_rejected(self, text, *fragments):
        with self.assertRaises(ValueError) as ctx:
            self.load(text)
        for fragment in fragments:
            self.assertIn(fragment, str(ctx.exception))

    def test_valid_file_gives_blocks_in_column_order(self):
        blocks = self.load(HEADER + block(0, [0, 1, 0]) + block(0.1, [0, 0.5, 0]))
        self.assertEqual(len(blocks), 2)
        self.assertEqual(blocks[0].shape, (3, 4))
        self.assertEqual(blocks[1][1].tolist(), [0.1, 0.5, 0.5, 0.5])

    def test_swapped_header_is_rejected(self):
        self.assert_rejected(
            "t,x,u_exact,u_numeric\n" + block(0, [0, 1, 0]) + block(0.1, [0, 0.5, 0]),
            "header",
        )

    def test_header_only_is_rejected(self):
        self.assert_rejected(HEADER, "no data rows")

    def test_header_and_one_data_row_is_rejected(self):
        self.assert_rejected(HEADER + "0,0,0,0\n", "truncated")

    def test_single_block_is_rejected(self):
        self.assert_rejected(HEADER + block(0, [0, 1, 0]), "truncated")

    def test_cut_inside_first_block_is_rejected(self):
        # two of the three rows of the t=0 block: one block of two rows
        self.assert_rejected(HEADER + block(0, [0, 1, 0]).splitlines(True)[0] + "0,0.5,1,1\n", "truncated")

    def test_cut_inside_a_later_block_is_rejected(self):
        text = HEADER + block(0, [0, 1, 0]) + block(0.1, [0, 0.5, 0])
        self.assert_rejected(text.rsplit("\n", 2)[0] + "\n", "truncated")

    def test_row_cut_mid_line_is_rejected(self):
        text = HEADER + block(0, [0, 1, 0]) + block(0.1, [0, 0.5, 0])
        self.assert_rejected(text.rstrip("\n").rsplit(",", 1)[0], "truncated")

    def test_non_finite_values_are_rejected_with_first_time(self):
        text = HEADER + block(0, [0, 1, 0]) + block(0.1, [0, 0.5, 0]) + block(0.2, [0, float("nan"), 0])
        self.assert_rejected(text, "non-finite", "t=0.2", "diverged")

    def test_infinite_values_are_rejected(self):
        text = HEADER + block(0, [0, 1, 0]) + block(0.1, [0, float("inf"), 0])
        self.assert_rejected(text, "non-finite", "t=0.1")

    def test_times_must_increase(self):
        self.assert_rejected(HEADER + block(0.1, [0, 1, 0]) + block(0, [0, 0.5, 0]), "increasing")


class MainTest(unittest.TestCase):
    def run_main(self, text):
        with tempfile.TemporaryDirectory() as d:
            path = Path(d) / "run.csv"
            path.write_text(text)
            err = io.StringIO()
            with contextlib.redirect_stderr(err), contextlib.redirect_stdout(io.StringIO()):
                code = plot.main([str(path)])
            return code, err.getvalue(), (Path(d) / "run.png").exists()

    def test_valid_file_saves_a_figure(self):
        code, _, saved = self.run_main(HEADER + block(0, [0, 1, 0]) + block(0.1, [0, 0.5, 0.1]))
        self.assertEqual(code, 0)
        self.assertTrue(saved)

    def test_bad_files_exit_non_zero_and_save_nothing(self):
        for text in (
            HEADER + "0,0,0,0\n",
            HEADER + block(0, [0, 1, 0]) + block(0.1, [0, float("nan"), 0]),
        ):
            code, err, saved = self.run_main(text)
            self.assertEqual(code, 1)
            self.assertIn("Error", err)
            self.assertFalse(saved)


if __name__ == "__main__":
    unittest.main()
