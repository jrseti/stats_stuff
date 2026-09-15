#!/usr/bin/env python3
"""Prepare weekly entirestats text from raw source.

Usage:
    ./prepare_stats.py 2026 1

Reads:
    stats/<year>/Week<week>/entirestats_raw.txt

Writes:
    stats/<year>/Week<week>/entirestats.txt
"""

from __future__ import annotations

import argparse
from datetime import date, timedelta
from pathlib import Path
from typing import Iterable


def labor_day(season_year: int) -> date:
    """Return Labor Day (first Monday of September) for a season year."""
    d = date(season_year, 9, 1)
    while d.weekday() != 0:  # Monday
        d += timedelta(days=1)
    return d


def week_monday_label(season_year: int, week: int) -> str:
    """Return the week banner line used before each offense section."""
    monday = labor_day(season_year) + timedelta(days=7 * week)
    return f"WEEK {week} / THROUGH MONDAY, {monday.strftime('%B').upper()} {monday.day}, {monday.year}"


def normalize_line(line: str) -> str:
    """Collapse repeated whitespace while preserving a single indent for sub-rows."""
    stripped = line.strip()
    if not stripped:
        return ""
    normalized = " ".join(stripped.split())
    if line[:1].isspace():
        return " " + normalized
    return normalized


def parse_category_header(line: str) -> str:
    """Convert CATEGORY header to just team columns."""
    tokens = line.strip().split()
    if not tokens:
        return ""
    if tokens[0].upper() == "CATEGORY":
        tokens = tokens[1:]
    return " ".join(tokens)


def transform_lines(lines: Iterable[str], season_year: int, week: int) -> list[str]:
    out: list[str] = []
    current_section: str | None = None
    week_banner = week_monday_label(season_year, week)
    src = list(lines)
    i = 0

    while i < len(src):
        raw = src[i].rstrip("\n")
        stripped = raw.strip()

        if stripped == "TEAM OFFENSIVE TOTALS" or stripped == "TEAM DEFENSIVE TOTALS":
            section = "OFFENSE" if "OFFENSIVE" in stripped else "DEFENSE"
            current_section = section

            i += 1
            while i < len(src) and not src[i].strip():
                i += 1

            category_line = src[i].rstrip("\n") if i < len(src) else ""
            teams = parse_category_header(category_line)

            if out and out[-1] != "":
                out.append("")

            if section == "OFFENSE":
                out.append(week_banner)
                out.append("")

            out.append(f"{section} {teams}".rstrip())

            i += 1
            continue

        normalized = normalize_line(raw)

        normalized_no_indent = normalized.lstrip()
        if current_section == "OFFENSE" and normalized_no_indent.startswith("KO Rec in EZ"):
            i += 1
            continue

        if not normalized:
            if out and out[-1] != "":
                out.append("")
            i += 1
            continue

        out.append(normalized)
        i += 1

    while out and out[-1] == "":
        out.pop()

    return out


def build_paths(base_dir: Path, year: int, week: int) -> tuple[Path, Path]:
    week_dir = base_dir / "stats" / str(year) / f"Week{week:02d}"
    return week_dir / "entirestats_raw.txt", week_dir / "entirestats.txt"


def main() -> int:
    parser = argparse.ArgumentParser(description="Prepare entirestats.txt from entirestats_raw.txt")
    parser.add_argument("year", type=int, help="Season year, e.g. 2026")
    parser.add_argument("week", type=int, help="Week number, e.g. 1")
    args = parser.parse_args()

    if args.week < 1:
        raise SystemExit("week must be >= 1")

    base_dir = Path.cwd()
    in_path, out_path = build_paths(base_dir, args.year, args.week)

    if not in_path.exists():
        raise SystemExit(f"Input file not found: {in_path}")

    transformed = transform_lines(in_path.read_text(encoding="utf-8").splitlines(), args.year, args.week)
    out_path.write_text("\n".join(transformed) + "\n", encoding="utf-8")

    print(f"Wrote {out_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())