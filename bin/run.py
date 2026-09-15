#!/usr/bin/env python3
"""Run weekly stats pipeline for a given year and week."""

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path


def run_step(cmd: list[str], cwd: Path, input_text: str | None = None) -> None:
    print(f"==> {' '.join(cmd)}")
    subprocess.run(cmd, cwd=str(cwd), input=input_text, text=True, check=True)


def main() -> int:
    parser = argparse.ArgumentParser(description="Run stats pipeline for year/week")
    parser.add_argument("year", type=int, help="Season year, e.g. 2026")
    parser.add_argument("week", type=int, help="Week number, e.g. 1")
    args = parser.parse_args()

    if args.week < 1:
        raise SystemExit("week must be >= 1")

    year = str(args.year)
    week = str(args.week)

    bin_dir = Path(__file__).resolve().parent
    root_dir = bin_dir.parent

    python = sys.executable
    steps = [
        ([python, str(bin_dir / "prepare_stats.py"), year, week], None),
        ([str(bin_dir / "convert"), year, week], "y\n"),
        ([str(bin_dir / "weekly_data"), year, week], None),
        ([python, str(root_dir / "weekly2db.py"), year, week], None),
        ([python, str(root_dir / "zipup.py"), year, week], None),
        ([python, str(root_dir / "email_stats.py"), year, week], None),
    ]

    try:
        for cmd, input_text in steps:
            run_step(cmd, root_dir, input_text=input_text)
    except subprocess.CalledProcessError as exc:
        print(f"Step failed with exit code {exc.returncode}: {' '.join(exc.cmd)}", file=sys.stderr)
        return exc.returncode

    print("Pipeline completed successfully.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
